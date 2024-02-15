// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Value/ValueExpression.cpp --
// 
// Copyright (c) 2008, 2010, 2011, 2012, 2013, 2015, 2023 Ricoh Company, Ltd.
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

#include "Analysis/Value/ValueExpression.h"

#include "Analysis/Function/Aggregation.h"
#include "Analysis/Function/Array.h"
#include "Analysis/Function/ArrayConstructor.h"
#include "Analysis/Function/Case.h"
#include "Analysis/Function/ElementReference.h"
#include "Analysis/Function/FullText.h"
#include "Analysis/Function/GroupingElement.h"
#include "Analysis/Function/Invoke.h"
#include "Analysis/Function/Nadic.h"
#include "Analysis/Function/Niladic.h"
#include "Analysis/Function/Numeric.h"
#include "Analysis/Function/Spatial.h"
#include "Analysis/Function/String.h"
#include "Analysis/Predicate/Between.h"
#include "Analysis/Predicate/Combinator.h"
#include "Analysis/Predicate/Comparison.h"
#include "Analysis/Predicate/Contains.h"
#include "Analysis/Predicate/Exists.h"
#include "Analysis/Predicate/In.h"
#include "Analysis/Predicate/IsSubstringOf.h"
#include "Analysis/Predicate/Like.h"
#include "Analysis/Predicate/Similar.h"
#include "Analysis/Query/TableValueConstructor.h"
#include "Analysis/Value/ItemReference.h"
#include "Analysis/Value/Literal.h"
#include "Analysis/Value/PlaceHolder.h"
#include "Analysis/Value/RowSubquery.h"
#include "Analysis/Value/RowValueConstructor.h"
#include "Analysis/Value/VariableReference.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Statement/ValueExpression.h"

_SYDNEY_USING
_SYDNEY_ANALYSIS_USING
_SYDNEY_ANALYSIS_VALUE_USING

//////////////////////////////////////
// Value::ValueExpression
//////////////////////////////////////

// FUNCTION public
//	Value::ValueExpression::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const Value::ValueExpression::Super*
//
// EXCEPTIONS

//static
const Interface::IAnalyzer*
Value::ValueExpression::
create(const Statement::ValueExpression* pStatement_)
{
	switch (pStatement_->getOperator()) {
	case Statement::ValueExpression::op_Literal:
	case Statement::ValueExpression::op_PathName:
	case Statement::ValueExpression::op_Nullobj:
	case Statement::ValueExpression::op_Defaultobj:
		{
			return Literal::create(pStatement_);
		}
	case Statement::ValueExpression::op_Itemref:
		{
			return ItemReference::create(pStatement_);
		}
	case Statement::ValueExpression::op_Placeholder:
		{
			return PlaceHolder::create(pStatement_);
		}
	case Statement::ValueExpression::op_Add:
	case Statement::ValueExpression::op_Sub:
	case Statement::ValueExpression::op_Mul:
	case Statement::ValueExpression::op_Div:
	case Statement::ValueExpression::op_Neg:
	case Statement::ValueExpression::op_Abs:
	case Statement::ValueExpression::op_Mod:
		{
			return Function::Numeric::create(pStatement_);
		}
	case Statement::ValueExpression::op_String_concat:
		{
			return Function::String::create(pStatement_);
		}
	case Statement::ValueExpression::op_And:
	case Statement::ValueExpression::op_Or:
	case Statement::ValueExpression::op_Not:
		{
			return Predicate::Combinator::create(pStatement_);
		}
	case Statement::ValueExpression::op_Eq:
	case Statement::ValueExpression::op_Ne:
	case Statement::ValueExpression::op_Le:
	case Statement::ValueExpression::op_Lt:
	case Statement::ValueExpression::op_Ge:
	case Statement::ValueExpression::op_Gt:
	case Statement::ValueExpression::op_IsNull:
	case Statement::ValueExpression::op_IsNotNull:
		{
			return Predicate::Comparison::create(pStatement_);
		}
	case Statement::ValueExpression::op_Like:
	case Statement::ValueExpression::op_NotLike:
		{
			return Predicate::Like::create(pStatement_);
		}
	case Statement::ValueExpression::op_Similar:
	case Statement::ValueExpression::op_NotSimilar:
		{
			return Predicate::Similar::create(pStatement_);
		}
	case Statement::ValueExpression::op_Exists:
		{
			return Predicate::Exists::create(pStatement_);
		}
	case Statement::ValueExpression::op_Tblconst:
		{
			return Query::TableValueConstructor::create(pStatement_);
		}
	case Statement::ValueExpression::op_Rowconst:
		{
			return RowValueConstructor::create(pStatement_);
		}
	case Statement::ValueExpression::op_Arrayref:
		{
			return Function::ElementReference::create(pStatement_);
		}
	case Statement::ValueExpression::op_Arrayconst:
		{
			return Function::ArrayConstructor::create(pStatement_);
		}
	case Statement::ValueExpression::op_Func:
		{
			switch (pStatement_->getFunction()) {
			default:
			case Statement::ValueExpression::func_Unknown:
			case Statement::ValueExpression::func_Value:
				{
					// never used
					break;
				}
			case Statement::ValueExpression::func_Count:
			case Statement::ValueExpression::func_Avg:
			case Statement::ValueExpression::func_Max:
			case Statement::ValueExpression::func_Min:
			case Statement::ValueExpression::func_Sum:							
				{
					return Function::Aggregation::create(pStatement_);
				}
			case Statement::ValueExpression::func_Char_Length:
			case Statement::ValueExpression::func_Octet_Length:
			case Statement::ValueExpression::func_SubString:
			case Statement::ValueExpression::func_Overlay:
			case Statement::ValueExpression::func_Normalize:
			case Statement::ValueExpression::func_Kwic:
			case Statement::ValueExpression::func_Expand_Synonym:
			case Statement::ValueExpression::func_Word_Count:
			case Statement::ValueExpression::func_FullText_Length:
			case Statement::ValueExpression::func_Char_Join:
				{
					return Function::String::create(pStatement_);
				}
			case Statement::ValueExpression::func_Tf:
			case Statement::ValueExpression::func_Cluster:
			case Statement::ValueExpression::func_ClusterId:
			case Statement::ValueExpression::func_ClusterWord:
			case Statement::ValueExpression::func_Score:
			case Statement::ValueExpression::func_Sectionized:
			case Statement::ValueExpression::func_Word:
			case Statement::ValueExpression::func_WordDf:
			case Statement::ValueExpression::func_WordScale:
			case Statement::ValueExpression::func_Existence:
				{
					return Function::FullText::create(pStatement_);
				}
			case Statement::ValueExpression::func_User:
			case Statement::ValueExpression::func_Session_User:
			case Statement::ValueExpression::func_Current_User:
			case Statement::ValueExpression::func_Current_Path:
			case Statement::ValueExpression::func_Current_Date:
			case Statement::ValueExpression::func_Current_Timestamp:
				{
					return Function::Niladic::create(pStatement_);
				}
			case Statement::ValueExpression::func_Coalesce:
			case Statement::ValueExpression::func_GetMax:
				{
					return Function::Nadic::create(pStatement_);
				}
			case Statement::ValueExpression::func_Cardinality:
				{
					return Function::Array::create(pStatement_);
				}
			case Statement::ValueExpression::func_Grouping_Element:
			   {
				   return Function::GroupingElement::create(pStatement_);
			   }
			case Statement::ValueExpression::func_Invoke:
			   {
				   return Function::Invoke::create(pStatement_);
			   }
			case Statement::ValueExpression::func_Neighbor:
			case Statement::ValueExpression::func_NeighborId:
			case Statement::ValueExpression::func_NeighborDistance:
			   {
				   return Function::Spatial::create(pStatement_);
			   }
			}
		
			break;
		}
	case Statement::ValueExpression::op_Between:
	case Statement::ValueExpression::op_NotBetween:
		{
			return Predicate::Between::create(pStatement_);
		}
	case Statement::ValueExpression::op_In:
	case Statement::ValueExpression::op_NotIn:
		{
			return Predicate::In::create(pStatement_);
		}
	case Statement::ValueExpression::op_Contains:
		{
			return Predicate::Contains::create(pStatement_);
		}
	case Statement::ValueExpression::op_RowSubquery:
		{
			return RowSubquery::create(pStatement_);
		}
	case Statement::ValueExpression::op_Case:
		{
			return Function::Case::create(pStatement_);
		}
	case Statement::ValueExpression::op_Variable:
		{
			return VariableReference::create(pStatement_);
		}
	case Statement::ValueExpression::op_IsSubstringOf:
		{
			return Predicate::IsSubstringOf::create(pStatement_);
		}
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
// Copyright (c) 2008, 2010, 2011, 2012, 2013, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
