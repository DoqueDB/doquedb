// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Count.cpp -- ロック数関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Lock";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Lock/Count.h"
#include "Lock/Config.h"
#include "Lock/Manager.h"

#include "Common/Assert.h"
#include "Common/DoubleLinkedList.h"
#include "Common/HashTable.h"
#include "Os/AutoCriticalSection.h"
#include "Os/Memory.h"

_SYDNEY_USING
_SYDNEY_LOCK_USING

namespace
{

namespace _Count
{
	//	TYPEDEF
	//	$$$::_Count::_HashTable --
	//		すべてのロック数を管理するハッシュ表の型

	typedef Common::HashTable<
		Common::DoubleLinkedList<Count>, Count> _HashTable;

	// 生成済のロック数を探す
	Count*
	find(_HashTable::Bucket& bucket, const Count& count);

	// ロックモードごとのロックの有無を表すビットマップから
	// 存在するすべてのロックの最小上界を求めるための配列を初期化する
	void
	initializeLeastUpperBound();

	// ロックモードごとのロックの有無を表すビットマップから
	// 存在するすべてのロックの最小上界を求めるための配列
	Mode::Value				_leastUpperBound[Count::Bit::Mask + 1];

	// 以下の情報を保護するためのラッチ
	Os::CriticalSection		_latch;

	// すべてのロック数を管理するハッシュ表
	_HashTable*				_countTable = 0;
	_HashTable::Length		_countTableLength = 0;
}

//	FUNCTION
//	$$$::_Count::find -- 生成済のロック数を探す
//
//	NOTES
//
//	ARGUMENTS
//		$$$::_Count::_HashTable::Bucket&	bucket
//			ロック数が格納されるべきハッシュ表のバケット
//		Count&				src
//			探しているロック数と等しいロック数
//
//	RETURN
//		0 以外の値
//			得られたロック数を格納する領域の先頭アドレス
//		0
//			見つからなかった
//
//	EXCEPIONS
//		なし

Count*
_Count::find(_HashTable::Bucket& bucket, const Count& src)
{
	//【注意】	呼び出し側で _Count::_latch をラッチする必要がある

	switch (bucket.getSize()) {
	default:
	{
		// バケットに登録されているロック数のうち、
		// 与えられたものとまったく同じロックを記録するものを探す

		//_Count::_HashTable::Bucket::Iterator		begin(bucket.begin());
		//_Count::_HashTable::Bucket::Iterator		ite(begin);
		_Count::_HashTable::Bucket::Iterator	ite = bucket.begin();
		const _Count::_HashTable::Bucket::Iterator&	end = bucket.end();

		do {
			Count& count = *ite;
			if (count == src) {

				// 見つかったロック数をバケットの先頭に移動して
				// 最近に参照されたものほど、見つかりやすくする

				bucket.splice(bucket.begin(), bucket, ite);

				return &count;
			}
		} while (++ite != end) ;

		break;
	}
	case 1:
	{
		Count& count = bucket.getFront();
		if (count == src) return &count;

		break;
	}
	case 0:
		break;
	}

	// 見つからなかった

	return 0;
}

//	FUNCTION
//	$$$::_Count::initializeLeastUpperBound --
//		ロックモードごとのロックの有無を表すビットマップから
//		存在するすべてのロックの最小上界を求めるための配列を初期化する
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

void
_Count::initializeLeastUpperBound()
{
	for (int index = 0; index < Count::Bit::Mask; index++) {

		Mode::Value	mode = Mode::N;
		Count::Bit::Value mask = Count::Bit::VX;

		do {
			if (index & mask) {
				int	appendNum = 0;
				for (int bitValue = static_cast<int>(mask);
					 bitValue != Count::Bit::VIS; bitValue >>= 1, appendNum++);

				Mode::Value	appendMode =
					static_cast<Mode::Value>(Mode::VIS + appendNum);
				mode = Mode::getLeastUpperBound(mode, appendMode);
			}
			if (mask == Count::Bit::VIS) break;

			mask = static_cast<Count::Bit::Value>(mask >> 1);
		} while (true) ;

		_leastUpperBound[index] = mode;
	}
}

}

//	FUNCTION private
//	Manager::Count::initialize --
//		マネージャーの初期化のうち、ロック数関連のものを行う
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
Manager::Count::initialize()
{
	_Count::initializeLeastUpperBound();
	Lock::Count::initializeCountTable();
}

//	FUNCTION private
//	Manager::Count::terminate --
//		マネージャーの後処理のうち、ロック数関連のものを行う
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
Manager::Count::terminate()
{
	if (_Count::_countTable) {

		// すべてのロック数を管理する
		// ハッシュ表のバケットはすべて空であるべき

		; _SYDNEY_ASSERT(_Count::_countTable->isEmpty());

		// すべてのロック数を管理するハッシュ表を破棄する

		delete _Count::_countTable, _Count::_countTable = 0;
	}
}

// static
void
Count::initializeCountTable()
{
	_Count::_countTable = new _Count::_HashTable(Config::CountTableSize::get(),
												 &Count::_hashPrev,
												 &Count::_hashNext);
	; _SYDNEY_ASSERT(_Count::_countTable);
	_Count::_countTableLength = _Count::_countTable->getLength();
}

//	FUNCTION public
//	Lock::Count::attach -- ロック数を得る
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Count&		src
//			得ようとしているロック数と等しいロック数
//
//	RETURN
//		得られたロック数を格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
Count*
Count::attach(const Count& src)
{
	// ロック数の生成・破棄に関する情報を
	// 保護するためのラッチをかける

	Os::AutoCriticalSection latch(_Count::_latch);

//	if (!_Count::_countTable) {

		// すべてのロック数を管理する
		// ハッシュ表が確保されていないので、まず、確保する

//		_Count::_countTable =
//			new _Count::_HashTable(
//				Config::CountTableSize::get(), &_hashPrev, &_hashNext);
//		; _SYDNEY_ASSERT(_Count::_countTable);
//		_Count::_countTableLength = _Count::_countTable->getLength();
//	}

	// 与えられたロック数を格納すべきハッシュ表のバケットを求める

	const Mode::Value lub = src.getLeastUpperBound();
	const unsigned int hashValue =
		((lub == Mode::N) ? 0 : src._value[lub]) + (src._bitmap << 16);
	; _SYDNEY_ASSERT(_Count::_countTable);
	const unsigned int addr = hashValue % _Count::_countTableLength;	//_Count::_countTable->getLength();
	_Count::_HashTable::Bucket& bucket = _Count::_countTable->getBucket(addr);

	// 与えられたロック数とまったく同じロックを記録するものが
	// 求めたバケットに登録されていれば、それを得る

	Count* count = _Count::find(bucket, src);
	if (count) {

		// 見つかった

		// 参照回数を 1 増やす

		++count->_refCount;
	} else {

		// 見つからなかったので、生成する

		count = new Count(src);
		; _SYDNEY_ASSERT(count);

		// 参照回数を 1 にする

		count->_refCount = 1;

		// ハッシュ表のバケットの先頭に挿入して、
		// 最近に参照されたものほど、見つかりやすくする

		bucket.pushFront(*count);
	}

	return count;
}

//	FUNCTION public
//	Lock::Count::attach -- ロック数の参照数を 1 増やす
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		参照数が 1 増えたロック数を格納する領域の先頭アドレス
//
//	EXCEPTIONS

Count*
Count::attach()
{
	// ロック数の生成・破棄に関する情報を保護するためにラッチする

	Os::AutoCriticalSection latch(_Count::_latch);

	// 参照回数を 1 増やす

	++_refCount;

	return this;
}

//	FUNCTION public
//	Lock::Count::detach -- ロック数の参照をやめる
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Count*&		count
//			参照をやめるロック数を格納する領域の先頭アドレスで、
//			呼び出しから返ると 0 になる
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Count::detach(Count*& count)
{
	if (count) {
		{
		// ロック数の生成・破棄に関する情報を保護するためにラッチする

		Os::AutoCriticalSection latch(_Count::_latch);

		// 参照回数が 0 より大きければ 1 減らす

		if (count->_refCount && --count->_refCount) {

			// 他から参照されているので、破棄できない

			count = 0;

		} else {

			// どこからも参照されなくなったので、破棄できる

			// 破棄するロック数を格納するバケットを求め、そこから取り除く

			const Mode::Value lub = count->getLeastUpperBound();
			const unsigned int hashValue =
				((lub == Mode::N) ?
				 0 : count->_value[lub]) + (count->_bitmap << 16);
			; _SYDNEY_ASSERT(_Count::_countTable);
			const unsigned int addr =
				hashValue %	_Count::_countTableLength;	//_Count::_countTable->getLength();
			_Count::_HashTable::Bucket& bucket =
				_Count::_countTable->getBucket(addr);
			bucket.erase(*count);
		}

		// ここで、ロック数の生成・破棄に関する
		// 情報を保護するためのラッチがはずれる
		}
		if (count) {

			// どこからも参照されていないロック数を破棄し、
			// 与えられたポインタは 0 を指すようにする

			delete count, count = 0;
		}
	}
}

#ifdef OBSOLETE
//	FUNCTION public
//	Lock::Count::detach -- ロック数の参照数を 1 減らす
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
Count::detach()
{
	// ロック数の生成・破棄に関する情報を保護するためにラッチする

	Os::AutoCriticalSection latch(_Count::_latch);
#ifdef DEBUG
	; _SYDNEY_ASSERT(_refCount);
#else
	if (_refCount)
#endif
		// 参照数が 0 より大きければ 1 減らす

		--_refCount;
}

//	FUNCTION public
//	Lock::Count::getRefCount -- ロック数の参照数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた参照数
//
//	EXCEPTIONS

unsigned int
Count::getRefCount() const
{
	// ロック数の生成・破棄に関する情報を保護するためにラッチする

	Os::AutoCriticalSection latch(_Count::_latch);

	return _refCount;
}
#endif

//	FUNCTION public
//	Lock::Count::operator = -- = 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Count&	r
//			自分自身に代入するロック数
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS
//		なし

Count&
Count::operator =(const Count& r)
{
	if (this != &r) {
		if ((_bitmap = r._bitmap) && r._value) {
			(void) Os::Memory::copy(_value, r._value, sizeof(_value));
		} else {
			clear();
		}
	}
	return *this;
}

//	FUNCTION public
//	Lock::Count::operator == -- == 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Count&	r
//			自分自身と比較するロック数
//
//	RETURN
//		true
//			自分自身と与えられたロック数は等しい
//		false
//			等しくない
//
//	EXCEPTIONS
//		なし

bool
Count::operator ==(const Count& r) const
{
	return _bitmap == r._bitmap &&
		!(_bitmap && Os::Memory::compare(_value, r._value, sizeof(_value)));
}

#ifdef OBSOLETE
//	FUNCTION public
//	Lock::Count::operator += -- += 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Count&		r
//			自分自身へ加えるロック数
//
//	RETURN
//		加算後の自分自身
//
//	EXCEPTIONS
//		なし

Count&
Count::operator +=(const Count&	r)
{
	if (r._bitmap) {
		if (_bitmap) {
			_bitmap |= r._bitmap;
			for (Mode::Value mode = Mode::VIS;
				 mode < Mode::N;
				 mode = static_cast<Mode::Value>(mode + 1))
				_value[mode] += r._value[mode];
		} else {
			*this = r;
		}
	}

	return *this;
}
#endif

//	FUNCTION public
//	Lock::Count::operator -= -- -= 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Count&		r
//			自分自身から減らすロック数
//
//	RETURN
//		減算後の自分自身
//
//	EXCEPTIONS
//		なし

Count&
Count::operator -=(const Count& r)
{
	if (r._bitmap && _bitmap) {
		for (Mode::Value mode = Mode::VIS;
			 mode < Mode::N; mode = static_cast<Mode::Value>(mode + 1)) {
			Value n = r._value[mode];
			(void) down(mode, n);
		}
	}

	return *this;
}

//	FUNCTION public
//	Lock::Count::substitute -- あるロック数を代入したロック数を得る
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Count*&		l
//			あるロック数を代入するロック数が
//			格納された領域の先頭アドレスで、
//			呼出し後、得られたロック数を格納する領域の先頭アドレスが設定される
//		Lock::Count&		r
//			代入するロック数
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Count::substitute(Count*& l, const Count& r)
{
	if (l) {
		Count* p = attach(r);
		detach(l);
		l = p;
	}
}

//	FUNCTION public
//	Lock::Count::up -- あるロックモードのロック数を増やす
//
//	NOTES
//		Lock::Mode::N を与えたときの動作は不定である
//
//	ARGUMENTS
//		Lock::Mode::Value	mode
//			ロック数を増やすロックのモード
//		Lock::Count::Value	n
//			指定されたとき
//				増やすロック数
//			指定されないとき
//				1 が指定されたものとみなす
//
//	RETURN
//		指定されたロックモードの加算後のロック数
//
//	EXCEPTIONS
//		なし

Count::Value
Count::up(Mode::Value mode, Value n)
{
	if (n) {
		_bitmap |= Bit::VIS << mode;
		_value[mode] += n;
	}
	return _value[mode];
}

//	FUNCTION public
//	Lock::Count::up -- あるロックモードのロック数を増やしたロック数を得る
//
//	NOTES
//		Lock::Mode::N を与えたときの動作は不定である
//
//	ARGUMENTS
//		Lock::Count*&		l
//			あるロックモードのロック数を増やしたいロック数が
//			格納された領域の先頭アドレスで、
//			呼出し後、得られたロック数を格納する領域の先頭アドレスが設定される
//		Lock::Mode::Value	mode
//			ロック数を増やすロックのモード
//		Lock::Count::Value	n
//			指定されたとき
//				増やすロック数
//			指定されないとき
//				1 が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Count::up(Count*& l, Mode::Value mode, Value n)
{
	if (l && n) {
		Count tmp(*l);
		(void) tmp.up(mode, n);

		Count* p = attach(tmp);
		detach(l);
		l = p;
	}
}

#ifdef OBSOLETE
//	FUNCTION public
//	Lock::Count::up -- あるロック数を増やしたロック数を得る
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Count*&		l
//			あるロック数を増やしたいロック数が
//			格納された領域の先頭アドレスで、
//			呼出し後、得られたロック数を格納する領域の先頭アドレスが設定される
//		Lock::Count&		r
//			増やすロック数
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Count::up(Count*& l, const Count& r)
{
	if (l) {
		Count tmp(*l);
		tmp += r;

		Count* p = attach(tmp);
		detach(l);
		l = p;
	}
}
#endif

//	FUNCTION public
//	Lock::Count::down --
//		あるロックモードのロック数を任意の数減らす
//
//	NOTES
//		Lock::Mode::N を与えたときの動作は不定である
//
//	ARGUMENTS
//		Lock::Mode::Value	mode
//			ロック数を減らすロックのモード
//		Lock::Count::Value&	n
//			減らすロック数。
//			呼び出し後は実際に減らしたロック数が記録される
//
//	RETURN
//		指定されたロックモードの減算後のロック数
//
//	EXCEPTIONS
//		なし

Count::Value
Count::down(Mode::Value mode, Value& n)
{
	if (n) {
		if (_value[mode] < n) {
			n = _value[mode];
		}
		if (!(_value[mode] -= n)) {
			_bitmap &= ~(Bit::VIS << mode);
		}
	}
	return _value[mode];
}

//	FUNCTION public
//	Lock::Count::down --
//		あるロックモードのロック数を任意の数減らしたロック数を得る
//
//	NOTES
//		Lock::Mode::N を与えたときの動作は不定である
//
//	ARGUMENTS
//		Lock::Count*&		l
//			あるロックモードのロック数を任意の数減らしたいロック数が
//			格納された領域の先頭アドレスで、
//			呼出し後、得られたロック数を格納する領域の先頭アドレスが設定される
//		Lock::Mode::Value	mode
//			ロック数を減らすロックのモード
//		Lock::Count::Value&	n
//			減らすロック数。
//			呼び出し後は実際に減らしたロック数が記録される
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Count::down(Count*& l, Mode::Value mode, Value& n)
{
	if (l && n) {
		Count tmp(*l);
		(void) tmp.down(mode, n);

		Count* p = attach(tmp);
		detach(l);
		l = p;
	}
}

//	FUNCTION public
//	Lock::Count::down -- あるロック数を減らしたロック数を得る
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Count*&		l
//			あるロック数を減らしたいロック数が
//			格納された領域の先頭アドレスで、
//			呼出し後、得られたロック数を格納する領域の先頭アドレスが設定される
//		Lock::Count&		r
//			減らすロック数
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Count::down(Count*& l, const Count& r)
{
	if (l) {
		Count tmp(*l);
		tmp -= r;

		Count* p = attach(tmp);
		detach(l);
		l = p;
	}
}

//	FUNCTION public
//	Lock::Count::clear -- ロック数を 0 にする
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

void
Count::clear()
{
	_bitmap = 0;
	(void) Os::Memory::reset(_value, sizeof(_value));
}

//	FUNCTION public
//	Lock::Count::clear -- ロック数を 0 にしたロック数を得る
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Count*&		l
//			ロック数を 0 にしたいロック数が
//			格納された領域の先頭アドレスで、
//			呼出し後、得られたロック数を格納する領域の先頭アドレスが設定される
//		
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Count::clear(Count*& l)
{
	if (l) {
		Count* p = attach(Count());
		detach(l);
		l = p;
	}
}

//	FUNCTION private
//	Lock::Count::getLeastUpperBound --
//		ロックモードごとのロックの有無を表すビットマップから
//		存在するすべてのロックの最小上界となるロックモードを得る
//
//	NOTES
//		あるロック対象を複数のモードであるトランザクションが
//		ロックしているときに、そのトランザクションは
//		どのモードで実際にロックしているかを求めるために使用する
//
//	ARGUMENTS
//		Lock::Count::Bitmap	bitmap
//			最小上界を求めるロックモード別に
//			そのロックの有無を表すビットマップ
//
//	RETURN
//		存在するすべてのロックの最小上界
//
//	EXCEPTIONS
//		なし

// static
Mode::Value
Count::getLeastUpperBound(Bitmap bitmap)
{
	return _Count::_leastUpperBound[bitmap];
}

//
// Copyright (c) 2000, 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
