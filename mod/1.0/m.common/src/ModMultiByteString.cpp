// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModMultiByteString.cpp -- ModMultiByteString のメンバ定義
// 
// Copyright (c) 2000, 2009, 2023 Ricoh Company, Ltd.
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

#include <iostream>

#include "ModMultiByteString.h"
#include "ModOsDriver.h"	// メモリアロケートのため
#include "ModKanjiCode.h"	// ModKanjiCode::KanjiCodeType のため
#include "ModDefaultManager.h"

using namespace std;		// iostream

//
// COMMON が初期化されているか否かを表す。初期化が完了するまでアロケート
// してはいけない。
//
ModBoolean
ModMultiByteString::s_CommonIsInitialized = ModFalse;

//
// FUNCTION private
// ModMultiByteString::ModMultiByteString() -- マルチバイト文字列
//
// NOTES
// マルチバイト文字列のコンストラクタ
//
// ARGUMENTS
// char* src
//	変換前の文字列。
// ModKanjiCode::KanjiCodeType srcKanjiCode
//	src の漢字コード
// ModKanjiCode::KanjiCodeType targetKanjiCode
//	このマルチバイト文字列クラスが内部に持つバッファの文字コード。
//	内部のバッファは getString で取得できる。
// ModBoolean					useAllocatedMemory
//	メモリのアロケートを行なって良い場合は ModTrue を渡す。アロケートは
//	唯一例外を送出する可能性のある処理である。この引数を ModFalse にすると
//	static な領域を使うのでエラーは絶対に起きなくなる。しかし、static な
//	領域は排他制御していないので呼出側が排他制御を行なうこと。
//
// RETURN
// なし
//
// EXCEPTIONS
// ModStandardManager::allocate を参照
//
ModMultiByteString::ModMultiByteString(
	const char*							src_,
	const ModKanjiCode::KanjiCodeType	srcKanjiCode_,
	const ModKanjiCode::KanjiCodeType	bufferKanjiCode_,
	const ModBoolean					useAllocatedMemory_ /* = ModFalse */)
	: d_size(0),
	  d_buffer(0)
{
	if (src_ == 0) {
		return;
	}

	ModKanjiCode::KanjiCodeType	srcCode = srcKanjiCode_;
	ModKanjiCode::KanjiCodeType	dstCode = bufferKanjiCode_;

	// 変なコードを渡されてた場合は "unknown" に変更する
	if (srcCode > ModKanjiCode::unknown) {
		srcCode = ModKanjiCode::unknown;
	}
	if (dstCode > ModKanjiCode::unknown) {
		dstCode = ModKanjiCode::unknown;
	}

	if (srcCode == dstCode
		|| srcCode == ModKanjiCode::unknown
		|| dstCode == ModKanjiCode::unknown) {

		//
		// 渡された文字コードをそのまま内部バッファに収める
		// (変換不能なので、変換しない)

		const char*	tmpSrc = 0;
		char*		tmpDst = 0;

		// バッファサイズを計算
		; ModAssert(d_size == 0);
		for (tmpSrc = src_; *tmpSrc != '\0'; ++tmpSrc, ++d_size) ;
		++d_size;	// 終端文字のために1バイト追加

		// バッファを確保。必要ならばバッファサイズを更新
		if (useAllocatedMemory_ && s_CommonIsInitialized) {
			d_buffer = (char*)ModDefaultManager::allocate(d_size);
		} else {
			d_buffer = s_staticBuffer;
			if (d_size > s_staticSize) {
				// 内部の静的バッファからはみ出さないようにする
				d_size = s_staticSize;
			}
		}

		// バッファにコピー
		for (tmpSrc = src_, tmpDst = d_buffer;
			 *tmpSrc != '\0' && tmpDst < d_buffer + d_size;
			 ++tmpSrc, ++tmpDst) {
			*tmpDst = *tmpSrc;
		}
		*tmpDst = '\0';
	} else {

		//
	   	// アロケートする領域のサイズ ＝ 変換後の文字列に必要なバイト数
		//								 	＋ 終端文字(UCS2 の場合も考えて
		//                                     ４バイトとする)

		const ModSize bytes = getTransferredSize(dstCode, src_, srcCode);
		d_size = bytes + 2;

		// バッファを確保。必要ならばバッファサイズを更新
		if (useAllocatedMemory_ && s_CommonIsInitialized) {
			d_buffer = (char*)ModDefaultManager::allocate(d_size);
		} else {
			d_buffer = s_staticBuffer;
			if (d_size > s_staticSize) {
				// 内部の静的バッファからはみ出さないようにする
				d_size = s_staticSize;
			}
		}		

		// コードを変換する
		if (bytes > 0)
			transfer(d_buffer, d_size - 2, dstCode, src_, srcCode);

		// 終端文字を書き込む (UCS2 でも大丈夫なように２バイト)
		*(d_buffer + d_size - 1) = 0;
		*(d_buffer + d_size - 2) = 0;
	}
}

ModMultiByteString::~ModMultiByteString()
{
	if (d_buffer != s_staticBuffer) {
		// アロケートした場合だけ解放する。コンストラクタの引数 src を
		// セットしただけの場合は解放してはいけない
		ModDefaultManager::free(d_buffer, d_size);
	}
	d_buffer	= 0;
	d_size		= 0;
}

const char*
ModMultiByteString::get() const
{
	return d_buffer;
}

void
ModMultiByteString::setCommonIsInitialized()
{
	s_CommonIsInitialized = ModTrue;
}

//
// PRIVATE
//

//
// FUNCTION public
// ModMultiByteString::transfer -- jj ライブラリを使ってコード変換
//
// NOTES
//	jj ライブラリを使ってコード変換
//
// ARGUMENTS
// 
// RETURN
//
// EXCEPTIONS
//	なし
//
char*
ModMultiByteString::transfer(char*								outString,
							 const int							outBytes,
							 const ModKanjiCode::KanjiCodeType	outType,
							 const char*						inString,
							 const ModKanjiCode::KanjiCodeType	inType)
{
	return ModKanjiCode::jjTransfer(outString,
									outBytes,
									outType,
									inString,
									inType);
}

//
// FUNCTION public
// ModMultiByteString::getTransferredSize -- jj ライブラリを使って各種漢字コードの変換後のバイト数を取得
//
// NOTES
// jj ライブラリを使って各種漢字コードの変換後のバイト数を取得
//
// ARGUMENTS
// const KanjiCodeType	outType
//		変換元文字列 inString のコード
// const char*			inString
//		変換元文字列。
//		char* 以外の型の文字列を渡す時はキャストが必要。
// const KanjiCodeType	inType
//		変換元文字列 inString のコード
//
// 
// RETURN
//
// EXCEPTIONS
//	なし
//
ModSize
ModMultiByteString::getTransferredSize(
	const ModKanjiCode::KanjiCodeType	outType,
	const char*							inString,
	const ModKanjiCode::KanjiCodeType	inType)
{
	return ModKanjiCode::jjGetTransferredSize(inString, inType, outType);
}

//
// Copyright (c) 2000, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
