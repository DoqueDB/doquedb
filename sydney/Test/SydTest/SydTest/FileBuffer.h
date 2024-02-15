// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileBuffer.h -- マルチバイト対応のバッファクラス
//				ModI*Streamがなかったので自作した
// 
// Copyright (c) 2001, 2003, 2004, 2006, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SYDTEST_FILEBUFFER_H
#define __SYDNEY_SYDTEST_FILEBUFFER_H

#include "Common/Common.h"
#include "Common/Internal.h"
#include "ModUnicodeString.h"
#include "ModKanjiCode.h"
#include <fstream>
#include <iostream>

#define SYDTEST_EOF (0xffff)

_SYDNEY_BEGIN

namespace SydTest
{

//
//	CLASS
//	SydTest::FileBuffer
//
//	NOTES
//
class FileBuffer
{
public:
	//コンストラクタ
	FileBuffer();
	//デストラクタ
	virtual ~FileBuffer();

	// 
	bool open(const char* pszFileName_);
	//
	void close();
	// 行番号を得る
	int getLineNumber() const;

	// 一字を取得する
	ModUnicodeChar get();
	// 一字を戻す
	void putback(const ModUnicodeChar);
	// 最終行か否かを返す
	bool isLastLine() const;

private:
	// 一行取得する
	ModUnicodeString* getLine();

	// 入力ファイル
	std::ifstream m_cFile;
	// 行を表すUnicode文字列へのポインタ
	ModUnicodeString* m_pLine;
	// 文字列中のカーソル
	ModUnicodeChar* m_pCursor;
	// 行番号
	int m_iLineNumber;
	// 最終行か否か
	bool m_bLastLine;
};

}

_SYDNEY_END

#endif //__SYDNEY_SYDTEST_FILEBUFFER_H

//
//	Copyright (c) 2001, 2003, 2004, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
