// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UnsignedInteger64Data.cpp -- 64 ビット長非負整数型データ関連の関数定義
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
#include "Common/Integer64Data.h"
#include "Common/IntegerData.h"
#include "Common/DoubleData.h"
#include "Common/FloatData.h"
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

#include "ModUnicodeString.h"
#include "ModUnicodeOstrStream.h"

#include <iostream>
using namespace std;

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace {
	// ダンプサイズ
	const ModSize _iDumpSize = sizeof(ModUInt64);

	// 最大値・最小値の定義
	const ModUInt64 _ullMax = Os::Limits<ModUInt64>::getMax();
	const ModUInt64 _ullMin = Os::Limits<ModUInt64>::getMin();
}	

//static
ModUInt64
UnsignedInteger64Data::getMaxValue()
{
	return _ullMax;
}

//static
ModSize
UnsignedInteger64Data::getArchiveSize()
{
	return _iDumpSize;
}

//	FUNCTION public
//	Common::UnsignedInteger64Data::UnsignedInteger64Data -- コンストラクタ(1)
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

UnsignedInteger64Data::UnsignedInteger64Data()
	: ScalarData(DataType::UnsignedInteger64),
	  m_ullValue(0)
{}

//	FUNCTION public
//	Common::UnsignedInteger64Data::UnsignedInteger64Data -- コンストラクタ(2)
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt64 ullValue_
//		データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

UnsignedInteger64Data::UnsignedInteger64Data(ModUInt64 ullValue_)
	: ScalarData(DataType::UnsignedInteger64),
	  m_ullValue(ullValue_)
{}

//	FUNCTION public
//	Common::UnsignedInteger64Data::~UnsignedInteger64Data -- デストラクタ
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

UnsignedInteger64Data::~UnsignedInteger64Data()
{}

// FUNCTION public
//	Common::UnsignedInteger64Data::hashCode -- ハッシュコードを取り出す
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
UnsignedInteger64Data::
hashCode() const
{
	if (isNull()) return 0;

	ModSize high = static_cast<ModSize>(m_ullValue >> 32);
	ModSize low = static_cast<ModSize>(m_ullValue & 0xffffffff);
	return (high << 4) + low;
}

//	FUNCTION private
//	Common::UnsignedInteger64Data::serialize_NotNull -- シリアル化
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
UnsignedInteger64Data::serialize_NotNull(ModArchive& cArchiver_)
{
	; _TRMEISTER_ASSERT(!isNull());

	ScalarData::serialize_NotNull(cArchiver_);
	if (cArchiver_.isStore())
		//書出し
		cArchiver_ << m_ullValue;
	else
		//読出し
		cArchiver_ >> m_ullValue;
}

//	FUNCTION private
//	Common::UnsignedInteger64Data::copy_NotNull -- コピーする
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
UnsignedInteger64Data::copy_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return new UnsignedInteger64Data(*this);
}

//	FUNCTION private
//	Common::UnsignedInteger64Data::cast_NotNull -- 指定の型にキャストする
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
UnsignedInteger64Data::cast_NotNull(DataType::Type eType_, bool bForAssign_ /* = false */) const
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (eType_)	{
	case DataType::Integer:
		; _TRMEISTER_ASSERT(Os::Limits<int>::getMin() < 0);
		if (m_ullValue <= Os::Limits<int>::getMax())
			return new IntegerData(static_cast<int>(m_ullValue));
		break;

	case DataType::UnsignedInteger:
		if (!(m_ullValue > Os::Limits<unsigned int>::getMax() ||
			  m_ullValue < Os::Limits<unsigned int>::getMin()))
			return new UnsignedIntegerData(
				static_cast<unsigned int>(m_ullValue));
		break;

	case DataType::Integer64:
		; _TRMEISTER_ASSERT(Os::Limits<ModInt64>::getMin() < 0);
		if (m_ullValue <= static_cast<ModUInt64>(Os::Limits<ModInt64>::getMax()))
			return new Integer64Data(static_cast<ModInt64>(m_ullValue));
		break;
	case DataType::UnsignedInteger64:
		return copy();
	case DataType::String:
		return new StringData(getString());
	case DataType::Float:
		if (m_ullValue <= static_cast<ModUInt64>(Os::Limits<ModInt64>::getMax()))
			return new FloatData(static_cast<float>(
				static_cast<ModInt64>(m_ullValue)));
		break;
	case DataType::Double:
		if (m_ullValue <= static_cast<ModUInt64>(Os::Limits<ModInt64>::getMax()))
			return new DoubleData(static_cast<double>(
				static_cast<ModInt64>(m_ullValue)));
		break;
	case DataType::Null:
		return NullData::getInstance();
	default:
		_TRMEISTER_THROW0(Exception::ClassCast);
	}
	if (bForAssign_)
		// 代入のためならエラー
		_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);
	else
		// 比較ならNull
		return NullData::getInstance();
}

//	FUNCTION private
//	Common::UnsignedInteger64Data::getString_NotNull -- 文字列でデータを得る
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
UnsignedInteger64Data::getString_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	ModUnicodeOstrStream ostr;
	ostr << m_ullValue;
	return ModUnicodeString(ostr.getString());
}

// FUNCTION public
//	Common::UnsignedInteger64Data::getInt_NotNull -- 数値で得る(自分自身が NULL 値でない)
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
UnsignedInteger64Data::
getInt_NotNull() const
{
	if (m_ullValue > Os::Limits<int>::getMax()) {
		_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);
	}
	return static_cast<int>(m_ullValue);
}

// FUNCTION public
//	Common::UnsignedInteger64Data::getUnsignedInt_NotNull -- 
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
UnsignedInteger64Data::
getUnsignedInt_NotNull() const
{
	if (m_ullValue > Os::Limits<unsigned int>::getMax()) {
		_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);
	}
	return static_cast<unsigned int>(m_ullValue);
}

//	FUNCTION public
//	Common::UnsignedInteger64Data::getValue -- 値を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		データ
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

ModUInt64
UnsignedInteger64Data::getValue() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return m_ullValue;
}

//	FUNCTION public
//	Common::UnsignedInteger64Data::setValue -- 値を設定する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt64 ullValue_
//		データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
UnsignedInteger64Data::setValue(ModUInt64 ullValue_)
{
	m_ullValue = ullValue_;

	// NULL 値でなくする

	setNull(false);
}

//	FUNCTION private
//	Common::UnsignedInteger64Data::compareTo_NoCast -- 大小比較を行う
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
UnsignedInteger64Data::compareTo_NoCast(const Data& r) const
{
	// castの結果Nullになることがある
	if (isNull())
		return NullData::getInstance()->compareTo(&r);

	; _TRMEISTER_ASSERT(r.getType() == DataType::UnsignedInteger64);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	const UnsignedInteger64Data& data =
		_SYDNEY_DYNAMIC_CAST(const UnsignedInteger64Data&, r);
	
	return (getValue() == data.getValue()) ? 0 :
		(getValue() < data.getValue()) ? -1 : 1;
}

//	FUNCTION private
//	Common::UnsignedInteger64Data::assign_NoCast -- 四則演算を行う
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
UnsignedInteger64Data::assign_NoCast(const Data& r)
{
	// castの結果Nullになることがある
	if (r.isNull()) {
		setNull();
		return false;
	}

	; _TRMEISTER_ASSERT(r.getType() == getType());
	; _TRMEISTER_ASSERT(!r.isNull());

	const UnsignedInteger64Data& data =
		_SYDNEY_DYNAMIC_CAST(const UnsignedInteger64Data&, r);
	setValue(data.getValue());
	return true;
}

//	FUNCTION private
//	Common::UnsignedInteger64Data::operateWith_NoCast -- 四則演算を行う
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
UnsignedInteger64Data::operateWith_NoCast(
	DataOperation::Type op, const Data& r)
{
	; _TRMEISTER_ASSERT(r.getType() == DataType::UnsignedInteger64);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	const UnsignedInteger64Data& data =
		_SYDNEY_DYNAMIC_CAST(const UnsignedInteger64Data&, r);
	ModUInt64 ullRhs = data.getValue();

	switch (op) {
	case DataOperation::Addition:
		if (!(ullRhs > _ullMax-m_ullValue)) {
			m_ullValue += ullRhs;
			return true;
		}
		break;

	case DataOperation::Subtraction:
		if (!(m_ullValue < ullRhs)) {
			m_ullValue -= ullRhs;
			return true;
		}
		break;

	case DataOperation::Multiplication:
		// 精密な判定式とはいえないが…。
		if (!(m_ullValue> 1 && ullRhs> 1 && m_ullValue > _ullMax/ullRhs)) {
			m_ullValue *= ullRhs;
			return true;
		}
		break;

	case DataOperation::Division:
		if (ullRhs == 0) {
			_TRMEISTER_THROW0(Exception::DivisionByZero);
		}
		m_ullValue /= ullRhs;
		return true;
	case DataOperation::Modulus:
		{
			if (ullRhs == 0) {
				_TRMEISTER_THROW0(Exception::DivisionByZero);
			}
			if (ullRhs == 1 || ullRhs == -1) {
				m_ullValue = 0;
			} else {
				m_ullValue %= ullRhs;
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
//	Common::UnsignedInteger64Data::operateWith_NotNull -- 単項演算を行う。
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
UnsignedInteger64Data::operateWith_NotNull(
	DataOperation::Type eOperation_, Data::Pointer& pResult_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (eOperation_) {
	case DataOperation::Negation:
		if (m_ullValue == 0) {
			pResult_ = copy();
			return true;
		} else if (m_ullValue <= static_cast<ModUInt64>(Os::Limits<ModInt64>::getMax()) + 1) {
			// Os::Limits<int64>::getMax() = ABS(getMin()) - 1 であることを前提としている
			; _TRMEISTER_ASSERT(Os::Limits<ModInt64>::getMax() == -(Os::Limits<ModInt64>::getMin()+1));

			pResult_ = new Integer64Data(-static_cast<ModInt64>(m_ullValue-1)-1);
			return true;
		}
		break;

	case DataOperation::AbsoluteValue:
		pResult_ = copy();
		return true;

	default:
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	pResult_ = NullData::getInstance();
	return false;
}

//単項演算関数その2
bool
UnsignedInteger64Data::operateWith_NotNull(DataOperation::Type eOperation_)
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (eOperation_) {
	case DataOperation::Increment:
		if (m_ullValue != _ullMax) {
			++m_ullValue;
			return true;
		}
		break;

	case DataOperation::Decrement:
		if (m_ullValue != _ullMin) {
			--m_ullValue;
			return true;
		}
		break;

	default:
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	return false;
}

//	FUNCTION private
//	Common::UnsignedInteger64Data::getClassID_NotNull -- クラスIDを得る
//
//	NOTES
//	シリアライズのためのクラスIDを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		Common::UnsignedInteger64DataクラスのクラスID
//
//	EXCEPTIONS
//	なし

int
UnsignedInteger64Data::getClassID_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return ClassID::UnsignedInteger64DataClass;
}

//	FUNCTION private
//	Common::UnsignedInteger64Data::print_NotNull -- 値を表示する
//
//	NOTES
//	値を表示する
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
UnsignedInteger64Data::print_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	cout << "UnsignedInteger64: "
		 << ModUnicodeString(getString()).getString(Common::LiteralCode)
		 << endl;
}

//	FUNCTION private
//	Common::UnsignedInteger64Data::isAbleToDump_NotNull --
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
UnsignedInteger64Data::isAbleToDump_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return true;
}

//	FUNCTION private
//	Common::UnsignedInteger64Data::isFixedSize_NotNull --
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
UnsignedInteger64Data::isFixedSize_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return true;
}

//	FUNCTION private
//	Common::UnsignedInteger64Data::getDumpSize_NotNull --
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
UnsignedInteger64Data::getDumpSize_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return _iDumpSize;
}

//	FUNCTION public
//	Common::UnsignedInteger64Data::setDumpedValue --
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
UnsignedInteger64Data::setDumpedValue(const char* pszValue_)
{
	Os::Memory::copy(&m_ullValue, pszValue_, _iDumpSize);

	setNull(false);

	return _iDumpSize;
}

//virtual
ModSize
UnsignedInteger64Data::setDumpedValue(const char* pszValue_, ModSize uSize_)
{
	if (uSize_ != _iDumpSize)
		_TRMEISTER_THROW0(Exception::BadArgument);

	return setDumpedValue(pszValue_);
}

//virtual
ModSize
UnsignedInteger64Data::setDumpedValue(ModSerialIO& cSerialIO_)
{
	cSerialIO_.readSerial(&m_ullValue, _iDumpSize, ModSerialIO::dataTypeInt64);

	setNull(false);

	return _iDumpSize;
}

//virtual
ModSize
UnsignedInteger64Data::setDumpedValue(ModSerialIO& cSerialIO_, ModSize uSize_)
{
	if (uSize_ != _iDumpSize)
		_TRMEISTER_THROW0(Exception::BadArgument);

	return setDumpedValue(cSerialIO_);
}

//virtual
ModSize
UnsignedInteger64Data::dumpValue_NotNull(ModSerialIO& cSerialIO_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	cSerialIO_.writeSerial(&m_ullValue, _iDumpSize, ModSerialIO::dataTypeInt64);
	return getDumpSize();
}

//	FUNCTION private
//	Common::UnsignedInteger64Data::dumpValue_NotNull --
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
UnsignedInteger64Data::dumpValue_NotNull(char* pszResult_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	Os::Memory::copy(pszResult_, &m_ullValue, _iDumpSize);
	return getDumpSize();
}

//
//	Copyright (c) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
