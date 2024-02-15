// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Procedure/ParameterDeclarationList.cpp --
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
const char moduleName[] = "Analysis::Procedure";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Procedure/ParameterDeclarationList.h"

#include "Common/Assert.h"

#include "Statement/ParameterDeclarationList.h"
#include "Statement/ParameterDeclaration.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_PROCEDURE_BEGIN

namespace Impl
{
	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Procedure::Impl::ParameterDeclarationListImpl -- parameter declaration list analyzer
	//
	// NOTES
	class ParameterDeclarationListImpl
		: public Procedure::ParameterDeclarationList
	{
	public:
		typedef ParameterDeclarationListImpl This;
		typedef Procedure::ParameterDeclarationList Super;

		// constructor
		ParameterDeclarationListImpl() : Super() {}
		// destructor
		~ParameterDeclarationListImpl() {}

	/////////////////////////////
	//Interface::Analyzer::
		virtual void addScalar(Opt::Environment& cEnvironment_,
							   Plan::Interface::IRelation* pRelation_,
							   Statement::Object* pStatement_,
							   VECTOR<Plan::Interface::IScalar*>& vecScalar_) const;
	protected:
	private:
	};
}

namespace
{
	// VARIABLE local
	//	$$$::_analyzer -- instance
	//
	// NOTES
	const Impl::ParameterDeclarationListImpl _analyzer;

} // namespace

////////////////////////////////////////////////////////////
// Analysis::Procedure::Impl::ParameterDeclarationListImpl
////////////////////////////////////////////////////////////

// FUNCTION public
//	Procedure::Impl::ParameterDeclarationListImpl::addScalar -- 
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
Impl::ParameterDeclarationListImpl::
addScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_,
		  VECTOR<Plan::Interface::IScalar*>& vecScalar_) const
{
	Statement::ParameterDeclarationList* pPDL =
		_SYDNEY_DYNAMIC_CAST(Statement::ParameterDeclarationList*, pStatement_);
	; _SYDNEY_ASSERT(pPDL);

	int n = pPDL->getCount();
	for (int i = 0; i < n; ++i) {
		Statement::Object* pElement = pPDL->getAt(i);
		pElement->getAnalyzer2()->addScalar(cEnvironment_,
											pRelation_,
											pElement,
											vecScalar_);
	}
}

//////////////////////////////////////////
// Procedure::ParameterDeclarationList
//////////////////////////////////////////

// FUNCTION public
//	Procedure::ParameterDeclarationList::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ParameterDeclarationList* pStatement_
//	
// RETURN
//	const ParameterDeclarationList*
//
// EXCEPTIONS

//static
const ParameterDeclarationList*
ParameterDeclarationList::
create(const Statement::ParameterDeclarationList* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_PROCEDURE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
