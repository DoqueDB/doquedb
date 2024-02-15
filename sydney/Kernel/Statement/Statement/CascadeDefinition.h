// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CascadeDefinition.h --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_CASCADEDEFINITION_H
#define __SYDNEY_STATEMENT_CASCADEDEFINITION_H

#include "Common/Common.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class Identifier;
class ValueExpression;

//
//	CLASS
//	Statement::CascadeDefinition --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION CascadeDefinition  : public Statement::Object
{
public:
	//constructor
	CascadeDefinition()
		: Object(ObjectType::CascadeDefinition)
	{}
	//コンストラクタ
	CascadeDefinition(Identifier* pId_,
					  ValueExpression* pHost_,
					  ValueExpression* pPont_,
					  Identifier* pDatabase_);
	//デストラクタ
	virtual ~CascadeDefinition();

	// アクセサ

	//子サーバー名を得る
	Identifier* getCascadeName() const;
	//子サーバー名設定
	void setCascadeName(Identifier* pId_);

	// ホスト名を得る
	ValueExpression*	getHost() const;
	// ホスト名を設定する
	void setHost(ValueExpression* pHost_);

	// ポート番号を得る
	ValueExpression*	getPort() const;
	// ポート番号を設定する
	void setPort(ValueExpression* pPort_);

	// データベース名を得る
	Identifier*	getDatabase() const;
	// データベース名を設定する
	void setDatabase(Identifier* pDatabase_);

	//自身をコピーする
	Object* copy() const;

protected:
private:
	// 代入オペレーターは使わない
	CascadeDefinition& operator=(const CascadeDefinition& cOther_);
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_CASCADEDEFINITION_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
