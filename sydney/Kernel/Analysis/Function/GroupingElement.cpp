// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/GroupingElement.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "Analysis/Function/GroupingElement.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Relation/RowElement.h"
#include "Plan/Scalar/Function.h"
#include "Plan/Tree/Node.h"

#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_FUNCTION_BEGIN

namespace Impl
{
	// CLASS local
	//	Function::Impl::GroupingElementImpl -- base class for niladic function analyzer
	//
	// NOTES
	class GroupingElementImpl
		: public Function::GroupingElement
	{
	public:
		typedef GroupingElementImpl This;
		typedef Function::GroupingElement Super;

		// constructor
		GroupingElementImpl() : Super() {}
		// destructor
		~GroupingElementImpl() {}

		// generate Scalar from Statement::Object
		virtual Plan::Interface::IScalar*
					getScalar(Opt::Environment& cEnvironment_,
							  Plan::Interface::IRelation* pRelation_,
							  Statement::Object* pStatement_) const;

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
		case Statement::ValueExpression::func_Grouping_Element:
			{
				return Plan::Tree::Node::GroupingElement;
			}
		}
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	// VARIABLE local
	//	$$$::_analyzer -- instance
	//
	// NOTES
	const Impl::GroupingElementImpl _analyzer;

} // namespace

//////////////////////////////////////
//	Function::Impl::GroupingElementImpl

// FUNCTION public
//	Function::Impl::GroupingElementImpl::getScalar -- generate Scalar from Statement::Object
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
Impl::GroupingElementImpl::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);

	Plan::Utility::RowElementSet cRowElementSet = cEnvironment_.getGroupingColumn();

	if ( cRowElementSet.getSize() != 1) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	return (*cRowElementSet.begin())->getScalar();

}

// FUNCTION private
//	Function::Impl::GroupingElementImpl::createScalar -- 
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
Impl::GroupingElementImpl::
createScalar(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	return getScalar(cEnvironment_,
					 pRelation_,
					 pStatement_);
}

//////////////////////////////
// Function::GroupingElement
//////////////////////////////

// FUNCTION public
//	Function::GroupingElement::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const GroupingElement*
//
// EXCEPTIONS

//static
const GroupingElement*
GroupingElement::
create(const Statement::ValueExpression* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_FUNCTION_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
