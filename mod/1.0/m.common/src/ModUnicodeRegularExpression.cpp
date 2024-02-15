// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModUnicodeRegularExpression.cpp -- ModUnicodeRegularExpression のメンバ定義 Unicode版
// 
// Copyright (c) 2000, 2002, 2004, 2011, 2023 Ricoh Company, Ltd.
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
//		$Revision
//


extern "C" {
#include <stdlib.h>						// free
#include "rx.h"							// rxライブラリ
}
#include "ModCommon.h"
#include "ModCommonMutex.h"
#include "ModDefaultManager.h"
#include "ModUnicodeRegularExpression.h"

//
// FUNCTION
// ModUnicodeRegularExpression::step -- 文字列と照合する
//
// NOTES
// この関数は設定されている正規表現が、与えられた文字列にマッチする部
// 分があるかどうかを調べるのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar* text
//		照合する文字列
// const ModSize length
//		text のうち照合する部分の長さ. この引数を省略するか 0 を指定す
//		るとtext の全体を照合する。
//
// RETURN VALUE
// 照合した場合 1 を返し、照合しなかった場合 0 を返す。
//
// EXCEPTIONS
// ModManager::allocate で発生する例外をそのまま送出する
//

int
ModUnicodeRegularExpression::step(
	const ModUnicodeChar* text, const ModSize length)
{
	if (this->handle == 0)
		return 0;

	if (this->matchee == 0) {
		// mallocする
		this->initialize();
	} else {
		rxMatcheeFree(this->matchee);
	}
		
	return rxStep(this->handle, 0, text,
				  (length == 0) ? RX_NULL_TERMINATE : (int) length,
				  this->matchee);
}

//
// FUNCTION
// ModUnicodeRegularExpression::advance -- 先頭から照合する
//
// NOTES
// この関数は設定されている正規表現が、与えられた文字列の先頭に対して
// マッチするかどうかを調べるのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar* text
//		照合する文字列
// const ModSize length
//		text のうち照合する部分の長さ. この引数を省略するか 0 を指定す
//		るとtext の全体を照合する。
//
// RETURN VALUE
// 照合した場合 1 を返し、照合しなかった場合 0 を返す。
//
// EXCEPTIONS
// なし
//

int
ModUnicodeRegularExpression::advance(const ModUnicodeChar* text, const ModSize length)
{
	if (this->handle == 0)
		return 0;

	if (this->matchee == 0) {
		// mallocする
		this->initialize();
	} else {
		rxMatcheeFree(this->matchee);
	}

	return rxAdvance(this->handle, 0, text,
					 (length == 0) ? RX_NULL_TERMINATE : (int) length,
					 this->matchee);
}

//
// FUNCTION
// ModUnicodeRegularExpression::walk -- 文字列全体をすべて照合する
//
// NOTES
// この関数は設定されている正規表現が、与えられた文字列に対して
// マッチする部分をすべて調べるのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar* text
//		照合する文字列
// const ModSize length
//		text のうち照合する部分の長さ. この引数を省略するか 0 を指定す
//		るとtext の全体を照合する。
//
// RETURN VALUE
// マッチした部分の個数を返す。
//
// EXCEPTIONS
// なし
//

int
ModUnicodeRegularExpression::walk(
	const ModUnicodeChar* text, const ModSize length)
{
	if (this->handle == 0)
		return 0;

	if (this->walkMatchee != 0) {
		for (int i = 0; i < this->numberOfMatchee; i++) {
			rxMatcheeFree(&(this->walkMatchee[i]));
		}
		::free(this->walkMatchee);
		this->walkMatchee = 0;
	}

	return this->numberOfMatchee =
		rxWalk(this->handle, 0, text,
			   (length == 0) ? RX_NULL_TERMINATE : (int) length,
			   &(this->walkMatchee));
}

//
// FUNCTION public
// ModUnicodeRegularExpression::matchBegin -- 直前の照合でマッチした部分を得る
//
// NOTES
// この関数は直前の照合でマッチした部分の先頭を指すポインタを得るのに
// 用いる。
//
// ARGUMENTS
// なし
//
// RETURN VALUE
// 直前の照合でマッチしていればマッチした部分の先頭を指すポインタを返す。
// マッチしていない場合は 0 を返す。
//
// EXCEPTIONS
// なし
//

ModUnicodeChar*
ModUnicodeRegularExpression::matchBegin()
{
	return this->matchee->stloc;
}

//
// FUNCTION public
// ModUnicodeRegularExpression::matchEnd -- 直前の照合でマッチした部分を得る
//
// NOTES
// この関数は直前の照合でマッチした部分の最後尾を指すポインタを得るの
// に用いる。
//
// ARGUMENTS
// なし
//
// RETURN VALUE
// 直前の照合でマッチしていればマッチした部分の最後尾を指すポインタを返す。
// マッチしていない場合は 0 を返す。
//
// EXCEPTIONS
// なし
//

ModUnicodeChar*
ModUnicodeRegularExpression::matchEnd()
{
	return this->matchee->edloc;
}

//
// FUNCTION
// ModUnicodeRegularExpression::matchBegin -- 直前の照合でマッチした部分を得る
//
// NOTES
// この関数は直前の照合で副表現があったとき、副表現にマッチした部分の
// 先頭を指すポインタを得るのに用いる。
//
// ARGUMENTS
// const int number
//		何番めの副表現にマッチしたものかを表す整数。
//
// RETURN VALUE
// 直前の照合で番号に該当する副表現がマッチしていればマッチした部分の
// 先頭を指すポインタを返す。マッチしていない場合は 0 を返す。
//
// EXCEPTIONS
// なし
//

ModUnicodeChar*
ModUnicodeRegularExpression::matchBegin(const int number)
{
	return (number == 0) ? this->matchBegin() :
		(this->matchee->nbras < 1 ||
		 number > this->matchee->bras[0].nbra) ? 0 :
		this->matchee->bras[0].braslist[number - 1];
}

//
// FUNCTION
// ModUnicodeRegularExpression::matchEnd -- 直前の照合でマッチした部分を得る
//
// NOTES
// この関数は直前の照合で副表現があったとき、副表現にマッチした部分の
// 最後尾を指すポインタを得るのに用いる。
//
// ARGUMENTS
// int number
//		何番めの副表現にマッチしたものかを表す整数。
//
// RETURN VALUE
// 直前の照合で番号に該当する副表現がマッチしていればマッチした部分の
// 最後尾を指すポインタを返す。マッチしていない場合は 0 を返す。
//
// EXCEPTIONS
// なし
//

ModUnicodeChar*
ModUnicodeRegularExpression::matchEnd(const int number)
{
	return (number == 0) ? this->matchEnd() :
		(this->matchee->nbras < 1 ||
		 number > this->matchee->bras[0].nbra) ? 0 :
		this->matchee->bras[0].braelist[number - 1];
}

//
// FUNCTION
// ModUnicodeRegularExpression::matchBegin -- 直前の照合でマッチした部分を得る
//
// NOTES
// この関数は直前のwalkによる照合で指定数番めにマッチした部分の
// 先頭を指すポインタを得るのに用いる。
//
// ARGUMENTS
// const int matcheeNumber
//		何番めにマッチした部分かを指定する整数。0 が最初のmatcheeを指す。
// const int number
//		何番めの副表現にマッチしたものかを表す整数。0 ならば全体。
//		1 が最初の副表現を指す。
//
// RETURN VALUE
// 直前のwalkによる照合で第一引数に該当するmatcheeがあれば、
// 第二引数に該当する副表現にマッチした部分の先頭を指すポインタを返す。
// matcheeの数が第一引数に示す数より少ないか、マッチしていない場合は 0 を返す。
//
// EXCEPTIONS
// なし
//

ModUnicodeChar*
ModUnicodeRegularExpression::matchBegin(
	const int matcheeNumber, const int number)
{
	return (matcheeNumber > this->numberOfMatchee) ? 0 :
		(number == 0) ? this->walkMatchee[matcheeNumber].stloc :
		(this->walkMatchee[matcheeNumber].nbras < 1 ||
		 number > this->walkMatchee[matcheeNumber].bras[0].nbra) ? 0 :
		this->walkMatchee[matcheeNumber].bras[0].braslist[number - 1];
}

//
// FUNCTION
// ModUnicodeRegularExpression::matchEnd -- 直前の照合でマッチした部分を得る
//
// NOTES
// この関数は直前のwalkによる照合で指定数番めにマッチした部分の
// 最後尾を指すポインタを得るのに用いる。
//
// ARGUMENTS
// const int matcheeNumber
//		何番めにマッチした部分かを指定する整数。0 が最初のmatcheeを指す。
// const int number
//		何番めの副表現にマッチしたものかを表す整数。0 ならば全体。
//		1 が最初の副表現を指す。
//
// RETURN VALUE
// 直前のwalkによる照合で第一引数に該当するmatcheeがあれば、
// 第二引数に該当する副表現にマッチした部分の最後尾を指すポインタを返す。
// matcheeの数が第一引数に示す数より少ないか、マッチしていない場合は 0 を返す。
//
// EXCEPTIONS
// なし
//

ModUnicodeChar*
ModUnicodeRegularExpression::matchEnd(const int matcheeNumber, const int number)
{
	return (matcheeNumber > this->numberOfMatchee) ? 0 :
		(number == 0) ? this->walkMatchee[matcheeNumber].edloc :
		(this->walkMatchee[matcheeNumber].nbras < 1 ||
		 number > this->walkMatchee[matcheeNumber].bras[0].nbra) ? 0 :
		this->walkMatchee[matcheeNumber].bras[0].braelist[number - 1];
}

//
// FUNCTION
// ModUnicodeRegularExpression::matchPID -- 照合した部分パターンIDを得る
//
// NOTES
// この関数は直前のwalkによる照合で指定数番めにマッチした部分の
// 部分パターンIDを得るのに用いる。
//
// ARGUMENTS
// const int matcheeNumber
//		何番めにマッチした部分かを指定する整数。0 が最初のmatcheeを指す。
//
// RETURN VALUE
// 直前のwalkによる照合で引数に該当するmatcheeがあれば、
// 照合した部分パターンIDを返す。
// matcheeの数が引数に示す数より少ない場合は 0 を返す。
//
// EXCEPTIONS
// なし
//

int
ModUnicodeRegularExpression::matchPID(const int matcheeNumber)
{
	return (matcheeNumber > this->numberOfMatchee) ? 0 :
		this->walkMatchee[matcheeNumber].expid;
}

//
// FUNCTION
// ModUnicodeRegularExpression::initialize -- 初期化を行なう
//
// NOTES
// この関数は正規表現モジュールの初期化を行なうために用いる。
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
void
ModUnicodeRegularExpression::initialize()
{
	// mallocする
	this->matchee = (struct _rxMatchee*)
		ModDefaultManager::allocate(sizeof(struct _rxMatchee));
	this->matchee->stloc = 0;
	this->matchee->edloc = 0;
	this->matchee->expid = 0;
	this->matchee->nbras = 0;
	this->matchee->bras = 0;
}

//
// FUNCTION
// ModUnicodeRegularExpression::terminate -- 終了処理を行なう
//
// NOTES
// この関数は正規表現モジュールの終了処理を行なうために用いる。
//
// ARGUMENTS
// なし
//
// RETURN VALUE
// なし
//
// EXCEPTIONS
// ModManager::free の生成する例外をそのまま送出する
//
void
ModUnicodeRegularExpression::terminate()
{
	rxMatcheeFree(this->matchee);
	if (this->walkMatchee != 0) {
		for (int i = 0; i < this->numberOfMatchee; i++) {
			rxMatcheeFree(&(this->walkMatchee[i]));
		}
		::free(this->walkMatchee);
		this->walkMatchee = 0;
	}
	rxFree(this->handle);
	this->handle = 0;
	ModDefaultManager::free(this->matchee, sizeof(struct _rxMatchee));
	this->matchee = 0;
}

//
// FUNCTION private
// ModUnicodeRegularExpression::doCompile -- compile の下請け関数
//
// NOTES
// この関数は compile やパターン付コンストラクタで実際に正規表現の
// コンパイルを行なうための下請け関数である。
//
// ARGUMENTS
// const ModUnicodeChar* pattern
//		コンパイルするパターン
//
// RETURN VALUE
// なし
//
// EXCEPTIONS
// ModCommonErrorBadArgument	rxがコンパイルに失敗した
//
void
ModUnicodeRegularExpression::doCompile(const ModUnicodeChar* pattern)
{
	if (this->handle != 0) {
		rxFree(this->handle);
		this->handle = 0;
	}
	this->handle = rxCompile(pattern);
	if (this->handle == 0) {
		ModThrow(ModModuleStandard, ModCommonErrorBadArgument,
				 ModErrorLevelError);
	}
}

//
// Copyright (c) 2000, 2002, 2004, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
