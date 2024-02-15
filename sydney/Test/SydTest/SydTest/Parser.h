// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Parser.h -- テスト内容記述ファイルのパーズを行うクラス
// 
// Copyright (c) 2000, 2001, 2003, 2006, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SYDTEST_PARSER_H
#define __SYDNEY_SYDTEST_PARSER_H

#include "Common/Common.h"
#include "Common/Internal.h"
#include "ModUnicodeString.h"
#include "SydTest/Command.h"
#include "SydTest/String.h"
#include "SydTest/Parameter.h"
#include "SydTest/Number.h"
#include "SydTest/Option.h"
#include "SydTest/FileBuffer.h"

_SYDNEY_BEGIN

namespace Common
{
	class DataArrayData;
	class ModString;
}

namespace SydTest
{

//
//	CLASS
//	SydTest::Parser -- テスト内容記述ファイルのパーズを行うクラス
//
//	NOTES
//
//
//##ModelId=3A9B47470309
class  Parser 
{
public:

	// Token のタイプ
	struct TokenType
	{
		enum Value
		{
			Letters = 256,
			Number, // 整数
			Decimal // 実数
		};
	};
	//コンストラクタ
	//##ModelId=3A9B4747032A
	Parser();
	//デストラクタ
	//##ModelId=3A9B47470329
	virtual ~Parser();
	// テスト内容記述ファイルのオープン
	//##ModelId=3A9B47470327
	bool openFile(const char* pszFileName_);
	// 次のコマンドオブジェクトを得る
	//##ModelId=3A9B47470323
	Item* getNextCommand(bool bUnreplaceBackSlash_);
	// ラベルまでを読み飛ばす
	//##ModelId=3A9B47470321
	bool getLabel(const char* pszLabel_);
	// 解析中の行番号を返す
	int getLine() const;

private:
	// ファイルよりトークンを取り出す
	//##ModelId=3A9B4747031F
	int getNextToken(char* pBuffer_);
	// UCS2ファイルより文字列を取り出す
	//##ModelId=3A9B4747031D
	Common::DataArrayData::Pointer readUCSFile(const char* cszFileName_);
	// パラメータを配列に取り出す
	//##ModelId=3A9B47470316
	Common::DataArrayData* getParameter(bool bReplaceBackSlash_);
	// 文字列を取り出す
	//##ModelId=3A9B47470313
	ModUnicodeString  getString(bool bReplaceBackSlash_ = false);
	
	// ファイルストリーム
	//##ModelId=3A9B4747030C
	FileBuffer m_file;
};

}

_SYDNEY_END

#endif //__SYDNEY_SYDTEST_PARSER_H

//
//	Copyright (c) 2000, 2001, 2003, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
