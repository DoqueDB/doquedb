// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Parser.cpp -- テスト内容記述ファイルをパーズするクラス
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2010, 2023 Ricoh Company, Ltd.
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

// 先頭に置くと(そしてそのときに限り)、変なwarningを抑止する効果をもつ
#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SydTest/FileBuffer.h"
#include "SydTest/Parser.h"
#include "SydTest/Executor.h"
#include "SydTest/SydTestException.h"
#include "SydTest/SydTestMessage.h"

#include "Common/Assert.h"
#include "Common/BinaryData.h"
#include "Common/DataArrayData.h"
#include "Common/IntegerData.h"
#include "Common/Integer64Data.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/UnsignedInteger64Data.h"
#include "Common/FloatData.h"
#include "Common/DoubleData.h"
#include "Common/DateData.h"
#include "Common/DateTimeData.h"
#include "Common/StringData.h"
#include "Common/NullData.h"
#include "Common/UnicodeString.h"

#include "ModAutoPointer.h"
#include "ModUnicodeString.h"
#include "ModCharString.h"
#include "ModUnicodeCharTrait.h"
#include "ModUnicodeOstrStream.h"

#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <iostream>

using namespace std;

namespace {
	// バッファの単位(バイト)
	const int _iBufferUnit = 16384; 
}

_SYDNEY_USING

using namespace SydTest;

//
//	FUNCTION public
//	SydTest::Parser::Parser -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
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
Parser::Parser()
{
}

//
//	FUNCTION public
//	SydTest::Parser::~Parser -- デストラクタ
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
Parser::~Parser()
{
}

//
//	FUNCTION public
//	SydTest::Parser::openFile -- ファイルをオープンする
//
//	NOTES
//	テスト内容記述ファイルをオープンする
//
//	ARGUMENTS
//	const char* cstrFileName_
//		オープンする内容記述ファイル名
//
//	RETURN
//	bool
//		true	オープン成功
//		false	オープン失敗
//
//	EXCEPTIONS
//	なし
//
bool
Parser::openFile(const char* pszFileName_)
{
	return m_file.open(pszFileName_);
}

//
//	FUNCTION public
//	SydTest::Parser::getNextCommand -- 処理内容を得る
//
//	NOTES
//	処理の内容を記述したファイルより、次の処理に関する情報を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Sydtest::Item*  
//		得られたコマンド要素オブジェクト
//
//	EXCEPTIONS
//	なし
//
Item* 
Parser::getNextCommand(bool bReplaceBackSlash_)
{
	int iReturn = 0;
	char pszString[256];
	Item* pItem = 0;

	// ';' がくるかファイル末尾に到達するまでが一コマンド
	// '{', '}'が来た場合もそこで1コマンドとしてそれ以降とは切る
	if ((iReturn = getNextToken(pszString)) != ';' 
		 && iReturn != '{' && iReturn != '}') {

		switch (iReturn) {
		// ""でくくられた文字列の場合SQL文かコマンドのオプション
		case '"':
			{
			ModUnicodeString tmpString = getString(bReplaceBackSlash_);
			/*
#ifdef SYD_OS_POSIX
			// パスの区切り文字を変更する。
			// 注意！ SQL文には'\'を直接使えなくなる。
			//        SQL文に書き込みたい時は、パラメータを使う。
			//        コマンドのオプションには全く使えなくなる。
			// 040714 
			//  ESCAPE(escape)指定している場合は使えることにする。
			//  ex: 1350,1690
			// 070410 
			//	UnreplaceBackSlash指定がある場合は、置き換えないことにする。
			// 070523 
			//	SYD_OS_POSIXやESCAPEなどの個別処理を廃止して、
			//	bReplaceBackSlash_ == true で置き換える。

			if (bUnreplaceBackSlash_ == false)
			{
				ModUnicodeString subString1 = ModUnicodeString("ESCAPE '");
				subString1 += Common::UnicodeChar::usBackSlash;
				subString1 += '\'';
				ModUnicodeString subString2 = ModUnicodeString("ESCAPE ('");
				subString2 += Common::UnicodeChar::usBackSlash;
				subString2 += ModUnicodeString("')");
				if (tmpString.search(subString1, ModFalse) == 0
					&& tmpString.search(subString2, ModFalse) == 0)
				{
					tmpString.replace(Common::UnicodeChar::usBackSlash,
									  ModOsDriver::File::getPathSeparator());
				}
			}
#endif
			*/
			pItem = new String(tmpString);
			break;
			}
		// []でくくられている場合はパラメータを表す
		case '[':
			pItem = new Parameter(getParameter(bReplaceBackSlash_));
			break;
		// くくられていない文字列が来た場合はコマンドもしくはオプション名を表す
		case TokenType::Letters:
		{
			char* p;
			// 末尾が「:」の場合はオプション名
			if ( (p = strchr(pszString, ':')) == pszString + strlen(pszString) - 1) {
				*p = '\0';
				pItem = new Option(pszString);
			} else {
				pItem = new Command(pszString);
			}
			break;
			}
		// 数字(整数)が来た場合はコマンドのオプション
		case TokenType::Number:
			pItem = new Number(pszString);
			break;
		// それ以外は文法エラー
		default:
			SydTestErrorMessage << "Syntax Error. Abort this thread." << ModEndl;
			SydTestErrorMessage << m_file.getLineNumber() << ModEndl;
			throw SydTestException(m_file.getLineNumber());
		}
	}

	// 要素が得られた場合はそれに関連する次の要素を得る
	if (pItem != 0)
	{
		pItem->setNext(getNextCommand(bReplaceBackSlash_));
		Item* pNext = pItem->getNext();
		if (pNext != 0 && pNext->getType() == Item::Type::Command)
		{
			// なんか不恰好だな…
			// '{'や'}'の扱いはコマンドでないほうがいいかも。
			Command* pNextCommand = dynamic_cast<Command*>(pNext);
			// getType()ではなく、getCommandType()
			if (!(pNextCommand->getCommandType() == Command::CommandType::BeginBrace
				|| pNextCommand->getCommandType() == Command::CommandType::EndBrace))
			{
				if (isSydTestInfoMessage())
				{
					SydTestInfoMessage << "You may dropped a semicolon around "
						<< pNextCommand->getCommand() << "." << ModEndl;
				}
				throw SydTestException(m_file.getLineNumber());
			}
		}
	}

	// '{', '}'が来た場合はそれで一つのコマンドと見なす
	if (iReturn == '{' || iReturn == '}')
	{
		if (iReturn == '{')
			pItem = new Command("BeginBrace");
		else
			pItem = new Command("EndBrace");
		pItem->setNext(0);
	}

	return (pItem);
}

//
//	FUNCTION public
//	SydTest::Parser::getLabel -- ラベルの存在を問い合わせる
//
//	NOTES
//	処理の内容を記述したファイルにおいて、
//  引数に示したラベルが存在するか否かを問い合わせる。
//  ファイルのポインタは、ラベルが存在するときはラベルの位置に、
//  ラベルが存在しないときはファイル末尾に移る。
//
//	ARGUMENTS
//	const char* pszLabel_
//    問い合わせるラベルの名前
//
//	RETURN
//	bool
//	  true: ラベルがある、false: ラベルがない
//
//	EXCEPTIONS
//	  なし
//
bool
Parser::getLabel(const char* pszLabel_)
{
	int iReturn;
	char pszString[_iBufferUnit];

	while ((iReturn = getNextToken(pszString)) != 0) 
	{
		// ""でくくられた文字列内は評価しない
		if (iReturn == '"')
		{
			getString();
			continue;
		}
		if (iReturn == TokenType::Letters && strcmp(pszString, pszLabel_) == 0)
		{
			break;
		}
	}

	return (iReturn != 0);
}

//
//	FUNCTION public
//	SydTest::Parser::getLabel -- 解析中の行番号を返す
//
//	NOTES
//    解析中の行番号を返す
//
//	ARGUMENTS
//	  なし
//
//	RETURN
//	int
//	  解析中の行番号
//
//	EXCEPTIONS
//	  なし
//
int
Parser::getLine() const
{
	return m_file.getLineNumber();
}

//
// private
//
//
//	FUNCTION private
//	SydTest::Parser::getNextToken -- トークンを得る
//
//	NOTES
//	トークンを切り出す
//
//	ARGUMENTS
//	char* pBuffer_
//		(戻り値)得られた文字列
//
//	RETURN
//	int
//		切り出した区切り文字そのもの あるいはトークンのタイプ
//
//	EXCEPTIONS
//	なし
//
int 
Parser::getNextToken(char* pBuffer_)
{
	char* buf;
	ModUnicodeChar us;
	int iType = 0;
	*pBuffer_ = '\0';
	buf = pBuffer_;

	while ((us = m_file.get()) != SYDTEST_EOF) 
	{
		switch (us) 
		{
		// '#'が来た場合は行末までコメントと見なす 
		case '#':
			while (us = m_file.get()) {
				if (us == SYDTEST_EOF || us == '\n') 
					break;
			}
			break;
		// 区切り文字が来た場合 そこまでに文字列があれば文字列を返す
		// 文字列がない場合は区切り文字そのものを返す
		case '[':
		case ']':
		case '"':
		case ',':
		case ';':
		case '{':
		case '}':
			if (*pBuffer_ != '\0') {
				*buf = '\0';
				m_file.putback(us);
				return iType;
			}
			else 
				return us;
			break;

		case '\r':
		case '\n':
		case ' ':
		case '\t':
			// 文字列の区切りであればそこまでの文字列を返す
			// それ以外のスペース,改行コード、タブは無視する
			if (*pBuffer_ != '\0') {
				*buf = '\0';
				m_file.putback(us);
				return iType;
			}
			break;

		case '-':
			// 第一字目ならばFloatかIntegerになりうる
			// それ以外なら文字列
			if (iType != 0) 
			{
				iType = TokenType::Letters;
			}
			*buf++ = us;
			break;

		case '.':
			// ここまで数字しか出てきていなければFloatと見なす
			// それ以外は文字列
			if (iType == TokenType::Number) 
				iType = TokenType::Decimal;
			else 
				iType = TokenType::Letters;
			*buf++ = us;
			break;

		default:
			// 文字が数字だったらそれまでのタイプで判断
			if (ModUnicodeCharTrait::isDigit(us))  {
				if (iType == 0) 
					iType = TokenType::Number;
			} else {
				iType = TokenType::Letters;
			}
			*buf++ = us;
			break;
		} // end of switch
	}

	if (*pBuffer_ != '\0') {
		*buf = '\0';
	}

	return iType;
}

//
//	FUNCTION private
//	SydTest::Parser::getParameter -- パラメータを得る
//
//	NOTES
//	[]でくくられたパラメータを得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::DataArrayData*	
//		得られたパラメータを表す配列
//
//	EXCEPTIONS
//	なし
//
Common::DataArrayData*
Parser::getParameter(bool bReplaceBackSlash_)
{
	char buff[_iBufferUnit];
	int iParamType = Parameter::ParamType::Other;
	int iFlag = 0;
	int iTokenType = 0;
	Common::DataArrayData* pArrayData = new Common::DataArrayData();
	Common::DataArrayData::Pointer pData;
	
	while ((iTokenType = getNextToken(buff)) != ']')
	{
		ModAutoPointer<Common::StringData> pParam = new Common::StringData(ModUnicodeString(buff));
		switch (iTokenType)
		{
		case TokenType::Letters:
			// 得た文字列が型名を表しているかどうか判定
			iParamType = Parameter::getParameterType(buff);
			if (iParamType == Parameter::ParamType::NullData)
			{
				pData = Common::NullData::getInstance();
				iFlag = 1;
			}
			else if (iParamType == Parameter::ParamType::Other)
			{
				SydTestErrorMessage << "Syntax error around '"
					<< buff << "': Not a type string." << ModEndl;
				throw SydTestException(m_file.getLineNumber());
			}
			break;
			// 得た文字列が数値
		case TokenType::Number:
			switch(iParamType){
			case Parameter::ParamType::Integer:
			case Parameter::ParamType::Other:
				pData = pParam->cast(Common::DataType::Integer, true);
				break;
			case Parameter::ParamType::Integer64:
				pData = pParam->cast(Common::DataType::Integer64, true);
				break;
			case Parameter::ParamType::UnsignedInteger:
				pData = pParam->cast(Common::DataType::UnsignedInteger, true);
				break;
			case Parameter::ParamType::UnsignedInteger64:
				pData = pParam->cast(Common::DataType::UnsignedInteger64, true);
				break;
			case Parameter::ParamType::Float:
				pData = pParam->cast(Common::DataType::Float, true);
				break;
			case Parameter::ParamType::Double:
				pData = pParam->cast(Common::DataType::Double, true);
				break;
			default: // error
				SydTestErrorMessage << "Syntax Error: Insufficient parameter." << ModEndl;
				throw SydTestException(m_file.getLineNumber());
			}
			// 値がセットされたらフラグは1にする
			if (pData.get() != 0)
				iFlag = 1;
			break;

		case TokenType::Decimal:
			if (iParamType == Parameter::ParamType::Float)
			{
				pData = pParam->cast(Common::DataType::Float, true);
			}
			else if (iParamType == Parameter::ParamType::Double
					 || iParamType == Parameter::ParamType::Other)
				pData = pParam->cast(Common::DataType::Double, true);
			else // error
			{
				SydTestErrorMessage << "Syntax Error: Not a number." << ModEndl;
				throw SydTestException(m_file.getLineNumber());
			}
			if (pData.get() != 0)
				iFlag = 1;
			break;

		case '"':
			if (iParamType == Parameter::ParamType::Char
				|| iParamType == Parameter::ParamType::Other)
				pData = new Common::StringData(getString());
			else if (iParamType == Parameter::ParamType::Time)
				pData = new Common::DateTimeData(getString());
			else if (iParamType == Parameter::ParamType::Date)
				pData = new Common::DateData(getString());
			// バイナリファイル
			else if (iParamType == Parameter::ParamType::BinaryFile)
			{
				ModUnicodeString cFilename = getString(bReplaceBackSlash_);
//#ifdef SYD_OS_POSIX
//				cFilename.replace(Common::UnicodeChar::usBackSlash,
//								  ModOsDriver::File::getPathSeparator());
//#endif
				ifstream cFile(cFilename.getString(), ios::in | ios::binary);
				if (cFile.fail())
				{
					delete pArrayData, pArrayData=0;
					SydTestErrorMessage 
						<< "No such file " << cFilename << "." << ModEndl;
					throw SydTestException(m_file.getLineNumber());
				}

				ModFileSize ulSize = ModOsDriver::File::getFileSize(cFilename);
				; _SYDNEY_ASSERT(ulSize < static_cast<ModFileSize>(ModSizeMax));
				ModSize iFileSize = static_cast<ModSize>(ulSize);

#ifdef SYD_OS_POSIX
				// テキストをバイナリとして読み込む場合
				ModUnicodeString txt = ModUnicodeString(".txt");
				if (cFilename.search(txt,ModTrue))
				{
					char* pszBuff = new char[iFileSize*2+1];
					int c = 0;
					iFileSize = 0;
					while ((c = cFile.get()) != EOF) 
					{
						if (c == 0x0a)
						{
							pszBuff[iFileSize++] = 0x0d;
						}
						pszBuff[iFileSize++] = c;
					}
					pData = new Common::BinaryData(pszBuff, iFileSize);
					delete [] pszBuff;
				}
				else
				{
					char* pszBuff = new char[iFileSize+1];
					cFile.read(pszBuff, iFileSize);
					pData = new Common::BinaryData(pszBuff, iFileSize);
					delete [] pszBuff;
				}

#else
				char* pszBuff = new char[iFileSize+1];
				cFile.read(pszBuff, iFileSize);
				pData = new Common::BinaryData(pszBuff, iFileSize);
				delete [] pszBuff;
#endif
			}

			// テキストファイルの場合はファイル内容をそのまま読み込む
			else if (iParamType == Parameter::ParamType::TextSFile)
			{
				// WindowsではSJIS、それ以外ではEUCとして読む
				
				ModUnicodeString cFilename = getString(bReplaceBackSlash_);
//#ifdef SYD_OS_POSIX
//				cFilename.replace(Common::UnicodeChar::usBackSlash,
//								  ModOsDriver::File::getPathSeparator());
//#endif
				ifstream cFile(cFilename.getString(), ios::in);
				if (cFile.fail()){
					delete pArrayData, pArrayData=0;
					SydTestErrorMessage << "No such file "
						<< cFilename << "." << ModEndl;
					throw SydTestException(m_file.getLineNumber());
				}
			
				ModCharString cstrString;
				// 041126 
				// getFileSizeがSamba上のファイルだとPermission denyを返した。
				// reallocateしているだけなのでコメントアウトする。
				// cstrString.reallocate(ModOsDriver::File::getFileSize(cFilename)+1);
				int c = 0;
				while ((c = cFile.get()) != EOF) 
				{
					if (c == 0x0a)
					{
						cstrString.append(0x0d);
					}
					cstrString.append(c);
				}
				ModUnicodeString strFile(cstrString, 0, Common::LiteralCode);
				pData = new Common::StringData(strFile);
			}

			else if (iParamType == Parameter::ParamType::TextUFile)
			{
				// UCS2ファイル
				ModUnicodeString cFilename = getString(bReplaceBackSlash_);
//#ifdef SYD_OS_POSIX
//				cFilename.replace(Common::UnicodeChar::usBackSlash,
//								  ModOsDriver::File::getPathSeparator());
//#endif
				pData = readUCSFile(cFilename.getString());
			}

			else 
			{
				SydTestErrorMessage << "Syntax Error: Unknown Parameter Type "
					<< iParamType << "." << ModEndl;
				throw SydTestException(m_file.getLineNumber());
			}
			if (pData.get() != 0)
				iFlag = 1;
			break;

		// ','はパラメータの要素の区切り
		// 次の値に移るので、フラグは0に戻す
		case ',':
			if (pData.get() == 0 && iFlag == 0)
				pData = Common::NullData::getInstance();
			else if(iFlag == 1)
				iFlag = 0;
			break;
		// '['は配列内に更に配列がある場合
		case '[':
			pData = getParameter(bReplaceBackSlash_);
			iFlag = 1;
			break;
		// ありえない値
		default:
			SydTestErrorMessage << "Syntax Error: Unknown Parameter Type." << ModEndl;
			throw SydTestException(m_file.getLineNumber());
		}
		if (pData.get() != 0)
		{
			pArrayData->pushBack(pData);
			pData.setPointer(0);
			iParamType = Parameter::ParamType::Other;
		}
	}
	return pArrayData;
}

//
//	FUNCTION private
//	SydTest::Parser::readUCSFile -- UCS2コードで書かれたファイルを読み込む
//
//	NOTES
//	UCS2コードの場合バイトオーダーが関わってくるので、
//  OSによって処理を分ける
//
//	ARGUMENTS
//	const char* cszFileName_
//		読み込むファイル名
//
//	RETURN
//	Common::DataArrayData::Pointer
//		ファイルの内容をさすポインタ
//
//	EXCEPTIONS
//	なし
//
Common::DataArrayData::Pointer
Parser::readUCSFile(const char* cszFileName_)
{
	ifstream cFile(cszFileName_, ios::in | ios::binary);
	if (cFile.fail())
	{
		SydTestErrorMessage << "No such file " 
					<< cszFileName_ << "." << ModEndl;
		throw SydTestException(m_file.getLineNumber());
	}
	// そのままの順序(big endian)の場合フラグはTrue、
	// ひっくり返す場合(little endian)はFalse
#if SYD_BYTEORDER == 0
	bool bEndian = false;
# else 
	bool bEndian = true;
#endif
	int c;
	ModFileSize ulSize = (ModOsDriver::File::getFileSize(cszFileName_)/2)+1;
	; _SYDNEY_ASSERT(ulSize < static_cast<ModFileSize>(ModSizeMax));
	ModSize iFileSize = static_cast<ModSize>(ulSize);
	ModUnicodeString cstrString;
	cstrString.reallocate(iFileSize);

	ModUnicodeChar cChar;
	int iCount = 0;
	while ((c = cFile.get()) != EOF) 
	{
		int cNext = cFile.get();
		if (cFile.eof())
		{
			break;
		}
		if (iCount++ == 0) {
			// BOMによってendianを判断する
			if (c == 0xff && cNext == 0xfe) 
			{
				bEndian = false;
			}
			else if (c == 0xfe && cNext == 0xff) 
			{
				bEndian = true;
			}
		}
		if (bEndian)
		{
			cChar = ((c << 8) | cNext);
		}
		else 
		{
			cChar = ((cNext << 8) | c);
		}
		if (cChar != 0xfeff) // BOMを取り除く
		{
			cstrString.append(cChar);
		}
	}

	return new Common::StringData(cstrString);
}

//
//	FUNCTION private
//	SydTest::Parser::getString -- 文字列を得る
//
//	NOTES
//	""でくくられた部分を文字列と見なして読み込む。 \はエスケープ文字。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		得られた文字列	
//
//	EXCEPTIONS
//	なし
//
ModUnicodeString
Parser::getString(bool bReplaceBackSlash_)
{
	ModUnicodeString cstrString;
	ModUnicodeOstrStream cStream;
	ModUnicodeChar us;
	while ((us = m_file.get()) && us != SYDTEST_EOF) {
		if (us == Common::UnicodeChar::usBackSlash) // '\'
		{
			if ((us = m_file.get()) != SYDTEST_EOF)
				cStream << us;
		}
		else if (us == '"')
			break;
		else
			cStream << us;
	}

	if (us == SYDTEST_EOF) {
		SydTestErrorMessage << "Quotation not closed." << ModEndl;
		throw SydTestException(m_file.getLineNumber());
	}

	cstrString = cStream.getString();
	if (bReplaceBackSlash_ == true)
	{
		cstrString.replace(Common::UnicodeChar::usBackSlash,
						   ModOsDriver::File::getPathSeparator());
	}
	return cstrString;
}

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
