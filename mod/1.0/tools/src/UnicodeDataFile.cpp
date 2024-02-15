// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
//
// Unicode 文字情報ファイルのクラス
// 
// Copyright (c) 2023 Ricoh Company, Ltd.
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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <iostream>

#include "UnicodeDataFile.h"
#include "UnicodeDataRowCreater.h"

using namespace std;

UnicodeDataFile::UnicodeDataFile(const char*						filename,
								 const UnicodeDataRowTypes::Type	rowType)
	: d_fp(0), d_rowType(rowType)
{
	d_fp = fopen(filename, "r");
	if (d_fp == 0) {
		cerr << "can not open file (" << filename << ")" << endl;
		; assert(0);
		throw 1;
	}

	; assert(d_fp != 0);
}

UnicodeDataFile::~UnicodeDataFile()
{
	if (d_fp != 0) {
		fclose(d_fp);
		d_fp = 0;
	}
}

//
// 次の行に対応する DataRow クラス(の派生クラス)を返す。
// ただし、'#' から始まる文字列は無視する
//
UnicodeDataRow*
UnicodeDataFile::getNextRow()
{
	if (d_fp == 0) {
		// ファイルの内容は全て読んでしまった
		return 0;
	}

	const int bufferSize = 256;
	char buffer[bufferSize];

	memset(buffer, '\0', bufferSize);

	char* ret = 0;
	while ((ret = fgets(buffer, bufferSize, d_fp)) != 0
		   && (ret[0] == '#'|| ret[0] == '\n'))
		; // コメントや改行は無視する

	if (ret == NULL) {
		// ファイルの内容を全て読んだ
		fclose(d_fp);
		d_fp = 0;
		return 0;	// !! 終了 !!
	}
	
	if (buffer[bufferSize - 2] != '\0') {
		// fgets は賢いので、バッファに収まらなかった時は buffer-1 に
		// 終端文字をセットする
		cerr << "buffer overflow (buffer size = " << bufferSize << ")" << endl;
		cerr << endl;
		cerr << "buffer =  " << buffer << endl;
		cerr << endl;
		; assert(0);
		throw 1;
	}

	return UnicodeDataRowCreater::create(d_rowType, buffer);
}
