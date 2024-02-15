// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AlterPartitionStatement.h --
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_ALTERPARTITIONSTATEMENT_H
#define __SYDNEY_STATEMENT_ALTERPARTITIONSTATEMENT_H

#include "Common/Common.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class Identifier;
class ColumnNameList;

//
//	CLASS
//	Statement::AlterPartitionStatement --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION AlterPartitionStatement  : public Statement::Object
{
public:
	//constructor
	AlterPartitionStatement()
		: Object(ObjectType::AlterPartitionStatement)
	{}
	//コンストラクタ
	AlterPartitionStatement(Identifier* pId_, Identifier* pFunction_, ColumnNameList* pColumnList_);
	//デストラクタ
	virtual ~AlterPartitionStatement();

	// アクセサ

	//ルール名を得る
	Identifier* getPartitionName() const;
	//ルール名設定
	void setPartitionName(Identifier* pId_);

	// 関数名を得る
	Identifier*	getFunction() const;
	// 関数名を設定する
	void setFunction(Identifier* pFunction_);

	// 列リスト番号を得る
	ColumnNameList*	getColumnList() const;
	// 列リスト番号を設定する
	void setColumnList(ColumnNameList* pColumnList_);

	//自身をコピーする
	Object* copy() const;

protected:
private:
	// 代入オペレーターは使わない
	AlterPartitionStatement& operator=(const AlterPartitionStatement& cOther_);
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_ALTERPARTITIONSTATEMENT_H

//
//	Copyright (c) 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
