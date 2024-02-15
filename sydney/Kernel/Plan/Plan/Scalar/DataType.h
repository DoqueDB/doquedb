// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/DataType.h --
// 
// Copyright (c) 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_DATATYPE_H
#define __SYDNEY_PLAN_SCALAR_DATATYPE_H

#include "Plan/Scalar/Module.h"
#include "Plan/Scalar/Declaration.h"
#include "Plan/Tree/Node.h"

#include "Common/DataType.h"
#include "Common/SQLData.h"
#include "Common/ObjectPointer.h"

_SYDNEY_BEGIN

namespace Schema
{
	class Column;
	class Field;
}

_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

class DataType
	: public Common::SQLData
{
public:
	typedef Common::SQLData Super;
	typedef DataType This;

	typedef Common::ObjectPointer<Common::Data> DataPointer;

	// constructor
	DataType();
	explicit DataType(const Super& cType_);
	explicit DataType(const Schema::Column& cColumn_);
	explicit DataType(const Schema::Field& cField_);
	explicit DataType(Common::DataType::Type eDataType_);
	DataType(const DataType& cOther_);

	// constructor
	static DataType getIntegerType();
	static DataType getUnsignedIntegerType();
	static DataType getBigIntegerType();
	static DataType getFloatType();
	static DataType getObjectIDType();
	static DataType getBinaryType(Flag::Value eFlag_ = Flag::Unlimited, int iLength_ = 0);
	static DataType getStringType(Flag::Value eFlag_ = Flag::Unlimited, int iLength_ = 0);
	static DataType getArrayType(int iLength_, const DataType& cElementType_);
	static DataType getElementType(const DataType& cArrayType_);
	static DataType getLanguageType();
	static DataType getWordType();
	static DataType getDateType();
	static DataType getTimestampType();
	static DataType getDecimalType(Flag::Value eFlag_ = Flag::Fixed, int iPrecision_ = 9, int iScale_ = 0);

	// destructor
	~DataType(); // no subclasses

	// get data size in bytes
	int getDataSize() const;

	// accessor
	Common::DataType::Type getDataType() const {return m_eType;}

	// get compatible type by operation
	static bool getCompatibleType(const DataType& cType1_,
								  const DataType& cType2_,
								  DataType& cResult_,
								  bool isForComparison_ = false,
								  Tree::Node::Type eType_ = Tree::Node::Undefined,
								  bool bForList_ = false);

	// check whether two types are compatible
	static bool isCompatibleType(const DataType& cType1_,
								 const DataType& cType2_);
	// check whether type1 can be assigned to type2 without cast
	static bool isAssignable(const DataType& cType1_,
							 const DataType& cType2_,
							 Tree::Node::Type eType_ = Tree::Node::Undefined);
	// check whether type1 can be compared with type2 without cast
	static bool isComparable(const DataType& cType1_,
							 const DataType& cType2_);

	// true for exact numeric type
	bool isExactNumericType() const;
	// true for unsigned numeric type
	bool isUnsignedType() const;
	// true if the type can be calculated
	bool isSimpleScalarType() const;
	// true for character string type
	bool isCharacterStringType() const;
	// true for string type
	bool isStringType() const;
	// true for LOB type
	bool isLobType() const;
	// true for language type
	bool isLanguageType() const;
	// true for ObjectID type
	bool isObjectIDType() const;
	// true for decimal type
	bool isDecimalType() const;
	// true for no type
	bool isNoType() const;
	// true for no type
	bool isBitSetType() const;

	// operator
	DataType& operator=(const DataType& cOther_);

	// create new data
	DataPointer createData() const;

protected:
private:
	DataType(Type::Value eType_,
			 Flag::Value eFlag_,
			 int iPrecision_,
			 int iScale_,
			 int iMaxCardinality_);
	DataType(Type::Value eType_,
			 Flag::Value eFlag_,
			 int iLength_,
			 int iMaxCardinality_);

	// used by createData
	DataPointer createArrayData() const;
	DataPointer createScalarData() const;

	// used by getDataSize()
	int getDataSize(Common::DataType::Type eType_, int iSize_) const;
	// get Common::DataType from Type::Value
	static Common::DataType::Type convDataType(Type::Value eType_);

	Common::DataType::Type m_eType;
};

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_DATATYPE_H

//
//	Copyright (c) 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
