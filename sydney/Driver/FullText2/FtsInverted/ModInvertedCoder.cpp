// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedCoder.cpp -- 符合器の実装
// 
// Copyright (c) 1997, 1998, 2000, 2001, 2002, 2003, 2004, 2005, 2010, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "ModMessage.h"
#include "ModOsDriver.h"
#include "ModUnicodeCharTrait.h"

#include "ModInvertedCoder.h"
#include "ModInvertedExpGolombCoder.h"
#include "ModInvertedParameterizedExpGolombCoder.h"
#include "ModInvertedExtendedGolombCoder.h"
#include "ModInvertedVoidCoder.h"
#include "ModInvertedUnaryCoder.h"

#include "Common/Message.h"
#include "Exception/BadArgument.h"

_SYDNEY_USING

//
// CONST
// ModInvertedExpGolombCoder::tokenizerName -- 符合化器の名称
//
// NOTES
// ModInvertedExpGolombCoder の符合化器の名称を表す
//
/*static*/
const ModUnicodeChar ModInvertedExpGolombCoder::coderName[] = {'E','X','G',0};

//
// CONST
// ModInvertedVoidCoder::tokenizerName -- 符合化器の名称
//
// NOTES
// ModInvertedVoidCoder の符合化器の名称を表す
//
/*static*/
const ModUnicodeChar ModInvertedVoidCoder::coderName[] = {'V','O','I','D',0};

//
// CONST
// ModInvertedExpUnaryCoder::tokenizerName -- 符合化器の名称
//
// NOTES
// ModInvertedUnaryCoder の符合化器の名称を表す
//
/*static*/
const ModUnicodeChar ModInvertedUnaryCoder::coderName[] = {'U','N','A',0};

//
// FUNCTION
// ModInvertedCoder::create --- 符号器の生成
//
// NOTES
// parameter に合致する符号器のインスタンスを生成する。
// 新たな符号器クラスを生成した場合、ここに追加しなければならない。
//
// ARGUMENTS
// const Parameter& parameter
//		生成する符号器のパラメータ
//
// RETRUN
// 生成された符号器
//
// EXCEPTIONS
// ModInvertedErrorInvalidCoderType
//		パラメータで指定された符号器タイプが不正である
// それ以外は下位の例外をそのまま返す
//
ModInvertedCoder*
ModInvertedCoder::create(const ModUnicodeString& description)
{
	try {
		ModSize length(description.getLength()), nameLen;

		if (description.compare(ModInvertedExpGolombCoder::coderName) == 0) {
			return new ModInvertedExpGolombCoder;
		}
		if (description.compare(ModInvertedVoidCoder::coderName) == 0) {
			return new ModInvertedVoidCoder;
		}

		if (description.compare(ModInvertedUnaryCoder::coderName) == 0) {
			return new ModInvertedUnaryCoder;
		}

		nameLen = ModUnicodeCharTrait::length(
			ModInvertedParameterizedExpGolombCoder::coderName);
		if (description.compare(
			ModInvertedParameterizedExpGolombCoder::coderName, nameLen) == 0 &&
				 (length == nameLen || description[nameLen] == ':')) {
			return new ModInvertedParameterizedExpGolombCoder(
				description.copy(nameLen + 1));
		}
		nameLen = ModUnicodeCharTrait::length(
			ModInvertedExtendedGolombCoder::coderName);
		if (description.compare(
			ModInvertedExtendedGolombCoder::coderName, nameLen) == 0 &&
				 (length == nameLen || description[nameLen] == ':')) {
			return new ModInvertedExtendedGolombCoder(
				description.copy(nameLen + 1));
		}

		// CoderType が指定されていない場合
		SydErrorMessage << "invalied description: " << description << ModEndl;
		_SYDNEY_THROW0(Exception::BadArgument);

	} catch (...) {
		_SYDNEY_RETHROW;
	}
	return 0;
}


//
// FUNCTION
// ModInvertedCoder::dump --- データ領域のダンプ
//
// NOTES
// dataBegin から dataEnd までをビット単位に ModDebugMessage に出力する
//
// ARGUMENTS
// const Unit* dataBegin
//		領域の先頭
// const Unit* dataEnd
//		領域の末尾
//
// RETRUN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedCoder::dump(const Unit* dataBegin,
					   const Unit* dataEnd)
{
	unsigned int i, j = 0;
	while (dataBegin < dataEnd) {
		for (i = 0x80000000; i > 0; i >>= 1)
#ifdef DEBUG0
			ModDebugMessage << (int)(!!(i&(*dataBegin)));
#else
			ModMessage << (int)(!!(i&(*dataBegin)));
#endif
		if (++j%2) {
#ifdef DEBUG0
			ModDebugMessage << ' ';
#else
			ModMessage << ' ';
#endif
		} else {
#ifdef DEBUG0
			ModDebugMessage << ModEndl;
#else
			ModMessage << ModEndl;
#endif
		}
		++dataBegin;
	}
	if (j%2) {
#ifdef DEBUG0
		ModDebugMessage << ModEndl;
#else
		ModMessage << ModEndl;
#endif
	}
}
		

//
// FUNCTION
// ModInvertedCoder::move -- データの移動
//
// NOTES
// 領域に重なりがあっても良い。だたし、十分な領域があるかの検査は行わない。
//
// ARGUMENTS
// Unit* dataBegin_
//		データの開始位置
// ModSize begin_
//		移動範囲の開始ビットオフセット
// ModSize end_
//		移動範囲の終了ビットオフセット
// ModSize newBegin_
//		移動先のビットオフセット
// Unit* targetBegin
//		移動のデータの開始位置（省略可能: 省略時は dataBegin_ となる）
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedCoder::move(Unit* dataBegin_,
					   ModSize begin_, ModSize end_, ModSize newBegin_,
					   Unit* targetBegin)
{
	if (begin_ == newBegin_ && targetBegin == 0) {
/* purecov:begin deadcode */
		return;
/* purecov:end */
	}

	Unit* unit = 0;
	ModSize newUnitNum(0);

	try {
		; ModAssert(begin_ < end_);

		ModSize unitNum(((end_ - 1)>>5) - (begin_>>5) + 1);
		ModSize newEnd(newBegin_ + end_ - begin_);
		newUnitNum = ((newEnd - 1)>>5) - (newBegin_>>5) + 1;
		unit = (Unit*)
			ModOsDriver::Memory::alloc(sizeof(Unit)*newUnitNum);
		Unit* tmpUnit = unit;

		ModOsDriver::memset(tmpUnit, 0, sizeof(Unit)*newUnitNum);

		Unit* dataBegin = dataBegin_ + (begin_>>5);

		ModSize begin(begin_&31), end(end_&31), newBegin(newBegin_&31);
		ModSize shift(0);
		if (newBegin < begin) {
			; ModAssert(begin != 0);
			shift = begin - newBegin;
			if (unitNum == 1) {
				; ModAssert(newUnitNum == 1);
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
				; ModAssert(unitNum == 1);
				*tmpUnit =
					((((begin == 0) ? ModSizeMax : (~(0xffffffff<<(32 - begin))))&
					  (0xffffffff<<(32 - end)))&*dataBegin)>>shift;
			} else if (unitNum == 1) {
				; ModAssert(newUnitNum == 2);
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
			; ModAssert(unitNum == newUnitNum);
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
			setOff(dataBegin_, newBegin_, newEnd);
			dataBegin = dataBegin_ + (newBegin_>>5);
		} else {
			// データをセットする範囲をクリアする
			setOff(targetBegin, newBegin_, newEnd);
			dataBegin = targetBegin + (newBegin_>>5);
		}

		while (tmpUnit < (unit + newUnitNum)) {
			*dataBegin |= *tmpUnit;
			++dataBegin;
			++tmpUnit;
		}

		ModOsDriver::Memory::free(unit);

	} catch (...) {
		SydErrorMessage << "move failed: begin=" << begin_
						<< " end=" << end_ << " newBegin=" << newBegin_
						<< ModEndl;
		if (unit != 0) {
			ModOsDriver::Memory::free(unit);
		}
		_SYDNEY_RETHROW;
    }
}


//
// Copyright (c) 1997, 1998, 2000, 2001, 2002, 2003, 2004, 2005, 2010, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
