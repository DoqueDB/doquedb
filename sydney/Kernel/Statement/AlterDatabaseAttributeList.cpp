// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AlterDatabaseAttributeList.cpp -- AlterDatabaseAttributeList
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

#include "Common/Assert.h"
#include "Statement/SQLParser.h"
#include "Statement/Type.h"
#include "Statement/IntegerValue.h"
#include "Statement/AlterDatabaseAttribute.h"
#include "Statement/AlterDatabaseAttributeList.h"

#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"
#include "ModUnicodeOstrStream.h"

#include "Exception/NotSupported.h"
#if 0
#include "Analysis/AlterDatabaseAttributeList.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

//
//	FUNCTION public
//		Statement::AlterDatabaseAttributeList::AlterDatabaseAttributeList -- コンストラクタ (2)
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
AlterDatabaseAttributeList::AlterDatabaseAttributeList(AlterDatabaseAttribute* pDatabaseAttribute_)
	: ObjectList(ObjectType::AlterDatabaseAttributeList)
{
	// DatabaseAttribute を加える
	append(pDatabaseAttribute_);
}

//
//	FUNCTION public
//		Statement::AlterDatabaseAttributeList::getDatabaseAttributeAt -- AlterDatabaseAttribute を得る
//
//	NOTES
//
//	ARGUMENTS
//		int iAt_
//
//	RETURN
//		AlterDatabaseAttribute*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//

AlterDatabaseAttribute*
AlterDatabaseAttributeList::
getDatabaseAttributeAt(int iAt_) const
{
	return _SYDNEY_DYNAMIC_CAST(Statement::AlterDatabaseAttribute*,
								getAt(iAt_));
}

//
//	FUNCTION public
//		Statement::AlterDatabaseAttributeList::assureElements -- 整合性を確認する。
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
AlterDatabaseAttributeList::assureElements(SQLParser& cParser_)
{
	bool pArray[AlterDatabaseAttribute::NumOfAttributeType];
	//検査用配列の初期化
	for ( int i = 0 ; i < sizeof(pArray)/sizeof(pArray[0]) ; ++i ) {
		pArray[i] = false;
	}
	//重複検査メイン
	AlterDatabaseAttribute* pObj = 0;
	for ( int j = getCount() ; 0 < j ; --j ) {

		pObj = getDatabaseAttributeAt(j-1);

		if (!pObj) {
			cParser_.throwSyntaxErrorException(srcFile ,__LINE__);
			return;//不整合：不正な要素
		}
		int id = pObj->getAttributeType();
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
//	Statement::AlterDatabaseAttributeList::copy -- 自身をコピーする
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
AlterDatabaseAttributeList::copy() const
{
	return new AlterDatabaseAttributeList(*this);
}

//
//	FUNCTION public
//	Statement::AlterDatabaseAttributeList::toSQLStatement -- SQL文で得る
//
//	NOTES
//
//	ARGUMENTS
//	bool bForCascade_ = false
//
//	RETURN
//	ModUnicodeString
//		文字列表記
//
//	EXCEPTIONS
//
ModUnicodeString
AlterDatabaseAttributeList::toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream s;
	int n = getCount();

	for (int i = 0; i < n; ++i)
	{
		if (i != 0) s << " ";
		
		AlterDatabaseAttribute* a = getDatabaseAttributeAt(i);
		if (a)
			s << a->toSQLStatement(bForCascade_);
		else
			s << "(null)";
	}

	return ModUnicodeString(s.getString());
}

#if 0
namespace
{
	Analysis::AlterDatabaseAttributeList _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
AlterDatabaseAttributeList::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 1999, 2002, 2005, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

