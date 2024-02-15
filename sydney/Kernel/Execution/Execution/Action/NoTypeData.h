// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Action/NoTypeData.h --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_ACTION_NOTYPEDATA_H
#define __SYDNEY_EXECUTION_ACTION_NOTYPEDATA_H

#include "Execution/Action/Module.h"
#include "Execution/Declaration.h"

#include "Common/Data.h"

_SYDNEY_BEGIN

namespace Common
{
	class BinaryData;
	class DataArrayData;
	class DateData;
	class DateTimeData;
	class DecimalData;
	class DoubleData;
	class FloatData;
	class Integer64Data;
	class IntegerData;
	class ObjectIDData;
	class StringData;
	class UnsignedInteger64Data;
	class UnsignedIntegerData;
}

_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

////////////////////////////////////
//	CLASS
//	Execution::Action::NoTypeData -- Common::Data subclass holding no-type data
//
//	NOTES

//////////////////////////////////////////////////////
// NoTypeData
//////////////////////////////////////////////////////
class NoTypeData
	: public Common::Data
{
public:
	typedef Common::Data Super;
	typedef NoTypeData This;

	// constructor
	NoTypeData()
		: Super(Common::DataType::Data),
		  m_pData()
	{}
	NoTypeData(const Common::Data::Pointer& pData_)
		: Super(Common::DataType::Data),
		  m_pData(pData_)
	{}

	// destructor
	~NoTypeData() {}

	// for serialize
	static This* getInstance(int iCategory_);

////////////////////////
// Common::Data::
	virtual void serialize(ModArchive& archiver);

	virtual Common::Data::Pointer copy() const;
	virtual Common::Data::Pointer cast(Common::DataType::Type type, bool bForAssign_ = false) const;
//	virtual Common::Data::Pointer cast(const Common::Data& target, bool bForAssign_ = false) const;
	virtual Common::Data::Pointer castToDecimal(bool bForAssign_ = false) const;
	virtual Common::DataType::Type getType() const;
	virtual Common::DataType::Type getElementType() const;
	virtual ModUnicodeString getString() const;
	virtual int getInt() const;
	virtual unsigned int getUnsignedInt() const;
	virtual bool isScalar() const;
	virtual void setNull(bool v = true);
	virtual void setDefault(bool v = true);
	virtual bool equals(const Common::Data* r) const;
	virtual int	compareTo(const Common::Data* r) const;
	virtual bool assign(const Common::Data* r, bool bForAssign_ = true);
	virtual bool operateWith(Common::DataOperation::Type op, const Common::Data* r, Pointer& result) const;
	virtual bool operateWith(Common::DataOperation::Type op, const Common::Data* r);
	virtual bool operateWith(Common::DataOperation::Type op, Common::Data::Pointer& result) const;
	virtual bool operateWith(Common::DataOperation::Type op);
	virtual bool isApplicable(Function::Value function);
	virtual Common::Data::Pointer apply(Function::Value function);
	virtual bool isAbleToDump() const;
	virtual bool isFixedSize() const;
	virtual ModSize getDumpSize() const;
	virtual ModSize setDumpedValue(const char* pszValue_);
	virtual ModSize setDumpedValue(const char* pszValue_, ModSize uSize_);
	virtual	ModSize	dumpValue(char* p) const;
	virtual ModSize setDumpedValue(ModSerialIO& cSerialIO_);
	virtual ModSize setDumpedValue(ModSerialIO& cSerialIO_, ModSize uSize_);
	virtual ModSize dumpValue(ModSerialIO& cSerialIO_) const;
	virtual bool getSQLType(Common::SQLData& cResult_);
	virtual int getClassID() const;
	virtual void print() const;

	////////////////////
	// cast operators
	operator const Common::BinaryData&() const;
	operator const Common::DataArrayData&() const;
	operator const Common::DateData&() const;
	operator const Common::DateTimeData&() const;
	operator const Common::DecimalData&() const;
	operator const Common::DoubleData&() const;
	operator const Common::FloatData&() const;
	operator const Common::Integer64Data&() const;
	operator const Common::IntegerData&() const;
	operator const Common::ObjectIDData&() const;
	operator const Common::StringData&() const;
	operator const Common::UnsignedInteger64Data&() const;
	operator const Common::UnsignedIntegerData&() const;
protected:
private:
	// never copied
	NoTypeData(const This& cOther_);

	// get source data from operator operand
	const Common::Data* getSource(const Common::Data* r) const;

	Common::Data::Pointer m_pData;
};

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ACTION_NOTYPEDATA_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
