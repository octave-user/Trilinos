// @HEADER
// *****************************************************************************
//            NOX: An Object-Oriented Nonlinear Solver Package
//
// Copyright 2002 NTESS and the NOX contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

// NOX Test Lacplace 2D Problem
// ----------------------------
// Simple nonlinear PDE problem.
// This file provides the problem
//
// -\Delta u + \lambda e^u = 0  in \Omega = (0,1) \times (0,1)
//                       u = 0  on \partial \Omega
//
// for use as a driver for various nox-epetra tests

#include "Epetra_ConfigDefs.h"
#ifdef HAVE_MPI
#include <mpi.h>
#include "Epetra_MpiComm.h"
#else
#include "Epetra_SerialComm.h"
#endif
#include "Epetra_Map.h"
#include "Epetra_Vector.h"
#include "Epetra_RowMatrix.h"
#include "Epetra_CrsMatrix.h"
#include "NOX.H"
#include "NOX_Epetra_Interface_Required.H"
#include "NOX_Epetra_Interface_Jacobian.H"
#include "NOX_Epetra_Interface_Preconditioner.H"
#include "NOX_Epetra_LinearSystem_AztecOO.H"

namespace Laplace2D {

// this is required to know the number of lower, upper, left and right
// node for each node of the Cartesian grid (composed by nx \timex ny
// elements)

void
get_myNeighbours( const int i, const int nx, const int ny,
                 int & left, int & right,
                 int & lower, int & upper);


// This function creates a CrsMatrix, whose elements corresponds
// to the discretization of a Laplacian over a Cartesian grid,
// with nx grid point along the x-axis and and ny grid points
// along the y-axis. For the sake of simplicity, I suppose that
// all the nodes in the matrix are internal nodes (Dirichlet
// boundary nodes are supposed to have been already condensated)

Epetra_CrsMatrix *
CreateLaplacian( const int nx, const int ny, const Epetra_Comm * Comm);

// ==========================================================================
// This class contians the main definition of the nonlinear problem at
// hand. A method is provided to compute F(x) for a given x, and another
// method to update the entries of the Jacobian matrix, for a given x.
// As the Jacobian matrix J can be written as
//    J = L + diag(lambda*exp(x[i])),
// where L corresponds to the discretization of a Laplacian, and diag
// is a diagonal matrix with lambda*exp(x[i]). Basically, to update
// the jacobian we simply update the diagonal entries. Similarly, to compute
// F(x), we reset J to be equal to L, then we multiply it by the
// (distributed) vector x, then we add the diagonal contribution
// ==========================================================================

} // namespace Laplace2D


class PDEProblem
{

public:

  // constructor. Requires the number of nodes along the x-axis
  // and y-axis, the value of lambda, and the Epetra_Communicator
  // (to define a Map, which is a linear map in this case)
  PDEProblem(const int nx, const int ny, const double lambda,
         const Epetra_Comm * Comm);

  // destructor
  ~PDEProblem();

  // compute F(x)
  void ComputeF(const Epetra_Vector & x, Epetra_Vector & f);

  // update the Jacobian matrix for a given x
  void UpdateJacobian(const Epetra_Vector & x);

  // returns a pointer to the internally stored matrix
  Epetra_CrsMatrix * GetMatrix()
  {
    return Matrix_;
  }

private:

  int nx_, ny_;
  double hx_, hy_;
  Epetra_CrsMatrix * Matrix_;
  double lambda_;

}; /* class PDEProblem */

// ==========================================================================
// This is the main NOX class for this example. Here we define
// the interface between the nonlinear problem at hand, and NOX.
// The constructor accepts a PDEProblem object. Using a pointer
// to this object, we can update the Jacobian and compute F(x),
// using the definition of our problem. This interface is bit
// crude: For instance, no PrecMatrix nor Preconditioner is specified.
// ==========================================================================

class SimpleProblemInterface : public NOX::Epetra::Interface::Required      ,
                               public NOX::Epetra::Interface::Jacobian      ,
                               public NOX::Epetra::Interface::Preconditioner
{

public:

  //! Constructor
  SimpleProblemInterface( PDEProblem * Problem ) :
    Problem_(Problem) {};

  //! Destructor
  ~SimpleProblemInterface()
  {
  };

  bool computeF(const Epetra_Vector & x, Epetra_Vector & f,
                NOX::Epetra::Interface::Required::FillType F )
  {
    Problem_->ComputeF(x,f);
    return true;
  };

  bool computeJacobian(const Epetra_Vector & x, Epetra_Operator & Jac)
  {

    Problem_->UpdateJacobian(x);
    return true;
  }

  bool computePreconditioner(const Epetra_Vector & x, Epetra_Operator & Op, Teuchos::ParameterList *)
  {

    Problem_->UpdateJacobian(x);
    return true;
  }

  bool computePrecMatrix(const Epetra_Vector & x, Epetra_RowMatrix & M)
  {
    std::cout << "*ERR* SimpleProblem::preconditionVector()\n";
    std::cout << "*ERR* don't use explicit preconditioning" << std::endl;
    throw 1;
  }

private:

  PDEProblem * Problem_;

}; /* class SimpleProblemInterface */

