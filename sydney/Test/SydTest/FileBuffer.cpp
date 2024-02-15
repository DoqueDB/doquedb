// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileBuffer.cpp -- マルチバイト対応のバッファクラス。
//				     ModI*Streamがなかったので自作した。
// 
// Copyright (c) 2001, 2002, 2003, 2004, 2006, 2023 Ricoh Company, Ltd.
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

// 課題: 排他制御も追加

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "SydTest";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SydTest/FileBuffer.h"
#include "SydTest/SydTestMessage.h"
#include "Common/Assert.h"
#include <fstream>
#include <iostream>
using namespace std;


_SYDNEY_USING
using namespace SydTest;

//
//	FUNCTION public
//	FileBuffer::FileBuffer -- コンストラクタ
//
//	NOTES
//    コンストラクタ
//
//	ARGUMENTS
//    なし
//
//	RETURN
//    なし
//
//	EXCEPTIONS
//    なし
//
FileBuffer::FileBuffer()
: m_iLineNumber(0), m_bLastLine(false),
  m_pLine(0), m_pCursor(0)
{
}

//
//	FUNCTION public
//	FileBuffer::~FileBuffer -- デストラクタ
//
//	NOTES
//    デストラクタ
//
//	ARGUMENTS
//    なし
//
//	RETURN
//    なし
//
//	EXCEPTIONS
//    なし
//
FileBuffer::~FileBuffer()
{
	if (m_pLine)
	{
		delete m_pLine, m_pLine=0;
	}
}

//
//	FUNCTION public
//	FileBuffer::open -- ファイルを開く
//
//	NOTES
//    ファイルを開く
//
//	ARGUMENTS
//    const char* pszFileName_
//      ファイルのパス
//
//	RETURN
//    bool
//      openに成功したか否か
//
//	EXCEPTIONS
//    なし
//
bool
FileBuffer::open(const char* pszFileName_)
{
	m_cFile.open(pszFileName_, ios::in);
	return !m_cFile.fail();
}

//
//	FUNCTION public
//	FileBuffer::close -- ファイルを閉じる
//
//	NOTES
//    ファイルを閉じる
//
//	ARGUMENTS
//    なし
//
//	RETURN
//    なし
//
//	EXCEPTIONS
//    なし
//
void
FileBuffer::close()
{
	m_cFile.close();
}

//
//	FUNCTION public
//	FileBuffer::getLineNumber -- 行番号を得る
//
//	NOTES
//    行番号を得る
//
//	ARGUMENTS
//    なし
//
//	RETURN
//    int
//      行番号
//
//	EXCEPTIONS
//    なし
//
int 
FileBuffer::getLineNumber() const
{
	return m_iLineNumber;
}

//
//	FUNCTION public
//	FileBuffer::get -- ファイルから一字取得する
//
//	NOTES
//    ファイルから一字取得する
//
//	ARGUMENTS
//    なし
//
//	RETURN
//    ModUnicodeChar
//      取得した文字
//
//	EXCEPTIONS
//    なし
//
ModUnicodeChar
FileBuffer::get()
{
	if (!isLastLine() && (m_pLine==0 || *m_pCursor == '\0'))
	{
		// 新しい行を読み込む
		delete m_pLine, m_pLine=0;
		m_pLine = getLine();
	}

	ModUnicodeChar ucResult = *m_pCursor;
	m_pCursor++;

	if (ucResult == '\0' && isLastLine())
		return SYDTEST_EOF;
	else 
		return ucResult;
}

//
//	FUNCTION public
//	FileBuffer::putback -- ファイルに一字戻す
//
//	NOTES
//    ファイルに一字戻す
//
//	ARGUMENTS
//    const ModUnicodeChar c_
//      戻す文字
//
//	RETURN
//    なし
//
//	EXCEPTIONS
//    なし
//
void
FileBuffer::putback(const ModUnicodeChar c_)
{
	if (m_pCursor != &(*m_pLine)[0]) {
		m_pCursor--;
	} else {
		if (isSydTestDebugMessage())
		{
			SydTestDebugMessage << m_pCursor << ModEndl;
		}
		ModUnicodeString* pTmp = m_pLine;
		m_pLine = new ModUnicodeString(c_);
		*m_pLine += *pTmp;
		m_pCursor = &(*m_pLine)[0];
	}
}

//
//	FUNCTION public
//  FileBuffer::isLastLine -- 最終行か否かを返す
//
//	NOTES
//    最終行か否かを返す
//
//	ARGUMENTS
//    なし
//
//	RETURN
//    bool
//      最終行ならばtrue
//
//	EXCEPTIONS
//    なし
//
bool
FileBuffer::isLastLine() const
{
	return m_bLastLine;
}

//private function

//
//	FUNCTION private
//	FileBuffer::getLine -- ファイルから1行取得
//
//	NOTES
//	  ファイルから1行取得する。
//
//	ARGUMENTS
//    なし
//
//	RETURN
//    ModUnicodeString*
//      取得した行
//
//	EXCEPTIONS
//    SydTestException
//
ModUnicodeString*
FileBuffer::getLine()
{
	ModCharString cstrBuff;
	ModUnicodeString* newLine;

	SydAssert(m_pLine==0);
	int i;
	while (true) {
		i = m_cFile.get();
		if (i == EOF) {
			m_bLastLine = true;
			break;
		}
		// 残念ながらModCharStringには\0をappendできないので、
		// 将来UCS-2に対応する際には Parser::readUCSFile()を拝借する必要がある
		cstrBuff.append(i);
		if (i == '\n') {
			m_iLineNumber++;
			break;
		}
	}

	newLine = new ModUnicodeString
			(cstrBuff, 0, ModOs::Process::getEncodingType());
	m_pCursor = &(*newLine)[0];
	return newLine;
}

//
//	Copyright (c) 2001, 2002, 2003, 2004, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
