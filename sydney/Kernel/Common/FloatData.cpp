// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FloatData.cpp -- 単精度浮動少数点数型データ関連の関数定義
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
	const ModSize _iDumpSize = sizeof(float);

	// maxやminの意味が整数型とは違うので注意
	const float fMax = Os::Limits<float>::getMax();
	const float fMin = Os::Limits<float>::getMin();
	const float fEps = Os::Limits<float>::getEpsilon();
}

//static
ModSize
FloatData::
getArchiveSize()
{
	return _iDumpSize;
}

// FUNCTION public
//	Common::FloatData::hashCode -- ハッシュコードを取り出す
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
FloatData::
hashCode() const
{
	if (isNull()) return 0;

	return ModUnicodeStringHasher()(getString());
}

//
//	FUNCTION public
//	Common::FloatData::FloatData -- コンストラクタ(1)
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
FloatData::
FloatData()
: ScalarData(DataType::Float), m_fValue(0)
{
}

//
//	FUNCTION public
//	Common::FloatData::FloatData -- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	float fValue_
//		データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
FloatData::
FloatData(float fValue_)
: ScalarData(DataType::Float), m_fValue(fValue_)
{
}

//
//	FUNCTION public
//	Common::FloatData::~FloatData -- デストラクタ
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
FloatData::
~FloatData()
{
}

//
//	FUNCTION private
//	Common::FloatData::serialize_NotNull -- シリアル化
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
FloatData::
serialize_NotNull(ModArchive& cArchiver_)
{
	; _TRMEISTER_ASSERT(!isNull());

	ScalarData::serialize_NotNull(cArchiver_);
	if (cArchiver_.isStore())
	{
		//書き出し
		cArchiver_ << m_fValue;
	}
	else
	{
		//読み出し
		cArchiver_ >> m_fValue;
	}
}

//
//	FUNCTION private
//	Common::FloatData::copy_NotNull -- コピーする
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
//	なし
//
Data::Pointer
FloatData::
copy_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return new FloatData(*this);
}

//
//	FUNCTION private
//	Common::FloatData::cast_NotNull -- キャストする
//
//	NOTES
//	指定の型にキャストする。
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
FloatData::
cast_NotNull(DataType::Type eType_, bool bForAssign_ /* = false */) const
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (eType_) {
	case DataType::Integer:
	// 自分より小さい型になるときにはNullDataを返すこととする
	{
		if (m_fValue <= Os::Limits<int>::getMax() 
			&& m_fValue >= Os::Limits<int>::getMin())
			return new IntegerData(static_cast<int>(m_fValue));
		break;
	}
	case DataType::UnsignedInteger:
	{
		if (m_fValue <= Os::Limits<unsigned int>::getMax() 
			&& m_fValue >= Os::Limits<unsigned int>::getMin())
			return new UnsignedIntegerData(static_cast<unsigned int>(m_fValue));
		break;
	}
	case DataType::Integer64:
	{
		if (m_fValue <= Os::Limits<ModInt64>::getMax() 
			&& m_fValue >= Os::Limits<ModInt64>::getMin())
			return new Integer64Data(static_cast<ModInt64>(m_fValue));
		break;
	}
	case DataType::UnsignedInteger64:
	{
		// UInt64をdoubleにキャストできないのでsignedのほうを使う
		// signedの最大値はunsignedの最大値の半分である
		if (m_fValue / 2 <= Os::Limits<ModInt64>::getMax()
			&& m_fValue >= 0)
			return new UnsignedInteger64Data(static_cast<ModUInt64>(m_fValue));
		break;
	}
	case DataType::String:
		return new StringData(getString());
	case DataType::Float:
		return copy();
	case DataType::Double:
		return new DoubleData(static_cast<double>(m_fValue));
	case DataType::Decimal:
	{
		const DecimalData* pDecimal = _SYDNEY_DYNAMIC_CAST(const DecimalData*, getTargetData());
		if (0 != pDecimal)
		{
			DecimalData ddValue(pDecimal->getPrecision(), pDecimal->getScale());
			if (ddValue.castFromDouble(DoubleData(static_cast<double>(m_fValue)), bForAssign_))
				return new DecimalData(ddValue);
		}
		break;
	}
	case DataType::Null:
		return NullData::getInstance();
	default:
		_TRMEISTER_THROW0(Exception::ClassCast);
	}

	if (bForAssign_)
		// 代入のためのキャストならエラーにする
		_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);
	else
		// 比較のためならNullにする
		return NullData::getInstance();
}

//
//	FUNCTION public
//	Common::FloatData::getValue -- 値を得る
//
//	NOTES
//	値を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	float
//		データ
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

float
FloatData::
getValue() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return m_fValue;
}

//
//	FUNCTION public
//	Common::FloatData::setValue -- 値を設定する
//
//	NOTES
//	値を設定する
//
//	ARGUMENTS
//	float fValue_
//		データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FloatData::
setValue(float fValue_)
{
	m_fValue = fValue_;

	// NULL 値でなくする

	setNull(false);
}

//
//	FUNCTION private
//	Common::FloatData::getString_NotNull -- 文字列で取り出す
//
//	NOTES
//	文字列で取り出す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		文字列クラス
//
//	EXCEPTIONS
//	なし
//
ModUnicodeString
FloatData::
getString_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	ModUnicodeOstrStream ostr;

	// 仮数部7桁一の位1桁ピリオド1フラグ1指数部5 = 15
	const int iBufferSize = 16;

	char buffer[iBufferSize];
	::sprintf(buffer, "%.7E", m_fValue);

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
//	Common::FloatData::compareTo_NoCast -- 大小比較を行う
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
FloatData::compareTo_NoCast(const Data& r) const
{
	if (isNull())
		return NullData::getInstance()->compareTo(&r);

	; _TRMEISTER_ASSERT(r.getType() == DataType::Float);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	const FloatData& data = _SYDNEY_DYNAMIC_CAST(const FloatData&, r);

	return (getValue() == data.getValue()) ? 0 :
		(getValue() < data.getValue()) ? -1 : 1;
}

//	FUNCTION private
//	Common::FloatData::assign_NoCast -- 代入を行う
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
FloatData::assign_NoCast(const Data& r)
{
#ifdef OBSOLETE // castの結果Nullになることがある場合生かす
	if (r.isNull()) {
		setNull();
		return false;
	}
#endif

	; _TRMEISTER_ASSERT(r.getType() == getType());
	; _TRMEISTER_ASSERT(!r.isNull());

	const FloatData& data = _SYDNEY_DYNAMIC_CAST(const FloatData&, r);
	
	setValue(data.getValue());
	return true;
}

//	FUNCTION private
//	Common::FloatData::operateWith_NoCast -- 四則演算を行う
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
FloatData::operateWith_NoCast(
	DataOperation::Type op, const Data& r)
{
	; _TRMEISTER_ASSERT(r.getType() == DataType::Float);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	const FloatData& data = _SYDNEY_DYNAMIC_CAST(const FloatData&, r);
	
	float fRhs = data.getValue();

	switch (op) {
	case DataOperation::Addition:
		//足し算
		{
			if ((m_fValue>0 && fRhs>0 && fRhs >  fMax-m_fValue)
			 || (m_fValue<0 && fRhs<0 && fRhs < -fMax-m_fValue))
			{				
				setNull();
				return false;
			}
			m_fValue += fRhs;
			return true;
		}
		break;
	case DataOperation::Subtraction:
		// 引き算
		{
			if ((m_fValue>0 && fRhs<0 && m_fValue >  fMax+fRhs)
			 || (m_fValue<0 && fRhs>0 && m_fValue < -fMax+fRhs))
			{				
				setNull();
				return false;
			}
			m_fValue -= fRhs;
			return true;
		}
		break;
	case DataOperation::Multiplication:
		//掛け算
		{
			if((m_fValue >=  1 
					&& ((fRhs >=  1 && m_fValue >  fMax/fRhs)
					 || (fRhs <= -1 && m_fValue > -fMax/fRhs)))
			|| (m_fValue <= -1
					&& ((fRhs >=  1 && m_fValue < -fMax/fRhs)
					 || (fRhs <= -1 && m_fValue <  fMax/fRhs)))
			|| (m_fValue >   0 && m_fValue < 1
					&& ((fRhs >  0 && fRhs < 1 && m_fValue <  fMin/fRhs)
					 || (fRhs > -1 && fRhs < 0 && m_fValue < -fMin/fRhs)))
			|| (m_fValue >  -1 && m_fValue < 0
					&& ((fRhs >  0 && fRhs < 1 && m_fValue > -fMin/fRhs)
					 || (fRhs > -1 && fRhs < 0 && m_fValue >  fMin/fRhs))))
			{				
				setNull();
				return false;
			}
			m_fValue *= fRhs;
			return true;
		}
		break;
	case DataOperation::Division:
		//割り算
		{
			if (fRhs == 0) {
				_TRMEISTER_THROW0(Exception::DivisionByZero);
			}
			if ((m_fValue >=  1 
					&& ((fRhs >   0 && fRhs <= 1 && m_fValue >  fMax*fRhs)
					 || (fRhs >= -1 && fRhs <  0 && m_fValue > -fMax*fRhs)))
			|| (m_fValue <= -1
					&& ((fRhs >   0 && fRhs <= 1 && m_fValue < -fMax*fRhs)
					 || (fRhs >= -1 && fRhs <  0 && m_fValue <  fMax*fRhs)))
			|| (m_fValue >   0 && m_fValue < 1
					&& ((fRhs >  1 && m_fValue <  fMin*fRhs)
					 || (fRhs < -1 && m_fValue < -fMin*fRhs)))
			|| (m_fValue >  -1 && m_fValue < 0
					&& ((fRhs >  1 && m_fValue > -fMin*fRhs)
					 || (fRhs < -1 && m_fValue >  fMin*fRhs))))
			{				
				setNull();
				return false;
			}
			m_fValue /= fRhs;
			return true;
		}
		break;
	case DataOperation::Modulus:
		{
			// float can't calculate modulus
			break;
		}
	default:
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	setNull();
	return false;
}

//	FUNCTION private
//	Common::FloatData::operateWith_NotNull -- 単項演算を行う。
//
//	NOTE
//		単項演算を行う。
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
FloatData::
operateWith_NotNull(DataOperation::Type eOperation_,
					Data::Pointer& pResult_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (eOperation_)
	{
	case DataOperation::Negation:
		pResult_ = new FloatData(-m_fValue);
		return true;
		break;
	case DataOperation::AbsoluteValue:
		if (m_fValue >= 0)
			pResult_ = new FloatData( m_fValue);
		else 
			pResult_ = new FloatData(-m_fValue);
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
//	Common::FloatData::getClassID_NotNull -- クラスIDを得る
//
//	NOTES
//	シリアライズのためのクラスIDを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		Common::FloatDataクラスのクラスID
//
//	EXCEPTIONS
//	なし
//
int
FloatData::
getClassID_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return ClassID::FloatDataClass;
}

//
//	FUNCTION private
//	Common::FloatData::print_NotNull -- 値を表示する
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
FloatData::
print_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	cout << "float: " << m_fValue << endl;
}

//	FUNCTION private
//	Common::FloatData::isAbleToDump_NotNull --
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
FloatData::
isAbleToDump_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return true;
}

//	FUNCTION private
//	Common::FloatData::isFixedSize_NotNull --
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
FloatData::
isFixedSize_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return true;
}

//	FUNCTION private
//	Common::FloatData::getDumpSize_NotNull --
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
FloatData::
getDumpSize_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return _iDumpSize;
}

//	FUNCTION public
//	Common::FloatData::setDumpedValue --
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
FloatData::
setDumpedValue(const char* pszValue_)
{
	Os::Memory::copy(&m_fValue, pszValue_, _iDumpSize);

	setNull(false);

	return _iDumpSize;
}

//virtual
ModSize
FloatData::
setDumpedValue(const char* pszValue_, ModSize uSize_)
{
	if (uSize_ != _iDumpSize) {
		_TRMEISTER_THROW0(Exception::BadArgument);
	}
	return setDumpedValue(pszValue_);
}

//	FUNCTION private
//	Common::FloatData::dumpValue_NotNull --
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
FloatData::
dumpValue_NotNull(char* pszResult_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	Os::Memory::copy(pszResult_, &m_fValue, _iDumpSize);
	return _iDumpSize;
}

//
//	Copyright (c) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
