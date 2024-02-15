// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// QualifiedJoin.cpp -- QualifiedJoin
// 
// Copyright (c) 2004, 2006, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/QualifiedJoin.h"
#include "Statement/Type.h"
#include "Statement/Utility.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/QualifiedJoin_ColumnList.h"
#include "Analysis/QualifiedJoin_Condition.h"
#include "Analysis/QualifiedJoin_Natural.h"
#endif

#include "Analysis/Query/QualifiedJoin.h"

#include "ModOstrStream.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Left,
		f_Right,
		f_JoinSpec,
		f__end_index
	};
}

//
//	FUNCTION public
//		Statement::QualifiedJoin::QualifiedJoin -- コンストラクタ (1)
//
//	NOTES
//		コンストラクタ (1)
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//
QualifiedJoin::QualifiedJoin()
	: Object(ObjectType::QualifiedJoin, f__end_index),
	  m_eType(JoinType::Inner)
{
}

//
//	FUNCTION public
//		Statement::QualifiedJoin::QualifiedJoin -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		Object* pLeft_
//		Object* pRight_
//		Object* pJoinSpec_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
QualifiedJoin::QualifiedJoin(JoinType::Value eType_, Object* pLeft_, Object* pRight_, Object* pJoinSpec_)
	: Object(ObjectType::QualifiedJoin, f__end_index),
	  m_eType(eType_)
{
	// Left を設定する
	setLeft(pLeft_);
	// Right を設定する
	setRight(pRight_);
	// JoinSpec を設定する
	setJoinSpec(pJoinSpec_);
}

// コピーコンストラクタ
QualifiedJoin::
QualifiedJoin(const QualifiedJoin& cOther_)
	: Object(cOther_),
	  m_eType(cOther_.m_eType)
{}

//
//	FUNCTION public
//		Statement::QualifiedJoin::getLeft -- Left を得る
//
//	NOTES
//		Left を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Object*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Object*
QualifiedJoin::getLeft() const
{
	return m_vecpElements[f_Left];
}

//
//	FUNCTION public
//		Statement::QualifiedJoin::setLeft -- Left を設定する
//
//	NOTES
//		Left を設定する
//
//	ARGUMENTS
//		Object* pLeft_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
QualifiedJoin::setLeft(Object* pLeft_)
{
	m_vecpElements[f_Left] = pLeft_;
}

//
//	FUNCTION public
//		Statement::QualifiedJoin::getRight -- Right を得る
//
//	NOTES
//		Right を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Object*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Object*
QualifiedJoin::getRight() const
{
	return m_vecpElements[f_Right];
}

//
//	FUNCTION public
//		Statement::QualifiedJoin::setRight -- Right を設定する
//
//	NOTES
//		Right を設定する
//
//	ARGUMENTS
//		Object* pRight_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
QualifiedJoin::setRight(Object* pRight_)
{
	m_vecpElements[f_Right] = pRight_;
}

//
//	FUNCTION public
//		Statement::QualifiedJoin::getJoinSpec -- JoinSpec を得る
//
//	NOTES
//		JoinSpec を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Object*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Object*
QualifiedJoin::getJoinSpec() const
{
	return m_vecpElements[f_JoinSpec];
}

//
//	FUNCTION public
//		Statement::QualifiedJoin::setJoinSpec -- JoinSpec を設定する
//
//	NOTES
//		JoinSpec を設定する
//
//	ARGUMENTS
//		Object* pJoinSpec_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
QualifiedJoin::setJoinSpec(Object* pJoinSpec_)
{
	m_vecpElements[f_JoinSpec] = pJoinSpec_;
}

//
//	FUNCTION public
//	Statement::QualifiedJoin::copy -- 自身をコピーする
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
QualifiedJoin::copy() const
{
	QualifiedJoin* pResult = new QualifiedJoin(*this);
	pResult->setJoinType(getJoinType());
	return pResult;
}

#ifdef USE_OLDER_VERSION
namespace
{
	Analysis::QualifiedJoin_ColumnList _analyzerColumn;
	Analysis::QualifiedJoin_Condition _analyzerCondition;
	Analysis::QualifiedJoin_Natural _analyzerNatural;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
QualifiedJoin::
getAnalyzer() const
{
	if (!getJoinSpec())
		return &_analyzerNatural;

	if (getJoinSpec()->getType() == ObjectType::ColumnNameList)
		return &_analyzerColumn;

	return &_analyzerCondition;
}
#endif

// FUNCTION public
//	Statement::QualifiedJoin::getAnalyzer2 -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Analysis::Interface::IAnalyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Interface::IAnalyzer*
QualifiedJoin::
getAnalyzer2() const
{
	return Analysis::Query::QualifiedJoin::create(this);
}

// FUNCTION public
//	Statement::QualifiedJoin::serialize -- 
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
QualifiedJoin::
serialize(ModArchive& cArchive_)
{
	Object::serialize(cArchive_);
	Utility::Serialize::EnumValue(cArchive_, m_eType);
}

//
//	Copyright (c) 2004, 2006, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
