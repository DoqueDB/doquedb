// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExistsJoin.cpp -- ExistsJoin
// 
// Copyright (c) 2004, 2006, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/ExistsJoin.h"
#include "Statement/Type.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/ExistsJoin.h"
#endif
#include "Analysis/Query/ExistsJoin.h"

#include "ModOstrStream.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Left,
		f_Right,
		f_Condition,
		f__end_index
	};
}

//
//	FUNCTION public
//		Statement::ExistsJoin::ExistsJoin -- コンストラクタ (1)
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
ExistsJoin::ExistsJoin()
	: Object(ObjectType::ExistsJoin, f__end_index),
	  m_bFlag(true)
{
}

//
//	FUNCTION public
//		Statement::ExistsJoin::ExistsJoin -- コンストラクタ (2)
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
ExistsJoin::ExistsJoin(Object* pLeft_, Object* pRight_, Object* pCondition_, bool bFlag_)
	: Object(ObjectType::ExistsJoin, f__end_index),
	  m_bFlag(bFlag_)
{
	// Left を設定する
	setLeft(pLeft_);
	// Right を設定する
	setRight(pRight_);
	// Condition を設定する
	setCondition(pCondition_);
}

// コピーコンストラクタ
ExistsJoin::
ExistsJoin(const ExistsJoin& cOther_)
	: Object(cOther_),
	  m_bFlag(cOther_.m_bFlag)
{}

//
//	FUNCTION public
//		Statement::ExistsJoin::getLeft -- Left を得る
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
ExistsJoin::getLeft() const
{
	return m_vecpElements[f_Left];
}

//
//	FUNCTION public
//		Statement::ExistsJoin::setLeft -- Left を設定する
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
ExistsJoin::setLeft(Object* pLeft_)
{
	m_vecpElements[f_Left] = pLeft_;
}

//
//	FUNCTION public
//		Statement::ExistsJoin::getRight -- Right を得る
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
ExistsJoin::getRight() const
{
	return m_vecpElements[f_Right];
}

//
//	FUNCTION public
//		Statement::ExistsJoin::setRight -- Right を設定する
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
ExistsJoin::setRight(Object* pRight_)
{
	m_vecpElements[f_Right] = pRight_;
}

//
//	FUNCTION public
//		Statement::ExistsJoin::getCondition -- Condition を得る
//
//	NOTES
//		Condition を得る
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
ExistsJoin::getCondition() const
{
	return m_vecpElements[f_Condition];
}

//
//	FUNCTION public
//		Statement::ExistsJoin::setCondition -- Condition を設定する
//
//	NOTES
//		Condition を設定する
//
//	ARGUMENTS
//		Object* pCondition_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ExistsJoin::setCondition(Object* pCondition_)
{
	m_vecpElements[f_Condition] = pCondition_;
}

//
//	FUNCTION public
//	Statement::ExistsJoin::copy -- 自身をコピーする
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
ExistsJoin::copy() const
{
	return new ExistsJoin(*this);
}

#ifdef OBSOLETE // 現在のところStatement::Objectをマップに登録することはない
// ハッシュコードを計算する
//virtual
ModSize
ExistsJoin::
getHashCode()
{
	ModSize value = Super::getHashCode();
	value <<= 1;
	value += (m_bFlag ? 1 : 0);
	return value;
}

// 同じ型のオブジェクト同士でless比較する
//virtual
bool
ExistsJoin::
compare(const Object& cObj_) const
{
#define _FlagToInt(_b_) ((_b_) ? 1 : 0)
	return Super::compare(cObj_)
		|| _FlagToInt(m_bFlag) < _FlagToInt(_SYDNEY_DYNAMIC_CAST(const ExistsJoin&, cObj_).m_bFlag);
#undef _FlagToInt
}
#endif

#ifdef USE_OLDER_VERSION
namespace
{
	Analysis::ExistsJoin _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
ExistsJoin::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::ExistsJoin::getAnalyzer2 -- 
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
ExistsJoin::
getAnalyzer2() const
{
	return Analysis::Query::ExistsJoin::create(this);
}

// FUNCTION public
//	Statement::ExistsJoin::serialize -- 
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
ExistsJoin::
serialize(ModArchive& cArchive_)
{
	Object::serialize(cArchive_);
	cArchive_(m_bFlag);
}

//
//	Copyright (c) 2004, 2006, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
