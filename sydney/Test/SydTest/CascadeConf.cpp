// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CascadeConf.cpp -- 子サーバの設定情報クラス
// 
// Copyright (c) 2000, 2001, 2004, 2013, 2015, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "SydTest";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SydTest/FileBuffer.h"
#include "SydTest/CascadeConf.h"
#include <stdlib.h>

namespace {
	const int iBufferUnit = 256;
	const char* KEY_HOST_NAME     = "HostName";
	const char* KEY_PORT_NUMBER   = "PortNumber";
	const char* KEY_CONF_PATH     = "ConfPath";
	const char* KEY_SYSTEM_CONF_PATH = "SystemConfPath"; // Linux/Solaris用
	const char* KEY_INSTALL_PATH  = "InstallPath";
	const char* KEY_SERVICE_NAME  = "ServiceName";	// Windows専用
}

_SYDNEY_USING

using namespace SydTest;

//
//	FUNCTION public
//	SydTest::CascadeConf::CascadeConf -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//  なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
CascadeConf::CascadeConf()
{
}

//
//	FUNCTION public
//	SydTest::CascadeConf::~CascadeConf -- デストラクタ
//
//	NOTES
//	デストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
CascadeConf::~CascadeConf()
{
}

//
//	FUNCTION public
//	SydTest::CascadeConf::getLine -- ファイルの行数を取得する
//
//	NOTES
//	現在読み込んでいるファイルの行数を取得する
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		整数値
//
//	EXCEPTIONS
//	なし
//
/*
int
CascadeConf::getLine() const
{
	return m_file.getLineNumber();
}
*/

//
//	FUNCTION private
//	SydTest::CascadeConf::openFile -- ファイルをオープンする
//
//	NOTES
//	ファイルをオープンする
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		true    オープン成功
//      false   オープン失敗
//
//	EXCEPTIONS
//	なし
//
bool
CascadeConf::openFile(const char* pszFileName_)
{
	return m_file.open(pszFileName_);
}

//
//	FUNCTION public
//	SydTest::CascadeConf::setCascadeInfo -- 設定ファイルを読み込み、子サーバ情報を設定する
//
//	NOTES
//	設定ファイルをパーズし、構造体 CascadeInfo にマッピングする
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//	    true: 正常終了
//      false: 異常終了
//
//	EXCEPTIONS
//	なし
//
bool
CascadeConf::setCascadeInfo(const char* pszFileName_, ModVector< CascadeInfo >& cascadeInfoVector)
{
    if (!openFile(pszFileName_))
		return false;

    ModVector< ConfMap > hashVector;
    mapConfToHash(hashVector);
    mapHashToCascadeInfo(hashVector, cascadeInfoVector);

    return true;
}

//
//	FUNCTION public
//	SydTest::CascadeConf::setCascadeInfo -- コマンドライン引数に直接記述された子サーバ情報を取り込む
//
//	NOTES
//  子サーバの情報は、コマンドライン引数 /distfile で指定された設定ファイルから読み込むか、
//  コマンドライン引数 /dist で直接指定された情報を使用する。
//  /dist でコマンドライン引数に直接子サーバ情報を記述した場合の処理をここで扱う。
//
//	ARGUMENTS
//  int& argc
//      引数の数
//	char**& argv
//      引数の配列
//  int& iOptInd
//      解析中の引数の添字
//  ModVector<CascadeInfo>& cascadeInfoVector
//      子サーバ情報のベクター
//
//	RETURN
//	    true: 正常終了
//      false: 異常終了
//
//	EXCEPTIONS
//	なし
//
bool
CascadeConf::setCascadeInfo(int& argc, char**& argv, int& iOptInd, ModVector<CascadeInfo>& cascadeInfoVector)
{
	if (iOptInd+2 >= argc)
		return false;

	iOptInd++;
	char *strHost = argv[iOptInd];
	char *strPort = argv[iOptInd+1];
	
	while(1)
	{
		CascadeInfo newCascade = 
			{
				ModUnicodeString(strHost, Common::LiteralCode),
				::atoi(strPort),
				"", // CONF_PATH
				""  // INSTALL_PATH
			};
		cascadeInfoVector.pushBack(newCascade);

		iOptInd++;
		char *p = strchr(strPort, ',');
		
		if (p != 0)
		{	
			// ポート番号の後の区切りが ", " (コンマの後ろに半角スペース) 
			// (e.g. "/dist localhost 54321, localhost 54322") 
			if (p == strPort + strlen(strPort) - 1)
			{
				if (iOptInd+1 >= argc)
					return false;
				
				strHost = argv[++iOptInd];
			}
			// ポート番号の後の区切りが "," (コンマの前後に半角スペースなし) 
			// (e.g. "/dist localhost 54321,localhost 54322")
			else
			{
				strHost = p + 1;
			} 

			if (iOptInd+1 >= argc)
				return false;

			strPort = argv[iOptInd+1];
		}
		else
		{
			// 子サーバ情報設定を正常終了
			// 最後のオプションまで読んだ、または、次のオプションが始まった
			if (iOptInd+1 == argc || argv[iOptInd+1][0] == '/')
			{
				return true;
			}
			// ポート番号の後の区切りが " , " (コンマの前後に半角スペース) 
			// (e.g. "/dist localhost 54321 , localhost 54322")
			else if (strchr(argv[iOptInd+1], ',') != 0)
			{
				iOptInd+=2;
				if (iOptInd+1 >= argc)
					return false;

				strHost = argv[iOptInd];
				strPort = argv[iOptInd+1];
			}
			// 子サーバ指定の異常終了(シンタックスエラー)
			else
			{
				return false;
			}
		}
	}
}

//
//	FUNCTION private
//	SydTest::CascadeConf::mapConfToHash -- 設定ファイルの情報をハッシュにマッピングする
//
//	NOTES
//  設定ファイルの情報をハッシュにマッピングする
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
CascadeConf::mapConfToHash(ModVector< ConfMap >& hashVector)
{
	char pszString[iBufferUnit];
	int iReturn = 0;

	ModUnicodeString cstrKey;
	ModUnicodeString cstrValue;
	
	do {
		iReturn = getNextToken(pszString);
		
		switch (iReturn)
		{
			case TokenType::KeyLetters:
				cstrKey = ModUnicodeString(pszString);
				break;
				
			case TokenType::ValueLetters:
				cstrValue = ModUnicodeString(pszString);
				hashVector[hashVector.getSize()-1].insert(cstrKey, cstrValue);
				break;
				
			case TokenType::Array:
				{
					ConfMap KeyValMap;
					hashVector.pushBack(KeyValMap);
					break;
				}
			default:
				break;
		}
	} while( !(iReturn == 0 && *pszString == '\0') );
}

//
//	FUNCTION private
//	SydTest::CascadeConf::mapHashToCascadeInfo -- ハッシュを構造体 CascadeInfo にマッピングする
//
//	NOTES
//  ハッシュを構造体 CascadeInfo にマッピングする
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
CascadeConf::mapHashToCascadeInfo(ModVector< ConfMap >& hashVector, 
								  ModVector< CascadeInfo >& cascadeInfoVector)
{
	for (unsigned int i = 0; i < hashVector.getSize(); i++)
	{
		CascadeInfo newCascade =
			{
#ifdef SYD_OS_WINDOWS				
				hashVector[i][KEY_HOST_NAME],
				::atoi(hashVector[i][KEY_PORT_NUMBER].getString()),
				hashVector[i][KEY_CONF_PATH],
				hashVector[i][KEY_SYSTEM_CONF_PATH],
				hashVector[i][KEY_INSTALL_PATH],
				hashVector[i][KEY_SERVICE_NAME]
#else
				hashVector[i][KEY_HOST_NAME],
				::atoi(hashVector[i][KEY_PORT_NUMBER].getString()),
				hashVector[i][KEY_CONF_PATH],
				hashVector[i][KEY_SYSTEM_CONF_PATH],
				hashVector[i][KEY_INSTALL_PATH]
				// LinuxにServiceName情報はない
#endif				
			};
		cascadeInfoVector.pushBack(newCascade);
	}
}

//
//	FUNCTION private
//	SydTest::CascadeConf::getNextToken -- 次のトークンを取得する
//
//	NOTES
//  次のトークンを取得する
//
//	ARGUMENTS
//	char* pBuffer_
//      切り出したトークン
//
//	RETURN
//	int
//      トークンの種類
//
//	EXCEPTIONS
//	なし
//
int
CascadeConf::getNextToken(char* pBuffer_)
{
	char* buf;
	ModUnicodeChar us;
	// iType = 0 は先頭文字であることを表す
	int iType = 0;
	
	*pBuffer_ = '\0';
	buf = pBuffer_;
	
	while ( (us = m_file.get()) != SYDTEST_EOF)
	{
		switch (us)
		{
			case '#':
				while ((us = m_file.get()) != SYDTEST_EOF && us != '\n');
				break;
				
			case '"':
				// ダブルクオーテーションの有効範囲は行末まで
				if (iType != TokenType::ExplicitLetters) {
					iType = TokenType::ExplicitLetters;
				} else {
					iType = TokenType::Letters;
				}
				break;
				
			case '-':
				if (iType == 0) {
					return TokenType::Array;
				} else {
					*buf++ = (char)us;
				}
				break;
				
			case '\n':
				// 改行はダブルクオーテーションで括っても無効
				if (*pBuffer_ != '\0') { 
					*buf = '\0';
					//m_file.putback(us);
					iType = TokenType::ValueLetters;
					return iType;
				}
				break;
				
			case ':':
				if (iType == TokenType::ExplicitLetters) {
					*buf++ = (char)us;
					break;
					
				} else {
					if (*pBuffer_ != '\0') { 
						*buf = '\0';
						
						//m_file.putback(us);
						return TokenType::KeyLetters;
						// return getKeyType(buf);
					}
					break;
				}
				
			case '\t':
			case ' ':
				if (iType != TokenType::ExplicitLetters) {
					break;
				}
				
			default:
				/*
			 if (ModUnicodeCharTrait::isDigit(us))
			 {
			 if (iType == 0 )
		     iType = TokenType::ValueNumber;
			 } 
			 else 
			 {
			 iType = TokenType::Letters;
			 }*/
				if (iType != TokenType::ExplicitLetters) {
					iType = TokenType::Letters;
				}
				*buf++ = (char)us;
				break;
		}
	}
	
	if (*pBuffer_ != '\0')
	{
		*buf = '\0';
		return TokenType::ValueLetters;
	}
	
	return iType;
}

//
//	Copyright (c) 2000, 2001, 2004, 2013, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
