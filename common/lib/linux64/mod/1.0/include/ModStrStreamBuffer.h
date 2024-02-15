// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ModStrStreamBuffer.h -- 文字列を格納するストリームバッファ関連のクラス定義
// 
// Copyright (c) 1997, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModStrStreamBuffer_H__
#define __ModStrStreamBuffer_H__

#include "ModCommonDLL.h"
#include "ModCommon.h"
#include "ModDefaultManager.h"
#include "ModTrait.h"

static const int ModEof = -1;

//
// CLASS
// ModPureStrStreamBufferBase -- 自動的に領域の伸びるバッファ機能クラスの基底
//
// NOTES
// テンプレートクラスである ModPureStrStreamBufferPure の定数を宣言するための
// クラス。テンプレート化する前の ModStrStreamPure は ModDefaultManager を
// 継承していなかったので、ここでも継承していない。おそらく、new されることが
// 無いはずからだろう。
//
class ModCommonDLL ModPureStrStreamBufferBase
{
public:
	// seekOffset で指定するシーク方向
	enum SeekDirection {
		begin,							// 先頭から
		current,						// 現在位置から
		end								// 最後尾から
	};
	// seekXXX で指定する移動させるポインター
	// 現在は入力ポインタのみ指定可能、出力ポインターの定義は拡張性のため
	enum SeekMode {
		in,								// 入力ポインター
		out								// 出力ポインター(指定不可能)
	};

protected:
	static const ModSize defaultSize;
	static const ModSize defaultAllocateStep;

	enum {
		dynamic = 1,					// 拡張あり
		frozen = 2						// フリーズされている
	};
};

//
// CLASS
// ModPureStrStreamBuffer -- 自動的に領域の伸びるバッファ機能クラス
//
// NOTES
// strstreambuf に相当する機能クラス。
// 実際にはメモリハンドルを明示した型ModStrStreamBufferを使う。
// 最小限の機能しか提供していない。
//

template <class BufferType>
class ModPureStrStreamBuffer
	: public ModPureStrStreamBufferBase
{
public:
	// デフォルトコンストラクタ
	ModPureStrStreamBuffer();
	// コンストラクタ (1)	--- bufferSize : バッファに格納できる文字数
	ModPureStrStreamBuffer(ModSize bufferSize);
	// コンストラクタ (3)	--- allocateSize : バッファに格納できる文字数
	ModPureStrStreamBuffer(BufferType* buf, ModSize length, BufferType* start = 0,
						   ModSize allocateSize = 0 // yogawa の要望
						  );

	// デストラクタ
	~ModPureStrStreamBuffer();

	// 凍結設定/解除
	void freeze(ModBoolean freezeFlag = ModTrue);
	// 凍結中か
	ModBoolean isFrozen();

	// 凍結してバッファを指すポインタを返す
	BufferType* getString();

	int					putChar(BufferType c);		// 文字を加える
	ModSize				putString(const BufferType* s, ModSize len = 0);
												// 文字列を加える

	// 入力ポインタを移動させる関数(移動量の単位 : 文字数)
	ModSize seekOffset(ModOffset offset,
					   SeekDirection direction = current,
					   SeekMode mode = in);
	ModSize seekPosition(ModSize pos, SeekMode mode = in);

	// 入っている文字列の文字数を得る(終端文字は含まない)
	ModSize getSize() const;

	// バッファの最後尾を指すポインタを返す
	BufferType* getTail() const;

	// 中身を空にする(flushの動作？)
	void clear();

	// 領域を伸ばすステップ幅(単位:文字数)
	ModSize getAllocateStep() const;
	void setAllocateStep(ModSize v);

private:
	int overflow(BufferType character);

	// 現在のバッファの状態
	int status;
	// 領域の先頭
	BufferType* buffer;
	// 領域の最後の次: 値は [buffer, tail) の範囲
	BufferType* tail;

	//
	// 文字列の更新範囲を指すポインタ
	// start は更新の開始位置を指す。
	// currentTail は現在の文字列の更新される位置を指す。
	// updateLimit は更新が可能なバッファ上の限界を指し、
	// この場所を越えて更新することはできない。
	// バッファが動的モードのときは再確保される。
	//
	BufferType* start;				// 更新の開始位置
	BufferType* currentTail;		// 更新の現在位置
	BufferType* updateLimit;		// 更新の限界位置

	ModSize allocateStep;			// 領域を伸ばすときのステップ幅(文字数)
};

//
// FUNCTION
// ModPureStrStreamBuffer -- デフォルトコンストラクタ
//
// NOTES
// この関数は ModPureStrStreamBuffer のデフォルトコンストラクタである。
//
// ARGUMENTS
// なし
//
// RETURN
// EXCEPTIONS
// なし
//

template <class BufferType>
inline
ModPureStrStreamBuffer<BufferType>::ModPureStrStreamBuffer()
	: buffer(0),
	  tail(0),
	  status(dynamic),
	  start(0),
	  currentTail(0),
	  updateLimit(0),
	  allocateStep(defaultAllocateStep)
{ }

//
// FUNCTION
// ModPureStrStreamBuffer -- バイト数を指定したコンストラクタ
//
// NOTES
// この関数は ModPureStrStreamBuffer のバイト数を指定するコンストラクタである。
//
// ARGUMENTS
// ModSize size
//		バッファの領域として確保する文字数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
// その他
// ModStandardManager::allocate の例外参照
//

template <class BufferType>
inline
ModPureStrStreamBuffer<BufferType>::ModPureStrStreamBuffer(ModSize size)
	: buffer(0),
	  tail(0),
	  status(dynamic),
	  start(0),
	  currentTail(0),
	  updateLimit(0),
	  allocateStep(defaultAllocateStep)
{
	const ModSize bytes = size * sizeof(BufferType);
	this->buffer = (BufferType*) ModStandardManager::allocate(bytes);
	; ModAssert(this->buffer != 0);
	ModOsDriver::Memory::reset(this->buffer, bytes);

	this->tail = this->buffer + size;
	this->start = this->buffer;
	this->currentTail = this->buffer;
	this->updateLimit = this->tail;
}

//
// FUNCTION
// ModPureStrStreamBuffer -- 使用する領域を指定したコンストラクタ
//
// NOTES
// この関数は ModPureStrStreamBuffer の領域を指定するコンストラクタである。
// 指定したバッファ上に静的な、または動的な streambuffer を作成する。
//
// ARGUMENTS
// char* buf
//		バッファとして使う領域へのポインタ
// ModSize length_
//		buf 上で strstream として使う領域の文字数。
//		strat_ の値が 0 以外の場合はこの引数の値は無視されるのが、
//		本家 ssbuf の仕様のはずだが、例を見ると無視されないようだ。
//		0 を指定すると null-terminate しているとみなしてその最後尾までを
//		領域とする。
// char* start_
//		格納の開始位置。
//		この引数に 0 を指定すると文字列の更新はできない。
//		0 以外のポインターを指定すると、start_ 以降の更新が可能となり、
//		buf から start_ - 1 までの文字の更新はできない。
//		この引数を省略すると 0 が指定されたとみなされる。
// ModSize allocateSize
//		呼出側の確保した buf の領域長。
//		この引数に 0 を指定したときは静的なバッファとなり、
//		0 以外の値(ただし buf の領域長でなければならない)を指定したときは
//		動的なバッファとなる。
//
// RETURN
// なし
//
// EXCEPTIONS
// その他
// なし
//

template <class BufferType>
inline
ModPureStrStreamBuffer<BufferType>::
ModPureStrStreamBuffer(BufferType*	buf,
					   ModSize		length,
					   BufferType*	start_,
					   ModSize		allocateSize)
	: buffer(buf),
	  tail(buf + length),
	  status(0),
	  start(start_),
	  currentTail(start_),
	  updateLimit(buf + length),
	  allocateStep(defaultAllocateStep)
{
	if (allocateSize > 0) {
		// 動的なバッファ
		this->status |= dynamic;
		this->tail = buf + allocateSize;
		this->updateLimit = this->tail;

	} else if (length == 0 || start_ != 0) {
		// buffer が null-terminate された文字列であると仮定する
		BufferType	nullCharacter('\0');
		BufferType* cp;
		for (cp = this->buffer; *cp != nullCharacter; ++cp);
		this->tail = cp;
		//
		// man ssbuf を見ると以下のコードがいいはずだが
		// this->updateLimit = cp;
		// Example を見ると length が正のときは無視できないようだ。
		//// ssbuf で length を無視すると言っているのは入力ポインタの事。
		//// 出力バッファの上限値のために length は必要なはず。(nori)
		if (length == 0) {
			this->updateLimit = cp;
		} else if (length > ModSize(cp - buffer)) {
			// 静的モードで文字を put したときにおかしくなるので、
			// 出力バッファを NUL 文字で初期化する(0で埋める)。
			//// いちばん修正量の少ない方法で対応しました(nori)
			const ModSize bytes =
				(length - ModSize(cp - buffer)) * sizeof(BufferType);
			ModOsDriver::Memory::reset(cp, bytes);
		}
	}
	if (start_ == 0) {
		// 更新は一切できない
		this->updateLimit = 0;
	}
}

//
// FUNCTION
// ModPureStrStreamBuffer::~ModPureStrStreamBuffer() -- デストラクタ
//
// NOTES
// ModPureStrStreamBuffer のデストラクタ
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
// その他
// ModStandardManager::free の例外参照
//

template <class BufferType>
inline
ModPureStrStreamBuffer<BufferType>::~ModPureStrStreamBuffer()
{
	if (this->buffer && this->isFrozen() == ModFalse) {
		// バッファが動的に確保されているので free する
		// フリーズされている場合はここでは free しない
		// アプリケーションが free の責任を持つ
		const ModSize bytes =
			(ModSize)(this->tail - this->buffer) * sizeof(BufferType);
		ModStandardManager::free(this->buffer, bytes);
	}
	this->buffer = 0;
}

//
// FUNCTION
// ModPureStrStreamBuffer::freeze -- バッファの凍結設定
//
// NOTES
// 引数によりバッファの凍結を設定したり解除したりする
//
// ARGUMENTS
// ModBoolean freezeFlag
//		設定するか否かを示すフラグ。
//		ModTrue のとき凍結し、ModFalse のとき凍結を解除する。
//		引数を省略したときは凍結する。
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class BufferType>
inline
void
ModPureStrStreamBuffer<BufferType>::freeze(ModBoolean freezeFlag)
{
	if (freezeFlag)
		this->status |= frozen;
	else
		this->status &= ~frozen;
}

//
// FUNCTION
// ModPureStrStreamBuffer::isFrozen -- 凍結中か否かを得る
//
// NOTES
// この関数はバッファが凍結中か否かを得るのに用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// バッファが凍結中の場合 ModTrue を返し、凍結中でない場合 ModFalse を返す。
//
// EXCEPTIONS
// なし
//

template <class BufferType>
inline
ModBoolean
ModPureStrStreamBuffer<BufferType>::isFrozen()
{
	return (this->status & frozen || (this->status & dynamic) == 0) ?
		ModTrue : ModFalse;
}

//
// FUNCTION
// ModPureStrStreamBuffer::getString -- バッファを凍結して内容を得る
//
// NOTES
// この関数はバッファを凍結して現在の内容を得るのに用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// バッファの内容を指すポインタを返す。内部で freeze しているので、
// freeze(ModFalse) によって解除しないと返されたポインターの free は
// 呼出側の責任となる。
//
// EXCEPTIONS
// なし
//

template <class BufferType>
inline BufferType*
ModPureStrStreamBuffer<BufferType>::getString()
{
	this->freeze(ModTrue);
	return this->buffer;
}

//
// FUNCTION
// ModPureStrStreamBuffer::putChar -- バッファに char を追加する
//
// NOTES
// この関数はバッファに char を一つ追加するのに用いる。
//
// ARGUMENTS
// char character
//		バッファに追加する文字
//
// RETURN
// 成功した場合は character を int にキャストしたものを返す。
// 失敗した場合は例外を投げる。
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//		拡張不可能なバッファに対して overflow が呼ばれた
//
// その他
// ModStandardManager::allocate、ModStandardManager::free の例外参照
//

template <class BufferType>
inline
int
ModPureStrStreamBuffer<BufferType>::putChar(BufferType c)
{
	return (this->currentTail >= this->updateLimit) ?
		this->overflow(c) :	(int) (*this->currentTail++ = c);
}

//	FUNCTION public
//	ModPureStrStreamBuffer::putString -- 文字列ストリームに C 文字列を追加する
//
//	NOTES
//
//	ARGUMENTS
//		char*				s
//			文字列ストリームに追加する C 文字列が
//			格納されている領域の先頭アドレス
//		ModSize				len
//			指定されたとき
//				文字列ストリームに追加する文字数
//				0 は、文字列全体を追加することを表す
//			指定されないとき
//				0 が指定されたものとみなす
//
//	RETURN
//		文字列ストリームに追加した文字数
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			追加する C 文字列が格納されている領域の
//			先頭アドレスとして 0 が与えられた

template <class BufferType>
inline
ModSize
ModPureStrStreamBuffer<BufferType>::putString(const BufferType* s, ModSize len)
{
	if (s == 0)
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	BufferType nullCharacter('\0');
	ModSize n = 0;
	for (const BufferType* p = s; *p != nullCharacter; ++p) {
		(void) this->putChar(*p);
		if (++n == len)
			break;
	}

	return n;
}

//
// FUNCTION
// ModPureStrStreamBuffer::seekOffset -- ポインターを相対移動する
//
// NOTES
// この関数は文字列上を走査しているポインターを相対位置で移動するのに用いる。
//
// ARGUMENTS
// ModOffset offset
//		移動させる相対位置
// SeekDirection direction
//		移動させる方向。先頭から、現在位置から、最後尾からの別。
//		この引数を省略したときは現在位置からとなる。
// SeekMode mode
//		移動させるポインターの種類。現在は入力ポインターのみ指定可能。
//		この引数は通常省略して使用する。将来の拡張のため。
//
// RETURN
// 移動後のポインターの位置を返す。移動後の位置が不正な場合は
// (ModSize) ModEof を返す。
//
// EXCEPTIONS
// ModCommonErrorNotSupported
//		mode に ModPureStrStreamBuffer::out を指定した
// ModCommonErrorBadArgument
//		direction の値が begin、end、current 以外であった
//

template <class BufferType>
inline
ModSize
ModPureStrStreamBuffer<BufferType>::seekOffset(
	ModOffset offset,
	ModPureStrStreamBufferBase::SeekDirection direction,
	ModPureStrStreamBufferBase::SeekMode mode)
{
	if (mode == out)

		// 出力ポインターは現在未実装である

		ModThrow(ModModuleStandard,
				 ModCommonErrorNotSupported, ModErrorLevelError);

	ModSize		pos;
	switch (direction) {
	case begin:
		pos = 0;	break;
	case end:
		pos = this->getSize();		break;
	case current:
		pos = (this->currentTail - this->buffer) * sizeof(BufferType);	break;
	default:
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}

	if ((offset > 0 && (ModSize) offset > this->getSize() - pos) ||
		(offset < 0 && (ModSize) -offset > pos))
		return (ModSize) ModEof;

	this->currentTail = this->buffer + (pos += offset);
	return pos;
}

//
// FUNCTION
// ModPureStrStreamBuffer::seekPosition -- ポインターを絶対移動する
//
// NOTES
// この関数は文字列上を走査しているポインターを絶対位置で移動するのに用いる。
//
// ARGUMENTS
// ModSize position
//		移動させる絶対位置
// SeekMode mode
//		移動させるポインターの種類。現在は入力ポインターのみ指定可能。
//		この引数は通常省略して使用する。将来の拡張のため。
//
// RETURN
// 移動後のポインターの位置を返す。移動後の位置が不正な場合は
// (ModSize) ModEof を返す。
//
// EXCEPTIONS
// ModCommonErrorNotSupported
//		mode に ModPureStrStreamBuffer::out を指定した
//

template <class BufferType>
inline
ModSize
ModPureStrStreamBuffer<BufferType>::seekPosition(
	ModSize pos, ModPureStrStreamBufferBase::SeekMode mode)
{
	if (mode == out)

		// 出力ポインターは現在未実装である

		ModThrow(ModModuleStandard,
				 ModCommonErrorNotSupported, ModErrorLevelError);

	if (pos > this->getSize())
		return (ModSize) ModEof;

	this->currentTail = this->buffer + pos;
	return pos;
}

//
// FUNCTION
// ModPureStrStreamBuffer::getSize -- 文字列の文字数を得る
//
// NOTES
// この関数は ModPureStrStreamBuffer に格納されている文字列の文字数を得るのに
// 用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// 文字列の文字数を返す。ただし null-terminate の分は数えない。
//
// EXCEPTIONS
// なし
//

template <class BufferType>
inline
ModSize
ModPureStrStreamBuffer<BufferType>::getSize() const
{
	return (ModSize)(this->getTail() - this->buffer);
}

//
// FUNCTION
// ModPureStrStreamBuffer::getTail -- 文字列の最後尾を得る
//
// NOTES
// この関数は ModPureStrStreamBuffer に格納されている文字列の最後尾を
// 指すポインタを得るのに用いる。ただし、固定長のバッファを使っている
// 場合はバッファの末尾位置を返す(固定長バッファの余白はコンストラクタで
// NUL 文字で埋めてある)。
//
// ARGUMENTS
// なし
//
// RETURN
// 文字列の最後尾を指すポインタを返す。
//
// EXCEPTIONS
// なし
//

template <class BufferType>
inline
BufferType*
ModPureStrStreamBuffer<BufferType>::getTail() const
{
	return 
		(this->status & dynamic) ? this->currentTail :
		(this->updateLimit) ? this->updateLimit : this->tail;
}

//
// FUNCTION
// ModPureStrStreamBuffer::clear -- ストリームを空にする
//
// NOTES
// この関数はバッファの内容を空にするために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class BufferType>
inline
void
ModPureStrStreamBuffer<BufferType>::clear()
{
	this->currentTail = this->start;
}

//
// FUNCTION
// ModPureStrStreamBuffer::getAllocateStep -- 文字列の領域再確保幅を得る
//
// NOTES
// この関数は ModPureStrStreamBuffer のデータメンバ allocateStep の
// 値を得るために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// データメンバ allocateStep の値を返す。
//
// EXCEPTIONS
// なし
//

template <class BufferType>
inline
ModSize
ModPureStrStreamBuffer<BufferType>::getAllocateStep(void) const
{
	return this->allocateStep;
}

//
// FUNCTION
// ModPureStrStreamBuffer::setAllocateStep -- 文字列の領域再確保幅をセットする
//
// NOTES
// この関数は ModPureStrStreamBuffer のデータメンバ allocateStep の
// 値をセットするために用いる。
//
// ARGUMENTS
// const ModSize newStep
//		セットしたい allocateStep の値。
//		この値が0の場合はデフォルト値が代わりに用いられる。
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

template <class BufferType>
inline
void
ModPureStrStreamBuffer<BufferType>::setAllocateStep(ModSize v)
{
	this->allocateStep = (v) ? v : this->defaultAllocateStep;
}


template <class BufferType>
inline
int
ModPureStrStreamBuffer<BufferType>::overflow(BufferType character)
{
	if (this->isFrozen()) {

		// 静的バッファ、フリーズされたバッファは拡張不可能

		ModErrorMessage << "Expand attempt for frozen or static buffer"
						<< ModEndl;
		return ModEof;
	}

	BufferType* oldBuffer = this->buffer;
	BufferType* oldStart = this->start;
	BufferType* oldCurrentTail = this->currentTail;
	ModSize oldSize = (ModSize)(this->updateLimit - this->buffer);
	ModSize newSize = 0;
	// ある程度の大きさになったら、領域を倍々にしていく。
	if (oldSize / 10 > this->allocateStep)
		newSize = oldSize * 2;
	else
		newSize = (this->buffer) ?
			oldSize + this->allocateStep : ModPureStrStreamBuffer::defaultSize;

	// 新しい領域を確保する
	const ModSize bytes = sizeof(BufferType) * newSize;
	this->buffer = (BufferType*)ModStandardManager::allocate(bytes);
	ModOsDriver::Memory::reset(this->buffer, bytes);

	this->start = this->buffer + (oldStart - oldBuffer);
	this->currentTail = this->buffer + (oldCurrentTail - oldBuffer);
	this->updateLimit = this->buffer + newSize;
	this->tail = this->updateLimit;

	if (oldBuffer != 0) {
		if (oldCurrentTail > oldBuffer) {
			// 今までのデータをコピーする
		  ModOsDriver::Memory::copy(this->buffer,
									oldBuffer,
									sizeof(BufferType) * oldSize);
		}
		// freeする
		ModStandardManager::free(oldBuffer, sizeof(BufferType) * oldSize);
	}

	// 値を代入する
	return (unsigned)(*(this->currentTail++) = character);
}

//
// CLASS
//	ModStrStreamBuffer -- ModPureStrStreamBufferクラスのメモリハンドル明示クラス
// NOTES
//	ModPureStrStreamBufferクラスをデフォルトメモリハンドルの管理下の
//	クラスとして利用するためのクラスである。ユーザは通常本クラスを利用する。
//

template <class BufferType>
class ModStrStreamBuffer
	: public	ModDefaultObject,
	  public	ModPureStrStreamBuffer<BufferType>
{
public:
	// コンストラクター
	ModStrStreamBuffer()
	{}
	ModStrStreamBuffer(ModSize size)
		: ModPureStrStreamBuffer<BufferType>(size)
	{}
	ModStrStreamBuffer(BufferType* buf, ModSize length, BufferType* start_ = 0,
					   ModSize allocateSize = 0)
		: ModPureStrStreamBuffer<BufferType>(buf, length, start_, allocateSize)
	{}
	// デストラクター
	~ModStrStreamBuffer()
	{}
};

#endif	// __ModStrStreamBuffer_H__

//
// Copyright (c) 1997, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
