// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModMemoryPool.h -- メモリープール関連のクラスの定義
// 
// Copyright (c) 1997, 2002, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModMemoryPool_H__
#define __ModMemoryPool_H__

#include "ModCommonDLL.h"
#include "ModConfig.h"
#include "ModTypes.h"
#include "ModUtility.h"

//	MACRO
//	MOD_SELF_MEMORY_MANAGEMENT -- MOD 独自の再利用領域管理を行うか
//
//	NOTES
//		定義時に、ModMemoryPool で破棄された自由領域は、
//		MOD 独自に管理されて再利用される
//		未定義時には、ModMemoryPool は即 ::malloc(), ::free() を呼び出す

//	MACRO
//	MOD_USE_FREE_BLOCK_LIST --
//		ModMemoryBlockSize 以上のサイズの領域も
//		MOD 独自の再利用領域管理を行うか
//
//	NOTES
//		MOD_SELF_MEMORY_MANAGEMENT と共に定義されているとき、
//		ModMemoryBlockHalfSize より大きいサイズの領域も破棄時に
//		MOD 独自に管理されて再利用される
//		MOD_SELF_MEMORY_MANAGEMENT しか定義されていないとき、
//		ModMemoryBlockHalfSize 以下のサイズの領域のみ
//		MOD 独自に管理されて再利用される

#if MOD_CONF_MEMORY_MANAGEMENT == 0
#undef	MOD_SELF_MEMORY_MANAGEMENT
#undef	MOD_USE_FREE_BLOCK_LIST
#endif
#if MOD_CONF_MEMORY_MANAGEMENT == 1
#define	MOD_SELF_MEMORY_MANAGEMENT
#undef	MOD_USE_FREE_BLOCK_LIST
#endif
#if MOD_CONF_MEMORY_MANAGEMENT == 2
#define	MOD_SELF_MEMORY_MANAGEMENT
#define	MOD_USE_FREE_BLOCK_LIST
#endif
#if MOD_CONF_MEMORY_MANAGEMENT == 3
#undef	MOD_SELF_MEMORY_MANAGEMENT
#undef	MOD_USE_FREE_BLOCK_LIST
#endif

//	CONST
//	ModMemoryBlockSize -- OS の自由領域確保関数で確保する最小のサイズ
//
//	NOTES
//		このサイズより小さい領域は、このサイズの領域を確保してから、
//		それを分割し、残りの部分を使用済領域リストで管理する
//		この値を変更したとき、ModMemoryBucketNum も変更する必要がある

const ModSize	ModMemoryBlockSize = 1024;

#ifdef MOD_SELF_MEMORY_MANAGEMENT
//	CONST
//	ModMemoryHalfBlockSize -- OS の自由領域確保関数で確保する最小サイズの半分
//
//	NOTES
//		このサイズより大きい領域を確保しようとしたとき、
//		ModMemoryBlockSize の倍数単位に丸められたサイズで実際に確保する
//		逆に、このサイズ以下の領域を確保しようとしたとき、
//		ModMemoryBlockSize の領域を確保してから、
//		それを分割し、残りの部分を使用済領域リストで管理する

const ModSize	ModMemoryHalfBlockSize = ModMemoryBlockSize >> 1;

//	CONST
//	ModMemoryBucketNum -- 使用済領域リストを管理するための配列のサイズ
//
//	NOTES
//		使用済領域を管理するリストを登録しておくための配列のサイズ
//		sizeof(ModPtr) から ModMemoryHalfBlockSize までの
//		2 のべき乗の大きさの空き領域をそれぞれ管理するための
//		使用済領域リストを登録する必要がある
//		たとえば、ModMemoryLargeBlockSize が 1024 のとき、
//		使用済領域リストは 4, 8, 16, 32, 64, 128, 256, 512 の 8 つになる

const int		ModMemoryBucketNum = 8;
#endif

//	STRUCT
//	ModMemoryCell -- 使用済領域リストに登録するセルを表すクラス
//
//	NOTES
//		使用済領域リストへ登録する領域はこの構造体にキャストする

//【注意】	ライブラリ外に公開しないクラスなので dllexport しない

struct ModMemoryCell
{
	ModSize					size;				// 領域のサイズ(B 単位)
	ModMemoryCell*			next;				// 次のセル
};

//	CLASS
//	ModMemoryPool -- 自由記憶領域管理用プールを表すクラス
//
//	NOTES
//		自由記憶領域の管理用のさまざまなリストを管理し、
//		自由記憶領域操作用のインタフェースを提供する
//
#ifdef MOD_SELF_MEMORY_MANAGEMENT
//		仕様は G-BASE の MM ライブラリーを元にしている
//
//		確保可能な領域の総サイズを指定でき、
//		領域の確保、破棄ごとに ::malloc, ::free を呼び出すのではなく、
//		一度確保した領域を使用済リストで管理しながら、使いまわす
#else
//		確保可能な領域の総サイズを指定でき、
//		領域の確保、破棄ごとに ::malloc, ::free を呼び出す
#endif
//
//		初期化時に非常用領域をあらかじめ確保しておき、
//		以降の ::malloc の呼び出しに失敗したときに、
//		その領域を使えるようにする

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModMemoryPool
{
	friend class ModMemoryHandle;
public:
	ModCommonDLL
	static void				initialize(ModSize limit = 0);
												// 初期化
	static void				terminate();		// 後処理

	ModCommonDLL
	static void*			allocateMemory(ModSize size);
												// 領域の確保
	ModCommonDLL
	static void				freeMemory(void* p, ModSize size);
												// 領域の破棄

	ModCommonDLL
	static void*			allocateEmergencyMemory(ModSize size);
												// 非常用領域の確保
	ModCommonDLL
	static void				freeEmergencyMemory(void* p, ModSize size);
												// 非常用領域の破棄

	ModCommonDLL
	static void				setTotalLimit(ModSize size);
												// 確保可能総サイズの指定
	ModCommonDLL
	static void				setEmergencyLimit(ModSize size);
												// 非常用領域の総サイズの指定

	static ModSize			getCurrentSize();
												// 使用サイズを得る
	static ModSize			getLimitSize();
												// 確保可能サイズを得る

	static ModBoolean		isEmergencyMemory(const void* p);
												// 非常用領域かどうか
#ifdef MOD_SELF_MEMORY_MANAGEMENT
	static ModSize			getAllocatedSize();
												// 確保サイズを得る
#endif
private:
	static void				doInitialize(ModSize limit = 0);
												// 初期化下位関数
	static void				doTerminate();		// 後処理下位関数

	static void*			allocateMemoryUnsafe(ModSize size);
												// 領域確保下位関数
	static void				freeMemoryUnsafe(void* p, ModSize size);
												// 領域破棄下位関数

	static void*			searchBlock(ModMemoryCell** head, ModSize size,
										ModSize block, ModBoolean doDivide);
												// ブロックリストの探索
	static void				chainBlock(ModMemoryCell** head, void* p,
									   ModSize size, ModSize block);
												// ブロックリストへ登録
#ifdef MOD_DEBUG
	static void				print();			// 現状の出力
	static void				printList();		// ブロックリストの現状の出力
#endif
	static ModBoolean		_initialized;		// 初期化したか
	ModCommonDLL
	static ModSize			_limit;				// 確保可能総サイズ(B 単位)
	ModCommonDLL
	static ModSize			_size;				// 使用サイズ(B 単位)
	static ModSize			_emergencyLimit;	// 非常用領域総サイズ(B 単位)
	static ModSize			_emergencySize;		// 非常用領域使用サイズ(B 単位)
	static const ModSize	_emergencyBlockSize;
												// 非常用領域の
												// ブロックサイズ(B 単位)
	static ModMemoryCell*	_emergencyBlockList;
												// 非常用領域管理リスト
	ModCommonDLL
	static void*			_emergencyBegin;	// 非常用領域の先頭アドレス
	ModCommonDLL
	static void*			_emergencyEnd;		// 非常用領域の末尾アドレス
#ifdef MOD_SELF_MEMORY_MANAGEMENT
	static void*			allocateBlock(ModSize size);
												// ブロックの確保

	ModCommonDLL
	static ModSize			_allocated;			// 確保済サイズ(B 単位)
	static ModSize			_sizeToRound[ModMemoryHalfBlockSize + 1];
												// 確保サイズを丸めるための配列
	static char**			_sizeToBucket[ModMemoryHalfBlockSize + 1];
												// 確保サイズから
												// 管理リストを得るための配列
	static char*			_freeBucket[ModMemoryBucketNum];
												// 管理リストを格納する配列
#ifdef MOD_USE_FREE_BLOCK_LIST
	static void				freeBlocks(ModMemoryCell** head);
												// 全使用済ブロックの破棄

	static unsigned long	_freeAreaPercent;	// 保持する使用済領域の割合
	static ModMemoryCell*	_freeBlockList;		// 使用済領域管理リスト
#endif
#endif
};

//	FUNCTION public
//	ModMemoryPool::terminate -- メモリープールを後処理する
//
//	NOTES
//		なにもしないが、昔のものとの互換性のために残しておく
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
inline
void
ModMemoryPool::terminate()
{ }

//	FUNCTION public
//	ModMemoryPool::getCurrentSize --
//		メモリープールで現在使用されている領域の総サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた総使用サイズ(B 単位)
//
//	EXCEPTIONS
//		なし

// static
inline
ModSize
ModMemoryPool::getCurrentSize()
{
	return _size;
}

#ifdef MOD_SELF_MEMORY_MANAGEMENT
//	FUNCTION public
//	ModMemoryPool::getAllocatedSize --
//		メモリープール内部で実際に確保されている領域の総サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた確保総サイズ(B 単位)
//
//	EXCEPTIONS
//		なし

// static
inline
ModSize
ModMemoryPool::getAllocatedSize()
{
	return _allocated;
}
#endif

//	FUNCTION public
//	ModMemoryPool::getLimitSize -- メモリープールの確保可能総サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた確保可能総サイズ(B 単位)
//
//	EXCEPTIONS
//		なし

// static
inline
ModSize
ModMemoryPool::getLimitSize()
{
	return ModUtility::byteToKbyte(_limit);
}

//	FUNCTION public
//	ModMemoryPool::isEmergencyMemory --
//		指定されたアドレスが非常用領域の先頭アドレスか調べる
//
//	NOTES
//
//	ARGUMENTS
//		void*				p
//			調べるアドレス
//
//	RETURN
//		ModTrue
//			非常用領域の先頭アドレスである
//		ModFalse
//			非常用領域の先頭アドレスでない
//
//	EXCEPTIONS
//		なし

// static
inline
ModBoolean
ModMemoryPool::isEmergencyMemory(const void* p)
{
	return (_emergencyEnd <= p || p < _emergencyBegin) ? ModFalse : ModTrue;
}

#endif	// __ModMemoryPool_H__

//
// Copyright (c) 1997, 2002, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
