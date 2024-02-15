// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BitSet.cpp -- ビット列を表すクラス
// 
// Copyright (c) 1999, 2001, 2002, 2004, 2005, 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Common";
const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Common/Assert.h"
#include "Common/BitSet.h"
#include "Common/ClassID.h"
#include "Common/DataArrayData.h"
#include "Common/Module.h"
#include "Common/UnsignedIntegerData.h"

#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"

#include "Os/Memory.h"

#include "ModHasher.h"
#include "ModUnicodeOstrStream.h"

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace
{

namespace _BitSet
{
	//	CONST
	//	$$$::_BitSet::ByteBits -- 1 バイトのビット数
	//
	//	NOTES

	const ModSize ByteBits = 8;

	//	CONST
	//	_$$::_BitSet::UIntBits -- unsigned intのビット数

	const ModSize UIntBits = ByteBits * BitSet::SIZEOF_UINT;

	//	CONST
	//	_$$::_BitSet::UnitBits -- 1ユニットのビット数

	const ModSize UnitBits = ByteBits * BitSet::UNIT_SIZE;

	//	CONST
	//	$$$::_BitSet::BitCount
	//		-- 1バイト中の1が立っているビットの数を引くテーブル
	//
	//	NOTES

	const ModSize BitCount[] =
	{
		0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
		1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
		1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
		1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
		3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8
	};

	//次のビット位置を得る
	void
	getNextPosition(const BitSet::UnitType& unit,
					ModSize& unit_position, ModSize& position);

	// ビットがひとつも立っていないユニット
	const BitSet::UnitType		EmptyUnit;
}

//	FUNCTION
//	$$$::_BitSet::getNextPosition -- 次のビット位置を得る
//
//	NOTES
//
//	ARGUMENTS
//	Common::BitSet::UnitType& unit
//		ユニット
//	ModSize& unit_position
//		ユニット内の位置
//	ModSize& position
//		ユニット内のuintの先頭からのビット値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
_BitSet::getNextPosition(const BitSet::UnitType& unit,
						 ModSize& unit_position, ModSize& position)
{
	ModSize n = 1;
	if (position)
	{
		if (unit._bitmap[unit_position])
		{
			//途中から探す
			n <<= position;
			for (; position < _BitSet::UIntBits; ++position)
			{
				if (unit._bitmap[unit_position] & n)
					return;
				n <<= 1;
			}
		}
		
		// 指定されたユニットにはビットは立っていない
		unit_position++;
		position = 0;
	}
	//次を探す
	for (; unit_position < (BitSet::UNIT_SIZE / BitSet::SIZEOF_UINT);
		 ++unit_position)
	{
		if (unit._bitmap[unit_position])
		{
			//あったのでビットを数える
			n = 1;
			position = 0;
			for (; position < _BitSet::UIntBits; ++position)
			{
				if (unit._bitmap[unit_position] & n)
					break;
				n <<= 1;
			}
			break;
		}
	}
}

}

//
//	FUNCTION public
//	Common::BitSet::Reference::~Reference -- デストラクタ
//
//	NOTES
//	デストラクタ
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
BitSet::Reference::
~Reference()
{
}

//
//	FUNCTION public
//	Common::BitSet::Reference::operator= -- b[i] = value_
//
//	NOTES
//	b[i] = value_
//
//	ARGUMENTS
//	bool bValue_
//
//	RETURN
//	Reference&
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
BitSet::Reference&
BitSet::Reference::
operator=(bool bValue_)
{
	m_rBitSet.set(m_msPosition, bValue_);
	return *this;
}

//
//	FUNCTION public
//	Common::BitSet::Reference::operator= -- b[i] = b[j]
//
//	NOTES
//	b[i] = b[j]
//
//	ARGUMENTS
//	const Reference& cReference_
//
//	RETURN
//	Reference&
//
//	EXCEPTIONS
//	???
//
BitSet::Reference&
BitSet::Reference::
operator=(const Reference& cReference_)
{
	m_rBitSet.set(m_msPosition,
				  cReference_.m_rBitSet.test(cReference_.m_msPosition));
	return *this;
}

//
//	FUNCTION public
//	Common::BitSet::Reference::operator~ -- ~b[i]
//
//	NOTES
//	~b[i]
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//	???
//
bool
BitSet::Reference::
operator~() const
{
	return !m_rBitSet.test(m_msPosition);
}

//
//	FUNCTION public
//	Common::BitSet::Reference::bool -- value = b[i]
//
//	NOTES
//	value = b[i]
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//	???
//
BitSet::Reference::
operator bool() const
{
	return m_rBitSet.test(m_msPosition);
}

//
//	FUNCTION public
//	Common::BitSet::Reference::flip -- b[i].flip
//
//	NOTES
//	b[i].flip
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Reference&
//
//	EXCEPTIONS
//	???
//
BitSet::Reference&
BitSet::Reference::
flip()
{
	m_rBitSet.flip(m_msPosition);
	return *this;
}

//
//	FUNCTION protected
//	Common::BitSet::Reference::Reference -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	BitSet& rBitSet_
//	ModSize msPosition_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	???
//
BitSet::Reference::
Reference(BitSet& rBitSet_, ModSize msPosition_)
: m_rBitSet(rBitSet_), m_msPosition(msPosition_)
{
}

//
//	FUNCTION protected
//	Common::BitSet::Reference::Reference -- コピーコンストラクタ
//
//	NOTES
//	コピーコンストラクタ
//
//	ARGUMENTS
//	const Reference& cReference_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	???
//
BitSet::Reference::
Reference(const Reference& cReference_)
: m_rBitSet(cReference_.m_rBitSet), m_msPosition(cReference_.m_msPosition)
{
}

//========================================

//
//	FUNCTION public
//	Common::BitSet::ConstIterator::ConstIterator -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
// 	const Common::BitSet* pBitSet
//		ビットセット(default 0)
//	ModSize msPosition_
//		ビット位置(default 0)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
BitSet::ConstIterator::ConstIterator(const Map* map_,
									 const Map::ConstIterator& i_,
									 ModSize msPosition_)
	: m_pMap(map_), m_iterator(i_), m_msPosition(msPosition_)
{
}

//
//	FUNCTION public
//	Common::BitSet::ConstIterator::operator ++ -- ++演算子
//
//	NOTES
//	++演算子
//
//	ARGUMENTS
//	なし
//
//	RETURN
//  Common::BitSet::ConstIterator&
//		自分自身の参照
//
//	EXCEPTIONS
//	なし
//
BitSet::ConstIterator&
BitSet::ConstIterator::operator ++()
{
	const ModSize max = _BitSet::UnitBits;
	
	ModSize pos = m_msPosition % max;
	++pos;
	while (m_iterator != m_pMap->end())
	{
		if (pos != max)
			pos = (*m_iterator).second.next(pos);
		if (pos == max)
		{
			// ここにはない次へ
			++m_iterator;
			pos = 0;
			continue;
		}
		break;
	}
	if (m_iterator != m_pMap->end())
	{
		m_msPosition = (*m_iterator).first * max + pos;
	}
	else
	{
		m_msPosition = ModSizeMax;
	}
	return *this;
}

//
//	FUNCTION public
//	Common::BitSet::ConstIterator::operator ++ -- ++演算子
//
//	NOTES
//	++演算子
//
//	ARGUMENTS
//	int dummy
//
//	RETURN
//  Common::BitSet::ConstIterator
//		ConstIteratorのコピー
//
//	EXCEPTIONS
//	なし
//
BitSet::ConstIterator
BitSet::ConstIterator::
operator ++(int dummy)
{
	ConstIterator i(*this);
	operator ++();
	return i;
}

//
//	FUNCTION public
//	Common::BitSet::ConstIterator::operator * -- *演算子
//
//	NOTES
//	*演算子
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		ビット位置
//
//	EXCEPTIONS
//	なし
//
ModSize
BitSet::ConstIterator::
operator *() const
{
	return m_msPosition;
}

//
//	FUNCTION public
//	Common::BitSet::ConstIterator::operator == -- 比較オペレータ
//
//	NOTES
//	比較オペレータ。
//
//	ARGUMENTS
//	const ConstIterator& r
//		比較する相手
//
//	RETURN
//	同じ場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//
bool
BitSet::ConstIterator::
operator ==(const ConstIterator& r) const
{
	return (m_msPosition == r.m_msPosition);
}

//
//	FUNCTION public
//	Common::BitSet::ConstIterator::operator != -- 比較オペレータ
//
//	NOTES
//	比較オペレータ。
//
//	ARGUMENTS
//	const ConstIterator& r
//		比較する相手
//
//	RETURN
//	違う場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//
bool
BitSet::ConstIterator::
operator !=(const ConstIterator& r) const
{
	return (m_msPosition != r.m_msPosition);
}

//	FUNCTION public
//	Common::BitSet::UnitType::UnitType --
//		ビットセットの処理単位を表すクラスのコンストラクター
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

BitSet::UnitType::UnitType()
{
	clear();
}

//	FUNCTION public
//	Common::BitSet::UnitTypeType::operator[] -- []オペレータ
//
//	NOTES
//
//	ARGUMENTS
//	int iPosition_
//		位置
//
//	RETURN
//	unsigned int&
//		指定の位置の値
//
//	EXCEPTIONS

unsigned int&
BitSet::UnitType::operator [](int iPosition_)
{
	return _bitmap[iPosition_];
}

//	FUNCTION public
//	Common::BitSet::UnitType::operator[] -- []オペレータ
//
//	NOTES
//
//	ARGUMENTS
//	int iPosition_
//		位置
//
//	RETURN
//	unsigned int
//		指定の位置の値
//
//	EXCEPTIONS

unsigned int
BitSet::UnitType::operator [](int iPosition_) const
{
	return _bitmap[iPosition_];
}

//
//	FUNCTION public
//	Common::BitSet::UnitType::none -- すべてのビットが0かどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETRURN
//	bool
//		すべてのビットが0の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
BitSet::UnitType::none() const
{
	for (int i = 0; i < UNIT_SIZE/SIZEOF_UINT; ++i)
		if (_bitmap[i] != 0)
			return false;
	return true;
}

//
//	FUNCTION public
//	Common::BitSet::UnitType::count -- onのビット数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETRURN
//	ModSize
//		onのビット数を得る
//
//	EXCEPTIONS
//
ModSize
BitSet::UnitType::count() const
{
	ModSize count = 0;
	
	for (int i = 0; i < UNIT_SIZE/SIZEOF_UINT; ++i)
	{
		ModSize b = _bitmap[i];
		for (int k = 0; k < SIZEOF_UINT && b != 0; ++k)
		{
			count += _BitSet::BitCount[b & 0xff];
			b >>= _BitSet::ByteBits;
		}
	}
	
	return count;
}

//
//	FUNCTION public
//	Common::BitSet::UnitType::clear -- すべてのビットをoffにする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
BitSet::UnitType::clear()
{
	(void) Os::Memory::reset(_bitmap, sizeof(_bitmap));
}

//
//	FUNCTION public
//	Common::BitSet:UnitType::operator &= -- 論理積
//
//	NOTES
//
//	ARGUMENTS
//	const Common::BitSet::UnitType& o
//		論理積を取る相手
//
//	RETURN
//	Common::BitSet::UnitType&
//		自分
//
//	EXCEPTIONS
//
BitSet::UnitType&
BitSet::UnitType::operator &= (const BitSet::UnitType& o)
{
	for (int i = 0; i < UNIT_SIZE/SIZEOF_UINT; ++i)
		_bitmap[i] &= o._bitmap[i];
	return *this;
}

//
//	FUNCTION public
//	Common::BitSet:UnitType::operator |=  -- 論理和
//
//	NOTES
//
//	ARGUMENTS
//	const Common::BitSet::UnitType& o
//		論理和を取る相手
//
//	RETURN
//	Common::BitSet::UnitType&
//		自分
//
//	EXCEPTIONS
//
BitSet::UnitType&
BitSet::UnitType::operator |= (const BitSet::UnitType& o)
{
	for (int i = 0; i < UNIT_SIZE/SIZEOF_UINT; ++i)
		_bitmap[i] |= o._bitmap[i];
	return *this;
}

//
//	FUNCTION public
//	Common::BitSet::UnitType::flip -- ビットを反転させる
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BitSet::UnitType::flip()
{
	for (int i = 0; i < UNIT_SIZE/SIZEOF_UINT; ++i)
		_bitmap[i] = ~(_bitmap[i]);
}

//
//	FUNCTION public
//	Common::BitSet::UnitType::next -- 次に立っているビットを得る
//
//	NOTES
//	引数position_の位置から探す
//
//	ARGUMENTS
//	ModSize position_
//		UnitType内のビット位置
//
//	RETURN
//	ModSize
//		ヒット位置、存在しない場合はユニットの最大値(UNIT_SIZE * ByteBits)
//
//	EXCEPTIONS
//
ModSize
BitSet::UnitType::next(ModSize position_) const
{
	ModSize unit_position = position_ / _BitSet::UIntBits;
	ModSize p = position_ % _BitSet::UIntBits;
	_BitSet::getNextPosition(*this, unit_position, p);
	return unit_position * _BitSet::UIntBits + p;
}

//	FUNCTION public
//	Common::BitSet::BitSet -- ビットセットを表すクラスのコンストラクター
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

BitSet::BitSet()
	: Data(DataType::BitSet),
	  m_bIterator(false)
{}

//	FUNCTION public
//	Common::BitSet::~BitSet -- ビットセットを表すクラスのデストラクター
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

BitSet::~BitSet()
{}

//	FUNCTION public
//	Common::BitSet::BitSet -- コピーコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const Common::BitSet& cOther_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

BitSet::BitSet(const BitSet& cOther_)
	: Data(cOther_),
	  m_mapBitHolder(cOther_.m_mapBitHolder),
	  m_bIterator(false)
{}

//
//	FUNCTION public
//	Common::BitSet::operator[] -- b[i]
//
//	NOTES
//	b[i]
//
//	ARGUMENTS
//	ModSize msPosition_
//
//	RETURN
//	Common::BitSet::Reference
//		リファレンス
//
//	EXCEPTIONS
//	なし
//
BitSet::Reference
BitSet::
operator[](ModSize msPosition_)
{
	return Reference(*this, msPosition_);
}

//
//	FUNCTION public
//	Common::BitSet::operator&= -- ANDをとる
//
//	NOTES
//	ANDをとる
//
//	ARGUMENTS
//	const Common::BitSet& cBitSet_
//		ANDをとる相手のビットセット
//
//	RETURN
//	Common::BitSet&
//		自分自身の参照
//
//	EXCEPTIONS
//	なし
//
BitSet&
BitSet::
operator&=(const BitSet& cBitSet_)
{
	Map::Iterator iThis = m_mapBitHolder.begin();
	Map::ConstIterator iOther = cBitSet_.m_mapBitHolder.begin();

	while (iThis != m_mapBitHolder.end())
	{
		//相手が小さい間読み飛ばす
		while (iOther != cBitSet_.m_mapBitHolder.end()
			   && (*iOther).first < (*iThis).first)
		{
			++iOther;
		}
		//自分が小さい間読み飛ばす
		while (iOther == cBitSet_.m_mapBitHolder.end()
			   || (*iOther).first > (*iThis).first)
		{
#ifdef OLDMAP
			Map::Iterator iTmp = iThis;
			++iThis;
			m_mapBitHolder.erase(iTmp);	//いらないので消す
#else
			iThis = m_mapBitHolder.erase(iThis); //いらないので消す
#endif
			if (iThis == m_mapBitHolder.end())
				break;
		}
		if (iThis != m_mapBitHolder.end()
			&& iOther != cBitSet_.m_mapBitHolder.end()
			&& (*iThis).first == (*iOther).first)
		{
			//同じ場所にビットが立っているのでANDを得る
			bool bBit = false;
			for (int i = 0; i < UNIT_SIZE/SIZEOF_UINT; ++i)
			{
				(*iThis).second[i] &= (*iOther).second[i];
				if ((*iThis).second[i]) bBit = true;
			}
#ifdef OLDMAP
			Map::Iterator iTmp = iThis;
			iThis++;
			iOther++;
			if (bBit == false)
			{
				//ビットが立っていないので消す
				m_mapBitHolder.erase(iTmp);
			}
#else
			iOther++;
			if (bBit == false)
			{
				//ビットが立っていないので消す
				iThis = m_mapBitHolder.erase(iThis);
			}
			else
			{
				++iThis;
			}
#endif
		}
	}
	
	return *this;
}

//
//	FUNCTION public
//	Common::BitSet::operator|= -- ORをとる
//
//	NOTES
//	ORをとる
//
//	ARGUMENTS
//	const Common::BitSet& cBitSet_
//		ORをとる相手のビットセット
//
//	RETURN
//	Common::BitSet&
//		自分自身の参照
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
BitSet&
BitSet::
operator|=(const BitSet& cBitSet_)
{
#ifndef OLDMAP
	m_mapBitHolder.reserve(m_mapBitHolder.getSize() + cBitSet_.m_mapBitHolder.getSize());
#endif
	Map::Iterator iThis = m_mapBitHolder.begin();
	Map::ConstIterator iOther = cBitSet_.m_mapBitHolder.begin();

	while (iOther != cBitSet_.m_mapBitHolder.end()
		   && iThis != m_mapBitHolder.end())
	{
		//相手が小さい間挿入する
		while ((*iOther).first < (*iThis).first)
		{
			m_mapBitHolder.insert(*iOther);
			++iOther;
			if (iOther == cBitSet_.m_mapBitHolder.end())
				break;
		}
		//自分が小さい間読み飛ばす
		while (iOther == cBitSet_.m_mapBitHolder.end()
			   || (*iOther).first > (*iThis).first)
		{
			++iThis;
			if (iThis == m_mapBitHolder.end())
				break;
		}
		if (iThis != m_mapBitHolder.end()
			&& iOther != cBitSet_.m_mapBitHolder.end()
			&& (*iThis).first == (*iOther).first)
		{
			//同じ場所にビットが立っているのでORを得る
			for (int i = 0; i < UNIT_SIZE/SIZEOF_UINT; ++i)
			{
				(*iThis).second[i] |= (*iOther).second[i];
			}
			iThis++;
			iOther++;
		}
	}
	//残りを挿入する
	m_mapBitHolder.insert(iOther, cBitSet_.m_mapBitHolder.end());
	
	return *this;
}

//
//	FUNCTION public
//	Common::BitSet::operator^= -- XORをとる
//
//	NOTES
//	XORをとる
//
//	ARGUMENTS
//	const Common::BitSet& cBitSet_
//		XORをとる相手のビットセット
//
//	RETURN
//	Common::BitSet&
//		自分自身の参照
//
//	EXCEPTIONS
//	なし
//
BitSet&
BitSet::
operator^=(const BitSet& cBitSet_)
{
#ifndef OLDMAP
	m_mapBitHolder.reserve(m_mapBitHolder.getSize() + cBitSet_.m_mapBitHolder.getSize());
#endif
	Map::Iterator iThis = m_mapBitHolder.begin();
	Map::ConstIterator iOther = cBitSet_.m_mapBitHolder.begin();

	while (iOther != cBitSet_.m_mapBitHolder.end()
		   && iThis != m_mapBitHolder.end())
	{
		//相手が小さい間挿入する
		while ((*iOther).first < (*iThis).first)
		{
			m_mapBitHolder.insert(*iOther);
			++iOther;
			if (iOther == cBitSet_.m_mapBitHolder.end())
				break;
		}
		//自分が小さい間読み飛ばす
		while (iOther == cBitSet_.m_mapBitHolder.end()
			   || (*iOther).first > (*iThis).first)
		{
			Map::Iterator iTmp = iThis;
			++iThis;
			if (iThis == m_mapBitHolder.end())
				break;
		}
		if (iThis != m_mapBitHolder.end()
			&& iOther != cBitSet_.m_mapBitHolder.end()
			&& (*iThis).first == (*iOther).first)
		{
			//同じ場所にビットが立っているのでXORを得る
			bool bBit = false;
			for (int i = 0; i < UNIT_SIZE/SIZEOF_UINT; ++i)
			{
				(*iThis).second[i] ^= (*iOther).second[i];
				if ((*iThis).second[i]) bBit = true;
			}
#ifdef OLDMAP
			Map::Iterator iTmp = iThis;
			iThis++;
			iOther++;
			if (bBit == false)
			{
				//ビットが立っていないので消す
				m_mapBitHolder.erase(iTmp);
			}
#else
			iOther++;
			if (bBit == false)
			{
				//ビットが立っていないので消す
				iThis = m_mapBitHolder.erase(iThis);
			}
			else
			{
				++iThis;
			}
#endif
		}
	}
	//残りを挿入する
	m_mapBitHolder.insert(iOther, cBitSet_.m_mapBitHolder.end());
	
	return *this;
}

//
//	FUNCTION public
//	Common::BitSet::operator-= -- ビット減算をする
//
//	NOTES
//	ビット減算をする
//	(0011) -= (0101) の時、左辺は (0010) となる。
//
//	ARGUMENTS
//	const Common::BitSet& cBitSet_
//		ビット減算をする相手のビットセット
//
//	RETURN
//	Common::BitSet&
//		自分自身の参照
//
//	EXCEPTIONS
//	なし
//
BitSet&
BitSet::
operator-=(const BitSet& cBitSet_)
{
	Map::Iterator iThis = m_mapBitHolder.begin();
	Map::ConstIterator iOther = cBitSet_.m_mapBitHolder.begin();

	while (iThis != m_mapBitHolder.end())
	{
		//相手が小さい間読み飛ばす
		while (iOther != cBitSet_.m_mapBitHolder.end()
			   && (*iOther).first < (*iThis).first)
		{
			++iOther;
		}
		//自分が小さい間読み飛ばす
		while (iOther == cBitSet_.m_mapBitHolder.end()
			   || (*iOther).first > (*iThis).first)
		{
			++iThis;
			if (iThis == m_mapBitHolder.end())
				break;
		}
		if (iThis != m_mapBitHolder.end()
			&& iOther != cBitSet_.m_mapBitHolder.end()
			&& (*iThis).first == (*iOther).first)
		{
			//同じ場所にビットが立っているので減算をする
			bool bBit = false;
			for (int i = 0; i < UNIT_SIZE/SIZEOF_UINT; ++i)
			{
				(*iThis).second[i] &= ~(*iOther).second[i];
				if ((*iThis).second[i]) bBit = true;
			}
#ifdef OLDMAP
			Map::Iterator iTmp = iThis;
			iThis++;
			iOther++;
			if (bBit == false)
			{
				//ビットが立っていないので消す
				m_mapBitHolder.erase(iTmp);
			}
#else
			iOther++;
			if (bBit == false)
			{
				//ビットが立っていないので消す
				iThis = m_mapBitHolder.erase(iThis);
			}
			else
			{
				++iThis;
			}
#endif
		}
	}
	
	return *this;
}

//	FUNCTION public
//	Common::BitSet::operator= -- 代入演算子
//
//	NOTES
//
//	ARGUMENTS
//	const Common::BitSet& cBitSet_
//		代入元のビットセット
//
//	RETURN
//	Common::BitSet&
//		自分自身の参照
//
//	EXCEPTIONS
//
BitSet&
BitSet::operator=(const BitSet& cBitSet_)
{
	m_mapBitHolder = cBitSet_.m_mapBitHolder;
	m_bIterator = false;
	setNull(cBitSet_.isNull());
	return *this;
}

//	FUNCTION public
//	Common::BitSet::set -- b[i] = value
//
//	NOTES
//
//	ARGUMENTS
//	ModSize msPosition_
//		ビット位置
//	bool iValue_
//		設定するビット(default true)
//
//	RETURN
//	Common::BitSet&
//		自分自身の参照
//
//	EXCEPTIONS

BitSet&
BitSet::set(ModSize msPosition_, bool bValue_)
{
	; _TRMEISTER_ASSERT(msPosition_ != ModSizeMax);

	const ModSize unit = msPosition_ / _BitSet::UnitBits;
	const int unit_position =
		(msPosition_ - unit * UNIT_SIZE * _BitSet::ByteBits) /
		(_BitSet::UIntBits);
	const ModSize n = static_cast<ModSize>(1) <<
		(msPosition_ - unit * UNIT_SIZE * _BitSet::ByteBits -
		 unit_position * _BitSet::UIntBits);
	if (bValue_) {

		// ビットを立てる

		Map::ValueType entry;
		entry.first = unit;
		ModPair<Map::Iterator, ModBoolean>
			result = m_mapBitHolder.insert(entry);

		if (result.second)
			(*result.first).second._bitmap[unit_position] = n;
		else
			(*result.first).second._bitmap[unit_position] |= n;
	} else {

		// ビットを落とす

		Map::Iterator ite(m_mapBitHolder.find(unit));
		if (ite != m_mapBitHolder.end() &&
			!((*ite).second._bitmap[unit_position] &= ~n) &&
			!Os::Memory::compare((*ite).second._bitmap,
								 _BitSet::EmptyUnit._bitmap,
								 sizeof(_BitSet::EmptyUnit._bitmap)))
			m_mapBitHolder.erase(ite);
	}

	return *this;
}

BitSet&
BitSet::set(ModSize msPosition_, bool bValue_, bool& before)
{
	; _TRMEISTER_ASSERT(msPosition_ != ModSizeMax);

	const ModSize unit = msPosition_ / (UNIT_SIZE * _BitSet::ByteBits);
	const int unit_position =
		(msPosition_ - unit * UNIT_SIZE * _BitSet::ByteBits) /
		(_BitSet::UIntBits);
	const ModSize n = static_cast<ModSize>(1) <<
		(msPosition_ - unit * UNIT_SIZE * _BitSet::ByteBits -
		 unit_position * _BitSet::UIntBits);
	if (bValue_) {

		// ビットを立てる

		Map::ValueType entry;
		entry.first = unit;
		ModPair<Map::Iterator, ModBoolean>
			result = m_mapBitHolder.insert(entry);
		unsigned int& bitmap = (*result.first).second._bitmap[unit_position];

		if (result.second) {
			before = false;
			bitmap = n;
		} else {
			before = bitmap & n;
			bitmap |= n;
		}
	} else {

		// ビットを落とす

		Map::Iterator ite(m_mapBitHolder.find(unit));
		if (ite == m_mapBitHolder.end())
			before = false;
		else {
			unsigned int& bitmap = (*ite).second._bitmap[unit_position];
			before = bitmap & n;
			if (!(bitmap &= ~n) &&
				!Os::Memory::compare((*ite).second._bitmap,
									 _BitSet::EmptyUnit._bitmap,
									 sizeof(_BitSet::EmptyUnit._bitmap)))
				m_mapBitHolder.erase(ite);
		}
	}

	return *this;
}

//	FUNCTION public
//	Common::BitSet::reset -- すべてのビットを0にする
//
//	NOTES
//	すべてのビットを0にする
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::BitSet&
//		自分自身の参照
//
//	EXCEPTIONS

BitSet&
BitSet::reset()
{
	m_mapBitHolder.erase(m_mapBitHolder.begin(), m_mapBitHolder.end());
	return *this;
}

//	FUNCTION public
//	Common::BitSet::clear -- 保持している領域を解放する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
BitSet::clear()
{
	m_mapBitHolder.clear();
}

//	FUNCTION public
//	Common::BitSet::reset -- b[i] = 0
//
//	NOTES
//	b[i] = 0
//
//	ARGUMENTS
//	ModSize msPosition_
//		ビット位置
//
//	RETURN
//	Common::BitSet&
//		自分自身の参照
//
//	EXCEPTIONS

BitSet&
BitSet::reset(ModSize msPosition_)
{
	return set(msPosition_, false);
}

//
//	FUNCTION public
//	Common::BitSet::flip -- b[i]を反転する
//
//	NOTES
//	b[i]を反転する
//
//	ARGUMENTS
//	ModSize msPosition_
//		ビット位置
//
//	RETURN
//	Common::BitSet&
//		自分自身の参照
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//

BitSet&
BitSet::flip(ModSize msPosition_)
{
	set(msPosition_, test(msPosition_) ? 0 : 1);
	return *this;
}

//
//	FUNCTION public
//	Common::BitSet::count -- 1の数を得る
//
//	NOTES
//	1の数を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		1の数
//
//	EXCEPTIONS
//	なし
//
ModSize
BitSet::
count() const
{
	ModSize msSum = 0;
	for (Map::ConstIterator i = m_mapBitHolder.begin();
		 i != m_mapBitHolder.end(); ++i)
	{
		for (int j = 0; j < UNIT_SIZE/SIZEOF_UINT; ++j)
		{
			ModSize msValue = (*i).second[j];

			for (int k = 0; k < SIZEOF_UINT; ++k) {
				msSum += _BitSet::BitCount[msValue & 0xff];
				msValue >>= _BitSet::ByteBits;
				if (!msValue) break;
			}
		}
	}
	return msSum;
}

//
//	FUNCTION public
//	Common::BitSet::next -- 次に1がセットされている位置を得る
//
//	NOTES
//	次に1がセットされている位置を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		次に1がセットされている位置
//
//	EXCEPTIONS
//	なし
//
ModSize
BitSet::
next() const
{
	Map::ConstIterator i = m_mapBitHolder.begin();
	ModSize unit;
	if (i == m_mapBitHolder.end())
	{
		return ModSizeMax;
	}
	else
	{
		unit = (*i).first;
	}
	ModSize unit_position = 0;
	ModSize pos = 0;
	_BitSet::getNextPosition((*i).second, unit_position, pos);
	ModSize newPosition;
	newPosition = unit * UNIT_SIZE * _BitSet::ByteBits +
		unit_position * _BitSet::UIntBits + pos;
	return newPosition;
}

//
//	FUNCTION public
//	Common::BitSet::next -- 次に1がセットされている位置を得る
//
//	NOTES
//	あたえられた位置の次に1がセットされている位置を得る
//
//	ARGUMENTS
//	ModSize msPosition_
//		ビット位置
//
//	RETURN
//	ModSize
//		次に1がセットされている位置
//
//	EXCEPTIONS
//	なし
//
ModSize
BitSet::
next(ModSize msPosition_) const
{
	// 与えられたポジションのユニットを得る
	ModSize unit = msPosition_ / (UNIT_SIZE * _BitSet::ByteBits);
	Map::ConstIterator i = m_mapBitHolder.find(unit);
	; _TRMEISTER_ASSERT(i != m_mapBitHolder.end());
	ModSize unit_position =
		(msPosition_ - unit * UNIT_SIZE * _BitSet::ByteBits) /
		(_BitSet::UIntBits);
	ModSize pos	= msPosition_ - unit * UNIT_SIZE * _BitSet::ByteBits -
		unit_position * _BitSet::UIntBits;
	
	pos++;
	_BitSet::getNextPosition((*i).second, unit_position, pos);
	if (unit_position == UNIT_SIZE/SIZEOF_UINT)
	{
		// 次のユニットを探す
		++i;
		if (i == m_mapBitHolder.end())
		{
			// 終り
			unit = ModSizeMax;
		}
		else
		{
			pos = 0;
			unit_position = 0;
			unit = (*i).first;
			_BitSet::getNextPosition((*i).second, unit_position, pos);
			; _TRMEISTER_ASSERT(unit_position != UNIT_SIZE/SIZEOF_UINT);
		}
	}
	ModSize newPosition;
	if (unit == ModSizeMax)
		newPosition = ModSizeMax;
	else
		newPosition = unit * UNIT_SIZE * _BitSet::ByteBits +
			unit_position * _BitSet::UIntBits + pos;
	return newPosition;
}

//
//	FUNCTION public
//	Common::BitSet::operator== -- 比較オペレータ
//
//	NOTES
//	等しいかどうかをチェックする。
//
//	ARGUMENTS
//	const Common::BitSet& cBitSet_
//		比較する相手のビットセット
//
//	RETURN
//	bool
//		等しい場合はtrue、それ以外の場合はfalse。
//
//	EXCEPTIONS
//	なし
//
bool
BitSet::
operator==(const BitSet& cBitSet_) const
{
	bool result = true;
	
	Map::ConstIterator iThis = m_mapBitHolder.begin();
	Map::ConstIterator iOther = cBitSet_.m_mapBitHolder.begin();
	for (; iThis != m_mapBitHolder.end()
		 && iOther != cBitSet_.m_mapBitHolder.end(); ++iThis, ++iOther)
	{
		if ((*iThis).first != (*iOther).first)
		{
			result = false;
			break;
		}
		for (int i = 0; i < UNIT_SIZE/SIZEOF_UINT; ++i)
		{
			if ((*iThis).second[i] != (*iOther).second[i])
			{
				result = false;
				break;
			}
		}
	}
	if (iThis != m_mapBitHolder.end()
		|| iOther != cBitSet_.m_mapBitHolder.end())
	{
		result = false;
	}
	
	return result;
}

//
//	FUNCTION public
//	Common::BitSet::operator!= -- 比較オペレータ
//
//	NOTES
//	等しくないかどうかをチェックする。
//
//	ARGUMENTS
//	const Common::BitSet& cBitSet_
//		比較する相手のビットセット
//
//	RETURN
//	bool
//		等しくない場合はtrue、それ以外の場合はfalse。
//
//	EXCEPTIONS
//	なし
//

bool
BitSet::operator !=(const BitSet& cBitSet_) const
{
	return !(operator ==(cBitSet_));
}

//	FUNCTION public
//	Common::BitSet::test -- b[i] == 1
//
//	NOTES
//
//	ARGUMENTS
//	ModSize msPosition_
//		ビット位置
//
//	RETURN
//	bool
//		ビットが立っていた場合はtrue、それ以外の場合はfalse。
//
//	EXCEPTIONS
//	なし

bool
BitSet::test(ModSize msPosition_) const
{
	const ModSize unit = msPosition_ / (UNIT_SIZE * _BitSet::ByteBits);
	const Map::ConstIterator& ite = m_mapBitHolder.find(unit);

	if (ite != m_mapBitHolder.end()) {
		const int unit_position =
			(msPosition_ - unit * UNIT_SIZE * _BitSet::ByteBits) /
			(_BitSet::UIntBits);
		const ModSize n = static_cast<ModSize>(1) <<
			(msPosition_ - unit * UNIT_SIZE * _BitSet::ByteBits -
			 unit_position * _BitSet::UIntBits);
		return (*ite).second._bitmap[unit_position] & n;
	}

	return false;
}

//
//	FUNCTION public
//	Common::BitSet::any -- 1のビットがあるか
//
//	NOTES
//	1のビットがあるか
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//	なし
//

bool
BitSet::any() const
{
	return m_mapBitHolder.getSize();
}

//
//	FUNCTION public
//	Common::BitSet::none -- すべてのビットが0か
//
//	NOTES
//	すべてのビットが0か
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//	なし
//

bool
BitSet::none() const
{
	return !any();
}

//
//	FUNCTION public
//	Common::BitSet::begin -- イテレータを得る
//
//	NOTES
//	イテレータを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ConstIterator
//		先頭のイテレータ
//
//	EXCEPTIONS
//	なし
//
BitSet::ConstIterator
BitSet::
begin() const
{
	return ConstIterator(&m_mapBitHolder, m_mapBitHolder.begin(), next());
}

//
//	FUNCTION public
//	Common::BitSet::end -- イテレータを得る
//
//	NOTES
//	イテレータを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ConstIterator
//		最後のイテレータ
//
//	EXCEPTIONS
//	なし
//
BitSet::ConstIterator
BitSet::
end() const
{
	return ConstIterator(&m_mapBitHolder, m_mapBitHolder.end(), ModSizeMax);
}

//
//	FUNCTION public
//	Common::BitSet::lowerBound -- イテレータを得る
//
//	NOTES
//	下限検索でイテレータを得る。
//
//	ARGUMENTS
//	ModSize msPosition_
//		下限検索するビット位置
//
//	RETURN
//	ConstIterator
//		イテレータ
//
//	EXCEPTIONS
//	なし
//
BitSet::ConstIterator
BitSet::
lowerBound(ModSize msPosition_) const
{
	// 与えられたポジションのユニットを得る
	ModSize unit = msPosition_ / (UNIT_SIZE * _BitSet::ByteBits);
	Map::ConstIterator i = m_mapBitHolder.lowerBound(unit);
	if (i == m_mapBitHolder.end())
		return end();

	ModSize unit_position =
		(msPosition_ - unit * UNIT_SIZE * _BitSet::ByteBits) /
		(_BitSet::UIntBits);
	ModSize pos	= msPosition_ - unit * UNIT_SIZE * _BitSet::ByteBits -
		unit_position * _BitSet::UIntBits;
	
	_BitSet::getNextPosition((*i).second, unit_position, pos);
	if (unit_position == UNIT_SIZE/SIZEOF_UINT)
	{
		// 次のユニットを探す
		// ビットが立っていないユニットは存在していないというのが前提
		
		++i;
		if (i == m_mapBitHolder.end())
			//終り
			return end();

		pos = 0;
		unit_position = 0;
		unit = (*i).first;
		_BitSet::getNextPosition((*i).second, unit_position, pos);
		; _TRMEISTER_ASSERT(unit_position != UNIT_SIZE/SIZEOF_UINT);
	}
	
	ModSize newPosition = unit * UNIT_SIZE * _BitSet::ByteBits +
		unit_position * _BitSet::UIntBits + pos;

	return ConstIterator(&m_mapBitHolder, i, newPosition);
}

//	FUNCTION private
//	Common::BitSet::serialize_NotNull -- シリアル化
//
//	NOTES
//	シリアル化
//
//	ARGUMENTS
//	ModArchive& cArchiver_
//		アーカイバ
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
BitSet::serialize_NotNull(ModArchive& cArchiver_)
{
	; _TRMEISTER_ASSERT(!isNull());

	Data::serialize_NotNull(cArchiver_);

	if (cArchiver_.isStore())
	{
		//書出し
		ModSize n;
		n = m_mapBitHolder.getSize();
		cArchiver_ << n;
		for (Map::Iterator i = m_mapBitHolder.begin();
			 i != m_mapBitHolder.end(); ++i)
		{
			n = (*i).first;
			cArchiver_ << n;
			for (int j = 0; j < UNIT_SIZE/SIZEOF_UINT; ++j)
			{
				n = (*i).second[j];
				cArchiver_ << n;
			}
		}
	}
	else
	{
		m_mapBitHolder.erase(m_mapBitHolder.begin(), m_mapBitHolder.end());
		//読込み
		ModSize n;
		cArchiver_ >> n;
		for (int i = 0; i < static_cast<int>(n); ++i)
		{
			ModSize u;
			cArchiver_ >> u;
			UnitType unit;
			for (int j = 0; j < UNIT_SIZE/SIZEOF_UINT; ++j)
			{
				cArchiver_ >> unit[j];
			}
			m_mapBitHolder.insert(u, unit);
		}
	}
}

// FUNCTION public
//	Common::BitSet::assign_NoCast -- 代入を行う(キャストなし)
//
// NOTES
//
// ARGUMENTS
//	const Data& r
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
BitSet::
assign_NoCast(const Data& r)
{
#ifdef OBSOLETE // castの結果Nullになることがある場合生かす
	if (r.isNull()) {
		setNull();
		return false;
	}
#endif

	; _TRMEISTER_ASSERT(r.getType() == getType());
	; _TRMEISTER_ASSERT(!r.isNull());

	const BitSet& data = _SYDNEY_DYNAMIC_CAST(const BitSet&, r);
	m_mapBitHolder = data.m_mapBitHolder;
	m_bIterator = false;
	setNull(false);
	return true;
}

//
//	FUNCTION private
//	Common::BitSet::getClassID_NotNull -- クラスIDを得る
//
//	NOTES
//	クラスIDを得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		クラスID
//
//	EXCEPTIONS
//	なし

int
BitSet::getClassID_NotNull() const
{
	return ClassID::BitSetClass;
}

//
//	FUNCTION private
//	Common::BitSet::copy_NotNull -- 自分自身のコピーを作成する
//
//	NOTES
//	自分自身のコピーを作成する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Data*
//		自分自身のコピー
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//

Data::Pointer
BitSet::copy_NotNull() const
{
	return new BitSet(*this);
}

//
//	FUNCTION private
//	Common::BitSet::getString_NotNull -- 内容を表す文字列を得る
//
//	NOTES
//	内容を表す文字列を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//
//	EXCEPTIONS

ModUnicodeString
BitSet::getString_NotNull() const
{
	ModUnicodeOstrStream ostr;
	ostr << "{";
	for (ConstIterator i = begin(); i != end(); ++i)
	{
		if (i != begin()) ostr << ",";
		ostr << (*i);
	}
	ostr << "}";
	return ModUnicodeString(ostr.getString());
}

//	FUNCTION public
//	Common::BitSet::equals -- 等しいか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Common::Data*	r
//			右辺に与えられたデータを格納する領域の先頭アドレス
//
//	RETURN
//		true
//			自分自身と与えられたデータは等しい
//		false
//			自分自身と与えられたデータは等しくない
//
//	EXCEPTIONS

bool
BitSet::equals(const Data* r) const
{
	if (!r)

		// 右辺の値が指定されていない

		_TRMEISTER_THROW0(Exception::BadArgument);

	// 左辺または右辺のいずれかが NULL であれば、
	// NULL より常に他の型の値のほうが大きいとみなす

	return (isNull()) ? r->isNull() :
		(r->isNull() || r->getType() != DataType::BitSet) ?
		false : equals_NoCast(*r);
}

//	FUNCTION private
//	Common::BitSet::equals_NoCast -- 等しいか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Common::Data&	r
//			右辺に与えられたデータ
//
//	RETURN
//		true
//			自分自身と与えられたデータは等しい
//		false
//			自分自身と与えられたデータは等しくない
//
//	EXCEPTIONS

bool
BitSet::equals_NoCast(const Data& r) const
{
	; _TRMEISTER_ASSERT(r.getType() == DataType::BitSet);

	const BitSet& data = _SYDNEY_DYNAMIC_CAST(const BitSet&, r);

	return operator ==(data);
}

// FUNCTION public
//	Common::BitSet::hashCode -- ハッシュコードを取り出す
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModSize
//
// EXCEPTIONS

//virtual
ModSize
BitSet::
hashCode() const
{
	if (isNull()) return 0;

	ModSize hashValue = 0;
	ModSize g;
	ModSize offset = 0;
	ModSize unit = UNIT_SIZE / sizeof(unsigned int);
	Map::ConstIterator iterator = m_mapBitHolder.begin();
	const Map::ConstIterator last = m_mapBitHolder.end();
	while (iterator != last) {
		hashValue <<= 4;
		hashValue += (*iterator).second._bitmap[offset];
		if (g = hashValue & ((ModSize) 0xf << (ModSizeBits - 4))) {
			hashValue ^= g >> (ModSizeBits - 8);
			hashValue ^= g;
		}
		++iterator;
		offset = (offset + 1) % unit;
	}
	return hashValue;
}

//
//	FUNCTION public
//	Common::BitSet::putTupleID -- タプルIDを設定する
//
//	NOTES
//	タプルIDを設定する
//
//	ARGUMENTS
//	const Common::Data* pData_
//		UnsignedIntegerData or DataArrayData
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::BadArgument
//		引数がDataArrayDataでもUnsignedIntegerDataでもない
//	その他
//		下位の例外はそのまま再送
//

void
BitSet::
putTupleID(const Data* pData_)
{
	; _TRMEISTER_ASSERT(pData_);

	if (pData_->getType() == DataType::Array &&
		pData_->getElementType() == DataType::Data) {
		pData_ = _SYDNEY_DYNAMIC_CAST(
				const DataArrayData&, *pData_).getElement(0).get();
		; _TRMEISTER_ASSERT(pData_);
	}

	if (pData_->getType() == DataType::UnsignedInteger) {
		const UnsignedIntegerData& position =
			_SYDNEY_DYNAMIC_CAST(const UnsignedIntegerData&, *pData_);
		set(position.getValue());
		m_bIterator = false;
	} else
		_TRMEISTER_THROW0(Exception::BadArgument);
}

//
//	FUNCTION public
//	Common::BitSet::getTupleID -- タプルIDを取出す
//
//	NOTES
//	タプルIDを取出す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//

Data*
BitSet::getTupleID() const
{
	if (!m_bIterator) {
		m_iIterator = begin();
		m_bIterator = true;
	}
	return (m_iIterator != end()) ?
		new UnsignedIntegerData(*m_iIterator++) : 0;
}

//
//	FUNCTION public
//	Common::BitSet::resetCursor -- タプルID取出しのカーソルをリセットする
//
//	NOTES
//	タプルID取出しのカーソルをリセットする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
BitSet::
resetCursor()
{
	m_bIterator = false;
}

//
//	FUNCTION public
//	Common::BitSet::insertUnitType -- UnitTypeを挿入する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize unitPosition_
//		ユニットの位置情報
//	const BitSet::UnitType& unit_
//		挿入するユニット
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BitSet::insertUnitType(ModSize unitPosition_, const UnitType& unit_)
{
	Map::ValueType entry;
	entry.first = unitPosition_;
	ModPair<Map::Iterator, ModBoolean> result = m_mapBitHolder.insert(entry);
	Os::Memory::copy((*result.first).second._bitmap,
					 unit_._bitmap, UNIT_SIZE);
}

//
//	FUNCTION public
//	Common::BitSet::getUnitType --
//
//	NOTES
//
//	ARGUMENTS
//	ModSize unitPosition_
//		ユニットの位置情報
//
//	RETURN
//	const BitSet::UnitType&
//		取得するユニット
//
//	EXCEPTIONS
//
BitSet::UnitType
BitSet::getUnitType(ModSize unitPosition_) const
{
	UnitType unit;
	const Map::ConstIterator& ite = m_mapBitHolder.find(unitPosition_);
	if (ite != m_mapBitHolder.end())
	{
		unit = (*ite).second;
	}
	return unit;
}

//
//	FUNCTION public
//	Common::BitSet::orUnitType -- Bitwise OR by the UnitType
//
//	NOTES
//
//	ARGUMENTS
//	ModSize unitPosition_
//		ユニットの位置情報
//	const BitSet::UnitType& unit_
//		挿入するユニット
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BitSet::orUnitType(ModSize unitPosition_, const UnitType& unit_)
{
	const Map::Iterator& ite = m_mapBitHolder.find(unitPosition_);
	if (ite != m_mapBitHolder.end())
	{
		UnitType& unit = (*ite).second;
		unit |= unit_;
	}
	else
	{
		insertUnitType(unitPosition_, unit_);
	}
}

//
//	Copyright (c) 1999, 2001, 2002, 2004, 2005, 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
