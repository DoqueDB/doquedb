// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Choice.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Plan::Scalar";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Scalar/Choice.h"
#include "Plan/Scalar/Field.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace ChoiceImpl
{
	struct ConvertOperandArgument
	{
		Relation::Table* m_pTable;
		VECTOR<Scalar::Field*> m_vecOperand;

		ConvertOperandArgument()
			: m_pTable(0),
			  m_vecOperand()
		{}
	};
}

////////////////////////////////////////
//	Scalar::Choice

// FUNCTION public
//	Scalar::Choice::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const VECTOR<Interface::IScalar*>& vecOperand_
//	const STRING& cstrName_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//static
Interface::IScalar*
Choice::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const VECTOR<Interface::IScalar*>& vecOperand_,
	   const STRING& cstrName_)
{
	switch (eOperator_) {
	case Tree::Node::GetMax:
		{
			; _SYDNEY_ASSERT(vecOperand_.ISEMPTY() == false);

			ChoiceImpl::ConvertOperandArgument cArgument;
			if (Opt::IsAll(vecOperand_,
						   boost::bind(&This::convertOperand,
									   boost::ref(cEnvironment_),
									   _1,
									   boost::ref(cArgument)))) {
				// create field object
				return Field::create(cEnvironment_,
									 eOperator_,
									 cArgument.m_vecOperand,
									 cArgument.m_pTable,
									 cstrName_);
			}
			break;
		}
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION private
//	Scalar::Choice::convertOperand -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pOperand_
//	ChoiceImpl::ConvertOperandArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Choice::
convertOperand(Opt::Environment& cEnvironment_,
			   Interface::IScalar* pOperand_,
			   ChoiceImpl::ConvertOperandArgument& cArgument_)
{
	if (pOperand_->isField()) {
		Scalar::Field* pField = pOperand_->getField();
		if (cArgument_.m_pTable == 0) {
			// first call
			cArgument_.m_pTable = pField->getTable();
		} else {
			if (cArgument_.m_pTable != pField->getTable()) {
				return false;
			}
		}
		cArgument_.m_vecOperand.PUSHBACK(pField);
		return true;
	}
	return false;
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
