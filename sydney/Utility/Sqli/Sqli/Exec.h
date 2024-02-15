// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Exec.h --
// 
// Copyright (c) 2002, 2004, 2006, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_SQLI_EXEC_H
#define __TRMEISTER_SQLI_EXEC_H

#include "Common/Common.h"
#include "Client2/DataSource.h"
#include "Sqli/Option.h"

#include "ModVector.h"
#include "ModPair.h"
#include "ModUnicodeString.h"
#include "ModCharString.h"

_TRMEISTER_USING

//
//	CLASS
//	Sqli::Exec --
//
//	NOTES
//
//
class Exec 
{
public:
	//コンストラクタ
	Exec(Client2::DataSource& cDataSource_, const Option& cOption_);
	//デストラクタ
	virtual ~Exec();

	//初期化
	virtual void initialize() {}
	//終了処理
	virtual void terminate() {}

	//文字列を置換する
	ModUnicodeString replace(const ModUnicodeString& cstrSQL_);

	//実行する
	void execute();

	// プロンプトを表示する
	virtual void prompt();

	//SQL文を1つ取り出す
	virtual bool getNext(ModUnicodeString& cstrSQL_) = 0;

	//マルチバイト文字列をUnicode文字列に変換する
	static ModUnicodeString multiByteToUnicode(const char* pszBuffer_);
	//Unicode文字列をマルチバイトに変換する
	static ModCharString unicodeToMultiByte(
		const ModUnicodeString& cstrBuffer_);

	//文字コードを設定する
	static bool setCode(const ModCharString& cstrCode);
	// エコーバックの有無を設定する
	static void setEcho(bool v);
	//時間出力を設定する
	static void setTime(bool bTime_);

	//終了ステータスを得る
	static int getExitStatus();
	//終了ステータスを設定する
	static void setExitStatus(int status_);

private:
	// Sessionを新たに作る
	Client2::Session* createSession();

	//データソース
	Client2::DataSource& m_cDataSource;
	// Options
	const Option& m_cOption;
};

#endif //__TRMEISTER_SQLI_EXEC_H

//
//	Copyright (c) 2002, 2004, 2006, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
