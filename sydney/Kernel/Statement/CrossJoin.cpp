// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CrossJoin.cpp -- CrossJoin
// 
// Copyright (c) 2004, 2006, 2011, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/CrossJoin.h"
#include "Statement/Type.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/CrossJoin.h"
#endif
#include "Analysis/Query/CrossJoin.h"

#include "ModOstrStream.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Left,
		f_Right,
		f__end_index
	};
}

//
//	FUNCTION public
//		Statement::CrossJoin::CrossJoin -- コンストラクタ (1)
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
CrossJoin::CrossJoin()
	: Object(ObjectType::CrossJoin, f__end_index)
{
}

//
//	FUNCTION public
//		Statement::CrossJoin::CrossJoin -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		Object* pLeft_
//		Object* pRight_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
CrossJoin::CrossJoin(Object* pLeft_, Object* pRight_)
	: Object(ObjectType::CrossJoin, f__end_index)
{
	// Left を設定する
	setLeft(pLeft_);
	// Right を設定する
	setRight(pRight_);
}

//
//	FUNCTION public
//		Statement::CrossJoin::getLeft -- Left を得る
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
CrossJoin::getLeft() const
{
	return m_vecpElements[f_Left];
}

//
//	FUNCTION public
//		Statement::CrossJoin::setLeft -- Left を設定する
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
CrossJoin::setLeft(Object* pLeft_)
{
	m_vecpElements[f_Left] = pLeft_;
}

//
//	FUNCTION public
//		Statement::CrossJoin::getRight -- Right を得る
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
CrossJoin::getRight() const
{
	return m_vecpElements[f_Right];
}

//
//	FUNCTION public
//		Statement::CrossJoin::setRight -- Right を設定する
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
CrossJoin::setRight(Object* pRight_)
{
	m_vecpElements[f_Right] = pRight_;
}

//
//	FUNCTION public
//	Statement::CrossJoin::copy -- 自身をコピーする
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
CrossJoin::copy() const
{
	return new CrossJoin(*this);
}

#ifdef USE_OLDER_VERSION
namespace
{
	Analysis::CrossJoin _analyzer;
}

// FUNCTION public
//	Statement::CrossJoin::getAnalyzer -- Analyzerを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Analysis::Analyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Analyzer*
CrossJoin::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::CrossJoin::getAnalyzer2 -- 
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
CrossJoin::
getAnalyzer2() const
{
	return Analysis::Query::CrossJoin::create(this);
}

//
//	Copyright (c) 2004, 2006, 2011, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
