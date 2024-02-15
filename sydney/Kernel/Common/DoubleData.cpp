// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DoubleData.cpp -- 倍精度浮動少数点数型データ関連の関数定義
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

#include "ModHasher.h"
#include "ModUnicodeString.h"
#include "ModUnicodeOstrStream.h"

#include <iostream>
#include <stdio.h>
using namespace std;

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace {

	// ダンプサイズ
	const ModSize _iDumpSize = sizeof(double);

	// maxやminの意味が整数型とは違うので注意
	const double dMax = Os::Limits<double>::getMax();
	const double dMin = Os::Limits<double>::getMin();
	const double dEps = Os::Limits<double>::getEpsilon();
}

//static
ModSize
DoubleData::
getArchiveSize()
{
	return _iDumpSize;
}

// FUNCTION public
//	Common::DoubleData::hashCode -- ハッシュコードを取り出す
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
DoubleData::
hashCode() const
{
	if (isNull()) return 0;

	return ModUnicodeStringHasher()(getString());
}

//
//	FUNCTION public
//	Common::DoubleData::DoubleData -- コンストラクタ(1)
//
//	NOTES
//	コンストラクタ
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
DoubleData::
DoubleData()
: ScalarData(DataType::Double), m_dValue(0)
{
}

//
//	FUNCTION public
//	Common::DoubleData::DoubleData -- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	double dValue_
//		データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
DoubleData::
DoubleData(double dValue_)
: ScalarData(DataType::Double), m_dValue(dValue_)
{
}

//
//	FUNCTION public
//	Common::DoubleData::~DoubleData -- デストラクタ
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
DoubleData::
~DoubleData()
{
}

//
//	FUNCTION private
//	Common::DoubleData::serialize_NotNull -- シリアル化
//
//	NOTES
//	シリアル化
//
//	ARGUMENTS
//	ModArchive& cArchiver_
//		アーカイバー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DoubleData::
serialize_NotNull(ModArchive& cArchiver_)
{
	; _TRMEISTER_ASSERT(!isNull());

	ScalarData::serialize_NotNull(cArchiver_);
	if (cArchiver_.isStore())
	{
		//書き出しWriting
		cArchiver_ << m_dValue;
	}
	else
	{
		//読み出しReading
		cArchiver_ >> m_dValue;
	}
}

//
//	FUNCTION private
//	Common::DoubleData::copy_NotNull -- コピーする
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
//	その他
//		下位の例外はそのまま再送
//
Data::Pointer
DoubleData::
copy_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return new DoubleData(*this);
}

//
//	FUNCTION private
//	Common::DoubleData::cast_NotNull -- キャストする
//
//	NOTES
//	指定の型にキャストする。It is Cast in a specified type. 
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
//	その他
//		下位の例外はそのまま再送
//
Data::Pointer
DoubleData::
cast_NotNull(DataType::Type eType_, bool bForAssign_ /* = false */) const
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (eType_) {
	case DataType::Integer:
	// 自分より小さい型になるときにはNullDataを返すこととする
//	 When becoming a type that is smaller than I, it is assumed that NullData is returned. 
	{
		if (m_dValue <= Os::Limits<int>::getMax() 
			&& m_dValue >= Os::Limits<int>::getMin())
			return new IntegerData(static_cast<int>(m_dValue));
		break;
	}
	case DataType::UnsignedInteger:
	{
		if (m_dValue <= Os::Limits<unsigned int>::getMax() 
			&& m_dValue >= Os::Limits<unsigned int>::getMin())
			return new UnsignedIntegerData(static_cast<unsigned int>(m_dValue));
		break;
	}
	case DataType::Integer64:
	{
		if (m_dValue <= Os::Limits<ModInt64>::getMax() 
			&& m_dValue >= Os::Limits<ModInt64>::getMin())
			return new Integer64Data(static_cast<ModInt64>(m_dValue));
		break;
	}
	case DataType::UnsignedInteger64:
	{
		// UInt64をdoubleにキャストできないのでsignedのほうを使う
		// signedの最大値はunsignedの最大値の半分である
		if (m_dValue / 2 <= Os::Limits<ModInt64>::getMax()
			&& m_dValue >= 0)
			return new UnsignedInteger64Data(static_cast<ModUInt64>(m_dValue));
		break;
	}
	case DataType::String:
		return new StringData(getString());
		break;
	case DataType::Decimal:
	{
		const DecimalData* pDecimal = _SYDNEY_DYNAMIC_CAST(const DecimalData*, getTargetData());
		if (0 != pDecimal)
		{
			DecimalData ddValue(pDecimal->getPrecision(), pDecimal->getScale());
			if (ddValue.castFromDouble(*this, bForAssign_))
				return new DecimalData(ddValue);
		}
		break;
	}
	case DataType::Float:
	{
		if (m_dValue == 0
			|| (m_dValue <= Os::Limits<float>::getMax() && m_dValue >= Os::Limits<float>::getMin())
			|| (m_dValue >= -(Os::Limits<float>::getMax()) && m_dValue <= -(Os::Limits<float>::getMin())))
			return new FloatData(static_cast<float>(m_dValue));
		break;
	}
	case DataType::Double:
		return copy();
	case DataType::Null:
		return NullData::getInstance();
	default:
		_TRMEISTER_THROW0(Exception::ClassCast);
	}

	if (bForAssign_)
		// 代入のためのキャストならエラーにする
		_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);
	else
		// 比較のためのキャストならNULLにする
		return NullData::getInstance();
}

Data::Pointer
DoubleData::castToDecimal(bool bForAssign_) const
{
	DecimalData cDecimal(0,0);
	if (cDecimal.castFromDouble(*this, bForAssign_))
		return new DecimalData(cDecimal);
	else
		return NullData::getInstance();

}
//
//	FUNCTION public
//	Common::DoubleData::getValue -- 値を得る
//
//	NOTES
//	値を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	double
//		データ
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

double
DoubleData::
getValue() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return m_dValue;
}

//
//	FUNCTION public
//	Common::DoubleData::setValue -- 値を設定する
//
//	NOTES
//	値を設定する
//
//	ARGUMENTS
//	double dValue_
//		データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
DoubleData::
setValue(double dValue_)
{
	m_dValue = dValue_;

	// NULL 値でなくする

	setNull(false);
}

//
//	FUNCTION private
//	Common::DoubleData::getString_NotNull -- 文字列で取り出すIt takes it out by the character string. 
//
//	NOTES
//	文字列で取り出す。
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
//
ModUnicodeString
DoubleData::
getString_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	ModUnicodeOstrStream ostr;

	// 仮数部16桁ピリオド1フラグ1指数部5 = 23      Period one flag one exponent part 5=23 of 16 mantissa digits
	const int iBufferSize = 24;

	char buffer[iBufferSize];
	::sprintf(buffer, "%.14E", m_dValue);

	char* p = buffer;
	char* zero = 0;
	char* delimiter = 0;
	int mantissa = 1;
	while (*p) {
		switch (*p) {
		case '.':
			{
				if (zero) ostr << *zero;
				zero = 0;
				delimiter = p;
				break;
			}
		case '0':
			{
				if (mantissa) {
					if (!zero) zero = p;
				} else {
					if (!delimiter) ostr << *p;
					else if (!zero) zero = p;
				}
				break;
			}
		case 'E':
			{
				mantissa = 0;
				delimiter = p;
				ostr << *p;
				break;
			}
		case '+':
			{
				break;
			}
		case '-':
			{
				if (!mantissa) delimiter = p;
				ostr << *p;
				break;
			}
		default:
			{
				if (mantissa) {
					if (delimiter) ostr << *delimiter;
					if (zero) {
						while (zero < p) ostr << *zero++;
						zero = 0;
					}
				}
				delimiter = 0;
				zero = 0;
				ostr << *p;
				break;
			}
		}
		++p;
	}
	if (zero) ostr << *zero;

	return ModUnicodeString(ostr.getString());
}

//	FUNCTION private
//	Common::DoubleData::compareTo_NoCast -- 大小比較を行う
//
//	NOTES
//		比較するデータは自分自身と同じ型である必要があるShould the type of the compared data is equal to oneself. 
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
DoubleData::compareTo_NoCast(const Data& r) const
{
#ifdef OBSOLETE // castの結果Nullになることがある場合生かす
	if (isNull())
		return NullData::getInstance()->compareTo(r);
#endif

	; _TRMEISTER_ASSERT(r.getType() == DataType::Double);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	const DoubleData& data = _SYDNEY_DYNAMIC_CAST(const DoubleData&, r);

	return (getValue() == data.getValue()) ? 0 :
		(getValue() < data.getValue()) ? -1 : 1;
}

//	FUNCTION private
//	Common::DoubleData::assign_NoCast -- 代入を行う
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
DoubleData::assign_NoCast(const Data& r)
{
#ifdef OBSOLETE // castの結果Nullになることがある場合生かす
	if (r.isNull()) {
		setNull();
		return false;
	}
#endif

	; _TRMEISTER_ASSERT(r.getType() == getType());
	; _TRMEISTER_ASSERT(!r.isNull());

	const DoubleData& data = _SYDNEY_DYNAMIC_CAST(const DoubleData&, r);
	setValue(data.getValue());
	return true;
}

//	FUNCTION private
//	Common::DoubleData::operateWith_NoCast -- 四則演算を行うThe arithmetic operation is done. 
//
//	NOTE
//
//	ARGUMENTS
//		DataOperation::Type	op
//			行う演算の種類Kind of done operation
//		Common::Data&	r
//			右辺に与えられたデータData given right
//			
//	RETURN
//		true
//			演算結果は NULL でないThe operation result is not NULL. 
//		false
//			演算結果は NULL である
//
//	EXCEPTIONS

bool 
DoubleData::operateWith_NoCast(
	DataOperation::Type op, const Data& r)
{
	; _TRMEISTER_ASSERT(r.getType() == DataType::Double);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	const DoubleData& data = _SYDNEY_DYNAMIC_CAST(const DoubleData&, r);
	
	double dRhs = data.getValue();

	switch (op)	{
	case DataOperation::Addition:
		//足し算
		{
			if ((m_dValue>0 && dRhs>0 && dRhs >  dMax-m_dValue)
			 || (m_dValue<0 && dRhs<0 && dRhs < -dMax-m_dValue))
			{				
				setNull();
				return false;
			}
			m_dValue += dRhs;
			return true;
		}
		break;
	case DataOperation::Subtraction:
		// 引き算
		{
			if ((m_dValue>0 && dRhs<0 && m_dValue >  dMax+dRhs)
			 || (m_dValue<0 && dRhs>0 && m_dValue < -dMax+dRhs))
			{				
				setNull();
				return false;
			}
			m_dValue -= dRhs;
			return true;
		}
		break;
	case DataOperation::Multiplication:
		//掛け算
		{
			if((m_dValue >=  1 
					&& ((dRhs >=  1 && m_dValue >  dMax/dRhs)
					 || (dRhs <= -1 && m_dValue > -dMax/dRhs)))
			|| (m_dValue <= -1
					&& ((dRhs >=  1 && m_dValue < -dMax/dRhs)
					 || (dRhs <= -1 && m_dValue <  dMax/dRhs)))
			|| (m_dValue >   0 && m_dValue < 1
					&& ((dRhs >  0 && dRhs < 1 && m_dValue <  dMin/dRhs)
					 || (dRhs > -1 && dRhs < 0 && m_dValue < -dMin/dRhs)))
			|| (m_dValue >  -1 && m_dValue < 0
					&& ((dRhs >  0 && dRhs < 1 && m_dValue > -dMin/dRhs)
					 || (dRhs > -1 && dRhs < 0 && m_dValue >  dMin/dRhs))))
			{				
				setNull();
				return false;
			}
			m_dValue *= dRhs;
			return true;
		}
		break;
	case DataOperation::Division:
		//割り算
		{
			if (dRhs == 0) {
				_TRMEISTER_THROW0(Exception::DivisionByZero);
			}
			if ((m_dValue >=  1 
					&& ((dRhs >   0 && dRhs <= 1 && m_dValue >  dMax*dRhs)
					 || (dRhs >= -1 && dRhs <  0 && m_dValue > -dMax*dRhs)))
			|| (m_dValue <= -1
					&& ((dRhs >   0 && dRhs <= 1 && m_dValue < -dMax*dRhs)
					 || (dRhs >= -1 && dRhs <  0 && m_dValue <  dMax*dRhs)))
			|| (m_dValue >   0 && m_dValue < 1
					&& ((dRhs >  1 && m_dValue <  dMin*dRhs)
					 || (dRhs < -1 && m_dValue < -dMin*dRhs)))
			|| (m_dValue >  -1 && m_dValue < 0
					&& ((dRhs >  1 && m_dValue > -dMin*dRhs)
					 || (dRhs < -1 && m_dValue >  dMin*dRhs))))
			{				
				setNull();
				return false;
			}
			m_dValue /= dRhs;
			return true;
		}
		break;
	case DataOperation::Modulus:
		{
			// double can't calculate modulus
			break;
		}
	default:
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	setNull();
	return false;
}

//	FUNCTION private
//	Common::DoubleData::operateWith_NotNull -- 単項演算を行う。The prefix operation is done. 
//
//	NOTE
//		単項演算を行う。The prefix operation is done. 
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
DoubleData::
operateWith_NotNull(DataOperation::Type eOperation_,
					Data::Pointer& pResult_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (eOperation_)
	{
	case DataOperation::Negation:
		pResult_ = new DoubleData(-m_dValue);
		return true;
		break;
	case DataOperation::AbsoluteValue:
		if (m_dValue >= 0)
			pResult_ = new DoubleData( m_dValue);
		else 
			pResult_ = new DoubleData(-m_dValue);
		return true;
		break;
	default:
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	pResult_ = NullData::getInstance();
	return false;
}

//
//	FUNCTION private
//	Common::DoubleData::getClassID_NotNull -- クラスIDを得る
//
//	NOTES
//	シリアライズのためのクラスIDを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		Common::DoubleDataクラスのクラスID
//
//	EXCEPTIONS
//	なし
//
int
DoubleData::
getClassID_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return ClassID::DoubleDataClass;
}

//
//	FUNCTION private
//	Common::DoubleData::print_NotNull -- 値を表示する
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
//
void
DoubleData::
print_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	cout << "double: " << m_dValue << endl;
}

//	FUNCTION private
//	Common::DoubleData::isAbleToDump_NotNull --
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
DoubleData::
isAbleToDump_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return true;
}

//	FUNCTION public
//	Common::DoubleData::isFixedSize_NotNull --
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
DoubleData::
isFixedSize_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return true;
}

//	FUNCTION private
//	Common::DoubleData::getDumpSize_NotNull --
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
DoubleData::
getDumpSize_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return _iDumpSize;
}

//	FUNCTION public
//	Common::DoubleData::setDumpedValue --
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
DoubleData::
setDumpedValue(const char* pszValue_)
{
	Os::Memory::copy(&m_dValue, pszValue_, _iDumpSize);

	setNull(false);

	return _iDumpSize;
}

//virtual
ModSize
DoubleData::
setDumpedValue(const char* pszValue_, ModSize uSize_)
{
	if (uSize_ != _iDumpSize) {
		_TRMEISTER_THROW0(Exception::BadArgument);
	}
	return setDumpedValue(pszValue_);
}

//virtual
ModSize
DoubleData::
setDumpedValue(ModSerialIO& cSerialIO_)
{
	cSerialIO_.readSerial(&m_dValue, _iDumpSize, ModSerialIO::dataTypeDouble);

	setNull(false);

	return _iDumpSize;
}

//virtual
ModSize
DoubleData::
setDumpedValue(ModSerialIO& cSerialIO_, ModSize uSize_)
{
	if (uSize_ != _iDumpSize) {
		_TRMEISTER_THROW0(Exception::BadArgument);
	}
	return setDumpedValue(cSerialIO_);
}

//virtual
ModSize
DoubleData::
dumpValue_NotNull(ModSerialIO& cSerialIO_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	cSerialIO_.writeSerial(&m_dValue, _iDumpSize, ModSerialIO::dataTypeDouble);
	return _iDumpSize;
}

//	FUNCTION private
//	Common::DoubleData::dumpValue_NotNull --
//		自身の値をdumpするDump does own value. 
//
//	NOTES
//
//	ARGUMENTS
//		char* pszResult_
//			dumpする領域の先頭
//
//	RETURN
//		値に使ったバイト数Number of bytes used for value
//
//	EXCEPTIONS

//virtual
ModSize
DoubleData::
dumpValue_NotNull(char* pszResult_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	Os::Memory::copy(pszResult_, &m_dValue, _iDumpSize);
	return _iDumpSize;
}

//
//	Copyright (c) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
