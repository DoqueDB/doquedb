// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Memory.cpp -- メモリ関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Os";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

extern "C" 
{
#ifdef SYD_OS_WINDOWS
#include <windows.h>
#include <malloc.h>
#include <assert.h>
#endif
#ifdef SYD_OS_POSIX
#include <assert.h>
#include <stdlib.h>
#include <sys/mman.h>
#ifdef SYD_OS_SOL8
#else
#include <fcntl.h>
#endif
#endif
}

// すべてのプラットフォームで利用する
#define USE_MEMORY_LIST

#include "Os/Assert.h"
#include "Os/AutoCriticalSection.h"
#include "Os/FakeError.h"
#include "Os/Manager.h"
#include "Os/Memory.h"
#include "Os/SysConf.h"
#include "Os/Unicode.h"

#ifdef USE_MEMORY_LIST
#include "Common/DoubleLinkedList.h"
#include "Common/LargeVector.h"
#endif

#include "Exception/MemoryExhaust.h"
#include "Exception/SystemCall.h"
#include "Exception/Unexpected.h"

#include "ModAlgorithm.h"

_TRMEISTER_USING
_TRMEISTER_OS_USING

//	MACRO
//	_TRMEISTER_OS_THROW -- システムコールのエラーを表す例外を投げる
//
//	NOTES

#define	_TRMEISTER_OS_THROW(func, osErrno)	\
	_Memory::throwOsError(func, osErrno, srcFile, __LINE__)

namespace
{

namespace _Literal
{
	// 擬似エラー関係

#ifdef SYD_FAKE_ERROR
	const char func_allocate_error_NotEnoughMemory[] = "Os::Memory::allocate_NotEnoughMemory";
	const char func_map_error_NotEnoughMemory[] = "Os::Memory::map_NotEnoughMemory";
#endif

#define	U(literal)	const UnicodeString	literal(#literal)

	// 関数名関係

	U(malloc);
#ifdef SYD_OS_WINDOWS
	U(VirtualAlloc);
#endif
#ifdef SYD_OS_POSIX
	U(mmap);
	U(open);
#endif

#undef U
}

namespace _Memory
{
#ifdef USE_MEMORY_LIST
	// サイズをある単位に切り上げる
	Memory::Size			roundUp(Memory::Size n, Memory::Size unit);
	// サイズをある単位に切り上げる
	Memory::Size			roundUp2(Memory::Size n, Memory::Size unit);
	// ポインターをある単位に切り捨てる
	void*					roundDown(void* p, Memory::Size unit);

	// メモリの予約単位(B 単位)
#ifdef SYD_ARCH64
	const Memory::Size		_UnitSize =	8 << 10 << 10;	// 8MB
#else
	const Memory::Size		_UnitSize =	64 << 10;		// 64KB
#endif

	// システム確保領域(B 単位)
#ifdef SYD_OS_SOLARIS
	const Memory::Size		_SystemReserveSize = 16 << 10;
#else
	const Memory::Size		_SystemReserveSize = 0;
#endif

	// 管理対象のサイズ(B 単位)
#ifdef SYD_OS_WINDOWS
	const Memory::Size		_MaxSize = 32 << 10;
#else
	const Memory::Size		_MaxSize = 64 << 10;
#endif

	// 与えられた数値の最下位ビットの位置を求めるための配列
	unsigned char			_BitPosition[] =
	{
		0,1,2,1,3,1,2,1, 4,1,2,1,3,1,2,1,	//	 0 -  15
		5,1,2,1,3,1,2,1, 4,1,2,1,3,1,2,1,	//	16 -  31
		6,1,2,1,3,1,2,1, 4,1,2,1,3,1,2,1,	//	32 -  47
		5,1,2,1,3,1,2,1, 4,1,2,1,3,1,2,1,	//	48 -  63
		7,1,2,1,3,1,2,1, 4,1,2,1,3,1,2,1,	//	64 -  79
		5,1,2,1,3,1,2,1, 4,1,2,1,3,1,2,1,	//	80 -  95
		6,1,2,1,3,1,2,1, 4,1,2,1,3,1,2,1,	//	96 - 111
		5,1,2,1,3,1,2,1, 4,1,2,1,3,1,2,1,	// 112 - 127
		8,1,2,1,3,1,2,1, 4,1,2,1,3,1,2,1,	// 128 - 143
		5,1,2,1,3,1,2,1, 4,1,2,1,3,1,2,1,	// 144 - 159
		6,1,2,1,3,1,2,1, 4,1,2,1,3,1,2,1,	// 160 - 175
		5,1,2,1,3,1,2,1, 4,1,2,1,3,1,2,1,	// 176 - 191
		7,1,2,1,3,1,2,1, 4,1,2,1,3,1,2,1,	// 192 - 207
		5,1,2,1,3,1,2,1, 4,1,2,1,3,1,2,1,	// 208 - 223
		6,1,2,1,3,1,2,1, 4,1,2,1,3,1,2,1,	// 224 - 239
		5,1,2,1,3,1,2,1, 4,1,2,1,3,1,2,1	// 240 - 255
	};
	
	// 情報を保護するためのラッチ
	CriticalSection			_latch;

	// メモリユニット管理用の構造体
	struct _UnitInfo
	{
		// コンストラクター
		_UnitInfo(Memory::Size uiPageSize_)
			: m_pBuffer(0), m_pBitMap(0), m_iBitMapPosition(0),
			  m_uiPageSize(uiPageSize_), m_uiUsedSize(0),
			  m_pListPrev(0), m_pListNext(0)
			{
			}
		// コンストラクター
		//
		//【注意】
		// 検索するときにのみ利用
		_UnitInfo(char* p_)
			: m_pBuffer(p_), m_uiPageSize(0)
			{
			}
		// デストラクタ
		~_UnitInfo() {}

		// 比較クラス
		struct Less {
			bool operator () (const _UnitInfo* a, const _UnitInfo* b)
				{
					return (a->m_pBuffer < b->m_pBuffer);
				}
		};

		// 利用中かどうか
		bool isUsed() const { return m_uiUsedSize != 0; }
		// 使い切っているかどうか
		bool isFull() const
			{
				return (m_uiUsedSize ==
						(_UnitSize -
						 roundUp(_SystemReserveSize, m_uiPageSize))) ?
					true : false;
			}

		// メモリを確保する
		void	allocate();
		// メモリを開放する
		void	deallocate();

		// ページを得る
		char*	next(bool reset_);
		// ページを開放する
		void	release(char* pBuffer_);

		// 確保したメモリユニットへのポインタ
		char*			m_pBuffer;
		// 空き領域を管理するビットマップ
		unsigned char*	m_pBitMap;
		// 現在参照しているビットマップの位置
		int				m_iBitMapPosition;
		// ページサイズ(B単位)
		const Memory::Size	m_uiPageSize;
		// 現在利用しているトータルサイズ
		Memory::Size	m_uiUsedSize;

		// フリーリストでの直前の要素へのポインタ
		_UnitInfo*		m_pListPrev;
		// フリーリストでの直後の要素へのポインタ
		_UnitInfo*		m_pListNext;
	};

	// メモリーユニットをアドレスの昇順に格納するための配列
	typedef Common::LargeVector<_UnitInfo*>			_Vector;
	_Vector		_cUnitInfoVector;

	// ユニット内に空き領域があるメモリーユニットを管理するリスト
	typedef Common::DoubleLinkedList<_UnitInfo>		_List;
	_List		_cFreeList4K(&_UnitInfo::m_pListPrev, &_UnitInfo::m_pListNext);
	_List		_cFreeList8K(&_UnitInfo::m_pListPrev, &_UnitInfo::m_pListNext);
	_List		_cFreeList16K(&_UnitInfo::m_pListPrev, &_UnitInfo::m_pListNext);
	_List		_cFreeList32K(&_UnitInfo::m_pListPrev, &_UnitInfo::m_pListNext);
	_List		_cFreeList64K(&_UnitInfo::m_pListPrev, &_UnitInfo::m_pListNext);

	// メモリーユニットを検索する
	_Vector::Iterator		lowerBound(_Vector& v, _UnitInfo* p);
	_Vector::Iterator		upperBound(_Vector& v, _UnitInfo* p);

#else
#ifdef SYD_OS_WINDOWS
	// サイズをある単位に切り上げる
	Memory::Size			roundUp(Memory::Size n, Memory::Size unit);
	// ポインターをある単位に切り捨てる
	void*					roundDown(void* p, Memory::Size unit);

	// システムのメモリページサイズ(B 単位)
	//【注意】	実際のページサイズと一致している必要がある
	const Memory::Size		_PageSize =		4 << 10;
	// メモリの予約単位(B 単位)
	const Memory::Size		_ReserveUnit =	64 << 10;
	
	// プロセスの仮想アドレスの最大値(B 単位)
	//
	// 【注意】
	//	32ビットWindowsのアプリ領域は0x7FFF0000(2GB)まで、しかし
	//	LARGEADDRESSAWAREでリンクしたアプリを64ビットWindowsで実行すると、
	//	0xFFFF0000(4GB)まで利用できる。_MapInfo::_bitmapの大きさは128KB。
	//	
	//
	//	ちなみに、64ビットWindowsのアプリ領域は0x7FFFFFF0000(8TB)まで。
	//	_MapInfo::_bitmapをunsigned charにして(最小のページを8KBにして)も、
	//	8TBすべての領域を管理するためには128MB必要
	const Memory::Size		_AddressMax =	0xffff0000;

	// 以下の情報を保護するためのラッチ
	CriticalSection			_latch;

	struct _MapInfo
	{
		// コンストラクター
		_MapInfo();

		// 仮想アドレス空間中のページがコミット中かを管理する
		// ビットマップにビットを立てる
		const char*			bitOn(const char* rounded, const char* p,
								  const char* head, const char* tail);
		// 仮想アドレス空間中のページがコミット中かを管理する
		// ビットマップからビットを落とす
		bool				bitOff(const char* rounded, const char* p,
								   const char* head, const char* tail);

		// 仮想アドレス空間中のページがコミット中かを管理するビットマップ
		unsigned short		_bitmap[_AddressMax / _ReserveUnit];

		// サイズごとに用意する
		// 最後に予約した領域のうち、
		// コミットされていない領域の先頭を含む予約単位の先頭
		char*				_rounded[_ReserveUnit / _PageSize];
		// 最後に予約した領域のうち、コミットされていない領域の先頭
		char*				_head[_ReserveUnit / _PageSize];
		// 最後に予約した領域の末尾
		char*				_tail[_ReserveUnit / _PageSize];
	}						_mapInfo;
#endif
#endif
	// システムコールのエラーを表す例外を投げる
	void
	throwOsError(const UnicodeString& func,
#ifdef SYD_OS_WINDOWS
				 DWORD osErrno,
#endif
#ifdef SYD_OS_POSIX
				 int osErrno,
#endif
				 const char* srcFile, int line);
}

#if defined(USE_MEMORY_LIST) || defined(SYD_OS_WINDOWS)
//	FUNCTION
//	$$$::_Memory::roundUp -- サイズをある単位に切り上げる
//
//	NOTES
//
//	ARGUMENTS
//		Os::Memory::Size	n
//			切り上げるサイズ
//		Os::Memory::Size	unit
//			単位
//
//	RETURN
//		切り上げ後のサイズ
//
//	EXCEPTIONS
//		なし

inline
Memory::Size
_Memory::roundUp(Memory::Size n, Memory::Size unit)
{
	return n + (unit - 1) & ~(unit - 1);
}

//	FUNCTION
//	$$$::_Memory::roundDown -- ポインターをある単位に切り捨てる
//
//	NOTES
//
//	ARGUMENTS
//		void* p
//			切り捨てるポインター
//		Os::Memory::Size	unit
//			単位
//
//	RETURN
//		切り捨て後のポインター
//
//	EXCEPTIONS
//		なし

inline
void*
_Memory::roundDown(void* p, Memory::Size unit)
{
	return syd_reinterpret_cast<void*>(
		syd_reinterpret_cast<ModUPtr>(p) & ~(static_cast<ModUPtr>(unit) - 1));
}
#endif

#ifdef USE_MEMORY_LIST
//	FUNCTION
//	$$$::_Memory::roundUp2 -- サイズを単位の2N乗に切り上げる
//
//	NOTES
//
//	ARGUMENTS
//		Os::Memory::Size	n
//			切り上げるサイズ
//		Os::Memory::Size	unit
//			単位
//
//	RETURN
//		切り上げ後のサイズ
//
//	EXCEPTIONS
//		なし

inline
Memory::Size
_Memory::roundUp2(Memory::Size n, Memory::Size unit)
{
	while (unit < n) unit <<= 1;
	return unit;
}


//	FUNCTION public
//	$$$::_Memory::_UnitInfo::allocate -- メモリを確保する
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
_Memory::_UnitInfo::allocate()
{
	// ユニットのサイズを求める
	
	size_t size = _UnitSize - roundUp(_SystemReserveSize, m_uiPageSize);
	
#ifdef SYD_OS_WINDOWS
	
	// 予約だけ行う
	
	m_pBuffer = static_cast<char*>(
		::VirtualAlloc(0, size,
					   MEM_RESERVE | MEM_TOP_DOWN,
					   PAGE_NOACCESS));
	
	if (m_pBuffer == 0) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_OS_THROW(_Literal::VirtualAlloc, osErrno);
	}
#endif
#ifdef SYD_OS_POSIX

	// 仮想アドレス空間上に匿名のページを確保する
	// Solaris or Linux 2.4 以上

	m_pBuffer = static_cast<char*>(
		::mmap(0, size, PROT_WRITE | PROT_READ,
			   MAP_PRIVATE | MAP_ANON, -1, 0));

	if (m_pBuffer == MAP_FAILED) {
		
		// システムコールのエラーを表す例外を投げる

		const int osErrno = errno;
		_TRMEISTER_OS_THROW(_Literal::mmap, osErrno);
	}
#endif

	try {

		ModSize s = (_UnitSize / m_uiPageSize + 7) / 8;
		
		// 空き領域を管理するビットマップを初期化する

		m_pBitMap = static_cast<unsigned char*>(Os::Memory::allocate(s));
		Os::Memory::set(m_pBitMap, 0xff, s);

		// _SystemReserveSize 分のビットを落とす
		// _SystemReserveSize は m_uiPageSize * 8 よりも小さいことが前提

		int n = static_cast<int>(
			roundUp(_SystemReserveSize, m_uiPageSize) / m_uiPageSize);
		unsigned char* p = m_pBitMap + s - 1;
		for (int i = 0; i < n; ++i)
			*p &= ~(static_cast<unsigned char>(1) << (7-i));

		// 現在参照しているビットマップの位置を初期化する

		m_iBitMapPosition = 0;

		// 使用サイズを初期化する

		m_uiUsedSize = 0;
	}
	catch (...)
	{
		deallocate();
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	$$$::_Memory::_UnitInfo::deallocate -- メモリを開放する
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
_Memory::_UnitInfo::deallocate()
{
#ifdef SYD_OS_WINDOWS
	
	// エラーは無視する

	(void) ::VirtualFree(m_pBuffer, 0, MEM_RELEASE);
	m_pBuffer = 0;
#endif
#ifdef SYD_OS_POSIX

	// ユニットのサイズを求める
	
	size_t size = _UnitSize - roundUp(_SystemReserveSize, m_uiPageSize);
	
	// エラーは無視する

	(void) ::munmap(m_pBuffer, size);
	m_pBuffer = 0;
#endif

	// 空き領域管理ビットマップを削除する
	void* p = m_pBitMap;
	Os::Memory::free(p);
	m_pBitMap = 0;

	//【注意】
	// クラスがデストラクトされるだけなので、その他の変数は初期化しない
}

//	FUNCTION public
//	$$$::_Memory::_UnitInfo::next -- ページを得る
//
//	NOTES
//
//	ARGUMENTS
//		bool				reset_
//			true
//				確保した領域を 0 埋めする
//			false
//				確保した領域を 0 埋めしない
//
//	RETURN
//		char*
//			確保されたページの先頭アドレス
//
//	EXCEPTIONS

char*
_Memory::_UnitInfo::next(bool reset_)
{
	if (isFull())

		// 空き領域は存在していない

		return 0;

	// 空いているブロックまで進める

	while (m_iBitMapPosition < static_cast<int>(_UnitSize / m_uiPageSize / 8) &&
		   m_pBitMap[m_iBitMapPosition] == 0)

		++m_iBitMapPosition;

	// ビット位置を求める
	
	int pos = _BitPosition[m_pBitMap[m_iBitMapPosition]] - 1;

	// ポインタを得る

	char* p = m_pBuffer + m_uiPageSize * (m_iBitMapPosition * 8 + pos);

#ifdef SYD_OS_WINDOWS
	
	// メモリを割り当てる
	
	p = static_cast<char*>(
		::VirtualAlloc(p, m_uiPageSize, MEM_COMMIT, PAGE_READWRITE));
	if (p == 0)
	{
		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_OS_THROW(_Literal::VirtualAlloc, osErrno);
	}
#endif

	// ビットをOFFする

	m_pBitMap[m_iBitMapPosition] &= ~(static_cast<unsigned char>(1) << pos);

	if (reset_)

		// 確保した領域を 0 埋めする

		Memory::reset(p, m_uiPageSize);

	// 利用サイズを増やす

	m_uiUsedSize += m_uiPageSize;

	return p;
}

//	FUNCTION public
//	$$$::_Memory::_UnitInfo::release -- メモリを開放する
//
//	NOTES
//
//	ARGUMENTS
//		char* pBuffer_
//			開放するページの先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_Memory::_UnitInfo::release(char* pBuffer_)
{
	// ユニットのサイズを求める
	
	size_t size = _UnitSize - roundUp(_SystemReserveSize, m_uiPageSize);

	// 範囲内か確認する
	
	if (pBuffer_ < m_pBuffer || pBuffer_ > (m_pBuffer + size))
		_SYDNEY_THROW0(Exception::Unexpected);
	
	// ビット位置を求める
	
	int pos = static_cast<int>((pBuffer_ - m_pBuffer) / m_uiPageSize);

#ifdef SYD_OS_WINDOWS
	
	// メモリを解放する

	::VirtualFree(pBuffer_, m_uiPageSize, MEM_DECOMMIT);
	
#endif

	// ビットをONする

	m_pBitMap[pos / 8] |= (static_cast<unsigned char>(1) << (pos % 8));

	if (m_iBitMapPosition > pos / 8)

		// m_iBitMapPositionより前の領域を開放する場合には
		// m_iBitMapPositionもその位置に変更する

		m_iBitMapPosition = (pos / 8);
	
	// 利用サイズを減らす

	m_uiUsedSize -= m_uiPageSize;
}

//	FUNCTION local
//	$$$::_Memory::lowerBound -- メモリーユニットを取得する
//
//	NOTES
//
//	ARGUMENTS
//		_Memory::_Vector& v_
//			メモリーユニットが格納された配列
//		_Memory::_UnitInfo* p_
//			メモリーユニットのアドレス
//
//	RETURN
//		_Memory::_Vector::Iterator
//			ヒットしたメモリーユニットへのイテレータ
//
//	EXCEPTIONS

_Memory::_Vector::Iterator
_Memory::lowerBound(_Memory::_Vector& v_, _Memory::_UnitInfo* p_)
{
	return ModLowerBound(v_.begin(), v_.end(), p_, _UnitInfo::Less());
}

//	FUNCTION local
//	$$$::_Memory::upperBound -- メモリーユニットを取得する
//
//	NOTES
//
//	ARGUMENTS
//		_Memory::_Vector& v_
//			メモリーユニットが格納された配列
//		_Memory::_UnitInfo* p_
//			メモリーユニットのアドレス
//
//	RETURN
//		_Memory::_Vector::Iterator
//			ヒットしたメモリーユニットへのイテレータ
//
//	EXCEPTIONS

_Memory::_Vector::Iterator
_Memory::upperBound(_Memory::_Vector& v_, _Memory::_UnitInfo* p_)
{
	return ModUpperBound(v_.begin(), v_.end(), p_, _UnitInfo::Less());
}

#else

//	FUNCTION public
//	$$$::_Memory::_MapInfo::_MapInfo --
//		メモリ関連の制御情報を初期化するクラスのコンストラクター
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

#ifdef SYD_OS_WINDOWS
inline
_Memory::_MapInfo::_MapInfo()
{
	// 仮想アドレス空間中のページがコミットされているかを
	// 管理するためのビットマップを初期化する

	Memory::reset(_bitmap, sizeof(_bitmap));

	// 最後に予約した領域の情報を初期化する
	
	Memory::reset(_rounded, sizeof(_rounded));
	Memory::reset(_head, sizeof(_head));
	Memory::reset(_tail, sizeof(_tail));
}
#endif

#ifdef SYD_OS_WINDOWS
//	FUNCTION public
//	$$$::_Memory::_MapInfo::bitOn --
//		仮想アドレス空間中のページがコミット中かを管理する
//		ビットマップにビットを立てる
//
//	NOTES
//
//	ARGUMENTS
//		char*&				rounded
//			コミットした領域を含む予約領域の先頭へのポインタ
//		char*				p
//			コミットした領域の先頭へのポインタ
//		char*				head
//			コミットした領域の末尾へのポインタ
//		char*				tail
//			コミットした領域を含む予約領域の末尾へのポインタ
//
//	RETURN
//		コミットした領域を含む予約領域の最後の予約単位の先頭へのポインタ
//
//	EXCEPTIONS
//		なし

inline
const char*
_Memory::_MapInfo::bitOn(const char* rounded, const char* p,
						 const char* head, const char* tail)
{
	; _TRMEISTER_ASSERT(rounded <= p);
	; _TRMEISTER_ASSERT(rounded + _ReserveUnit <= tail);
	; _TRMEISTER_ASSERT(p < head);
	; _TRMEISTER_ASSERT(head <= tail);
	; _TRMEISTER_ASSERT(head + _ReserveUnit > tail);

	const unsigned short mask = ~static_cast<unsigned short>(0);

	//	  rounded, p                               head    tail
	//		├───────┬───────┬───┼───┤
	//		│              │              │      │      │
	//		└───────┴───────┴───┴───┘
	//		├──────── size ────────┤
	//		├── unit ──┼── unit ──┼── unit ──┤

	for (; rounded + _ReserveUnit < head; p = (rounded += _ReserveUnit)) {
		unsigned short& v = _bitmap[
			syd_reinterpret_cast<Memory::Size>(rounded) / _ReserveUnit];
		; _TRMEISTER_ASSERT(v == 0);
		v = mask;
	}

	//  rounded p     head tail
	//		├─┼───┼─┤
	//		│  │      │  │
	//		└─┴───┴─┘
	//		    ├ size ┤
	//		├── unit ──┤

	_bitmap[syd_reinterpret_cast<Memory::Size>(rounded) / _ReserveUnit] |=
		((mask >> (p - rounded) / _PageSize) &
		 (mask << (tail - head) / _PageSize));

	return rounded;
}

//	FUNCTION public
//	$$$::_Memory::_MapInfo::bitOff --
//		仮想アドレス空間中のページがコミット中かを管理する
//		ビットマップからビットを落とす
//
//	NOTES
//
//	ARGUMENTS
//		char*&				rounded
//			デコミットした領域を含む予約領域の先頭へのポインタ
//		char*				p
//			デコミットした領域の先頭へのポインタ
//		char*				head
//			デコミットした領域の末尾へのポインタ
//		char*				tail
//			デコミットした領域を含む予約領域の末尾へのポインタ
//
//	RETURN
//		true
//			デコミットした領域を含む予約領域内に
//			コミットされているページがまだ存在する
//		false
//			デコミットした領域を含む予約領域内に
//			コミットされているページはない
//
//	EXCEPTIONS
//		なし

inline
bool
_Memory::_MapInfo::bitOff(const char* rounded, const char* p,
						  const char* head, const char* tail)
{
	; _TRMEISTER_ASSERT(rounded <= p);
	; _TRMEISTER_ASSERT(rounded + _ReserveUnit <= tail);
	; _TRMEISTER_ASSERT(p < head);
	; _TRMEISTER_ASSERT(head <= tail);
	; _TRMEISTER_ASSERT(head + _ReserveUnit > tail);

	const unsigned short mask = ~static_cast<unsigned short>(0);

	//	  rounded, p                               head    tail
	//		├───────┬───────┬───┼───┤
	//		│              │              │      │      │
	//		└───────┴───────┴───┴───┘
	//		├──────── size ────────┤
	//		├── unit ──┼── unit ──┼── unit ──┤

	for (; rounded + _ReserveUnit < head; p = (rounded += _ReserveUnit)) {
		unsigned short& v = _bitmap[
			syd_reinterpret_cast<Memory::Size>(rounded) / _ReserveUnit];
		; _TRMEISTER_ASSERT(v == mask);
		v = 0;
	}

	//  rounded p     head tail
	//		├─┼───┼─┤
	//		│  │      │  │
	//		└─┴───┴─┘
	//		    ├ size ┤
	//		├── unit ──┤

	return _bitmap[syd_reinterpret_cast<Memory::Size>(rounded) / _ReserveUnit] ^=
		((mask >> (p - rounded) / _PageSize) &
		 (mask << (tail - head) / _PageSize));
}
#endif
#endif

//	FUNCTION
//	$$$::_Memory::throwOsError -- システムコールのエラーを表す例外を投げる
//
//	NOTES
//		与えられたエラー番号が NO_ERROR のとき、
//		例外を投げないことに注意する必要がある
//
//	ARGUMENTS
//		Os::UnicodeString&	func
//			エラーになったシステムコールの名前
#ifdef SYD_OS_WINDOWS
//		DWORD				osErrno
#endif
#ifdef SYD_OS_POSIX
//		int					osErrno
#endif
//			エラー番号
//		char*				srcFile
//			エラーになったシステムコールを呼び出したソースファイルの名前
//		int					line
//			エラーになったシステムコールを呼び出したソースファイルの行
//
//	RETURN
//		なし
//
//	EXCETIONS

void
_Memory::throwOsError(const UnicodeString& func,
#ifdef SYD_OS_WINDOWS
					  DWORD osErrno,
#endif
#ifdef SYD_OS_POSIX
					  int osErrno,
#endif
					  const char* srcFile, int line)
{
	switch (osErrno) {
#ifdef SYD_OS_WINDOWS
//  0 :		The operation completed successfully.
	case NO_ERROR:

		// エラーが起きていない

		break;

//	8 :		Not enough storage is available to process this command.
	case ERROR_NOT_ENOUGH_MEMORY:
#endif
#ifdef SYD_OS_POSIX
	case ENOMEM:
#endif
		// 必要なメモリが利用可能でない

		throw Exception::MemoryExhaust(moduleName, srcFile, line);

	default:

		// システムコールでエラーが起きた

		throw Exception::SystemCall(moduleName, srcFile, line, func, osErrno);
	}
}

}

//	FUNCTION private
//	Os::Manager::Memory::initialize --
//		マネージャーの初期化のうち、メモリ関連のものを行う
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
Manager::Memory::initialize()
{}

//	FUNCTION private
//	Os::Manager::Memory::terminate --
//		マネージャーの後処理のうち、メモリ関連のものを行う
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

// static
void
Manager::Memory::terminate()
{
#ifndef USE_MEMORY_LIST
#ifdef SYD_OS_WINDOWS
#ifdef DEBUG

	// 仮想アドレス空間上に確保した領域のうち、
	// 開放されていないものがないことを確認する

	unsigned int i = 0;
	do {
		; _TRMEISTER_ASSERT(!_Memory::_mapInfo._bitmap[i]);
	} while (++i < _Memory::_AddressMax / _Memory::_ReserveUnit) ;
#endif
#endif
#endif
}

//	FUNCTION
//	Os::Memory::allocate -- ヒープ領域上にメモリーを確保する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Memory::Size	size
//			確保するメモリーのサイズ(B 単位)
//
//	RETURN
//		確保されたメモリーの先頭アドレス
//
//	EXCEPTIONS

void*
Memory::allocate(Size size)
{
	// 条件を満たせば、メモリが枯渇したことを表す例外を発生させる

	_TRMEISTER_OS_FAKE_ERROR(
		_Literal::func_allocate_error_NotEnoughMemory,
		Exception::MemoryExhaust);

	void* p = ::malloc(size);
	if (p == 0) {

		// システムコールのエラーを表す例外を投げる
		//
		//【注意】	マニュアルを読む限り、
		//			エラー時に errno をちゃんと設定するのは、Solaris のみである
#ifdef SYD_OS_WINDOWS
		const int osErrno = ERROR_NOT_ENOUGH_MEMORY;
#endif
#ifdef SYD_OS_POSIX
#ifdef SYD_OS_SOL8
		const int osErrno = errno;
#else
		const int osErrno = ENOMEM;
#endif
#endif
		_TRMEISTER_OS_THROW(_Literal::malloc, osErrno);
	}
	return p;
}

//	FUNCTION
//	Os::Memory::free -- ヒープ領域上に確保されているメモリーを解放する
//
//	NOTES
//
//	ARGUMENTS
//		void*&				p
//			解放するメモリーの先頭アドレスのリファレンス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Memory::free(void*& p)
{
	//【注意】	allocate と free は両方を inline かそうでないかを合わせること
	//
	//			そうしないと、Microsoft Visual C++ は
	//			バージョンによって CRT を提供する DLL が異なるので、
	//			allocate と free で異なる DLL を呼び出し、
	//			正しく領域管理できない

	if (p)
		::free(p), p = 0;
}

//	FUNCTION
//	Os::Memory::copy -- メモリー内容をコピーする
//
//	NOTES
//		あるサイズのメモリー内容を、ある位置へコピーする
//		コピー先の位置がコピーするメモリー内容に含まれていたときは
//		正しく動作しない
//
//	ARGUMENTS
//		void*				dst
//			コピー先の先頭アドレス
//		void*				src
//			内容をコピーするメモリーの先頭アドレス
//		Os::Memory::Size	size
//			コピーするメモリー内容のサイズ(B 単位)
//
//	RETURN
//		dst
//
//	EXCEPTIONS
//		なし

void*
Memory::copy(void* dst, const void* src, Size size)
{
#ifdef SYD_OS_WINDOWS
	::CopyMemory(dst, src, size);
	return dst;
#endif
#ifdef SYD_OS_POSIX
	return ::memcpy(dst, src, size);
#endif
}

//	FUNCTION
//	Os::Memory::move -- メモリー内容を移動する
//
//	NOTES
//		あるサイズのメモリー内容を、ある位置へ移動する
//		移動先の位置が移動するメモリー内容に含まれていても正しく動作する
//
//	ARGUMENTS
//		void*				dst
//			移動先の先頭アドレス
//		void*				src
//			内容を移動するメモリーの先頭アドレス
//		Os::Memory::Size	size
//			移動するメモリー内容のサイズ(B 単位)
//
//	RETURN
//		dst
//
//	EXCEPTIONS
//		なし

void*
Memory::move(void* dst, const void* src, Size size)
{
#ifdef SYD_OS_WINDOWS
	::MoveMemory(dst, src, size);
	return dst;
#endif
#ifdef SYD_OS_POSIX
	return ::memmove(dst, src, size);
#endif
}

//	FUNCTION
//	Os::Memory::reset -- メモリーを 0 埋めする
//
//	NOTES
//		指定された領域を 1 バイトづつ 0 で埋めていく
//
//	ARGUMENTS
//		void*				p
//			0 埋めするメモリーの先頭アドレス
//		Os::Memory::Size	size
//			0 埋めするメモリーのサイズ(B 単位)
//
//	RETURN
//		p
//
//	EXCEPTIONS
//		なし

void*
Memory::reset(void* p, Size size)
{
#ifdef SYD_OS_WINDOWS
	::ZeroMemory(p, size);
	return p;
#endif
#ifdef SYD_OS_POSIX
	return ::memset(p, 0, size);
#endif
}

//	FUNCTION
//	Os::Memory::set -- メモリーを特定の値で初期化する
//
//	NOTES
//		指定された領域を 1 バイトづつ与えられた文字で埋めていく
//
//	ARGUMENTS
//		void*				p
//			初期化するメモリーの先頭アドレス
//		unsigned char		c
//			メモリー中の 1 バイトごとを初期化する値
//		Os::Memory::Size	size
//			初期化するメモリーのサイズ(B 単位)
//
//	RETURN
//		dst
//
//	EXCEPTIONS
//		なし

void*
Memory::set(void* p, unsigned char c, Size size)
{
#ifdef SYD_OS_WINDOWS
	::FillMemory(p, size, c);
	return p;
#endif
#ifdef SYD_OS_POSIX
	return ::memset(p, c, size);
#endif
}

//	FUNCTION
//	Os::Memory::compare -- メモリー内容を比較する
//
//	NOTES
//
//	ARGUMENTS
//		void*				l
//			r に与えられたメモリーと比較するメモリーの先頭アドレス
//		void*				r
//			l に与えられたメモリーと比較するメモリーの先頭アドレス
//		Os::Memory::Size	size
//			与えられたメモリーの先頭から比較するサイズ(B 単位)
//
//	RETURN
//		0
//			l と r のメモリー内容はまったく等しい
//		-1
//			l より r のほうが辞書順で大きい
//		1
//			l より r のほうが辞書順で小さい
//
//	EXCEPTIONS
//		なし

int
Memory::compare(const void* l, const void* r, Size size)
{
	return ::memcmp(l, r, size);
}

//	FUNCTION
//	Os::Memory::map -- 仮想アドレス空間上の領域を確保する
//
//	NOTES
//		仮想アドレス空間上のある領域に対して、物理ストレージを割り当てる
//
//	ARGUMENTS
//		Os::Memory::Size	size
//			確保する領域のサイズ(B 単位)
//		bool				reset
//			true
//				確保した領域を 0 埋めする
//			false
//				確保した領域を 0 埋めしない
//
//	RETURN
//		確保された領域の先頭アドレス
//
//	EXCEPTIONS

void*
Memory::map(Size size, bool reset)
{
	// 条件を満たせば、システムコールエラーの例外を発生させる

	_TRMEISTER_OS_FAKE_ERROR(
		_Literal::func_map_error_NotEnoughMemory, Exception::MemoryExhaust);

#ifdef USE_MEMORY_LIST

	void*	p = 0;

	if (size <= _Memory::_MaxSize) {
		
		// 指定されたサイズをシステムのメモリページサイズの2N乗に切り上げる
		//
		//【注意】
		// Os::SysConf::PageSize::get() は 4K 以上でかつ、2N乗の値であること

		size = _Memory::roundUp2(size, Os::SysConf::PageSize::get());

		// 対象のフリーリストを得る

		_Memory::_List* pList = 0;

		switch (size >> 10) {
		case 4:  pList = &_Memory::_cFreeList4K;  break;
		case 8:  pList = &_Memory::_cFreeList8K;  break;
		case 16: pList = &_Memory::_cFreeList16K; break;
		case 32: pList = &_Memory::_cFreeList32K; break;
		case 64: pList = &_Memory::_cFreeList64K; break;
		}

		; _TRMEISTER_ASSERT(pList);

		_Memory::_UnitInfo* pInfo = 0;

		// ここで排他制御用のクリティカルセクションをロックする

		Os::AutoCriticalSection cAuto(_Memory::_latch);

		while (p == 0) {

			if (pList->isEmpty() == true) {

				// 存在していないので、新しく確保する

				pInfo = new _Memory::_UnitInfo(size);

				// バッファを確保する

				try {
					pInfo->allocate();
				} catch (...) {
					delete pInfo;
					_SYDNEY_RETHROW;
				}

				// 配列に加える

				_Memory::_Vector::Iterator i
					= _Memory::upperBound(_Memory::_cUnitInfoVector, pInfo);
				_Memory::_cUnitInfoVector.insert(i, pInfo);

				// リストに加える

				pList->pushFront(*pInfo);
				
			} else {

				// 存在しているので、一番先頭の要素を得る

				pInfo = &(pList->getFront());
			}

			// 空き領域を取得する
		
			p = pInfo->next(reset);

			if (pInfo->isFull())

				// この UnitInfo には空きがないので、リストから削除する

				pList->popFront();

		}

	} else {
		
		// 指定されたサイズをシステムのメモリページサイズの倍数に切り上げる

		size = _Memory::roundUp(size, Os::SysConf::PageSize::get());
	
#ifdef SYD_OS_WINDOWS
	
		// 指定されたサイズの領域を予約し、コミットする

		p = ::VirtualAlloc(0, size,
						   MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN,
						   PAGE_READWRITE);
	
		if (p == 0) {

			// システムコールのエラーを表す例外を投げる

			const DWORD osErrno = ::GetLastError();
			_TRMEISTER_OS_THROW(_Literal::VirtualAlloc, osErrno);
		}
#endif
#ifdef SYD_OS_POSIX
#ifdef SYD_OS_SOLARIS
	
		// Solaris の mmap は 64KB 単位でしか確保できない
		// しかも管理用の領域として 16KB が必要で、64KB mmap すると、
		// 128 KB の領域を消費してしまう。(Solaris10のman参照)
		// そのため、Solarisでは malloc を利用する
	
		p = allocate(size);
	
#else

		// 仮想アドレス空間上に匿名のページを確保する
		// Solaris or Linux 2.4 以上

		p = ::mmap(0, size, PROT_WRITE | PROT_READ,
						 MAP_PRIVATE | MAP_ANON, -1, 0);

		if (p == MAP_FAILED) {

			// システムコールのエラーを表す例外を投げる

			const int osErrno = errno;
			_TRMEISTER_OS_THROW(_Literal::mmap, osErrno);
		}

#endif
	
		if (reset)

			// 確保した領域を 0 埋めする

			Memory::reset(p, size);
#endif
	}
#else
#ifdef SYD_OS_WINDOWS

	// 指定されたサイズをシステムのメモリページサイズの倍数に切り上げる

	size = _Memory::roundUp(size, _Memory::_PageSize);

	char*	p;

	if (size > _Memory::_ReserveUnit) {

		// 予約単位より大きい領域の割り当てが要求されたので、
		// 指定されたサイズの領域を予約し、コミットする
		//
		//【注意】	予約した領域の一部分の予約の解除はできない

		p = static_cast<char*>(
			::VirtualAlloc(0, size,
						   MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN,
						   PAGE_READWRITE));
		if (p == 0) {

			// システムコールのエラーを表す例外を投げる

			const DWORD osErrno = ::GetLastError();
			_TRMEISTER_OS_THROW(_Literal::VirtualAlloc, osErrno);
		}

		// メモリ管理関連の情報を保護するためにラッチをかける

		AutoCriticalSection	latch(_Memory::_latch);

		// 仮想アドレス空間中のページがコミット中かを管理するビットマップに
		// コミットした領域中のページに対応するビットを立てる

		(void) _Memory::_mapInfo.bitOn(
			p, p,
			p + size, p + _Memory::roundUp(size, _Memory::_ReserveUnit));
	} else {

		int n = static_cast<int>(size / _Memory::_PageSize) - 1;

		// メモリ管理関連の情報を保護するためにラッチをかける

		AutoCriticalSection	latch(_Memory::_latch);

		if (static_cast<Size>(_Memory::_mapInfo._tail[n]
							  - _Memory::_mapInfo._head[n]) < size) {

			// 最後に予約した領域のうち、
			// コミットされていない部分では要求されたサイズに足りないので、
			// 新たに仮想アドレス空間上の領域を予約する
			//
			//【注意】	::VirtualAlloc は必ず 64 KB 単位で領域を予約するので、
			//			予約と同時にコミットしていると、確保するサイズを
			//			64 KB で割ったときの余りのぶんの領域はコミットできない
			//
			//			そこで、プロセスのアドレス空間をページ単位に分割し、
			//			ページをコミットしたかをビットマップで管理する
			//
			//			最後に予約した領域に、確保するサイズぶんの
			//			コミットされていない領域があれば、新たに領域を
			//			予約せずにその領域をコミットして返すことにする

			p = static_cast<char*>(
				::VirtualAlloc(0, _Memory::_ReserveUnit,
							   MEM_RESERVE | MEM_TOP_DOWN, PAGE_NOACCESS));
			if (p == 0) {

				// システムコールのエラーを表す例外を投げる

				const DWORD osErrno = ::GetLastError();
				_TRMEISTER_OS_THROW(_Literal::VirtualAlloc, osErrno);
			}

			_Memory::_mapInfo._rounded[n] = _Memory::_mapInfo._head[n] = p;
			_Memory::_mapInfo._tail[n] = p + _Memory::_ReserveUnit;
		}

		// 最後に予約した領域のコミットされていない部分から、
		// 要求されたサイズの領域をコミットする
		//
		//【注意】	::VirtualAlloc は必ずシステムの
		//			メモリページサイズ単位で領域をコミットする

		p = static_cast<char*>(
			::VirtualAlloc(_Memory::_mapInfo._head[n], size,
						   MEM_COMMIT, PAGE_READWRITE));
		if (p == 0) {

			// システムコールのエラーを表す例外を投げる

			const DWORD osErrno = ::GetLastError();
			_TRMEISTER_OS_THROW(_Literal::VirtualAlloc, osErrno);
		}

		_Memory::_mapInfo._head[n] += size;

		// 仮想アドレス空間中のページがコミット中かを管理するビットマップに
		// コミットした領域中のページに対応するビットを立てる

		_Memory::_mapInfo._rounded[n] = const_cast<char*>(
			_Memory::_mapInfo.bitOn(
				_Memory::_mapInfo._rounded[n], p,
				_Memory::_mapInfo._head[n], _Memory::_mapInfo._tail[n]));
	}

	//【注意】	::VirtualAlloc は 0 埋めした領域を確保するので、
	//			自分で 0 埋めする必要はない
#endif
#ifdef SYD_OS_POSIX
#ifdef SYD_OS_SOLARIS
	
	// Solaris の mmap は 64KB 単位でしか確保できない
	// しかも管理用の領域として 16KB が必要で、64KB mmap すると、
	// 128 KB の領域を消費してしまう。(Solaris10のman参照)
	// そのため、Solarisでは malloc を利用する
	
	void* p = allocate(size);
	
#else

	// 仮想アドレス空間上に匿名のページを確保する
	// Solaris or Linux 2.4 以上

	void* p = ::mmap(0, size, PROT_WRITE | PROT_READ,
					 MAP_PRIVATE | MAP_ANON, -1, 0);

	if (p == MAP_FAILED) {

		// システムコールのエラーを表す例外を投げる

		const int osErrno = errno;
		_TRMEISTER_OS_THROW(_Literal::mmap, osErrno);
	}
#endif
	
	if (reset)

		// 確保した領域を 0 埋めする

		Memory::reset(p, size);
#endif
#endif
	return p;
}

//	FUNCTION
//	Os::Memory::unmap -- 仮想アドレス空間上の領域を解放する
//
//	NOTES
//		仮想アドレス空間上のある領域に対して、
//		物理ストレージの割り当てを解除する
//
//	ARGUMENTS
//		void*				p
//			解放する領域の先頭アドレスのリファレンス
//		Os::Memory::Size	size
//			解放する領域のサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Memory::unmap(void*& p, Size size)
{
#ifdef USE_MEMORY_LIST
	if (p && size) {

		if (size <= _Memory::_MaxSize) {

			char* q = syd_reinterpret_cast<char*>(p);
			
			// 指定されたサイズをシステムのメモリページサイズの2N乗に切り上げる
			//
			//【注意】
			// Os::SysConf::PageSize::get() は 4K 以上でかつ、2N乗の値であること

			size = _Memory::roundUp2(size, Os::SysConf::PageSize::get());

			// ここで排他制御用のクリティカルセクションをロックする

			Os::AutoCriticalSection cAuto(_Memory::_latch);

			// 配列からUnitInfoを取得する

			_Memory::_UnitInfo u(q);
			_Memory::_Vector::Iterator i =
				_Memory::upperBound(_Memory::_cUnitInfoVector, &u);
			--i;
			_Memory::_UnitInfo* pInfo = *i;

			// フリーリストに登録されているか確認する

			bool isList = (pInfo->isFull() ? false : true);

			// 領域を開放する

			pInfo->release(q);

			if (isList == false || pInfo->isUsed() == false) {

				// 対象のフリーリストを得る

				_Memory::_List* pList = 0;

				switch (size >> 10) {
				case 4:  pList = &_Memory::_cFreeList4K;  break;
				case 8:  pList = &_Memory::_cFreeList8K;  break;
				case 16: pList = &_Memory::_cFreeList16K; break;
				case 32: pList = &_Memory::_cFreeList32K; break;
				case 64: pList = &_Memory::_cFreeList64K; break;
				}

				; _TRMEISTER_ASSERT(pList);

				if (isList == false)

					// フリーリストにないので登録する

					pList->pushBack(*pInfo);

				else if (pInfo->isUsed() == false) {

					// すべての領域が開放されたので、リストから削除して
					// UnitInfo を開放する

					_Memory::_cUnitInfoVector.erase(i);
					pList->erase(*pInfo);
				
					pInfo->deallocate();
					delete pInfo;
				}
			}
			
		} else {
#ifdef SYD_OS_WINDOWS
			// 指定された領域を解放する
			//
			//【注意】	エラーは無視する

			(void) ::VirtualFree(p, 0, MEM_RELEASE);
#endif
#ifdef SYD_OS_POSIX
#ifdef SYD_OS_SOLARIS
			// 指定された領域を開放する
			
			free(p);
#else
			// 指定されたサイズをシステムのメモリページサイズの倍数に切り上げる

			size = _Memory::roundUp(size, Os::SysConf::PageSize::get());

			// 指定されたサイズの領域を解放する
			//
			//【注意】	エラーは無視する

			(void) ::munmap(
#if defined(SYD_OS_SOLARIS) && !((_POSIX_C_SOURCE > 2) || defined(_XPG4_2))
				static_cast<caddr_t>(p)
#else
				p
#endif
				, size);
#endif
#endif			
		}

		p = 0;
	}
#else
#ifdef SYD_OS_WINDOWS
	if (p && size) {

		char*	q = syd_reinterpret_cast<char*>(p);

		// 指定されたサイズをシステムのメモリページサイズの倍数に切り上げる

		size = _Memory::roundUp(size, _Memory::_PageSize);

		if (size > _Memory::_ReserveUnit) {

			// 予約単位より大きい領域の割り当ての解除が要求された
			{
			// メモリ管理関連の情報を保護するためにラッチをかける

			Os::AutoCriticalSection	latch(_Memory::_latch);

			// 仮想アドレス空間中のページがコミット中かを管理する
			// ビットマップから解放する領域のページに対応するビットを落とす

			(void) _Memory::_mapInfo.bitOff(
				q, q,
				q + size, q + _Memory::roundUp(size, _Memory::_ReserveUnit));
			}
			// 指定されたサイズの領域を解放する
			//
			//【注意】	エラーは無視する

			(void) ::VirtualFree(q, 0, MEM_RELEASE);

		} else {

			// 指定された領域を含む予約領域の先頭のポインタを求める

			char* reserved = syd_reinterpret_cast<char*>(
				_Memory::roundDown(q, _Memory::_ReserveUnit));
			const char* tail = reserved + _Memory::_ReserveUnit;

			int n = static_cast<int>(size / _Memory::_PageSize) - 1;

			// メモリ管理関連の情報を保護するためにラッチをかける

			Os::AutoCriticalSection	latch(_Memory::_latch);

			// 仮想アドレス空間中のページがコミット中かを管理する
			// ビットマップから解放する領域のページに対応するビットを落とす

			if (_Memory::_mapInfo.bitOff(reserved, q, q + size, tail))

				// 指定された領域を含む予約領域には、
				// 指定された領域のほかにコミットされたページがあるので、
				// 指定された領域をデコミットする
				//
				//【注意】	エラーは無視する

				(void) ::VirtualFree(q, size, MEM_DECOMMIT);
			else {
				// 指定された領域を含む予約領域には、
				// コミットされたページがなくなるので、予約領域を解放する
				//
				//【注意】	エラーは無視する

				(void) ::VirtualFree(reserved, 0, MEM_RELEASE);

				if (_Memory::_mapInfo._tail[n] == tail)

					// 最後に予約した領域を解放したので、
					// それに関する情報を初期化する

					_Memory::_mapInfo._rounded[n] =
					_Memory::_mapInfo._head[n] = _Memory::_mapInfo._tail[n] = 0;
			}
		}

		p = 0;
	}
#endif
#ifdef SYD_OS_POSIX
	if (p && size) {

#ifdef SYD_OS_SOLARIS
		
		// Solaris の mmap は 64KB 単位でしか確保できない(Solaris10のman参照)
		// そのため、Solarisでは malloc を利用する

		free(p);
#else

		//【注意】	エラーは無視する

		(void) ::munmap(
#if defined(SYD_OS_SOLARIS) && !((_POSIX_C_SOURCE > 2) || defined(_XPG4_2))
			static_cast<caddr_t>(p)
#else
			p
#endif
			, size);

		p = 0;
#endif
	}
#endif
#endif
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
