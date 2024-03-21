// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ModUnicodeOstrStream.h -- 出力文字列ストリーム関連のUnicode版クラス定義
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

#ifndef	__ModUnicodeOstrStream_H__
#define __ModUnicodeOstrStream_H__

#include "ModCommonDLL.h"
#include "ModCommon.h"
#include "ModDefaultManager.h"
#include "ModOstream.h"
#include "ModStrStreamBuffer.h"
#include "ModUnicodeChar.h"
#include "ModUnicodeCharTrait.h"

class ModCharString;
class ModUnicodeString;

//	CLASS
//	ModUnicodeOstrStream -- 出力ストリームを表すクラス
//
//	NOTES
//		ostrstream の代用クラス

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModUnicodeOstrStream
	: public	ModOstream
{
public:

	//
	// コンストラクタ、デストラクタ
	//
	ModUnicodeOstrStream();
	ModUnicodeOstrStream(ModUnicodeChar*	buf_,
						 const ModSize		length_,
						 const int 			mode_ = ModIos::output);
	~ModUnicodeOstrStream();

	//
	// アクセッサ
	//

	// 空かどうか
	ModBoolean			isEmpty() const;

	// 出力内容を格納したバッファを取得。
	// マニピュレータを呼び出しても取得したバッファの内容は保護されるが、
	// getString をもう一度呼び出すと以前のバッファは自動的に破棄される。
	ModCommonDLL
	ModUnicodeChar*		getString();

	//
	// マニピュレータ
	//

	// 文字を追加 (ストリームの必須関数)
	ModCommonDLL
	ModOstream&			put(const char			character_);
	ModCommonDLL
	ModOstream&			put(const ModUnicodeChar character_);

	// 文字列を追加 (ストリームの必須関数)
	ModCommonDLL
	ModOstream&			write(const char* string_);
	ModCommonDLL
	ModOstream&			write(const ModUnicodeChar* string_);

	// バッファを空にする
	void				clear();

private:

	ModStrStreamBuffer<ModUnicodeChar> _strStream;		// 文字列ストリーム
	ModUnicodeChar*				_buffer;		// 出力文字列を格納する領域
	ModSize						_size;			// この領域のサイズ(B 単位)

	// 入力マルチバイト文字を文字コード変換した結果の一時格納場所
	ModUnicodeChar*				_workBuffer;
	ModSize						_workBufferSize;
	static const ModSize		_allocateStep;
};

//
// コンストラクタ、デストラクタ
//

//	FUNCTION public
//	ModUnicodeOstrStream::ModUnicodeOstrStream --
//		出力ストリームを表すクラスのデフォルトコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
ModUnicodeOstrStream::ModUnicodeOstrStream()
	: _buffer(0),
	  _size(0),
	  _workBuffer(0),
	  _workBufferSize(0)
{ }

//	FUNCTION public
//	ModUnicodeOstrStream::ModUnicodeOstrStream --
//		指定されたバッファを使用する出力ストリームのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeChar*		buf_
//			出力ストリームが使用する文字列ストリームが出力文字列を
//			格納するために使用する領域の先頭アドレス
//		ModSize				length_
//			0 以外の値
//				与えられた領域のサイズ(B 単位)
//			0
//				与えられた領域の先頭から
//				ModUnicodeCharTrait::null() の直前までの範囲を使用する
//		int					mode
//			ModIos::output または指定されないとき
//				出力内容は与えられたバッファ中の文字列を上書きする
//			ModIos::append
//				出力内容は与えられたバッファ中の文字列に追加される
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			出力内容を与えられたバッファに追加しようとしているが、
//			与えられたバッファには追加する空き領域がない

inline
ModUnicodeOstrStream::ModUnicodeOstrStream(ModUnicodeChar*	buf_,
										   const ModSize	length_,
										   const int		mode_)
	: _strStream(buf_, length_, buf_),
	  _buffer(0),
	  _size(0),
	  _workBuffer(0),
	  _workBufferSize(0)
{
	if (buf_ == 0) {
		ModThrow(ModModuleStandard,
					 ModCommonErrorBadArgument, ModErrorLevelError);
	}

	ModSize	start = 0;
	if (mode_ == ModIos::append) {
		start = ModUnicodeCharTrait::length(buf_);
		// 必要な文字数(start)と格納可能な文字数を比較する
		if (start > length_ / sizeof(ModUnicodeChar))

			// 与えられたバッファに格納されている文字列が
			// すでにバッファ長を超えているので、
			// append が指定されてももう追加できない

			ModThrow(ModModuleStandard,
					 ModCommonErrorBadArgument, ModErrorLevelError);
	}
	_strStream.seekPosition(start);
}

//	FUNCTION public
//	ModUnicodeOstrStream::~ModUnicodeOstrStream -- 出力ストリームを表すクラスのデストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
ModUnicodeOstrStream::~ModUnicodeOstrStream()
{
	if (_buffer)
		ModStandardManager::free(_buffer, _size), _buffer = 0;
	if (_workBuffer)
		ModStandardManager::free(_workBuffer, _workBufferSize), _workBuffer = 0;
}

//
// アクセッサ
//

//	FUNCTIONS public
//	ModUnicodeOstrStream::isEmpty -- 出力ストリームが空か調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModTrue
//			出力ストリームは空である
//		ModFalse
//			出力ストリームは空でない
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModUnicodeOstrStream::isEmpty() const
{
	return (_strStream.getSize()) ? ModFalse : ModTrue;
}

//
// マニピュレータ
//

//	FUNCTION public
//	ModUnicodeOstrStream::clear -- 出力ストリームを空にする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
ModUnicodeOstrStream::clear()
{
	_strStream.clear();
}

#endif	// __ModUnicodeOstrStream_H__

//
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
