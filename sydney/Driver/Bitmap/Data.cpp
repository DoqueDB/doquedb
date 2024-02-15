// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Data.cpp --
// 
// Copyright (c) 2005, 2006, 2007, 2009, 2011, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"
#include "Bitmap/Data.h"

#include "Common/Assert.h"
#include "Common/BasicString.h"
#include "Common/Collation.h"
#include "Common/IntegerData.h"
#include "Common/UnsignedIntegerData.h"
#ifdef OBSOLETE
#include "Common/FloatData.h"
#endif
#include "Common/DecimalData.h"
#include "Common/DoubleData.h"
#include "Common/StringData.h"
#ifdef OBSOLETE
#include "Common/DateData.h"
#endif
#include "Common/DateTimeData.h"
#include "Common/DataArrayData.h"
#include "Common/NullData.h"
#include "Common/ObjectIDData.h"
#include "Common/LanguageData.h"
#include "Common/Integer64Data.h"

#include "Os/Memory.h"
#include "Os/Limits.h"

#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"
#include "Exception/NullabilityViolation.h"

#include "FileCommon/FileOption.h"

#include "ModAlgorithm.h"
#include "ModAutoPointer.h"
#include "ModOsDriver.h"
#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

namespace
{
	//
	//	TEMPLATE FUNCTION
	//	_$$::_compare -- 配列の比較
	//
	template <class TYPE>
	int _compare(const TYPE* l, const TYPE* r, ModSize length)
	{
		ModSize n = 0;
		for (; *l == *r; ++l, ++r)
			if (++n == length)
				return 0;
		return (*l < *r) ? -1 : 1;
	}

	//
	//	TEMPLATE FUNCTION
	//	_$$::_compare -- 配列の比較
	//
	template <class DATA>
	int _compare(const DATA& c1_, const DATA& c2_)
	{
		ModSize length = (c1_.m_length < c2_.m_length)
			? c1_.m_length : c2_.m_length;
		int comp = 0;
		if (length)
		{
			comp = _compare(c1_.m_value, c2_.m_value, length);
		}
		if (comp == 0 && c1_.m_length != c2_.m_length)
			comp = (c1_.m_length < c2_.m_length) ? -1 : 1;
		return comp;
	}
	
	//
	//	TEMPLATE FUNCTION
	//	_$$::_compareSpace -- Compare with white space
	//
	template <class TYPE>
	int _compareSpace(const TYPE* s, ModSize begin_, ModSize end_)
	{
		const TYPE* e = s + end_;
		s += begin_;
		for (; s < e && *s == 0x20; ++s);
		return (s == e) ? 0 : ((*s < 0x20) ? -1 : 1);
	}
	
	//
	//	TEMPLATE FUNCTION
	//	_$$::_comparePadSpace -- Comapre strings
	//
	template <class DATA>
	int _comparePadSpace(const DATA& c1_, const DATA& c2_)
	{
		ModSize length = ModMin(c1_.m_length, c2_.m_length);
		int comp = 0;
		if (length)
		{
			comp = _compare(c1_.m_value, c2_.m_value, length);
		}
		if (comp == 0)
		{
			if (c1_.m_length > c2_.m_length)
			{
				comp = _compareSpace(c1_.m_value, c2_.m_length, c1_.m_length);
			}
			else if (c1_.m_length < c2_.m_length)
			{
				comp = _compareSpace(c2_.m_value, c1_.m_length, c2_.m_length);
				// Interchange the result.
				comp *= -1;
			}
		}
		return comp;
	}
}

//
//	FUNCTION public static
//	Bitmap::Data::Integer::getSize -- サイズを得る
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
//	Bitmap::Data::Integer::dump -- ダンプする
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
		= static_cast<const Common::IntegerData&>(cData_);
	m_value = c.getValue();
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Bitmap::Data::Integer::getData -- 取り出す
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
	; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::Integer);
	_SYDNEY_DYNAMIC_CAST(Common::IntegerData&, cData_).setValue(m_value);
}

//
//	FUNCTION public static
//	Bitmap::Data::Integer::round -- 丸める
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		入力
//	Bitmap::Data::MatchType::Value& eMatch_
//		一致条件(入力)
//	int& value_
//		出力
//
//	RETURN
//	bool
//		検索でヒットする可能性がある場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Data::Integer::round(const Common::Data& cData_,
					 MatchType::Value& eMatch_,
					 int& value_)
{
	bool result = true;

	// ここにはInteger64DataとDoubleDataしか来ない
	if (cData_.getType() == Common::DataType::Double)
	{
		const Common::DoubleData& c
			= _SYDNEY_DYNAMIC_CAST(const Common::DoubleData&, cData_);
		double d = c.getValue();

		if (d == static_cast<double>(static_cast<int>(d)))
		{
			value_ = static_cast<int>(d);
			return true;
		}
		
		switch (eMatch_)
		{
		case MatchType::Equals:
			result = false;
			break;
		case MatchType::GreaterThan:
			if (d > static_cast<double>(Os::Limits<int>::getMax()))
				result = false;
			else if (d < static_cast<double>(Os::Limits<int>::getMin()))
			{
				value_ = Os::Limits<int>::getMin();
				eMatch_ = MatchType::GreaterThanEquals;
			}
			else
			{
				value_ = static_cast<int>(d);
			}
			break;
		case MatchType::GreaterThanEquals:
			if (d > static_cast<double>(Os::Limits<int>::getMax()))
				result = false;
			else if (d < static_cast<double>(Os::Limits<int>::getMin()))
			{
				value_ = Os::Limits<int>::getMin();
			}
			else
			{
				value_ = static_cast<int>(d) + 1;
			}
			break;
		case MatchType::LessThan:
			if (d < static_cast<double>(Os::Limits<int>::getMin()))
				result = false;
			else if (d > static_cast<double>(Os::Limits<int>::getMax()))
			{
				value_ = Os::Limits<int>::getMax();
				eMatch_ = MatchType::LessThanEquals;
			}
			else
			{
				value_ = static_cast<int>(d) + 1;
			}
			break;
		case MatchType::LessThanEquals:
			if (d < static_cast<double>(Os::Limits<int>::getMin()))
				result = false;
			else if (d > static_cast<double>(Os::Limits<int>::getMax()))
			{
				value_ = Os::Limits<int>::getMax();
			}
			else
			{
				value_ = static_cast<int>(d);
			}
			break;
		}
	}
	else if (cData_.getType() == Common::DataType::Integer64)
	{
		const Common::Integer64Data& c
			= _SYDNEY_DYNAMIC_CAST(const Common::Integer64Data&, cData_);
		ModInt64 d = c.getValue();

		if (d == static_cast<ModInt64>(static_cast<int>(d)))
		{
			value_ = static_cast<int>(d);
			return true;
		}

		// ここにくるのはintの範囲を超えているとき
		switch (eMatch_)
		{
		case MatchType::Equals:
			result = false;
			break;
		case MatchType::GreaterThan:
			if (d > static_cast<ModInt64>(Os::Limits<int>::getMax()))
				result = false;
			else
			{
				value_ = Os::Limits<int>::getMin();
				eMatch_ = MatchType::GreaterThanEquals;
			}
			break;
		case MatchType::GreaterThanEquals:
			if (d > static_cast<ModInt64>(Os::Limits<int>::getMax()))
				result = false;
			else
			{
				value_ = Os::Limits<int>::getMin();
			}
			break;
		case MatchType::LessThan:
			if (d < static_cast<ModInt64>(Os::Limits<int>::getMin()))
				result = false;
			else
			{
				value_ = Os::Limits<int>::getMax();
				eMatch_ = MatchType::LessThanEquals;
			}
			break;
		case MatchType::LessThanEquals:
			if (d < static_cast<ModInt64>(Os::Limits<int>::getMin()))
				result = false;
			else
			{
				value_ = Os::Limits<int>::getMax();
			}
			break;
		}
	}
	else
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	return result;
}

//
//	FUNCTION public static
//	Bitmap::Data::UnsignedInteger::getSize -- サイズを得る
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
//	Bitmap::Data::UnsignedInteger::dump -- ダンプする
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
		= static_cast<const Common::UnsignedIntegerData&>(cData_);
	m_value = c.getValue();
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Bitmap::Data::UnsignedInteger::getData -- 取り出す
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
	; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::UnsignedInteger);
	_SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData&, cData_)
		.setValue(m_value);
}

#ifdef OBSOLETE
//
//	FUNCTION public static
//	Bitmap::Data::Float::getSize -- サイズを得る
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
//	Bitmap::Data::Float::dump -- ダンプする
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
		= static_cast<const Common::FloatData&>(cData_);
	m_value = c.getValue();
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Bitmap::Data::Float::getData -- 取り出す
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
	; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::Float);
	_SYDNEY_DYNAMIC_CAST(Common::FloatData&, cData_).setValue(m_value);
}
#endif

//
//	FUNCTION public static
//	Bitmap::Data::Double::getSize -- サイズを得る
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
//	Bitmap::Data::Double::dump -- ダンプする
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
		= static_cast<const Common::DoubleData&>(cData_);
	m_value = c.getValue();
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Bitmap::Data::Double::getData -- 取り出す
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
	; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::Double);
	_SYDNEY_DYNAMIC_CAST(Common::DoubleData&, cData_).setValue(m_value);
}

//
//	FUNCTION public static
//	Bitmap::Data::Decimal::getSize -- サイズを得る
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
	return calcUnitSize(cData_.getDumpSize());
}

//
//	FUNCTION public
//	Bitmap::Data::Decimal::dump -- ダンプする
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
		= static_cast<const Common::DecimalData&>(cData_);
	m_length = d.getDumpSize();
	d.dumpValue(syd_reinterpret_cast<char*>(&m_value[0]));
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Bitmap::Data::Decimal::compare --
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
	// ModLanguageSet::compareと同じ
	int result = 0;
	if (m_length != 0 && cOther_.m_length != 0)
	{
		ModUInt32 length
			= m_length < cOther_.m_length ? m_length : cOther_.m_length;
		result = ModOsDriver::Memory::compare(m_value, cOther_.m_value, length);
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
//	Bitmap::Data::Decimal::getData -- 
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
	; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::Decimal);
	_SYDNEY_DYNAMIC_CAST(Common::DecimalData&, cData_)
		//.setDumpedValue(syd_reinterpret_cast<const char*>(&m_value[0]), m_length);
		.setDumpedValue(syd_reinterpret_cast<const char*>(&m_value[0]));
}

//
//	FUNCTION public
//	Bitmap::Data::Decimal::round -- 
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
bool 
Data::Decimal::round(const Common::Data& cData_,
					 MatchType::Value& eMatch_,
					 ModSize iPrecision_, ModSize iScale_, 
					 ModUnicodeString& cstrValue_)
{
	Common::DecimalData ddMaxValue, ddMinValue;
	ddMaxValue.setToMaxDecimalData(iPrecision_, iScale_);
	ddMinValue.setToMinDecimalData(iPrecision_, iScale_);

	enum nearType
	{
		isBiggerMax = 0,
		isLessMin = 1,
		isMiddle = 2,
	};
	bool bMax = false;
	bool isNegative = false;

	nearType tNear = isBiggerMax;
	Common::DecimalData dDataValue;
	if (cData_.getType() == Common::DataType::Decimal)
	{
		if (ddMaxValue.compareTo(&cData_) == -1)  //ddMinValue < ddMaxValue < cData_
		{
			tNear = isBiggerMax;
		}
		else if (ddMaxValue.compareTo(&cData_) == 0)
		{
			tNear = isMiddle;
			bMax = true;
		}
		else if (cData_.compareTo(&ddMinValue) == -1)  //cData_ < ddMinValue < ddMaxValue
		{
			tNear = isLessMin;
			isNegative = true;
		}
		else  //ddMinValue <= cData_ <= ddMaxValue
		{
			tNear = isMiddle;
			dDataValue = _SYDNEY_DYNAMIC_CAST(const Common::DecimalData&, cData_);
			isNegative = dDataValue.getValue().isNegative();
		}
	}
	else
	{
		Common::Data::Pointer pData(cData_.castToDecimal());
		if (!pData->isNull())
		{
			dDataValue = _SYDNEY_DYNAMIC_CAST(const Common::DecimalData&, *pData);
			isNegative = dDataValue.getValue().isNegative();
			if (ddMaxValue.compareTo(&dDataValue) == -1)  //ddMinValue < ddMaxValue < cData_
			{
				tNear = isBiggerMax;
			}
			else if (ddMaxValue.compareTo(&dDataValue) == 0)
			{
				tNear = isMiddle;
				bMax = true;
			}
			else if (dDataValue.compareTo(&ddMinValue) == -1)  //cData_ < ddMinValue < ddMaxValue
			{
				tNear = isLessMin;
				isNegative = true;
			}
			else  //ddMinValue <= cData_ <= ddMaxValue
			{
				tNear = isMiddle;
			}
		}
		else
		{
			Common::ObjectPointer<Common::Data> ppMax(ddMaxValue.cast(cData_));
			int iCompare = ppMax->compareTo(&cData_);
			if (iCompare == 1)  //ddMaxValue > cData_
			{
				isNegative = true;
				tNear = isLessMin;
			}
			else  //ddMaxValue < cData_
			{
				tNear = isBiggerMax;
			}
		}
	}

	Common::DecimalData ddValue(iPrecision_, iScale_);
	if ((tNear == isMiddle) && (!bMax))
	{		
		if (dDataValue.canNoLostCastTo(iPrecision_, iScale_))
		{
			cstrValue_ = dDataValue.getString();
			return true;	
		}
		else
			ddValue.assign(&dDataValue); 
	}
	else if (tNear == isLessMin) 
		ddValue = ddMinValue;
	else if ((tNear == isBiggerMax) || (bMax))
		ddValue = ddMaxValue;

	bool result = true;
	switch (eMatch_)
	{
	case MatchType::Equals:
		result = false;
		break;
	case MatchType::NotEquals:
		cstrValue_ = ddMinValue.getString();
		eMatch_ = MatchType::GreaterThanEquals;
		break;
	case MatchType::GreaterThan:
		if ((tNear == isBiggerMax) || bMax)
			result = false;
		else if (tNear == isLessMin)
		{
			cstrValue_ = ddValue.getString();
			eMatch_ = MatchType::GreaterThanEquals;
		}
		else
		{
			cstrValue_ = ddValue.getString();
			if (isNegative)
				eMatch_ = MatchType::GreaterThanEquals;
		}

		break;
	case MatchType::GreaterThanEquals:
		if ((tNear == isBiggerMax) && (!bMax))
			result = false;
		else if (tNear == isLessMin)
			cstrValue_ = ddValue.getString();
		else if (bMax)
			cstrValue_ = ddValue.getString();
		else
		{
			cstrValue_ = ddValue.getString();
			if (!isNegative)
			{
				eMatch_ = MatchType::GreaterThan;
			}
		}		
		break;
	case MatchType::LessThan:
		if (tNear == isLessMin)
			result = false;
		else if ((tNear == isBiggerMax) && (!bMax))
		{
			cstrValue_ = ddValue.getString();
			eMatch_ = MatchType::LessThanEquals;
		}
		else if (bMax)
			cstrValue_ = ddValue.getString();
		else
		{
			cstrValue_ = ddValue.getString();
			if (!isNegative)
				eMatch_ = MatchType::LessThanEquals; 
		}		
		break;
	case MatchType::LessThanEquals:
		if (tNear == isLessMin)
			result = false;
		else if ((tNear == isBiggerMax) || (bMax))
			cstrValue_ = ddValue.getString();
		else if ((tNear == isMiddle) && (!bMax))
		{
			cstrValue_ = ddValue.getString();
			if (isNegative)
				eMatch_ = MatchType::LessThan;			
		}
		break;

	}

#ifdef DEBUG
	if (result)
		_SYDNEY_ASSERT(cstrValue_.getLength() > 0);
#endif

	return result;
}

//
//	FUNCTION public static
//	Bitmap::Data::Decimal::getParameter -- 
//
//	NOTES
//	int iPosition_
//		The position of the field in the point of upper module
//	int isArray_
//		Is the field Array?
//
//	ARGUMENTS
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
void
Data::Decimal::getParameter(const LogicalFile::FileID& cFileID_,
							int iPosition_,
							bool isArray_,
							int& iPrecision_,
							int& iScale_)
{
	; _SYDNEY_ASSERT(iPosition_ != -1);
	
	// FieldLength has two meanings.
	// When isArray_==false, FieldLength means the size of the field [byte].
	// When isArray_==true, FieldLength means the number of the fields.
	// Instead of it, ElementLength means the size of the field [byte].
	int precisionKey = (isArray_ == true) ?
		FileCommon::FileOption::ElementLength::Key :
		FileCommon::FileOption::FieldLength::Key;
	iPrecision_ = cFileID_.getInteger(
		_SYDNEY_FILE_PARAMETER_FORMAT_KEY(precisionKey, iPosition_));
	// FieldFraction has only one meaning.
	int scaleKey = FileCommon::FileOption::FieldFraction::Key;
	iScale_ = cFileID_.getInteger(
		_SYDNEY_FILE_PARAMETER_FORMAT_KEY(scaleKey, iPosition_));
}

//
//	FUNCTION public static
//	Bitmap::Data::CharString::getSize -- サイズを得る
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
Data::CharString::getSize(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::String)
		_SYDNEY_THROW0(Exception::BadArgument);
	const Common::StringData& d
		= static_cast<const Common::StringData&>(cData_);
#ifdef OBSOLETE
	const ModUnicodeString& str = d.getValue();
	const ModUnicodeChar* p = str;
	unsigned short size = 0;
	while (*p)
	{
		int s = 0;
		if (*p < 0x80)			s = 1;
		else if (*p < 0x800)	s = 2;
		else					s = 3;
		size += s;
		p += s;
	}
	return calcUnitSize(size);
#else
	return calcUnitSize(static_cast<unsigned short>(d.getValue().getLength()));
#endif
}

//
//	FUNCTION public
//	Bitmap::Data::CharString::compare -- 比較関数
//
//	NOTES
//
//	ARGUMENTS
//	const Bitmap::Data::CharString& cOther_
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
Data::CharString::compare(const CharString& cOther_) const
{
	return _comparePadSpace(*this, cOther_);
}

//
//	FUNCTION public
//	Bitmap::Data::CharString::dump -- ダンプする
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
Data::CharString::dump(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::String)
		_SYDNEY_THROW0(Exception::BadArgument);
	const Common::StringData& c
		= static_cast<const Common::StringData&>(cData_);
#ifdef OBSOLETE
	char* s = &m_value[0];
	ModSize size = c.dumpValue(s, Common::StringData::EncodingForm::UTF8);
	m_length = static_cast<unsigned short>(size);
#else
	const ModUnicodeString& str = c.getValue();
	m_length = static_cast<unsigned short>(str.getLength());
	const ModUnicodeChar* p = str;
	unsigned char* s = &m_value[0];
	while (*p)
	{
		if (*p < 0x80)
			*s++ = static_cast<unsigned char>(*p);
		else
			// The value is NOT belonged to the ASCII code,
			// so it is replaced with 0x80.
			*s++ = 0x80;
		++p;
	}
#endif
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Bitmap::Data::CharString::getData -- 取り出す
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
Data::CharString::getData(Common::Data& cData_) const
{
	; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::String);
	_SYDNEY_DYNAMIC_CAST(Common::StringData&, cData_)
		.setDumpedValue(syd_reinterpret_cast<const char*>(&m_value[0]),
						m_length,
						Common::StringData::EncodingForm::UTF8);
}

//
//	FUNCTION public static
//	Bitmap::Data::UnicodeString::getSize -- サイズを得る
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
Data::UnicodeString::getSize(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::String)
		_SYDNEY_THROW0(Exception::BadArgument);
	const Common::StringData& d
		= static_cast<const Common::StringData&>(cData_);
	return calcUnitSize(static_cast<unsigned short>(d.getValue().getLength()));
}

//
//	FUNCTION public
//	Bitmap::Data::UnicodeString::compare -- 比較関数
//
//	NOTES
//
//	ARGUMENTS
//	const Bitmap::Data::UnicodeString& cOther_
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
Data::UnicodeString::compare(const UnicodeString& cOther_) const
{
	return _comparePadSpace(*this, cOther_);
}

//
//	FUNCTION public
//	Bitmap::Data::UnicodeString::dump -- ダンプする
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
Data::UnicodeString::dump(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::String)
		_SYDNEY_THROW0(Exception::BadArgument);
	const Common::StringData& c
		= static_cast<const Common::StringData&>(cData_);
	const ModUnicodeString& str = c.getValue();
	m_length = static_cast<unsigned short>(str.getLength());
	const ModUnicodeChar* p = str;
	ModUnicodeChar* s = &m_value[0];
	Os::Memory::copy(s, p, sizeof(ModUnicodeChar) * m_length);
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Bitmap::Data::UnicodeString::getData -- 取り出す
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
Data::UnicodeString::getData(Common::Data& cData_) const
{
	; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::String);
	_SYDNEY_DYNAMIC_CAST(Common::StringData&, cData_)
		.setValue(&m_value[0], m_length);
}

//
//	FUNCTION public
//	Bitmap::Data::NoPadCharString::compare -- 比較関数
//
//	NOTES
//
//	ARGUMENTS
//	const Btree2::Data::NoPadCharString& cOther_
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
Data::NoPadCharString::compare(const NoPadCharString& cOther_) const
{
	return _compare(*this, cOther_);
}

//
//	FUNCTION public
//	Bitmap::Data::NoPadCharString::like --
//
//	NOTES
//
//	ARGUMENTS
//	const Btree2::Data::NoPadCharString& cOther_
//		比較対象
//	ModUnicodeChar escape_
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool
Data::NoPadCharString::like(const NoPadCharString& cOther_,
							ModUnicodeChar escape_) const
{
	bool result = false;
	if (escape_ == 0)
	{
		result = Common::BasicString<unsigned char>::like(
			m_value, m_length, cOther_.m_value, cOther_.m_length);
	}
	else
	{
		result = Common::BasicString<unsigned char>::like(
			m_value, m_length, cOther_.m_value, cOther_.m_length,
			static_cast<unsigned char>(escape_));
	}
	return result;
}

//
//	FUNCTION public
//	Bitmap::Data::NoPadUnicodeString::compare -- 比較関数
//
//	NOTES
//
//	ARGUMENTS
//	const Btree2::Data::NoPadUnicodeString& cOther_
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
Data::NoPadUnicodeString::compare(const NoPadUnicodeString& cOther_) const
{
	return _compare(*this, cOther_);
}

//
//	FUNCTION public
//	Bitmap::Data::NoPadUnicodeString::like --
//
//	NOTES
//
//	ARGUMENTS
//	const Btree2::Data::NoPadUnicodeString& cOther_
//		比較対象
//	ModUnicodeChar escape_
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool
Data::NoPadUnicodeString::like(const NoPadUnicodeString& cOther_,
							   ModUnicodeChar escape_) const
{
	bool result = false;
	if (escape_ == 0)
	{
		result = Common::BasicString<ModUnicodeChar>::like(
			m_value, m_length, cOther_.m_value, cOther_.m_length);
	}
	else
	{
		result = Common::BasicString<ModUnicodeChar>::like(
			m_value, m_length, cOther_.m_value, cOther_.m_length, escape_);
	}
	return result;
}

#ifdef OBSOLETE
//
//	FUNCTION public static
//	Bitmap::Data::Date::getSize -- サイズを得る
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
//	Bitmap::Data::Date::dump -- ダンプする
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
		= static_cast<const Common::DateData&>(cData_);
	c.dumpValue(syd_reinterpret_cast<char*>(this));
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Bitmap::Data::Date::getData -- 取り出す
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
	; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::Date);
	_SYDNEY_DYNAMIC_CAST(Common::DateData&, cData_)
		.setDumpedValue(syd_reinterpret_cast<const char*>(this));
}
#endif

//
//	FUNCTION public static
//	Bitmap::Data::DateTime::getSize -- サイズを得る
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
//	Bitmap::Data::DateTime::dump -- ダンプする
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
		= static_cast<const Common::DateTimeData&>(cData_);
	c.dumpValue(syd_reinterpret_cast<char*>(this));
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Bitmap::Data::DateTime::getData -- 取り出す
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
	; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::DateTime);
	_SYDNEY_DYNAMIC_CAST(Common::DateTimeData&, cData_)
		.setDumpedValue(syd_reinterpret_cast<const char*>(this));
}

//
//	FUNCTION public static
//	Bitmap::Data::ObjectID::getSize -- サイズを得る
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
//	Bitmap::Data::ObjectID::dump -- ダンプする
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
		= static_cast<const Common::ObjectIDData&>(cData_);
	m_value = c.getValue();
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Bitmap::Data::ObjectID::getData -- 取り出す
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
	; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::ObjectID);
	_SYDNEY_DYNAMIC_CAST(Common::ObjectIDData&, cData_).setValue(m_value);
}

//
//	FUNCTION public static
//	Bitmap::Data::LanguageSet::getSize -- サイズを得る
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
Data::LanguageSet::getSize(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::Language)
		_SYDNEY_THROW0(Exception::BadArgument);
	return cData_.getDumpSize() / sizeof(ModUInt32);
}

//
//	FUNCTION public
//	Bitmap::Data::LanguageSet::compare -- 比較関数
//
//	NOTES
//
//	ARGUMENTS
//	const Bitmap::Data::LanguageSet& cOther_
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
Data::LanguageSet::compare(const LanguageSet& cOther_) const
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
//	Bitmap::Data::LanguageSet::dump -- ダンプする
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
Data::LanguageSet::dump(const Common::Data& cData_)
{
	if (cData_.getType() != Common::DataType::Language)
		_SYDNEY_THROW0(Exception::BadArgument);
	const Common::LanguageData& d
		= static_cast<const Common::LanguageData&>(cData_);
	d.dumpValue(syd_reinterpret_cast<char*>(this));
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Bitmap::Data::LanguageSet::getData -- 取り出す
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
Data::LanguageSet::getData(Common::Data& cData_) const
{
	; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::Language);
	_SYDNEY_DYNAMIC_CAST(Common::LanguageData&, cData_)
		.setDumpedValue(syd_reinterpret_cast<const char*>(this),
						getSize()*sizeof(ModUInt32));
}

//
//	FUNCTION public static
//	Bitmap::Data::Integer64::getSize -- サイズを得る
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
//	Bitmap::Data::Integer64::dump -- ダンプする
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
		= static_cast<const Common::Integer64Data&>(cData_);
	m_value = c.getValue();
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Bitmap::Data::Integer64::getData -- 取り出す
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
	; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::Integer64);
	_SYDNEY_DYNAMIC_CAST(Common::Integer64Data&, cData_).setValue(m_value);
}

//
//	FUNCTION public static
//	Bitmap::Data::Integer64::round -- 丸める
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		入力
//	Bitmap::Data::MatchType::Value& eMatch_
//		一致条件(入力)
//	ModInt64& value_
//		出力
//
//	RETURN
//	bool
//		検索でヒットする可能性がある場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Data::Integer64::round(const Common::Data& cData_,
					   MatchType::Value& eMatch_,
					   ModInt64& value_)
{
	bool result = true;

	// DoubleDataしか来ないはず
	if (cData_.getType() == Common::DataType::Double)
	{
		const Common::DoubleData& c
			= _SYDNEY_DYNAMIC_CAST(const Common::DoubleData&, cData_);
		double d = c.getValue();

		if (d == static_cast<double>(static_cast<ModInt64>(d)))
		{
			value_ = static_cast<ModInt64>(d);
			return true;
		}
		
		switch (eMatch_)
		{
		case MatchType::Equals:
			result = false;
			break;
		case MatchType::GreaterThan:
			if (d > static_cast<double>(Os::Limits<ModInt64>::getMax()))
				result = false;
			else if (d < static_cast<double>(Os::Limits<ModInt64>::getMin()))
			{
				value_ = Os::Limits<ModInt64>::getMin();
				eMatch_ = MatchType::GreaterThanEquals;
			}
			else
			{
				value_ = static_cast<ModInt64>(d);
			}
			break;
		case MatchType::GreaterThanEquals:
			if (d > static_cast<double>(Os::Limits<ModInt64>::getMax()))
				result = false;
			else if (d < static_cast<double>(Os::Limits<ModInt64>::getMin()))
			{
				value_ = Os::Limits<ModInt64>::getMin();
			}
			else
			{
				value_ = static_cast<ModInt64>(d) + 1;
			}
			break;
		case MatchType::LessThan:
			if (d < static_cast<double>(Os::Limits<ModInt64>::getMin()))
				result = false;
			else if (d > static_cast<double>(Os::Limits<ModInt64>::getMax()))
			{
				value_ = Os::Limits<ModInt64>::getMax();
				eMatch_ = MatchType::LessThanEquals;
			}
			else
			{
				value_ = static_cast<ModInt64>(d) + 1;
			}
			break;
		case MatchType::LessThanEquals:
			if (d < static_cast<double>(Os::Limits<ModInt64>::getMin()))
				result = false;
			else if (d > static_cast<double>(Os::Limits<ModInt64>::getMax()))
			{
				value_ = Os::Limits<ModInt64>::getMax();
			}
			else
			{
				value_ = static_cast<ModInt64>(d);
			}
			break;
		}
	}
	else
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	return result;
}

//
//	FUNCTION public
//	Bitmap::Data::Data -- コンストラクタ
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
	: m_Size(0)
{
}

//
//	FUNCTION public
//	Bitmap::Data::~Data -- デストラクタ
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
//	FUNCTION public
//	Bitmap::Data::setType -- 型を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<Bitmap::Data::Type::Value>& vecType_
//		型の配列
//	ModSize size_
//		すべてのフィールドが固定長の場合、そのサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::setType(const ModVector<Type::Value>& vecType_,
			  ModSize size_)
{
	m_vecType = vecType_;
	m_Size = size_;
}

//
//	FUNCTION public
//	Bitmap::Data::setType -- 型を設定する
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::Data::Type::Value eType_
//		型の配列
//	ModSize size_
//		すべてのフィールドが固定長の場合、そのサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::setType(Type::Value eType_,
			  ModSize size_)
{
	m_vecType.pushBack(eType_);
	m_Size = size_;
}

//
//	FUNCTION public
//	Bitmap::Data::getSize -- サイズを得る
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
Data::getSize(const ModUInt32* p) const
{
	; _SYDNEY_ASSERT(m_vecType.getSize() > 0);

	ModSize size = 0;
	ModVector<Data::Type::Value>::ConstIterator i = m_vecType.begin();
	for (; i != m_vecType.end(); ++i)
	{
		size += Data::getSize(p, *i);
	}

	return size;
}

//
//	FUNCTION public
//	Bitmap::Data::getSize -- サイズを得る
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
Data::getSize(const Common::Data& cData_) const
{
	; _SYDNEY_ASSERT(m_vecType.getSize() > 0);
	
	if (cData_.isNull())
	{
		ModUnicodeOstrStream stream;
		stream << "field(0)";
		_SYDNEY_THROW1(Exception::NullabilityViolation, stream.getString());
	}
	return Data::getSize(cData_, m_vecType[0]);
}

//
//	FUNCTION public static
//	Bitmap::Data::getSize -- 1つのサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//	Bitmap::Data::Type::Value eType_
//		データ型
//
//	RETURN
//	ModSize
//		1つのサイズ
//
//	EXCEPTIONS
//
ModSize
Data::getSize(const Common::Data& cData_, Type::Value eType_)
{
	ModSize size = 0;
	
	switch (eType_)
	{
	case Type::Integer:
		size = Data::Integer::getSize(cData_);
		break;
	case Type::UnsignedInteger:
		size = Data::UnsignedInteger::getSize(cData_);
		break;
#ifdef OBSOLETE
	case Type::Float:
		size = Data::Float::getSize(cData_);
		break;
#endif
	case Type::Double:
		size = Data::Double::getSize(cData_);
		break;
	case Type::Decimal:
		size = Data::Decimal::getSize(cData_);
		break;
	case Type::CharString:
	case Type::NoPadCharString:
		size = Data::CharString::getSize(cData_);
		break;
	case Type::UnicodeString:
	case Type::NoPadUnicodeString:
		size = Data::UnicodeString::getSize(cData_);
		break;
#ifdef OBSOLETE
	case Type::Date:
		size = Data::Date::getSize(cData_);
		break;
#endif
	case Type::DateTime:
		size = Data::DateTime::getSize(cData_);
		break;
	case Type::ObjectID:
		size = Data::ObjectID::getSize(cData_);
		break;
	case Type::LanguageSet:
		size = Data::LanguageSet::getSize(cData_);
		break;
	case Type::Integer64:
		size = Data::Integer64::getSize(cData_);
		break;
	default:
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	return size;
}

//
//	FUNCTION public static
//	Bitmap::Data::getSize -- 1つのサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32*& p
//		メモリ
//	Bitmap::Data::Type::Value eType_
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
		size = Data::Integer::getSize(p);
		break;
	case Type::UnsignedInteger:
		size = Data::UnsignedInteger::getSize(p);
		break;
#ifdef OBSOLETE
	case Type::Float:
		size = Data::Float::getSize(p);
		break;
#endif
	case Type::Double:
		size = Data::Double::getSize(p);
		break;
	case Type::Decimal:
		size = Data::Decimal::getSize(p);
		break;
	case Type::CharString:
	case Type::NoPadCharString:
		size = Data::CharString::getSize(p);
		break;
	case Type::UnicodeString:
	case Type::NoPadUnicodeString:
		size = Data::UnicodeString::getSize(p);
		break;
#ifdef OBSOLETE
	case Type::Date:
		size = Data::Date::getSize(p);
		break;
#endif
	case Type::DateTime:
		size = Data::DateTime::getSize(p);
		break;
	case Type::ObjectID:
		size = Data::ObjectID::getSize(p);
		break;
	case Type::LanguageSet:
		size = Data::LanguageSet::getSize(p);
		break;
	case Type::Integer64:
		size = Data::Integer64::getSize(p);
		break;
	default:
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	p += size;
	return size;
}

//
//	FUNCTION public
//	Bitmap::Data::dump -- ダンプする
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32* p
//		メモリ
//	const Common::Data& cKey_
//		キー
//	const Common::Data& cValue_
//		バリュー
//
//	RETURN
//	ModSize
//		サイズ
//
//	EXCEPTIONS
//
ModSize
Data::dump(ModUInt32* p,
		   const Common::Data& cKey_,
		   const Common::Data& cValue_) const
{
	; _SYDNEY_ASSERT(m_vecType.getSize() > 1);

	ModUInt32* s = p;
	Data::dump(p, cKey_, m_vecType[0]);
	Data::dump(p, cValue_, m_vecType[1]);
	return static_cast<ModSize>(p - s);
}

//
//	FUNCTION public
//	Bitmap::Data::dump -- ダンプする
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32* p
//		メモリ
//	const Common::Data& cKey_
//	   	キー
//
//	RETURN
//	ModSize
//		サイズ
//
//	EXCEPTIONS
//
ModSize
Data::dump(ModUInt32* p, const Common::Data& cKey_) const
{
	; _SYDNEY_ASSERT(m_vecType.getSize() > 0);

	ModUInt32* s = p;
	Data::dump(p, cKey_, m_vecType[0]);
	return static_cast<ModSize>(p - s);
}

//
//	FUNCTION public static
//	Bitmap::Data::dump -- 1つのダンプする
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32*& p
//		メモリ
//	const Common::Data& cData_
//		データ
//	Bitmap::Data::Type::Value eType_
//		データ型
//
//	RETURN
//	ModSize
//		1つのサイズ
//
//	EXCEPTIONS
//
void
Data::dump(ModUInt32*& p, const Common::Data& cData_, Type::Value eType_)
{
	switch (eType_)
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
#ifdef OBSOLETE
	case Type::Float:
		{
			Data::Float* d = syd_reinterpret_cast<Data::Float*>(p);
			p = d->dump(cData_);
		}
		break;
#endif
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
	case Type::CharString:
	case Type::NoPadCharString:
		{
			Data::CharString* d = syd_reinterpret_cast<Data::CharString*>(p);
			p = d->dump(cData_);
		}
		break;
	case Type::UnicodeString:
	case Type::NoPadUnicodeString:
		{
			Data::UnicodeString* d = syd_reinterpret_cast<Data::UnicodeString*>(p);
			p = d->dump(cData_);
		}
		break;
#ifdef OBSOLETE
	case Type::Date:
		{
			Data::Date* d = syd_reinterpret_cast<Data::Date*>(p);
			p = d->dump(cData_);
		}
		break;
#endif
	case Type::DateTime:
		{
			Data::DateTime* d = syd_reinterpret_cast<Data::DateTime*>(p);
			p = d->dump(cData_);
		}
		break;
	case Type::ObjectID:
		{
			Data::ObjectID* d = syd_reinterpret_cast<Data::ObjectID*>(p);
			p = d->dump(cData_);
		}
		break;
	case Type::LanguageSet:
		{
			Data::LanguageSet* d = syd_reinterpret_cast<Data::LanguageSet*>(p);
			p = d->dump(cData_);
		}
		break;
	case Type::Integer64:
		{
			Data::Integer64* d = syd_reinterpret_cast<Data::Integer64*>(p);
			p = d->dump(cData_);
		}
		break;
	default:
		_SYDNEY_THROW0(Exception::NotSupported);
	}
}

//
//	FUNCTION public
//	Bitmap::Data::getPageID -- ページIDを得る
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
PhysicalFile::PageID
Data::getPageID(const ModUInt32* p) const
{
	; _SYDNEY_ASSERT(m_vecType.getSize() > 0);
	
	PhysicalFile::PageID uiPageID = PhysicalFile::ConstValue::UndefinedPageID;
	ModVector<Data::Type::Value>::ConstIterator i = m_vecType.begin();
	for (; i != (m_vecType.end() - 1); ++i)
	{
		Data::getSize(p, *i);
	}
	if (*i == Type::UnsignedInteger)
	{
		const Data::UnsignedInteger* id
			= syd_reinterpret_cast<const Data::UnsignedInteger*>(p);
		uiPageID = id->m_value;
	}
	return uiPageID;
}

//
//	FUNCTION public
//	Bitmap::Data::getValue -- バリューを得る
//
//	NOTES
//	最後の要素をバリューだと思って取り出す
//
//	ARGUMENTS
//	const ModUInt32* p
//		メモリ
//	Common::Data& cData_
//		バリュー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::getValue(const ModUInt32* p, Common::Data& cData_) const
{
	; _SYDNEY_ASSERT(m_vecType.getSize() > 0);
	
	ModVector<Data::Type::Value>::ConstIterator i = m_vecType.begin();
	for (; i != (m_vecType.end() - 1); ++i)
	{
		Data::getSize(p, *i);
	}
	getData(p, *i, cData_);
}

//
//	FUNCTION public
//	Bitmap::Data::getKeyAndValue -- キーとバリューを得る
//
//	NOTES
//	先頭の要素がキーで、次の要素がバリューだと思って取り出す
//
//	ARGUMENTS
//	const ModUInt32* p
//		メモリ
//	Common::Data& cKey_
//		キー
//	Common::Data& cValue_
//		バリュー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::getKeyAndValue(const ModUInt32* p,
					 Common::Data& cKey_, Common::Data& cValue_) const
{
	; _SYDNEY_ASSERT(m_vecType.getSize() > 1);

	getData(p, m_vecType[0], cKey_);
	getData(p, m_vecType[1], cValue_);
}

//
//	FUNCTION public static
//	Bitmap::Data::getFieldType -- フィールドの型を得る
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cFileID_
//		ファイルID
//	int iPosition_
//		フィールド位置
//	bool isVersion2_
//		バージョン番号が2以降か
//	bool& isFixed_
//		固定長かどうか
//	bool& isArray_
//		配列かどうか
//
//	RETURN
//	Bitmap::Data::Type::Value
//		フィールドの型
//
//	EXCEPTIONS
//
Data::Type::Value
Data::getFieldType(const LogicalFile::FileID& cFileID_,
				   int iPosition_,  bool isVersion2_,
				   bool& isFixed_, bool& isArray_)
{
	isFixed_ = false;
	isArray_ = false;
	Data::Type::Value result;
	Common::DataType::Type eType
		= static_cast<Common::DataType::Type>(
			cFileID_.getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
				FileCommon::FileOption::FieldType::Key, iPosition_)));
	if (eType == Common::DataType::Array)
	{
		// 配列である
		isArray_ = true;
		// 要素の型を得る
		eType = static_cast<Common::DataType::Type>(
			cFileID_.getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
						FileCommon::FileOption::ElementType::Key, iPosition_)));
	}
	
	switch (eType)
	{
	case Common::DataType::Integer:
		result = Data::Type::Integer;
		isFixed_ = true;
		break;
	case Common::DataType::UnsignedInteger:
		result = Data::Type::UnsignedInteger;
		isFixed_ = true;
		break;
	case Common::DataType::Integer64:
		result = Data::Type::Integer64;
		isFixed_ = true;
		break;
	case Common::DataType::String:
		{
			result = Data::Type::UnicodeString;
			int tmp;
			if (cFileID_.getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
				isArray_ ? FileCommon::FileOption::ElementEncodingForm::Key :
				FileCommon::FileOption::FieldEncodingForm::Key, iPosition_),
									tmp) == true)
			{
				if (tmp == Common::StringData::EncodingForm::UTF8)
					result = Data::Type::CharString;
			}
			bool tmp0;
			if (cFileID_.getBoolean(
					_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
						isArray_ ? FileCommon::FileOption::ElementFixed::Key :
						FileCommon::FileOption::FieldFixed::Key,
						iPosition_), tmp0) == true)
			{
				if (tmp0 == true) isFixed_ = true;
			}
			
			bool isNontruncate = false;
			if (isVersion2_ == false)
			{
				// BACKWARD COMPATIBILITY

				if (isFixed_ == false)
					// The variable field of v2 is sorted with PAD SPACE.
					// But, the field of v1 is sorted with NO PAD.
					// NonTruncate is available for the flag which means
					// the field is sorted with NO PAD.
					isNontruncate = true;
			}
			else
			{
				int eCollation = 0;
				if (cFileID_.getInteger(
						_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
							FileCommon::FileOption::FieldCollation::Key,
							iPosition_),
						eCollation))
				{
					if (eCollation == Common::Collation::Type::NoPad)
						isNontruncate = true;
				}
			}
			if (isNontruncate == true)
			{
				if (result == Data::Type::UnicodeString)
				{
					result = Data::Type::NoPadUnicodeString;
				}
				else
				{
					; _SYDNEY_ASSERT(result == Data::Type::CharString);
					result = Data::Type::NoPadCharString;
				}
			}
		}
		break;
#ifdef OBSOLETE
	case Common::DataType::Float:
		isFixed_ = true;
		result = Data::Type::Float;
		break;
#endif
	case Common::DataType::Double:
		isFixed_ = true;
		result = Data::Type::Double;
		break;
	case Common::DataType::Decimal:
		isFixed_ = true;
		result = Data::Type::Decimal;
		break;
#ifdef OBSOLETE
	case Common::DataType::Date:
		isFixed_ = true;
		result = Data::Type::Date;
		break;
#endif
	case Common::DataType::DateTime:
		isFixed_ = true;
		result = Data::Type::DateTime;
		break;
	case Common::DataType::ObjectID:
		isFixed_ = true;
		result = Data::Type::ObjectID;
		break;
	case Common::DataType::Language:
		result = Data::Type::LanguageSet;
		break;
	default:
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	return result;
}

//
//	FUNCTION public static
//	Bitmap::Data::getFieldSize -- フィールドのサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cFileID_
//		ファイルID
//	int iPosition_
//		フィールド位置
//	Bitmap::Data::Type::Value eType_
//		データ型
//	bool isArray_
//		配列かどうか
//	int& iPrecision_
//		Decimal型の場合、そのprecision
//	int& iScale_
//		Decimal型の場合、そのscale
//
//	RETURN
//	ModSize
//		フィールドのユニットサイズ
//
//	EXCEPTIONS
//
ModSize
Data::getFieldSize(const LogicalFile::FileID& cFileID_,
				   int iPosition_, Data::Type::Value eType_, bool isArray_,
				   int& iPrecision_, int& iScale_)
{
	ModSize result;
	ModUInt32* p = 0;
	ModSize length = 0;

	switch (eType_)
	{
	case Data::Type::CharString:
	case Data::Type::UnicodeString:
	case Data::Type::NoPadCharString:
	case Data::Type::NoPadUnicodeString:
		// 可変長
		length = cFileID_.getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
			isArray_ ? FileCommon::FileOption::ElementLength::Key :
			FileCommon::FileOption::FieldLength::Key, iPosition_));
		if (length == 0)
			_SYDNEY_THROW0(Exception::NotSupported);
		break;
	case Data::Type::Decimal:
		iPrecision_ = cFileID_.getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
			isArray_ ? FileCommon::FileOption::ElementLength::Key :
			FileCommon::FileOption::FieldLength::Key, iPosition_));
		iScale_ = cFileID_.getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
			FileCommon::FileOption::FieldFraction::Key, iPosition_));

		length = Common::DecimalData::getDumpSizeBy(iPrecision_, iScale_);
		if (length == 0)
			_SYDNEY_THROW0(Exception::NotSupported);
		break;
	default:
		;
	}

	switch (eType_)
	{
	case Data::Type::Integer:
		result = Data::Integer::getSize(p);
		break;
	case Data::Type::UnsignedInteger:
		result = Data::UnsignedInteger::getSize(p);
		break;
#ifdef OBSOLETE
	case Data::Type::Float:
		result = Data::Float::getSize(p);
		break;
#endif
	case Data::Type::Double:
		result = Data::Double::getSize(p);
		break;
	case Data::Type::Decimal:
		result = Data::Decimal::calcUnitSize(length);
		break;
	case Data::Type::CharString:
	case Data::Type::NoPadCharString:
		result = Data::CharString::calcUnitSize(
			static_cast<unsigned short>(length));
		break;
	case Data::Type::UnicodeString:
	case Data::Type::NoPadUnicodeString:
		result = Data::UnicodeString::calcUnitSize(
			static_cast<unsigned short>(length / sizeof(ModUnicodeChar)));
		break;
#ifdef OBSOLETE
	case Data::Type::Date:
		result = Data::Date::getSize(p);
		break;
#endif
	case Data::Type::DateTime:
		result = Data::DateTime::getSize(p);
		break;
	case Data::Type::ObjectID:
		result = Data::ObjectID::getSize(p);
		break;
	case Data::Type::LanguageSet:
		result = Data::LanguageSet::calcUnitSize(3);	// 3にしとく
		break;
	case Data::Type::Integer64:
		result = Data::Integer64::getSize(p);
		break;
	default:
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	return result;
}

//
//	FUNCTION public static
//	Bitmap::Data::makeData -- タイプからCommon::Dataを得る
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::Data::Type::Value eType_
//		データ型
//
//	RETURN
//	Common::Data::Pointer
//		Common::Dataへのポインタ
//
//	EXCEPTIONS
//
Common::Data::Pointer
Data::makeData(Type::Value eType_, int precision_, int scale_)
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
#ifdef OBSOLETE
	case Type::Float:
		p = new Common::FloatData;
		break;
#endif
	case Type::Double:
		p = new Common::DoubleData;
		break;
	case Type::Decimal:
        p = new Common::DecimalData(precision_, scale_);
		break;
	case Type::CharString:
	case Type::NoPadCharString:
		p = new Common::StringData(Common::StringData::EncodingForm::UTF8);
		break;
	case Type::UnicodeString:
	case Type::NoPadUnicodeString:
		p = new Common::StringData(Common::StringData::EncodingForm::UCS2);
		break;
#ifdef OBSOLETE
	case Type::Date:
		p = new Common::DateData;
		break;
#endif
	case Type::DateTime:
		p = new Common::DateTimeData;
		break;
	case Type::ObjectID:
		p = new Common::ObjectIDData;
		break;
	case Type::LanguageSet:
		p = new Common::LanguageData;
		break;
	case Type::Integer64:
		p = new Common::Integer64Data;
		break;
	default:
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	return p;
}

//
//	FUNCTION private
//	Bitmap::Data::getData -- 1つデータを得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32*& p
//		メモリ
//	Bitmap::Data::Type::Value eType_
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
			  Common::Data& cData_) const
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
#ifdef OBSOLETE
	case Type::Float:
		{
			const Data::Float* d
				= syd_reinterpret_cast<const Data::Float*>(p);
			d->getData(cData_);
			p += d->getSize();
		}
		break;
#endif
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
	case Type::CharString:
	case Type::NoPadCharString:
		{
			const Data::CharString* d
				= syd_reinterpret_cast<const Data::CharString*>(p);
			d->getData(cData_);
			p += d->getSize();
		}
		break;
	case Type::UnicodeString:
	case Type::NoPadUnicodeString:
		{
			const Data::UnicodeString* d
				= syd_reinterpret_cast<const Data::UnicodeString*>(p);
			d->getData(cData_);
			p += d->getSize();
		}
		break;
#ifdef OBSOLETE
	case Type::Date:
		{
			const Data::Date* d
				= syd_reinterpret_cast<const Data::Date*>(p);
			d->getData(cData_);
			p += d->getSize();
		}
		break;
#endif
	case Type::DateTime:
		{
			const Data::DateTime* d
				= syd_reinterpret_cast<const Data::DateTime*>(p);
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
	case Type::LanguageSet:
		{
			const Data::LanguageSet* d
				= syd_reinterpret_cast<const Data::LanguageSet*>(p);
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
	default:
		_SYDNEY_THROW0(Exception::NotSupported);
	}
}

//
//	Copyright (c) 2005, 2006, 2007, 2009, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
