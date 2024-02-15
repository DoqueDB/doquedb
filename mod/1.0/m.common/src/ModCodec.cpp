// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4
//
// ModCodec.c -- 圧縮、暗号化
// 
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
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


#include "ModCodec.h"
#include "ModSerialIO.h"
#include "ModOsDriver.h"
#include "ModOsException.h"
#include "ModCommonInitialize.h"

//
// FUNCTION public
// ModPureCodec::ModPureCodec -- ModPureCodecクラスのコンストラクタ
//
// NOTES
// ModPureCodecクラスをコンストラクトし、コーデック用バッファを
// 用意する。バッファリングのみの場合は、1つのバッファを
// 圧縮・伸長、暗号化の場合は、2つのバッファを確保する。
//
// ARGUMENTS
// ModSize bufferSize_
//	コーデック用バッファサイズ
// CodecMode mode_
//  コーデックモード
//
// RETURN
// なし
//
// EXCEPTIONS
//	その他
//		ModCommonInitialize::checkAndInitialize(初期化前のみ)、ModMemoryHandle::allocateMemoryの例外参照
//

ModPureCodec::ModPureCodec(ModSize bufferSize_, CodecMode mode_)
{
	// 必要ならば汎用ライブラリーを初期化する

	ModCommonInitialize::checkAndInitialize();

	// 例外を起こさない初期化

	this->mode = mode_; 
	this->bufferSize = (bufferSize_ + sizeof(long) - 1)
		/ sizeof(long) * sizeof(long);

	// 例外処理のための初期化

	this->buffer = 0;
	this->compressBuffer = 0;

	// 例外を起こす可能性のある初期化

	try {
		this->buffer = (char*) ModOsManager::allocate(this->getBufferSize());
		; ModAssert(this->buffer != 0);

		if (mode & compressMode || mode & cryptMode) {
			this->compressBuffer =
				(char*) ModOsManager::allocate(this->getBufferSize());
			; ModAssert(this->compressBuffer != 0);
		}
	} catch (ModException& exception) {

		// 初期化に失敗した

		this->destruct();
		ModRethrow(exception);
	}

	// バッファが確保されたので、リセットしておく

	this->reset();
}

//
// FUNCTION public
// ModPureCodec::~ModPureCodec -- ModPureCodecクラスのデストラクタ
//
// NOTES
// ModPureCodecクラスをデストラクトし、コーデック用バッファを
// 解放する。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
//	その他
//		MemoryHandle::freeMemory(多くの場合、ModMemoryErrorFreeUnAllocated)
//

ModPureCodec::~ModPureCodec()
{
	this->destruct();
}

//	FUNCTION private
//	ModPureCodec::desturct -- 符号化クラスのデストラクター下位関数
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

void
ModPureCodec::destruct()
{
	if (this->buffer != 0)
		ModOsManager::free(this->buffer, this->getBufferSize()),
		this->buffer = 0;
	if (this->compressBuffer != 0)
		ModOsManager::free(this->compressBuffer, this->getBufferSize()),
		this->compressBuffer = 0;
}

//
// FUNCTION public
// ModPureCodec::reset -- ModPureCodecクラスのバッファのリセット
//
// NOTES
// ModPureCodecクラスで使用するバッファのポインタ位置をリセットする。
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

void
ModPureCodec::reset()
{
	this->current = this->buffer;
	this->encodedSize = 0;
	this->decodedSize = 0;
	this->compressedSize = 0;
}

//
// FUNCTION public
// ModPureCodec::encodeFlush -- エンコードバッファのフラッシュ
//
// NOTES
// エンコードバッファをフラッシュする。
//
// ARGUMENTS
// ModSerialIO* io_
//	実際にIOを行うクラスへのポインタ
//
// RETURN
// なし
//
// EXCEPTIONS
//	ModOsErrorWriteProtocolInCodec
//		コーデックのプロトコルによるwriteでエラーが起きた(Fatal)
//	ModOsErrorWriteDataInCodec
//		コーデックによるデータ部分のwriteでエラーが起きた
//  その他
//		ModSerialIOのサブクラスのrawWriteの実装で発生する例外を送出
//

void
ModPureCodec::encodeFlush(ModSerialIO* io_)
{
	; ModAssert(io_ != 0);

	if (this->getEncodedSize() == 0)

		// バッファ中にデータが存在しない

		return;

	// バッファ上に未読み出しのデータは存在しないはず

	; ModAssert(this->getDecodedSize() == 0);

	switch (this->getMode()) {
	case blockMode:
	{
		// ブロック単位の入出力を行うとき、
		// バッファの未使用領域を 0 埋めしておく

		ModSize	n = this->getBufferSize() - this->getEncodedSize();
		if (n > 0) {
			ModOsDriver::Memory::reset(
				this->buffer + this->getEncodedSize(), n);
			this->encodedSize = this->getBufferSize();
		}
		// thru
	}
	case bufferMode:

		// データを書き出す

		if ((ModSize) io_->rawWrite(
			this->buffer, this->getEncodedSize()) < this->getEncodedSize()) {
error:
			// データ部分を書き出せなかった

			ModThrowOsError(ModOsErrorWriteDataInCodec);
		}
		break;

	default:
	{
		// 圧縮、暗号化して書き出すとき

		; ModAssert(this->compressBuffer != 0);

		// バッファ中のデータを圧縮、暗号化する

		ModSize	compressSize;
		ModSize	size =
			this->rawEncode(this->buffer, this->getEncodedSize(),
							this->compressBuffer, &compressSize);
		this->compressedSize += compressSize;

		// 得られた値を自分自身のバイトオーダーから、
		// ネットワークバイトオーダーへ変換する

		ModSize network_size =
			ModOsDriver::Socket::hostToNetwork(size);
		ModSize	network_compressSize =
			ModOsDriver::Socket::hostToNetwork(compressSize);
		ModSize	network_encodedSize =
			ModOsDriver::Socket::hostToNetwork(this->encodedSize);

		// 圧縮、暗号化されているか、圧縮、暗号化後のサイズ、
		// 圧縮、暗号化前のサイズをそれぞれ書き出す

		if (io_->rawWrite(&network_size,
						  sizeof(ModSize)) < sizeof(ModSize) ||
			io_->rawWrite(&network_compressSize,
						  sizeof(ModSize)) < sizeof(ModSize) ||
			io_->rawWrite(&network_encodedSize,
						  sizeof(ModSize)) < sizeof(ModSize)) {

			// プロトコル部分の書き出しに失敗した

			ModThrowOsFatal(ModOsErrorWriteProtocolInCodec);
		}

		// 圧縮、暗号化したデータ部分を書き出す

		if ((ModSize) io_->rawWrite(
			this->compressBuffer, compressSize) < compressSize)
			goto error;
	}
	}

	// バッファ中の未書き出しのデータはなくなった

	this->current = this->buffer;
	this->encodedSize = 0;
}

//
// FUNCTION public
// ModPureCodec::decodeFlush -- デコードバッファのフラッシュ
//
// NOTES
// デコードバッファをフラッシュする。
//
// ARGUMENTS
// ModSerialIO* io_
//	実際にIOを行うクラスへのポインタ
// ModSize	min_
//	最低でも読み出す必要のあるデータのサイズ(B 単位)
//	指定されないときは、1 が指定されたものとみなす
//
// RETURN
// なし
//
// EXCEPTIONS
//	ModOsErrorReadProtocolInCodec
//		コーデックのプロトコルによるreadでエラーが起きた(Fatal)
//	ModOsErrorReadDataInCodec
//		コーデックによるデータのreadでエラーが起きた
//	ModOsErrorEndOfFile
//		EOF から読み出そうとしたため、
//		ぜんぜんデータ部分を読み出せなかった
//  その他
//		ModSerialIOのサブクラスのrawReadの実装で発生する例外を送出
//

void
ModPureCodec::decodeFlush(ModSerialIO* io_, ModSize min_)
{
	; ModAssert(io_ != 0);

	if (this->getDecodedSize() > 0)

		// バッファ中にデータが存在する

		return;

	if (this->getEncodedSize() > 0)

		// バッファ上の未書き出しのデータは捨てる

		this->reset();

	ModSize	decodeSize;

	switch (this->getMode()) {
	case blockMode:

		// ブロック単位の入出力を行うとき、
		// 必ず、バッファサイズぶんのデータを読み出す

		min_ = this->getBufferSize();
		// thru

	case bufferMode:

		// データを読み出す

		if (min_ > this->getBufferSize())
			min_ = this->getBufferSize();

		decodeSize = io_->rawRead(this->buffer, this->getBufferSize(), min_);

		if (decodeSize < min_)
			if (decodeSize == 0) {

				// EOF から読み出そうとしたため、
				// ぜんぜんデータ部分を読み出せなかった

				ModThrowOsError(ModOsErrorEndOfFile);
			} else {
error:
				// データ部分を読み出せなかった

				ModThrowOsError(ModOsErrorReadDataInCodec);
			}
		break;

	default:
	{
		// 圧縮、暗号化されている符号を読み出すとき

		; ModAssert(this->compressBuffer != 0);

		// 圧縮、暗号化されているか、圧縮、暗号化後のサイズ、
		// 圧縮、暗号化前のサイズをそれぞれ読み出す

		ModSize	size;
		ModSize	compressSize;

		if (io_->rawRead(&size, sizeof(ModSize)) < sizeof(ModSize) ||
			io_->rawRead(&compressSize, sizeof(ModSize)) < sizeof(ModSize) ||
			io_->rawRead(&decodeSize, sizeof(ModSize)) < sizeof(ModSize)) {

			// プロトコル部分の読み出しに失敗した

			ModThrowOsFatal(ModOsErrorReadProtocolInCodec);
		}

		// 得られた値をネットワークバイトオーダーから
		// 自分自身のバイトオーダーへ変換する

		size = ModOsDriver::Socket::networkToHost(size);
		compressSize = ModOsDriver::Socket::networkToHost(compressSize);
		decodeSize = ModOsDriver::Socket::networkToHost(decodeSize);

		// 圧縮、暗号化されているデータ部分を読み出す
		//
		//【注意】	データ部分はネットワーク
		//			バイトオーダーに変換されていない

		if ((ModSize) io_->rawRead(
			this->compressBuffer, compressSize) < compressSize)
			goto error;

		// 読み出したデータを解凍、解読する

		this->rawDecode(size, this->compressBuffer, compressSize,
						this->buffer, &decodeSize);
		this->compressedSize += compressSize;
	}
	}

	this->current = this->buffer;
	this->decodedSize = decodeSize;
}

//  FUNCTION public
//  ModPureCodec::hasBufferedData -- バッファにデータがあるか
//
//  NOTES
//
//  ARGUMENTS
//      なし
//
//  RETURN
//      バッファにデータがあれば true, なければ false
//
//  EXCEPTIONS
//      なし
bool
ModPureCodec::hasBufferedData() const
{
	return (this->getDecodedSize() > 0);
}

//	FUNCTION public
//	ModPureCodec::encode -- 符号化する
//
//	NOTES
//
//	ARGUMENTS
//		ModSerialIO*		io_
//			符号化するデータの出力先を表すクラス
//		void*				buffer_
//			符号化するデータが格納されている領域の先頭アドレス
//		ModSize				size_
//			符号化するデータのサイズ(B 単位)
//
//	RETURN
//		符号化されたデータのサイズ(B 単位)
//
//	EXCEPTIONS
//		なし

int
ModPureCodec::encode(ModSerialIO* io_, const void* buffer_, ModSize size_)
{
	; ModAssert(io_ != 0);
	; ModAssert(buffer_ != 0);

	int accum = 0;

retry:
	if (this->getDecodedSize() > 0)

		// バッファ上の未読み出しのデータは捨てる

		this->reset();

	ModSize	n = this->getBufferSize() - this->getEncodedSize();

	if (n < size_) {

		// バッファ上のデータでは指定されたサイズに足りない

		if (n > 0) {

			// まず、バッファの未使用領域のぶんのデータを格納しておく

			ModOsDriver::Memory::copy(this->current, buffer_, n);

			this->current += n;
			this->encodedSize += n;
		}

		// バッファサイズぶんのデータを符号化し、書き出す

		this->encodeFlush(io_);

		// 残りのぶんのデータを符号化する

		//return n + this->encode(io_, (char*) buffer_ + n, size_ - n);
		accum += n;
		buffer_ = (char*) buffer_ + n;
		size_ -= n;
		goto retry;
	}

	// バッファの未使用領域に指定されたサイズぶんのデータを格納できる

	ModOsDriver::Memory::copy(this->current, buffer_, size_);

	this->current += size_;
	this->encodedSize += size_;

    //return size_;
	return accum + size_;
}

//	FUNCTION public
//	ModPureCodec::decode -- 復号化する
//
//	NOTES
//
//	ARGUMENTS
//		ModSerialIO*	io_
//			復号化するデータを得る入力を表すクラス
//		void*			buffer_
//			復号化されたデータを格納する領域の先頭アドレス
//		ModSize			size_
//			復号化するデータのサイズ(B 単位)
//
//	RETURN
//		復号化されたデータのサイズ(B 単位)
//
//	EXCEPTIONS

int
ModPureCodec::decode(ModSerialIO* io_, void* buffer_, ModSize size_)
{
	; ModAssert(io_ != 0);
	; ModAssert(buffer_ != 0);

	int accum = 0;
retry:
	if (this->getEncodedSize() > 0)

		// バッファ上の未書き出しのデータは捨てる

		this->reset();

	ModSize	n = this->getDecodedSize();

	if (n < size_) {

		// バッファ上のデータでは指定されたサイズに足りない

		if (n > 0) {

			// まず、バッファ上のぶんだけ取り出しておく

			ModOsDriver::Memory::copy(buffer_, this->current, n);

			this->current = this->buffer;
			this->decodedSize = 0;
		}

		try {

			// 最大でバッファサイズぶんのデータを読み出し、復号化する

			this->decodeFlush(io_, size_ - n);

		} catch (ModException& exception) {

			switch (exception.getErrorNumber()) {
			case ModOsErrorEndOfFile:

				// EOF から読み出そうとしたため、
				// ぜんぜんデータ部分が読み出せなかったときは、
				// エラーを無視して、
				// バッファ上のぶんだけ復号化できたことにする

				ModErrorHandle::reset();
				return n;
			default:
				ModRethrow(exception);
			}
		}

		// 残りのぶんのデータを取り出す

		//return n + this->decode(io_, (char*) buffer_ + n, size_ - n);
		accum += n;
		buffer_ = (char*) buffer_ + n;
		size_ -= n;
		goto retry;
	}

	// バッファ上に指定されたサイズぶんの復号化済のデータが存在する

	ModOsDriver::Memory::copy(buffer_, this->current, size_);

	if (n == size_) {
		this->current = this->buffer;
		this->decodedSize = 0;
	} else {
		this->current += size_;
		this->decodedSize -= size_;
	}

	//return size_;
	return accum + size_;
}

//
// Private Method
//

//
// FUNCTION
// ModPureCodec::rawEncode -- エンコードの実行
//
// NOTES
// エンコードを実装するために用意したメソッド。
// 本メソッドを実際のエンコードアルゴリズムに合わせて実装する。
// ここでは、実際にエンコードせずに、入力バッファから
// 出力バッファへコピーを行う。
//
// ARGUMENTS
// void* in_
//  入力データへのポインタ
// ModSize inSize_
//  入力データサイズ
// void* out_
//  出力データへのポインタ
// ModSize* outSize_
//	出力（圧縮）サイズへのポインタ
//
// RETURN
// エンコードサイズ
//
// EXCEPTIONS
//	なし
//
int
ModPureCodec::rawEncode(void* in_, ModSize inSize_, void* out_, ModSize* outSize_)
{
    // ここで本当に圧縮する。
    ModOsDriver::Memory::copy(out_, in_, inSize_);
    *outSize_ = inSize_;

	return 0;
}

//
// FUNCTION
// ModPureCodec::rawDecode -- デコードの実行
//
// NOTES
// デコードを実装するために用意したメソッド。
// 本メソッドを実際のデコードアルゴリズムに合わせて実装する。
// ここでは、実際にデコードせずに、入力バッファから
// 出力バッファへコピーを行う。
//
// ARGUMENTS
// void* in_
//  入力データへのポインタ
// ModSize inSize_
//  入力データサイズ
// void* out_
//  出力データへのポインタ
// ModSize* outSize_
//	出力（伸長）サイズへのポインタ
//
// RETURN
// デコードサイズ
//
// EXCEPTIONS
// なし
//
int
ModPureCodec::rawDecode(int encodeStatus_, void* in_, ModSize inSize_, void* out_, ModSize* outSize_)
{
    // ここで本当に解凍する。
    ModOsDriver::Memory::copy(out_, in_, inSize_);
    *outSize_ = inSize_;

	return 0;
}

//
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
