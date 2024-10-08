// @HEADER
// *****************************************************************************
//            NOX: An Object-Oriented Nonlinear Solver Package
//
// Copyright 2002 NTESS and the NOX contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

#include "NOX_Common.H"
#include "Basis.H"
#include <vector>

// Constructor
Basis::Basis()
{
  phi = new double[2];
  dphide = new double[2];
}

// Destructor
Basis::~Basis() {
  delete [] phi;
  delete [] dphide;
}

// Calculates a linear 1D basis
void Basis::getBasis( int gp, double *x, double *u, double *uold, std::vector<double*>& dep )
{
  int N = 2;
  if (gp==0) {eta=-1.0/sqrt(3.0); wt=1.0;}
  if (gp==1) {eta=1.0/sqrt(3.0); wt=1.0;}

  // Calculate basis function and derivatives at nodel pts
  phi[0]=(1.0-eta)/2.0;
  phi[1]=(1.0+eta)/2.0;
  dphide[0]=-0.5;
  dphide[1]=0.5;

  // Caculate basis function and derivative at GP.
  dx=0.5*(x[1]-x[0]);
  xx=0.0;
  uu = duu = uuold = duuold = 0.0;
  ddep.clear();
  ddep.assign(dep.size(), 0.0);
  for( int i = 0; i < N; ++i )
  {
    xx += x[i] * phi[i];
    uu += u[i] * phi[i];
    duu += u[i] * dphide[i];
    uuold += uold[i] * phi[i];
    duuold += uold[i] * dphide[i];
    for( unsigned int j = 0; j < dep.size(); ++j )
      ddep[j] += dep[j][i] * phi[i];
  }

  return;
}
