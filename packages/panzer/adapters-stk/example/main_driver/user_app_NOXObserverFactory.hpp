// @HEADER
// ***********************************************************************
//
//           Panzer: A partial differential equation assembly
//       engine for strongly coupled complex multiphysics systems
//                 Copyright (2011) Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Roger P. Pawlowski (rppawlo@sandia.gov) and
// Eric C. Cyr (eccyr@sandia.gov)
// ***********************************************************************
// @HEADER

#ifndef USER_APP_NOX_OBSERVER_FACTORY_HPP
#define USER_APP_NOX_OBSERVER_FACTORY_HPP

#include "PanzerAdaptersSTK_config.hpp"

#include "Panzer_STK_NOXObserverFactory.hpp"
#include "Panzer_ResponseLibrary.hpp"
#include "Panzer_Traits.hpp"
#include "NOX_PrePostOperator_Vector.H"

#include "Teuchos_ParameterList.hpp"
#include "Teuchos_ParameterListAcceptorDefaultBase.hpp"
#include "Teuchos_StandardParameterEntryValidators.hpp"

// Individual Observers
#include "user_app_NOXObserver_WriteToExodus.hpp"
#include "user_app_NOXObserver_NeumannBCAnalyticSystemTest.hpp"
#include "NOX_Observer_ReusePreconditionerFactory.hpp"
#include "NOX_Observer_Print.hpp"

namespace user_app {
  
  class NOXObserverFactory :
    public panzer_stk::NOXObserverFactory,
    public Teuchos::ParameterListAcceptorDefaultBase  {
    
    //! Store STK IO response library...be careful, it will be modified externally
    Teuchos::RCP<panzer::ResponseLibrary<panzer::Traits> > stkIOResponseLibrary_;

    mutable Teuchos::RCP<Teuchos::ParameterList> valid_params_;

  public:

    NOXObserverFactory(const Teuchos::RCP<panzer::ResponseLibrary<panzer::Traits> > & stkIOResponseLibrary)
       : stkIOResponseLibrary_(stkIOResponseLibrary) {}
    
    Teuchos::RCP<NOX::Abstract::PrePostOperator>
    buildNOXObserver(const Teuchos::RCP<panzer_stk::STK_Interface>& mesh,
		     const Teuchos::RCP<const panzer::GlobalIndexer>& dof_manager,
		     const Teuchos::RCP<const panzer::LinearObjFactory<panzer::Traits> >& lof) const
    {
      using Teuchos::RCP;
      using Teuchos::rcp;
      using Teuchos::ParameterList;

      TEUCHOS_ASSERT(nonnull(this->getParameterList()));

      RCP<NOX::PrePostOperatorVector> observer = rcp(new NOX::PrePostOperatorVector);

      // Delay the preconditioner update
      if (this->getParameterList()->get<bool>("Delay Preconditioner Update")) {
        Teuchos::ParameterList delay_list = this->getParameterList()->sublist("Delay Preconditioner Factory Sublist");
        auto ppo = NOX::createReusePreconditionerObserver(delay_list);
	observer->pushBack(ppo);
      }

      // New output format via observer
      if (this->getParameterList()->get<bool>("New Output Format")) {
        auto utils = Teuchos::rcp(new NOX::Utils(0,mesh->getComm()->getRank(),0,6));
	Teuchos::RCP<NOX::Abstract::PrePostOperator> po = 
	  Teuchos::rcp(new NOX::ObserverPrint(utils));
	observer->pushBack(po);
      }

      // Exodus writer to output solution
      if (this->getParameterList()->get<std::string>("Write Solution to Exodus File") == "ON") {
	Teuchos::RCP<NOX::Abstract::PrePostOperator> solution_writer = 
	  Teuchos::rcp(new user_app::NOXObserver_WriteToExodus(mesh,dof_manager,lof,stkIOResponseLibrary_));
	observer->pushBack(solution_writer);
      }
      
      // Neumann BC unit test
      if (this->getParameterList()->get<std::string>("Neumann BC Analytic System Test") == "ON") {
	Teuchos::RCP<NOX::Abstract::PrePostOperator> ppo = 
	  Teuchos::rcp(new user_app::NOXObserver_NeumannBCAnalyticSystemTest);
	observer->pushBack(ppo);
      }

      return observer;
    }

    /** \name Overridden from Teuchos::ParameterListAcceptor */
    //@{
    
    void setParameterList(Teuchos::RCP<Teuchos::ParameterList> const& paramList)
    {
      using Teuchos::RCP;
      using Teuchos::rcp;

      paramList->validateParametersAndSetDefaults(*(this->getValidParameters()),0);
      setMyParamList(paramList);
    }
    
    Teuchos::RCP<const Teuchos::ParameterList> getValidParameters() const
    {
      if (valid_params_.is_null()) {
	
	valid_params_ = Teuchos::rcp(new Teuchos::ParameterList);

        valid_params_->set<std::string>("Write Solution to Exodus File", "ON",
                                        "Enables or disables writing of solution to Exodus file at end of NOX solve",
                                        rcp(new Teuchos::StringValidator(Teuchos::tuple<std::string>("ON", "OFF"))));
	
        valid_params_->set<std::string>("Neumann BC Analytic System Test", "OFF",
                                        "Checks solution values for Neumann BC Analytic System Test",
                                        rcp(new Teuchos::StringValidator(Teuchos::tuple<std::string>("ON", "OFF"))));

        valid_params_->set<bool>("Delay Preconditioner Update",false,"Sets an observer that controls when to update the preconditioner.");
        valid_params_->sublist("Delay Preconditioner Factory Sublist",false,"Sublist used to build the ReusePreconditioner observer.");

        valid_params_->set<bool>("New Output Format",false,"Enables new output format from observer.");
      }
      return valid_params_;
    }
 
    //@}

  };

}

#endif
