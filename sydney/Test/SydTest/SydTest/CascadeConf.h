// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CascadeConf.h -- 子サーバの設定情報クラス
// 
// Copyright (c) 2012, 2013, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SYDTEST_CASCADE_CONF_H
#define __SYDNEY_SYDTEST_CASCADE_CONF_H

#include "SydTest/Map.h"
#include "SydTestException.h"
#include "SydTest/FileBuffer.h"
#include "ModUnicodeString.h"
#include "ModVector.h"
#include "ModMap.h"

_SYDNEY_BEGIN

namespace SydTest
{

//子サーバの情報を格納する構造体
struct CascadeInfo
{
	//子サーバのホスト名
	ModUnicodeString cstrHostName;
	//子サーバのポート番号
	int iPortNumber;
	//設定ファイルのパス
	//Windowsの場合はレジストリパス、Linuxの場合はインストール先の設定ファイルパス
	ModUnicodeString cstrConfPath;
	//Linux用
	ModUnicodeString cstrSystemConfPath;
	//インストールパス
	ModUnicodeString cstrInstallPath;
	//サービス名
	ModUnicodeString cstrServiceName;
};

//
//	CLASS
//	SydTest::CascadeConf -- 子サーバの情報
//
//	NOTES
//
//
class CascadeConf
{
public:
	    typedef ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> > ConfMap;

        //コンストラクタ
		CascadeConf();
		//デストラクタ
		~CascadeConf();
		//子サーバの設定が記述された外部ファイルから子サーバの情報を取り込む
		bool setCascadeInfo(const char* pszFileName_, ModVector<CascadeInfo>& cascadeInfoVector);
		//コマンドライン引数に直接記述された子サーバの情報を取り込む
		bool setCascadeInfo(int& argc, char**& argv, int& iOptInd, ModVector<CascadeInfo>& cascadeInfoVector);

private:
		//ファイルの行数を取得する
		//int getLine() const;
		//ファイルをオープンする
		bool openFile(const char* pszFileName_);
		//設定ファイル情報をハッシュにマッピングする
		void mapConfToHash(ModVector< ConfMap >& hashVector);
		//ハッシュを構造体 CascadeInfo にマッピングする
		void mapHashToCascadeInfo(ModVector< ConfMap >& hashVector,
								  ModVector< CascadeInfo >& cascadeInfoVector);
		//次のトークンを取得する
		int getNextToken(char* pBuffer_);

		// ファイルストリーム
		FileBuffer m_file;

		// Token のタイプ
		struct TokenType
		{
				enum Value
				{
					KeyLetters = 1,    /* 項目名 (key) */
					ValueLetters,      /* 文字列の設定値 (value) */
					ValueNumber,       /* 数値の設定値 (value) */
					Letters,           
					ExplicitLetters,
					Array              /* 配列の要素 */
				};
		};
};

}

_SYDNEY_END

#endif //__SYDNEY_SYDTEST_CASCADE_CONF_H

//
//	Copyright (c) 2012, 2013, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
