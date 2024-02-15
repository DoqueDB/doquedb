// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Value/RowSubquery.cpp --
// 
// Copyright (c) 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Analysis::Value";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Value/RowSubquery.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Interface/IRelation.h"
#include "Plan/Relation/RowElement.h"
#include "Plan/Relation/RowInfo.h"
#include "Plan/Scalar/Subquery.h"

#include "Statement/QueryExpression.h"
#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_VALUE_BEGIN

namespace Impl
{
	// CLASS local
	//	Value::Impl::RowSubqueryImpl -- rowSubquery analyzer
	//
	// NOTES
	class RowSubqueryImpl
		: public Value::RowSubquery
	{
	public:
		typedef RowSubqueryImpl This;
		typedef Value::RowSubquery Super;

		// constructor
		RowSubqueryImpl() : Super() {}
		// destructor
		~RowSubqueryImpl() {}

	/////////////////////////////
	// Interface::IAnalyzer::
		virtual Plan::Interface::IScalar*
					getScalar(Opt::Environment& cEnvironment_,
							  Plan::Interface::IRelation* pRelation_,
							  Statement::Object* pStatement_) const;
		virtual Plan::Relation::RowElement*
					getRowElement(Opt::Environment& cEnvironment_,
								  Plan::Interface::IRelation* pRelation_,
								  Statement::Object* pStatement_) const;

		virtual int getDegree(Opt::Environment& cEnvironment_,
							  Statement::Object* pStatement_) const;
		virtual void addScalar(Opt::Environment& cEnvironment_,
							   Plan::Interface::IRelation* pRelation_,
							   Statement::Object* pStatement_,
							   VECTOR<Plan::Interface::IScalar*>& vecScalar_) const;
		virtual void addColumns(Opt::Environment& cEnvironment_,
								Plan::Relation::RowInfo* pRowInfo_,
								Plan::Interface::IRelation* pRelation_,
								Statement::Object* pStatement_) const;
	protected:
	private:
		Plan::Interface::IRelation* getSubRelation(Opt::Environment& cEnvironment_,
												   Statement::Object* pStatement_,
												   Plan::Utility::RelationSet& cOuterRelation_) const;
		Plan::Interface::IScalar* createScalar(Opt::Environment& cEnvironment_,
											   Plan::Interface::IRelation* pRelation_,
											   Plan::Utility::RelationSet& cOuterRelation_,
											   int iPosition_) const;
	};
}

namespace
{
	// VARIABLE local
	//	$$$::_analyzer -- instance
	//
	// NOTES
	const Impl::RowSubqueryImpl _analyzer;

} // namespace

///////////////////////////////////////////
//	Value::Impl::RowSubqueryImpl

// FUNCTION public
//	Value::Impl::RowSubqueryImpl::getScalar -- generate Scalar from Statement::Object
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
Impl::RowSubqueryImpl::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Plan::Utility::RelationSet cOuterRelation;
	Plan::Interface::IRelation* pSubRelation = getSubRelation(cEnvironment_,
															  pStatement_,
															  cOuterRelation);

	if (pSubRelation->getDegree(cEnvironment_) != 1) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	return createScalar(cEnvironment_,
						pSubRelation,
						cOuterRelation,
						0);
}

// FUNCTION public
//	Value::Impl::RowSubqueryImpl::getRowElement -- generate RowElement from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Relation::RowElement*
//
// EXCEPTIONS

//virtual
Plan::Relation::RowElement*
Impl::RowSubqueryImpl::
getRowElement(Opt::Environment& cEnvironment_,
			  Plan::Interface::IRelation* pRelation_,
			  Statement::Object* pStatement_) const
{
	return Plan::Relation::RowElement::create(cEnvironment_,
											  0, /* no relation */
											  0, /* no position */
											  getScalar(cEnvironment_,
														pRelation_,
														pStatement_));
}

// FUNCTION public
//	Value::Impl::RowSubqueryImpl::getDegree -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Statement::Object* pStatement_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Impl::RowSubqueryImpl::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);
	Statement::QueryExpression* pQE =
		_SYDNEY_DYNAMIC_CAST(Statement::QueryExpression*, pVE->getPrimary());
	; _SYDNEY_ASSERT(pQE);

	return pQE->getAnalyzer2()->getDegree(cEnvironment_, pQE);

}

// FUNCTION public
//	Value::Impl::RowSubqueryImpl::addScalar -- add Scalar from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	VECTOR<Plan::Interface::IScalar*>& vecScalar_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::RowSubqueryImpl::
addScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_,
		  VECTOR<Plan::Interface::IScalar*>& vecScalar_) const
{
	Plan::Utility::RelationSet cOuterRelation;
	Plan::Interface::IRelation* pSubRelation = getSubRelation(cEnvironment_,
															  pStatement_,
															  cOuterRelation);

	int n = pSubRelation->getDegree(cEnvironment_);
	for (int i = 0; i < n; ++i) {
		vecScalar_.PUSHBACK(createScalar(cEnvironment_,
										 pSubRelation,
										 cOuterRelation,
										 i));
	}
}

// FUNCTION public
//	Value::Impl::RowSubqueryImpl::addColumns -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Relation::RowInfo* pRowInfo_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::RowSubqueryImpl::
addColumns(Opt::Environment& cEnvironment_,
		   Plan::Relation::RowInfo* pRowInfo_,
		   Plan::Interface::IRelation* pRelation_,
		   Statement::Object* pStatement_) const
{
	Plan::Utility::RelationSet cOuterRelation;
	Plan::Interface::IRelation* pSubRelation = getSubRelation(cEnvironment_,
															  pStatement_,
															  cOuterRelation);

	int n = pSubRelation->getDegree(cEnvironment_);
	for (int i = 0; i < n; ++i) {
		// create rowelement
		Plan::Relation::RowElement* pElement =
			Plan::Relation::RowElement::create(cEnvironment_,
											   0, /* no relation */
											   0, /* no position */
											   createScalar(cEnvironment_,
															pSubRelation,
															cOuterRelation,
															i));
		// add to result with no specific name
		pRowInfo_->PUSHBACK(Plan::Relation::RowInfo::Element(STRING(), pElement));
	}
}

// FUNCTION private
//	Value::Impl::RowSubqueryImpl::getSubRelation -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Statement::Object* pStatement_
//	Plan::Utility::RelationSet& cOuterRelation_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

Plan::Interface::IRelation*
Impl::RowSubqueryImpl::
getSubRelation(Opt::Environment& cEnvironment_,
			   Statement::Object* pStatement_,
			   Plan::Utility::RelationSet& cOuterRelation_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);
	Statement::QueryExpression* pQE =
		_SYDNEY_DYNAMIC_CAST(Statement::QueryExpression*, pVE->getPrimary());
	; _SYDNEY_ASSERT(pQE);

	Opt::Environment::AutoPop cAutoPop0 =
		cEnvironment_.pushNameScope();
	Opt::Environment::AutoPop cAutoPop1 =
		cEnvironment_.pushStatus(Opt::Environment::Status::Subquery
								 | Opt::Environment::Status::Reset);

	Plan::Interface::IRelation* pResult =
		pQE->getAnalyzer2()->getRelation(cEnvironment_, pQE);

	// set retrieved for all outer reference
	cEnvironment_.retrieveOuterReference();

	// outer relation is returned
	cOuterRelation_ = cEnvironment_.getOuterRelation();

	return pResult;
}

// FUNCTION private
//	Value::Impl::RowSubqueryImpl::createScalar -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	int iPosition_
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS

Plan::Interface::IScalar*
Impl::RowSubqueryImpl::
createScalar(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Plan::Utility::RelationSet& cOuterRelation_,
			 int iPosition_) const
{
	return Plan::Scalar::Subquery::create(cEnvironment_,
										  pRelation_,
										  cOuterRelation_,
										  iPosition_);
}

//////////////////////////////
// Value::RowSubquery
//////////////////////////////

// FUNCTION public
//	Value::RowSubquery::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const RowSubquery*
//
// EXCEPTIONS

//static
const RowSubquery*
RowSubquery::
create(const Statement::ValueExpression* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_VALUE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
