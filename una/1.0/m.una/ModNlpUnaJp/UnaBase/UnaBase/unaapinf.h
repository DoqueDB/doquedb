//
// unaapinf.h -
//      アプリケーション辞書情報取得モジュール
//		形態素解析処理の中でアプリ情報を取得するためのモジュール
// 
// Copyright (c) 1998-2009, 2023 Ricoh Company, Ltd.
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

#ifndef UNAAPINF_H
#define UNAAPINF_H

//--------------------------------------------------------------------------
// 必要なヘッダの読み込み
//--------------------------------------------------------------------------

#include "una.h"		/* UNAグローバルなヘッダファイル */
#include "unamorph.h"	/* 形態素解析メインモジュール */

//--------------------------------------------------------------------------
// データ型の定義とマクロ定数
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// TAG:	  unaAppInfoT
//
// ABSTRACT:	  アプリケーション情報型
//
// NOTE:
//	  アプリケーション毎に、固有の形態素情報を持つことができる。
//	  ただし、その情報はあくまでアプリケーションによって決められるものなので
//	  実際の情報の構造がどのようなものであるかは分からない。
//	  (例えば複雑な構造体とか)
//	  そこで、アプリケーション情報型を定義し、ここへのポインタという形で
//	  アプリケーション情報を表わす。
//	  実際の単語情報は、infoのアドレスから始まり、lenのバイト数分の長さを持つ
//	  可変長バイナリデータである。アプリケーション側で適当な型にキャストして
//	  使用する。
//
typedef struct unaAppInfoT{		// アプリケーション情報型
	int len;					/* 長さ(バイト数) */
	char info[1];				/* 実際は可変長のデータ */
}unaAppInfoT;

//--------------------------------------------------------------------------
// TAG:	  unaApInfHandleT
//
// ABSTRACT:	  ハンドラ型
//
// NOTE:
//	  アプリ情報取得モジュールをマルチスレッドセーフで実行するためのハンドラ
//
typedef struct unaApInfHandleT {	// アプリ情報取得モジュール用ハンドル */
 	const char* appInfoImg;  		/* アプリ辞書のイメージ */
 	uintT recCount;					/* アプリ辞書(登録されてるアプリ情報の数) */
 	const uintT* appInfoOffset;		/* アプリ辞書(プールへのオフセット情報) */
 	const char* appInfoPool;		/* アプリ辞書(アプリ情報のプール) */
}	unaApInfHandleT;

//--------------------------------------------------------------------------
// 関数のプロトタイプ宣言とマクロ関数
//--------------------------------------------------------------------------

/* 初期化処理 */
int unaAppInfo_init(unaApInfHandleT *ah,const char *appInfoImg);
/* 終了処理 */
int unaAppInfo_term(unaApInfHandleT *ah);
/* アプリ情報取得 */
int unaAppInfo_get(const unaMorphT *morph,unaAppInfoT **appInf,
		const unaApInfHandleT *ah);

#endif /* end of UNAAPINF_H */

//--------------------------------------------------------------------------
// Copyright (c) 1998-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
