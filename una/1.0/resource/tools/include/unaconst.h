//
// unaconst.h - 辞書ビルドツールの共通定数
// 
// Copyright (c) 2014, 2023 Ricoh Company, Ltd.
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

#ifndef	UNACONST_H
#define	UNACONST_H

#define MAX_REC_NO	1000000		// 形態素数
#define MAX_LST_NO	150000		// 形態素の下位構造リスト用配列の大きさ
#define MAX_DA		6000000		// RevuzのTrie用配列サイズ
#define MAX_HINSHI	3000		// 品詞数の最大(連語の品詞、素性含む)
#define MAX_STR_POOL	50000000	// 文字列プールのサイズ
#define MAX_APP_POOL	40000000	// アプリケーション情報の合計バイト数
#define AVE_WORD_LEN	4		// 形態素長の平均
#define STR_POOL_BLOCK	1000000		// 文字列プールのブロック確保単位
#define LINES_BLOCK	20000		// 行数のブロック確保単位
#define MAX_LINE_LEN	1024		// テキストデータ行の最大長
#define MAX_NAME_LEN	128		// 名前の最大長
#define MAX_TOKEN_CNT	10		// 1行あたりのトークン最大個数

#endif /* UNACONST_H */

//--------------------------------------------------------------------------
// Copyright (c) 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
