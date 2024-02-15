// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DropCascadeStatement.h --
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_DROPCASCADESTATEMENT_H
#define __SYDNEY_STATEMENT_DROPCASCADESTATEMENT_H

#include "Common/Common.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class Identifier;

//
//	CLASS
//	Statement::DropCascadeStatement --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION DropCascadeStatement  : public Statement::Object
{
public:
	//constructor
	DropCascadeStatement()
		: Object(ObjectType::DropCascadeStatement)
	{}
	//コンストラクタ
	DropCascadeStatement(Identifier* pId_, bool bIfExists_);
	//デストラクタ
	virtual ~DropCascadeStatement();

	// アクセサ

	//子サーバー名を得る
	Identifier* getCascadeName() const;
	//子サーバー名設定
	void setCascadeName(Identifier* pId_);

	bool isIfExists() const;

	//自身をコピーする
	Object* copy() const;

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);

protected:
private:
	// 代入オペレーターは使わない
	DropCascadeStatement& operator=(const DropCascadeStatement& cOther_);

	bool m_bIfExists;
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_DROPCASCADESTATEMENT_H

//
//	Copyright (c) 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
