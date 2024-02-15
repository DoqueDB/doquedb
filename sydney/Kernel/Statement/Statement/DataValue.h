// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::DataValue -- Common::Data値
// 
// Copyright (c) 1999, 2002, 2003, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_DATAVALUE_H
#define __SYDNEY_STATEMENT_DATAVALUE_H

#include "Statement/Object.h"
#include "Common/DataType.h"
#include "Common/Data.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

//	CLASS
//	DataValue -- Common::Data値
//
//	NOTES

class SYD_STATEMENT_FUNCTION DataValue : public Statement::Object
{
public:
	// コンストラクタ (1)
	DataValue();
#ifdef OBSOLETE
	// コンストラクタ (2)
	explicit DataValue(const Common::Data* pData_);
	// コンストラクタ (3)
	DataValue(Common::DataType::Type eType_,
			   const ModUnicodeString* pstrValue_);
	// コピーコンストラクタ
	DataValue(const DataValue& cOther_);
#endif
	// デストラクタ
	virtual ~DataValue();

#ifndef SYD_COVERAGE
	// 文字列化
	ModUnicodeString toString() const;
	// LISP形式で出力する
	void toString(ModUnicodeOstrStream& cStream_, int iIndent_ = 0) const;
#endif

	// 値を得る
	const Common::Data::Pointer& getValue() const;
	// 値を設定する
	void setValue(const Common::Data* pData_);
#ifdef OBSOLETE
	// 値を設定する
	void setValue(Common::DataType::Type eType_,
				  const ModUnicodeString* pstrValue_);
#endif

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);

private:
	// 代入オペレーターは使わない
	DataValue& operator=(const DataValue& cOther_);

	// メンバ変数
	mutable Common::Data::Pointer m_pData;
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_DATAVALUE_H

//
// Copyright (c) 1999, 2002, 2003, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
