// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Verification.cpp -- 整合性検査関連の関数定義
// 
// Copyright (c) 2001, 2002, 2003, 2005, 2009, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Version";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyInclude.h"

#include "Version/Configuration.h"
#include "Version/File.h"
#include "Version/HashTable.h"
#include "Version/Manager.h"
#include "Version/Verification.h"

#include "Common/Assert.h"
#include "Os/AutoCriticalSection.h"
#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_VERSION_USING

namespace
{

namespace _Verification
{
	namespace _Bitmap
	{
		// ある数のビットを格納するためのビットマップ単位数を求める
		unsigned int
		getLength(unsigned int n);

		// 1 ビットマップ単位あたりのビット数
		const unsigned int		_bitCountPerBitmap = 8 * sizeof(unsigned int);
	}

	// すべての整合性検査に関する情報を表すクラスを管理する
	// ハッシュ表に登録するためのハッシュ値を計算する
	unsigned int
	verificationTableHash(const Trans::Transaction& trans, const File& file);

	// 以下の情報を保護するためのラッチ
	Os::CriticalSection		_latch;
	// すべての整合性検査に関する情報を表すクラスを管理するハッシュ表
	HashTable<Verification>* _verificationTable = 0;
}

//	FUNCTION
//	$$$::_Verification::verificationTableHash --
//		すべての整合性検査に関する情報を表すクラスを管理するハッシュ表に
//		登録するためのハッシュ値を計算する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			このトランザクション記述子の表すトランザクションによる
//			整合性検査に関する情報を表すクラスを登録するための
//			ハッシュ値を計算する
//		Version::File&		file
//			このバージョンファイル記述子の表すバージョンファイルに対する
//			整合性検査に関する情報を表すクラスを登録するための
//			ハッシュ値を計算する
//
//	RETURN
//		得られたハッシュ値
//
//	EXCEPTIONS
//		なし

inline
unsigned int
_Verification::verificationTableHash(
	const Trans::Transaction& trans, const File& file)
{
	return trans.getID().hashCode();
}

//	FUNCTION
//	$$$::_Verification::_Bitmap::getLength --
//		ある数のビットを格納するためのビットマップ単位数を求める
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		n
//			この数のビットを格納するためのビットマップ単位数を求める
//
//	RETURN
//		求めたビットマップ単位数
//
//	EXCEPTIONS
//		なし

inline
unsigned int
_Verification::_Bitmap::getLength(unsigned int n)
{
	return (n + 1) / _bitCountPerBitmap;
}

}

//	FUNCTION private
//	Manager::Verification::initialize --
//		マネージャーの初期化のうち、整合性検査関連のものを行う
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
Manager::Verification::initialize()
{}

//	FUNCTION private
//	Manager::Verification::terminate --
//		マネージャーの後処理のうち、整合性検査関連のものを行う
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
Manager::Verification::terminate()
{
	//【注意】	他のスレッドが同時に実行されることはないので、
	//			ラッチしない

	if (_Verification::_verificationTable) {

		// この時点で、整合性検査に関する情報を表すクラスは存在しないはず

		; _SYDNEY_ASSERT(_Verification::_verificationTable->isEmpty());

		// すべての整合性検査に関する情報を表すクラスを
		// 管理するハッシュ表を破棄する

		delete _Verification::_verificationTable,
		_Verification::_verificationTable = 0;
	}
}

//	FUNCTION public
//	Version::Verification::attach -- 整合性検査に関する情報を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			このトランザクション記述子の表すトランザクションによる
//			整合性検査に関する情報を表すクラスを得る
//		Version::File&		file
//			このバージョンファイル記述子の表すバージョンファイルに対する
//			整合性検査に関する情報を表すクラスを得る
//
//	RETURN
//		得られた整合性検査に関する情報を表すクラスを格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
Verification*
Verification::attach(const Trans::Transaction& trans, File& file)
{
	// 整合性検査に関する情報を表すクラスの生成・破棄に関する情報を
	// 保護するためのラッチをかける

	Os::AutoCriticalSection	latch(_Verification::_latch);

	if (!_Verification::_verificationTable) {

		// すべての整合性検査に関する情報を表すクラスを管理する
		// ハッシュ表が確保されていないので、まず、確保する

		_Verification::_verificationTable =
			new HashTable<Verification>(
				Configuration::VerificationTableSize::get(),
				&Verification::_hashPrev, &Verification::_hashNext);
		; _SYDNEY_ASSERT(_Verification::_verificationTable);
	}

	// 指定されたバージョンファイル記述子の表す
	// バージョンファイルに対する指定された
	// トランザクション記述子の表すトランザクションによる
	// 整合性検査に関する情報を表すクラスを格納すべき
	// ハッシュ表のバケットを求める

	const unsigned int addr =
		_Verification::verificationTableHash(trans, file) %
			_Verification::_verificationTable->getLength();
	HashTable<Verification>::Bucket& bucket =
		_Verification::_verificationTable->getBucket(addr);

	switch (bucket.getSize()) {
	default:
	{
		// バケットに登録されている整合性検査に関する情報を表すクラスのうち、
		// 指定されたバージョンファイルに対する
		// 指定されたトランザクションによるものを探す

		HashTable<Verification>::Bucket::Iterator		 begin(bucket.begin());
		HashTable<Verification>::Bucket::Iterator		 ite(begin);
		const HashTable<Verification>::Bucket::Iterator& end = bucket.end();

		do {
			Verification& verification = *ite;
			if (verification._transaction.getID() == trans.getID() &&
				&verification._file == &file) {

				// 見つかったので、参照回数を 1 増やす

				++verification._refCount;

				// 見つかった整合性検査に関する情報を表すクラスを
				// バケットの先頭に移動して、
				// 最近に参照されたものほど、見つかりやすくする

				bucket.splice(begin, bucket, ite);

				return &verification;
			}
		} while (++ite != end) ;

		break;
	}
	case 1:
	{
		Verification& verification = bucket.getFront();
		if (verification._transaction.getID() == trans.getID() &&
			&verification._file == &file) {
			++verification._refCount;
			return &verification;
		}
		break;
	}
	case 0:
		break;
	}

	// 見つからなかったので、生成する

	Verification* verification = new Verification(trans, file);
	; _SYDNEY_ASSERT(verification);

	// 生成された整合性検査に関する情報を表すクラスが存在する間に、
	// 指定されたトランザクション記述子とバージョンファイル記述子が
	// 破棄されると困るので、参照回数を 1 増やして、破棄されないようにする

	(void) Trans::Transaction::attach(trans);
	(void) file.attach();

	// 参照回数を 1 にする

	verification->_refCount = 1;

	// ハッシュ表のバケットの先頭に挿入して、
	// 最近に参照されたものほど、見つかりやすくする

	bucket.pushFront(*verification);

	return verification;
}

#ifdef OBSOLETE
//	FUNCTION private
//	Version::Verification::attach --
//		整合性検査に関する情報を表すクラスの参照数を 1 増やす
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		参照数が 1 増えた整合性検査に関する情報を格納する領域の先頭アドレス
//
//	EXCEPTIONS

Verification*
Verification::attach()
{
	// 整合性検査に関する情報を表すクラスの生成・破棄に関する情報を
	// 保護するためのラッチをかける

	Os::AutoCriticalSection	latch(_Verification::_latch);

	// 参照数を 1 増やす

	++_refCount;

	return this;
}
#endif

//	FUNCTION public
//	Version::Verification::detach --
//		整合性検査に関する情報を表すクラスの参照をやめる
//
//	NOTES
//		整合性検査に関する情報を表すクラスの参照をやめても、
//		他のどこかで参照されていれば、そのクラスは破棄されない
//		逆にどこからも参照されていないとき、
//		指定によっては、そのクラスは破棄される
//
//	ARGUMENTS
//		Version::Verification*&	verification
//			参照をやめる整合性検査に関する情報を格納する領域の先頭アドレスで、
//			呼び出しから返ると、0 になる
//		bool				reserve
//			true
//				どこからも参照されなくなった整合性検査に関する情報を表す
//				クラスでも、また参照されるときのためにとっておく
//			false
//				どこからも参照されなくなった整合性検査に関する情報を表す
//				クラスは破棄する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Verification::detach(Verification*& verification, bool reserve)
{
	if (verification) {
		{
		// 整合性検査に関する情報を表すクラスの生成・破棄に関する情報を
		// 保護するためのラッチをかける

		Os::AutoCriticalSection	latch(_Verification::_latch);

		; _SYDNEY_ASSERT(verification->getRefCount());

		// 参照数が 0 より大きければ 1 減らす

		if (verification->getRefCount() &&
			(--verification->_refCount || reserve))

			// 他から参照されているか、
			// また参照されるときのためにとっておくので、破棄できない

			verification = 0;
		else {

			// 与えられた整合性検査に関する情報を表すクラスを格納する
			// ハッシュ表のバケットを求め、そのクラスを取り除く
			//
			//【注意】	バケットは _Verification::_latch で保護される

			; _SYDNEY_ASSERT(_Verification::_verificationTable);
			const unsigned int addr =
				_Verification::verificationTableHash(
					verification->_transaction, verification->_file) %
				_Verification::_verificationTable->getLength();
			HashTable<Verification>::Bucket& bucket =
				_Verification::_verificationTable->getBucket(addr);

			bucket.erase(*verification);
		}

		// ここで、整合性検査に関する情報を表すクラスの生成・破棄に関する情報を
		// 保護するためのラッチがはずれる
		}
		if (verification) {
			const Trans::Transaction* trans = &verification->_transaction;
			File* file = &verification->_file;

			// どこからも参照されていない整合性検査に関する情報を
			// 表すクラスを破棄し、与えられたポインタは 0 を指すようにする

			delete verification, verification = 0;

			// 破棄した整合性検査に関する情報を表すクラスが参照していた
			// トランザクション記述子の参照数を 1 減らす

			Trans::Transaction::detach(trans);

			// 破棄した整合性検査に関する情報を表すクラスが参照していた
			// バージョンファイル記述子の参照数を 1 減らし、
			// どこからも参照されなくなったら破棄する

			File::detach(file, false);
		}
	}
}

#ifdef OBSOLETE
//	FUNCTION private
//	Version::Verification::detach --
//		整合性検査に関する情報を表すクラスの参照数を 1 減らす
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
Verification::detach()
{
	// 整合性検査に関する情報を表すクラスの生成・破棄に関する情報を
	// 保護するためのラッチをかける

	Os::AutoCriticalSection	latch(_Verification::_latch);
#ifdef DEBUG
	; _SYDNEY_ASSERT(getRefCount());
#else
	if (getRefCount())
#endif
		// 参照数を 1 減らす

		--_refCount;
}
#endif

//	FUNCTION public
//	Version::Verification::Bitmap::getBit -- ビットが立っているか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification::Bitmap::Index::Value	i
//			立っているか調べるビットがビットマップの先頭から何番目か
//
//	RETURN
//		true
//			ビットは立っている
//		false
//			ビットは落ちている
//
//	EXCEPTIONS
//		なし

bool
Verification::Bitmap::getBit(Index::Value i) const
{
	; _SYDNEY_ASSERT(i != Index::Invalid);

	const ModSize index = i / _Verification::_Bitmap::_bitCountPerBitmap;
	if (index >= _v.getSize())
		return false;

	const unsigned int& b = _v[index];
	const unsigned int m = 1 << i % _Verification::_Bitmap::_bitCountPerBitmap;
	return b & m;
}

//	FUNCTION public
//	Version::Verification::Bitmap::setBit -- ビットを操作する
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification::Bitmap::Index::Value	i
//			ビットマップの先頭からこの数値番目のビットを操作する
//		bool				on
//			true
//				ビットを立てる
//			false
//				ビットを落とす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Verification::Bitmap::setBit(Index::Value i, bool on)
{
	; _SYDNEY_ASSERT(i != Index::Invalid);

	const ModSize index = i / _Verification::_Bitmap::_bitCountPerBitmap;
	const unsigned int m = 1 << i % _Verification::_Bitmap::_bitCountPerBitmap;

	if (index < _v.getSize()) {
		unsigned int& b = _v[index];
		if (b & m) {
			if (!on) {
				b &= ~m;
				--_bitCount;
			}
		} else
			if (on) {
				b |= m;
				++_bitCount;
			}
	} else if (on) {
		while (_v.getSize() < index)
			_v.insert(_v.end(), 0);
		(void) _v.insert(_v.end(), m);
		++_bitCount;
	}

	if (getMaxIndex() < i || getMaxIndex() == Index::Invalid)
		_maxIndex = i;
}

//	FUNCTION public
//	Version::Verification::Bitmap::reserve --
//		ある数のビットを格納するための領域を確保する
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		n
//			この数のビットを格納するための領域を確保する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Verification::Bitmap::reserve(unsigned int n)
{
	_v.assign(_Verification::_Bitmap::getLength(n));
}

//
// Copyright (c) 2001, 2002, 2003, 2005, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
