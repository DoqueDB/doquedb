// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AlterIndexStatement.h --
// 
// Copyright (c) 2000, 2002, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_ALTERINDEXSTATEMENT_H
#define __SYDNEY_STATEMENT_ALTERINDEXSTATEMENT_H

#include "Common/Common.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN

namespace Statement
{

class Identifier;
class AlterIndexAction;

//
//	CLASS
//	Statement::AlterIndexStatement --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION AlterIndexStatement	: public Statement::Object
{
public:
	//constructor
	AlterIndexStatement()
		: Object(ObjectType::AlterIndexStatement)
	{}
	//コンストラクタ(2)
	AlterIndexStatement(Identifier* pcIdent_, AlterIndexAction* pcAction_);
	//デストラクタ
	virtual ~AlterIndexStatement();

	//
	//	アクセサ
	//

	//エリア名アクセサ
	Identifier* getIndexName() const;
	void setIndexName(Identifier* pcIdent_);

	//AlterIndexAction アクセサ
	AlterIndexAction* getAlterIndexAction() const;
	void setAlterIndexAction(AlterIndexAction* pcAction_);

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

protected:

private:
	//代入オペレータは使用しない
	AlterIndexStatement& operator= (const AlterIndexStatement& cOther_);

};

} // namespace Statement

_SYDNEY_END

#endif //__SYDNEY_STATEMENT_ALTERINDEXSTATEMENT_H

//
//	Copyright (c) 2000, 2002, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
