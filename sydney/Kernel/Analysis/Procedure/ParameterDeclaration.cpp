// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Procedure/ParameterDeclaration.cpp --
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

#include "Analysis/Procedure/ParameterDeclaration.h"

#include "Common/Assert.h"

#include "Plan/Scalar/DataType.h"
#include "Plan/Scalar/Value.h"

#include "Statement/ParameterDeclaration.h"
#include "Statement/Identifier.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_PROCEDURE_BEGIN

namespace Impl
{
	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Procedure::Impl::ParameterDeclarationImpl -- parameter declaration  analyzer
	//
	// NOTES
	class ParameterDeclarationImpl
		: public Procedure::ParameterDeclaration
	{
	public:
		typedef ParameterDeclarationImpl This;
		typedef Procedure::ParameterDeclaration Super;

		// constructor
		ParameterDeclarationImpl() : Super() {}
		// destructor
		~ParameterDeclarationImpl() {}

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
	const Impl::ParameterDeclarationImpl _analyzer;

} // namespace

////////////////////////////////////////////////////////////
// Analysis::Procedure::Impl::ParameterDeclarationImpl
////////////////////////////////////////////////////////////

// FUNCTION public
//	Procedure::Impl::ParameterDeclarationImpl::addScalar -- 
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
Impl::ParameterDeclarationImpl::
addScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_,
		  VECTOR<Plan::Interface::IScalar*>& vecScalar_) const
{
	Statement::ParameterDeclaration* pPD =
		_SYDNEY_DYNAMIC_CAST(Statement::ParameterDeclaration*, pStatement_);
	; _SYDNEY_ASSERT(pPD);

	Statement::Identifier* pName = pPD->getParameterName();
	; _SYDNEY_ASSERT(pName);
	; _SYDNEY_ASSERT(pName->getIdentifier());

	vecScalar_.PUSHBACK(Plan::Scalar::Value::create(cEnvironment_,
													Plan::Scalar::DataType(pPD->getParameterType()),
													*pName->getIdentifier()));
}

//////////////////////////////////////////
// Procedure::ParameterDeclaration
//////////////////////////////////////////

// FUNCTION public
//	Procedure::ParameterDeclaration::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ParameterDeclaration* pStatement_
//	
// RETURN
//	const ParameterDeclaration*
//
// EXCEPTIONS

//static
const ParameterDeclaration*
ParameterDeclaration::
create(const Statement::ParameterDeclaration* pStatement_)
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
