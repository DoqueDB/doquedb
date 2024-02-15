// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Verification.h -- 整合性検査関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2002, 2005, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_VERSION_VERIFICATION_H
#define	__SYDNEY_VERSION_VERIFICATION_H

#include "Version/Module.h"
#include "Version/VersionLog.h"

#include "Common/Object.h"
#include "Common/LargeVector.h"
#include "Admin/Verification.h"

template <class T>
class ModAutoPointer;

_SYDNEY_BEGIN

namespace Trans
{
	class Transaction;
}

_SYDNEY_VERSION_BEGIN

class File;

//	NAMESPACE
//	Version::Verification -- 整合性検査に関する情報を保持するためのクラス
//
//	NOTES

class Verification
{
	friend class File;
	friend class ModAutoPointer<Verification>;
public:
	//	CLASS
	//	Version::Verification::Bitmap --
	//		整合性検査において検査済のバージョンページやブロックを
	//		記憶するために使用するビットマップを表すクラス
	//
	//	NOTES

	class Bitmap
	{
	public:
		//	CLASS
		//	Version::Verification::Bitmap::Index --
		//		ビットマップのビット位置を表すクラス
		//
		//	NOTES

		struct Index
		{
			//	TYPEDEF
			//	Version::Verification::Bitmap::Index::Value --
			//		ビットマップのビット位置を表す値の型
			//
			//	NOTES

			typedef	unsigned int	Value;
			enum {
				// 不正な値
				Invalid =	~static_cast<Value>(0)
			};
		};

		// コンストラクター
		Bitmap();
		// デストラクター
		~Bitmap();

		// ビットが立っているか
		bool					getBit(Index::Value i) const;
		// ビットを操作する
		void
		setBit(Index::Value i, bool on);
		void
		setBit(Index::Value i, unsigned int n, bool on);

		// あらかじめ領域を確保しておく
		void					reserve(unsigned int n);
		// 領域を空にする
		void					clear();
		// 立っているビット数を得る
		unsigned int			getBitCount() const;
		// 操作したビットの最大位置を得る
		Index::Value			getMaxIndex() const;

	private:
		// 立っているビット数
		unsigned int			_bitCount;
		// 操作したビットの最大位置
		Index::Value			_maxIndex;
		// 値
		Common::LargeVector<unsigned int>	_v;
	};

	// 生成する
	static Verification*
	attach(const Trans::Transaction& trans, File& file);
	// 参照をやめ、場合により破棄する
	static void
	detach(Verification*& verification, bool reserve);

	// 検査済のバージョンページを管理するためのビットマップを得る
	const Bitmap&			getPageBitmap() const;
	Bitmap&					getPageBitmap();
	// 検査済のバージョンログファイルを管理するためのビットマップを得る
	const Bitmap&			getBlockBitmap() const;
	Bitmap&					getBlockBitmap();

	// バージョンファイル全体の整合性検査を行うか
	bool					isOverall() const;

	// バージョン番号を設定する
	void			setVersionNumber(VersionLog::VersionNumber::Value v_);
	// ファイルバージョンを得る
	VersionLog::VersionNumber::Value	getVersionNumber() const;

private:
	// コンストラクター
	Verification(const Trans::Transaction& trans, File& file);
	// デストラクター
	~Verification();

#ifdef OBSOLETE
	// 参照数を 1 増やす
	Verification*
	attach();
	// 参照数を 1 減らす
	void
	detach();
#endif
	// 参照数を得る
	unsigned int
	getRefCount() const;

	// 検査中のトランザクションのトランザクション記述子
	const Trans::Transaction& _transaction;
	// 検査しているバージョンファイルのバージョンファイル記述子
	File&					_file;
	// 参照回数
	mutable unsigned int	_refCount;

	// 矛盾を検出時にどう処置するか
	Admin::Verification::Treatment::Value _treatment;
	// バージョンファイルの全体の整合性検査を行うか
	bool					_overall;

	// 検査済のバージョンページを管理するためのビットマップ
	Bitmap					_pageBitmap;
	// 検査済のバージョンログファイルのブロックを管理するためのビットマップ
	Bitmap					_blockBitmap;

	// ファイルのバージョン番号
	VersionLog::VersionNumber::Value	_versionNumber;

	// ハッシュリストでの直前の要素へのポインタ
	Verification*			_hashPrev;
	// ハッシュリストでの直後の要素へのポインタ
	Verification*			_hashNext;
}; 

//	FUNCTION private
//	Version::Verification::Verification --
//		整合性検査に関する情報を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			このトランザクション記述子の表すトランザクションが
//			実行中の整合性検査に関する情報を表す
//		Version::File&		file
//			このバージョンファイル記述子の表すバージョンファイルに対して
//			実行中の整合性検査に関する情報を表す
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
Verification::Verification(const Trans::Transaction& trans, File& file)
	: _transaction(trans),
	  _file(file),
	  _refCount(0),
	  _treatment(Admin::Verification::Treatment::None),
	  _overall(false),
	  _hashPrev(0),
	  _hashNext(0)
{}

//	FUNCTION private
//	Verification::~Verification --
//		整合性検査に関する情報を表すクラスのデストラクター
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
Verification::~Verification()
{}

//	FUNCTION private
//	Version::Verification::getRefCount --
//		整合性検査に関する情報を表すクラスの参照数を得る
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
//		なし

inline
unsigned int
Verification::getRefCount() const
{
	return _refCount;
}

//	FUNCTION public
//	Version::Verification::getPageBitmap --
//		検査済のバージョンページを管理するためのビットマップを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたビットマップを表すクラス
//
//	EXCEPTIONS
//		なし

inline
const Verification::Bitmap&
Verification::getPageBitmap() const
{
	return _pageBitmap;
}

inline
Verification::Bitmap&
Verification::getPageBitmap()
{
	return _pageBitmap;
}

//	FUNCTION public
//	Version::Verification::getPageBitmap --
//		検査済のブロックを管理するためのビットマップを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたビットマップを表すクラス
//
//	EXCEPTIONS
//		なし

inline
const Verification::Bitmap&
Verification::getBlockBitmap() const
{
	return _blockBitmap;
}

inline
Verification::Bitmap&
Verification::getBlockBitmap()
{
	return _blockBitmap;
}

//	FUNCTION public
//	Version::Verification::isOverall --
//		バージョンファイル全体の整合性検査を行うか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			バージョンファイル全体の整合性検査を行う
//		false
//			バージョンファイルの一部の整合性検査を行う
//
//	EXCEPTIONS
//		なし

inline
bool
Verification::isOverall() const
{
	return _overall;
}

//	FUNCTION public
//	Version::Verification::setVersionNumber --
//		バージョンファイルのバージョン番号を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Version::VersionLog::VersionNumber::Value v_
//			バージョン番号
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
Verification::setVersionNumber(VersionLog::VersionNumber::Value v_)
{
	_versionNumber = v_;
}

//	FUNCTION public
//	Version::Verification::getVersionNumber --
//		バージョンファイルのバージョン番号を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Version::VersionLog::VersionNumber::Value
//			バージョン番号
//
//	EXCEPTIONS
//		なし

inline
VersionLog::VersionNumber::Value
Verification::getVersionNumber() const
{
	return _versionNumber;
}

//	FUNCTION public
//	Version::Verification::Bitmap::Bitmap --
//		整合性検査に関する情報を保持するために使用する
//		ビットマップを表すクラスのコンストラクター
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
Verification::Bitmap::Bitmap()
	: _bitCount(0),
	  _maxIndex(Index::Invalid)
{}

//	FUNCTION public
//	Version::Verification::Bitmap::~Bitmap --
//		整合性検査に関する情報を保持するために使用する
//		ビットマップを表すクラスのデストラクター
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
Verification::Bitmap::~Bitmap()
{}

//	FUNCTION public
//	Version::Verification::Bitmap::setBit -- ビットを操作する
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification::Bitmap::Index::Value	i
//			ビットマップの先頭からこの数値番目のビットを操作する
//		unsigned int		n
//			いくつの連続したビットを操作するか
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

inline
void
Verification::Bitmap::setBit(Index::Value i, unsigned int n, bool on)
{
	for (; n > 0; --n, ++i)
		setBit(i, on);
}

//	FUNCTION public
//	Version::Verification::Bitmap::clear --
//		ビットを格納するための領域を空にする
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
void
Verification::Bitmap::clear()
{
	_bitCount = 0;
	_maxIndex = Index::Invalid;
	_v.clear();
}

//	FUNCTION public
//	Version::Verification::Bitmap::getBitCount --
//		立っているビットの数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた立っているビットの数
//
//	EXCEPTIONS
//		なし

inline
unsigned int
Verification::Bitmap::getBitCount() const
{
	return _bitCount;
}

//	FUNCTION public
//	Version::Verification::Bitmap::getMaxIndex --
//		操作したビットの最大位置を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた操作したビットの最大位置
//
//	EXCEPTIONS
//		なし

inline
Verification::Bitmap::Index::Value
Verification::Bitmap::getMaxIndex() const
{
	return _maxIndex;
}

_SYDNEY_VERSION_END
_SYDNEY_END

#endif	// __SYDNEY_VERSION_VERIFICATION_H

//
// Copyright (c) 2001, 2002, 2005, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
