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

#include "UnicodeDataRow.h"

UnicodeDataRow::UnicodeDataRow(const char*	line,
							   const int	fieldNum)
	: d_fieldNum(fieldNum),
	  d_fieldArray(0)
{
	int i;

	// 領域確保と初期化
	d_fieldArray = new char*[d_fieldNum];
	for (i = 0; i < d_fieldNum; ++i) d_fieldArray[i] = 0;

	// 行に含まれる文字数を取得
	const int len = (int)strlen(line);

	//
	// 変数を初期化
	//


	int		fieldCount	= 0;	// 読み込んだフィールドの総数
	int		fieldBegin	= 0;	// 読み込み中のフィールドの開始位置
	int		fieldWidth	= 0;	// 読み込み中のフィールドの文字数

	for (i = 0; i < len; ++i) {

		if (line[i] == ';' || line[i] == '\0' || line[i] == '\n' || line[i] == 0x0d) {
			// 区切り文字が出現したらフィールドをコピー
			//
			// (定義ファイルに ^M が含まれていたので(Windows でダウンロード
			// したから？)区切り文字として 0x0d を追加した)
			//
			char* ptr = new char[fieldWidth + 1];
			strncpy(ptr, line + fieldBegin, fieldWidth);
			ptr[fieldWidth] = '\0';

			// フィールド配列に追加
			d_fieldArray[fieldCount] = ptr;

			// 変数を更新
			++fieldCount;
			fieldBegin = i + 1;	// ';' を飛ばすために加算
			fieldWidth = 0;

			if (fieldCount >= d_fieldNum) {
				// 必要なだけフィールドを読み込んだ
				break;
			}
		} else if (line[i] == ' ' && fieldWidth == 0) {
			// フィールド値の前にある空白は除去
			// (文字列フィールドの場合は文字列中に空白があるかもしれない)

			// 変数を更新
			// fieldCount;	// 更新不要
			++fieldBegin;
			// fieldWidth;	// 更新不要	
		} else {
			// 変数を更新
			// fieldCount;	// 更新不要
			// fieldBegin;	// 更新不要	
			++fieldWidth;
		}
	}	
}

UnicodeDataRow::~UnicodeDataRow()
{
	; assert(d_fieldArray != 0);
	for (int i = 0; i < d_fieldNum; ++i) {
		delete [] d_fieldArray[i];
		d_fieldArray[i] = 0;
	}

	delete [] d_fieldArray;	
}
