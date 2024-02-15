// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaDataDefinition.h --
// 
// Copyright (c) 2000, 2002, 2003, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_AREADATADEFINITION_H
#define __SYDNEY_STATEMENT_AREADATADEFINITION_H

#include "Common/Common.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

	class Literal;
	class Hint;

//	CLASS
//	Statement::AreaDataDefinition --
//
//	NOTES

class SYD_STATEMENT_FUNCTION AreaDataDefinition  : public Statement::Object
{
public:
	//constructor
	AreaDataDefinition()
		: Object(ObjectType::AreaDataDefinition)
	{}
	//コンストラクタ(2)
	AreaDataDefinition(Literal* pPath_, Hint* pHint_);

	//デストラクタ
	virtual ~AreaDataDefinition();
	// Area データを取得する
	Literal* getAreaData() const;
	// Area データを設定する
	void setAreaData(Literal* pPath_);
#ifdef OBSOLETE
	// Hint を取得する
	Hint* getHint() const;
#endif
	// Hint を設定する
	void setHint(Hint* pHint_);

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

private:
	// 代入オペレーターは使わない
	AreaDataDefinition& operator=(const AreaDataDefinition& cOther_);
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_AREADATADEFINITION_H

//
//	Copyright (c) 2000, 2002, 2003, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
