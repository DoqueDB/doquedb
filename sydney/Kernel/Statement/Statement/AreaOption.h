// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaOption.h --
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

#ifndef __SYDNEY_STATEMENT_AREAOPTION_H
#define __SYDNEY_STATEMENT_AREAOPTION_H

#include "Common/Common.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class Identifier;
class Hint;

//	CLASS
//	Statement::AreaOption --
//
//	NOTES

class SYD_STATEMENT_FUNCTION AreaOption  : public Statement::Object
{
public:
	//constructor
	AreaOption()
		: Object(ObjectType::AreaOption)
	{}
	//コンストラクタ(2)
	AreaOption(Identifier* Default_, Identifier* Table_,
			   Identifier* Heap_, Identifier* Index_,
			   Identifier* FullText_, Identifier* LogicalLog_,
			   Identifier* PhysicalLog_, Hint* HintArea_);
	//デストラクタ
	virtual ~AreaOption();

	//エリアタイプ
	enum AreaType
	{
		Default = 0,
		Table,
		Heap,
		Index,
		FullText,
		LogicalLog,
		PhysicalLog,
		HintArea,
        ValueNum
	};

	//--- アクセサ ---
	// エリア名取得
	Identifier* getAreaName(const AreaType& eAreaType_) const;
	// エリア名設定
	void setAreaName(const AreaType& eAreaType_, Identifier* pId_);

#ifdef OBSOLETE
	// Hint を得る
	Hint* getHintArea() const;
#endif
	// Hint を取得する
	void setHintArea(Hint* pHint_);

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

private:
	// 代入オペレーターは使わない
	AreaOption& operator=(const AreaOption& cOther_);
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_AREAOPTION_H

//
//	Copyright (c) 2000, 2002, 2003, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
