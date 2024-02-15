// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Niladic.cpp --
// 
// Copyright (c) 2010, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Analysis::Function";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Function/Niladic.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Plan/Scalar/Function.h"
#include "Plan/Tree/Node.h"

#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_FUNCTION_BEGIN

namespace Impl
{
	// CLASS local
	//	Function::Impl::NiladicImpl -- base class for niladic function analyzer
	//
	// NOTES
	class NiladicImpl
		: public Function::Niladic
	{
	public:
		typedef NiladicImpl This;
		typedef Function::Niladic Super;

		// constructor
		NiladicImpl() : Super() {}
		// destructor
		~NiladicImpl() {}

	protected:
	private:
	/////////////////////
	// Base::
		virtual Plan::Interface::IScalar*
					createScalar(Opt::Environment& cEnvironment_,
								 Plan::Interface::IRelation* pRelation_,
								 Statement::Object* pStatement_) const;

	};
} // namespace Impl

namespace
{
	// FUNCTION local
	//	$$$::_getNodeType -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	int iFunc_
	//	
	// RETURN
	//	Plan::Tree::Node::Type
	//
	// EXCEPTIONS

	Plan::Tree::Node::Type
	_getNodeType(int iFunc_)
	{
		switch (iFunc_) {
		case Statement::ValueExpression::func_Current_Date:
			{
				return Plan::Tree::Node::CurrentDate;
			}
		case Statement::ValueExpression::func_Current_Timestamp:
			{
				return Plan::Tree::Node::CurrentTimestamp;
			}
		}
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	// VARIABLE local
	//	$$$::_analyzer -- instance
	//
	// NOTES
	const Impl::NiladicImpl _analyzer;

} // namespace

//////////////////////////////////////
//	Function::Impl::NiladicImpl

// FUNCTION private
//	Function::Impl::NiladicImpl::createScalar -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS

//virtual
Plan::Interface::IScalar*
Impl::NiladicImpl::
createScalar(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);

	return Plan::Scalar::Function::create(cEnvironment_,
										  _getNodeType(pVE->getFunction()),
										  pVE->toSQLStatement());
}

//////////////////////////////
// Function::Niladic
//////////////////////////////

// FUNCTION public
//	Function::Niladic::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const Niladic*
//
// EXCEPTIONS

//static
const Niladic*
Niladic::
create(const Statement::ValueExpression* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_FUNCTION_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
