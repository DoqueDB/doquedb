// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Data.cpp --
// 
// Copyright (c) 2013, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Collection";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Execution/Collection/Data.h"

#include "Common/Assert.h"
#include "Common/BinaryData.h"
#include "Common/DataArrayData.h"
#include "Common/DateData.h"
#include "Common/DateTimeData.h"
#include "Common/DecimalData.h"
#include "Common/DoubleData.h"
#include "Common/FloatData.h"
#include "Common/Integer64Data.h"
#include "Common/IntegerData.h"
#include "Common/LanguageData.h"
#include "Common/NullData.h"
#include "Common/ObjectIDData.h"
#include "Common/StringData.h"
#include "Common/UnsignedInteger64Data.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/WordData.h"

#include "Os/Memory.h"
#include "Os/Limits.h"

#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"
#include "Exception/NullabilityViolation.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_COLLECTION_BEGIN

namespace
{
	//
	//	TEMPLATE FUNCTION
	//	_$$::_compare -- 配列の比較
	//
	template <class TYPE>
	int _compare(const TYPE* l, ModSize l_len, const TYPE* r, ModSize r_len)
	{
		ModSize n = 0;
		ModSize length = (l_len < r_len) ? l_len : r_len;
		for (; n < length && *l == *r; ++l, ++r, ++n);
		
		if (n == length)
		{
			// 短い方の長さまで比較したが同じだった
				
			return (l_len == r_len) ? 0 : ((l_len < r_len) ? -1 : 1);
		}
		
		// 違いがあるので、比較終了
		
		return (*l < *r) ? -1 : 1;
	}

}

//
//	FUNCTION public static
//	Execution::Collection::Data::Integer::getSize -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModSize
//		ダンプした時のサイズ
//
//	EXCEPTIONS
//
ModSize
Data::Integer::getSize(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::Integer)
		_SYDNEY_THROW0(Exception::BadArgument);
	return _size();
}

//
//	FUNCTION public
//	Execution::Collection::Data::Integer::dump -- ダンプする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModUInt32*
//		次へのポインタ
//
//	EXCEPTIONS
//
ModUInt32*
Data::Integer::dump(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::Integer)
		_SYDNEY_THROW0(Exception::BadArgument);
	const Common::IntegerData& c
		= _SYDNEY_DYNAMIC_CAST(const Common::IntegerData&, cData_);
	m_value = c.getValue();
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Execution::Collection::Data::Integer::getData -- 取り出す
//
//	NOTES
//
//	ARGUMENTS
// 	Common::Data& cData_
//		値を設定するクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::Integer::getData(Common::Data& cData_) const
{
	Common::IntegerData v(m_value);
	cData_.assign(&v);
}

//
//	FUNCTION public static
//	Execution::Collection::Data::UnsignedInteger::getSize -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModSize
//		ダンプした時のサイズ
//
//	EXCEPTIONS
//
ModSize
Data::UnsignedInteger::getSize(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::UnsignedInteger)
		_SYDNEY_THROW0(Exception::BadArgument);
	return _size();
}

//
//	FUNCTION public
//	Execution::Collection::Data::UnsignedInteger::dump -- ダンプする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModUInt32*
//		次へのポインタ
//
//	EXCEPTIONS
//
ModUInt32*
Data::UnsignedInteger::dump(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::UnsignedInteger)
		_SYDNEY_THROW0(Exception::BadArgument);
	const Common::UnsignedIntegerData& c
		= _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData&, cData_);
	m_value = c.getValue();
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Execution::Collection::Data::UnsignedInteger::getData -- 取り出す
//
//	NOTES
//
//	ARGUMENTS
// 	Common::Data& cData_
//		値を設定するクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::UnsignedInteger::getData(Common::Data& cData_) const
{
	Common::UnsignedIntegerData v(m_value);
	cData_.assign(&v);
}

//
//	FUNCTION public static
//	Execution::Collection::Data::Integer64::getSize -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModSize
//		ダンプした時のサイズ
//
//	EXCEPTIONS
//
ModSize
Data::Integer64::getSize(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::Integer64)
		_SYDNEY_THROW0(Exception::BadArgument);
	return _size();
}

//
//	FUNCTION public
//	Execution::Collection::Data::Integer64::dump -- ダンプする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModUInt32*
//		次へのポインタ
//
//	EXCEPTIONS
//
ModUInt32*
Data::Integer64::dump(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::Integer64)
		_SYDNEY_THROW0(Exception::BadArgument);
	const Common::Integer64Data& c
		= _SYDNEY_DYNAMIC_CAST(const Common::Integer64Data&, cData_);
	m_value = c.getValue();
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Execution::Collection::Data::Integer64::getData -- 取り出す
//
//	NOTES
//
//	ARGUMENTS
// 	Common::Data& cData_
//		値を設定するクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::Integer64::getData(Common::Data& cData_) const
{
	Common::Integer64Data v(m_value);
	cData_.assign(&v);
}

//
//	FUNCTION public static
//	Execution::Collection::Data::UnsignedInteger64::getSize -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModSize
//		ダンプした時のサイズ
//
//	EXCEPTIONS
//
ModSize
Data::UnsignedInteger64::getSize(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::UnsignedInteger64)
		_SYDNEY_THROW0(Exception::BadArgument);
	return _size();
}

//
//	FUNCTION public
//	Execution::Collection::Data::UnsignedInteger64::dump -- ダンプする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModUInt32*
//		次へのポインタ
//
//	EXCEPTIONS
//
ModUInt32*
Data::UnsignedInteger64::dump(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::UnsignedInteger64)
		_SYDNEY_THROW0(Exception::BadArgument);
	const Common::UnsignedInteger64Data& c
		= _SYDNEY_DYNAMIC_CAST(const Common::UnsignedInteger64Data&, cData_);
	m_value = c.getValue();
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Execution::Collection::Data::UnsignedInteger64::getData -- 取り出す
//
//	NOTES
//
//	ARGUMENTS
// 	Common::Data& cData_
//		値を設定するクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::UnsignedInteger64::getData(Common::Data& cData_) const
{
	Common::UnsignedInteger64Data v(m_value);
	cData_.assign(&v);
}

//
//	FUNCTION public static
//	Execution::Collection::Data::String::getSize -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModSize
//		ダンプした時のサイズ
//
//	EXCEPTIONS
//
ModSize
Data::String::getSize(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::String)
		_SYDNEY_THROW0(Exception::BadArgument);
	const Common::StringData& d
		= _SYDNEY_DYNAMIC_CAST(const Common::StringData&, cData_);
	return calcUnitSize(d.getValue().getLength());
}

//
//	FUNCTION public
//	Execution::Collection::Data::String::compare -- 比較関数
//
//	NOTES
//
//	ARGUMENTS
//	const Execution::Collection::Data::String& cOther_
//		比較対象
//
//	RETURN
//	int
//		*this < cOther_		-1
//		*this == cOther_	0
//		*this > cOther_		1
//
//	EXCEPTIONS
//
int
Data::String::compare(const String& cOther_) const
{
	return _compare(m_value, m_length, cOther_.m_value, cOther_.m_length);
}

//
//	FUNCTION public
//	Execution::Collection::Data::String::dump -- ダンプする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModUInt32*
//		次へのポインタ
//
//	EXCEPTIONS
//
ModUInt32*
Data::String::dump(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::String)
		_SYDNEY_THROW0(Exception::BadArgument);
	const Common::StringData& c
		= _SYDNEY_DYNAMIC_CAST(const Common::StringData&, cData_);
	return dump(c.getValue());
}

//
//	FUNCTION public
//	Execution::Collection::Data::String::dump -- ダンプする
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cValue_
//		データ
//
//	RETURN
//	ModUInt32*
//		次へのポインタ
//
//	EXCEPTIONS
//
ModUInt32*
Data::String::dump(const ModUnicodeString& cValue_)
{
	m_length = cValue_.getLength();
	const ModUnicodeChar* p = cValue_;
	ModUnicodeChar* s = &m_value[0];
	Os::Memory::copy(s, p, sizeof(ModUnicodeChar) * m_length);
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Execution::Collection::Data::String::getData -- 取り出す
//
//	NOTES
//
//	ARGUMENTS
// 	Common::Data& cData_
//		値を設定するクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::String::getData(Common::Data& cData_) const
{
	Common::StringData v;
	if (m_length) v.setValue(&m_value[0], m_length);
	cData_.assign(&v);
}

//
//	FUNCTION public
//	Execution::Collection::Data::String::getData -- 取り出す
//
//	NOTES
//
//	ARGUMENTS
// 	ModUnicodeString& cValue_
//		値を設定するクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::String::getData(ModUnicodeString& cValue_) const
{
	if (m_length)
		cValue_.allocateCopy(&m_value[0], m_length);
	else
		cValue_.clear();
}

//
//	FUNCTION public static
//	Execution::Collection::Data::Float::getSize -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModSize
//		ダンプした時のサイズ
//
//	EXCEPTIONS
//
ModSize
Data::Float::getSize(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::Float)
		_SYDNEY_THROW0(Exception::BadArgument);
	return _size();
}

//
//	FUNCTION public
//	Execution::Collection::Data::Float::dump -- ダンプする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModUInt32*
//		次へのポインタ
//
//	EXCEPTIONS
//
ModUInt32*
Data::Float::dump(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::Float)
		_SYDNEY_THROW0(Exception::BadArgument);
	const Common::FloatData& c
		= _SYDNEY_DYNAMIC_CAST(const Common::FloatData&, cData_);
	m_value = c.getValue();
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Execution::Collection::Data::Float::getData -- 取り出す
//
//	NOTES
//
//	ARGUMENTS
//	Common::Data& cData_
//		値を設定するクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::Float::getData(Common::Data& cData_) const
{
	Common::FloatData v(m_value);
	cData_.assign(&v);
}

//
//	FUNCTION public static
//	Execution::Collection::Data::Double::getSize -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModSize
//		ダンプした時のサイズ
//
//	EXCEPTIONS
//
ModSize
Data::Double::getSize(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::Double)
		_SYDNEY_THROW0(Exception::BadArgument);
	return _size();
}

//
//	FUNCTION public
//	Execution::Collection::Data::Double::dump -- ダンプする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModUInt32*
//		次へのポインタ
//
//	EXCEPTIONS
//
ModUInt32*
Data::Double::dump(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::Double)
		_SYDNEY_THROW0(Exception::BadArgument);
	const Common::DoubleData& c
		= _SYDNEY_DYNAMIC_CAST(const Common::DoubleData&, cData_);
	m_value = c.getValue();
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Execution::Collection::Data::Double::getData -- 取り出す
//
//	NOTES
//
//	ARGUMENTS
//	Common::Data& cData_
//		値を設定するクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::Double::getData(Common::Data& cData_) const
{
	Common::DoubleData v(m_value);
	cData_.assign(&v);
}

//
//	FUNCTION public static
//	Execution::Collection::Data::Decimal::getSize -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModSize
//		ダンプした時のサイズ
//
//	EXCEPTIONS
//
ModSize
Data::Decimal::getSize(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::Decimal)
			_SYDNEY_THROW0(Exception::BadArgument);
	const Common::DecimalData& c
		= _SYDNEY_DYNAMIC_CAST(const Common::DecimalData&, cData_);
	return calcUnitSize(c.getPrecision(), c.getScale());
}

//
//	FUNCTION public
//	Execution::Collection::Data::Decimal::compare --
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	int
//
//	EXCEPTIONS
//
int
Data::Decimal::compare(const Decimal& cOther_) const
{
	if (m_precision != cOther_.m_precision || m_scale != cOther_.m_scale)
		// プレシジョンとスケールが同じものしか比較できない
		_SYDNEY_THROW0(Exception::BadArgument);
	
	ModSize len = getSize();	// プレシジョンとスケールが同じなら長さは同じ
	return Os::Memory::compare(m_value, cOther_.m_value, len);
}

//
//	FUNCTION public static
//	Execution::Collection::Data::Decimal::calcUnitSize
//		-- プレシジョンとスケールをユニット長にする
//
//	NOTES
//
//	ARGUMENTS
//	int precision_
//	   	プレシジョン
//	int scale_
//		スケール
//
//	RETURN
//	ModSize
//		ユニット長
//
//	EXCEPTIONS
//
ModSize
Data::Decimal::calcUnitSize(int precision_, int scale_)
{
	// Common::DecimalData::getDumpSizeBy は 4バイト境界
	// m_precision と m_scale 分を足す
	
	return Common::DecimalData::getDumpSizeBy(precision_, scale_) / 4 + 2;
}

//
//	FUNCTION public
//	Execution::Collection::Data::Decimal::dump -- ダンプする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModUInt32*
//		次へのポインタ
//
//	EXCEPTIONS
//
ModUInt32*
Data::Decimal::dump(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::Decimal)
		_SYDNEY_THROW0(Exception::BadArgument);
	const Common::DecimalData& d
		= _SYDNEY_DYNAMIC_CAST(const Common::DecimalData&, cData_);
	m_precision = d.getPrecision();
	m_scale = d.getScale();
	d.dumpValue(syd_reinterpret_cast<char*>(&m_value[0]));
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Execution::Collection::Data::Decimal::getData -- 
//
//	NOTES
//
//	ARGUMENTS
//	Common::Data& cData_
//		値を設定するクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::Decimal::getData(Common::Data& cData_) const
{
	Common::DecimalData v(m_precision, m_scale);
	v.setDumpedValue(syd_reinterpret_cast<const char*>(&m_value[0]));
	cData_.assign(&v);
}

//
//	FUNCTION public static
//	Execution::Collection::Data::Date::getSize -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModSize
//		ダンプした時のサイズ
//
//	EXCEPTIONS
//
ModSize
Data::Date::getSize(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::Date)
		_SYDNEY_THROW0(Exception::BadArgument);
	return _size();
}

//
//	FUNCTION public
//	Execution::Collection::Data::Date::dump -- ダンプする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModUInt32*
//		次へのポインタ
//
//	EXCEPTIONS
//
ModUInt32*
Data::Date::dump(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::Date)
		_SYDNEY_THROW0(Exception::BadArgument);
	const Common::DateData& c
		= _SYDNEY_DYNAMIC_CAST(const Common::DateData&, cData_);
	c.dumpValue(syd_reinterpret_cast<char*>(this));
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Execution::Collection::Data::Date::getData -- 取り出す
//
//	NOTES
//
//	ARGUMENTS
//	Common::Data& cData_
//		値を設定するクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::Date::getData(Common::Data& cData_) const
{
	Common::DateData v;
	v.setDumpedValue(syd_reinterpret_cast<const char*>(this));
	cData_.assign(&v);
}

//
//	FUNCTION public static
//	Execution::Collection::Data::DateTime::getSize -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModSize
//		ダンプした時のサイズ
//
//	EXCEPTIONS
//
ModSize
Data::DateTime::getSize(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::DateTime)
		_SYDNEY_THROW0(Exception::BadArgument);
	return _size();
}

//
//	FUNCTION public
//	Execution::Collection::Data::DateTime::dump -- ダンプする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModUInt32*
//		次へのポインタ
//
//	EXCEPTIONS
//
ModUInt32*
Data::DateTime::dump(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::DateTime)
		_SYDNEY_THROW0(Exception::BadArgument);
	const Common::DateTimeData& c
		= _SYDNEY_DYNAMIC_CAST(const Common::DateTimeData&, cData_);
	c.dumpValue(syd_reinterpret_cast<char*>(this));
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Execution::Collection::Data::DateTime::getData -- 取り出す
//
//	NOTES
//
//	ARGUMENTS
//	Common::Data& cData_
//		値を設定するクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::DateTime::getData(Common::Data& cData_) const
{
	Common::DateTimeData v;
	v.setDumpedValue(syd_reinterpret_cast<const char*>(this));
	cData_.assign(&v);
}

//
//	FUNCTION public static
//	Execution::Collection::Data::Binary::getSize -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModSize
//		ダンプした時のサイズ
//
//	EXCEPTIONS
//
ModSize
Data::Binary::getSize(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::Binary)
		_SYDNEY_THROW0(Exception::BadArgument);
	return calcUnitSize(cData_.getDumpSize());
}

//
//	FUNCTION public
//	Execution::Collection::Data::Binary::compare -- 比較関数
//
//	NOTES
//
//	ARGUMENTS
//	const Execution::Collection::Data::Binary& cOther_
//		比較対象
//
//	RETURN
//	int
//		*this < cOther_		-1
//		*this == cOther_	0
//		*this > cOther_		1
//
//	EXCEPTIONS
//
int
Data::Binary::compare(const Binary& cOther_) const
{
	ModSize len = (m_length < cOther_.m_length) ? m_length : cOther_.m_length;
	int result = Os::Memory::compare(m_value, cOther_.m_value, len);
	if (result == 0)
		return (m_length == cOther_.m_length) ? 0 :
			((m_length < cOther_.m_length) ? -1 : 1);
	return result;
}

//
//	FUNCTION public
//	Execution::Collection::Data::Binary::dump -- ダンプする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModUInt32*
//		次へのポインタ
//
//	EXCEPTIONS
//
ModUInt32*
Data::Binary::dump(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::Binary)
		_SYDNEY_THROW0(Exception::BadArgument);
	const Common::BinaryData& d
		= _SYDNEY_DYNAMIC_CAST(const Common::BinaryData&, cData_);
	m_length = d.getDumpSize();
	d.dumpValue(m_value);
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Execution::Collection::Data::Binary::getData -- 取り出す
//
//	NOTES
//
//	ARGUMENTS
//	Common::Data& cData_
//		値を設定するクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::Binary::getData(Common::Data& cData_) const
{
	Common::BinaryData v(m_value, m_length);
	cData_.assign(&v);
}

//
//	FUNCTION public static
//	Execution::Collection::Data::ObjectID::getSize -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModSize
//		ダンプした時のサイズ
//
//	EXCEPTIONS
//
ModSize
Data::ObjectID::getSize(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::ObjectID)
		_SYDNEY_THROW0(Exception::BadArgument);
	return _size();
}

//
//	FUNCTION public
//	Execution::Collection::Data::ObjectID::dump -- ダンプする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModUInt32*
//		次へのポインタ
//
//	EXCEPTIONS
//
ModUInt32*
Data::ObjectID::dump(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::ObjectID)
		_SYDNEY_THROW0(Exception::BadArgument);
	const Common::ObjectIDData& c
		= _SYDNEY_DYNAMIC_CAST(const Common::ObjectIDData&, cData_);
	m_value = c.getValue();
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Execution::Collection::Data::ObjectID::getData -- 取り出す
//
//	NOTES
//
//	ARGUMENTS
//	Common::Data& cData_
//		値を設定するクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::ObjectID::getData(Common::Data& cData_) const
{
	Common::ObjectIDData v(m_value);
	cData_.assign(&v);
}

//
//	FUNCTION public static
//	Execution::Collection::Data::Language::getSize -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModSize
//		ダンプした時のサイズ
//
//	EXCEPTIONS
//
ModSize
Data::Language::getSize(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::Language)
		_SYDNEY_THROW0(Exception::BadArgument);
	return cData_.getDumpSize() / sizeof(ModUInt32);
}

//
//	FUNCTION public
//	Execution::Collection::Data::Language::compare -- 比較関数
//
//	NOTES
//
//	ARGUMENTS
//	const Execution::Collection::Data::Language& cOther_
//		比較対象
//
//	RETURN
//	int
//		*this < cOther_		-1
//		*this == cOther_	0
//		*this > cOther_		1
//
//	EXCEPTIONS
//
int
Data::Language::compare(const Language& cOther_) const
{
	// ModLanguageSet::compareと同じ
	int result = 0;
	if (m_length != 0 && cOther_.m_length != 0)
	{
		ModUInt32 length
			= m_length < cOther_.m_length ? m_length : cOther_.m_length;
		result = ModOsDriver::Memory::compare(m_value, cOther_.m_value,
											  length * sizeof(ModUInt32));
		if (result == 0 && m_length != cOther_.m_length)
		{
			result = (m_length < cOther_.m_length) ? -1 : 1;
		}
	}
	if (result == 0)
	{
		result = m_length < cOther_.m_length ? -1 :
			(m_length > cOther_.m_length ? 1 : 0);
	}
	return result;
}

//
//	FUNCTION public
//	Execution::Collection::Data::Language::dump -- ダンプする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModUInt32*
//		次へのポインタ
//
//	EXCEPTIONS
//
ModUInt32*
Data::Language::dump(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::Language)
		_SYDNEY_THROW0(Exception::BadArgument);
	const Common::LanguageData& d
		= _SYDNEY_DYNAMIC_CAST(const Common::LanguageData&, cData_);
	d.dumpValue(syd_reinterpret_cast<char*>(this));
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Execution::Collection::Data::Language::dump -- ダンプする
//
//	NOTES
//
//	ARGUMENTS
//	const ModLanguageSet& cValue_
//		データ
//
//	RETURN
//	ModUInt32*
//		次へのポインタ
//
//	EXCEPTIONS
//
ModUInt32*
Data::Language::dump(const ModLanguageSet& cValue_)
{
	// 非効率だがしょうがない
	
	Common::LanguageData c(cValue_);
	c.dumpValue(syd_reinterpret_cast<char*>(this));
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Execution::Collection::Data::Language::getData -- 取り出す
//
//	NOTES
//
//	ARGUMENTS
//	Common::Data& cData_
//		値を設定するクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::Language::getData(Common::Data& cData_) const
{
	Common::LanguageData v;
	v.setDumpedValue(syd_reinterpret_cast<const char*>(this),
					 getSize() * sizeof(ModUInt32));
	cData_.assign(&v);
}

//
//	FUNCTION public
//	Execution::Collection::Data::Language::getData -- 取り出す
//
//	NOTES
//
//	ARGUMENTS
//	ModLanguageSet& cValue_
//		値を設定するクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::Language::getData(ModLanguageSet& cValue_) const
{
	// 非効率だがしょうがない

	Common::LanguageData c;
	c.setDumpedValue(syd_reinterpret_cast<const char*>(this),
					 getSize()*sizeof(ModUInt32));
	cValue_ = c.getValue();
}

//
//	FUNCTION public
//	Execution::Collection::Data::Word::getSize -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		自身のサイズ
//
//	EXCEPTIONS
//
ModSize
Data::Word::getSize() const
{
	// 文字列の言語のサイズを得る 
	const ModUInt32* p = &m_value[0];
	ModSize s = String::getSize(p);
	p += s;
	s += Language::getSize(p);

	// その他を加える
	return s + getOtherSize();
}

//
//	FUNCTION public static
//	Execution::Collection::Data::Word::getSize -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModSize
//		ダンプした時のサイズ
//
//	EXCEPTIONS
//
ModSize
Data::Word::getSize(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::Word)
		_SYDNEY_THROW0(Exception::BadArgument);

	const Common::WordData& c
		= _SYDNEY_DYNAMIC_CAST(const Common::WordData&, cData_);

	// 文字列と言語のサイズを得る
	ModSize s = String::calcUnitSize(c.getTerm().getLength())
		+ Language::calcUnitSize(c.getLanguage().getSize());

	// その他を加える
	return s + getOtherSize();
}

//
//	FUNCTION public
//	Execution::Collection::Data::Word::compare -- 比較関数
//
//	NOTES
//
//	ARGUMENTS
//	const Execution::Collection::Data::Word& cOther_
//		比較対象
//
//	RETURN
//	int
//		*this < cOther_		-1
//		*this == cOther_	0
//		*this > cOther_		1
//
//	EXCEPTIONS
//
int
Data::Word::compare(const Word& cOther_) const
{
	// まず言語を比べて、その後単語で比較する

	const ModUInt32* p1 = &m_value[0];
	const ModUInt32* p2 = &cOther_.m_value[0];
	const Data::String* s1 = syd_reinterpret_cast<const Data::String*>(p1);
	const Data::String* s2 = syd_reinterpret_cast<const Data::String*>(p2);
	const Data::Language* l1
		= syd_reinterpret_cast<const Data::Language*>(p1 + s1->getSize());
	const Data::Language* l2
		= syd_reinterpret_cast<const Data::Language*>(p2 + s2->getSize());

	int r = l1->compare(*l2);
	if (r == 0)
		r = s1->compare(*s2);
	return r;
}

//
//	FUNCTION public
//	Execution::Collection::Data::Word::compare_df -- 比較関数
//
//	NOTES
//
//	ARGUMENTS
//	const Execution::Collection::Data::Word& cOther_
//		比較対象
//
//	RETURN
//	int
//		*this < cOther_		-1
//		*this == cOther_	0
//		*this > cOther_		1
//
//	EXCEPTIONS
//
int
Data::Word::compare_df(const Word& cOther_) const
{
	// df を比べる

	return (m_df < cOther_.m_df) ? -1 : ((m_df == cOther_.m_df) ? 0 : 1);
}

//
//	FUNCTION public
//	Execution::Collection::Data::Word::compare_scale -- 比較関数
//
//	NOTES
//
//	ARGUMENTS
//	const Execution::Collection::Data::Word& cOther_
//		比較対象
//
//	RETURN
//	int
//		*this < cOther_		-1
//		*this == cOther_	0
//		*this > cOther_		1
//
//	EXCEPTIONS
//
int
Data::Word::compare_scale(const Word& cOther_) const
{
	// scale を比べる

	return (m_scale < cOther_.m_scale) ? -1 :
		((m_scale == cOther_.m_scale) ? 0 : 1);
}

//
//	FUNCTION public
//	Execution::Collection::Data::Word::dump -- ダンプする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModUInt32*
//		次へのポインタ
//
//	EXCEPTIONS
//
ModUInt32*
Data::Word::dump(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::Word)
		_SYDNEY_THROW0(Exception::BadArgument);
	const Common::WordData& d
		= _SYDNEY_DYNAMIC_CAST(const Common::WordData&, cData_);

	m_scale = d.getScale();
	m_df = d.getDocumentFrequency();
	m_category = d.getCategory();

	ModUInt32* p = &m_value[0];
	p = syd_reinterpret_cast<Data::String*>(p)->dump(d.getTerm());
	p = syd_reinterpret_cast<Data::Language*>(p)->dump(d.getLanguage());
	
	return p;
}

//
//	FUNCTION public
//	Execution::Collection::Data::Word::getData -- 取り出す
//
//	NOTES
//
//	ARGUMENTS
//	Common::Data& cData_
//		値を設定するクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::Word::getData(Common::Data& cData_) const
{
	Common::WordData v;
	v.setScale(m_scale);
	v.setDocumentFrequency(m_df);
	v.setCategory(static_cast<Common::WordData::Category::Value>(m_category));
	
	// 単語
	const ModUInt32* p = &m_value[0];
	const Data::String* str = syd_reinterpret_cast<const Data::String*>(p);
	ModUnicodeString term;
	str->getData(term);
	v.setTerm(term);

	// 言語
	p += str->getSize();
	const Data::Language* lang = syd_reinterpret_cast<const Data::Language*>(p);
	ModLanguageSet la;
	lang->getData(la);
	v.setLanguage(la);

	// null ではなくする
	v.setNull(false);

	cData_.assign(&v);
}

//
//	FUNCTION public
//	Execution::Collection::Data::Array -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		サイズ
//
//	EXCEPTIONS
//
ModSize
Data::Array::getSize() const
{
	// 要素の先頭
	const ModUInt32* p = getElementData();

	// 要素数と型の分
	ModSize size = (sizeof(int) + sizeof(int) * m_count) / sizeof(ModUInt32);
	const int* t = &m_type[0];

	for (int i = 0; i < m_count; ++i, ++t)
	{
		size += Data::getSize(p, static_cast<Type::Value>(*t));
		// nullの場合は0が返る
	}

	return size;
}

//
//	FUNCTION public static
//	Execution::Collection::Data::Array::getSize -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModSize
//		ダンプした時のサイズ
//
//	EXCEPTIONS
//
ModSize
Data::Array::getSize(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::Array ||
		cData_.getElementType() != Common::DataType::Data)
		_SYDNEY_THROW0(Exception::BadArgument);

	const Common::DataArrayData& d
		= _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&, cData_);
	int n = d.getCount();

	// m_count と m_type 分
	ModSize size = (sizeof(int) + sizeof(int) * n) / sizeof(ModUInt32);

	// データ部分
	for (int i = 0; i < n; ++i)
	{
		const Common::Data::Pointer p = d.getElement(i);
		size += Data::getSize(*p);	// nullの場合は0が返る
	}

	return size;
}

//
//	FUNCTION public
//	Execution::Collection::Data::Array::compare -- 比較する
//
//	NOTES
//
//	ARGUMENTS
//	const Execution::Collection::Data::Array& cOther_
//		比較対象
//
//	RETURN
//	int
//		*this < cOther_		-1
//		*this == cOther_	0
//		*this > cOther_		1
//
//	EXCEPTIONS
//
int
Data::Array::compare(const Array& cOther_) const
{
	int n = (m_count < cOther_.m_count) ? m_count : cOther_.m_count;

	const int* t0 = &m_type[0];
	const int* t1 = &(cOther_.m_type[0]);

	const ModUInt32* p0 = getElementData();
	const ModUInt32* p1 = cOther_.getElementData();

	for (int i = 0; i < n; ++i, ++t0, ++t1)
	{
		int r = Data::compare(p0, static_cast<Type::Value>(*t0),
							  p1, static_cast<Type::Value>(*t1));
		if (r != 0)
			return r;

		// 次へ
		Data::getSize(p0, static_cast<Type::Value>(*t0));
		Data::getSize(p1, static_cast<Type::Value>(*t1));
	}

	return (m_count == cOther_.m_count) ? 0 :
		((m_count < cOther_.m_count) ? -1 : 1);
}

//
//	FUNCTION public
//	Execution::Collection::Data::Array::dump -- ダンプする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModUInt32*
//		次へのポインタ
//
//	EXCEPTIONS
//
ModUInt32*
Data::Array::dump(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::Array ||
		cData_.getElementType() != Common::DataType::Data)
		_SYDNEY_THROW0(Exception::BadArgument);
	
	const Common::DataArrayData& d
		= _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&, cData_);

	m_count = d.getCount();
	int* t = &m_type[0];
	ModUInt32* p = syd_reinterpret_cast<ModUInt32*>(t + m_count);

	for (int i = 0; i < m_count; ++i, ++t)
	{
		const Common::Data::Pointer e = d.getElement(i);
		*t = Data::getType(*e);
		Data::dump(p, *e);
	}
	
	return p;
}

//
//	FUNCTION public
//	Execution::Collection::Data::Array::getData -- 取り出す
//
//	NOTES
//
//	ARGUMENTS
//	Common::Data& cData_
//		値を設定するクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::Array::getData(Common::Data& cData_) const
{
	Common::DataArrayData v;
	v.reserve(m_count);
	
	const int* t = &m_type[0];
	const ModUInt32* p = syd_reinterpret_cast<const ModUInt32*>(t + m_count);
	for (int i = 0; i < m_count; ++i, ++t)
	{
		// インスタンスを確保
		Common::Data::Pointer e = makeData(static_cast<Type::Value>(*t));
		// データコピー
		Data::getData(p, static_cast<Type::Value>(*t), *e);
		// 追加
		v.pushBack(e);
	}

	cData_.assign(&v);
}

//
//	FUNCTION public
//	Execution::Collection::Array::getElementData
//		-- 要素データの先頭を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModUInt32*
//		要素データの先頭
//
//	EXCEPTIONS
//
const ModUInt32*
Data::Array::getElementData() const
{
	const int* t = &m_type[0];
	return syd_reinterpret_cast<const ModUInt32*>(t + m_count);
}

//
//	FUNCTION public
//	Execution::Collection::Array::getElementData
//		-- 要素データの先頭を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32*
//		要素データの先頭
//
//	EXCEPTIONS
//
ModUInt32*
Data::Array::getElementData()
{
	int* t = &m_type[0];
	return syd_reinterpret_cast<ModUInt32*>(t + m_count);
}

//
//	FUNCTION public
//	Execution::Collection::Data::Data -- コンストラクタ
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
Data::Data()
	: m_count(0)
{
}

//
//	FUNCTION public
//	Execution::Collection::Data::~Data -- デストラクタ
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
Data::~Data()
{
}

//
//	FUNCTION public static
//	Execution::Collection::Data::getSize -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* p
//		メモリ
//
//	RETURN
//	ModSize
//		サイズ
//
//	EXCEPTIONS
//
ModSize
Data::getSize(const ModUInt32* p)
{
	return Array::getSize(p);
}

//
//	FUNCTION public static
//	Execution::Collection::Data::getSize -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cData_
//		データ
//
//	RETURN
//	ModSize
//		サイズ
//
//	EXCEPTIONS
//
ModSize
Data::getSize(const Common::DataArrayData& cData_)
{
	return Array::getSize(cData_);
}

//
//	FUNCTION public static
//	Execution::Collection::Data::getSize -- １つのサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModSize
//		サイズ
//
//	EXCEPTIONS
//
ModSize
Data::getSize(const Common::Data& cData_)
{
	ModSize size = 0;
	if (!cData_.isNull())
	{
		switch (cData_.getType())
		{
		case Common::DataType::Integer:
			size = Integer::getSize(cData_);
			break;
		case Common::DataType::UnsignedInteger:
			size = UnsignedInteger::getSize(cData_);
			break;
		case Common::DataType::Integer64:
			size = Integer64::getSize(cData_);
			break;
		case Common::DataType::UnsignedInteger64:
			size = UnsignedInteger64::getSize(cData_);
			break;
		case Common::DataType::String:
			size = String::getSize(cData_);
			break;
		case Common::DataType::Float:
			size = Float::getSize(cData_);
			break;
		case Common::DataType::Double:
			size = Double::getSize(cData_);
			break;
		case Common::DataType::Decimal:
			size = Decimal::getSize(cData_);
			break;
		case Common::DataType::Date:
			size = Date::getSize(cData_);
			break;
		case Common::DataType::DateTime:
			size = DateTime::getSize(cData_);
			break;
		case Common::DataType::Binary:
			size = Binary::getSize(cData_);
			break;
		case Common::DataType::ObjectID:
			size = ObjectID::getSize(cData_);
			break;
		case Common::DataType::Language:
			size = Language::getSize(cData_);
			break;
		case Common::DataType::Word:
			size = Word::getSize(cData_);
			break;
		case Common::DataType::Array:
			size = Array::getSize(cData_);
			break;
		default:
			// エラー
			_SYDNEY_THROW0(Exception::BadArgument);
		}
	}
	return size;
}

//
//	FUNCTION public static
//	Execution::Collection::Data::getSize -- 1つのサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32*& p
//		メモリ
//	Execution::Collection::Data::Type::Value eType_
//		データ型
//
//	RETURN
//	ModSize
//		1つのサイズ
//
//	EXCEPTIONS
//
ModSize
Data::getSize(const ModUInt32*& p, Type::Value eType_)
{
	ModSize size = 0;
	
	switch (eType_)
	{
	case Type::Integer:
		size = Integer::getSize(p);
		break;
	case Type::UnsignedInteger:
		size = UnsignedInteger::getSize(p);
		break;
	case Type::Integer64:
		size = Integer64::getSize(p);
		break;
	case Type::UnsignedInteger64:
		size = UnsignedInteger64::getSize(p);
		break;
	case Type::String:
		size = String::getSize(p);
		break;
	case Type::Float:
		size = Data::Float::getSize(p);
		break;
	case Type::Double:
		size = Data::Double::getSize(p);
		break;
	case Type::Decimal:
		size = Data::Decimal::getSize(p);
		break;
	case Type::Date:
		size = Data::Date::getSize(p);
		break;
	case Type::DateTime:
		size = Data::DateTime::getSize(p);
		break;
	case Type::Binary:
		size = Data::Binary::getSize(p);
		break;
	case Type::ObjectID:
		size = Data::ObjectID::getSize(p);
		break;
	case Type::Language:
		size = Data::Language::getSize(p);
		break;
	case Type::Word:
	case Type::Word_Df:
	case Type::Word_Scale:
		size = Data::Word::getSize(p);
		break;
	case Type::Array:
		size = Data::Array::getSize(p);
		break;
	case Type::Null:
		size = 0;
		break;
	default:
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	p += size;
	return size;
}

//
//	FUNCTION public
//	Execution::Collection::Data::dump -- ダンプする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cData_
//		データ
//
//	RETURN
//	ModSize
//		サイズ
//
//	EXCEPTIONS
//
void
Data::dump(const Common::DataArrayData& cData_)
{
	Data::Array* d = syd_reinterpret_cast<Data::Array*>(this);
	d->dump(cData_);
}

//
//	FUNCTION public static
//	Execution::Collection::Data::dump -- 1つのダンプする
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32*& p
//		メモリ
//	const Common::Data& cData_
//		データ
//
//	RETURN
//	ModSize
//		1つのサイズ
//
//	EXCEPTIONS
//
void
Data::dump(ModUInt32*& p, const Common::Data& cData_)
{
	switch (getType(cData_))
	{
	case Type::Integer:
		{
			Data::Integer* d = syd_reinterpret_cast<Data::Integer*>(p);
			p = d->dump(cData_);
		}
		break;
	case Type::UnsignedInteger:
		{
			Data::UnsignedInteger* d
				= syd_reinterpret_cast<Data::UnsignedInteger*>(p);
			p = d->dump(cData_);
		}
		break;
	case Type::Integer64:
		{
			Data::Integer64* d = syd_reinterpret_cast<Data::Integer64*>(p);
			p = d->dump(cData_);
		}
		break;
	case Type::UnsignedInteger64:
		{
			Data::UnsignedInteger64* d
				= syd_reinterpret_cast<Data::UnsignedInteger64*>(p);
			p = d->dump(cData_);
		}
		break;
	case Type::String:
		{
			Data::String* d = syd_reinterpret_cast<Data::String*>(p);
			p = d->dump(cData_);
		}
		break;
	case Type::Float:
		{
			Data::Float* d = syd_reinterpret_cast<Data::Float*>(p);
			p = d->dump(cData_);
		}
		break;
	case Type::Double:
		{
			Data::Double* d = syd_reinterpret_cast<Data::Double*>(p);
			p = d->dump(cData_);
		}
		break;
	case Type::Decimal:
		{
			Data::Decimal* d = syd_reinterpret_cast<Data::Decimal*>(p);
			p = d->dump(cData_);
		}
		break;
	case Type::Date:
		{
			Data::Date* d = syd_reinterpret_cast<Data::Date*>(p);
			p = d->dump(cData_);
		}
		break;
	case Type::DateTime:
		{
			Data::DateTime* d = syd_reinterpret_cast<Data::DateTime*>(p);
			p = d->dump(cData_);
		}
		break;
	case Type::Binary:
		{
			Data::Binary* d = syd_reinterpret_cast<Data::Binary*>(p);
			p = d->dump(cData_);
		}
		break;
	case Type::ObjectID:
		{
			Data::ObjectID* d = syd_reinterpret_cast<Data::ObjectID*>(p);
			p = d->dump(cData_);
		}
		break;
	case Type::Language:
		{
			Data::Language* d = syd_reinterpret_cast<Data::Language*>(p);
			p = d->dump(cData_);
		}
		break;
	case Type::Word:
	case Type::Word_Df:
	case Type::Word_Scale:
		{
			Data::Word* d = syd_reinterpret_cast<Data::Word*>(p);
			p = d->dump(cData_);
		}
		break;
	case Type::Array:
		{
			Data::Array* d = syd_reinterpret_cast<Data::Array*>(p);
			p = d->dump(cData_);
		}
		break;
	case Type::Null:
		// 何も書かない
		break;
	default:
		_SYDNEY_THROW0(Exception::NotSupported);
	}
}

//
//	FUNCTION public
//	Execution::Collection::Data::getData -- データを得る
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData& cTuple_
//		データを格納する配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::getData(Common::DataArrayData& cTuple_) const
{
	//【注意】 	引数の cTuple_ には要素が設定された状態で渡されるので、
	//			要素をクリアしてしまう Array::getData とは同じにはできない

	if (cTuple_.getCount() != m_count)
		_SYDNEY_THROW0(Exception::BadArgument);

	const int* t = &m_type[0];
	const ModUInt32* p
		= syd_reinterpret_cast<const ModUInt32*>(t + m_count);
	for (int i = 0; i < m_count; ++i, ++t)
	{
		Common::Data::Pointer pData = cTuple_.getElement(i);
		
		if (static_cast<Type::Value>(*t) == Type::Null)
		{
			// null なので、null にする
			
			pData->setNull();
		}
		else
		{
			// データを取得する

			getData(p, static_cast<Type::Value>(*t), *pData);
		}
	}
}

//
//	FUNCTION public
//	Execution::Colection::Data::getElement
//		-- 要素を得る
//
//	NOTES
//
//	ARGUMENTS
//	int n_
//		要素番号
//	Execution::Collection::Data::Type::Value& t_
//		要素の型
//
//	RETURN
//	const ModUInt32*
//		要素へのポインター
//
//	EXCEPTIONS
//
const ModUInt32*
Data::getElement(int n_, Type::Value& t_) const
{
	// 先頭要素の型情報とデータのポインターを得る
	const int* t = &m_type[0];
	const ModUInt32* p
		= syd_reinterpret_cast<const ModUInt32*>(t + m_count);
	
	// 対象のデータまで読み飛ばす
	for (int i = 0; i < n_; ++i, ++t)
		Data::getSize(p, static_cast<Type::Value>(*t));

	t_ = static_cast<Type::Value>(*t);
	return p;
}

//
//	FUNCTION public
//	Execution::Collection::Data::compare
//		-- 1つの要素の大小比較を行う
//
//	NOTES
//
//	ARGUMENTS
//	const Execution::Collection::Data* other_
//		比較対象の行データ
//	int iElement_
//		比較対象の要素番号
//	Execution::Collection::Data::Type::Value
//		比較型。Type::Noneを指定すると、行データ内の型情報で比較を行う
//		(default は Type::None)
//
//	RETURN
//	int
//		p0_ の方が小さいと -1、同じなら 0、それ以外なら 1 を返す
//
//	EXCEPTIONS
//
int
Data::compare(const Data* other_, int iElement_, Type::Value t_) const
{
	if (m_count != other_->m_count)
		_SYDNEY_THROW0(Exception::BadArgument);
	if (iElement_ >= m_count)
		_SYDNEY_THROW0(Exception::BadArgument);

	// 対象データを得る
	
	Type::Value t0;
	const ModUInt32* p0 = getElement(iElement_, t0);
	Type::Value t1;
	const ModUInt32* p1 = other_->getElement(iElement_, t1);
	

	// 比較用の型を得る

	if (t_ != Type::None)
	{
		if (t0 == Type::Word)
		{
			if (t_ == Type::Word ||
				t_ == Type::Word_Df ||
				t_ == Type::Word_Scale)
			{
				t0 = t_;
			}
			else
			{
				_SYDNEY_THROW0(Exception::BadArgument);
			}
		}
		else if (t0 != Type::Null)
		{
			if (t_ != t0)
				_SYDNEY_THROW0(Exception::BadArgument);
		}
		
		if (t1 == Type::Word)
		{
			if (t_ == Type::Word ||
				t_ == Type::Word_Df ||
				t_ == Type::Word_Scale)
			{
				t1 = t_;
			}
			else
			{
				_SYDNEY_THROW0(Exception::BadArgument);
			}
		}
		else if (t1 != Type::Null)
		{
			if (t_ != t1)
				_SYDNEY_THROW0(Exception::BadArgument);
		}
	}

	// 比較する
	
	return compare(p0, t0, p1, t1);
}

//
//	FUNCTION public static
//	Execution::Collection::Data::compare
//		-- 1つの要素の大小比較を行う
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* p0_
//		要素データ
//	Execution::Collection::Data::Type::Value t0_
//		要素の型
//	const ModUInt32* p1_
//		要素データ
//	Execution::Collection::Data::Type::Value t1_
//		要素の型
//
//	RETURN
//	int
//		p0_ の方が小さいと -1、同じなら 0、それ以外なら 1 を返す
//
//	EXCEPTIONS
//
int
Data::compare(const ModUInt32* p0_, Type::Value t0_,
			  const ModUInt32* p1_, Type::Value t1_)
{
	// どちらかが null かどうか確認する
	// null は一番小さいとする

	if (t0_ == Type::Null || t1_ == Type::Null)
	{
		if (t0_ == Type::Null && t1_ == Type::Null)
			return 0;
		if (t1_ != Type::Null)
			return -1;
		else
			return 1;
	}

	// どちらも null ではない場合

	if (t0_ != t1_)
		// 型が違う場合はエラー
		_SYDNEY_THROW0(Exception::BadArgument);

	int result = 0;

	switch (t0_)
	{
	case Type::Integer:
		{
			const Data::Integer* d0
				= syd_reinterpret_cast<const Data::Integer*>(p0_);
			const Data::Integer* d1
				= syd_reinterpret_cast<const Data::Integer*>(p1_);
			result = d0->compare(*d1);
		}
		break;
	case Type::UnsignedInteger:
		{
			const Data::UnsignedInteger* d0
				= syd_reinterpret_cast<const Data::UnsignedInteger*>(p0_);
			const Data::UnsignedInteger* d1
				= syd_reinterpret_cast<const Data::UnsignedInteger*>(p1_);
			result = d0->compare(*d1);
		}
		break;
	case Type::Integer64:
		{
			const Data::Integer64* d0
				= syd_reinterpret_cast<const Data::Integer64*>(p0_);
			const Data::Integer64* d1
				= syd_reinterpret_cast<const Data::Integer64*>(p1_);
			result = d0->compare(*d1);
		}
		break;
	case Type::UnsignedInteger64:
		{
			const Data::UnsignedInteger64* d0
				= syd_reinterpret_cast<const Data::UnsignedInteger64*>(p0_);
			const Data::UnsignedInteger64* d1
				= syd_reinterpret_cast<const Data::UnsignedInteger64*>(p1_);
			result = d0->compare(*d1);
		}
		break;
	case Type::String:
		{
			const Data::String* d0
				= syd_reinterpret_cast<const Data::String*>(p0_);
			const Data::String* d1
				= syd_reinterpret_cast<const Data::String*>(p1_);
			result = d0->compare(*d1);
		}
		break;
	case Type::Float:
		{
			const Data::Float* d0
				= syd_reinterpret_cast<const Data::Float*>(p0_);
			const Data::Float* d1
				= syd_reinterpret_cast<const Data::Float*>(p1_);
			result = d0->compare(*d1);
		}
		break;
	case Type::Double:
		{
			const Data::Double* d0
				= syd_reinterpret_cast<const Data::Double*>(p0_);
			const Data::Double* d1
				= syd_reinterpret_cast<const Data::Double*>(p1_);
			result = d0->compare(*d1);
		}
		break;
	case Type::Decimal:
		{
			const Data::Decimal* d0
				= syd_reinterpret_cast<const Data::Decimal*>(p0_);
			const Data::Decimal* d1
				= syd_reinterpret_cast<const Data::Decimal*>(p1_);
			result = d0->compare(*d1);
		}
		break;
	case Type::Date:
		{
			const Data::Date* d0 = syd_reinterpret_cast<const Data::Date*>(p0_);
			const Data::Date* d1 = syd_reinterpret_cast<const Data::Date*>(p1_);
			result = d0->compare(*d1);
		}
		break;
	case Type::DateTime:
		{
			const Data::DateTime* d0
				= syd_reinterpret_cast<const Data::DateTime*>(p0_);
			const Data::DateTime* d1
				= syd_reinterpret_cast<const Data::DateTime*>(p1_);
			result = d0->compare(*d1);
		}
		break;
	case Type::Binary:
		{
			const Data::Binary* d0
				= syd_reinterpret_cast<const Data::Binary*>(p0_);
			const Data::Binary* d1
				= syd_reinterpret_cast<const Data::Binary*>(p1_);
			result = d0->compare(*d1);
		}
		break;
	case Type::ObjectID:
		{
			const Data::ObjectID* d0
				= syd_reinterpret_cast<const Data::ObjectID*>(p0_);
			const Data::ObjectID* d1
				= syd_reinterpret_cast<const Data::ObjectID*>(p1_);
			result = d0->compare(*d1);
		}
		break;
	case Type::Language:
		{
			const Data::Language* d0
				= syd_reinterpret_cast<const Data::Language*>(p0_);
			const Data::Language* d1
				= syd_reinterpret_cast<const Data::Language*>(p1_);
			result = d0->compare(*d1);
		}
		break;
	case Type::Word:
		{
			const Data::Word* d0
				= syd_reinterpret_cast<const Data::Word*>(p0_);
			const Data::Word* d1
				= syd_reinterpret_cast<const Data::Word*>(p1_);
			result = d0->compare(*d1);
		}
		break;
	case Type::Word_Df:
		{
			const Data::Word* d0
				= syd_reinterpret_cast<const Data::Word*>(p0_);
			const Data::Word* d1
				= syd_reinterpret_cast<const Data::Word*>(p1_);
			result = d0->compare_df(*d1);
		}
		break;
	case Type::Word_Scale:
		{
			const Data::Word* d0
				= syd_reinterpret_cast<const Data::Word*>(p0_);
			const Data::Word* d1
				= syd_reinterpret_cast<const Data::Word*>(p1_);
			result = d0->compare_scale(*d1);
		}
		break;
	case Type::Array:
		{
			const Data::Array* d0
				= syd_reinterpret_cast<const Data::Array*>(p0_);
			const Data::Array* d1
				= syd_reinterpret_cast<const Data::Array*>(p1_);
			result = d0->compare(*d1);
		}
		break;
	default:
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	return result;
}

//
//	FUNCTION public static
//	Execution::Collection::Data::getType
//		-- Common::DataType から Data::Type::Value に変換する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//
//	RETGURN
//	Execution::Collection::Data::Type
//		タイプ
//
//	EXCEPTIONS
//
Data::Type::Value
Data::getType(const Common::Data& cData_)
{
	Type::Value eType = Type::Null;
	if (cData_.isNull() == false)
	{
		switch (cData_.getType())
		{
		case Common::DataType::Integer:
			eType = Type::Integer;
			break;
		case Common::DataType::UnsignedInteger:
			eType = Type::UnsignedInteger;
			break;
		case Common::DataType::Integer64:
			eType = Type::Integer64;
			break;
		case Common::DataType::UnsignedInteger64:
			eType = Type::UnsignedInteger64;
			break;
		case Common::DataType::String:
			eType = Type::String;
			break;
		case Common::DataType::Float:
			eType = Type::Float;
			break;
		case Common::DataType::Double:
			eType = Type::Double;
			break;
		case Common::DataType::Decimal:
			eType = Type::Decimal;
			break;
		case Common::DataType::Date:
			eType = Type::Date;
			break;
		case Common::DataType::DateTime:
			eType = Type::DateTime;
			break;
		case Common::DataType::Binary:
			eType = Type::Binary;
			break;
		case Common::DataType::ObjectID:
			eType = Type::ObjectID;
			break;
		case Common::DataType::Language:
			eType = Type::Language;
			break;
		case Common::DataType::Word:
			eType = Type::Word;
			break;
		case Common::DataType::Array:
			eType = Type::Array;
			break;
		default:
			// エラー
			_SYDNEY_THROW0(Exception::BadArgument);
		}
	}
	return eType;
}

//
//	FUNCTION public static
//	Execution::Collection::Data::makeData -- タイプからCommon::Dataを得る
//
//	NOTES
//
//	ARGUMENTS
//	Execution::Collection::Data::Type::Value eType_
//		データ型
//
//	RETURN
//	Common::Data::Pointer
//		Common::Dataへのポインタ
//
//	EXCEPTIONS
//
Common::Data::Pointer
Data::makeData(Type::Value eType_)
{
	Common::Data::Pointer p;
	
	switch (eType_)
	{
	case Type::Integer:
		p = new Common::IntegerData;
		break;
	case Type::UnsignedInteger:
		p = new Common::UnsignedIntegerData;
		break;
	case Type::Integer64:
		p = new Common::Integer64Data;
		break;
	case Type::UnsignedInteger64:
		p = new Common::UnsignedInteger64Data;
		break;
	case Type::String:
		p = new Common::StringData;
		break;
	case Type::Float:
		p = new Common::FloatData;
		break;
	case Type::Double:
		p = new Common::DoubleData;
		break;
	case Type::Decimal:
        p = new Common::DecimalData;
		break;
	case Type::Date:
		p = new Common::DateData;
		break;
	case Type::DateTime:
		p = new Common::DateTimeData;
		break;
	case Type::Binary:
		p = new Common::BinaryData;
		break;
	case Type::ObjectID:
		p = new Common::ObjectIDData;
		break;
	case Type::Language:
		p = new Common::LanguageData;
		break;
	case Type::Word:
	case Type::Word_Df:
	case Type::Word_Scale:
		p = new Common::WordData;
		break;
	case Type::Array:
		p = new Common::DataArrayData;
		break;
	case Type::Null:
		p = Common::NullData::getInstance();
		break;
	default:
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	return p;
}

//
//	FUNCTION public static
//	Execution::Collection::Data::getData -- 1つデータを得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32*& p
//		メモリ
//	Execution::Collection::Data::Type::Value eType_
//		データ型
//	Common::Data& cData_
//		値を設定するクラス
//
//	RETURN
//	ModSize
//		1つのサイズ
//
//	EXCEPTIONS
//
void
Data::getData(const ModUInt32*& p,
			  Type::Value eType_,
			  Common::Data& cData_)
{
	switch (eType_)
	{
	case Type::Integer:
		{
			const Data::Integer* d
				= syd_reinterpret_cast<const Data::Integer*>(p);
			d->getData(cData_);
			p += d->getSize();
		}
		break;
	case Type::UnsignedInteger:
		{
			const Data::UnsignedInteger* d
				= syd_reinterpret_cast<const Data::UnsignedInteger*>(p);
			d->getData(cData_);
			p += d->getSize();
		}
		break;
	case Type::Integer64:
		{
			const Data::Integer64* d
				= syd_reinterpret_cast<const Data::Integer64*>(p);
			d->getData(cData_);
			p += d->getSize();
		}
		break;
	case Type::UnsignedInteger64:
		{
			const Data::UnsignedInteger64* d
				= syd_reinterpret_cast<const Data::UnsignedInteger64*>(p);
			d->getData(cData_);
			p += d->getSize();
		}
		break;
	case Type::String:
		{
			const Data::String* d
				= syd_reinterpret_cast<const Data::String*>(p);
			d->getData(cData_);
			p += d->getSize();
		}
		break;
	case Type::Float:
		{
			const Data::Float* d
				= syd_reinterpret_cast<const Data::Float*>(p);
			d->getData(cData_);
			p += d->getSize();
		}
		break;
	case Type::Double:
		{
			const Data::Double* d
				= syd_reinterpret_cast<const Data::Double*>(p);
			d->getData(cData_);
			p += d->getSize();
		}
		break;
	case Type::Decimal:
		{
			const Data::Decimal* d
				= syd_reinterpret_cast<const Data::Decimal*>(p);
			d->getData(cData_);
			p += d->getSize();
		}
		break;
	case Type::Date:
		{
			const Data::Date* d
				= syd_reinterpret_cast<const Data::Date*>(p);
			d->getData(cData_);
			p += d->getSize();
		}
		break;
	case Type::DateTime:
		{
			const Data::DateTime* d
				= syd_reinterpret_cast<const Data::DateTime*>(p);
			d->getData(cData_);
			p += d->getSize();
		}
		break;
	case Type::Binary:
		{
			const Data::Binary* d
				= syd_reinterpret_cast<const Data::Binary*>(p);
			d->getData(cData_);
			p += d->getSize();
		}
		break;
	case Type::ObjectID:
		{
			const Data::ObjectID* d
				= syd_reinterpret_cast<const Data::ObjectID*>(p);
			d->getData(cData_);
			p += d->getSize();
		}
		break;
	case Type::Language:
		{
			const Data::Language* d
				= syd_reinterpret_cast<const Data::Language*>(p);
			d->getData(cData_);
			p += d->getSize();
		}
		break;
	case Type::Word:
	case Type::Word_Df:
	case Type::Word_Scale:
		{
			const Data::Word* d
				= syd_reinterpret_cast<const Data::Word*>(p);
			d->getData(cData_);
			p += d->getSize();
		}
		break;
	case Type::Array:
		{
			const Data::Array* d
				= syd_reinterpret_cast<const Data::Array*>(p);
			d->getData(cData_);
			p += d->getSize();
		}
		break;
	case Type::Null:
		{
			cData_.setNull();
		}
		break;
	default:
		_SYDNEY_THROW0(Exception::NotSupported);
	}
}

_SYDNEY_EXECUTION_COLLECTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
//	Copyright (c) 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
