// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PartitionDefinition.h --
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

#ifndef __SYDNEY_STATEMENT_PARTITIONDEFINITION_H
#define __SYDNEY_STATEMENT_PARTITIONDEFINITION_H

#include "Common/Common.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class Identifier;
class ColumnNameList;

//
//	CLASS
//	Statement::PartitionDefinition --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION PartitionDefinition  : public Statement::Object
{
public:
	struct Category
	{
		enum Value
		{
			ReadOnly = 0,
			Normal,
			ValueNum
		};
	};

	//constructor
	PartitionDefinition()
		: Object(ObjectType::PartitionDefinition)
	{}
	//コンストラクタ
	PartitionDefinition(Identifier* pId_,
						Identifier* pFunction_,
						ColumnNameList* pColumnList_,
						Category::Value eCategory_);
	//デストラクタ
	virtual ~PartitionDefinition();

	// アクセサ

	//対象の表名を得る
	Identifier* getTableName() const;
	//対象の表名設定
	void setTableName(Identifier* pId_);

	// 関数名を得る
	Identifier*	getFunction() const;
	// 関数名を設定する
	void setFunction(Identifier* pFunction_);

	// 列リストを得る
	ColumnNameList*	getColumnList() const;
	// 列リストを設定する
	void setColumnList(ColumnNameList* pColumnList_);

	// カテゴリーを得る
	Category::Value	getCategory() const;
	// カテゴリーを設定する
	void setCategory(Category::Value eCategory_);

	//自身をコピーする
	Object* copy() const;

protected:
private:
	// 代入オペレーターは使わない
	PartitionDefinition& operator=(const PartitionDefinition& cOther_);
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_PARTITIONDEFINITION_H

//
//	Copyright (c) 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
