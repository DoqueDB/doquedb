// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/AggregationImpl.h --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 

#ifndef __SYDNEY_DPLAN_SCALAR_AGGREGATIONIMPL_H
#define __SYDNEY_DPLAN_SCALAR_AGGREGATIONIMPL_H

#include "DPlan/Scalar/Module.h"
#include "DPlan/Scalar/Aggregation.h"
#include "Plan/Scalar/Impl/AggregationImpl.h"
#include "Plan/Scalar/Impl/ArithmeticImpl.h"
#include "Plan/Sql/Argument.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_SCALAR_BEGIN
namespace AggregationImpl
{

	class Monadic
		:public Plan::Scalar::AggregationImpl::Monadic
	{
	public:
		typedef Plan::Scalar::AggregationImpl::Monadic Super;
		typedef Monadic This;
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
		Monadic()
			: Super()
		{}
#endif
		Monadic(Plan::Tree::Node::Type eType_,
				const STRING& cstrName_,
				Plan::Interface::IScalar* pOperand_)
			: Super(eType_, cstrName_, pOperand_)
		{}

		virtual ~Monadic() {}
		
		Monadic(Plan::Tree::Node::Type eType_,
				const Plan::Scalar::DataType& cDataType_,
				const STRING& cstrName_,
				Plan::Interface::IScalar* pOperand_)
			: Super(eType_, cDataType_, cstrName_, pOperand_)
		{}

		void registerToEnvironment(Opt::Environment& cEnvironment_)
		{
			Super::registerToEnvironment(cEnvironment_);
		}

		virtual void createDataType(Opt::Environment& cEnvironment_)
		{
			Super::createDataType(cEnvironment_);
		}

		

		virtual void retrieveFromCascade(Opt::Environment& cEnvironment_,
										 Plan::Sql::Query* pQuery_)
		{
			getOperand()->retrieveFromCascade(cEnvironment_, pQuery_);
		}

		virtual void setDataType(const Plan::Scalar::DataType& cType_)
		{
			Super::setDataType(cType_);
		}


		virtual void DataType(Opt::Environment& cEnvironment_)
		{
			Super::createDataType(cEnvironment_);
		}

		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec_,
								  const Plan::Sql::QueryArgument& cArgument_)
		{
			getOperand()->setParameter(cEnvironment_,
										   cProgram_,
										   pIterator_,
										   cExec_,
										   cArgument_);
		}
		
	protected:
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Plan::Candidate::AdoptArgument& cArgument_,
								 int iDataID_);
	};

	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::AggregationImpl::MonadicWithMonadicOption --
	//					implementation class for aggregation operations
	//
	// NOTES
	class MonadicWithMonadicOption
		: public Plan::Tree::MonadicOption<Monadic, Plan::Interface::IScalar>
	{
	public:
		typedef Plan::Tree::MonadicOption<Monadic, Plan::Interface::IScalar> Super;
		typedef MonadicWithMonadicOption This;


		// constructor
		MonadicWithMonadicOption(Plan::Tree::Node::Type eType_,
								 const STRING& cstrName_,
								 Plan::Interface::IScalar* pOperand_,
								 Plan::Interface::IScalar* pOption_)
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
			: Super(pOption_)
		{
			setOperand(pOperand_);
			setArgument(eType_, cstrName_);
		}
#else
			: Super(eType_, cstrName_, pOperand_, pOption_)
		{}
#endif


/*
		MonadicWithMonadicOption(Plan::Tree::Node::Type eType_,
								 const Common::DataType& cDataType_,
								 const STRING& cstrName_,
								 Plan::Interface::IScalar* pOperand_,
								 Plan::Interface::IScalar* pOption_)
 #ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
			: Super(pOption_)
		{
			setOperand(pOperand_);
			setArgument(eType_, cDataType_, cstrName_);
		}

#else
		: Super(eType_, cDataType_, cstrName_, pOperand_, pOption_)
		{}
#endif
		*/

		// destructor
		virtual ~MonadicWithMonadicOption() {}

	////////////////////////
	// Interface::IScalar::

		virtual void retrieveFromCascade(Opt::Environment& cEnvironment_,
										 Plan::Sql::Query* pQuery_)
		{
			if (getOption()->getType() == Plan::Tree::Node::Distinct
				&& pQuery_->isDistribution()) {
				Super::retrieveFromCascade(cEnvironment_, pQuery_);
			} else {
				pQuery_->addProjectionColumn(this);
				
			}
		}
		
		virtual Plan::Interface::IScalar* convertFunction(Opt::Environment& cEnvironment_,
														  Plan::Interface::IRelation* pRelation_,
														  Plan::Interface::IScalar* pFunction_,
														  Schema::Field::Function::Value eFunction_)
		{return 0;}



		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec_,
								  const Plan::Sql::QueryArgument& cArgument_)
		{
			Super::Super::Super::setParameter(cEnvironment_,
											  cProgram_,
											  pIterator_,
											  cExec_,
											  cArgument_);
		}

		

	protected:
	////////////////////////
	// Scalar::Function::
	private:
	};	

	class Avg
		:public Plan::Scalar::ArithmeticImpl::Dyadic
	{
	public:
		typedef Plan::Scalar::ArithmeticImpl::Dyadic Super;
		typedef Avg This;

		Avg(Plan::Tree::Node::Type eType_,
			const STRING& cstrName_,
			const PAIR<Plan::Interface::IScalar*, Plan::Interface::IScalar*>& cOperand_)
			: Super(eType_, cstrName_, cOperand_)
		{}
		
		Avg(Plan::Tree::Node::Type eType_,
			const Plan::Scalar::DataType& cDataType_,
			const STRING& cstrName_,
			const PAIR<Plan::Interface::IScalar*, Plan::Interface::IScalar*>& cOperand_)
			: Super(eType_, cDataType_, cstrName_, cOperand_)
		{}

		void registerToEnvironment(Opt::Environment& cEnvironment_)
		{
			Super::registerToEnvironment(cEnvironment_);
		}

		virtual void createDataType(Opt::Environment& cEnvironment_)
		{
			Super::createDataType(cEnvironment_);
		}

		virtual void setDataType(const Plan::Scalar::DataType& cType_)
		{
			Super::setDataType(cType_);
		}

		virtual void retrieveFromCascade(Opt::Environment& cEnvironment_,
										 Plan::Sql::Query* pQuery_)
		{
			getOperand0()->retrieveFromCascade(cEnvironment_, pQuery_);
			getOperand1()->retrieveFromCascade(cEnvironment_, pQuery_);
		}

	};
}

_SYDNEY_DPLAN_SCALAR_END
_SYDNEY_DPLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_AGGREGATIONIMPL_H

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

