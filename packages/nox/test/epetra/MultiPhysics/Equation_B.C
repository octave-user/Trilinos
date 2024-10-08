// @HEADER
// *****************************************************************************
//            NOX: An Object-Oriented Nonlinear Solver Package
//
// Copyright 2002 NTESS and the NOX contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

#include "NOX_Common.H"
#include "Epetra_Comm.h"
#include "Epetra_Map.h"
#include "Epetra_Vector.h"
#include "Epetra_Import.h"
#include "Epetra_CrsGraph.h"
#include "Epetra_CrsMatrix.h"
#include "Basis.H"

#include "Problem_Manager.H"
#include "Equation_B.H"

// Constructor - creates the Epetra objects (maps and vectors)
Equation_B::Equation_B(Epetra_Comm& comm, int numGlobalNodes,
                                           std::string name_) :
  GenericEpetraProblem(comm, numGlobalNodes, name_),
  xmin(0.0),
  xmax(1.0),
  dt(1.0e-3),
  id_temp(-1),// Index for needed dependent Temperature vector
  id_vel(-1), // Index for auxilliary velocity vector
  useConvection(false)
{

  // Create mesh and solution vectors

  // We first initialize the mesh and then the solution since the latter
  // can depend on the mesh.
  xptr = Teuchos::rcp( new Epetra_Vector(*StandardMap) );
  double Length= xmax - xmin;
  dx=Length/((double) NumGlobalNodes-1);
  for (int i=0; i < NumMyNodes; i++)
    (*xptr)[i]=xmin + dx*((double) StandardMap->MinMyGID()+i);

  // Create extra vector needed for this transient problem
  oldSolution = new Epetra_Vector(*StandardMap);

  // Next we create and initialize the solution vector
  initialSolution = Teuchos::rcp(new Epetra_Vector(*StandardMap));
  initializeSolution();
//  initialSolution->PutScalar(0.0);

  // Allocate the memory for a matrix dynamically (i.e. the graph is dynamic).
  AA = Teuchos::rcp( new Epetra_CrsGraph(Copy, *StandardMap, 0) );
  generateGraph();

#ifdef DEBUG
  AA->Print(cout);
#endif

  // Create a matrix using the graph just created - this creates a
  // static graph so we can refill the new matirx after FillComplete()
  // is called.
  A = Teuchos::rcp(new Epetra_CrsMatrix (Copy, *AA));
  A->FillComplete();

  // Create the Importer needed for FD coloring
  ColumnToOverlapImporter = new Epetra_Import(A->ColMap(),*OverlapMap);
}

//-----------------------------------------------------------------------------

// Destructor
Equation_B::~Equation_B()
{
  delete oldSolution; oldSolution = 0;
  delete ColumnToOverlapImporter; ColumnToOverlapImporter = 0;
}

//-----------------------------------------------------------------------------

// Reset function
void Equation_B::reset(const Epetra_Vector& x)
{
  *oldSolution = x;
}

//-----------------------------------------------------------------------------

// Initialize based on registrations
void Equation_B::initialize()
{
  // Get id of required Species problem
  map<string, int>::iterator id_ptr = nameToMyIndex.find("Temperature");
  if( id_ptr == nameToMyIndex.end() )
  {
    std::string msg = "ERROR: Equation_B (\"" + myName + "\") could not get "
         + "vector for problem \"Temperature\" !!";
    throw msg;
  }
  else
    id_temp = (*id_ptr).second;

  // Check for dependence on velocity (convection)

  // Rather than merely existing, we should change this to search the
  // dependent problems vector
  id_ptr = nameToMyIndex.find("Burgers");
  if( id_ptr == nameToMyIndex.end() )
  {
    std::cout << "WARNING: Equation_B (\"" << myName << "\") could not get "
         << "vector for problem \"Burgers\". Omitting convection." << std::endl;

    useConvection = false;
  }
  else
  {
    id_vel = (*id_ptr).second;
    useConvection = true;
  }

  return;
}

//-----------------------------------------------------------------------------

// Empty Reset function
void Equation_B::reset()
{
  std::cout << "WARNING: reset called without passing any update vector !!"
       << std::endl;
}

//-----------------------------------------------------------------------------

// Set initialSolution to desired initial condition
void Equation_B::initializeSolution()
{
  // Aliases for convenience
  Epetra_Vector& soln = *initialSolution;
  Epetra_Vector& x = *xptr;

  // Here we do a sinusoidal perturbation of the unstable
  // steady state.

  double pi = 4.*atan(1.0);

  for (int i=0; i<x.MyLength(); i++)
    soln[i] = 10.0/3.0 + 1.e-1*sin(1.0*pi*x[i]);

  *oldSolution = soln;
}

//-----------------------------------------------------------------------------

// Matrix and Residual Fills
bool Equation_B::evaluate(
                    NOX::Epetra::Interface::Required::FillType fillType,
            const Epetra_Vector* soln,
            Epetra_Vector* rhs)
{
  bool fillRes = false;
  bool fillJac = false;

  // We currently make no provision for combined Residual and Jacobian fills
  // i.e. it is either or

  if( NOX::Epetra::Interface::Required::Jac == fillType )
    fillJac = true;
   else
   {
    fillRes = true;
    assert( NULL != rhs );
   }

  int numDep = depProblems.size();

  // Create the overlapped solution and position vectors
  Epetra_Vector u(*OverlapMap);
  Epetra_Vector uold(*OverlapMap);
  std::vector<Epetra_Vector*> dep(numDep);
  for( int i = 0; i<numDep; i++)
    dep[i] = new Epetra_Vector(*OverlapMap);
  Epetra_Vector xvec(*OverlapMap);

  // Export Solution to Overlap vector
  // If the vector to be used in the fill is already in the Overlap form,
  // we simply need to map on-processor from column-space indices to
  // OverlapMap indices. Note that the old solution is simply fixed data that
  // needs to be sent to an OverlapMap (ghosted) vector.  The conditional
  // treatment for the current soution vector arises from use of
  // FD coloring in parallel.
  uold.Import(*oldSolution, *Importer, Insert);
  for( int i = 0; i<numDep; i++ )
    dep[i]->Import(*( (*(depSolutions.find(depProblems[i]))).second ), *Importer, Insert);

  xvec.Import(*xptr, *Importer, Insert);

  if( NOX::Epetra::Interface::Required::FD_Res == fillType )
    // Overlap vector for solution received from FD coloring, so simply reorder
    // on processor
    u.Export(*soln, *ColumnToOverlapImporter, Insert);
  else // Communication to Overlap vector is needed
    u.Import(*soln, *Importer, Insert);

  int OverlapNumMyNodes = OverlapMap->NumMyElements();

  int OverlapMinMyNodeGID;
  if (MyPID==0) OverlapMinMyNodeGID = StandardMap->MinMyGID();
  else OverlapMinMyNodeGID = StandardMap->MinMyGID()-1;

  int row, column;
  double Dcoeff = 0.025;
  double alpha = 0.6;
  double beta = 2.0;
  double jac;
  double xx[2];
  double uu[2];
  double uuold[2];
  std::vector<double*> ddep(numDep);
  for( int i = 0; i<numDep; i++)
    ddep[i] = new double[2];
  Basis basis;

  double convection = 0.0;

  // Zero out the objects that will be filled
  if ( fillJac ) A->PutScalar(0.0);
  if ( fillRes ) rhs->PutScalar(0.0);

  // Loop Over # of Finite Elements on Processor
  for (int ne=0; ne < OverlapNumMyNodes-1; ne++)
  {
    // Loop Over Gauss Points
    for(int gp=0; gp < 2; gp++)
    {
      // Get the solution and coordinates at the nodes
      xx[0]=xvec[ne];
      xx[1]=xvec[ne+1];
      uu[0] = u[ne];
      uu[1] = u[ne+1];
      uuold[0] = uold[ne];
      uuold[1] = uold[ne+1];
      for( int i = 0; i<numDep; i++ )
      {
        ddep[i][0] = (*dep[i])[ne];
        ddep[i][1] = (*dep[i])[ne+1];
      }
      // Calculate the basis function at the gauss point
      basis.getBasis(gp, xx, uu, uuold, ddep);

      // Loop over Nodes in Element
      for (int i=0; i< 2; i++)
      {
    row=OverlapMap->GID(ne+i);
    if (StandardMap->MyGID(row))
        {
      if ( fillRes )
          {
            convection = 0.0;
            if( useConvection )
              convection = basis.ddep[id_vel]*basis.duu/basis.dx;

        (*rhs)[StandardMap->LID(OverlapMap->GID(ne+i))]+=
          + basis.wt*basis.dx
          * ((basis.uu - basis.uuold)/dt * basis.phi[i]
          + convection * basis.phi[i]
          + (1.0/(basis.dx*basis.dx))*Dcoeff*basis.duu*basis.dphide[i]
              + basis.phi[i] * ( -beta*basis.ddep[id_temp]
                 + basis.ddep[id_temp]*basis.ddep[id_temp]*basis.uu) );
      }
    }
    // Loop over Trial Functions
    if ( fillJac )
        {
      for( int j = 0; j < 2; ++j )
          {
        if (StandardMap->MyGID(row))
            {
          column=OverlapMap->GID(ne+j);
          jac=basis.wt*basis.dx*(
                      basis.phi[j]/dt*basis.phi[i]
                      +(1.0/(basis.dx*basis.dx))*Dcoeff*basis.dphide[j]*
                                                        basis.dphide[i]
                      + basis.phi[i] * basis.ddep[id_temp]*basis.ddep[id_temp]*
                  basis.phi[j] );
          A->SumIntoGlobalValues(row, 1, &jac, &column);
        }
      }
    }
      }
    }
  }

  // Insert Boundary Conditions and modify Jacobian and function (F)
  // U(0)=1
  if (MyPID==0)
  {
    if ( fillRes )
      (*rhs)[0]= (*soln)[0] - beta/alpha;
    if ( fillJac )
    {
      int column=0;
      double jac=1.0;
      A->ReplaceGlobalValues(0, 1, &jac, &column);
      column=1;
      jac=0.0;
      A->ReplaceGlobalValues(0, 1, &jac, &column);
    }
  }
  // U(1)=1
  if ( StandardMap->LID(StandardMap->MaxAllGID()) >= 0 )
  {
    int lastDof = StandardMap->LID(StandardMap->MaxAllGID());
    if ( fillRes )
      (*rhs)[lastDof] = (*soln)[lastDof] - beta/alpha;
    if ( fillJac )
    {
      int row=StandardMap->MaxAllGID();
      int column = row;
      double jac = 1.0;
      A->ReplaceGlobalValues(row, 1, &jac, &column);
      jac=0.0;
      column--;
      A->ReplaceGlobalValues(row, 1, &jac, &column);
    }
  }

  // Sync up processors to be safe
  Comm->Barrier();

  A->FillComplete();

#ifdef DEBUG
  A->Print(cout);

  if( fillRes )
    std::cout << "For residual fill :" << std::endl << *rhs << std::endl;

  if( fillJac ) {
    std::cout << "For jacobian fill :" << std::endl;
    A->Print(cout);
  }
#endif

  // Cleanup
  for( int i = 0; i<numDep; i++)
  {
    delete [] ddep[i];
    delete    dep[i];
  }

  return true;
}

//-----------------------------------------------------------------------------

Epetra_Vector& Equation_B::getOldSoln()
{
  return *oldSolution;
}

//-----------------------------------------------------------------------------

void Equation_B::generateGraph()
{

  // Declare required variables
  int i;
  int row, column;
  int OverlapNumMyNodes = OverlapMap->NumMyElements();
  int OverlapMinMyNodeGID;
  if (MyPID==0) OverlapMinMyNodeGID = StandardMap->MinMyGID();
  else OverlapMinMyNodeGID = StandardMap->MinMyGID()-1;

  // Loop Over # of Finite Elements on Processor
  for (int ne=0; ne < OverlapNumMyNodes-1; ne++)
  {
    // Loop over Nodes in Element
    for (i=0; i<2; i++)
    {
      // If this node is owned by current processor, add indices
      if (StandardMap->MyGID(OverlapMap->GID(ne+i)))
      {
        // Loop over unknowns in Node
        row=OverlapMap->GID(ne+i);

        // Loop over supporting nodes
        for(int j = 0; j < 2; ++j )
        {
          // Loop over unknowns at supporting nodes
          column=OverlapMap->GID(ne+j);
          AA->InsertGlobalIndices(row, 1, &column);
        }
      }
    }
  }

  AA->FillComplete();

  return;
}
