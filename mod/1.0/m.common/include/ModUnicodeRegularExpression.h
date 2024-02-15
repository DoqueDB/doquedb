// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModUnicodeRegularExpression.h -- ModUnicodeRegularExpression のクラス定義
// 
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModUnicodeRegularExpression_H__
#define __ModUnicodeRegularExpression_H__

#include "ModCommonDLL.h"
#include "ModCommon.h"

struct _rxHandle;
struct _rxMatchee;

//
// CLASS
// ModUnicodeRegularExpression -- 正規表現を扱うクラス
//
// NOTES
// このクラスは正規表現に関する操作を行なうためのクラスである。
//

class ModCommonDLL ModUnicodeRegularExpression
{
public:
	// デフォルトコンストラクタ
	ModUnicodeRegularExpression();
	// コンストラクタ(1)
	ModUnicodeRegularExpression(const ModUnicodeChar* pattern);
	// デストラクタ
	~ModUnicodeRegularExpression();

	// 正規表現をコンパイルする
	void compile(const ModUnicodeChar* pattern);
	// 文字列と照合する
	int step(const ModUnicodeChar* text, const ModSize length = 0);
	// 先頭から照合する
	int advance(const ModUnicodeChar* text, const ModSize length = 0);
	// 文字列全体と照合する
	int walk(const ModUnicodeChar* text, const ModSize length = 0);

	// 直前の照合でマッチした部分を得る
	ModUnicodeChar* matchBegin();			// マッチ部分の先頭
	ModUnicodeChar* matchEnd();					// マッチ部分の最後の次

	ModUnicodeChar* matchBegin(const int number);	// 副表現のマッチ部分の先頭
	ModUnicodeChar* matchEnd(const int number);	// 副表現のマッチ部分の次
	ModUnicodeChar* matchBegin(const int matcheeNumber,
					 const int number);	// 副表現のマッチ部分の先頭
										// walkでmatcheeが複数あるときに
										// matcheeを指定する
	ModUnicodeChar* matchEnd(const int matcheeNumber,
				   const int number);	// 副表現のマッチ部分の次
										// walkでmatcheeが複数あるときに
										// matcheeを指定する
	// 照合した部分パターンIDを得る
	int matchPID(const int matcheeNumber);

private:
	// コピーコンストラクタ -- 定義だけして実装しない
	ModUnicodeRegularExpression(const ModUnicodeRegularExpression& original);
	// 代入演算子 -- 定義だけして実装しない
	ModUnicodeRegularExpression& operator=(
				const ModUnicodeRegularExpression& original);

	void initialize();
	void terminate();
	void doCompile(const ModUnicodeChar* pattern);

	// rxライブラリで使う構造体
	struct _rxHandle* handle;
	struct _rxMatchee* matchee;
	struct _rxMatchee* walkMatchee;
	int numberOfMatchee;
};

//
// FUNCTION
// ModUnicodeRegularExpression::ModUnicodeRegularExpression -- デフォルトコンストラクタ
//
// NOTES
// ModUnicodeRegularExpressionのデフォルトコンストラクタ。rx の初期化のみ行ない
// compile を実行しない限り文字列の照合はできない。
//
// ARGUMENTS
// なし
//
// RETURN VALUE
// なし
//
// EXCEPTIONS
// ModManager::allocate の生成する例外をそのまま送出する
// 

inline
ModUnicodeRegularExpression::ModUnicodeRegularExpression()
	: handle(0), matchee(0), walkMatchee(0), numberOfMatchee(0)
{
	this->initialize();
}

//
// FUNCTION
// ModUnicodeRegularExpression::ModUnicodeRegularExpression -- パターン付コンストラクタ
//
// NOTES
// ModUnicodeRegularExpressionのパターンを指定したコンストラクタ。
// rx の初期化を行ない、引数のパターンをコンパイルする。
//
// ARGUMENTS
// const ModUnicodeChar* pattern
//		正規表現
//
// RETURN VALUE
// なし
//
// EXCEPTIONS
// ModManager::allocate の生成する例外をそのまま送出する
// ModCommonErrorBadArgument	rxがコンパイルに失敗した
// 

inline
ModUnicodeRegularExpression::ModUnicodeRegularExpression(
	const ModUnicodeChar* pattern)
	: handle(0), matchee(0), walkMatchee(0), numberOfMatchee(0)
{
	this->initialize();
	this->doCompile(pattern);
}

//
// FUNCTION
// ModUnicodeRegularExpression::~ModUnicodeRegularExpression -- デストラクタ
//
// NOTES
// ModUnicodeRegularExpression のデストラクタ
//
// ARGUMENTS
// なし
//
// RETURN VALUE
// なし
//
// EXCEPTIONS
// ModManager::freeの生成する例外をそのまま送出する
//

inline
ModUnicodeRegularExpression::~ModUnicodeRegularExpression()
{
	this->terminate();
}

//
// FUNCTION public
// ModUnicodeRegularExpression::compile -- 正規表現をコンパイルする
//
// NOTES
// この関数は ModUnicodeRegularExpression クラスに対して新たな正規表現を
// 設定するために用いる。
//
// ARGUMENTS
// const ModUnicodeChar* pattern
//
// RETURN VALUE
// なし
//
// EXCEPTIONS
// ModCommonErrorBadArgument rxライブラリーがcompileに失敗した
//

inline
void
ModUnicodeRegularExpression::compile(const ModUnicodeChar* pattern)
{
	this->doCompile(pattern);
}

#endif	// __ModUnicodeRegularExpression_H__

//
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
