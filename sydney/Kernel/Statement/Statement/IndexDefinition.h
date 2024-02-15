// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IndexDefinition.h --
// 
// Copyright (c) 1999, 2002, 2003, 2005, 2006, 2007, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_INDEXDEFINITION_H
#define __SYDNEY_STATEMENT_INDEXDEFINITION_H

#include "Statement/Module.h"
#include "Statement/Object.h"

class ModUnicodeString;

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class ColumnName;
class ColumnNameList;
class Hint;
class Identifier;
class AreaOption;
	
//
//	CLASS
//		IndexDefinition -- IndexDefinition
//
//	NOTES
//		IndexDefinition
//
class SYD_STATEMENT_FUNCTION IndexDefinition : public Statement::Object
{
public:
	//constructor
	IndexDefinition()
		: Object(ObjectType::IndexDefinition)
	{}
	// コンストラクタ
	IndexDefinition(Identifier* pName_, Identifier* pTableName_,
					ColumnNameList* pColumnNameList_, int iIndexType_,
					ColumnName* languageColumnName,
					ColumnName* scoreColumnName,
					Hint* pHint_, AreaOption* pAreaOpt_);

	// アクセサ
	// Name を得る
	Identifier* getName() const;
	// Name を設定する
	void setName(Identifier* pName_);
#ifdef OBSOLETE
	// Name を ModUnicodeString で得る
	const ModUnicodeString* getNameString() const;
#endif
	// TableName を得る
	Identifier* getTableName() const;
	// TableName を設定する
	void setTableName(Identifier* pTableName_);
#ifdef OBSOLETE
	// TableName を ModUnicodeString で得る
	const ModUnicodeString* getTableNameString() const;
#endif

	// ColumnNameList を得る
	ColumnNameList* getColumnNameList() const;
	// ColumnNameList を設定する
	void setColumnNameList(ColumnNameList* pColumnNameList_);

	// IndexType を得る
	int getIndexType() const;
	// IndexType を設定する
	void setIndexType(int iIndexType_);
	//
	//  Enum global
	//  IndexType -- IndexTypeの値
	//
	//  NOTES
	//  IndexTypeの値
	//
	enum IndexType {
		None = 0,
		Clustered,
		NonClustered,
		Inverted,
		FullText,
		Bitmap,
		Unique,
		Array,
		AllRows,
		KdTree
	};

	// 言語指定を得るための列の名前を得る
	ColumnName* getLanguageColumnName() const;
	// 言語指定を得るための列の名前を設定する
	void setLanguageColumnName(ColumnName* columnName);

	// スコア指定を得るための列の名前を得る
	ColumnName* getScoreColumnName() const;
	// スコア指定を得るための列の名前を設定する
	void setScoreColumnName(ColumnName* columnName);

	// Hint を得る
	Hint* getHint() const;
	// Hint を設定する
	void setHint(Hint* pHint_);

	// AreaOption を得る
	AreaOption* getAreaOption() const;
	// AreaOption を設定する
	void setAreaOption(AreaOption* pAreaOpt_);

	//自身をコピーする
	Object* copy() const;

	// SQL文で値を得る
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

private:
	// 代入オペレーターは使わない
	IndexDefinition& operator=(const IndexDefinition& cOther_);

	// メンバ変数
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_INDEXDEFINITION_H

//
// Copyright (c) 1999, 2002, 2003, 2005, 2006, 2007, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
