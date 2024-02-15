// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::TableCorrelationSpec -- TableCorrelationSpec
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

#ifndef __SYDNEY_STATEMENT_TABLECORRELATIONSPEC_H
#define __SYDNEY_STATEMENT_TABLECORRELATIONSPEC_H

#include "Statement/Object.h"

class ModUnicodeString;

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

	class ColumnNameList;
	class Identifier;
	
//	CLASS
//	TableCorrelationSpec -- TableCorrelationSpec
//
//	NOTES

class SYD_STATEMENT_FUNCTION TableCorrelationSpec : public Statement::Object
{
public:
	//constructor
	TableCorrelationSpec()
		: Object(ObjectType::TableCorrelationSpec)
	{}
	// コンストラクタ (2)
	TableCorrelationSpec(Identifier* pCorrelation_, ColumnNameList* pDerived_);

	// アクセサ
	// Correlation を得る
	Identifier* getCorrelation() const;
	// Correlation を設定する
	void setCorrelation(Identifier* pCorrelation_);
#ifdef OBSOLETE
	// Correlation を ModUnicodeString で得る
	const ModUnicodeString* getCorrelationString() const;
#endif

	// Derived を得る
	ColumnNameList* getDerived() const;
	// Derived を設定する
	void setDerived(ColumnNameList* pDerived_);

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

private:
	// 代入オペレーターは使わない
	TableCorrelationSpec& operator=(const TableCorrelationSpec& cOther_);
};

//	FUNCTION public
//	Statement::TableCorrelationSpec::copy -- 自身をコピーする
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

inline
Object*
TableCorrelationSpec::copy() const
{
	return new TableCorrelationSpec(*this);
}

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_TABLECORRELATIONSPEC_H

//
// Copyright (c) 1999, 2002, 2003, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
