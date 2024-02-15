// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ArrayIndex.h -- array索引オブジェクト関連のクラス定義、関数宣言
// 
// Copyright (c) 2007, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_ARRAYINDEX_H
#define	__SYDNEY_SCHEMA_ARRAYINDEX_H

#include "Schema/Module.h"
#include "Schema/Index.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

//	CLASS
//	Schema::ArrayIndex -- arrayインデックスオブジェクトを表すクラス
//
//	NOTES

class ArrayIndex
	: public	Index
{
public:

	// コンストラクター
	ArrayIndex();
	ArrayIndex(const Database& database, Table& table, const Name& cName_, Hint* pHint_);
	ArrayIndex(const Database& database, const LogData& cLogData_);

	// このクラスのクラス ID を得る
	virtual int				getClassID() const;

	// 索引ファイルを格納する エリアの種別を得る
	AreaCategory::Value		getAreaCategory() const;

protected:
//Index::
//	// 索引のキーオブジェクトを生成する
//	virtual void			createKey(Trans::Transaction& cTrans_,
//									  Table& cTable_,
//									  const Statement::IndexDefinition& cStatement_,
//									  Common::DataArrayData& cLogData_);

	// 索引を実現するファイルを生成し、既存のデータを反映する
	FilePointer				createFile(Trans::Transaction& cTrans_);

	// 索引を実現するファイルにフィールドを設定する
	void					createField(Trans::Transaction& cTrans_, File& cFile_,
										const Common::DataArrayData* pLogData_ = 0);

private:
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_ARRAYINDEX_H

//
// Copyright (c) 2007, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
