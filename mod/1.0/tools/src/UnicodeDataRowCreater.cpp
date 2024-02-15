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

#include "UnicodeDataRowCreater.h"

#include <iostream>
#include <assert.h>

// 各種データ行のヘッダファイル
#include "UnicodeDataRowUnicodeData.h"
#include "UnicodeDataRowBlocks.h"

using namespace std;

// 任意のデータ行の取得
//
// type		---	行データの種類
//
// buffer	---	ファイルから読んだ1行分の文字列
//
UnicodeDataRow*
UnicodeDataRowCreater::create(const UnicodeDataRowTypes::Type	type,
							  const char*						buffer)
{
	switch (type) {
	case UnicodeDataRowTypes::UnicodeData:
		return new UnicodeDataRowUnicodeData(buffer);
		break;

	case UnicodeDataRowTypes::Blocks:
		return new UnicodeDataRowBlocks(buffer);
		break;

	default:
		cerr << "[UnicodeDataRowCreater::create] unknown type" << endl;
		; assert(0);
		throw 1;
	}
}
