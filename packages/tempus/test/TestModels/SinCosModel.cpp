//@HEADER
// *****************************************************************************
//          Tempus: Time Integration and Sensitivity Analysis Package
//
// Copyright 2017 NTESS and the Tempus contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
//@HEADER

#include "Tempus_ExplicitTemplateInstantiation.hpp"

#ifdef HAVE_TEMPUS_EXPLICIT_INSTANTIATION
#include "SinCosModel.hpp"
#include "SinCosModel_impl.hpp"

namespace Tempus_Test {
TEMPUS_INSTANTIATE_TEMPLATE_CLASS(SinCosModel)
TEMPUS_INSTANTIATE_TEMPLATE_CLASS(SinCosModelAdjoint)
}  // namespace Tempus_Test

#endif
