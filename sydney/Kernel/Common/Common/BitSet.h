// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BitSet.h -- ビット列関連のクラス定義、関数宣言
// 
// Copyright (c) 1999, 2001, 2002, 2004, 2005, 2008, 2009, 2010, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_BITSET_H
#define __TRMEISTER_COMMON_BITSET_H

#include "Common/Module.h"
#include "Common/Data.h"
#ifdef OLDMAP
#include "ModMap.h"
#else
#include "Common/VectorMap.h"
#endif

#include "ModTypes.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//	CLASS
//	Common::BitSet -- ビット列を表すクラス
//
//	NOTES
//		インターフェースは STL の bitset と同等である
//		ただし、サイズが可変である点が異なる

class SYD_COMMON_FUNCTION BitSet
	: public	Data
{
public:
	//
	//	ENUM public
	//	Common::BitSet::UNIT_SIZE
	//
	//	NOTES
	//	ユニットサイズ(バイト)
	//	ただし、sizeof(unsigned int)の倍数でなければならない。
	//
	enum { UNIT_SIZE = 128 };

	//
	//	ENUM public
	//	Common::BitSet::SIZEOF_UINT
	//
	//	NOTES
	//	unsigned int のサイズ
	//
	enum { SIZEOF_UINT = sizeof(unsigned int) };

	//	CLASS
	//	Common::BitSet::UnitType -- ビットセットの処理単位
	//
	//	NOTES

	struct SYD_COMMON_FUNCTION UnitType
	{
		// コンストラクター
		UnitType();

		unsigned int&	operator [](int iPosition_);
		unsigned int	operator [](int iPosition_) const;

		// すべてのビットが0か
		bool			none() const;
		// onのビット数を得る
		ModSize			count() const;
		// すべてのビットをoffにする
		void			clear();

		// 論理積
		UnitType& operator &= (const UnitType& o);
		// 論理和
		UnitType& operator |= (const UnitType& o);

		// ビット反転
		void flip();

		// 次に立っているビットを得る(引数のposition_から探す)
		ModSize next(ModSize position_) const;

		// ビットを保持する配列
		unsigned int	_bitmap[UNIT_SIZE/SIZEOF_UINT];
	};

	//
	//	CLASS
	//	Common::BitSet::Reference -- ビットのリファレンス
	//
	//	NOTES
	//	1ビットのリファレンス
	//
	class SYD_COMMON_FUNCTION Reference
	{
	public:
		// デストラクタ
		~Reference();

		// b[i] = value_
		Reference& operator=(bool bValue_);
		// b[i] = b[j]
		Reference& operator=(const Reference& cReference_);
		// ~b[i]
		bool operator~() const;
		// value = b[i]
		operator bool() const;
		// b[i].flip
		Reference& flip();

	protected:
		// BitSetだけはreferenceを作ることができる
		friend class BitSet;
		// コンストラクタ
		Reference(BitSet& rBitSet_, ModSize msPosition_);
		// コピーコンストラクタ
		Reference(const Reference& cReference_);

	private:
		ModSize m_msPosition;
		BitSet& m_rBitSet;
	};

	//
	//	TYPEDEF
	//	Common::BitSet::Map -- ビットユニットを保持するマップ
	//
	//	NOTES
	//	ビットユニットを保持するマップ
	//
#ifdef OLDMAP
	typedef ModMap<ModSize, UnitType, ModLess<ModSize> > Map;
#else
	typedef VectorMap<ModSize, UnitType, ModLess<ModSize> > Map;
#endif

	//
	//	CLASS
	//	Common::BitSet::ConstIterator -- 参照専用のイテレータ
	//
	//	NOTES
	//	参照専用のイテレータ。ビットが立っている場所を移動する。
	//
	class SYD_COMMON_FUNCTION ConstIterator
	{
	public:
		//コンストラクタ(1)
		ConstIterator() : m_pMap(0), m_msPosition(0) {}
		//コンストラクタ(2)
		ConstIterator(const Map* map_,
					  const Map::ConstIterator& i_,
					  ModSize msPosition_);
		//コピーコンストラクタ
		ConstIterator(const ConstIterator& i)
			: m_pMap(i.m_pMap), m_iterator(i.m_iterator),
			  m_msPosition(i.m_msPosition) {}
		
		// ++演算子
		ConstIterator& operator ++();
		ConstIterator operator ++(int dummy_);

		// *演算子
		ModSize operator *() const;

		// ==演算子
		bool operator ==(const ConstIterator& r) const;
		// !=演算子
		bool operator !=(const ConstIterator& r) const;

	private:
		//マップ
		const Map* m_pMap;
		//マップのイテレータ
		Map::ConstIterator m_iterator;
		//位置
		ModSize m_msPosition;
	};

	// コンストラクタ
	BitSet();
	// デストラクタ
	~BitSet();
	// コピーコンストラクタ
	BitSet(const BitSet& cOther_);

	// b[i]
	Reference operator[](ModSize msPosition_);

	// ANDをとる
	BitSet& operator&=(const BitSet& cBitSet_);
	// ORをとる
	BitSet& operator|=(const BitSet& cBitSet_);
	// XORをとる
	BitSet& operator^=(const BitSet& cBitSet_);
	// 減算をする
	BitSet& operator-=(const BitSet& cBitSet_);

	// 代入演算子
	BitSet& operator=(const BitSet& cBitSet_);
	
	// b[i] = value
	BitSet& set(ModSize msPosition_, bool bValue_ = true);
	BitSet& set(ModSize msPosition_, bool bValue_, bool& before);

	// 保持している領域を解放する
	void clear();

	// すべてのビットを0にする
	BitSet& reset();
	// b[i] = 0
	BitSet& reset(ModSize msPosition_);

	// b[i]を反転する
	BitSet& flip(ModSize msPosition_);

	// 1の数を得る
	ModSize count() const;
	
	// 次に1がセットされている位置を得る(ConstIterator用)
	ModSize next() const;
	ModSize next(ModSize msPosition_) const;

	//比較
	bool operator==(const BitSet& cBitSet_) const;
	bool operator!=(const BitSet& cBitSet_) const;

	// b[i] == 1
	bool test(ModSize msPosition_) const;
	// 1のビットがあるか
	bool any() const;
	// すべてのビットが0か
	bool none() const;

	//イテレータを得る
	ConstIterator begin() const;
	ConstIterator end() const;
	ConstIterator lowerBound(ModSize msPosition_) const;

	// シリアル化する
//	Common::Data
//	virtual void
//	serialize(ModArchive& archiver);

	// コピーする
//	Common::Data
//	virtual Pointer
//	copy() const;
	// キャストする
//	Common::Data
//	virtual Pointer
//	cast(DataType::Type type) const;
//	virtual Pointer
//	cast(const Data& target) const;

	// 文字列の形式で値を得る
//	Common::Data
//	virtual ModUnicodeString
//	getString() const;

	// 等しいか調べる
	virtual bool
	equals(const Data* r) const;
	// 大小比較を行う
//	Common::Data
//	virtual int
//	compareTo(const Data* r) const;

	// 代入を行う
//	Common::Data
//	virtual bool
//	assign(const Data* r);
	// 四則演算を行う
//	Common::Data
//	virtual bool
//	operateWith(DataOperation::Type op, const Data* r, Pointer& result) const;
	// 単項演算を行う
//	Common::Data
//	virtual bool
//	operateWith(DataOperation::Type op, Pointer& result) const;
//	virtual bool
//	operateWith(DataOperation::Type op);

	// クラスIDを得る
//	Common::Data
//	virtual int
//	getClassID() const;

	//ハッシュコードを取り出す
	virtual ModSize hashCode() const;

	//タプルIDを設定する
	void putTupleID(const Data* pData_);
	//タプルIDを取出す
	Data* getTupleID() const;
	//リセットする
	void resetCursor();

	// UnitTypeを挿入する(既存のものは上書きされる)
	void insertUnitType(ModSize unitPosition_, const UnitType& unit_);

	// UnitTypeを取得する。
	UnitType BitSet::getUnitType(ModSize unitPosition_) const;

	// UnitTypeを論理和する
	void orUnitType(ModSize unitPosition_, const UnitType& unit_);
	
private:
	// シリアル化する(自分自身が NULL 値でない)
	virtual void
	serialize_NotNull(ModArchive& archiver);

	// コピーする(自分自身が NULL 値でない)
	virtual Pointer
	copy_NotNull() const;

	// 文字列の形式で値を得る(自分自身が NULL 値でない)
	virtual ModUnicodeString
	getString_NotNull() const;

	// 等しいか調べる(キャストなし)
	virtual bool
	equals_NoCast(const Data& r) const;

 	// 代入を行う(キャストなし)
 	virtual bool
 	assign_NoCast(const Data& r);
	// 大小比較を行う(キャストなし)
//	Common::Data
//	virtual int
//	compareTo_NoCast(const Data& r) const;

// 	// 四則演算を行う(キャストなし)
// 	virtual bool
// 	operateWith_NoCast(DataOperation::Type op, const Data& r);

	// クラスIDを得る(自分自身が NULL 値でない)
	virtual int
	getClassID_NotNull() const;

	// ビット配列を保持する
	Map m_mapBitHolder;

	//Iterator
	mutable ConstIterator m_iIterator;
	mutable bool m_bIterator;
};

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_BITSET_H

//
// Copyright (c) 1999, 2001, 2002, 2004, 2005, 2008, 2009, 2010, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
