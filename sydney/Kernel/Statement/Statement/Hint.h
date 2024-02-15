// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::Hint -- Hint
// 
// Copyright (c) 1999, 2002, 2003, 2007, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_HINT_H
#define __SYDNEY_STATEMENT_HINT_H

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

	class HintElement;
	
//
//	CLASS
//		Hint -- Hint
//
//	NOTES
//		Hint
//
class SYD_STATEMENT_FUNCTION Hint : public Statement::Object
{
public:
	//constructor
	Hint()
		: Object(ObjectType::Hint)
	{}
	// コンストラクタ (2)
	explicit Hint(HintElement* pHintElement_);

	// アクセサ
	// HintElement を得る
	HintElement* getHintElementAt(int iAt_) const;
#ifdef OBSOLETE
	// HintElement を設定する
	void setHintElementAt(int iAt_, HintElement* pHintElement_);
#endif
	// HintElement の個数を得る
	int getHintElementCount() const;
	// HintElement を加える
	void append(HintElement* pHintElement_);

	//自身をコピーする
	Object* copy() const;

	// get SQL statement
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

private:
	// 代入オペレーターは使わない
	Hint& operator=(const Hint& cOther_);

	// メンバ変数
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_HINT_H

//
// Copyright (c) 1999, 2002, 2003, 2007, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
