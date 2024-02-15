// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IDBlock.cpp -- 
// 
// Copyright (c) 2007, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Bitmap";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Bitmap/IDBlock.h"

#include "Common/Assert.h"

#include "Os/Memory.h"

#include "ModOsDriver.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

namespace
{
	//	最大ビット数
	enum { _MAX_LENGTH = IDBlock::BLOCK_SIZE * sizeof(ModUInt32) * 8 };
	
	//	この名前なし名前空間にはModInvertedCoderなどから必要なものを
	//	コピーしておいておく。本来ならCommonなどに移動した方がいいが、
	//	少し Bitmap 用にいじっているのと、移動のコストなどの理由でここに置く。
	//	エンバグしないように極力いじらずに置いておく。
	//	よって、コーディング規約にマッチしていない
	
	
	//
	//	ENUM -- パラメータ付き ExpGolomb 法のλ(PEG:3に該当)
	//
	enum
	{
		LAMBDA = 3,
		LAMBDA1 = LAMBDA + 1,
		LAMBDA3 = 1 << LAMBDA,
		LAMBDA2 = LAMBDA3 - 1,
	};


	//
	//	FUNCTION
	//	_$$::_getPrefixBitLength -- プレフィックス長の計算
	//
	ModSize _getPrefixBitLength(ModSize value)
	{
		//	【注意】
		//	ModInvertedParameterizedExpGolombCoder::getPrefixBitLength と同じ

		if (value <= LAMBDA3) {
			return 0;
		}
		ModSize prefixLength(1);
		value += LAMBDA2;
		value >>= 1;
		while ((value >>= 1) > LAMBDA2) {
			++prefixLength;
		}
		return prefixLength;
	}

	//
	//	FUNCTION
	//	_$$::_setBit -- ビットを設定する
	//
	void _set(ModUInt32* dataBegin,
			  ModSize bitOffset,
			  ModSize bitLength,
			  ModFileOffset bitString)
	{
		//	【注意】
		//	ModInvertedCoder::set と同じ
	   
		dataBegin += (bitOffset>>5);
		bitOffset &= 31;	//	bitOffset %= 32;

		bitOffset += bitLength;
		if (bitOffset <= 32) {				// shift >= 0
			// １つのユニットに収まる場合
			*dataBegin |= (ModSize)(bitString<<(32 - bitOffset)) & 0xffffffff;
		} else if (bitOffset <= 64) {
			// ２つのユニットに跨る場合
			*dataBegin |= (ModSize)(bitString>>(bitOffset - 32)) & 0xffffffff;
			++dataBegin;
			*dataBegin |= (ModSize)(bitString<<(64 - bitOffset)) & 0xffffffff;
		} else {
			// ３つのユニットに跨る場合
			*dataBegin |= (ModSize)(bitString>>(bitOffset - 32)) & 0xffffffff;
			++dataBegin;
			*dataBegin |= (ModSize)(bitString>>(bitOffset - 64)) & 0xffffffff;
			++dataBegin;
			*dataBegin |= (ModSize)(bitString<<(96 - bitOffset)) & 0xffffffff;
		}
	}

	//
	//	FUNCTION
	//	_$$::_setOff -- ビットをクリアする
	//
	void _setOff(ModUInt32* dataBegin,
				 ModSize bitOffset1, ModSize bitOffset2)
	{
		//	【注意】
		//	ModInvertedCoder::setOff と同じ
		
		// 0 を詰めおわるべきユニットを得る
		const ModUInt32* dataTail = dataBegin + (bitOffset2>>5);
		// 0 を詰めはじめるべきユニットまでシフトする
		dataBegin += bitOffset1>>5;

		bitOffset1 &= 31;
		bitOffset2 &= 31;

		if (dataBegin == dataTail) {
			// 単一のユニットの場合
			; _TRMEISTER_ASSERT(bitOffset1 < bitOffset2);
			if (bitOffset1 == 0) {
				*dataBegin &= ~(0xffffffff<<(32 - bitOffset2));
//			} else if (bitOffset2 == 32) {
//				*dataBegin &= (0xffffffff<<(32 - bitOffset1));
			} else {
				*dataBegin &= (0xffffffff<<(32 - bitOffset1))|
					~(0xffffffff<<(32 - bitOffset2));
			}
		} else {
			// 複数のユニットの場合
			if (bitOffset1 == 0) {
				*dataBegin = 0;
			} else {
				*dataBegin &= (0xffffffff<<(32 - bitOffset1));
			}
			while (++dataBegin < dataTail) {
				*dataBegin = 0;
			}
			if (bitOffset2 != 0) {
				// bitOffset2 == 0 の時は前のユニットを 0 で詰めればよいので
				// 何もしない。bitOffset2 != 0 のときだけ処理する。
				*dataBegin &= ~(0xffffffff<<(32 - bitOffset2));
			}
		}
	}

	//
	//	FUNCTION
	//	_$$::_move -- ビットを移動する
	//
	void _move(ModUInt32* dataBegin_,
			   ModSize begin_, ModSize end_, ModSize newBegin_,
			   ModUInt32* targetBegin)
	{
		//	【注意】
		//	ModInvertedCoder::moveと基本的には同じ
		//	ただ、毎回メモリ確保しているので、最大値のバッファで行うように
		//	修正した
		
		ModUInt32 unit[IDBlock::BLOCK_SIZE];
		ModOsDriver::memset(unit, 0, sizeof(ModUInt32)*IDBlock::BLOCK_SIZE);
		ModSize newUnitNum(0);

		ModSize unitNum(((end_ - 1)>>5) - (begin_>>5) + 1);
		ModSize newEnd(newBegin_ + end_ - begin_);
		newUnitNum = ((newEnd - 1)>>5) - (newBegin_>>5) + 1;
		ModUInt32* tmpUnit = unit;
		ModUInt32* dataBegin = dataBegin_ + (begin_>>5);

		ModSize begin(begin_&31), end(end_&31), newBegin(newBegin_&31);
		ModSize shift(0);
		if (newBegin < begin) {
			; _TRMEISTER_ASSERT(begin != 0);
			shift = begin - newBegin;
			if (unitNum == 1) {
				; _TRMEISTER_ASSERT(newUnitNum == 1);
				*tmpUnit = 
					(((~(0xffffffff<<(32 - begin)))&(0xffffffff<<(32 - end)))&
					 *dataBegin)<<shift;
			} else {
				*tmpUnit = ((~(0xffffffff<<(32 - begin)))&*dataBegin)<<shift;
				ModSize tmpUnitNum(unitNum);
				while (--tmpUnitNum > 1) {
					++dataBegin;
					*tmpUnit |= 
						((0xffffffff<<(32 - shift))&(*dataBegin))>>(32 - shift);
					++tmpUnit;
					*tmpUnit = 
						((~(0xffffffff<<(32 - shift)))&(*dataBegin))<<shift;
				}
				++dataBegin;
				// end != 0  をはずしても Array bound error を起こさない？
				*tmpUnit |=
					((0xffffffff<<(32 - end))&*dataBegin)>>(32 - shift);
				if (unitNum == newUnitNum) {
					++tmpUnit;
					*tmpUnit = 
						((0xffffffff<<(32 - end))&*dataBegin)<<shift;
				}
			}
		} else if (begin < newBegin) {
			shift = newBegin - begin;
			if (newUnitNum == 1) {
				; _TRMEISTER_ASSERT(unitNum == 1);
				*tmpUnit =
					((((begin == 0) ? ModSizeMax : (~(0xffffffff<<(32 - begin))))&
					  (0xffffffff<<(32 - end)))&*dataBegin)>>shift;
			} else if (unitNum == 1) {
				; _TRMEISTER_ASSERT(newUnitNum == 2);
				*tmpUnit =
					((((begin == 0) ? ModSizeMax : (~(0xffffffff<<(32 - begin))))&
					  (0xffffffff<<(32 - end)))&*dataBegin)>>shift;
				++tmpUnit;
				*tmpUnit =
					((((begin == 0) ? ModSizeMax : (~(0xffffffff<<(32 - begin))))&
					  (0xffffffff<<(32 - end)))&*dataBegin)<<(32 - shift);
			} else {
				*tmpUnit =
					(((begin == 0) ? ModSizeMax : (~(0xffffffff<<(32 - begin))))&
					 *dataBegin)>>shift;
				ModSize tmpNewUnitNum(newUnitNum);
				while (--tmpNewUnitNum > 1) {
					++tmpUnit;
					*tmpUnit =
						((~(0xffffffff<<shift))&(*dataBegin))<<(32 - shift);
					++dataBegin;
					*tmpUnit |=
						((0xffffffff<<shift)&(*dataBegin))>>shift;
				}
				++tmpUnit;
				if (newUnitNum != unitNum) {
					*tmpUnit =
						(((~(0xffffffff<<shift))&(0xffffffff<<(32 - end)))&
						 (*dataBegin))<<(32 - shift);
				} else {
					*tmpUnit =
						((~(0xffffffff<<shift))&(*dataBegin))<<(32 - shift);
					++dataBegin;
					*tmpUnit |=
						((0xffffffff<<(32 - end))&*dataBegin)>>shift;
				}
			}
		} else {
			; _TRMEISTER_ASSERT(unitNum == newUnitNum);
			if (unitNum == 1) {
				*tmpUnit =
					(((begin == 0) ? ModSizeMax : ~(0xffffffff<<(32 - begin)))&
					 (0xffffffff<<(32 - end)))&*dataBegin;
			} else {
				*tmpUnit =
					((begin == 0) ? ModSizeMax : ~(0xffffffff<<(32 - begin)))&
					*dataBegin;
				ModSize tmpUnitNum(unitNum);
				while (--tmpUnitNum > 1) {
					++tmpUnit;
					++dataBegin;
					*tmpUnit = *dataBegin;
				}
				++tmpUnit;
				++dataBegin;
				*tmpUnit = (0xffffffff<<(32 - end))&*dataBegin;
			}
		}

		tmpUnit = unit;

		if (targetBegin == 0) {
			// データをセットする範囲をクリアする
			_setOff(dataBegin_, newBegin_, newEnd);
			dataBegin = dataBegin_ + (newBegin_>>5);
		} else {
			// データをセットする範囲をクリアする
			_setOff(targetBegin, newBegin_, newEnd);
			dataBegin = targetBegin + (newBegin_>>5);
		}

		while (tmpUnit < (unit + newUnitNum)) {
			*dataBegin |= *tmpUnit;
			++dataBegin;
			++tmpUnit;
		}
	}

	//
	//	FUNCTION
	//	_$$::_append -- PEGのビットを追加する
	//
	void _append(const ModSize value,
				 ModUInt32* dataBegin,
				 ModSize& bitOffset)
	{
		//	【注意】
		//	ModInvertedParameterizedExpGolombCoder::appendと同じ
		
		ModSize prefixBitLength(_getPrefixBitLength(value));

		_set(dataBegin, bitOffset + prefixBitLength,
			 prefixBitLength + LAMBDA1,
			 value + LAMBDA2);
		bitOffset += (prefixBitLength<<1) + LAMBDA1;
	}

	//
	//	FUNCTION
	//	_$$::_get -- PEGのビットから値を得る
	//
	ModBoolean _get(ModSize& value,
					const ModUInt32* dataBegin_,
					const ModSize bitLength_,
					ModSize& bitOffset_)
	{
		if (bitOffset_ >= bitLength_) {
			// ここは事前のチェックなので等号が必要
			return ModFalse;
		}
		// prefix 長をはかる
		dataBegin_ += bitOffset_>>5;
		ModSize shift(bitOffset_&31);
		ModSize bitMask(0x80000000>>shift);
		ModUInt32 tmp(*dataBegin_);

		if ( tmp&bitMask ) {
			// prefix がない場合
			++bitOffset_;			// skip separator
			shift += LAMBDA1;
			if (shift <= 32 ) {
				bitOffset_ += LAMBDA;
				value = LAMBDA2&(tmp>>(32 - (bitOffset_& 31)));
			} else {
				++dataBegin_;
				if ((bitOffset_&31) == 0)
					value = ((*dataBegin_)>>(32 - LAMBDA));
				else
				{
					// ２つのユニットに跨る場合
					value = (((ModUInt32Max>>((bitOffset_ &31)))&tmp)<< (shift - 32));
					value += (*dataBegin_)>>(64 - shift);
				}
				bitOffset_ += LAMBDA;
			}
			++value;
			return ModTrue;
		}
		ModSize prefixBitLength(0);
		do {
			++prefixBitLength;
			if (bitMask & 0x01)
			{
				if ((bitOffset_ + prefixBitLength) >= bitLength_)
					// これを外に出す方が遅くなる
					return ModFalse;
				++dataBegin_;
				tmp = *dataBegin_;
				bitMask = 0x80000000;
			}
			else
				bitMask >>= 1;
		} while ((tmp&bitMask) == 0);

		// tail 値を調べる
		bitOffset_ += prefixBitLength + 1;	// 現在位置を tail の先頭にする
		prefixBitLength += LAMBDA;
		bitMask = 1<<prefixBitLength;		// bitMask はもう使わない
		value = bitMask - LAMBDA2;
		shift = (bitOffset_&31) + prefixBitLength;
		if (shift <= 32) {
			// １つのユニットに収まる場合
			bitOffset_ += prefixBitLength;
			if (shift == prefixBitLength)
			{
				// tail が新しいユニットの先頭になる場合
				++dataBegin_;
				value += (*dataBegin_)>>(32 - prefixBitLength);
			} else
				value += (bitMask - 1)&(tmp>>(32 - (bitOffset_&31)));
		}
		else
		{
			// ２つのユニットに跨る場合
			++dataBegin_;
			value += ((ModUInt32Max>>(bitOffset_&31))&tmp)<<(shift - 32);
			value += (*dataBegin_)>>(64 - shift);
			bitOffset_ += prefixBitLength;
		}
		return ModTrue;
	}
}

//
//	FUNCTION public
//	Bitmap::IDBlock::IDBlock -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiFirstRowID_
//		最初のROWID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
IDBlock::IDBlock(ModUInt32 uiFirstRowID_)
{
	initialize(uiFirstRowID_);
}

//
//	FUNCTION public
//	Bitmap::IDBlock::initialize -- 初期化する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiFirstRowID_
//		先頭のROWID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
IDBlock::initialize(ModUInt32 uiFirstRowID_)
{
	ModOsDriver::Memory::reset(this, sizeof(IDBlock));
	m_uiFirstRowID = uiFirstRowID_;
	m_usCount = 1;
}

//
//	FUNCTION public
//	Bitmap::IDBlock::getLastID -- 最終のROWIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		最終のROWID
//
//	EXCEPTIONS
//
ModUInt32
IDBlock::getLastID() const
{
	ModUInt32 prevID = m_uiFirstRowID;
	ModUInt32 nextID = prevID;
	ModSize offset = 0;
	while ((nextID = getID(prevID, offset)) != prevID)
	{
		prevID = nextID;
	}
	return prevID;
}

//
//	FUNCTION public
//	Bitmap::IDBlock::getID -- ROWIDを1つ得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiLastID_
//		前回読み出したROWID
//	ModSize& uiOffset_
//		読み出すビット列のオフセット。
//		呼出し後は次のエントリのオフセットが設定される。
//
//	RETURN
//	ModUInt32
//		読み出したROWID
//		読みきっている場合は、引数 uiLastID_ と同じ値が返る
//
//	EXCEPTIONS
//
ModUInt32
IDBlock::getID(ModSize uiLastID_, ModSize& uiOffset_) const
{
	return read(uiLastID_, m_pBuffer, m_usBitLength, uiOffset_);
}

//
//	FUNCTION public
//	Bitmap::IDBlock::insert -- ROWIDを挿入する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		追加するROWID
//
//	RETURN
//	bool
//		挿入できた場合はtrue、領域が不足して追加できなかった場合はfalse
//
//	EXCEPTIONS
//
bool
IDBlock::insert(ModUInt32 uiRowID_)
{
	if (m_uiFirstRowID == uiRowID_)
	{
		// 配列中に同じ値がある場合
		return true;
	}
	
	if (m_uiFirstRowID > uiRowID_)
	{
		// 先頭の前に挿入する

		// 挿入する長さ
		ModSize uiLength = getBitLength(uiRowID_, m_uiFirstRowID);

		if ((uiLength + m_usBitLength) > _MAX_LENGTH)
			// 領域が足らない
			return false;

		// まずは後ろへ移動する
		move(m_pBuffer, uiLength,
			 m_pBuffer, 0, static_cast<ModSize>(m_usBitLength));

		// 書き出す場所をクリアする
		reset(m_pBuffer, 0, uiLength);

		// 書き出す
		ModSize uiOffset = 0;
		write(uiRowID_, m_uiFirstRowID, m_pBuffer, uiOffset);

		// ビット長を追加する
		m_usBitLength += static_cast<unsigned short>(uiLength);
		// 先頭ROWIDを上書きする
		m_uiFirstRowID = uiRowID_;
	}
	else if (getCount() == 1)
	{
		// まだ1件しか登録されていない。そのままwriteする
		// -> 1件は必ず書き込める領域があることが前提
		
		ModSize uiOffset = 0;
		write(m_uiFirstRowID, uiRowID_, m_pBuffer, uiOffset);
		m_usBitLength = static_cast<unsigned short>(uiOffset);
	}
	else
	{
		// 複数件登録されている

		// 挿入する位置を特定する
		ModUInt32 uiPrevID = m_uiFirstRowID;
		ModUInt32 uiNextID = m_uiFirstRowID;
		ModSize uiPrevOffset = 0;
		ModSize uiNextOffset = 0;
		
		while (uiNextID < uiRowID_)
		{
			uiPrevID = uiNextID;
			uiPrevOffset = uiNextOffset;

			if (uiNextOffset >= static_cast<ModSize>(m_usBitLength))
				// 最後まで読んだので break
				break;
		
			// uiRowID_を超えるまで読み進む
			uiNextID = getID(uiPrevID, uiNextOffset);
		}

		if (uiNextID == uiRowID_)
		{
			// 配列中に同じ値がある場合
			return true;
		}

		if (uiPrevID == uiNextID)
		{
			// 最後に追加
			
			return append(uiPrevID, uiRowID_);
		}
		else
		{
			// 途中に挿入

			//           uiPrevOffset   uiNextOffset          m_usBitLength
			//                ↓          ↓                    ↓
			//	<---><-PrevID-><--NextID--><---><---><----><---->
			//
			//                            ｜-----｜ ← uiMoved
			//  <---><-PrevID-><-RowID-><-NextID-><---><---><----><---->
			//                ｜---- uiLength ---｜
		
			; _TRMEISTER_ASSERT(uiPrevOffset < uiNextOffset);
			
			// 上書きする長さ
			ModSize uiLength = getBitLength(uiPrevID, uiRowID_)
				+ getBitLength(uiRowID_, uiNextID);
			// 移動する長さ
			ModSize uiMoved = uiLength - (uiNextOffset - uiPrevOffset);

			if ((uiMoved + m_usBitLength) > _MAX_LENGTH)
				// 領域が足らない
				return false;

			// まずは後ろへ移動する
			if (uiMoved)
			{
				move(m_pBuffer, uiNextOffset + uiMoved, m_pBuffer, uiNextOffset,
					 static_cast<ModSize>(m_usBitLength) - uiNextOffset);
			}
			// 上書きする場所をクリアする
			reset(m_pBuffer, uiPrevOffset, uiLength);

			// 上書きする
			write(uiPrevID, uiRowID_, m_pBuffer, uiPrevOffset);
			write(uiRowID_, uiNextID, m_pBuffer, uiPrevOffset);
			// ビット長を追加する
			m_usBitLength += static_cast<unsigned short>(uiMoved);
		}
	}

	++m_usCount;

	return true;
}

//
//	FUNCTION public
//	Bitmap::IDBlock::append -- ROWIDを末尾に追加する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiLastID_
//		このIDBlockの現在の最大値
//	ModUInt32 uiRowID_
//		新しく追加するROWID
//
//	RETURN
//	bool
//		挿入できた場合はtrue、領域が不足して追加できなかった場合はfalse
//
//	EXCEPTIONS
//
bool
IDBlock::append(ModUInt32 uiLastID_, ModUInt32 uiRowID_)
{
	//	【注意】
	//	引数 uiLastID_ と実際の最大値が異なっていた場合の動作は不正となる

	ModSize uiLength = getBitLength(uiLastID_, uiRowID_);
	if ((uiLength + m_usBitLength) > _MAX_LENGTH)
		// 領域が足らない
		return false;

	// 最後に追加する
	ModSize uiTotalLength = static_cast<ModSize>(m_usBitLength);
	write(uiLastID_, uiRowID_, m_pBuffer, uiTotalLength);
	; _TRMEISTER_ASSERT(uiLength + m_usBitLength == uiTotalLength);
	m_usBitLength = static_cast<unsigned short>(uiTotalLength);
	++m_usCount;

	return true;
}

//
//	FUNCTION public
//	Bitmap::IDBlock::erase -- ROWIDを削除する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowiD_
//		削除するROWID
//	ModUInt32& uiPrevID_
//		削除したものの直前のROWID。先頭を削除した場合はuiRowID_と同じ値
//
//	RETURN
//	bool
//		削除対象のROWIDが見つからない場合はfalse、削除できた場合はtrue
//
//	EXCEPTIONS
//
bool
IDBlock::erase(ModUInt32 uiRowID_, ModUInt32& uiPrevID_)
{
	if (m_uiFirstRowID == uiRowID_)
	{
		// 先頭の削除

		uiPrevID_ = uiRowID_;
		
		ModSize uiOffset = 0;
		ModUInt32 newFirstID = getID(m_uiFirstRowID, uiOffset);

		if (newFirstID != m_uiFirstRowID)
		{
			//  0         uiOffset                     m_usBitLength
			// ↓            ↓                         ↓
			//	<-newFirstID-><---><---><---><----><---->
			//               ｜-------------------------｜← uiSize
			// ｜------------｜← moveする
			//                            ｜------------｜← resetする
			//  <---><---><---><----><---->
			
			// 読み出せたのでビット列を前につめる

			// 移動する領域のサイズを求める
			ModSize uiSize = static_cast<ModSize>(m_usBitLength) - uiOffset;
			// uiOffsetから先頭に移動する
			move(m_pBuffer, 0, m_pBuffer, uiOffset, uiSize);
			// 移動したころりの部分のビットをクリアする
			reset(m_pBuffer, uiSize, uiOffset);
			// ビット長を短くする
			m_usBitLength -= static_cast<unsigned short>(uiOffset);
			// 先頭ROWIDを変更する
			m_uiFirstRowID = newFirstID;
		}
	}
	else
	{
		// その他の削除

		ModUInt32 uiPrevID = m_uiFirstRowID;
		ModUInt32 uiNextID = m_uiFirstRowID;
		ModSize uiPrevOffset = 0;
		ModSize uiNextOffset = 0;

		while (uiNextID < uiRowID_)
		{
			uiPrevID = uiNextID;
			uiPrevOffset = uiNextOffset;

			if (uiNextOffset >= static_cast<ModSize>(m_usBitLength))
				// 最後まで読んだので break
				break;

			// uiRowID_まで読み進む
			uiNextID = getID(uiPrevID, uiNextOffset);
		}

		if (uiNextID != uiRowID_)
			// 見つからなかった
			return false;

		// 削除する直前のROWIDを設定する
		uiPrevID_ = uiPrevID;

		if (uiNextOffset >= static_cast<ModSize>(m_usBitLength))
		{
			// 最後なので、resetするのみ
			
			// ビット長を修正する
			m_usBitLength -= static_cast<unsigned short>(
				uiNextOffset - uiPrevOffset);
			// 削除する部分をresetする
			reset(m_pBuffer, uiPrevOffset, uiNextOffset - uiPrevOffset);
		}
		else
		{
			// 途中なので、後ろをつめる

			// 今の uiNextID は uiRowID_ であるので、その次を読み込む
			uiNextID = getID(uiNextID, uiNextOffset);

			; _TRMEISTER_ASSERT(uiNextID != uiRowID_);

			//           uiPrevOffset       uiNextOffset    m_usBitLength
			//                ↓                 ↓              ↓
			//  <---><-PrevID-><-RowID-><-NextID-><---><---><---->
			//                ｜-----------------｜resetする
			//
			//                         uiPrevOffset
			//                            ↓
			//  <---><-PrevID-><--NextID--><---><---><---->
			//                            ｜-----｜moveする(uiMoved)
			//                                            ｜-----｜resetする

			// 上書きする部分をresetする
			reset(m_pBuffer, uiPrevOffset, uiNextOffset - uiPrevOffset);
			// 上書きする
			write(uiPrevID, uiNextID, m_pBuffer, uiPrevOffset);

			ModSize uiMoved = uiNextOffset - uiPrevOffset;
			; _TRMEISTER_ASSERT(uiMoved >= 0);
			if (uiMoved)
			{
				// 短くなった分を移動する
				move(m_pBuffer, uiPrevOffset, m_pBuffer, uiNextOffset,
					 static_cast<ModSize>(m_usBitLength) - uiNextOffset);
				// ビット長を修正する
				m_usBitLength -= static_cast<unsigned short>(uiMoved);
				// 移動した部分をresetする
				reset(m_pBuffer, static_cast<ModSize>(m_usBitLength), uiMoved);
			}
		}
	}

	--m_usCount;

	return true;
}

//
//	FUNCTION public
//	Bitmap::IDBlock::split -- 後ろ半分(件数ベース)を引数に分け与える
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::IDBlock* pDst_
//		分け与える先
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IDBlock::split(IDBlock* pDst_)
{
	int count = static_cast<int>(m_usCount);

	ModUInt32 uiPrevID = m_uiFirstRowID;
	ModSize uiPrevOffset = 0;

	// 半分まで読み飛ばす
	for (int i = 1; i < count/2; ++i)
	{
		uiPrevID = getID(uiPrevID, uiPrevOffset);
	}

	// 分け与える先頭を読む
	ModSize uiNextOffset = uiPrevOffset;
	ModUInt32 uiNextID = getID(uiPrevID, uiNextOffset);

	// 分け与える先にデータを書き出す
	pDst_->initialize(uiNextID);
	move(pDst_->m_pBuffer, 0, m_pBuffer, uiNextOffset,
		 static_cast<ModSize>(m_usBitLength) - uiNextOffset);
	pDst_->m_usBitLength
		= m_usBitLength - static_cast<unsigned short>(uiNextOffset);
	pDst_->m_usCount = static_cast<unsigned short>(count - count/2);

	// 分け与えたデータをresetする
	reset(m_pBuffer, uiPrevOffset,
		  static_cast<ModSize>(m_usBitLength) - uiPrevOffset);
	m_usBitLength = static_cast<unsigned short>(uiPrevOffset);
	m_usCount = static_cast<unsigned short>(count/2);
}

//
//	FUNCTION public
//	Bitmap::IDBlock::merge
//		-- 引数ブロックの内容をすべて自身のエントリとして追加する
//
//	NOTES
//
//	ARGUMENTS
//	const Bitmap::IDBlock* pSrc_
//		挿入するブロック
//
//	RETURN
//	bool
//		追加できた場合はtrue、領域が不足して追加できなかった場合はfalse
//
//	EXCEPTIONS
//
bool
IDBlock::merge(const IDBlock* pSrc_)
{
	if (pSrc_->getCount() == 0)
	{
		// 相手が空
		return true;
	}
	
	if (getCount() == 0)
	{
		// 自分が空
		Os::Memory::move(this, pSrc_, sizeof(IDBlock));
		return true;
	}
	
	// まず、最後のエントリを得る
	ModSize uiOffset = 0;
	ModUInt32 uiID = m_uiFirstRowID;
	while (uiOffset < static_cast<ModSize>(m_usBitLength))
	{
		uiID = getID(uiID, uiOffset);
	}

	// 追加するビット数を調べる
	ModSize uiBit
		= getBitLength(uiID, pSrc_->getFirstID()) + pSrc_->m_usBitLength;
	if ((uiBit + m_usBitLength) > _MAX_LENGTH)
		// 格納できない
		return false;

	// 先頭を追加する
	append(uiID, pSrc_->getFirstID());
	// ビットをコピーする
	move(m_pBuffer, static_cast<ModSize>(m_usBitLength),
		 pSrc_->m_pBuffer, 0, static_cast<ModSize>(pSrc_->m_usBitLength));
	m_usCount += (pSrc_->m_usCount - 1);	// 先頭分を差し引く
	m_usBitLength += pSrc_->m_usBitLength;

	return true;
}

//
//	FUNCTION public static
//	Bitmap::IDBlock::getMaxBitLength -- 最大ビット長を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		最大ビット長
//
//	EXCEPTIONS
//
ModSize
IDBlock::getMaxBitLength()
{
	return _MAX_LENGTH;
}

//
//	FUNCTION private
//	Bitmap::IDBlock::move -- ビット単位のmove
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32* pDst_
//		移動先のバッファの先頭
//	ModSize uiDstBegin_
//		移動先のバッファの先頭からのオフセット(ビット単位)
//	const ModUInt32* pSrc_
//		移動元のバッファの先頭
//	ModSize uiSrcBegin_
//		移動元のバッファの先頭からのオフセット(ビット単位)
//	ModSize uiSize_
//		移動する領域の大きさ(ビット単位)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
//static
void
IDBlock::move(ModUInt32* pDst_, ModSize uiDstBegin_,
			  const ModUInt32* pSrc_, ModSize uiSrcBegin_,
			  ModSize uiSize_)
{
	if (uiSize_)
	{
		_move(const_cast<ModUInt32*>(pSrc_), uiSrcBegin_, uiSrcBegin_ + uiSize_,
			  uiDstBegin_, pDst_);
	}
}

//
//	FUNCTION private
//	Bitmap::IDBlock::reset -- ビット単位のreset
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32* pUnit_
//		ビットをクリアするバッファの先頭
//	ModSize uiBegin_
//		クリアするビットのオフセット(ビット単位)
//	ModSize uiSize_
//		ビットをクリアする大きさ(ビット単位)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
//static
void
IDBlock::reset(ModUInt32* pBuffer_, ModSize uiBegin_, ModSize uiSize_)
{
	if (uiSize_)
	{
		_setOff(pBuffer_, uiBegin_, uiBegin_ + uiSize_);
	}
}

//
//	FUNCTION private
//	Bitmap::IDBlock::getBitLength -- ビット長を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiLastRowID_
//		直前のROWID
//	ModUInt32 uiRowID_
//		圧縮するROWID
//
//	RETURN
//	ModSize
//		uiRowID_の値を圧縮した後のビット長
//
//	EXCEPTIONS
//
//static
ModSize
IDBlock::getBitLength(ModUInt32 uiLastRowID_, ModUInt32 uiRowID_)
{
	return (_getPrefixBitLength(uiRowID_ - uiLastRowID_) << 1) + LAMBDA1;
}

//
//	FUNCTION private
//	Bitmap::IDBlock::write -- ビットを書き出す
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiLastRowID_
//		直前のROWID
//	ModUInt32 uiRowID_
//		圧縮して書き込むROWID
//	ModUInt32* pBuffer_
//		書き込むバッファの先頭
//	ModSize& uiOffset_
//		バッファの書き込む位置。書き込んだ領域の直後が設定される。
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
//static
void
IDBlock::write(ModUInt32 uiLastRowID_, ModUInt32 uiRowID_,
			   ModUInt32* pBuffer_, ModSize& uiOffset_)
{
#ifdef DEBUG
	ModSize saveOffset = uiOffset_;
#endif
	_append(uiRowID_ - uiLastRowID_, pBuffer_, uiOffset_);
#ifdef DEBUG
	ModSize v = 0;
	_get(v, pBuffer_, uiOffset_, saveOffset);
	; _TRMEISTER_ASSERT(v == (uiRowID_ - uiLastRowID_));
#endif
}

//
//	FUNCTION private
//	Bitmap::IDBlock::read -- ビットを読み出す
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiLastRowID_
//		直前のROWID
//	const ModUInt32* pBuffer_
//		バッファの先頭
//	ModSize uiBitLength_
//		バッファのビット長
//	ModSize& uiOffset_
//		読み出す位置。呼出し後は次の位置が設定される
//
//	RETURN
//	ModUInt32
//		読み出したROWID
//
//	EXCEPTIONS
//
//static
ModUInt32
IDBlock::read(ModUInt32 uiLastRowID_, const ModUInt32* pBuffer_,
			  ModSize uiBitLength_, ModSize& uiOffset_)
{
	ModSize v = 0;
	_get(v, pBuffer_, uiBitLength_, uiOffset_);
	return uiLastRowID_ + v;
}

//
//	Copyright (c) 2007, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

