// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UnsignedIntegerData.cpp -- 32 ビット長非負整数型データ関連の関数定義
// 
// Copyright (c) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2010, 2012, 2023 Ricoh Company, Ltd.
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
	const ModSize _iDumpSize = sizeof(unsigned int);

	// 最大値・最小値の定義
	const unsigned int uiMax = Os::Limits<unsigned int>::getMax();
	const unsigned int uiMin = Os::Limits<unsigned int>::getMin();
}	

//static
ModSize
UnsignedIntegerData::getArchiveSize()
{
	return _iDumpSize;
}

//	FUNCTION public
//	Common::UnsignedIntegerData::UnsignedIntegerData -- コンストラクタ(1)
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

UnsignedIntegerData::UnsignedIntegerData()
	: ScalarData(DataType::UnsignedInteger),
	  m_uiValue(0)
{}

//	FUNCTION public
//	Common::UnsignedIntegerData::UnsignedIntegerData -- コンストラクタ(2)
//
//	NOTES
//
//	ARGUMENTS
//	unsigned int uiValue_
//		データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

UnsignedIntegerData::UnsignedIntegerData(unsigned int uiValue_)
	: ScalarData(DataType::UnsignedInteger),
	  m_uiValue(uiValue_)
{}

//	FUNCTION public
//	Common::UnsignedIntegerData::~UnsignedIntegerData -- デストラクタ
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

UnsignedIntegerData::~UnsignedIntegerData()
{}

//	FUNCTION private
//	Common::UnsignedIntegerData::serialize_NotNull -- シリアル化
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
UnsignedIntegerData::serialize_NotNull(ModArchive& cArchiver_)
{
	; _TRMEISTER_ASSERT(!isNull());

	ScalarData::serialize_NotNull(cArchiver_);
	if (cArchiver_.isStore())
		//書出し
		cArchiver_ << m_uiValue;
	else
		//読出し
		cArchiver_ >> m_uiValue;
}

//	FUNCTION private
//	Common::UnsignedIntegerData::copy_NotNull -- コピーする
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
UnsignedIntegerData::copy_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return new UnsignedIntegerData(*this);
}

//	FUNCTION private
//	Common::UnsignedIntegerData::cast_NotNull -- 指定の型にキャストする
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
UnsignedIntegerData::cast_NotNull(DataType::Type eType_, bool bForAssign_ /* = false */) const
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (eType_) {
	case DataType::Integer:
		if (m_uiValue <= static_cast<unsigned int>(Os::Limits<int>::getMax()))
			return new IntegerData(static_cast<int>(m_uiValue));
		break;
	case DataType::UnsignedInteger:
		return copy();
	case DataType::Integer64:
		return new Integer64Data(static_cast<ModInt64>(m_uiValue));
	case DataType::UnsignedInteger64:
		return new UnsignedInteger64Data(static_cast<ModUInt64>(m_uiValue));
	case DataType::String:
		return new StringData(getString());
	case DataType::Float:
		return new FloatData(static_cast<float>(m_uiValue));
	case DataType::Double:
		return new DoubleData(static_cast<double>(m_uiValue));
	case DataType::Decimal:
		{
			const DecimalData* pDecimal = _SYDNEY_DYNAMIC_CAST(const DecimalData*, getTargetData());
			if (0 != pDecimal)
			{
				ModAutoPointer<DecimalData> pData =
					new DecimalData(pDecimal->getPrecision(), pDecimal->getScale());
				if (pData->castFromUnsignedInteger(*this, bForAssign_))
					return pData.release();
			}
			break;
		}
	case DataType::Null:
		return NullData::getInstance();
	default:
		_TRMEISTER_THROW0(Exception::ClassCast);
	}

	if (bForAssign_)
		// 代入のためのキャストならエラー
		_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);
	else
		// 比較のためならNull
		return NullData::getInstance();
}
Data::Pointer
UnsignedIntegerData::castToDecimal(bool bForAssign_) const
{
	DecimalData cDecimal(0,0);
	if (cDecimal.castFromUnsignedInteger(*this, bForAssign_))
		return new DecimalData(cDecimal);
	else
		return NullData::getInstance();
}

//	FUNCTION private
//	Common::UnsignedIntegerData::getString_NotNull -- 文字列でデータを得る
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
UnsignedIntegerData::getString_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	ModUnicodeOstrStream ostr;
	ostr << m_uiValue;
	return ModUnicodeString(ostr.getString());
}

// FUNCTION private
//	Common::UnsignedIntegerData::getInt_NotNull -- 数値で得る(自分自身が NULL 値でない)
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
UnsignedIntegerData::
getInt_NotNull() const
{
	if (m_uiValue > Os::Limits<int>::getMax()) {
		_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);
	}
	return static_cast<int>(m_uiValue);
}

// FUNCTION public
//	Common::UnsignedIntegerData::getUnsignedInt_NotNull -- 
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
UnsignedIntegerData::
getUnsignedInt_NotNull() const
{
	return m_uiValue;
}

//	FUNCTION public
//	Common::UnsignedIntegerData::getValue -- データを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	unsigned int
//		データ
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

unsigned int
UnsignedIntegerData::getValue() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return getValue_NotNull();
}

// FUNCTION public
//	Common::UnsignedIntegerData::hashCode -- ハッシュコードを取り出す
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
UnsignedIntegerData::
hashCode() const
{
	return isNull() ? 0 : m_uiValue;
}

// FUNCTION public
//	Common::UnsignedIntegerData::getValue_NotNull -- Nullでないことが保証されるときのgetValue
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

unsigned int
UnsignedIntegerData::
getValue_NotNull() const
{
	return m_uiValue;
}

//	FUNCTION public
//	Common::UnsignedIntegerData::setValue -- データを設定する
//
//	NOTES
//
//	ARGUMENTS
//	unsigned int uiValue_
//		データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
UnsignedIntegerData::setValue(unsigned int uiValue_)
{
	m_uiValue = uiValue_;

	// NULL 値でなくする

	setNull(false);
}

//	FUNCTION private
//	Common::UnsignedIntegerData::compareTo_NoCast -- 大小比較を行う
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
UnsignedIntegerData::compareTo_NoCast(const Data& r) const
{
	// castの結果Nullになることがある
	if (isNull())
		return NullData::getInstance()->compareTo(&r);

	; _TRMEISTER_ASSERT(r.getType() == DataType::UnsignedInteger);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	const UnsignedIntegerData& data =
		_SYDNEY_DYNAMIC_CAST(const UnsignedIntegerData&, r);

	return (getValue() == data.getValue()) ? 0 :
		(getValue() < data.getValue()) ? -1 : 1;
}

//	FUNCTION private
//	Common::UnsignedIntegerData::assign_NoCast -- 代入を行う
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
UnsignedIntegerData::assign_NoCast(const Data& r)
{
	// castの結果Nullになることがある
	if (r.isNull()) {
		setNull();
		return false;
	}

	; _TRMEISTER_ASSERT(r.getType() == getType());
	; _TRMEISTER_ASSERT(!r.isNull());

	const UnsignedIntegerData& data =
		_SYDNEY_DYNAMIC_CAST(const UnsignedIntegerData&, r);
	setValue(data.getValue());
	return true;
}

//	FUNCTION private
//	Common::UnsignedIntegerData::operateWith_NoCast -- 四則演算を行う
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
UnsignedIntegerData::operateWith_NoCast(
	DataOperation::Type op, const Data& r)
{
	; _TRMEISTER_ASSERT(r.getType() == DataType::UnsignedInteger);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	const UnsignedIntegerData& data =
		_SYDNEY_DYNAMIC_CAST(const UnsignedIntegerData&, r);
	unsigned int uiRhs = data.getValue();

	switch (op)	{
	case DataOperation::Addition:
		if (!(uiRhs > uiMax-m_uiValue)) {
			m_uiValue += uiRhs;
			return true;
		}
		break;

	case DataOperation::Subtraction:
		if (!(m_uiValue < uiRhs)) {
			m_uiValue -= uiRhs;
			return true;
		}
		break;

	case DataOperation::Multiplication:
		// 精密な判定式とはいえないが…。
		if (!(m_uiValue> 1 && uiRhs> 1 && m_uiValue > uiMax/uiRhs)) {
			m_uiValue *= uiRhs;
			return true;
		}
		break;

	case DataOperation::Division:
		if (uiRhs == 0) {
			_TRMEISTER_THROW0(Exception::DivisionByZero);
		}
		m_uiValue /= uiRhs;
		return true;
	case DataOperation::Modulus:
		{
			if (uiRhs == 0) {
				_TRMEISTER_THROW0(Exception::DivisionByZero);
			}
			if (uiRhs == 1 || uiRhs == -1) {
				m_uiValue = 0;
			} else {
				m_uiValue %= uiRhs;
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
//	Common::UnsignedIntegerData::operateWith_NotNull -- 単項演算を行う
//
//	NOTE
//		単項演算を行う
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
//		なし

bool
UnsignedIntegerData::operateWith_NotNull(
	DataOperation::Type eOperation_, Data::Pointer& pResult_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (eOperation_) {
	case DataOperation::Negation:
		if (m_uiValue == 0) {
			pResult_ = copy();
			return true;
		} else if (m_uiValue <= static_cast<unsigned int>(Os::Limits<int>::getMax()) + 1) {
			// Os::Limits<int>::getMax() = ABS(getMin()) - 1 であることを前提としている
			; _TRMEISTER_ASSERT(Os::Limits<int>::getMax() == -(Os::Limits<int>::getMin()+1));

			pResult_ = new IntegerData(-static_cast<int>(m_uiValue-1)-1);
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
UnsignedIntegerData::
operateWith_NotNull(DataOperation::Type eOperation_)
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (eOperation_) {
	case DataOperation::Increment:
		if (m_uiValue != uiMax) {
			++m_uiValue;
			return true;
		}
		break;

	case DataOperation::Decrement:
		if (m_uiValue != uiMin) {
			--m_uiValue;
			return true;
		}
		break;

	default:
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	return false;
}

//	FUNCTION private
//	Common::UnsignedIntegerData::getClassID_NotNull -- クラスIDを得る
//
//	NOTES
//	シリアライズのためのクラスIDを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		Common::UnsignedIntegerData クラスのクラスID
//
//	EXCEPTIONS
//	なし

int
UnsignedIntegerData::getClassID_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return ClassID::UnsignedIntegerDataClass;
}

//	FUNCTION private
//	Common::UnsignedIntegerData::print_NotNull -- 値を表示する
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
UnsignedIntegerData::print_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	cout << "unsigned integer: " << m_uiValue << endl;
}

//	FUNCTION private
//	Common::UnsignedIntegerData::isAbleToDump_NotNull --
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
UnsignedIntegerData::isAbleToDump_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return true;
}

//	FUNCTION private
//	Common::UnsignedIntegerData::isFixedSize_NotNull --
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
UnsignedIntegerData::isFixedSize_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return true;
}

//	FUNCTION private
//	Common::UnsignedIntegerData::getDumpSize_NotNull --
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
UnsignedIntegerData::getDumpSize_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return _iDumpSize;
}

//	FUNCTION public
//	Common::UnsignedIntegerData::setDumpedValue --
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
UnsignedIntegerData::setDumpedValue(const char* pszValue_)
{
	Os::Memory::copy(&m_uiValue, pszValue_, _iDumpSize);

	setNull(false);

	return _iDumpSize;
}

//virtual
ModSize
UnsignedIntegerData::setDumpedValue(const char* pszValue_, ModSize uSize_)
{
	if (uSize_ != _iDumpSize)
		_TRMEISTER_THROW0(Exception::BadArgument);

	return setDumpedValue(pszValue_);
}

//virtual
ModSize
UnsignedIntegerData::setDumpedValue(ModSerialIO& cSerialIO_)
{
	cSerialIO_.readSerial(&m_uiValue, _iDumpSize, ModSerialIO::dataTypeInteger);

	setNull(false);

	return _iDumpSize;
}

//virtual
ModSize
UnsignedIntegerData::setDumpedValue(ModSerialIO& cSerialIO_, ModSize uSize_)
{
	if (uSize_ != _iDumpSize)
		_TRMEISTER_THROW0(Exception::BadArgument);

	return setDumpedValue(cSerialIO_);
}

//virtual
ModSize
UnsignedIntegerData::dumpValue_NotNull(ModSerialIO& cSerialIO_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	cSerialIO_.writeSerial(&m_uiValue, _iDumpSize, ModSerialIO::dataTypeInteger);
	return getDumpSize();
}

//	FUNCTION private
//	Common::UnsignedIntegerData::dumpValue_NotNull --
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
UnsignedIntegerData::dumpValue_NotNull(char* pszResult_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	Os::Memory::copy(pszResult_, &m_uiValue, _iDumpSize);
	return getDumpSize();
}

//
//	Copyright (c) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2010, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
