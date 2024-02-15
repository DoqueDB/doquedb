// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Data.cpp --
// 
// Copyright (c) 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Array";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Array/Data.h"

#include "Common/Assert.h"
#include "Common/BasicString.h"
#include "Common/BitSet.h"
#include "Common/DataArrayData.h"
#include "Common/DateTimeData.h"
#include "Common/DecimalData.h"
#include "Common/DoubleData.h"
#include "Common/Integer64Data.h"
#include "Common/IntegerData.h"
#include "Common/LanguageData.h"
#include "Common/NullData.h"
#include "Common/ObjectIDData.h"
#include "Common/StringData.h"
#include "Common/UnsignedIntegerData.h"
#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"
#include "Exception/NullabilityViolation.h"
#include "FileCommon/FileOption.h"
#include "Os/Limits.h"
#include "Os/Memory.h"

#include "ModOsDriver.h"
#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_ARRAY_USING

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
//	Array::Data::Integer::getSize -- サイズを得る
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
//	Array::Data::Integer::dump -- ダンプする
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
//	Array::Data::Integer::getData -- 取り出す
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
//	Array::Data::Integer::round -- 丸める
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		入力
//	LogicalFile::TreeNodeInterface::Type& eMatch_
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
					 LogicalFile::TreeNodeInterface::Type& eMatch_,
					 int& value_)
{
	using namespace LogicalFile;

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
		case TreeNodeInterface::Equals:
			result = false;
			break;
		case TreeNodeInterface::GreaterThan:
			if (d > static_cast<double>(Os::Limits<int>::getMax()))
				result = false;
			else if (d < static_cast<double>(Os::Limits<int>::getMin()))
			{
				value_ = Os::Limits<int>::getMin();
				eMatch_ = TreeNodeInterface::GreaterThanEquals;
			}
			else
			{
				value_ = static_cast<int>(d);
			}
			break;
		case TreeNodeInterface::GreaterThanEquals:
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
		case TreeNodeInterface::LessThan:
			if (d < static_cast<double>(Os::Limits<int>::getMin()))
				result = false;
			else if (d > static_cast<double>(Os::Limits<int>::getMax()))
			{
				value_ = Os::Limits<int>::getMax();
				eMatch_ = TreeNodeInterface::LessThanEquals;
			}
			else
			{
				value_ = static_cast<int>(d) + 1;
			}
			break;
		case TreeNodeInterface::LessThanEquals:
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
		case TreeNodeInterface::Equals:
			result = false;
			break;
		case TreeNodeInterface::GreaterThan:
			if (d > static_cast<ModInt64>(Os::Limits<int>::getMax()))
				result = false;
			else
			{
				value_ = Os::Limits<int>::getMin();
				eMatch_ = TreeNodeInterface::GreaterThanEquals;
			}
			break;
		case TreeNodeInterface::GreaterThanEquals:
			if (d > static_cast<ModInt64>(Os::Limits<int>::getMax()))
				result = false;
			else
			{
				value_ = Os::Limits<int>::getMin();
			}
			break;
		case TreeNodeInterface::LessThan:
			if (d < static_cast<ModInt64>(Os::Limits<int>::getMin()))
				result = false;
			else
			{
				value_ = Os::Limits<int>::getMax();
				eMatch_ = TreeNodeInterface::LessThanEquals;
			}
			break;
		case TreeNodeInterface::LessThanEquals:
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
//	Array::Data::UnsignedInteger::getSize -- サイズを得る
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
//	Array::Data::UnsignedInteger::dump -- ダンプする
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
//	Array::Data::UnsignedInteger::getData -- 取り出す
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

//
//	FUNCTION public static
//	Array::Data::Double::getSize -- サイズを得る
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
//	Array::Data::Double::dump -- ダンプする
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
//	Array::Data::Double::getData -- 取り出す
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
//	Array::Data::Decimal::getSize -- サイズを得る
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
//	Array::Data::Decimal::dump -- ダンプする
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
//	Array::Data::Decimal::compare -- 
//
//	NOTES
//
//	ARGUMENTS
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
//	Array::Data::Decimal::getData -- 
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
		.setDumpedValue(syd_reinterpret_cast<const char*>(&m_value[0]));
}

//
//	FUNCTION public
//	Array::Data::Decimal::round -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool 
Data::Decimal::round(const Common::Data& cData_,
				  LogicalFile::TreeNodeInterface::Type& eMatch_,
				  ModSize iPrecision_, ModSize iScale_, 
				  ModUnicodeString& cstrValue_)
{
	using namespace LogicalFile;

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
	case TreeNodeInterface::Equals:
		result = false;
		break;
	case TreeNodeInterface::NotEquals:
		cstrValue_ = ddMinValue.getString();
		eMatch_ = TreeNodeInterface::GreaterThanEquals;
		break;
	case TreeNodeInterface::GreaterThan:
		if ((tNear == isBiggerMax) || bMax)
			result = false;
		else if (tNear == isLessMin)
		{
			cstrValue_ = ddValue.getString();
			eMatch_ = TreeNodeInterface::GreaterThanEquals;
		}
		else
		{
			cstrValue_ = ddValue.getString();
			if (isNegative)
				eMatch_ = TreeNodeInterface::GreaterThanEquals;
		}

		break;
	case TreeNodeInterface::GreaterThanEquals:
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
				eMatch_ = TreeNodeInterface::GreaterThan;
			}
		}		
		break;
	case TreeNodeInterface::LessThan:
		if (tNear == isLessMin)
			result = false;
		else if ((tNear == isBiggerMax) && (!bMax))
		{
			cstrValue_ = ddValue.getString();
			eMatch_ = TreeNodeInterface::LessThanEquals;
		}
		else if (bMax)
			cstrValue_ = ddValue.getString();
		else
		{
			cstrValue_ = ddValue.getString();
			if (!isNegative)
				eMatch_ = TreeNodeInterface::LessThanEquals; 
		}		
		break;
	case TreeNodeInterface::LessThanEquals:
		if (tNear == isLessMin)
			result = false;
		else if ((tNear == isBiggerMax) || (bMax))
			cstrValue_ = ddValue.getString();
		else if ((tNear == isMiddle) && (!bMax))
		{
			cstrValue_ = ddValue.getString();
			if (isNegative)
				eMatch_ = TreeNodeInterface::LessThan;			
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
//	Array::Data::Decimal::getParameter -- 
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
//	Array::Data::CharString::getSize -- サイズを得る
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
	return calcUnitSize(static_cast<unsigned short>(d.getValue().getLength()));
}

//
//	FUNCTION public
//	Array::Data::CharString::compare -- 比較関数
//
//	NOTES
//
//	ARGUMENTS
//	const Array::Data::CharString& cOther_
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
//	Array::Data::CharString::dump -- ダンプする
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
	return syd_reinterpret_cast<ModUInt32*>(this) + getSize();
}

//
//	FUNCTION public
//	Array::Data::CharString::getData -- 取り出す
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
//	Array::Data::UnicodeString::getSize -- サイズを得る
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
//	Array::Data::UnicodeString::compare -- 比較関数
//
//	NOTES
//
//	ARGUMENTS
//	const Array::Data::UnicodeString& cOther_
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
//	Array::Data::UnicodeString::dump -- ダンプする
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
//	Array::Data::UnicodeString::getData -- 取り出す
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
//	Array::Data::NoPadCharString::compare -- 比較関数
//
//	NOTES
//
//	ARGUMENTS
//	const Array::Data::NoPadCharString& cOther_
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
//	Array::Data::NoPadCharString::like --
//
//	NOTES
//
//	ARGUMENTS
//	const Array::Data::NoPadCharString& cOther_
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
//	Array::Data::NoPadUnicodeString::compare -- 比較関数
//
//	NOTES
//
//	ARGUMENTS
//	const Array::Data::NoPadUnicodeString& cOther_
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
//	Array::Data::NoPadUnicodeString::like --
//
//	NOTES
//
//	ARGUMENTS
//	const Array::Data::NoPadUnicodeString& cOther_
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

//
//	FUNCTION public static
//	Array::Data::DateTime::getSize -- サイズを得る
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
//	Array::Data::DateTime::dump -- ダンプする
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
//	Array::Data::DateTime::getData -- 取り出す
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
//	Array::Data::ObjectID::getSize -- サイズを得る
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
//	Array::Data::ObjectID::dump -- ダンプする
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
//	Array::Data::ObjectID::getData -- 取り出す
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
//	Array::Data::LanguageSet::getSize -- サイズを得る
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
//	Array::Data::LanguageSet::compare -- 比較関数
//
//	NOTES
//
//	ARGUMENTS
//	const Array::Data::LanguageSet& cOther_
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
//	Array::Data::LanguageSet::dump -- ダンプする
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
//	Array::Data::LanguageSet::getData -- 取り出す
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
//	Array::Data::Integer64::getSize -- サイズを得る
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
//	Array::Data::Integer64::dump -- ダンプする
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
//	Array::Data::Integer64::getData -- 取り出す
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
//	Array::Data::Integer64::round -- 丸める
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		入力
//	LogicalFile::TreeNodeInterface::Type& eMatch_
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
					   LogicalFile::TreeNodeInterface::Type& eMatch_,
					   ModInt64& value_)
{
	using namespace LogicalFile;

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
		case TreeNodeInterface::Equals:
			result = false;
			break;
		case TreeNodeInterface::GreaterThan:
			if (d > static_cast<double>(Os::Limits<ModInt64>::getMax()))
				result = false;
			else if (d < static_cast<double>(Os::Limits<ModInt64>::getMin()))
			{
				value_ = Os::Limits<ModInt64>::getMin();
				eMatch_ = TreeNodeInterface::GreaterThanEquals;
			}
			else
			{
				value_ = static_cast<ModInt64>(d);
			}
			break;
		case TreeNodeInterface::GreaterThanEquals:
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
		case TreeNodeInterface::LessThan:
			if (d < static_cast<double>(Os::Limits<ModInt64>::getMin()))
				result = false;
			else if (d > static_cast<double>(Os::Limits<ModInt64>::getMax()))
			{
				value_ = Os::Limits<ModInt64>::getMax();
				eMatch_ = TreeNodeInterface::LessThanEquals;
			}
			else
			{
				value_ = static_cast<ModInt64>(d) + 1;
			}
			break;
		case TreeNodeInterface::LessThanEquals:
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
//	Array::Data::Data -- コンストラクタ
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
	: m_iFieldCount(0), m_uiFixedSize(0), m_uiVariableSize(0)
{
}

//
//	FUNCTION public
//	Array::Data::~Data -- デストラクタ
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
//	Array::Data::setTypeCount --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::setTypeCount(int iFieldCount_)
{
	; _SYDNEY_ASSERT(iFieldCount_ > 0 && iFieldCount_ <= MaxFieldCount);
	m_iFieldCount = iFieldCount_;
}

//
//	FUNCTION public
//	Array::Data::setType -- 型を設定する
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::setType(Data::Type::Value eFieldType_, int iFieldPosition_)
{
	; _SYDNEY_ASSERT(m_iFieldCount != 0);
	; _SYDNEY_ASSERT(iFieldPosition_ >= 0 && iFieldPosition_ < m_iFieldCount);
	
	m_cFieldTypes[iFieldPosition_] = eFieldType_;
}

// 
//	FUNCTION public
//	Array::Data::getType -- Get the type of data
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Data::Type::Value
Data::getType(int iFieldPosition_) const
{
	; _SYDNEY_ASSERT(m_iFieldCount != 0);
	; _SYDNEY_ASSERT(iFieldPosition_ >= 0 && iFieldPosition_ < m_iFieldCount);
	
	return m_cFieldTypes[iFieldPosition_];
}

//
//	FUNCTION public
//	Array::Data::setSize --
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiFixedSize_ [unit size]
//	ModSize uiVariableSize_ [unit size]
//
//	RETURN
//
//	EXCEPTIONS
//
void
Data::setSize(ModSize uiFixedSize_, ModSize uiVariableSize_)
{
	; _SYDNEY_ASSERT(uiFixedSize_ > 0 && uiVariableSize_ >= 0);
	m_uiFixedSize = uiFixedSize_;
	m_uiVariableSize = uiVariableSize_;
}

//
//	FUNCTION public
//	Array::Data::getSize -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* p
//		メモリ
//
//	RETURN
//	ModSize
//		ユニットサイズ
//
//	EXCEPTIONS
//
ModSize
Data::getSize(const ModUInt32* p) const
{
//	; _SYDNEY_ASSERT(m_iFieldCount < Tree::MaxFieldCount);

	ModSize size = 0;
	for (int i = 0; i < m_iFieldCount; ++i)
	{
		size += getSize(p, m_cFieldTypes[i]);
	}

	return size;
}

//
//	FUNCTION public
//	Array::Data::getSize -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cData_
//		データ
//
//	RETURN
//	ModSize
//		ユニットサイズ
//
//	EXCEPTIONS
//
ModSize
Data::getSize(const Common::DataArrayData& cData_) const
{
	ModSize size = 0;
	int n = 0;
	for (int i = 0; i < m_iFieldCount; ++i, ++n)
	{
		const Common::Data& c = *cData_.getElement(n).get();
		if (c.isNull()) {
			ModUnicodeOstrStream stream;
			stream << "field(" << n << ")";
			_SYDNEY_THROW1(Exception::NullabilityViolation, stream.getString());
		}
		size += getSize(c, m_cFieldTypes[i]);
	}
	return size;
}

//
//	FUNCTION public
//	Array::Data::getSize -- １つのサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
// 	int n_
//		何番目の要素か
//
//	RETURN
//	ModSize
//		ユニットサイズ
//
//	EXCEPTIONS
//
ModSize
Data::getSize(const Common::Data& cData_, int n_) const
{
	; _SYDNEY_ASSERT(n_ < m_iFieldCount);
	
	ModSize size = 0;
	if (!cData_.isNull())
	{
		size = getSize(cData_, m_cFieldTypes[n_]);
	}
	return size;
}

//
//	FUNCTION public
//	Array::Data::dump -- ダンプする
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32* p
//		メモリ
//	const Common::DataArrayData& cData_
//		データ
//
//	RETURN
//	ModSize
//		ユニットサイズ
//
//	EXCEPTIONS
//
ModSize
Data::dump(ModUInt32* p, const Common::DataArrayData& cData_) const
{
	; _SYDNEY_ASSERT(cData_.getCount() == m_iFieldCount);

	ModUInt32* s = p;
	for (int i = 0; i < m_iFieldCount; ++i)
	{
		dump(p, *cData_.getElement(i).get(), m_cFieldTypes[i]);
	}
	return static_cast<ModSize>(p - s);
}

//
//	FUNCTION public
//	Array::Data::dump -- １つダンプする
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32* p
//		メモリ
//	const Common::Data& cData_
//		データ
// 	int n_
//		何番目の要素か
//
//	RETURN
//	ModSize
//		ユニットサイズ
//
//	EXCEPTIONS
//
ModSize
Data::dump(ModUInt32* p, const Common::Data& cData_, int n_) const
{
	; _SYDNEY_ASSERT(n_ < m_iFieldCount);
	; _SYDNEY_ASSERT(cData_.isNull() == false);

	ModUInt32* s = p;
	dump(p, cData_, m_cFieldTypes[n_]);
	return static_cast<ModSize>(p - s);
}

//
//	FUNCTION public
//	Array::Data::getOneData -- データを得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* p
//	Common::Data& cData_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::getData(const ModUInt32* p,
			  int iFieldPosition_,
			  Common::Data::Pointer pData_) const
{
	; _SYDNEY_ASSERT(iFieldPosition_ >= 0);
	; _SYDNEY_ASSERT(iFieldPosition_ < m_iFieldCount);

	for (int i = 0; i < m_iFieldCount; ++i)
	{
		if (i == iFieldPosition_)
		{
			getData(p, m_cFieldTypes[i], *pData_);
		}
		else
		{
			getSize(p, m_cFieldTypes[i]);
		}
	}
}

//
//	FUNCTION public
//	Array::Data::getModUInt32Data --
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* p
//		メモリ
//
//	RETURN
//	ModUInt32
//
//	EXCEPTIONS
//
ModUInt32
Data::getModUInt32Data(const ModUInt32* p, int iFieldPosition_) const
{
	; _SYDNEY_ASSERT(iFieldPosition_ >= 0);
	; _SYDNEY_ASSERT(iFieldPosition_ < m_iFieldCount);
	; _SYDNEY_ASSERT(m_cFieldTypes[iFieldPosition_] == Type::UnsignedInteger);
	
	ModUInt32 uiData = ModUInt32Max;
	
	for (int i = 0; i < m_iFieldCount; ++i)
	{
		if (i == iFieldPosition_)
		{
			const Data::UnsignedInteger* d
				= syd_reinterpret_cast<const Data::UnsignedInteger*>(p);
			uiData = d->m_value;
		}
		getSize(p, m_cFieldTypes[i]);
	}
	return uiData;
}

//
//	FUNCTION public
//	Array::Data::getModUInt32Data --
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* p
//		メモリ
//	Common::DataArrayData& cTuple_
//		The ModUInt32 data is set at the top of the cTuple_.
//
//	RETURN
// 	ModUInt32
//
//	EXCEPTIONS
//
ModUInt32
Data::getModUInt32Data(const ModUInt32* p,
					   int iFieldPosition_,
					   Common::DataArrayData& cTuple_) const
{
	; _SYDNEY_ASSERT(iFieldPosition_ >= 0);
	; _SYDNEY_ASSERT(iFieldPosition_ < m_iFieldCount);
	; _SYDNEY_ASSERT(m_cFieldTypes[iFieldPosition_] == Type::UnsignedInteger);
	
	ModUInt32 uiData = ModUInt32Max;
	
	for (int i = 0; i < m_iFieldCount; ++i)
	{
		if (i == iFieldPosition_)
		{
			const Data::UnsignedInteger* d
				= syd_reinterpret_cast<const Data::UnsignedInteger*>(p);
			Common::Data::Pointer pData = cTuple_.getElement(0);
			d->getData(*pData);
			uiData = d->m_value;
		}
		getSize(p, m_cFieldTypes[i]);
	}
	return uiData;
}

//
//	FUNCTION public
//	Array::Data::getModUInt32Data --
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* p
//		メモリ
//	Common::BitSet& cBitSet_
//		The ModUInt32 data is set in the cBitSet_.
//
//	RETURN
// 	ModUInt32
//
//	EXCEPTIONS
//
ModUInt32
Data::getModUInt32DataByBitSet(const ModUInt32* p,
							   int iFieldPosition_,
							   Common::BitSet& cBitSet_) const
{
	; _SYDNEY_ASSERT(iFieldPosition_ >= 0);
	; _SYDNEY_ASSERT(iFieldPosition_ < m_iFieldCount);
	; _SYDNEY_ASSERT(m_cFieldTypes[iFieldPosition_] == Type::UnsignedInteger);
	
	ModUInt32 uiData = ModUInt32Max;
	
	for (int i = 0; i < m_iFieldCount; ++i)
	{
		if (i == iFieldPosition_)
		{
			const Data::UnsignedInteger* d
				= syd_reinterpret_cast<const Data::UnsignedInteger*>(p);
			cBitSet_.set(d->m_value);
			uiData = d->m_value;
		}
		getSize(p, m_cFieldTypes[i]);
	}
	return uiData;
}

//
//	FUNCTION public static
//	Array::Data::getFieldType -- フィールドの型を得る
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cFileID_
//		ファイルID
//	int iPosition_
//		フィールド位置
//	bool& isFixed_
//		固定長かどうか
//
//	RETURN
//	Array::Data::Type::Value
//		フィールドの型
//
//	EXCEPTIONS
//
Data::Type::Value
Data::getFieldType(const LogicalFile::FileID& cFileID_,
				   int iPosition_, bool& isFixed_, bool& isArray_)
{
	isFixed_ = false;
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

			int encodingForm = (isArray_ == true) ?
				FileCommon::FileOption::ElementEncodingForm::Key :
				FileCommon::FileOption::FieldEncodingForm::Key;
			int tmp;
			if (cFileID_.getInteger(
					_SYDNEY_FILE_PARAMETER_FORMAT_KEY(encodingForm, iPosition_),
					tmp) == true)
			{
				if (tmp == Common::StringData::EncodingForm::UTF8)
					result = Data::Type::CharString;
			}

			int fixedKey = (isArray_ == true) ?
				FileCommon::FileOption::ElementFixed::Key :
				FileCommon::FileOption::FieldFixed::Key;
			bool tmp0;
			if (cFileID_.getBoolean(
					_SYDNEY_FILE_PARAMETER_FORMAT_KEY(fixedKey,	iPosition_),
					tmp0) == true)
			{
				if (tmp0 == true) isFixed_ = true;
			}

			int eCollation = 0;
			if (cFileID_.getInteger(
					_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
						FileCommon::FileOption::FieldCollation::Key,
						iPosition_),
					eCollation) == true)
			{
				if (eCollation == Common::Collation::Type::NoPad)
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
		}
		break;
	case Common::DataType::Double:
		isFixed_ = true;
		result = Data::Type::Double;
		break;
	case Common::DataType::Decimal:
		isFixed_ = true;
		result = Data::Type::Decimal;
		break;
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
//	Array::Data::getFieldSize -- フィールドのサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cFileID_
//		ファイルID
//	int iPosition_
//		フィールド位置
//	Array::Data::Type::Value eType_
//		データ型
//
//	RETURN
//	ModSize
//		フィールドのユニットサイズ [unit size]
//		When the field is variable, the size is maximum in the range.
//
//	EXCEPTIONS
//
ModSize
Data::getFieldSize(const LogicalFile::FileID& cFileID_,
				   int iPosition_, Data::Type::Value eType_, bool isArray_)
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
	{
		int lengthKey = (isArray_ == true) ?
			FileCommon::FileOption::ElementLength::Key :
			FileCommon::FileOption::FieldLength::Key;
		length = cFileID_.getInteger(
			_SYDNEY_FILE_PARAMETER_FORMAT_KEY(lengthKey, iPosition_));
		if (length == 0)
			_SYDNEY_THROW0(Exception::NotSupported);
		break;
	}
	case Data::Type::Decimal:
	{
		int precision;
		int scale;
		Data::Decimal::getParameter(cFileID_, iPosition_, isArray_,
									precision, scale);
		length = Common::DecimalData::getDumpSizeBy(precision, scale);
		if (length == 0)
			_SYDNEY_THROW0(Exception::NotSupported);
		break;
	}
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
//	Array::Data::getSize -- 1つのサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32*& p
//		メモリ
//	Array::Data::Type::Value eType_
//		データ型
//
//	RETURN
//	ModSize
//		1つのデータのユニットサイズ
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
//	FUNCTION public static
//	Array::Data::getSize -- 1つのサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cData_
//		データ
//	Array::Data::Type::Value eType_
//		データ型
//
//	RETURN
//	ModSize
//		1つのユニットサイズ
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
//	Array::Data::makeData -- タイプからCommon::Dataを得る
//
//	NOTES
//
//	ARGUMENTS
//	Array::Data::Type::Value eType_
//		データ型
//
//	RETURN
//	Common::Data::Pointer
//		Common::Dataへのポインタ
//
//	EXCEPTIONS
//
Common::Data::Pointer
Data::makeData(const LogicalFile::FileID& cFileID_,
			   int iPosition_,
			   Type::Value eType_,
			   bool isArray_)
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
	case Type::Double:
		p = new Common::DoubleData;
		break;
	case Type::Decimal:
	{
		int precision;
		int scale;
		Data::Decimal::getParameter(cFileID_, iPosition_, isArray_,
									precision, scale);
        p = new Common::DecimalData(precision, scale);
	}
		break;
	case Type::CharString:
	case Type::NoPadCharString:
		p = new Common::StringData(Common::StringData::EncodingForm::UTF8);
		break;
	case Type::UnicodeString:
	case Type::NoPadUnicodeString:
		p = new Common::StringData(Common::StringData::EncodingForm::UCS2);
		break;
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
//	FUNCTION private static
//	Array::Data::dump -- 1つのダンプする
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32*& p
//		メモリ
//	const Common::Data& cData_
//		データ
//	Array::Data::Type::Value eType_
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
//	FUNCTION private
//	Array::Data::getData -- 1つデータを得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32*& p
//		メモリ
//	Array::Data::Type::Value eType_
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
//	Copyright (c) 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
