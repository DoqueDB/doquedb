// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ModOstrStream.h -- 出力文字列ストリーム関連のクラス定義
// 
// Copyright (c) 1997, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModOstrStream_H__
#define __ModOstrStream_H__

#include "ModCommonDLL.h"
#include "ModCommon.h"
#include "ModDefaultManager.h"
#include "ModOstream.h"
#include "ModStrStreamBuffer.h"

class ModCharString;
class ModWideString;
class ModUnicodeString;

//	CLASS
//	ModOstrStream -- 出力ストリームを表すクラス
//
//	NOTES
//		ostrstream の代用クラス
//
//		他と同様に ModPure* を定義し、それを継承した
//		メモリーハンドルを明示するクラスを定義するには、
//		operator =() などを再定義し、copy() などのオブジェクトを返す
//		メソッドに対応する必要があるので、断念する

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModOstrStream
	: public	ModOstream
{
public:
	ModOstrStream();
	ModOstrStream(char* buf, ModSize length, int mode = output);
												// コンストラクター
	~ModOstrStream();							// デストラクター

	ModCommonDLL
	ModOstream&			put(const char v);
	ModCommonDLL
	ModOstream&			put(const ModUnicodeChar v);
	ModCommonDLL
	ModOstrStream&		put(const ModWideChar v);
												// 文字を追加

	ModCommonDLL
	ModOstream&			write(const char* v);
	ModCommonDLL
	ModOstream&			write(const ModUnicodeChar* v);
	ModCommonDLL
	ModOstrStream&		write(const ModWideChar* v);
												// 文字列を追加

	ModCommonDLL
	char*				getString();			// 出力内容を C 文字列で得る

	void				clear();				// 空にする

	ModBoolean			isEmpty() const;		// 空かどうか

private:
	ModStrStreamBuffer<char> _strStream;		// 文字列ストリーム
	char*				_buffer;				// 出力文字列を格納する領域
	ModSize				_size;					// この領域のサイズ(B 単位)
};

//	FUNCTION public
//	ModOstrStream::ModOstrStream --
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
ModOstrStream::ModOstrStream()
	: _buffer(0),
	  _size(0)
{ }

//	FUNCTION public
//	ModOstrStream::ModOstrStream --
//		指定されたバッファを使用する出力ストリームのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		char*				buf
//			出力ストリームが使用する文字列ストリームが出力文字列を
//			格納するために使用する領域の先頭アドレス
//		ModSize				length
//			0 以外の値
//				与えられた領域のサイズ(B 単位)
//			0
//				与えられた領域の先頭から
//				ModCharTrait::null() の直前までの範囲を使用する
//		int					mode
//			ModOstrStream::output または指定されないとき
//				出力内容は与えられたバッファ中の文字列を上書きする
//			ModOstrStream::append
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
ModOstrStream::ModOstrStream(char* buf, ModSize length, int mode)
	: _strStream(buf, length, buf),
	  _buffer(0),
	  _size(0)
{
	; ModAssert(buf != 0);

	ModSize	start = 0;
	if (mode == append) {
		start = ModCharTrait::length(buf);
		if (start > length)

			// 与えられたバッファに格納されている文字列が
			// すでにバッファ長を超えているので、
			// append が指定されてももう追加できない

			ModThrow(ModModuleStandard,
					 ModCommonErrorBadArgument, ModErrorLevelError);
	}
	_strStream.seekPosition(start);
}

//	FUNCTION public
//	ModOstrStream::~ModOstrStream -- 出力ストリームを表すクラスのデストラクター
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
ModOstrStream::~ModOstrStream()
{
	if (_buffer)
		ModStandardManager::free(_buffer, _size), _buffer = 0;
}

//	FUNCTION public
//	ModOstrStream::operator << -- << 演算子
//
//	NOTES
//		出力ストリームに与えられた値を追加する
//		(基底クラス ModOstream に無い operator<< だけ実装する)
//
//	ARGUMENTS
//		ModWideChar*		v
//			追加する ModWideChar*
//
//	RETURN
//		与えられた値を追加後の自分自身
//
//	EXCEPTIONS
#if 0
inline
ModOstream&
ModOstrStream::operator <<(const ModWideChar* v)
{
	return this->write(v);
}
#endif

//	FUNCTION public
//	ModOstrStream::clear -- 出力ストリームを空にする
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
ModOstrStream::clear()
{
	_strStream.clear();
}

//	FUNCTIONS public
//	ModOstrStream::isEmpty -- 出力ストリームが空か調べる
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
ModOstrStream::isEmpty() const
{
	return (_strStream.getSize()) ? ModFalse : ModTrue;
}

#endif	// __ModOstrStream_H__

//
// Copyright (c) 1997, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
