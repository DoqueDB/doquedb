// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::SelectSubList -- SelectSubList
// 
// Copyright (c) 1999, 2002, 2008, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_SELECTSUBLIST_H
#define __SYDNEY_STATEMENT_SELECTSUBLIST_H

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class DerivedColumn;
class Identifier;
	
//
//	CLASS
//		SelectSubList -- SelectSubList
//
//	NOTES
//		SelectSubList
//
class SYD_STATEMENT_FUNCTION SelectSubList : public Statement::Object
{
public:
	//constructor
	SelectSubList()
		: Object(ObjectType::SelectSubList)
	{}
	// コンストラクタ
	explicit SelectSubList(Statement::DerivedColumn* pDerivedColumn_);
	explicit SelectSubList(Statement::Identifier* pIdentifier_);

	// アクセサ
	// DerivedColumnOrIdentifier を得る
	Statement::Object* getDerivedColumnOrIdentifier() const;
	// DerivedColumnOrIdentifier を設定する
	void setDerivedColumnOrIdentifier(Statement::Object* pDerivedColumnOrIdentifier_);

	// ExpressionType を得る
	int getExpressionType() const;
	// ExpressionType を設定する
	void setExpressionType(int iType_);

	//自身をコピーする
	Object* copy() const;

	// Analyzerを得る
#ifdef USE_OLDER_VERSION
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
	virtual const Analysis::Interface::IAnalyzer*
						getAnalyzer2() const;

protected:
private:
	// 代入オペレーターは使わない
	SelectSubList& operator=(const SelectSubList& cOther_);

	// メンバ変数
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_SELECTSUBLIST_H

//
// Copyright (c) 1999, 2002, 2008, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
