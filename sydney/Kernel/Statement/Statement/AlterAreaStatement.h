// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AlterAreaStatement.h --
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

#ifndef __SYDNEY_STATEMENT_ALTERAREASTATEMENT_H
#define __SYDNEY_STATEMENT_ALTERAREASTATEMENT_H

#include "Common/Common.h"
#include "Statement/Object.h"


_SYDNEY_BEGIN

namespace Statement
{

class AlterAreaAction;
class Identifier;

//
//	CLASS
//	Statement::AlterAreaStatement --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION AlterAreaStatement  : public Statement::Object
{
public:
	//constructor
	AlterAreaStatement()
		: Object(ObjectType::AlterAreaStatement)
	{}
	//コンストラクタ(2)
	AlterAreaStatement(Identifier* pcIdent_, AlterAreaAction* pcAction_);
	//デストラクタ
	virtual ~AlterAreaStatement();

	//
	//	アクセサ
	//

	//エリア名アクセサ
	Identifier* getAreaName() const;
	void setAreaName(Identifier* pcIdent_);

	//AlterAreaAction アクセサ
	AlterAreaAction* getAlterAreaAction() const;
	void setAlterAreaAction(AlterAreaAction* pcAction_);

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

protected:

private:
	//代入オペレータは使わない
	AlterAreaStatement& operator= (const AlterAreaStatement& cOther_);

};

} // namespace Statement

_SYDNEY_END

#endif //__SYDNEY_STATEMENT_ALTERAREASTATEMENT_H

//
//	Copyright (c) 2000, 2002, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
