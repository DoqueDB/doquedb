// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::SQLWrapper -- SQLWrapper
// 
// Copyright (c) 2001, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_SQLWRAPPER_H
#define __SYDNEY_STATEMENT_SQLWRAPPER_H

#include "Statement/Object.h"

_SYDNEY_BEGIN

namespace Statement
{
//
//	CLASS
//		SQLWrapper -- SQLWrapper
//
//	NOTES
//		SQLWrapper: parse前のSQL文の情報を保持するラッパー
//
class SYD_STATEMENT_FUNCTION SQLWrapper : public Statement::Object
{
public:
	// コンストラクタ (1)
	SQLWrapper();

	// アクセサ
	// Objectを渡す。戻り値は呼出側がdeleteすること。
	Object* getReleaseObject();

#ifndef SYD_COVERAGE
	// SQL文を得る
	const ModUnicodeString* getSQLString() const;
	// placeholderの範囲を得る。
	// placeholderが使われていなければ Lower == Upperとなる
	// placeholderの下限(最小番号)を得る
	int getPlaceHolderLower() const;
	// placeholderの上限(最大番号+1)を得る
	int getPlaceHolderUpper() const;
#endif

	// Objectを得る。戻り値をdeleteしてはいけない。
	Object* getObject() const;
	// オブジェクトを設定する
	void setObject(Object* pObject_);
#ifndef SYD_COVERAGE
	// SQL文を設定する
	void setSQLString(const ModUnicodeString& str_);
	// 下限を設定する
	void setPlaceHolderLower(int lower_);
	// 上限を設定する
	void setPlaceHolderUpper(int upper_);
#endif

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

protected:
private:
	// 代入オペレーターは使わない
	SQLWrapper& operator=(const SQLWrapper& cOther_);

	// メンバ変数
};

} // namescape Statement

_SYDNEY_END

#endif // __SYDNEY_STATEMENT_SQLWRAPPER_H

//
// Copyright (c) 2001, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
