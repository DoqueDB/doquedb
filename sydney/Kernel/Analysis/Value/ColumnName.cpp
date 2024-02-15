// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Value/ColumnName.cpp --
// 
// Copyright (c) 2008, 2010, 2023 Ricoh Company, Ltd.
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

#include "Analysis/Value/ColumnName.h"

#include "Common/Assert.h"

#include "Exception/ColumnNotFound.h"
#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Statement/ColumnName.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_VALUE_BEGIN

namespace Impl
{
	// CLASS local
	//	Value::Impl::ColumnNameImpl -- columnName analyzer
	//
	// NOTES
	class ColumnNameImpl
		: public Value::ColumnName
	{
	public:
		typedef ColumnNameImpl This;
		typedef Value::ColumnName Super;

		// constructor
		ColumnNameImpl() : Super() {}
		// destructor
		~ColumnNameImpl() {}

	///////////////////////////
	// Interface::IAnalyzer::
		virtual Plan::Relation::RowElement*
						getRowElement(Opt::Environment& cEnvironment_,
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
	const Impl::ColumnNameImpl _analyzer;

} // namespace

///////////////////////////////////////////
//	Value::Impl::ColumnNameImpl

// FUNCTION public
//	Value::Impl::ColumnNameImpl::getRowElement -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
Plan::Relation::RowElement*
Impl::ColumnNameImpl::
getRowElement(Opt::Environment& cEnvironment_,
			  Plan::Interface::IRelation* pRelation_,
			  Statement::Object* pStatement_) const
{
	Statement::ColumnName* pCN =
		_SYDNEY_DYNAMIC_CAST(Statement::ColumnName*, pStatement_);

	const ModUnicodeString* pColumnName = pCN->getIdentifierString();
	; _SYDNEY_ASSERT(pColumnName);

	Plan::Relation::RowElement* pResult = cEnvironment_.searchScalar(*pColumnName);
	if (pResult == 0) {
		_SYDNEY_THROW1(Exception::ColumnNotFound, pCN->toSQLStatement());
	}
	return pResult;
}

//////////////////////////////
// Value::ColumnName
//////////////////////////////

// FUNCTION public
//	Value::ColumnName::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ColumnName* pStatement_
//	
// RETURN
//	const ColumnName*
//
// EXCEPTIONS

//static
const ColumnName*
ColumnName::
create(const Statement::ColumnName* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_VALUE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2008, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
