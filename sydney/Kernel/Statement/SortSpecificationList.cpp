// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SortSpecificationList.cpp -- SortSpecificationList
// 
// Copyright (c) 1999, 2002, 2003, 2005, 2006, 2008, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Statement";
const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Statement/SortSpecificationList.h"
#include "Statement/Type.h"
#include "Statement/SortSpecification.h"
#include "Statement/Utility.h"
#include "Statement/ValueExpression.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModOstrStream.h"
#if 0
#include "Analysis/SortSpecificationList.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

//
//	FUNCTION public
//		Statement::SortSpecificationList::SortSpecificationList -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		SortSpecification* pSortSpecification_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
SortSpecificationList::SortSpecificationList(SortSpecification* pSortSpecification_)
	: ObjectList(ObjectType::SortSpecificationList),
	  m_pPartitionBy(0)
{
	// SortSpecification を加える
	append(pSortSpecification_);
}

// FUNCTION public
//	Statement::SortSpecificationList::SortSpecificationList -- コピーコンストラクタ
//
// NOTES
//
// ARGUMENTS
//	const SortSpecificationList& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

SortSpecificationList::
SortSpecificationList(const SortSpecificationList& cOther_)
	: ObjectList(cOther_),
	  m_pPartitionBy(0)
{
	if (cOther_.m_pPartitionBy) {
		m_pPartitionBy = _SYDNEY_DYNAMIC_CAST(ValueExpression*, cOther_.m_pPartitionBy->copy());
	}
}

// FUNCTION public
//	Statement::SortSpecificationList::~SortSpecificationList -- デストラクタ
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

SortSpecificationList::
~SortSpecificationList()
{
	if (m_pPartitionBy) {
		delete m_pPartitionBy, m_pPartitionBy = 0;
	}
}

//
//	FUNCTION public
//		Statement::SortSpecificationList::getSortSpecificationAt -- SortSpecification を得る
//
//	NOTES
//		SortSpecification を得る
//
//	ARGUMENTS
//		int iAt_
//
//	RETURN
//		SortSpecification*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
SortSpecification*
SortSpecificationList::getSortSpecificationAt(int iAt_) const
{
	return _SYDNEY_DYNAMIC_CAST(SortSpecification*, getAt(iAt_));
}

// FUNCTION public
//	Statement::SortSpecificationList::getPartitionBy -- PartitionByを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

ValueExpression*
SortSpecificationList::
getPartitionBy() const
{
	return m_pPartitionBy;
}

// FUNCTION public
//	Statement::SortSpecificationList::setPartitionBy -- PartitionByをセットする
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* pPartitionBy_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
SortSpecificationList::
setPartitionBy(ValueExpression* pPartitionBy_)
{
	m_pPartitionBy = pPartitionBy_;
}

//
//	FUNCTION public
//	Statement::SortSpecificationList::copy -- 自身をコピーする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Statement::Object*
//
//	EXCEPTIONS
//	なし
//
Object*
SortSpecificationList::copy() const
{
	return new SortSpecificationList(*this);
}

#if 0
namespace
{
	Analysis::SortSpecificationList _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
SortSpecificationList::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::SortSpecificationList::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchive_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SortSpecificationList::
serialize(ModArchive& cArchive_)
{
	ObjectList::serialize(cArchive_);
	Utility::Serialize::Object(cArchive_, m_pPartitionBy);
}

//
//	Copyright (c) 1999, 2002, 2003, 2005, 2006, 2008, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
