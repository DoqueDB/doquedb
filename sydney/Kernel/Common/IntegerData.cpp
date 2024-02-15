// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IntegerData.cpp -- 32 ビット長整数型データ関連の関数定義
// 
// Copyright (c) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
#include "Common/DoubleData.h"
#include "Common/FloatData.h"
#include "Common/DecimalData.h"
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
	const ModSize _iDumpSize = sizeof(int);

	// 最大値・最小値の定義
	const int iMax = Os::Limits<int>::getMax();
	const int iMin = Os::Limits<int>::getMin();
}	

//ハッシュコードを取り出す
//virtual
ModSize
IntegerData::
hashCode() const
{
	if (isNull()) return 0;

	return m_iValue;
}

//static
ModSize
IntegerData::getArchiveSize()
{
	return _iDumpSize;
}

//	FUNCTION public
//	Common::IntegerData::IntegerData -- コンストラクタ(1)
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

IntegerData::IntegerData()
	: ScalarData(DataType::Integer),
	  m_iValue(0)
{}

//	FUNCTION public
//	Common::IntegerData::IntegerData -- コンストラクタ(2)
//
//	NOTES
//
//	ARGUMENTS
//	int iValue_
//		データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

IntegerData::IntegerData(int iValue_)
	: ScalarData(DataType::Integer),
	  m_iValue(iValue_)
{}

//	FUNCTION public
//	Common::IntegerData::~IntegerData -- デストラクタ
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
//		なし

IntegerData::~IntegerData()
{}

//	FUNCTION private
//	Common::IntegerData::serialize_NotNull -- シリアル化
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
IntegerData::serialize_NotNull(ModArchive& cArchiver_)
{
	; _TRMEISTER_ASSERT(!isNull());

	ScalarData::serialize_NotNull(cArchiver_);

	if (cArchiver_.isStore())
		//書出し
		cArchiver_ << m_iValue;
	else
		//読出し
		cArchiver_ >> m_iValue;
}

//	FUNCTION private
//	Common::IntegerData::copy_NotNull -- コピーする
//
//	NOTES
//	自分自身のコピーを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::IntegerData::Pointer
//		自分自身のコピー
//
//	EXCEPTIONS

Data::Pointer
IntegerData::copy_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return new IntegerData(*this);
}

//	FUNCTION private
//	Common::IntegerData::cast_NotNull -- 指定の型にキャストする
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
IntegerData::cast_NotNull(DataType::Type eType_, bool bForAssign_ /* = false */) const
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (eType_)	{
	case DataType::Integer:
		return copy();
	case DataType::UnsignedInteger:
		if (m_iValue >= 0)
			return new UnsignedIntegerData(static_cast<unsigned int>(m_iValue));
		break;
	case DataType::String:
		return new StringData(getString());
	case DataType::Integer64:
		return new Integer64Data(static_cast<ModInt64>(m_iValue));
	case DataType::UnsignedInteger64:
		if (m_iValue >= 0)
			return new UnsignedInteger64Data(static_cast<ModUInt64>(m_iValue));
		break;
	case DataType::Float:
		return new FloatData(static_cast<float>(m_iValue));
	case DataType::Double:
		return new DoubleData(static_cast<double>(m_iValue));
	case DataType::Decimal:
		{
			const DecimalData* pDecimal = _SYDNEY_DYNAMIC_CAST(const DecimalData*, getTargetData());
			if (0 != pDecimal)
			{
				ModAutoPointer<DecimalData> pData =
					new DecimalData(pDecimal->getPrecision(), pDecimal->getScale());
				if (pData->castFromInteger(*this, bForAssign_))
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
		// 代入のためのキャストならエラーにする
		_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);
	else
		// 比較のためならNull
		return NullData::getInstance();
}
Data::Pointer
IntegerData::castToDecimal(bool bForAssign_) const
{
	DecimalData cDecimal(0,0);
	if (cDecimal.castFromInteger(*this, bForAssign_))
		return new DecimalData(cDecimal);
	else
		return NullData::getInstance();

}
//	FUNCTION private
//	Common::IntegerData::getString_NotNull -- 文字列でデータを得る
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
IntegerData::getString_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	ModUnicodeOstrStream ostr;
	ostr << m_iValue;
	return ModUnicodeString(ostr.getString());
}

// FUNCTION private
//	Common::IntegerData::getInt_NotNull -- 数値で得る(自分自身が NULL 値でない)
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
IntegerData::
getInt_NotNull() const
{
	return m_iValue;
}

// FUNCTION public
//	Common::IntegerData::getUnsignedInt_NotNull -- 
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
IntegerData::
getUnsignedInt_NotNull() const
{
	if (m_iValue < 0) {
		_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);
	}
	return static_cast<unsigned int>(m_iValue);
}

//	FUNCTION public
//	Common::IntegerData::getValue -- データを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		データ
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

int
IntegerData::getValue() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return m_iValue;
}

//	FUNCTION public
//	Common::IntegerData::setValue -- データを設定する
//
//	NOTES
//
//	ARGUMENTS
//	int iValue_
//		データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
IntegerData::setValue(int iValue_)
{
	m_iValue = iValue_;

	// NULL 値でなくする

	setNull(false);
}

//	FUNCTION private
//	Common::IntegerData::compareTo_NoCast -- 大小比較を行う
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
IntegerData::compareTo_NoCast(const Data& r) const
{
	// castの結果Nullになることがある
	if (isNull())
		return NullData::getInstance()->compareTo(&r);

	; _TRMEISTER_ASSERT(r.getType() == DataType::Integer);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	const IntegerData& data = _SYDNEY_DYNAMIC_CAST(const IntegerData&, r);

	return (getValue() == data.getValue()) ? 0 :
		(getValue() < data.getValue()) ? -1 : 1;
}

//	FUNCTION private
//	Common::IntegerData::assign_NoCast -- 代入を行う
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
IntegerData::assign_NoCast(const Data& r)
{
	// castの結果Nullになることがある
	if (r.isNull()) {
		setNull();
		return false;
	}

	; _TRMEISTER_ASSERT(r.getType() == getType());
	; _TRMEISTER_ASSERT(!r.isNull());

	const IntegerData& data = _SYDNEY_DYNAMIC_CAST(const IntegerData&, r);
	setValue(data.getValue());
	return true;
}

//	FUNCTION private
//	Common::IntegerData::operateWith_NoCast -- 四則演算を行う
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
IntegerData::operateWith_NoCast(
	DataOperation::Type op, const Data& r)
{
	; _TRMEISTER_ASSERT(r.getType() == DataType::Integer);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	const IntegerData& data = _SYDNEY_DYNAMIC_CAST(const IntegerData&, r);
	int iRhs = data.getValue();

	switch (op)	{
	case DataOperation::Addition:
		if (!((m_iValue>0 && iRhs>0 && iRhs > iMax-m_iValue) ||
			  (m_iValue<0 && iRhs<0 && iRhs < iMin-m_iValue))) {
			m_iValue += iRhs;
			return true;
		}
		break;

	case DataOperation::Subtraction:
		if (!((m_iValue>0 && iRhs<0 && m_iValue > iMax+iRhs) ||
			  (m_iValue<0 && iRhs>0 && m_iValue < iMin+iRhs))) {
			m_iValue -= iRhs;
			return true;
		}
		break;

	case DataOperation::Multiplication:
		// 精密な判定式とはいえないが…。
		// -> 割り算が0に近いほうに切り捨てである限り精密
		//    ↑この動作はたいていのコンパイラーではOKだが、
		//      ANSI Cの仕様では実装依存らしいので移植時に注意が必要
		if (!((m_iValue>  1 && iRhs>  1 && m_iValue > iMax/iRhs) ||
			  (m_iValue<=-1 && iRhs<=-1 && m_iValue < iMax/iRhs) ||
			  (m_iValue>  1 && iRhs< -1 && m_iValue > iMin/iRhs) ||
			  (m_iValue< -1 && iRhs>  1 && m_iValue < iMin/iRhs))) {
			m_iValue *= iRhs;
			return true;
		}
		break;

	case DataOperation::Division:
		if (iRhs == 0) {
			_TRMEISTER_THROW0(Exception::DivisionByZero);
		}
		if (!(m_iValue == iMin && iRhs == -1)) {
			m_iValue /= iRhs;
			return true;
		}
		break;
	case DataOperation::Modulus:
		{
			if (iRhs == 0) {
				_TRMEISTER_THROW0(Exception::DivisionByZero);
			}
			if (iRhs == 1 || iRhs == -1) {
				m_iValue = 0;
			} else {
				m_iValue %= iRhs;
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
//	Common::IntegerData::operateWith_NotNull -- 単項演算を行う。
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
IntegerData::operateWith_NotNull(
	DataOperation::Type eOperation_, Data::Pointer& pResult_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (eOperation_) {
	case DataOperation::Negation:
		if (m_iValue != iMin) {
			pResult_ = new IntegerData(-m_iValue);
			return true;
		}
		break;

	case DataOperation::AbsoluteValue:
		if (m_iValue != iMin) {
			pResult_ = new IntegerData((m_iValue >= 0) ? m_iValue : -m_iValue);
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
IntegerData::operateWith_NotNull(DataOperation::Type eOperation_)
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (eOperation_) {
	case DataOperation::Increment:
		if (m_iValue != iMax) {
			++m_iValue;
			return true;
		}
		break;

	case DataOperation::Decrement:
		if (m_iValue != iMin) {
			--m_iValue;
			return true;
		}
		break;

	default:
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	return false;
}

//	FUNCTION private
//	Common::IntegerData::getClassID_NotNull -- クラスIDを得る
//
//	NOTES
//	シリアライズのためのクラスIDを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		Common::IntegerData クラスのクラスID
//
//	EXCEPTIONS
//	なし

int
IntegerData::getClassID_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return ClassID::IntegerDataClass;
}

//	FUNCTION private
//	Common::IntegerData::print_NotNull -- 値を表示する
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
IntegerData::print_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	cout << "integer: " << m_iValue << endl;
}

//	FUNCTION private
//	Common::IntegerData::isAbleToDump_NotNull --
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
IntegerData::isAbleToDump_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return true;
}

//	FUNCTION private
//	Common::IntegerData::isFixedSize_NotNull --
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
IntegerData::isFixedSize_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return true;
}

//	FUNCTION private
//	Common::IntegerData::getDumpSize_NotNull --
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
IntegerData::getDumpSize_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return _iDumpSize;
}

//	FUNCTION public
//	Common::IntegerData::setDumpedValue --
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
IntegerData::setDumpedValue(const char* pszValue_)
{
	Os::Memory::copy(&m_iValue, pszValue_, _iDumpSize);

	setNull(false);

	return _iDumpSize;
}

//virtual
ModSize
IntegerData::setDumpedValue(const char* pszValue_, ModSize uSize_)
{
	if (uSize_ != _iDumpSize)
		_TRMEISTER_THROW0(Exception::BadArgument);

	return setDumpedValue(pszValue_);
}

//virtual
ModSize
IntegerData::setDumpedValue(ModSerialIO& cSerialIO_)
{
	cSerialIO_.readSerial(&m_iValue, _iDumpSize, ModSerialIO::dataTypeInteger);

	setNull(false);

	return _iDumpSize;
}

//virtual
ModSize
IntegerData::setDumpedValue(ModSerialIO& cSerialIO_, ModSize uSize_)
{
	if (uSize_ != _iDumpSize)
		_TRMEISTER_THROW0(Exception::BadArgument);

	return setDumpedValue(cSerialIO_);
}

//virtual
ModSize
IntegerData::dumpValue_NotNull(ModSerialIO& cSerialIO_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	cSerialIO_.writeSerial(&m_iValue, _iDumpSize, ModSerialIO::dataTypeInteger);
	return getDumpSize();
}

//	FUNCTION private
//	Common::IntegerData::dumpValue_NotNull --
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
IntegerData::dumpValue_NotNull(char* pszResult_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	Os::Memory::copy(pszResult_, &m_iValue, _iDumpSize);
	return getDumpSize();
}

//
//	Copyright (c) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
