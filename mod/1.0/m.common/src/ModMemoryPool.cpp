// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModMemoryPool.cpp -- メモリープール関連のメソッドの定義
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


#include "ModMemoryPool.h"
#include "ModOsDriver.h"
#include "ModAutoMutex.h"
#include "ModParameter.h"
#include "ModCommonException.h"

//	VARIABLE private
//	ModMemoryPool::_initialized -- メモリープールが初期化されているか
//
//	NOTES

ModBoolean	ModMemoryPool::_initialized = ModFalse;

//	VARIABLE private
//	ModMemoryPool::_limit -- メモリープールの総確保サイズの上限
//
//	NOTES
//		メモリープールから確保可能な領域の総サイズの上限を表す
//		ただし、総サイズには非常用領域のサイズは含まない
//		この値は、ModMemoryPool::initialize または
//		ModMemoryPool::setTotalLimit を明示的に呼び出して設定することもできる
//		明示的に設定する前はパラメーター MemoryPoolTotalLimit の値が設定される

ModSize		ModMemoryPool::_limit = 100 << 10;		// B 単位

//	VARIABLE private
//	ModMemoryPool::_size --
//		メモリープールで現在確保し、使用している領域の総サイズ
//
//	NOTES
//		メモリープールで現在確保し、使用している領域の総サイズを表す
//		これには使用済領域や非常用領域のサイズは含まない

ModSize		ModMemoryPool::_size = 0;					// B 単位

//	VARIABLE private
//	ModMemoryPool::_emergencyLimit -- メモリープールの非常用領域の総サイズ
//
//	NOTES
//		非常用領域は、ModMemoryPool::allocateEmergencyMemory により確保される
//		パラメーター MemoryPoolEmergencyLimit の値が設定される

ModSize		ModMemoryPool::_emergencyLimit = 1 << 10;	// B 単位

//	VARIABLE private
//	ModMemoryPool::_emergencySize --
//		メモリープールで現在確保している非常用領域の総サイズ
//
//	NOTES

ModSize		ModMemoryPool::_emergencySize = 0;

//	VARIABLE private
//	ModMemoryPool::_emergencyBlockSize -- 非常用領域のブロックサイズ
//
//	NOTES
//		非常用領域の最小サイズで、確保される非常用領域はこの値の倍数となる
//		使用済の非常用領域をブロックリストで管理するためには、
//		最低でも sizeof(ModMemoryCell) バイト必要となる

const ModSize	ModMemoryPool::_emergencyBlockSize = sizeof(ModMemoryCell);

//	VARIABLE private
//	ModMemoryPool::_emergencyBlockList -- 使用済非常用領域リスト
//
//	NOTES
//		使用済の非常用領域が管理されるリスト
//		このリストは使用済非常用領域の先頭アドレスの昇順にソートされる

ModMemoryCell*	ModMemoryPool::_emergencyBlockList = 0;

//	VARIABLE private
//	ModMemoryPool::_emergencyBegin --
//		メモリープールの非常用メモリー領域の先頭アドレス
//
//	NOTES
//		非常用領域を確保するために使用する領域の先頭アドレス
//		この領域を初期化時に確保しておき、
//		ModMemoryPool::allocateEmergencyMemory でこの領域の一部分を取り分ける

void*		ModMemoryPool::_emergencyBegin = 0;

//	VARIABLE private
//	ModMemoryPool::_emergencyEnd --
//		メモリープールの非常用メモリー領域の末尾アドレス
//
//	NOTES
//		非常用領域を確保するために使用する領域の末尾アドレス

void*		ModMemoryPool::_emergencyEnd = 0;

//	FUNCTION public
//	ModMemoryPool::initialize -- メモリープールを初期化する
//
//	NOTES
//		指定された上限の総確保サイズを持つメモリープールを初期化する
//		実際の初期化は ModMemoryPool::doInitialize が行う
//
//	ARGUMENTS
//		ModSize				limit
//			メモリープールの総確保サイズの上限(KB 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
ModMemoryPool::initialize(ModSize limit)
{
	// 指定された上限の総確保サイズを持つ
	// メモリープールを実際に初期化する
	//
	//【注意】	実際の初期化は ModMemoryPool::doInitialize で行われる

	ModMemoryPool::setTotalLimit(limit);
}

//	FUNCTION public
//	ModMemoryPool::allocateMemory --
//		メモリープールから指定されたサイズの領域を確保する
//
//	NOTES
//		実際には ModMemoryPool::allocateMemoryUnsafe で領域を確保する
//
//	ARGUMENTS
//		ModSize				size
//			確保する領域のサイズ(B 単位)
//
//	RETURN
//		確保した領域の先頭アドレス
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			確保する領域のサイズとして 0 が指定された
//			(ModMemoryPool::allocateMemory より)
//		ModMemoryErrorOverPoolLimit
//			確保する領域のサイズとして
//			メモリープールの総確保サイズの上限より大きい値が指定された
//			(ModMemoryPool::allocateMemory より)
//		ModMemoryErrorPoolLimitSize
//			指定されたサイズの領域を確保すると、
//			メモリープールの総確保サイズの上限を超える
//			(ただし、エラー設定されない)
//			(ModMemoryPool::allocateMemory より)
//		ModMemoryErrorOsAlloc
//			新たな領域を確保できない
//			(ただし、エラー設定されない)
//			(ModMemoryPool::allocateMemory より)

// static
void*
ModMemoryPool::allocateMemory(ModSize size)
{
	// 必要ならば汎用ライブラリーを初期化する

	ModCommonInitialize::checkAndInitialize();

	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	(void) m.lock();

	return ModMemoryPool::allocateMemoryUnsafe(size);
}

//	FUNCTION public
//	ModMemoryPool::allocateEmergencyMemory -- 非常用領域を確保する
//
//	NOTES
//		メモリープールの初期化時に確保済の非常用領域の使用済リストから、
//		非常用領域を確保する
//
//	ARGUMENTS
//		ModSize				size
//			確保する非常用領域のサイズ(B 単位)
//
//	RETURN
//		確保した非常用領域の先頭アドレス
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			確保する非常用領域のサイズとして 0 が指定された
//		ModMemoryErrorEmergencyLimit
//			指定されたサイズの非常用領域を確保すると、
//			非常用領域の総サイズの上限を超える

// static
void*
ModMemoryPool::allocateEmergencyMemory(ModSize size)
{
	// 必要ならば汎用ライブラリーを初期化する

	ModCommonInitialize::checkAndInitialize();

	if (!size) {

		// 確保する領域のサイズとして 0 が指定された

		ModThrow(ModModuleMemory,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}

	// 指定されたサイズを非常用ブロックサイズの倍数に丸める

	ModSize	rounded = _emergencyBlockSize;
	for (; rounded < size; rounded <<= 1) ;

	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	(void) m.lock();

	if (_emergencyLimit < rounded ||
		_emergencyLimit - rounded < _emergencySize) {

		// 指定されたサイズの非常用領域を確保すると、
		// 非常用領域の総サイズの上限を超える

		ModThrow(ModModuleMemory,
				 ModMemoryErrorEmergencyLimit, ModErrorLevelError);
	}

	// 使用済非常用ブロックリストから必要なサイズの非常用領域を探す

	void*	p =
		ModMemoryPool::searchBlock(&_emergencyBlockList, rounded,
								   _emergencyBlockSize, ModTrue);
	if (!p) {

		// リスト上に存在しないということは、
		// あらたに確保しないと、非常用領域がとれない

		ModThrow(ModModuleMemory,
				 ModMemoryErrorEmergencyLimit, ModErrorLevelError);
	}

	_emergencySize += rounded;
	return p;
}

//	FUNCTION public
//	ModMemoryPool::freeMemory --
//		メモリープールから確保した領域を破棄する
//	NOTES
//		実際には ModMemoryPool::freeMemoryUnsafe で領域を破棄する
//
//	ARGUMENTS
//		void*				p
//			破棄する領域の先頭アドレス
//		ModSize				size
//			破棄する領域のサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
ModMemoryPool::freeMemory(void* p, ModSize size)
{
	// 必要ならば汎用ライブラリーを初期化する

	ModCommonInitialize::checkAndInitialize();

	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	(void) m.lock();

	ModMemoryPool::freeMemoryUnsafe(p, size);
}

//	FUNCTION public
//	ModMemoryPool::freeEmergencyMemory -- 非常用領域を破棄する
//
//	NOTES
//
//	ARGUMENTS
//		void*				p
//			破棄する非常用領域の先頭アドレス
//		ModSize				size
//			破棄する非常用領域のサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
ModMemoryPool::freeEmergencyMemory(void* p, ModSize size)
{
	if (p) {

		// 必要ならば汎用ライブラリーを初期化する

		ModCommonInitialize::checkAndInitialize();

		// 指定されたサイズを非常用ブロックサイズの倍数に丸める

		ModSize	rounded = _emergencyBlockSize;
		for (; rounded < size; rounded <<= 1) ;

		ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
		(void) m.lock();

		// 破棄する非常用領域を使用済非常用ブロックリストにつなぐ

		ModMemoryPool::chainBlock(&_emergencyBlockList,
								  p, rounded, _emergencyBlockSize);

		_emergencySize -= rounded;
	}
}

//	FUNCTION public
//	ModMemoryPool::setEmergencyLimit --
//		メモリープールの非常用領域の総サイズの上限を設定する
//
//	NOTES
//		現在非常用領域を使用中のときは設定できない
//
//	ARGUMENTS
//		ModSize			size
//			新しい上限(KB 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModMemoryErrorEmergencyAreaUsed
//			非常用領域を使用中である

// static
void
ModMemoryPool::setEmergencyLimit(ModSize size)
{
	// 必要ならば汎用ライブラリーを初期化する

	ModCommonInitialize::checkAndInitialize();

	// 与えられたサイズを B 単位にし、
	// 非常用ブロックサイズ境界に丸める

	ModSize	bytes =
		(ModUtility::kbyteToByte(size) + _emergencyBlockSize - 1) /
		_emergencyBlockSize * _emergencyBlockSize;

	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	(void) m.lock();

	if (_emergencyLimit == bytes)

		// 現在の値から変更されない

		return;
	if (_emergencySize > 0) {

		// 非常用領域を使用中である

		ModThrow(ModModuleMemory,
				 ModMemoryErrorEmergencyAreaUsed, ModErrorLevelError);
	}

	// 現在存在するメモリープールの非常用領域を破棄し、
	// あらたな制限値による非常用領域を確保する

	if (_emergencyBegin) {
		ModOsDriver::Memory::free(_emergencyBegin);
		_emergencyBegin = 0, _emergencyEnd = 0;
	}

	try {
		_emergencyBegin = ModOsDriver::Memory::alloc(bytes);

	} catch (ModException& exception) {
		ModRethrow(exception);
	}
#ifndef NO_CATCH_ALL
	catch (...) {
		ModUnexpectedThrow(ModModuleMemory);
	}
#endif

	_emergencyEnd = (char*) _emergencyBegin + bytes;
	_emergencyLimit = bytes;

	// 今、確保した非常用領域を使用済にする

	ModMemoryPool::chainBlock(&_emergencyBlockList, _emergencyBegin,
							  _emergencyLimit, _emergencyBlockSize);
}

//	FUNCTION private
//	ModMemoryPool::searchBlock --
//		あるブロックリストから指定されたサイズの領域を探す
//
//	NOTES
//		あるブロックリストへつながれるすべての領域は、
//		同じブロックサイズ境界を持つ必要がある
//
//	ARGUMENTS
//		ModMemoryCell**		head
//			領域を探すブロックリストの先頭
//		ModSize				size
//			探す領域のサイズ(B 単位)
//		ModSize				block
//			ブロックリストへつながれている領域の
//			共通のブロックサイズ境界(B 単位)
//		ModBoolean			doDivide
//			ModTrue
//				指定されたサイズ以上の領域があれば、
//				その領域の指定されたサイズぶんを返し、
//				残りの部分をリストへつないだままにする
//			ModFalse
//				指定されたものとまったく等しいサイズの領域を探す
//
//	RETURN
//		0 以外の値
//			見つかった領域の先頭アドレス
//		0
//			指定されたサイズの領域が見つからなかった
//
//	EXCEPTIONS
//		なし

// static
void*
ModMemoryPool::searchBlock(ModMemoryCell** head, ModSize size,
						   ModSize block, ModBoolean doDivide)
{
	; ModAssert(!(size % block));

	ModMemoryCell* prev = 0;
	for (ModMemoryCell*	m = *head; m; prev = m, m = m->next)
		if (m->size >= size) {
			; ModAssert(m->size >= block);
			; ModAssert(!(m->size % block));
			if (m->size == size) {

				// 指定されたサイズと同じである

				(prev ? prev->next : *head) = m->next;
				return (void*) m;

			} else if (doDivide)

				// 残りの部分をブロックリストへ残しておく

				return (void*) ((char*) m + (m->size -= size));
		}

	// みつからなかった

	return 0;
}

//	FUNCTION private
//	ModMemoryPool::chainBlock -- あるブロックリストへ指定した領域をつなぐ
//
//	NOTES
//		あるブロックリストへつながれるすべての領域は、
//		同じブロックサイズ境界を持つ必要がある
//
//	ARGUMENTS
//		ModMemoryCell**		head
//			領域をつなぐブロックリストの先頭
//		void*				p
//			ブロックリストへつなぐ領域の先頭アドレス
//		ModSize				size
//			ブロックリストへつなぐ領域のサイズ(B 単位)
//		ModSize				block
//			ブロックリストへつながれている領域の
//			共通のブロックサイズ境界(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
ModMemoryPool::chainBlock(ModMemoryCell** head,
						  void* p, ModSize size, ModSize block)
{
	if (p) {
		; ModAssert(size >= block);
		; ModAssert(!(size % block));

		// ブロックリストへつなぐ領域は ModMemoryCell として扱う

		ModMemoryCell*	cell = (ModMemoryCell*) p;
		cell->size = size;

		// 領域の先頭アドレスの昇順にソートされている
		// ブロックリスト上で、自分より大きな領域を探す

		ModMemoryCell*	m = *head;
		ModMemoryCell*	prev = 0;
		for (; m && m <= cell; prev = m, m = m->next) ;

		if (m && prev == m) {

			// ブロックリスト上の領域が与えられた

			ModThrow(ModModuleMemory,
					 ModMemoryErrorFreeUnAllocated, ModErrorLevelError);
		}

		cell->next = m;
		if (!prev)

			// 使用済にする領域を先頭へつなぐ

			*head = cell;

		else if ((char*) prev + prev->size == (char*) cell) {

			// 直前の使用済領域とのみマージする

			prev->next = m;
			prev->size += size;
			cell = prev;
		} else

			// 直前の使用済領域とマージしない

			prev->next = cell;

		if (m && (char*) cell + cell->size == (char*) m) {

			// 直後の使用済領域とマージする

			cell->next = m->next;
			cell->size += m->size;
		}
	}
}

#ifdef MOD_SELF_MEMORY_MANAGEMENT
//	VARIABLE private
//	ModMemoryPool::_allocated --
//		メモリープールで現在実際に確保している領域の総サイズ
//
//	NOTES
//		メモリープールで現在実際に確保している領域の総サイズを表す
//		これには非常用領域のサイズは含まない

ModSize		ModMemoryPool::_allocated = 0;				// B 単位

//	VARIABLE private
//	ModMemoryPool::_sizeToRound --
//		要求された領域のサイズから実際に確保する領域のサイズへ丸める表
//
//	NOTES
//		0 から ModMemoryHalfBlockSize までの要求された領域のサイズを、
//		実際に確保する領域のサイズへ丸めるための配列

ModSize		ModMemoryPool::_sizeToRound[ModMemoryHalfBlockSize + 1];

//	VARIABLE private
//	ModMemoryPool::_sizeToBucket --
//		要求された領域のサイズからそのサイズの領域が管理される
//		フリーリストを得るための表
//
//	NOTES
//		0 から ModMemoryHalfBlockSize までの要求された領域のサイズから、
//		そのサイズの領域が管理されるフリーリストを得るための表
//		ModMemoryPool::doInitialize で初期化する

char**		ModMemoryPool::_sizeToBucket[ModMemoryHalfBlockSize + 1];

//	VARIABLE private
//	ModMemoryPool::_freeBucket -- 使用済領域リストの配列
//
//	NOTES
//		0 から ModMemoryHalfBlockSize までのそれぞれのサイズごとに
//		使用済領域を管理するリストの配列
//		ModMemoryPool::doInitialize で初期化する

char*		ModMemoryPool::_freeBucket[ModMemoryBucketNum];

#ifdef MOD_USE_FREE_BLOCK_LIST
//	VARIABLE private
//	ModMemoryPool::_freeAreaPercent -- ブロックの使用済の割合の上限
//
//	NOTES
//		新たにブロックを確保しようとするときに、
//		全ラージ部ロックのうち使用済のものが占める割合が
//		この値より大きければ、すべての使用済ブロックが実際に破棄される
//		この値が 100 以上のとき、使用済ブロックは破棄されることはない

unsigned int	ModMemoryPool::_freeAreaPercent = 100;

//	VARIABLE private
//	ModMemoryPool::_freeBlockList -- 使用済ブロックリスト
//
//	NOTES
//		ブロックサイズ以上の使用済領域が管理されるリスト
//		このリストは使用済領域の先頭アドレスの昇順にソートされる

ModMemoryCell*	ModMemoryPool::_freeBlockList = 0;
#endif
//	FUNCTION public
//	ModMemoryPool::setTotalLimit --
//		メモリープールの総確保サイズの上限を設定する
//
//	NOTES
//		現在確保している領域の総サイズより大きい値へ
//		メモリープールの総確保サイズの上限を変更することができる
//
//	ARGUMENTS
//		ModSize			size
//			新しい上限(KB 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			指定された値が現在確保されている領域の総サイズよりも小さい

// static
void
ModMemoryPool::setTotalLimit(ModSize size)
{
	// 必要ならば汎用ライブラリーを初期化する

	ModCommonInitialize::checkAndInitialize();

	ModSize	bytes = ModUtility::kbyteToByte(size);

	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	(void) m.lock();

	if (bytes < _allocated) {

		// 現在確保されている領域の総サイズよりも
		// 小さくしようとしている

		ModThrow(ModModuleMemory,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}

	_limit = bytes;
}

//	FUNCTION private
//	ModMemoryPool::doInitialize -- メモリープールを実際に初期化する
//
//	NOTES
//		ModCommonInitialize::initialize から
//		ModMemoryHandle::initialize を通して、
//		一回だけ他にメインスレッドのみの状態で呼び出されることが保証されている
//
//	ARGUMENTS
//		ModSize				limit
//			メモリープールの総確保サイズの上限(KB 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
ModMemoryPool::doInitialize(ModSize limit)
{
	try {
		// まず、パラメーター値を得て初期化する

		ModParameter	parameter(ModFalse);	// 初期化中なので領域を
												// 確保することはないようにする
		if (limit)
			_limit = ModUtility::kbyteToByte(limit);
		else
			(void) parameter.getModSize(_limit, (const char*)
										"MemoryPoolTotalLimit");
		(void) parameter.getModSize(_emergencyLimit, (const char*)
									"MemoryPoolEmergencyLimit");
#ifdef MOD_USE_FREE_BLOCK_LIST
		(void) parameter.getUnsignedLong(_freeAreaPercent, (const char*)
										 "MemoryPoolFreeAreaPercent");
#endif
		// 最初は使用済領域リストは存在しない

		ModOsDriver::Memory::reset(_freeBucket, sizeof(_freeBucket));

		// 非常用領域の上限を設定し、
		// その上限値で非常用領域を確保しておく

		ModMemoryPool::setEmergencyLimit(_emergencyLimit);

		// 変換表を初期化しておく

		ModSize	rounded = sizeof(ModPtr);
		int	i = 0;
		for (ModSize size = 0; size <= ModMemoryHalfBlockSize; size++) {
			for (; rounded < size; rounded <<= 1, i++) ;
			_sizeToRound[size] = rounded;
			_sizeToBucket[size] = &_freeBucket[i];
		}

		// 初期化が終了した

		_initialized = ModTrue;

	} catch (ModException& exception) {
		ModRethrow(exception);
	}
#ifndef NO_CATCH_ALL
	catch (...) {
		ModUnexpectedThrow(ModModuleMemory);
	}
#endif
}

//	FUNCTION private
//	ModMemoryPool::allocateMemoryUnsafe --
//		メモリープールから指定されたサイズの領域を実際に確保する
//
//	NOTES
//		メモリープールから指定されたサイズを丸めたサイズの領域を確保する
//		この関数の呼び出し側で、汎用ライブラリーの初期化を行い、
//		必要なスレッド間排他制御を行うこと
//
//	ARGUMENTS
//		ModSize				size
//			確保する領域のサイズ(B 単位)
//
//	RETURN
//		確保した領域の先頭アドレス
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			確保する領域のサイズとして 0 が指定された
//		ModMemoryErrorOverPoolLimit
//			確保する領域のサイズとして
//			メモリープールの総確保サイズの上限より大きい値が指定された
//			(ModMemoryPool::allocateBlock より)
//		ModMemoryErrorPoolLimitSize
//			指定されたサイズの領域を確保すると、
//			メモリープールの総確保サイズの上限を超える
//			(ただし、エラー設定されない)
//			(ModMemoryPool::allocateBlock より)
//		ModMemoryErrorOsAlloc
//			新たな領域を確保できない
//			(ただし、エラー設定されない)
//			(ModMemoryPool::allocateBlock より)

// static
void*
ModMemoryPool::allocateMemoryUnsafe(ModSize size)
{
	if (!size) {

		// 確保する領域のサイズとして 0 が指定された

		ModThrow(ModModuleMemory,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}

	void*	p;

	if (size > ModMemoryHalfBlockSize) {

		// 要求されたサイズがブロックサイズの半分より大きいとき、
#ifdef MOD_USE_FREE_BLOCK_LIST
		// ブロックサイズの倍数単位のサイズの領域を実際に確保する

		size = (size + ModMemoryBlockSize - 1) /
			ModMemoryBlockSize * ModMemoryBlockSize;
#else
		// 要求されたサイズの領域を実際に確保する
#endif
		p = ModMemoryPool::allocateBlock(size);
	} else {

		// 要求されたサイズがブロックサイズの半分以下のとき
		// 2 の階乗に丸めたサイズで実際に確保する

		if (p = *_sizeToBucket[size = _sizeToRound[size]])

			// 使用済リスト中の領域をひとつはずして使うことにする

			*_sizeToBucket[size] = *(char**) p;
		else {

			// 必要以上のサイズの領域を使用済リストからさがす

			ModSize	large = size;
			while ((large <<= 1) < ModMemoryBlockSize)
				if (p = *_sizeToBucket[large])
					break;

			if (p)

				// 見つけた領域を使用済リストからはずして使うことにする

				*_sizeToBucket[large] = *(char**) p;
			else
				// ブロックサイズの領域を確保して使うことにする

				p = ModMemoryPool::allocateBlock(ModMemoryBlockSize);

			// 必要な領域の残りの部分は、フリーリストへ登録しておく
			//
			//	┌─┬─┬───┬───────┬─
			//	│　│　│      │              │
			//	├─┤8	│      │              │
			//	│4 │　│      │              │
			//	├─┴─┤  32  │              │
			//	│      │      │              │
			//	│  16  │      │              │
			//	│      │      │              │
			//	├───┴───┤      128     │
			//	│              │              │
			//	│              │              │
			//	│              │              │
			//	│      64      │              │
			//	│              │              │
			//	│              │              │
			//	│              │              │
			//	├───────┴───────┤512
			//	│              256             │

			while (large > size) {
				large >>= 1;
				char*	rest = (char*) p + large;
				*(char**) rest = *_sizeToBucket[large];
				*_sizeToBucket[large] = rest;
			}
		}
	}

	_size += size;
	return p;
}

//	FUNCTION private
//	ModMemoryPool::allocateBlock --
//		メモリープールから指定されたブロックサイズ以上の領域を実際に確保する
//
//	NOTES
//
//	ARGUMENTS
//		ModSize				size
//			確保する領域のサイズ(B 単位)
//
//	RETURN
//		確保した領域の先頭アドレス
//
//	EXCEPTIONS
//		ModMemoryErrorOverPoolLimit
//			確保する領域のサイズとして
//			メモリープールの総確保サイズの上限より大きい値が指定された
//		ModMemoryErrorPoolLimitSize
//			指定されたサイズの領域を確保すると、
//			メモリープールの総確保サイズの上限を超える
//			(ただし、エラー設定されない)
//		ModMemoryErrorOsAlloc
//			新たな領域を確保できない
//			(ただし、エラー設定されない)

// static
void*
ModMemoryPool::allocateBlock(ModSize size)
{
	if (size > _limit) {

		// 確保する領域のサイズとして
		// メモリープールの総確保サイズの上限より大きい値が指定された

		ModThrow(ModModuleMemory,
				 ModMemoryErrorOverPoolLimit, ModErrorLevelError);
	}

	void*	p;
#ifdef MOD_USE_FREE_BLOCK_LIST
	// 使用済ブロックリストから指定されたサイズの領域を探す

	p =	ModMemoryPool::searchBlock(&_freeBlockList, size, ModMemoryBlockSize,
								   (ModBoolean) (_freeAreaPercent >= 100));
	if (p)
		return p;

	if (size > ModMemoryBlockSize &&
		(!_freeAreaPercent ||
		 (_freeAreaPercent < 100 &&
		  static_cast<double>(_freeAreaPercent) / 100 <
		  static_cast<double>(_allocated - _size) / _allocated)))

		// メモリープールで確保された領域のうち、
		// 使用済の領域が占める割合が多くなったので、
		// 使用済ブロックリストに登録されている領域をすべて解放する
		//
		//【注意】		オーバーフローを防ぐために浮動小数点数による
		//				計算を行っているため、誤差がある

		ModMemoryPool::freeBlocks(&_freeBlockList);
#endif
	if (_allocated + size > _limit) {

		// 指定されたサイズの領域を確保すると、
		// メモリープールの総確保サイズの上限を超える
		//
		//【注意】	呼び出し側でメモリー削減交渉をする余地を残すため、
		//			ここではエラー状態を設定しない

		ModMemoryErrorThrow(ModModuleMemory,
							ModMemoryErrorPoolLimitSize, ModErrorLevelError);
	}

	try {
		p = ModOsDriver::Memory::alloc(size, ModTrue);

	} catch (ModException) {

		//【注意】	呼び出し側でメモリー削減交渉をする余地を残すため、
		//			ここではエラー状態を設定しない

		ModMemoryErrorThrow(ModModuleMemory,
							ModMemoryErrorOsAlloc, ModErrorLevelError);
	}
#ifndef NO_CATCH_ALL
	catch (...) {
		ModUnexpectedThrow(ModModuleMemory);
	}
#endif

	_allocated += size;
	return p;
}

#ifdef MOD_USE_FREE_BLOCK_LIST
//	FUNCTION private
//	ModMemoryPool::freeBlocks --
//		使用済リストに登録されている使用済領域をすべて破棄する
//
//	NOTES
//
//	ARGUMENTS
//		ModMemoryCell**		head
//			破棄する使用済領域が登録されている使用済リストの先頭
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
ModMemoryPool::freeBlocks(ModMemoryCell** head)
{
	if (head) {
		ModMemoryCell*	next;
		for (ModMemoryCell*	m = *head; m; m = next) {
			next = m->next;
			_allocated -= m->size;
			ModOsDriver::Memory::free((void*) m);
		}
		*head = 0;
	}
}
#endif

//	FUNCTION private
//	ModMemoryPool::freeMemoryUnsafe --
//		メモリープールから確保した領域を実際に破棄する
//
//	NOTES
//		この関数の呼び出し側で、汎用ライブラリーの初期化を行い、
//		必要なスレッド間排他制御を行うこと
//
//	ARGUMENTS
//		void*				p
//			破棄する領域の先頭アドレス
//		ModSize				size
//			破棄する領域のサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
ModMemoryPool::freeMemoryUnsafe(void* p, ModSize size)
{
	if (p) {
		if (size > ModMemoryHalfBlockSize) {

			// 破棄する領域のサイズがブロックの半分より大きいとき、
#ifdef MOD_USE_FREE_BLOCK_LIST
			// ブロックの倍数単位のサイズで実際に破棄する

			size = (size + ModMemoryBlockSize - 1) /
				ModMemoryBlockSize * ModMemoryBlockSize;

			// 破棄する領域を使用済ブロックリストへつなぐ

			ModMemoryPool::chainBlock(&_freeBlockList,
									  p, size, ModMemoryBlockSize);
#else
			// そのままサイズのまま実際に破棄する

			_allocated -= size;
			ModOsDriver::Memory::free(p);
#endif
		} else {

			// 破棄するサイズがブロックの半分以下のとき

			size = _sizeToRound[size];

			// 使用済リストへその領域をつなぐ

			*(char**) p = *_sizeToBucket[size];
			*_sizeToBucket[size] = (char*) p;
		}
		_size -= size;
	}
}

#ifdef MOD_DEBUG
//	FUNCTION public
//	ModMemoryPool::print -- メモリープールの現在の状態を出力する
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

// static
void
ModMemoryPool::print()
{
	try {
		// 必要ならば汎用ライブラリーを初期化する

		ModCommonInitialize::checkAndInitialize();

		ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
		(void) m.lock();

		ModDebugMessage << "_limit:          " << _limit << ModEndl;
		ModDebugMessage << "_size:           " << _size << ModEndl;
		ModDebugMessage << "_allocated:      " << _allocated << ModEndl;
		ModDebugMessage << "_emergencyLimit: " << _emergencyLimit << ModEndl;
		ModDebugMessage << "_emergencySize:  " << _emergencySize << ModEndl;

	} catch (...) {

		// どんな例外が発生しても、エラーにしない

		ModErrorHandle::reset();
	}
}

//	FUNCTION public
//	ModMemoryPool::printList --
//		メモリープールの現在の領域管理リストの状態を出力する
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

// static
void
ModMemoryPool::printList()
{
	try {
		// 必要ならば汎用ライブラリーを初期化する

		ModCommonInitialize::checkAndInitialize();

		ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
		(void) m.lock();

		const int	column = 8;

		ModSize	rounded = sizeof(ModPtr);
		for (int i = 0; i < ModMemoryBucketNum; i++, rounded <<= 1) {
			ModDebugMessage << "_freeBucket[" << rounded << "]" << ModEndl;
			int	j = 0;
			for (char* p = _freeBucket[i]; p; p = *((char**) p)) {
				if (!(j++ % column))
					ModDebugMessage << ModEndl << "	    ";
				ModDebugMessage << (int) p << "->";
			}
			ModDebugMessage << "------" << ModEndl;
			ModDebugMessage << "count = " << j << ModEndl;
		}
#ifdef MOD_USE_FREE_BLOCK_LIST
		{
		ModDebugMessage << "_freeBlockList" << ModEndl;
		int	j = 0;
		for (ModMemoryCell* m = _freeBlockList; m; m = m->next) {
			if (!(j++ % column))
				ModDebugMessage << ModEndl << "    ";
			ModDebugMessage << "[" << (int) m << "] " << m->size << "->";
		}
		ModDebugMessage << "------" << ModEndl;
		}
#endif
		{
		ModDebugMessage << "_emergencyBlockList" << ModEndl;
		int	j = 0;
		for (ModMemoryCell* m = _emergencyBlockList; m; m = m->next) {
			if (!(j++ % column))
				ModDebugMessage << ModEndl << "    ";
			ModDebugMessage << "[" << (int) m << "] " << m->size << "->";
		}
		ModDebugMessage << "------" << ModEndl;
		}
	} catch (...) {

		// どんな例外が発生しても、エラーにしない

		ModErrorHandle::reset();
	}
}
#endif	// MOD_DEBUG
#else
//	FUNCTION public
//	ModMemoryPool::setTotalLimit --
//		メモリープールの総確保サイズの上限を設定する
//
//	NOTES
//		現在確保している領域の総サイズより大きい値へ
//		メモリープールの総確保サイズの上限を変更することができる
//
//	ARGUMENTS
//		ModSize			size
//			新しい上限(KB 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			指定された値が現在確保されている領域の総サイズよりも小さい

// static
void
ModMemoryPool::setTotalLimit(ModSize size)
{
	// 必要ならば汎用ライブラリーを初期化する

	ModCommonInitialize::checkAndInitialize();

	ModSize	bytes = ModUtility::kbyteToByte(size);

	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	(void) m.lock();

	if (bytes < _size) {

		// 現在確保されている領域の総サイズよりも
		// 小さくしようとしている

		ModThrow(ModModuleMemory,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}

	_limit = bytes;
}

//	FUNCTION private
//	ModMemoryPool::doInitialize -- メモリープールを実際に初期化する
//
//	NOTES
//		ModCommonInitialize::initialize から
//		ModMemoryHandle::initialize を通して、
//		一回だけ他にメインスレッドのみの状態で呼び出されることが保証されている
//
//	ARGUMENTS
//		ModSize				limit
//			メモリープールの総確保サイズの上限(KB 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
ModMemoryPool::doInitialize(ModSize limit)
{
	try {
		// まず、パラメーター値を得て初期化する

		ModParameter	parameter(ModFalse);	// 初期化中なので領域を
												// 確保することはないようにする
		if (limit)
			_limit = ModUtility::kbyteToByte(limit);
		else
			(void) parameter.getModSize(_limit, (const char*)
										"MemoryPoolTotalLimit");
		(void) parameter.getModSize(_emergencyLimit, (const char*)
									"MemoryPoolEmergencyLimit");

		// 非常用領域の上限を設定し、
		// その上限値で非常用領域を確保しておく

		ModMemoryPool::setEmergencyLimit(_emergencyLimit);

		// 初期化が終了した

		_initialized = ModTrue;

	} catch (ModException& exception) {
		ModRethrow(exception);
	}
#ifndef NO_CATCH_ALL
	catch (...) {
		ModUnexpectedThrow(ModModuleMemory);
	}
#endif
}

//	FUNCTION private
//	ModMemoryPool::doTerminate -- メモリープールを実際に後処理する
//
//	NOTES
//		ModCommonInitialize::terminate から
//		ModMemoryHandle::terminate を通して、
//		一回だけ他にメインスレッドのみの状態で呼び出されることが保証されている
//
//		特になにも処理しない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
ModMemoryPool::doTerminate()
{
	ModOsDriver::Memory::free( _emergencyBegin );
	_emergencyBegin = 0;
	_emergencyEnd = 0;
}

//	FUNCTION private
//	ModMemoryPool::allocateMemoryUnsafe --
//		メモリープールから指定されたサイズの領域を実際に確保する
//
//	NOTES
//		この関数の呼び出し側で、汎用ライブラリーの初期化を行い、
//		必要なスレッド間排他制御を行うこと
//
//	ARGUMENTS
//		ModSize				size
//			確保する領域のサイズ(B 単位)
//
//	RETURN
//		確保した領域の先頭アドレス
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			確保する領域のサイズとして 0 が指定された
//		ModMemoryErrorOverPoolLimit
//			確保する領域のサイズとして
//			メモリープールの総確保サイズの上限より大きい値が指定された
//		ModMemoryErrorPoolLimitSize
//			指定されたサイズの領域を確保すると、
//			メモリープールの総確保サイズの上限を超える
//			(ただし、エラー設定されない)
//		ModMemoryErrorOsAlloc
//			新たな領域を確保できない
//			(ただし、エラー設定されない)

// static
void*
ModMemoryPool::allocateMemoryUnsafe(ModSize size)
{
	if (!size) {

		// 確保する領域のサイズとして 0 が指定された

		ModThrow(ModModuleMemory,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}
	if (size > _limit) {

		// 確保する領域のサイズとして
		// メモリープールの総確保サイズの上限より大きい値が指定された

		ModThrow(ModModuleMemory,
				 ModMemoryErrorOverPoolLimit, ModErrorLevelError);
	}
	if (_size + size > _limit) {

		// 指定されたサイズの領域を確保すると、
		// メモリープールの総確保サイズの上限を超える
		//
		//【注意】	呼び出し側でメモリー削減交渉をする余地を残すため、
		//			ここではエラー状態を設定しない

		ModMemoryErrorThrow(ModModuleMemory,
							ModMemoryErrorPoolLimitSize, ModErrorLevelError,
							0);
	}

	void*	p;
	try {
		p = ModOsDriver::Memory::alloc(size, ModTrue);

	} catch (ModException&) {

		//【注意】	呼び出し側でメモリー削減交渉をする余地を残すため、
		//			ここではエラー状態を設定しない

		ModMemoryErrorThrow(ModModuleMemory,
							ModMemoryErrorOsAlloc, ModErrorLevelError,
							0);
	}
#ifndef NO_CATCH_ALL
	catch (...) {
		ModUnexpectedThrow(ModModuleMemory);
	}
#endif

	_size += size;
	return p;
}

//	FUNCTION private
//	ModMemoryPool::freeMemoryUnsafe --
//		メモリープールから確保した領域を実際に破棄する
//
//	NOTES
//		この関数の呼び出し側で、汎用ライブラリーの初期化を行い、
//		必要なスレッド間排他制御を行うこと
//
//	ARGUMENTS
//		void*				p
//			破棄する領域の先頭アドレス
//		ModSize				size
//			破棄する領域のサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
ModMemoryPool::freeMemoryUnsafe(void* p, ModSize size)
{
	if (p) {
		ModOsDriver::Memory::free(p);
		_size -= size;
	}
}

#ifdef MOD_DEBUG
//	FUNCTION public
//	ModMemoryPool::print -- メモリープールの現在の状態を出力する
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

// static
void
ModMemoryPool::print()
{
	try {
		// 必要ならば汎用ライブラリーを初期化する

		ModCommonInitialize::checkAndInitialize();

		ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
		(void) m.lock();

		ModDebugMessage << "_limit:          " << _limit << ModEndl;
		ModDebugMessage << "_size:           " << _size << ModEndl;
		ModDebugMessage << "_emergencyLimit: " << _emergencyLimit << ModEndl;
		ModDebugMessage << "_emergencySize:  " << _emergencySize << ModEndl;

	} catch (...) {

		// どんな例外が発生しても、エラーにしない

		ModErrorHandle::reset();
	}
}

//	FUNCTION public
//	ModMemoryPool::printList --
//		メモリープールの現在の領域管理リストの状態を出力する
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

// static
void
ModMemoryPool::printList()
{
	try {
		// 必要ならば汎用ライブラリーを初期化する

		ModCommonInitialize::checkAndInitialize();

		ModAutoMutex<ModOsMutex>	mu(ModCommonMutex::getMutex());
		(void) mu.lock();

		{
		const int	column = 8;

		ModDebugMessage << "_emergencyBlockList" << ModEndl;
		int	j = 0;
		for (ModMemoryCell* m = _emergencyBlockList; m; m = m->next) {
			if (!(j++ % column))
				ModDebugMessage << ModEndl << "    ";
			ModDebugMessage << "["
#ifdef MOD64
							<< (ModUInt64)m
#else
							<< (unsigned int)m
#endif
							<< "] " << m->size << "->";
		}
		ModDebugMessage << "------" << ModEndl;
		}
	} catch (...) {

		// どんな例外が発生しても、エラーにしない

		ModErrorHandle::reset();
	}
}
#endif	// MOD_DEBUG
#endif	// MOD_SELF_MEMORY_MANAGEMENT

//
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
