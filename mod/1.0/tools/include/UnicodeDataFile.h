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

#include "UnicodeDataRowTypes.h"

#include <stdio.h>

class UnicodeDataRow;

//
// いろんな定義が書いてあるファイル(情報は;で区切られている)
//
class UnicodeDataFile
{
public:
	UnicodeDataFile(const char*						filename,
					const UnicodeDataRowTypes::Type	rowType);
	~UnicodeDataFile();

	//
	// マニピュレータ
	//

    // データファイルの先頭行から順番に返していく。
	// 文字コードが連続でない場合があるので利用者は注意しましょう。
    UnicodeDataRow* getNextRow();

	//
	// アクセッサ
	//

	// まだファイルに読み残しがあれば true を返す
	// (C++ では、比較結果を bool にキャストしなきゃいけないの？)
	bool isEmpty() const { return bool(d_fp == 0); }

private:
	// データソース固有のメンバ変数
	FILE*						d_fp;		// 情報ファイル(これが0でなければ
											// 読み残しの情報が存在する)

	const UnicodeDataRowTypes::Type	d_rowType;
};
