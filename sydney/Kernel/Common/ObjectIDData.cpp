// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ObjectIDData.cpp -- オブジェクト ID 関連の関数定義
// 
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
#include "Common/Integer64Data.h"
#include "Common/IntegerData.h"
#include "Common/NullData.h"
#include "Common/UnsignedInteger64Data.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/ObjectIDData.h"
#include "Common/StringData.h"
#include "Common/UnicodeString.h"

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
#ifdef DEBUG
	class _TypeCheck
	{
	public:
		_TypeCheck() {check();}
	private:
		void check()
		{
			; _TRMEISTER_ASSERT(Os::Limits<ObjectIDData::FormerType>::IsSpecialized);
			; _TRMEISTER_ASSERT(Os::Limits<ObjectIDData::LatterType>::IsSpecialized);
		}
	};
	_TypeCheck cCheck;
#endif
	// ダンプサイズ
	const ModSize _iDumpSize = sizeof(ObjectIDData::FormerType) + sizeof(ObjectIDData::LatterType);

	// マスクの定義
	const int _iLatterBits = sizeof(ObjectIDData::LatterType) * 8;
	const ModUInt64 _ullLatterMask = static_cast<ModUInt64>(~static_cast<ObjectIDData::LatterType>(0));

	// UInt64を分解する
	ObjectIDData::FormerType
	_getFormerValue(ModUInt64 ullValue_)
	{
		return static_cast<ObjectIDData::FormerType>(ullValue_ >> _iLatterBits);
	}

	ObjectIDData::LatterType
	_getLatterValue(ModUInt64 ullValue_)
	{
		return static_cast<ObjectIDData::LatterType>(ullValue_ & _ullLatterMask);
	}

	void
	_getBinaryValue(ModUInt64 ullValue_, char* pszResult_)
	{
		ObjectIDData::FormerType Former = _getFormerValue(ullValue_);
		ModOsDriver::Memory::copy(pszResult_, &Former, sizeof(ObjectIDData::FormerType));
		ObjectIDData::LatterType Latter = _getLatterValue(ullValue_);
		ModOsDriver::Memory::copy(pszResult_ + sizeof(ObjectIDData::FormerType), &Latter, sizeof(ObjectIDData::LatterType));
	}

	void
	_getBinaryValue(ModUInt64 ullValue_, ModSerialIO& cSerialIO_)
	{
		ObjectIDData::FormerType Former = _getFormerValue(ullValue_);
		cSerialIO_.writeSerial(&Former, sizeof(ObjectIDData::FormerType), ModSerialIO::dataTypeVariable);
		ObjectIDData::LatterType Latter = _getLatterValue(ullValue_);
		cSerialIO_.writeSerial(&Latter, sizeof(ObjectIDData::LatterType), ModSerialIO::dataTypeVariable);
	}

	// UInt64に合成する
	ModUInt64
	_getValue(ObjectIDData::FormerType ulFormerValue_, ObjectIDData::LatterType usLatterValue_)
	{
		return (static_cast<ModUInt64>(ulFormerValue_) << _iLatterBits)
				| static_cast<ModUInt64>(usLatterValue_);
	}

	ModUInt64
	_getValue(const char* pszValue_)
	{
		ObjectIDData::FormerType Former;
		ModOsDriver::Memory::copy(&Former, pszValue_, sizeof(ObjectIDData::FormerType));
		ObjectIDData::LatterType Latter;
		ModOsDriver::Memory::copy(&Latter, pszValue_ + sizeof(ObjectIDData::FormerType), sizeof(ObjectIDData::LatterType));

		return _getValue(Former, Latter);
	}

	ModUInt64
	_getValue(ModSerialIO& cSerialIO_)
	{
		ObjectIDData::FormerType Former;
		cSerialIO_.readSerial(&Former, sizeof(ObjectIDData::FormerType), ModSerialIO::dataTypeVariable);
		ObjectIDData::LatterType Latter;
		cSerialIO_.readSerial(&Latter, sizeof(ObjectIDData::LatterType), ModSerialIO::dataTypeVariable);

		return _getValue(Former, Latter);
	}

	// 最大値・最小値の定義
#ifdef SYD_DLL
	const ObjectIDData::FormerType _ulFormerMax = Os::Limits<ObjectIDData::FormerType>::getMax();
	const ObjectIDData::FormerType _ulFormerMin = Os::Limits<ObjectIDData::FormerType>::getMin();
	const ObjectIDData::LatterType _ulLatterMax = Os::Limits<ObjectIDData::LatterType>::getMax();
	const ObjectIDData::LatterType _ulLatterMin = Os::Limits<ObjectIDData::LatterType>::getMin();
	const ModUInt64 _ullObjectIDMax = _getValue(_ulFormerMax, _ulLatterMax);
	const ModUInt64 _ullObjectIDMin = _getValue(_ulFormerMin, _ulLatterMin);

	ObjectIDData::FormerType getFormerMax()
	{
		return _ulFormerMax;
	}
	ObjectIDData::FormerType getFormerMin()
	{
		return _ulFormerMin;
	}
	ObjectIDData::LatterType getLatterMax()
	{
		return _ulLatterMax;
	}
	ObjectIDData::LatterType getLatterMin()
	{
		return _ulLatterMin;
	}

	ModUInt64 getObjectIDMax()
	{
		return _ullObjectIDMax;
	}
	ModUInt64 getObjectIDMin()
	{
		return _ullObjectIDMin;
	}
#else
	ObjectIDData::FormerType getFormerMax()
	{
		return Os::Limits<ObjectIDData::FormerType>::getMax();
	}
	ObjectIDData::FormerType getFormerMin()
	{
		return Os::Limits<ObjectIDData::FormerType>::getMin();
	}
	ObjectIDData::LatterType getLatterMax()
	{
		return Os::Limits<ObjectIDData::LatterType>::getMax();
	}
	ObjectIDData::LatterType getLatterMin()
	{
		return Os::Limits<ObjectIDData::LatterType>::getMin();
	}

	ModUInt64 getObjectIDMax()
	{
		return _getValue(getFormerMax(), getLatterMax());
	}
	ModUInt64 getObjectIDMin()
	{
		return _getValue(getFormerMin(), getLatterMin());
	}
#endif
}

//static
ModSize
ObjectIDData::
getArchiveSize()
{
	return _iDumpSize;
}

//
//	FUNCTION public
//	Common::ObjectIDData::ObjectIDData -- コンストラクタ(1)
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
ObjectIDData::
ObjectIDData()
: ScalarData(DataType::ObjectID), m_ullValue(0)
{
}

//
//	FUNCTION public
//	Common::ObjectIDData::ObjectIDData -- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ
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
//
ObjectIDData::
ObjectIDData(ModUInt64 ullValue_)
: ScalarData(DataType::ObjectID)
{
	setValue(ullValue_);
}

//
//	FUNCTION public
//	Common::ObjectIDData::ObjectIDData -- コンストラクタ(3)
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	Common::ObjectIDData::FormerType ul32Value_
//	Common::ObjectIDData::LatterType us16Value_
//		データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ObjectIDData::
ObjectIDData(FormerType ulFormerValue_, LatterType usLatterValue_)
: ScalarData(DataType::ObjectID)
{
	setValue(ulFormerValue_, usLatterValue_);
}

//
//	FUNCTION public
//	Common::ObjectIDData::~ObjectIDData -- デストラクタ
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
ObjectIDData::
~ObjectIDData()
{
}

// FUNCTION public
//	Common::ObjectIDData::hashCode -- ハッシュコードを取り出す
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
ObjectIDData::
hashCode() const
{
	if (isNull()) return 0;

	ModSize high = static_cast<ModSize>(m_ullValue >> 32);
	ModSize low = static_cast<ModSize>(m_ullValue & 0xffffffff);
	return (high << 4) + low;
}

//
//	FUNCTION private
//	Common::ObjectIDData::serialize_NotNull -- シリアル化
//
//	NOTES
//	シリアル化を行う
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
ObjectIDData::
serialize_NotNull(ModArchive& cArchiver_)
{
	; _TRMEISTER_ASSERT(!isNull());

	ScalarData::serialize_NotNull(cArchiver_);
	if (cArchiver_.isStore())
	{
		//書出し
		cArchiver_ << m_ullValue;
	}
	else
	{
		//読出し
		cArchiver_ >> m_ullValue;
	}
}

//
//	FUNCTION private
//	Common::ObjectIDData::copy_NotNull -- コピーする
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
ObjectIDData::
copy_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return new ObjectIDData(*this);
}

//
//	FUNCTION private
//	Common::ObjectIDData::cast_NotNull -- キャストする
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
ObjectIDData::
cast_NotNull(DataType::Type eType_, bool bForAssign_ /* = false */) const
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (eType_)
	{
	case DataType::Integer:
	{
		if (m_ullValue <= Os::Limits<int>::getMax())
			return new IntegerData(static_cast<int>(m_ullValue));
		break;
	}
	case DataType::UnsignedInteger:
	{
		if (m_ullValue <= Os::Limits<unsigned int>::getMax())
			return new UnsignedIntegerData(static_cast<unsigned int>(m_ullValue));
		break;
	}
	case DataType::Integer64:
		return new Integer64Data(static_cast<ModInt64>(m_ullValue));
	case DataType::UnsignedInteger64:
		return new UnsignedInteger64Data(m_ullValue);
	case DataType::ObjectID:
		return copy();
	case DataType::String:
		return new StringData(getString());
	case DataType::Float:
		return new FloatData(static_cast<float>(
			static_cast<ModInt64>(m_ullValue)));
	case DataType::Double:
		return new DoubleData(static_cast<double>(
			static_cast<ModInt64>(m_ullValue)));
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

//
//	FUNCTION private
//	Common::ObjectIDData::getString_NotNull -- 文字列でデータを得る
//
//	NOTES
//	文字列でデータを得る
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
ObjectIDData::
getString_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	ModUnicodeOstrStream ostr;
	ostr << m_ullValue;
	return ModUnicodeString(ostr.getString());
}

//
//	FUNCTION public
//	Common::ObjectIDData::getValue -- 値を得る
//
//	NOTES
//	値を得る
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
ObjectIDData::
getValue() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return m_ullValue;
}

//
//	FUNCTION public
//	Common::ObjectIDData::getFormerValue -- 値を得る
//
//	NOTES
//	値を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FormerType
//		データ
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

ObjectIDData::FormerType
ObjectIDData::
getFormerValue() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return _getFormerValue(m_ullValue);
}

// static
ObjectIDData::FormerType
ObjectIDData::getFormerValue(ModUInt64 value)
{
	return _getFormerValue(value);
}

//
//	FUNCTION public
//	Common::ObjectIDData::getLatterValue -- 値を得る
//
//	NOTES
//	値を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	LatterType
//		データ
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

ObjectIDData::LatterType
ObjectIDData::
getLatterValue() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return _getLatterValue(m_ullValue);
}

// static
ObjectIDData::LatterType
ObjectIDData::getLatterValue(ModUInt64 value)
{
	return _getLatterValue(value);
}

//	FUNCTION public
//	Common::ObjectIDData::getMaxValue -- 可能な最大値を得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//	なし

//static
ModUInt64
ObjectIDData::
getMaxValue()
{
	return getObjectIDMax();
}

//	FUNCTION public
//	Common::ObjectIDData::getMaxFormerValue -- 可能な最大値を得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//	なし

//static
ObjectIDData::FormerType
ObjectIDData::
getMaxFormerValue()
{
	return getFormerMax();
}

//	FUNCTION public
//	Common::ObjectIDData::getMaxLatterValue -- 可能な最大値を得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

//static
ObjectIDData::LatterType
ObjectIDData::
getMaxLatterValue()
{
	return getLatterMax();
}

//
//	FUNCTION public
//	Common::ObjectIDData::setValue -- 値を設定する
//
//	NOTES
//	値を設定する
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
//
void
ObjectIDData::
setValue(ModUInt64 ullValue_)
{
	if (ullValue_ > getObjectIDMax() || ullValue_ < getObjectIDMin())
		_TRMEISTER_THROW0(Exception::BadArgument);

	m_ullValue = ullValue_;

	// NULL 値でなくする

	setNull(false);
}

//
//	FUNCTION public
//	Common::ObjectIDData::setValue -- 値を設定する
//
//	NOTES
//	値を設定する
//
//	ARGUMENTS
//	Common::ObjectIDData::FormerType ul32Value_
//	Common::ObjectIDData::LatterType us16Value_
//		データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
ObjectIDData::
setValue(FormerType ulFormerValue_, LatterType usLatterValue_)
{
	m_ullValue = _getValue(ulFormerValue_, usLatterValue_);

	// NULL 値でなくする

	setNull(false);
}

//	FUNCTION private
//	Common::ObjectIDData::compareTo_NoCast -- 大小比較を行う
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
ObjectIDData::compareTo_NoCast(const Data& r) const
{
	// castの結果Nullになることがある
	if (isNull())
		return NullData::getInstance()->compareTo(&r);

	; _TRMEISTER_ASSERT(r.getType() == DataType::ObjectID);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	const ObjectIDData& data = _SYDNEY_DYNAMIC_CAST(const ObjectIDData&, r);

	return (getValue() == data.getValue()) ? 0 :
		(getValue() < data.getValue()) ? -1 : 1;
}

//	FUNCTION private
//	Common::ObjectIDData::assign_NoCast -- 代入を行う
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
ObjectIDData::assign_NoCast(const Data& r)
{
	// castの結果Nullになることがある
	if (r.isNull()) {
		setNull();
		return false;
	}

	; _TRMEISTER_ASSERT(r.getType() == getType());
	; _TRMEISTER_ASSERT(!r.isNull());

	const ObjectIDData& data = _SYDNEY_DYNAMIC_CAST(const ObjectIDData&, r);

	setValue(data.getValue());
	return true;
}

//	FUNCTION private
//	Common::ObjectIDData::operateWith_NoCast -- 四則演算を行う
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
ObjectIDData::operateWith_NoCast(
	DataOperation::Type op, const Data& r)
{
	; _TRMEISTER_ASSERT(r.getType() == DataType::ObjectID);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	const ObjectIDData& data = _SYDNEY_DYNAMIC_CAST(const ObjectIDData&, r);

	ModUInt64 ullRhs = data.getValue();

	switch (op)	{
	case DataOperation::Addition:
		//足し算
		{
			// 両辺とも正なのでオーバーフロー条件は単純(以下同じ)
			if (ullRhs > getObjectIDMax()-m_ullValue)
			{				
				setNull();
				return false;
			}
			m_ullValue += ullRhs;
			return true;
		}
		break;
	case DataOperation::Subtraction:
		//引き算
		{
			if (m_ullValue < ullRhs)
			{				
				setNull();
				return false;
			}
			m_ullValue -= ullRhs;
			return true;
		}
		break;
	case DataOperation::Multiplication:
		//掛け算
		{
			if (m_ullValue> 1 && ullRhs> 1 && m_ullValue > getObjectIDMax() / ullRhs)
			{				
				setNull();
				return false;
			}
			m_ullValue *= ullRhs;
			return true;
		}
		break;
	case DataOperation::Division:
		//割り算
		{
			// /0だけ調べれば良いので楽
			if (ullRhs == 0)
			{				
				_TRMEISTER_THROW0(Exception::DivisionByZero);
			}
			m_ullValue /= ullRhs;
			return true;
		}
		break;
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
		break;
	}
	// never reach
	return false;
}

//	FUNCTION public
//	Common::ObjectIDData::operateWith -- 単項演算を行う。
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
//		BadArgument

bool
ObjectIDData::
operateWith_NotNull(DataOperation::Type eOperation_,
					Data::Pointer& pResult_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (eOperation_)
	{
	case DataOperation::Negation:
		if (m_ullValue != 0)
		{				
			pResult_ = NullData::getInstance();
			return false;
		}
		pResult_ = new ObjectIDData(0);
		return true;
		break;
	case DataOperation::AbsoluteValue:
		pResult_ = new ObjectIDData(m_ullValue);
		return true;
		break;
	default:
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	pResult_ = NullData::getInstance();
	return false;
}

//単項演算関数その2
bool
ObjectIDData::
operateWith_NotNull(DataOperation::Type eOperation_)
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (eOperation_)
	{
	case DataOperation::Increment:
		if (m_ullValue == getObjectIDMax())
		{				
			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
			return false;
		}
		m_ullValue++;
		return true;
		break;
	case DataOperation::Decrement:
		if (m_ullValue == getObjectIDMin())
		{				
			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
			return false;
		}
		m_ullValue--;
		return true;
		break;
	default:
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	return false;
}

//
//	FUNCTION private
//	Common::ObjectIDData::getClassID_NotNull -- クラスIDを得る
//
//	NOTES
//	クラスIDを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		Common::ObjectIDDataクラスのクラスID
//
//	EXCEPTIONS
//	なし
//
int
ObjectIDData::
getClassID_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return ClassID::ObjectIDDataClass;
}

//
//	FUNCTION private
//	Common::ObjectIDData::print_NotNull -- 値を表示する
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
ObjectIDData::
print_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	ModUnicodeString str(getString());
	cout << "ObjectID: " << str.getString(Common::LiteralCode) << endl;
}

//	FUNCTION private
//	Common::ObjectIDData::isAbleToDump_NotNull --
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
ObjectIDData::
isAbleToDump_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return true;
}

//	FUNCTION private
//	Common::ObjectIDData::isFixedSize_NotNull --
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
ObjectIDData::
isFixedSize_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return true;
}

//	FUNCTION private
//	Common::ObjectIDData::getDumpSize_NotNull --
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
ObjectIDData::
getDumpSize_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return _iDumpSize;
}

//	FUNCTION public
//	Common::ObjectIDData::setDumpedValue --
//		dumpされたデータから自身の値をsetする
//
//	NOTES
//		デフォルトの実装は常にNotSupportedを投げる
//
//	ARGUMENTS
//		const char* pszValue_
//			dumpされた領域の先頭
//		ModSize uSize_(省略可)
//			サイズを指定する必要のあるデータの場合指定する
//
//	RETURN
//		値に使ったバイト数
//
//	EXCEPTIONS

//virtual
ModSize
ObjectIDData::
setDumpedValue(const char* pszValue_)
{
	m_ullValue = _getValue(pszValue_);

	setNull(false);

	return _iDumpSize;
}

//virtual
ModSize
ObjectIDData::
setDumpedValue(const char* pszValue_, ModSize uSize_)
{
	if (uSize_ != _iDumpSize) {
		_TRMEISTER_THROW0(Exception::BadArgument);
	}
	return setDumpedValue(pszValue_);
}

//virtual
ModSize
ObjectIDData::
setDumpedValue(ModSerialIO& cSerialIO_)
{
	m_ullValue = _getValue(cSerialIO_);

	setNull(false);

	return _iDumpSize;
}

//virtual
ModSize
ObjectIDData::
setDumpedValue(ModSerialIO& cSerialIO_, ModSize uSize_)
{
	if (uSize_ != _iDumpSize) {
		_TRMEISTER_THROW0(Exception::BadArgument);
	}
	return setDumpedValue(cSerialIO_);
}

//virtual
ModSize
ObjectIDData::
dumpValue_NotNull(ModSerialIO& cSerialIO_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	_getBinaryValue(m_ullValue, cSerialIO_);
	return _iDumpSize;
}

//	FUNCTION public
//	Common::ObjectIDData::dumpValue --
//		自身の値をdumpする
//
//	NOTES
//		デフォルトの実装は常にNotSupportedを投げる
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
ObjectIDData::
dumpValue_NotNull(char* pszResult_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	_getBinaryValue(m_ullValue, pszResult_);
	return _iDumpSize;
}

//
//	Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
