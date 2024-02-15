// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Procedure/ReturnsClause.cpp --
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

#include "Analysis/Procedure/ReturnsClause.h"

#include "Common/Assert.h"

#include "Plan/Scalar/DataType.h"
#include "Plan/Scalar/Value.h"

#include "Statement/ReturnsClause.h"
#include "Statement/Identifier.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_PROCEDURE_BEGIN

namespace Impl
{
	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Procedure::Impl::ReturnsClauseImpl -- parameter declaration  analyzer
	//
	// NOTES
	class ReturnsClauseImpl
		: public Procedure::ReturnsClause
	{
	public:
		typedef ReturnsClauseImpl This;
		typedef Procedure::ReturnsClause Super;

		// constructor
		ReturnsClauseImpl() : Super() {}
		// destructor
		~ReturnsClauseImpl() {}

	/////////////////////////////
	//Interface::Analyzer::
		virtual Plan::Interface::IScalar*
					getScalar(Opt::Environment& cEnvironment_,
							  Plan::Interface::IRelation* pRelation_,
							  Statement::Object* pStatement_) const;
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
	const Impl::ReturnsClauseImpl _analyzer;

} // namespace

////////////////////////////////////////////////////////////
// Analysis::Procedure::Impl::ReturnsClauseImpl
////////////////////////////////////////////////////////////

// FUNCTION public
//	Procedure::Impl::ReturnsClauseImpl::getScalar -- 
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
Impl::ReturnsClauseImpl::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Statement::ReturnsClause* pRC =
		_SYDNEY_DYNAMIC_CAST(Statement::ReturnsClause*, pStatement_);
	; _SYDNEY_ASSERT(pRC);

	return Plan::Scalar::Value::create(cEnvironment_,
									   Plan::Scalar::DataType(pRC->getDataType()),
									   STRING());
}

//////////////////////////////////////////
// Procedure::ReturnsClause
//////////////////////////////////////////

// FUNCTION public
//	Procedure::ReturnsClause::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ReturnsClause* pStatement_
//	
// RETURN
//	const ReturnsClause*
//
// EXCEPTIONS

//static
const ReturnsClause*
ReturnsClause::
create(const Statement::ReturnsClause* pStatement_)
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
