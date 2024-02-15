// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Integer64Data.cpp -- 64 ビット長整数型データ関連の関数定義
// 
// Copyright (c) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Common";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "SyInclude.h"

#include "Common/Assert.h"
#include "Common/ClassID.h"
#include "Common/DecimalData.h"
#include "Common/DoubleData.h"
#include "Common/FloatData.h"
#include "Common/Integer64Data.h"
#include "Common/IntegerData.h"
#include "Common/NullData.h"
#include "Common/StringData.h"
#include "Common/UnicodeString.h"
#include "Common/UnsignedInteger64Data.h"
#include "Common/UnsignedIntegerData.h"

#include "Exception/BadArgument.h"
#include "Exception/ClassCast.h"
#include "Exception/DivisionByZero.h"
#include "Exception/NullNotAllowed.h"
#include "Exception/NumericValueOutOfRange.h"
#include "Os/Limits.h"
#include "Os/Memory.h"

#include "ModAutoPointer.h"
#include "ModUnicodeString.h"
#include "ModUnicodeOstrStream.h"

#include <iostream>
using namespace std;

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace {

	// ダンプサイズ
	const ModSize _iDumpSize = sizeof(ModInt64);

	// 最小値・最大値の定義
	const ModInt64 llMax = Os::Limits<ModInt64>::getMax();
	const ModInt64 llMin = Os::Limits<ModInt64>::getMin();
}

//static
ModSize
Integer64Data::getArchiveSize()
{
	return _iDumpSize;
}

// FUNCTION public
//	Common::Integer64Data::hashCode -- ハッシュコードを取り出す
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
Integer64Data::
hashCode() const
{
	if (isNull()) return 0;

	ModSize high = static_cast<ModSize>(m_llValue >> 32);
	ModSize low = static_cast<ModSize>(m_llValue & 0xffffffff);
	return (high << 4) + low;
}

//	FUNCTION public
//	Common::Integer64Data::Integer64Data -- コンストラクタ(1)
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

Integer64Data::Integer64Data()
	: ScalarData(DataType::Integer64),
	  m_llValue(0)
{}

//	FUNCTION public
//	Common::Integer64Data::Integer64Data -- コンストラクタ(2)
//
//	NOTES
//
//	ARGUMENTS
//	ModInt64 llValue_
//		データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

Integer64Data::Integer64Data(ModInt64 llValue_)
	: ScalarData(DataType::Integer64),
	  m_llValue(llValue_)
{}

//	FUNCTION public
//	Common::Integer64Data::~Integer64Data -- デストラクタ
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

Integer64Data::~Integer64Data()
{}

//	FUNCTION private
//	Common::Integer64Data::serialize_NotNull -- シリアル化
//
//	NOTES
//
//	ARGUMENTS
//	ModArchive& cArchiver_
//		アーカイバー
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Integer64Data::serialize_NotNull(ModArchive& cArchiver_)
{
	; _TRMEISTER_ASSERT(!isNull());

	ScalarData::serialize_NotNull(cArchiver_);
	if (cArchiver_.isStore())
		//書出し
		cArchiver_ << m_llValue;
	else
		//読出し
		cArchiver_ >> m_llValue;
}

//	FUNCTION private
//	Common::Integer64Data::copy_NotNull -- コピーする
//
//	NOTES
//	自分自身のコピーを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::Data::Pointer
//		自分自身のコピー
//
//	EXCEPTIONS

Data::Pointer
Integer64Data::copy_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return new Integer64Data(*this);
}

//	FUNCTION private
//	Common::Integer64Data::cast_NotNull -- 指定の型にキャストする
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataType::Type eType_
//		キャストする型
//
//	RETURN
//	Common::Data::Pointer
//		キャストしたデータ。
//
//	EXCEPTIONS
//	Exception::ClassCast
//		キャストに失敗した
//	Exception::NumericValueOutOfRange
//		キャスト先で表現できる範囲を超えていた

Data::Pointer
Integer64Data::cast_NotNull(DataType::Type eType_, bool bForAssign_ /* = false */) const
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (eType_)	{
	case DataType::Integer:
		if (!(m_llValue > Os::Limits<int>::getMax() ||
			  m_llValue < Os::Limits<int>::getMin()))
			return new IntegerData(static_cast<int>(m_llValue));
		break;

	case DataType::UnsignedInteger:
		if (!(m_llValue > Os::Limits<unsigned int>::getMax() ||
			  m_llValue < Os::Limits<unsigned int>::getMin()))
			return new UnsignedIntegerData(
				static_cast<unsigned int>(m_llValue));
		break;

	case DataType::Integer64:
		return copy();
	case DataType::UnsignedInteger64:
		return new UnsignedInteger64Data(static_cast<ModUInt64>(m_llValue));
	case DataType::String:
		return new StringData(getString());
	case DataType::Float:
		return new FloatData(static_cast<float>(m_llValue));
	case DataType::Double:
		return new DoubleData(static_cast<double>(m_llValue));
	case DataType::Decimal:
		{
			const DecimalData* pDecimal = _SYDNEY_DYNAMIC_CAST(const DecimalData*, getTargetData());
			if (0 != pDecimal)
			{
				ModAutoPointer<DecimalData> pData =
					new DecimalData(pDecimal->getPrecision(), pDecimal->getScale());
				if (pData->castFromInteger64(*this, bForAssign_))
					return pData.release();
			}
		}
		break;
	case DataType::Null:
		return NullData::getInstance();
	default:
		_TRMEISTER_THROW0(Exception::ClassCast);
	}

	if (bForAssign_)
		// 代入のためのキャストの場合エラーにする
		_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);
	else
		// 比較のためならNullにする
		return NullData::getInstance();
}
Data::Pointer
Integer64Data::castToDecimal(bool bForAssign_) const
{
	DecimalData cDecimal(0,0);
	if (cDecimal.castFromInteger64(*this, bForAssign_))
		return new DecimalData(cDecimal);
	else
		return NullData::getInstance();

}
//	FUNCTION private
//	Common::Integer64Data::getString_NotNull -- 文字列でデータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		文字列にしたデータ
//
//	EXCEPTIONS
//	なし

ModUnicodeString
Integer64Data::getString_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	ModUnicodeOstrStream ostr;
	ostr << m_llValue;
	return ModUnicodeString(ostr.getString());
}

// FUNCTION public
//	Common::Integer64Data::getInt_NotNull -- 数値で得る(自分自身が NULL 値でない)
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Integer64Data::
getInt_NotNull() const
{
	if (m_llValue > Os::Limits<int>::getMax()
		|| m_llValue < Os::Limits<int>::getMin()) {
		_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);
	}
	return static_cast<int>(m_llValue);
}

// FUNCTION public
//	Common::Integer64Data::getUnsignedInt_NotNull -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	unsigned int
//
// EXCEPTIONS

//virtual
unsigned int
Integer64Data::
getUnsignedInt_NotNull() const
{
	if (m_llValue > Os::Limits<unsigned int>::getMax()
		|| m_llValue < 0) {
		_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);
	}
	return static_cast<unsigned int>(m_llValue);
}

//	FUNCTION public
//	Common::Integer64Data::getValue -- 値を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModInt64
//		データ
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

ModInt64
Integer64Data::getValue() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return m_llValue;
}

//	FUNCTION public
//	Common::Integer64Data::setValue -- 値を設定する
//
//	NOTES
//
//	ARGUMENTS
//	ModInt64 llValue_
//		データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
Integer64Data::setValue(ModInt64 llValue_)
{
	m_llValue = llValue_;

	// NULL 値でなくする

	setNull(false);
}

//	FUNCTION private
//	Common::Integer64Data::compareTo_NoCast -- 大小比較を行う
//
//	NOTES
//		比較するデータは自分自身と同じ型である必要がある
//
//	ARGUMENTS
//		Common::Data&	r
//			右辺に与えられたデータ
//
//	RETURN
//		0
//			左辺と右辺は等しい
//		-1
//			左辺のほうが小さい
//		1
//			右辺のほうが小さい
//
//	EXCEPTIONS
//		なし

int
Integer64Data::compareTo_NoCast(const Data& r) const
{
	if (isNull())
		return NullData::getInstance()->compareTo(&r);

	; _TRMEISTER_ASSERT(r.getType() == DataType::Integer64);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	const Integer64Data& data = _SYDNEY_DYNAMIC_CAST(const Integer64Data&, r);

	return (getValue() == data.getValue()) ? 0 :
		(getValue() < data.getValue()) ? -1 : 1;
}

//	FUNCTION private
//	Common::Integer64Data::assign_NoCast -- 代入を行う
//
//	NOTE
//
//	ARGUMENTS
//		Common::Data&	r
//			右辺に与えられたデータ
//			
//	RETURN
//		true
//			演算結果は NULL でない
//		false
//			演算結果は NULL である
//
//	EXCEPTIONS

bool 
Integer64Data::assign_NoCast(const Data& r)
{
	if (r.isNull()) {
		setNull();
		return false;
	}

	; _TRMEISTER_ASSERT(r.getType() == getType());
	; _TRMEISTER_ASSERT(!r.isNull());

	const Integer64Data& data =	_SYDNEY_DYNAMIC_CAST(const Integer64Data&, r);
	setValue(data.getValue());
	return true;
}

//	FUNCTION private
//	Common::Integer64Data::operateWith_NoCast -- 四則演算を行う
//
//	NOTE
//
//	ARGUMENTS
//		DataOperation::Type	op
//			行う演算の種類
//		Common::Data&	r
//			右辺に与えられたデータ
//			
//	RETURN
//		true
//			演算結果は NULL でない
//		false
//			演算結果は NULL である
//
//	EXCEPTIONS

bool 
Integer64Data::operateWith_NoCast(
	DataOperation::Type op, const Data& r)
{
	; _TRMEISTER_ASSERT(r.getType() == DataType::Integer64);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	const Integer64Data& data =	_SYDNEY_DYNAMIC_CAST(const Integer64Data&, r);
	ModInt64 llRhs = data.getValue();

	switch (op)	{
	case DataOperation::Addition:
		if (!((m_llValue>0 && llRhs>0 && llRhs > llMax-m_llValue) ||
			  (m_llValue<0 && llRhs<0 && llRhs < llMin-m_llValue))) {
			m_llValue += llRhs;
			return true;
		}
		break;

	case DataOperation::Subtraction:
		if (!((m_llValue>0 && llRhs<0 && m_llValue > llMax+llRhs) ||
			  (m_llValue<0 && llRhs>0 && m_llValue < llMin+llRhs))) {
			m_llValue -= llRhs;
			return true;
		}
		break;

	case DataOperation::Multiplication:
		// 精密な判定式とはいえないが…。
		// -> 割り算が0に近いほうに切り捨てである限り精密
		//    ↑この動作はたいていのコンパイラーではOKだが、
		//      ANSI Cの仕様では実装依存らしいので移植時に注意が必要
		if (!((m_llValue>  1 && llRhs>  1 && m_llValue > llMax/llRhs) ||
			  (m_llValue<=-1 && llRhs<=-1 && m_llValue < llMax/llRhs) ||
			  (m_llValue>  1 && llRhs< -1 && m_llValue > llMin/llRhs) ||
			  (m_llValue< -1 && llRhs>  1 && m_llValue < llMin/llRhs))) {
			m_llValue *= llRhs;
			return true;
		}
		break;

	case DataOperation::Division:
		if (llRhs == 0) {
				_TRMEISTER_THROW0(Exception::DivisionByZero);
		}
		if (!(m_llValue == llMin && llRhs == -1)) {
			m_llValue /= llRhs;
			return true;
		}
		break;
	case DataOperation::Modulus:
		{
			if (llRhs == 0) {
				_TRMEISTER_THROW0(Exception::DivisionByZero);
			}
			if (llRhs == 1 || llRhs == -1) {
				m_llValue = 0;
			} else {
				m_llValue %= llRhs;
			}
			return true;
		}
	default:
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	setNull();
	return false;
}

//	FUNCTION private
//	Common::Integer64Data::operateWith_NotNull -- 単項演算を行う。
//
//	NOTE
//
//	ARGUMENTS
//		DataOperation::Type eOperation_
//			
//		Data::Pointer& pResult_
//			
//	RETURN
//		bool
//
//	EXCEPTIONS
//		BadArgument

bool
Integer64Data::operateWith_NotNull(
	DataOperation::Type eOperation_, Data::Pointer& pResult_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (eOperation_) {
	case DataOperation::Negation:
		if (m_llValue != llMin) {
			pResult_ = new Integer64Data(-m_llValue);
			return true;
		}
		break;

	case DataOperation::AbsoluteValue:
		if (m_llValue != llMin) {
			pResult_ = new Integer64Data(
				(m_llValue >= 0) ? m_llValue : -m_llValue);
			return true;
		}
		break;

	default:
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	pResult_ = NullData::getInstance();
	return false;
}

//単項演算関数その2
bool
Integer64Data::operateWith_NotNull(DataOperation::Type eOperation_)
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (eOperation_) {
	case DataOperation::Increment:
		if (m_llValue != llMax) {
			++m_llValue;
			return true;
		}
		break;

	case DataOperation::Decrement:
		if (m_llValue != llMin) {
			--m_llValue;
			return true;
		}
		break;

	default:
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	return false;
}

//	FUNCTION private
//	Common::Integer64Data::getClassID_NotNull -- クラスIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		Common::Integer64DataクラスのクラスID
//
//	EXCEPTIONS
//	なし

int
Integer64Data::getClassID_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return ClassID::Integer64DataClass;
}

//	FUNCTION private
//	Common::Integer64Data::print_NotNull -- 値を表示する
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

void
Integer64Data::print_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	cout << "integer64: "
		 << ModUnicodeString(getString()).getString(Common::LiteralCode)
		 << endl;
}

//	FUNCTION private
//	Common::Integer64Data::isAbleToDump_NotNull --
//		dumpできるかを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//
//	EXCEPTIONS

//virtual
bool
Integer64Data::isAbleToDump_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return true;
}

//	FUNCTION private
//	Common::Integer64Data::isFixedSize_NotNull --
//		常に固定長であるかを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//
//	EXCEPTIONS

//virtual
bool
Integer64Data::isFixedSize_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return true;
}

//	FUNCTION private
//	Common::Integer64Data::getDumpSize_NotNull --
//		ダンプサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//
//	EXCEPTIONS

//virtual
ModSize
Integer64Data::getDumpSize_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return _iDumpSize;
}

//	FUNCTION public
//	Common::Integer64Data::setDumpedValue --
//		dumpされたデータから自身の値をsetする
//
//	NOTES
//
//	ARGUMENTS
//		const char* pszValue_
//			dumpされた領域の先頭
//		ModSize uSize_(省略可)
//			指定した場合正しい値かを検証する
//
//	RETURN
//		値に使ったバイト数
//
//	EXCEPTIONS

//virtual
ModSize
Integer64Data::setDumpedValue(const char* pszValue_)
{
	Os::Memory::copy(&m_llValue, pszValue_, _iDumpSize);

	setNull(false);

	return _iDumpSize;
}

//virtual
ModSize
Integer64Data::setDumpedValue(const char* pszValue_, ModSize uSize_)
{
	if (uSize_ != _iDumpSize)
		_TRMEISTER_THROW0(Exception::BadArgument);

	return setDumpedValue(pszValue_);
}

//virtual
ModSize
Integer64Data::setDumpedValue(ModSerialIO& cSerialIO_)
{
	cSerialIO_.readSerial(&m_llValue, _iDumpSize, ModSerialIO::dataTypeInt64);

	setNull(false);

	return _iDumpSize;
}

//virtual
ModSize
Integer64Data::setDumpedValue(ModSerialIO& cSerialIO_, ModSize uSize_)
{
	if (uSize_ != _iDumpSize)
		_TRMEISTER_THROW0(Exception::BadArgument);

	return setDumpedValue(cSerialIO_);
}

//virtual
ModSize
Integer64Data::dumpValue_NotNull(ModSerialIO& cSerialIO_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	cSerialIO_.writeSerial(&m_llValue, _iDumpSize, ModSerialIO::dataTypeInt64);
	return getDumpSize();
}

//	FUNCTION private
//	Common::Integer64Data::dumpValue_NotNull --
//		自身の値をdumpする
//
//	NOTES
//
//	ARGUMENTS
//		char* pszResult_
//			dumpする領域の先頭
//
//	RETURN
//		値に使ったバイト数
//
//	EXCEPTIONS

//virtual
ModSize
Integer64Data::dumpValue_NotNull(char* pszResult_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	Os::Memory::copy(pszResult_, &m_llValue, _iDumpSize);
	return getDumpSize();
}

//
//	Copyright (c) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
