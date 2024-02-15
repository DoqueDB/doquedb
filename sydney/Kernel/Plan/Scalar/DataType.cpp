// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/DataType.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Scalar";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Plan/Scalar/DataType.h"

#include "Common/Data.h"
#include "Common/DataArrayData.h"
#include "Common/DataInstance.h"
#include "Common/DataType.h"
#include "Common/NullData.h"
#include "Common/Message.h"
#include "Common/SQLData.h"

#include "Execution/Action/NoTypeData.h"

#include "Schema/Column.h"
#include "Schema/Field.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_SCALAR_USING

namespace
{
#define ComXXX Common::DataType::Undefined
#define ComInt Common::DataType::Integer
#define ComUIt Common::DataType::UnsignedInteger
#define ComI64 Common::DataType::Integer64
#define ComU64 Common::DataType::UnsignedInteger64
#define ComStr Common::DataType::String
#define ComFlt Common::DataType::Float
#define ComDbl Common::DataType::Double
#define ComDec Common::DataType::Decimal
#define ComDat Common::DataType::Date
#define ComTim Common::DataType::DateTime
#define ComBin Common::DataType::Binary
#define ComBit Common::DataType::BitSet
#define ComOID Common::DataType::ObjectID
#define ComLan Common::DataType::Language
#define ComMet Common::DataType::ColumnMetaData
#define ComWrd Common::DataType::Word
	// Type -> Common::DataType table
	const Common::DataType::Type _TypeTable[Common::SQLData::Type::ValueNum] =
	{
		ComXXX, // NoType
		ComStr, // Char
		ComStr, // NChar
		ComInt, // Int
		ComDbl, // Float
		ComTim, // DateTime
		ComStr, // UniqueIdentifier
		ComBin, // Binary
		ComBin, // Image
		ComStr, // NText
		ComStr, // Fulltext
		ComLan, // Language
		ComBin, // BLOB
		ComStr, // CLOB
		ComStr, // NCLOB
		ComDec, // Decimal
		ComDat, // Date
		ComTim, // Time
		ComTim, // Timestamp
		ComUIt,	// UInt
		ComWrd,	// Word
		ComI64,	// BigInt
	};
#undef ComXXX
#undef ComInt
#undef ComUIt
#undef ComI64
#undef ComU64
#undef ComStr
#undef ComFlt
#undef ComDbl
#undef ComDec
#undef ComDat
#undef ComTim
#undef ComBin
#undef ComBit
#undef ComOID
#undef ComLan
#undef ComMet
#undef ComWrd

#define OpeUnk Common::DataOperation::Unknown
#define OpeAdd Common::DataOperation::Addition
#define OpeSub Common::DataOperation::Subtraction
#define OpeMul Common::DataOperation::Multiplication
#define OpeDiv Common::DataOperation::Division
#define OpeNeg Common::DataOperation::Negation
#define OpeAbs Common::DataOperation::AbsoluteValue

	// type -> dataoperation table
	const Common::DataOperation::Type _OperationTable[] =
	{
		OpeUnk,			//ArithmeticStart
		OpeAdd,			//Add
		OpeSub,			//Subtract
		OpeMul,			//Multiply
		OpeDiv,			//Divide
		OpeNeg,			//Negative
		OpeUnk,			//StringConcatenate
		OpeAbs,			//Absolute
		OpeUnk,			//CharLength
		OpeUnk,			//SubString
		OpeUnk,			//Overlay
		OpeUnk,			//Truncate
		OpeUnk,			//Nop
		OpeUnk,			//Case
		OpeUnk,			//NullIf
		OpeUnk,			//Coalesce
		OpeUnk,			//Cast
		OpeUnk,			//WordCount
		OpeUnk,			//FullTextLength
		OpeUnk,			//Normalize
		OpeUnk,			//OctetLength
		OpeUnk,			//CoalesceDefault
		OpeUnk,			//ArrayConcatenate
		OpeUnk			//ArithmeticEnd
	};
#undef OpeUnk
#undef OpeAdd
#undef OpeSub
#undef OpeMul
#undef OpeDiv
#undef OpeNeg
#undef OpeAbs

	Common::DataOperation::Type
	_getOperation(Tree::Node::Type eType_)
	{
		if (eType_ > Tree::Node::ArithmeticStart
			&& eType_ < Tree::Node::ArithmeticEnd) {
			return _OperationTable[eType_ - Tree::Node::ArithmeticStart];
		}
		return Common::DataOperation::Unknown;
	}

} // namespace

// FUNCTION public
//	Scalar::DataType::DataType -- constructor
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

DataType::
DataType()
	: Super(),
	  m_eType(Common::DataType::Data)
{
}

// FUNCTION public
//	Scalar::DataType::DataType -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Super& cType_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

DataType::
DataType(const Super& cType_)
	: Super(cType_),
	  m_eType(convDataType(getType()))
{
}

// FUNCTION public
//	Scalar::DataType::DataType -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Schema::Column& cColumn_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

DataType::
DataType(const Schema::Column& cColumn_)
	: Super(cColumn_.getType()),
	  m_eType(convDataType(getType()))
{
	// 特殊な列はデータ型が違う
	if (cColumn_.isTupleID()) {
		setType(Common::SQLData::Type::UInt);
		m_eType = Schema::TupleID().getType();
	}
	// UniqueIdentifier
	if (getType() == Common::SQLData::Type::UniqueIdentifier) {
		setType(Common::SQLData::Type::Char);
		setLength(36);
	}
}

// FUNCTION public
//	Scalar::DataType::DataType -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Schema::Field& cField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

DataType::
DataType(const Schema::Field& cField_)
	: Super(),
	  m_eType(Common::DataType::Undefined)
{
	if (cField_.getType() == Common::DataType::Array) {
		m_eType = cField_.getElementType();
		setLength(cField_.getElementLength());
		setScale(cField_.getScale());
		setMaxCardinality(cField_.getLength() == 0 ? -1 : cField_.getLength());

	} else {
		m_eType = cField_.getType();
		setLength(cField_.getLength());
		setScale(cField_.getScale());
	}

	// SQLDataとしての値をセットする
	Common::SQLData cType;
	if (Common::Data::getSQLType(m_eType, cType)) {
		setType(cType.getType());
		setFlag(cField_.isFixed() || cType.getFlag() == Flag::Fixed
				? Flag::Fixed
				: Flag::Variable);
	}
}

// FUNCTION public
//	Scalar::DataType::DataType -- constructor
//
// NOTES
//
// ARGUMENTS
//	Common::DataType::Type eDataType_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

DataType::
DataType(Common::DataType::Type eDataType_)
	: Super(),
	  m_eType(eDataType_)
{
	// SQLDataとしての値をセットする
	(void) Common::Data::getSQLType(m_eType, *this);
}

// FUNCTION public
//	Scalar::DataType::DataType -- 
//
// NOTES
//
// ARGUMENTS
//	const DataType& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

DataType::
DataType(const DataType& cOther_)
	: Super(cOther_),
	  m_eType(cOther_.m_eType)
{}

// FUNCTION public
//	Scalar::DataType::getIntegerType -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	DataType
//
// EXCEPTIONS

//static
DataType
DataType::
getIntegerType()
{
	return DataType(Common::DataType::Integer);
}

// FUNCTION public
//	Scalar::DataType::getUnsignedIntegerType -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	DataType
//
// EXCEPTIONS

//static
DataType
DataType::
getUnsignedIntegerType()
{
	return DataType(Common::DataType::UnsignedInteger);
}

// FUNCTION public
//	Scalar::DataType::getBigIntegerType -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	DataType
//
// EXCEPTIONS

//static
DataType
DataType::
getBigIntegerType()
{
	return DataType(Common::DataType::Integer64);
}

// FUNCTION public
//	Scalar::DataType::getFloatType -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	DataType
//
// EXCEPTIONS

//static
DataType
DataType::
getFloatType()
{
	return DataType(Common::DataType::Double);
}

// FUNCTION public
//	Scalar::DataType::getObjectIDType -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	DataType
//
// EXCEPTIONS

//static
DataType
DataType::
getObjectIDType()
{
	return DataType(Common::DataType::ObjectID);
}

// FUNCTION public
//	Scalar::DataType::getBinaryType -- 
//
// NOTES
//
// ARGUMENTS
//	Flag::Value eFlag_
//	int iLength_
//	
// RETURN
//	DataType
//
// EXCEPTIONS

//static
DataType
DataType::
getBinaryType(Flag::Value eFlag_, int iLength_)
{
	return DataType(Type::Binary, eFlag_, iLength_, 0);
}

// FUNCTION public
//	Scalar::DataType::getStringType -- 
//
// NOTES
//
// ARGUMENTS
//	Flag::Value eFlag_
//	int iLength_
//	
// RETURN
//	DataType
//
// EXCEPTIONS

//static
DataType
DataType::
getStringType(Flag::Value eFlag_, int iLength_)
{
	return DataType(Type::NChar, eFlag_, iLength_, 0);
}

// FUNCTION public
//	Scalar::DataType::getArrayType -- 
//
// NOTES
//
// ARGUMENTS
//	int iLength_
//	const DataType& cElementType_
//	
// RETURN
//	DataType
//
// EXCEPTIONS

//static
DataType
DataType::
getArrayType(int iLength_, const DataType& cElementType_)
{
	DataType cResult(cElementType_);
	cResult.setMaxCardinality(iLength_);
	return cResult;
}

// FUNCTION public
//	Scalar::DataType::getElementType -- 
//
// NOTES
//
// ARGUMENTS
//	const DataType& cArrayType_
//	
// RETURN
//	DataType
//
// EXCEPTIONS

//static
DataType
DataType::
getElementType(const DataType& cArrayType_)
{
	if (cArrayType_.isArrayType()) {
		DataType cResult(cArrayType_);
		cResult.setMaxCardinality(0);
		return cResult;
	} else {
		return DataType();
	}
}

// FUNCTION public
//	Scalar::DataType::getLanguageType -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	DataType
//
// EXCEPTIONS

//static
DataType
DataType::
getLanguageType()
{
	return DataType(Type::Language, Flag::None, 0, 0);
}

// FUNCTION public
//	Scalar::DataType::getWordType -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	DataType
//
// EXCEPTIONS

//static
DataType
DataType::
getWordType()
{
	return DataType(Type::Word, Flag::None, 0, 0);
}

// FUNCTION public
//	Scalar::DataType::getDateType -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	DataType
//
// EXCEPTIONS

//static
DataType
DataType::
getDateType()
{
	return DataType(Common::DataType::Date);
}

// FUNCTION public
//	Scalar::DataType::getTimestampType -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	DataType
//
// EXCEPTIONS

//static
DataType
DataType::
getTimestampType()
{
	return DataType(Common::DataType::DateTime);
}

// FUNCTION public
//	Scalar::DataType::getDecimalType -- static 
//
// NOTES
//
// ARGUMENTS
//	Flag::Value eFlag_
//	int iPrecision_
//	int iScale_
//	
// RETURN
//	DataType 
//
// EXCEPTIONS

//static 
DataType 
DataType::
getDecimalType(Flag::Value eFlag_, int iPrecision_, int iScale_)
{
	return DataType(Type::Decimal, eFlag_, iPrecision_, iScale_, 0);
}

// FUNCTION public
//	Scalar::DataType::~DataType -- destructor
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

DataType::
~DataType()
{}

// FUNCTION public
//	Scalar::DataType::getDataSize -- get data size in bytes
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

int
DataType::
getDataSize() const
{
	if (!isArrayType())
		return getDataSize(m_eType, getLength());

	int n = getMaxCardinality();
	if (n < 0)
		// use dummy data
		n = 30;

	return getDataSize(m_eType, getLength()) * n;
}

// FUNCTION public
//	Plan::DataType::getCompatibleType -- 上位互換性のある型を作る
//
// NOTES
//
// ARGUMENTS
//	const DataType& cType1_
//	const DataType& cType2_
//		比較される型
//	DataType& cResult_
//		上位互換性のある型
//	bool bForComparison_ = false
//		trueのとき比較のための互換性を調べる
//		(双方にキャスト可能なときに左辺にキャストする)
//	bool bForList_ = false
//		trueのときArrayと要素型の組み合わせも許す
//	
// RETURN
//	bool
//		上位互換性のある型が得られればcResult_に設定した上でtrueを返す
//
// EXCEPTIONS

//static
bool
DataType::
getCompatibleType(const DataType& cType1_, const DataType& cType2_,
				  DataType& cResult_, bool bForComparison_ /* = false */,
				  Tree::Node::Type eType_ /* = Tree::Node::Undefined */,
				  bool bForList_ /* = false */)
{
	if (bForComparison_ || cType1_.isArrayType() == cType2_.isArrayType()) {
		// SQLDataとして互換性のある型を得る
		if (Common::SQLData::getCompatibleType(cType1_, cType2_, cResult_,
											   bForComparison_,
											   _getOperation(eType_))) {
			// 結果の型からDataTypeも作る
			cResult_.m_eType = convDataType(cResult_.getType());
			return true;

		} else if (cType1_.isNoType() && cType2_.isNoType()
				   && (cType1_.getDataType() == cType2_.getDataType()
					   || cType1_.getDataType() == Common::DataType::Null
					   || cType2_.getDataType() == Common::DataType::Null)) {
			// SQLDataとして互換性がない場合でも両方NoTypeならDataTypeを比較して調べる
			cResult_.m_eType = (cType1_.getDataType() == Common::DataType::Null)
				? cType2_.getDataType()
				: cType1_.getDataType();
			return true;
		}
	} else if (bForList_ && cType1_.isArrayType() != cType2_.isArrayType()) {
		DataType cElement1 = (cType1_.isArrayType() ? getElementType(cType1_) : cType1_);
		DataType cElement2 = (cType2_.isArrayType() ? getElementType(cType2_) : cType2_);
		if (getCompatibleType(cElement1, cElement2, cResult_,
							  bForComparison_,
							  eType_)) {
			cResult_.setMaxCardinality(-1);
		}
		return true;
	}

	return false;
}

// FUNCTION public
//	Scalar::DataType::isCompatibleType -- 型に互換性があるかを得る
//
// NOTES
//
// ARGUMENTS
//	const DataType& cType1_
//	const DataType& cType2_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
DataType::
isCompatibleType(const DataType& cType1_, const DataType& cType2_)
{
	Common::DataType::Type eType1 = cType1_.m_eType;
	Common::DataType::Type eType2 = cType2_.m_eType;

	// 型が同一なら当然互換性がある
	// どちらかがNullなら互換性がある(Nullにしかならない)
	// どちらかがNoTypeならCommon::DataTypeで調べる
	// 両方NoType以外ならSQLDataとして調べる
	return (cType1_.isArrayType() == cType2_.isArrayType())
		&& (eType1 == eType2
			||
			eType1 == Common::DataType::Null
			||
			eType2 == Common::DataType::Null
			|| ((cType1_.isNoType() || cType2_.isNoType())
				&& Common::Data::isCompatible(eType1, eType2))
			|| (!cType1_.isNoType() && !cType2_.isNoType()
				&& Common::SQLData::isCompatibleType(cType1_, cType2_)));
}

// FUNCTION public
//	Scalar::DataType::isAssignable -- check whether type1 can be assigned to type2 without cast
//
// NOTES
//
// ARGUMENTS
//	const DataType& cType1_
//	const DataType& cType2_
//	Tree::Node::Type eType_ = Tree::Node::Undefined
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
DataType::
isAssignable(const DataType& cType1_,
			 const DataType& cType2_,
			 Tree::Node::Type eType_ /* = Tree::Node::Undefined */)
{
	if (cType1_.getDataType() == cType2_.getDataType())
		return (cType1_.isNoType() && cType2_.isNoType())
			|| Common::SQLData::isAssignable(cType1_, cType2_,
											 _getOperation(eType_));

	// 型が異なっても代入元がNullか代入先がNoTypeなら可能
	return cType1_.getDataType() == Common::DataType::Null
		|| cType2_.isNoType();
}

// FUNCTION public
//	Scalar::DataType::isComparable -- check whether type1 can be compared with type2 without cast
//
// NOTES
//
// ARGUMENTS
//	const DataType& cType1_
//	const DataType& cType2_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
DataType::
isComparable(const DataType& cType1_,
			 const DataType& cType2_)
{
	return (cType1_.m_eType == Common::DataType::Null
			|| cType2_.m_eType == Common::DataType::Null
			|| Common::SQLData::isComparable(cType1_, cType2_));
}

// FUNCTION public
//	Scalar::DataType::isExactNumericType -- 数値型か
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
DataType::
isExactNumericType() const
{
	switch (getType()) {
	case Type::Int:
	case Type::UInt:
	case Type::BigInt:
	case Type::Decimal:
		{
			return true;
		}
	case Type::NoType:
		{
			switch (getDataType()) {
			case Common::DataType::Integer:
			case Common::DataType::UnsignedInteger:
			case Common::DataType::Integer64:
			case Common::DataType::UnsignedInteger64:
			case Common::DataType::Decimal:
				{
					return true;
				}
			default:
				{
					break;
				}
			}
			break;
		}
	default:
		{
			break;
		}
	}
	return false;
}

// FUNCTION public
//	Scalar::DataType::isUnsignedType -- true for unsigned numeric type
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
DataType::
isUnsignedType() const
{
	switch (getType()) {
	case Type::UInt:
		{
			return true;
		}
	case Type::NoType:
		{
			switch (getDataType()) {
			case Common::DataType::UnsignedInteger:
			case Common::DataType::UnsignedInteger64:
				{
					return true;
				}
			default:
				{
					break;
				}
			}
			break;
		}
	default:
		{
			break;
		}
	}
	return false;
}

// FUNCTION public
//	Scalar::DataType::isSimpleScalarType -- 四則演算のできるScalar型か
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
DataType::
isSimpleScalarType() const
{
	switch (getType()) {
	case Type::Int:
	case Type::BigInt:
	case Type::Float:
	case Type::Decimal:
	case Type::UInt:
		{
			return true;
		}
	case Type::NoType:
		{
			switch (getDataType()) {
			case Common::DataType::Integer:
			case Common::DataType::UnsignedInteger:
			case Common::DataType::Integer64:
			case Common::DataType::UnsignedInteger64:
			case Common::DataType::Float:
			case Common::DataType::Double:
			case Common::DataType::Decimal:
				{
					return true;
				}
			default:
				{
					break;
				}
			}
			break;
		}
	default:
		{
			break;
		}
	}
	return false;
}

// 文字列型か
bool
DataType::
isCharacterStringType() const
{
	switch (getType()) {
	case Type::Char:
	case Type::NChar:
	case Type::UniqueIdentifier:
	case Type::NText:
	case Type::Fulltext:
	case Type::CLOB:
	case Type::NCLOB:
		{
			return true;
		}
	case Type::NoType:
		{
			switch (getDataType()) {
			case Common::DataType::String:
				{
					return true;
				}
			default:
				{
					break;
				}
			}
			break;
		}
	default:
		{
			break;
		}
	}
	return false;
}

// FUNCTION public
//	Scalar::DataType::isStringType -- ストリング型か
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
DataType::
isStringType() const
{
	switch (getType()) {
	case Type::Char:
	case Type::NChar:
	case Type::UniqueIdentifier:
	case Type::Binary:
	case Type::Image:
	case Type::NText:
	case Type::Fulltext:
	case Type::BLOB:
	case Type::CLOB:
	case Type::NCLOB:
		{
			return true;
		}
	case Type::NoType:
		{
			switch (getDataType()) {
			case Common::DataType::String:
			case Common::DataType::Binary:
				{
					return true;
				}
			default:
				{
					break;
				}
			}
			break;
		}
	default:
		{
			break;
		}
	}
	return false;
}

// FUNCTION public
//	Scalar::DataType::isLobType -- LOB型か
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
DataType::
isLobType() const
{
	switch (getType()) {
	case Type::BLOB:
	case Type::CLOB:
	case Type::NCLOB:
		{
			return true;
		}
	default:
		{
			break;
		}
	}
	return false;
}

// FUNCTION public
//	Scalar::DataType::isLanguageType -- 言語型か
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
DataType::
isLanguageType() const
{
	return getDataType() == Common::DataType::Language;
}

// FUNCTION public
//	Scalar::DataType::isObjectIDType -- ObjectID型か
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
DataType::
isObjectIDType() const
{
	return getDataType() == Common::DataType::ObjectID;
}

// FUNCTION public
//	Scalar::DataType::isDecimalType -- true for decimal type
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
DataType::
isDecimalType() const
{
	return getDataType() == Common::DataType::Decimal;
}

// FUNCTION public
//	Scalar::DataType::isNoType -- NoTypeか
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
DataType::
isNoType() const
{
	return getType() == Type::NoType;
}


// FUNCTION public
//	Scalar::DataType::isBitSetType -- BitSetTypeか
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
DataType::
isBitSetType() const
{
	return getDataType() == Common::DataType::BitSet;
}




// FUNCTION public
//	Scalar::DataType::operator= -- 演算子
//
// NOTES
//
// ARGUMENTS
//	const DataType& cOther_
//	
// RETURN
//	DataType&
//
// EXCEPTIONS

DataType&
DataType::
operator=(const DataType& cOther_)
{
	if (this != &cOther_) {
		Super::operator=(cOther_);
		m_eType = cOther_.m_eType;
	}
	return *this;
}

// FUNCTION public
//	Scalar::DataType::createData -- データを作る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	DataType::DataPointer
//
// EXCEPTIONS

DataType::DataPointer
DataType::
createData() const
{
	return isArrayType() ? createArrayData()
		: (getDataType() == Common::DataType::Null ? Common::NullData::getInstance()
		   : createScalarData());
}

// FUNCTION private
//	Scalar::DataType::DataType -- 
//
// NOTES
//
// ARGUMENTS
//	Type::Value eType_
//	Flag::Value eFlag_
//	int iPrecision_
//	int iScale_
//	int iMaxCardinality_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

DataType::
DataType(Type::Value eType_, Flag::Value eFlag_, int iPrecision_, int iScale_, int iMaxCardinality_)
	: Super(),
	  m_eType(convDataType(eType_))
{
	setType(eType_);
	setFlag(eFlag_);
	setLength(iPrecision_);
	setScale(iScale_);
	setMaxCardinality(iMaxCardinality_);
}

// FUNCTION private
//	Scalar::DataType::DataType -- 
//
// NOTES
//
// ARGUMENTS
//	Type::Value eType_
//	Flag::Value eFlag_
//	int iLength_
//	int iMaxCardinality_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

DataType::
DataType(Type::Value eType_, Flag::Value eFlag_, int iLength_, int iMaxCardinality_)
	: Super(),
	  m_eType(convDataType(eType_))
{
	setType(eType_);
	setFlag(eFlag_);
	setLength(iLength_);
	setMaxCardinality(iMaxCardinality_);
}

// FUNCTION private
//	Scalar::DataType::createArrayData -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	DataType::DataPointer
//
// EXCEPTIONS

DataType::DataPointer
DataType::
createArrayData() const
{
	return new Common::DataArrayData;
}

// FUNCTION private
//	Scalar::DataType::createScalarData -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	DataType::DataPointer
//
// EXCEPTIONS

DataType::DataPointer
DataType::
createScalarData() const
{
	return isNoType()
		? (getDataType() != Common::DataType::Data
		   ? Common::DataInstance::create(getDataType())
		   : new Execution::Action::NoTypeData)
		: Common::DataInstance::create(*this);
}

// FUNCTION private
//	Scalar::DataType::getDataSize -- 
//
// NOTES
//
// ARGUMENTS
//	Common::DataType::Type eType_
//	Size iSize_
//	
// RETURN
//	DataType::Size
//
// EXCEPTIONS

int
DataType::
getDataSize(Common::DataType::Type eType_, int iSize_) const
{
	int iResult = Common::Data::getDumpSize(eType_);
	if (iResult == 0) {
		switch (eType_) {
		case Common::DataType::Data:
		case Common::DataType::BitSet:
		case Common::DataType::Language:
			{
				// 不明なので適当な値
				iResult = 64;
				break;
			}
		case Common::DataType::String:
		case Common::DataType::Decimal:
		case Common::DataType::Binary:
			{
				if (iSize_ > 0)
					iResult = iSize_;
				else
					// 無制限を意味するので適当な値
					iResult = 20000;
				break;
			}
		default:
			{
				break;
			}
		}
	}
	return iResult;
}

// FUNCTION private
//	Scalar::DataType::convDataType -- TypeからCommon::Dataのデータ型を得る
//
// NOTES
//
// ARGUMENTS
//	Type::Value eType_
//	
// RETURN
//	Common::DataType::Type
//
// EXCEPTIONS

//static
Common::DataType::Type
DataType::
convDataType(Type::Value eType_)
{
	if (eType_ >= Type::NoType && eType_ < Type::ValueNum)
		return _TypeTable[eType_];
	else
		return Common::DataType::Undefined;
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
