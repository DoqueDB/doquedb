// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DisconnectStatement.h --
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

#ifndef __SYDNEY_STATEMENT_DISCONNECTSTATEMENT_H
#define __SYDNEY_STATEMENT_DISCONNECTSTATEMENT_H

#include "Common/Common.h"
#include "Statement/Object.h"
#include "Common/Data.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class Literal;

//
//	CLASS
//	Statement::DisconnectStatement --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION DisconnectStatement  : public Statement::Object
{
public:

	struct Mode
	{
		//	
		//	Statement::DisconnectStatement::Mode::Value --
		//		Disconnectモードを示す
		//
		//	NOTES

		enum Value
		{
			// セッション削除モード
			SESSION_MODE = 0,

			// クライアント削除モード
			CLIENT_MODE,
			
			// 値の数
			Count
		};
	};
	  
	//constructor
	DisconnectStatement()
		: Object(ObjectType::DisconnectStatement)
	{}
	//コンストラクタ
	DisconnectStatement(Mode::Value iMode_);
	//デストラクタ
	virtual ~DisconnectStatement();

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

	// セッションIDを得る
	int getSessionId() const { return m_iSessionId; }

	// セッションIDを設定する
	void setSessionId(const Literal& cLiteral_);

		// クライアントIDを得る
	int getClientId() const { return m_iClientId; }

	// セッションIDを設定する
	void setClientId(const Literal& cLiteral_);

	Mode::Value getDisconnectMode() const { return m_iMode; };

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);
	
private:
	// 代入オペレーターは使わない
	DisconnectStatement& operator=(const DisconnectStatement& cOther_);
	
	// モードタイプ
	Mode::Value m_iMode;

	// クライアントID
	int m_iClientId;
	
	// セッションID
	int m_iSessionId;




	
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_DISCONNECTSTATEMENT_H

//
//	Copyright (c) 2005, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
