// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/ChoiceImpl.h --
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_CHOICEIMPL_H
#define __SYDNEY_PLAN_SCALAR_CHOICEIMPL_H

#include "Plan/Scalar/Choice.h"
#include "Plan/Scalar/Impl/FunctionImpl.h"

#include "Plan/Interface/IFile.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Utility/ObjectSet.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace ChoiceImpl
{
	////////////////////////////
	// STRUCT
	//	ConvertOperandArgument --
	//
	// NOTES
	struct ConvertOperandArgument
	{
		// input values
		Interface::IRelation* m_pRelation;
		Interface::IScalar* m_pFunction;
		Schema::Field::Function::Value m_eFunction;

		// return values
		Relation::Table* m_pTable;
		VECTOR<Scalar::Field*> m_vecOperand;

		ConvertOperandArgument(Interface::IRelation* pRelation_,
							   Interface::IScalar* pFunction_,
							   Schema::Field::Function::Value eFunction_)
			: m_pRelation(pRelation_),
			  m_pFunction(pFunction_),
			  m_eFunction(eFunction_),
			  m_pTable(0),
			  m_vecOperand()
		{}
		~ConvertOperandArgument() {}
	};

	////////////////////////////////////////
	// TEMPLATE CLASS
	//	Plan::Scalar::ChoiceImpl::Base -- base class for choice operations
	//
	// TEMPLATE ARGUMENTS
	//	class Handle_
	//
	// NOTES
	template <class Handle_>
	class Base
		: public FunctionImpl::Base<Handle_>
	{
	public:
		typedef FunctionImpl::Base<Handle_> Super;
		typedef Base<Handle_> This;

		// constructor
		Base(Tree::Node::Type eType_,
			 const STRING& cstrName_,
			 typename Handle_::Argument cArgument_)
			: Super(eType_, cstrName_, cArgument_)
		{}
		Base(Tree::Node::Type eType_,
			 const DataType& cDataType_,
			 const STRING& cstrName_,
			 typename Handle_::Argument cArgument_)
			: Super(eType_, cDataType_, cstrName_, cArgument_)
		{}
		// destructor
		virtual ~Base() {}

	////////////////////////
	// Interface::IScalar::
		virtual Interface::IScalar* convertFunction(Opt::Environment& cEnvironment_,
													Interface::IRelation* pRelation_,
													Interface::IScalar* pFunction_,
													Schema::Field::Function::Value eFunction_)
		{
			if (eFunction_ == Schema::Field::Function::Undefined) {
				ConvertOperandArgument cArgument(pRelation_,
												 pFunction_,
												 eFunction_);

				// convert operands
				if (isAll(boost::bind(&This::convertOperand,
									  this,
									  boost::ref(cEnvironment_),
									  _1,
									  boost::ref(cArgument)))) {
					return Field::create(cEnvironment_,
										 getType(),
										 cArgument.m_vecOperand,
										 cArgument.m_pTable,
										 pFunction_);
				}
			}
			return 0;
		}

	protected:
	private:
	////////////////////////
	// Scalar::Function::
	};

	////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::ChoiceImpl::Dyadic -- function class for choice operations
	//
	// NOTES
	typedef Base< Tree::Dyadic<Choice, Interface::IScalar> > Dyadic;

	////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::ChoiceImpl::Nadic -- function class for choice operations
	//
	// NOTES
	typedef Base< Tree::Nadic<Choice, Interface::IScalar> > Nadic;
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_CHOICEIMPL_H

//
//	Copyright (c) 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
