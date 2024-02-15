// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::ObjectList -- ObjectList
// 
// Copyright (c) 1999, 2002, 2004, 2005, 2010, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_OBJECTLIST_H
#define __SYDNEY_STATEMENT_OBJECTLIST_H

#include "Common/Common.h"
#include "Common/Assert.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN

namespace Statement
{

	class Object;
	class SQLParser;
	
//
//	CLASS
//		ObjectList -- 繰り返しの Node
//
//	NOTES
//		ObjectList
//
class SYD_STATEMENT_FUNCTION ObjectList : public Statement::Object
{
public:
	// アクセサ
	// 要素の個数を得る
	int getCount() const;
	// オブジェクト要素を得る
	Object* getAt(int iAt_) const;
	// オブジェクト要素を設定する
	void setAt(int iAt_, Object* pObject_);
	// オブジェクト要素を加える
	void append(Object* pObject_);
	// オブジェクト要素を先頭に加える
	void insert(Object* pObject_);
	// オブジェクトリストを追加する
	void merge(ObjectList& cList_);

	// SQL文で値を得る
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

protected:
	// コンストラクタ
	//(ObjectListクラスはオブジェクト化しない)
	ObjectList(ObjectType::Type eType_);

private:
	// 代入オペレーターは使わない
	ObjectList& operator=(const ObjectList& cOther_);

	// メンバ変数
};

} // namescape Statement

_SYDNEY_END

#endif // __SYDNEY_STATEMENT_TABLEREFERENCELIST_H

//
// Copyright (c) 1999, 2002, 2004, 2005, 2010, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
