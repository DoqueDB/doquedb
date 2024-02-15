// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DatabaseCreateOptionList.cpp -- DatabaseCreateOptionList
// 
// Copyright (c) 1999, 2002, 2005, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/SQLParser.h"
#include "Statement/Type.h"
#include "Statement/DatabaseCreateOption.h"
#include "Statement/DatabaseCreateOptionList.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModUnicodeOstrStream.h"
#if 0
#include "Analysis/DatabaseCreateOptionList.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

//
//	FUNCTION public
//		Statement::DatabaseCreateOptionList::DatabaseCreateOptionList -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		Object* pObject_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
DatabaseCreateOptionList::DatabaseCreateOptionList(DatabaseCreateOption* pCreateOption_)
	: ObjectList(ObjectType::DatabaseCreateOptionList)
{
	// CreateOption を加える
	append(pCreateOption_);
}

//
//	FUNCTION public
//		Statement::DatabaseCreateOptionList::getDatabaseCreateOption -- CreateOptionを得る
//
//	NOTES
//
//	ARGUMENTS
//		int iAt_
//
//	RETURN
//		DatabaseCreateOption*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//

DatabaseCreateOption*
DatabaseCreateOptionList::
getCreateOptionAt(int iAt_) const
{
	return _SYDNEY_DYNAMIC_CAST(DatabaseCreateOption*, getAt(iAt_));
}

//
//	FUNCTION public
//		Statement::DatabaseCreateOptionList::assureElements -- 整合性を確認する。
//
//	NOTES
//		要素間の整合性を確認する
//		親クラス(ObjectList) の virtual メソッドのオーバーライド
//
//	ARGUMENTS
//		SQLParser& cParser_
//
//	RETURN
//		void
//			不整合なら例外発生
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DatabaseCreateOptionList::assureElements(SQLParser& cParser_)
{
	bool pArray[DatabaseCreateOption::NumOfOptionType];
	//検査用配列の初期化
	for ( int i = 0 ; i < sizeof(pArray)/sizeof(pArray[0]) ; ++i ) {
		pArray[i] = false;
	}
	//重複検査メイン
	for ( int j = getCount() ; 0 < j ; --j ) {

		if ( ObjectType::DatabaseCreateOption != getCreateOptionAt(j-1)->getType() ) {
			cParser_.throwSyntaxErrorException(srcFile ,__LINE__);
			return;//不整合：不正な要素
		}

		DatabaseCreateOption* pObj
			= _SYDNEY_DYNAMIC_CAST(DatabaseCreateOption*, getCreateOptionAt(j-1));
		; _SYDNEY_ASSERT(pObj);

		int id = pObj->getOptionType();
		if ( sizeof(pArray)/sizeof(pArray[0]) <= id ) {
			cParser_.throwSyntaxErrorException(srcFile ,__LINE__);
			return;//不整合：type範囲外
		}
		if ( pArray[id] ) {
			cParser_.throwSyntaxErrorException(srcFile ,__LINE__);
			return;//不整合：要素の重複あり
		}
		pArray[id] = true;
	}
	//整合
}

//
//	FUNCTION public
//	Statement::DatabaseCreateOptionList::copy -- 自身をコピーする
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
DatabaseCreateOptionList::copy() const
{
	return new DatabaseCreateOptionList(*this);
}

//
//	FUNCTION public
//	Statement::DatabaseCreateOptionList::toSQLStatement -- SQL文で得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		文字列表記
//
//	EXCEPTIONS
//
ModUnicodeString
DatabaseCreateOptionList::toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream s;

	int n = getCount();
	for (int i = 0; i < n; ++i)
	{
		if (i != 0)
			s << " ";
		
		DatabaseCreateOption* e = getCreateOptionAt(i);
		if (e == 0)
		{
			s << "null";
		}
		else
		{
			s << e->toSQLStatement(bForCascade_);
		}
	}

	return ModUnicodeString(s.getString());
}

#if 0
namespace
{
	Analysis::DatabaseCreateOptionList _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
DatabaseCreateOptionList::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 1999, 2002, 2005, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

