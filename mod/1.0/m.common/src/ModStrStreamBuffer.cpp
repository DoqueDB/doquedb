// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModStrStreamBuffer.cpp -- 文字列を格納するストリームバッファ関連の
//							 メソッド定義
// 
// Copyright (c) 1997, 2000, 2023 Ricoh Company, Ltd.
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


#include "ModStrStreamBuffer.h"

//
// CONST private
// ModPureStrStreamBuffer::defaultSize -- 最初の確保バイト数
//
// NOTES
// この定数は ModStrStreamBuffer のバッファを最初に確保するときに使う
// バイト数を設定するのに用いる。
//
const ModSize ModPureStrStreamBufferBase::defaultSize = 16;

//
// CONST private
// ModPureStrStreamBuffer::defaultAllocateStep -- 領域を伸ばす幅のデフォルト値
//
// NOTES
// この定数は ModStrStreamBuffer のバッファ領域を伸ばすときに使用する
// ステップ幅のデフォルト値を指定する。
//
const ModSize ModPureStrStreamBufferBase::defaultAllocateStep = 32;



#if 0


//
// FUNCTION private
// ModPureStrStreamBuffer::overflow -- 領域再確保と代入
//
// NOTES
// この関数は必要な領域が確保されている領域を上回ったときに呼ばれ、
// バッファのステータスに応じて領域の拡張やエラー発生を行なう。
//
// ARGUMENTS
// char character
//		代入する文字
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

int
ModPureStrStreamBuffer::overflow(BufferType character)
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
	ModSize oldSize = this->updateLimit - this->buffer;
	ModSize newSize = (this->buffer) ?
		oldSize + this->allocateStep : ModPureStrStreamBuffer::defaultSize;

	// 新しい領域を確保する
	this->buffer = (BufferType*)
		ModStandardManager::allocate(sizeof(BufferType) * newSize);
	ModOsDriver::Memory::reset(this->buffer, sizeof(BufferType) * newSize);

	this->start = this->buffer + (oldStart - oldBuffer);
	this->currentTail = this->buffer + (oldCurrentTail - oldBuffer);
	this->updateLimit = this->buffer + newSize;
	this->tail = this->updateLimit;

	if (oldBuffer != 0) {
		if (oldCurrentTail > oldBuffer) {
			// 今までのデータをコピーする
		  ModOsDriver::Memory::copy(this->buffer, oldBuffer,
									sizeof(BufferType) * oldSize);
		}
		// freeする
		ModStandardManager::free(oldBuffer, sizeof(BufferType) * oldSize);
	}

	// 値を代入する
	return (unsigned)(*(this->currentTail++) = character);
}
#endif

//
// Copyright (c) 1997, 2000, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
