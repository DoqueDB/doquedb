// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::CrossJoin -- CROSS JOIN
// 
// Copyright (c) 2004, 2011, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_CROSSJOIN_H
#define __SYDNEY_STATEMENT_CROSSJOIN_H

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

//
//	CLASS
//		CrossJoin --
//
//	NOTES
//
class SYD_STATEMENT_FUNCTION CrossJoin : public Statement::Object
{
public:
	// コンストラクタ (1)
	CrossJoin();
	// コンストラクタ (2)
	CrossJoin(Object* pLeft_, Object* pRight_);

	// Left を得る
	Object* getLeft() const;
	// Left を設定する
	void setLeft(Object* pLeft_);

	// Right を得る
	Object* getRight() const;
	// Right を設定する
	void setRight(Object* pRight_);

	//自身をコピーする
	Object* copy() const;

	// Analyzerを得る
#ifdef USE_OLDER_VERSION
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

private:
	// 代入オペレーターは使わない
	CrossJoin& operator=(const CrossJoin& cOther_);
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_CROSSJOIN_H

//
// Copyright (c) 2004, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
