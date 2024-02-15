// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CheckpointStatement.h --
// 
// Copyright (c) 2005, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_CHECKPOINTSTATEMENT_H
#define __SYDNEY_STATEMENT_CHECKPOINTSTATEMENT_H

#include "Common/Common.h"
#include "Statement/Object.h"
#include "Common/Data.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class Literal;

//
//	CLASS
//	Statement::CheckpointStatement --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION CheckpointStatement  : public Statement::Object
{
public:
	//コンストラクタ
	CheckpointStatement();
	//デストラクタ
	virtual ~CheckpointStatement();

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

	// カウントを得る
	int getCount() const { return m_iCount; }

	// カウントを設定する
	void setCount(const Literal& cLiteral_);

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);

private:
	// 代入オペレーターは使わない
	CheckpointStatement& operator=(const CheckpointStatement& cOther_);

	// カウント
	int m_iCount;
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_CHECKPOINTSTATEMENT_H

//
//	Copyright (c) 2005, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
