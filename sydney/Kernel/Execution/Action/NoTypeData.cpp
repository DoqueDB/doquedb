// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/NoTypeData.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Action";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Action/NoTypeData.h"
#include "Execution/Action/Class.h"

#include "Common/Assert.h"
#include "Common/InputArchive.h"
#include "Common/OutputArchive.h"
#include "Common/SQLData.h"

#include "Common/BinaryData.h"
#include "Common/DataArrayData.h"
#include "Common/DateData.h"
#include "Common/DateTimeData.h"
#include "Common/DecimalData.h"
#include "Common/DefaultData.h"
#include "Common/DoubleData.h"
#include "Common/FloatData.h"
#include "Common/Integer64Data.h"
#include "Common/IntegerData.h"
#include "Common/NullData.h"
#include "Common/ObjectIDData.h"
#include "Common/StringData.h"
#include "Common/UnsignedInteger64Data.h"
#include "Common/UnsignedIntegerData.h"

#include "Exception/ClassCast.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

///////////////////////////////////////
// Execution::Action::NoTypeData

// FUNCTION public
//	Action::NoTypeData::getInstance -- 
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	NoTypeData*
//
// EXCEPTIONS

NoTypeData*
NoTypeData::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::NoTypeData);
	return new This;
}

// FUNCTION public
//	Action::NoTypeData::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
NoTypeData::
serialize(ModArchive& archiver_)
{
	if (archiver_.isStore()) {
		Common::OutputArchive& cOut = dynamic_cast<Common::OutputArchive&>(archiver_);
		cOut.writeObject(m_pData.get());

	} else {
		Common::InputArchive& cIn = dynamic_cast<Common::InputArchive&>(archiver_);
		m_pData = dynamic_cast<Common::Data*>(cIn.readObject());
	}
}

// FUNCTION public
//	Action::NoTypeData::copy -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Common::Data::Pointer
//
// EXCEPTIONS

//virtual
Common::Data::Pointer
NoTypeData::
copy() const
{
	if (m_pData.get() == 0) {
		return new This;
	} else {
		return m_pData->copy();
	}
}

// FUNCTION public
//	Action::NoTypeData::cast -- 
//
// NOTES
//
// ARGUMENTS
//	Common::DataType::Type type
//	bool bForAssign_ = false
//	
// RETURN
//	Common::Data::Pointer
//
// EXCEPTIONS

//virtual
Common::Data::Pointer
NoTypeData::
cast(Common::DataType::Type type, bool bForAssign_ /* = false */) const
{
	if (m_pData.get() == 0) {
		_SYDNEY_THROW0(Exception::ClassCast);
	}
	return m_pData->cast(type, bForAssign_);
}

// FUNCTION public
//	Action::NoTypeData::castToDecimal -- 
//
// NOTES
//
// ARGUMENTS
//	bool bForAssign_ = false
//	
// RETURN
//	Common::Data::Pointer
//
// EXCEPTIONS

//virtual
Common::Data::Pointer
NoTypeData::
castToDecimal(bool bForAssign_ /* = false */) const
{
	if (m_pData.get() == 0) {
		_SYDNEY_THROW0(Exception::ClassCast);
	}
	return m_pData->castToDecimal(bForAssign_);
}

// FUNCTION public
//	Action::NoTypeData::getType -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Common::DataType::Type
//
// EXCEPTIONS

//virtual
Common::DataType::Type
NoTypeData::
getType() const
{
	if (m_pData.get() == 0
		|| m_pData->getType() == Common::DataType::Null) {
		return Super::getType();
	}
	return m_pData->getType();
}

// FUNCTION public
//	Action::NoTypeData::getElementType -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Common::DataType::Type
//
// EXCEPTIONS

//virtual
Common::DataType::Type
NoTypeData::
getElementType() const
{
	if (m_pData.get() == 0) {
		return Common::DataType::Undefined;
	}
	return m_pData->getElementType();
}

// FUNCTION public
//	Action::NoTypeData::getString -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

//virtual
ModUnicodeString
NoTypeData::
getString() const
{
	if (m_pData.get() == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return m_pData->getString();
}

// FUNCTION public
//	Action::NoTypeData::getInt -- 
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
NoTypeData::
getInt() const
{
	if (m_pData.get() == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return m_pData->getInt();
}

// FUNCTION public
//	Action::NoTypeData::getUnsignedInt -- 
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
NoTypeData::
getUnsignedInt() const
{
	if (m_pData.get() == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return m_pData->getUnsignedInt();
}

// FUNCTION public
//	Action::NoTypeData::isScalar -- 
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

//virtual
bool
NoTypeData::
isScalar() const
{
	return m_pData.get() ? m_pData->isScalar() : false;
}

// FUNCTION public
//	Action::NoTypeData::setNull -- 
//
// NOTES
//
// ARGUMENTS
//	bool v = true
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
NoTypeData::
setNull(bool v /* = true */)
{
	Super::setNull(v);
	if (m_pData.get()) {
		m_pData->setNull(v);
	} else {
		// set nulldata
		m_pData = Common::NullData::getInstance();
	}
}

// FUNCTION public
//	Action::NoTypeData::setDefault -- 
//
// NOTES
//
// ARGUMENTS
//	bool v = true
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
NoTypeData::
setDefault(bool v /* = true */)
{
	Super::setDefault(v);
	if (m_pData.get()) {
		m_pData->setDefault(v);
	} else {
		// set defaultdata
		m_pData = Common::DefaultData::getInstance();
	}
}

// FUNCTION public
//	Action::NoTypeData::equals -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data* r
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
NoTypeData::
equals(const Common::Data* r) const
{
	if (m_pData.get() == 0) {
		return false;
	}
	const Common::Data* pSource = getSource(r);
	if (pSource == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return m_pData->equals(pSource);
}

// FUNCTION public
//	Action::NoTypeData::compareTo -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data* r
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
NoTypeData::
compareTo(const Common::Data* r) const
{
	if (m_pData.get() == 0) {
		_SYDNEY_THROW0(Exception::ClassCast);
	}
	const Common::Data* pSource = getSource(r);
	if (pSource == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return m_pData->compareTo(pSource);
}

// FUNCTION public
//	Action::NoTypeData::assign -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data* r
//	bool bForAssign_ = true
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
NoTypeData::
assign(const Common::Data* r, bool bForAssign_ /* = true */)
{
	const Common::Data* pSource = getSource(r);
	if (pSource == 0) {
		if (r &&
			r->Super::getType() == Common::DataType::Data) {
			const This* p = _SYDNEY_DYNAMIC_CAST(const This*, r);
			if (this->m_pData.get() == 0
				&& p->m_pData.get() == 0) {
				return true;
			}
		}
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	bool bResult = true;
	if (m_pData.get() == 0) {
		// just set copy of data
		m_pData = pSource->copy();
	} else if (m_pData->isDefault()) {
		if (!pSource->isDefault()) {
			m_pData = pSource->copy();
		}
	} else if (pSource->isNull()) {
		m_pData->setNull();
	} else if (m_pData->isCompatible(pSource)) {
		// assign
		bResult = m_pData->assign(pSource, bForAssign_);
	} else {
		// replace data
		m_pData = pSource->copy();
	}
	return bResult;
}

// FUNCTION public
//	Action::NoTypeData::operateWith -- 
//
// NOTES
//
// ARGUMENTS
//	Common::DataOperation::Type op
//	const Common::Data* r
//	Pointer& result
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
NoTypeData::
operateWith(Common::DataOperation::Type op, const Common::Data* r, Pointer& result) const
{
	if (m_pData.get() == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	const Common::Data* pSource = getSource(r);
	if (pSource == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return m_pData->operateWith(op, pSource, result);
}

// FUNCTION public
//	Action::NoTypeData::operateWith -- 
//
// NOTES
//
// ARGUMENTS
//	Common::DataOperation::Type op
//	const Common::Data* r
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
NoTypeData::
operateWith(Common::DataOperation::Type op, const Common::Data* r)
{
	if (m_pData.get() == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	const Common::Data* pSource = getSource(r);
	if (pSource == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return m_pData->operateWith(op, pSource);
}

// FUNCTION public
//	Action::NoTypeData::operateWith -- 
//
// NOTES
//
// ARGUMENTS
//	Common::DataOperation::Type op
//	Common::Data::Pointer& result
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
NoTypeData::
operateWith(Common::DataOperation::Type op, Common::Data::Pointer& result) const
{
	if (m_pData.get() == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return m_pData->operateWith(op, result);
}

// FUNCTION public
//	Action::NoTypeData::operateWith -- 
//
// NOTES
//
// ARGUMENTS
//	Common::DataOperation::Type op
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
NoTypeData::
operateWith(Common::DataOperation::Type op)
{
	if (m_pData.get() == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return m_pData->operateWith(op);
}

// FUNCTION public
//	Action::NoTypeData::isApplicable -- 
//
// NOTES
//
// ARGUMENTS
//	Function::Value function
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
NoTypeData::
isApplicable(Function::Value function)
{
	if ((function & Function::Unfold) == Function::Unfold) {
		return true;
	}
	if (m_pData.get() == 0) {
		return false;
	}
	return m_pData->isApplicable(function);
}

// FUNCTION public
//	Action::NoTypeData::apply -- 
//
// NOTES
//
// ARGUMENTS
//	Function::Value function
//	
// RETURN
//	Common::Data::Pointer
//
// EXCEPTIONS

//virtual
Common::Data::Pointer
NoTypeData::
apply(Function::Value function)
{
	if (m_pData.get() == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	if (function == Function::Unfold) {
		// if applied function is only unfold, use m_pData directly
		return m_pData;
	}
	return m_pData->apply(function);
}

// FUNCTION public
//	Action::NoTypeData::isAbleToDump -- 
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

//virtual
bool
NoTypeData::
isAbleToDump() const
{
	if (m_pData.get() == 0) {
		return false;
	}
	return m_pData->isAbleToDump();
}

// FUNCTION public
//	Action::NoTypeData::isFixedSize -- 
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

//virtual
bool
NoTypeData::
isFixedSize() const
{
	if (m_pData.get() == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return m_pData->isFixedSize();
}

// FUNCTION public
//	Action::NoTypeData::getDumpSize -- 
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
NoTypeData::
getDumpSize() const
{
	if (m_pData.get() == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return m_pData->getDumpSize();
}

// FUNCTION public
//	Action::NoTypeData::setDumpedValue -- 
//
// NOTES
//
// ARGUMENTS
//	const char* pszValue_
//	
// RETURN
//	ModSize
//
// EXCEPTIONS

//virtual
ModSize
NoTypeData::
setDumpedValue(const char* pszValue_)
{
	if (m_pData.get() == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return m_pData->setDumpedValue(pszValue_);
}

// FUNCTION public
//	Action::NoTypeData::setDumpedValue -- 
//
// NOTES
//
// ARGUMENTS
//	const char* pszValue_
//	ModSize uSize_
//	
// RETURN
//	ModSize
//
// EXCEPTIONS

//virtual
ModSize
NoTypeData::
setDumpedValue(const char* pszValue_, ModSize uSize_)
{
	if (m_pData.get() == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return m_pData->setDumpedValue(pszValue_, uSize_);
}

// FUNCTION public
//	Action::NoTypeData::dumpValue -- 
//
// NOTES
//
// ARGUMENTS
//	char* p
//	
// RETURN
//	ModSize
//
// EXCEPTIONS

//virtual
ModSize
NoTypeData::
dumpValue(char* p) const
{
	if (m_pData.get() == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return m_pData->dumpValue(p);
}

// FUNCTION public
//	Action::NoTypeData::setDumpedValue -- 
//
// NOTES
//
// ARGUMENTS
//	ModSerialIO& cSerialIO_
//	
// RETURN
//	ModSize
//
// EXCEPTIONS

//virtual
ModSize
NoTypeData::
setDumpedValue(ModSerialIO& cSerialIO_)
{
	if (m_pData.get() == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return m_pData->setDumpedValue(cSerialIO_);
}

// FUNCTION public
//	Action::NoTypeData::setDumpedValue -- 
//
// NOTES
//
// ARGUMENTS
//	ModSerialIO& cSerialIO_
//	ModSize uSize_
//	
// RETURN
//	ModSize
//
// EXCEPTIONS

//virtual
ModSize
NoTypeData::
setDumpedValue(ModSerialIO& cSerialIO_, ModSize uSize_)
{
	if (m_pData.get() == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return m_pData->setDumpedValue(cSerialIO_, uSize_);
}

// FUNCTION public
//	Action::NoTypeData::dumpValue -- 
//
// NOTES
//
// ARGUMENTS
//	ModSerialIO& cSerialIO_
//	
// RETURN
//	ModSize
//
// EXCEPTIONS

//virtual
ModSize
NoTypeData::
dumpValue(ModSerialIO& cSerialIO_) const
{
	if (m_pData.get() == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return m_pData->dumpValue(cSerialIO_);
}

// FUNCTION public
//	Action::NoTypeData::getSQLType -- 
//
// NOTES
//
// ARGUMENTS
//	Common::SQLData& cResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
NoTypeData::
getSQLType(Common::SQLData& cResult_)
{
	if (m_pData.get() == 0) {
		cResult_ = Common::SQLData();
		return true;
	} else {
		return m_pData->getSQLType(cResult_);
	}
}

// FUNCTION public
//	Action::NoTypeData::getClassID -- 
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
NoTypeData::
getClassID() const
{
	return Class::getClassID(Class::Category::NoTypeData);
}

// FUNCTION public
//	Action::NoTypeData::print -- 
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

//virtual
void
NoTypeData::
print() const
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Action::NoTypeData::operator const Common::BinaryData& -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Common::BinaryData&
//
// EXCEPTIONS

NoTypeData::
operator const Common::BinaryData&() const
{
	if (m_pData.get() && m_pData->getType() == Common::DataType::Binary) {
		return _SYDNEY_DYNAMIC_CAST(const Common::BinaryData&, *m_pData);
	}
	_SYDNEY_THROW0(Exception::ClassCast);
}

// FUNCTION public
//	Action::NoTypeData::operator const Common::DataArrayData& -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Common::DataArrayData&
//
// EXCEPTIONS

NoTypeData::
operator const Common::DataArrayData&() const
{
	if (m_pData.get()
		&& m_pData->getType() == Common::DataType::Array
		&& m_pData->getElementType() == Common::DataType::Data) {
		return _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&, *m_pData);
	}
	_SYDNEY_THROW0(Exception::ClassCast);
}

// FUNCTION public
//	Action::NoTypeData::operator const Common::DateData& -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Common::DateData&
//
// EXCEPTIONS

NoTypeData::
operator const Common::DateData&() const
{
	if (m_pData.get() && m_pData->getType() == Common::DataType::Date) {
		return _SYDNEY_DYNAMIC_CAST(const Common::DateData&, *m_pData);
	}
	_SYDNEY_THROW0(Exception::ClassCast);
}

// FUNCTION public
//	Action::NoTypeData::operator const Common::DateTimeData& -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Common::DateTimeData&
//
// EXCEPTIONS

NoTypeData::
operator const Common::DateTimeData&() const
{
	if (m_pData.get() && m_pData->getType() == Common::DataType::DateTime) {
		return _SYDNEY_DYNAMIC_CAST(const Common::DateTimeData&, *m_pData);
	}
	_SYDNEY_THROW0(Exception::ClassCast);
}

// FUNCTION public
//	Action::NoTypeData::operator const Common::DecimalData& -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Common::DecimalData&
//
// EXCEPTIONS

NoTypeData::
operator const Common::DecimalData&() const
{
	if (m_pData.get() && m_pData->getType() == Common::DataType::Decimal) {
		return _SYDNEY_DYNAMIC_CAST(const Common::DecimalData&, *m_pData);
	}
	_SYDNEY_THROW0(Exception::ClassCast);
}

// FUNCTION public
//	Action::NoTypeData::operator const Common::DoubleData& -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Common::DoubleData&
//
// EXCEPTIONS

NoTypeData::
operator const Common::DoubleData&() const
{
	if (m_pData.get() && m_pData->getType() == Common::DataType::Double) {
		return _SYDNEY_DYNAMIC_CAST(const Common::DoubleData&, *m_pData);
	}
	_SYDNEY_THROW0(Exception::ClassCast);
}

// FUNCTION public
//	Action::NoTypeData::operator const Common::FloatData& -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Common::FloatData&
//
// EXCEPTIONS

NoTypeData::
operator const Common::FloatData&() const
{
	if (m_pData.get() && m_pData->getType() == Common::DataType::Float) {
		return _SYDNEY_DYNAMIC_CAST(const Common::FloatData&, *m_pData);
	}
	_SYDNEY_THROW0(Exception::ClassCast);
}

// FUNCTION public
//	Action::NoTypeData::operator const Common::Integer64Data& -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Common::Integer64Data&
//
// EXCEPTIONS

NoTypeData::
operator const Common::Integer64Data&() const
{
	if (m_pData.get() && m_pData->getType() == Common::DataType::Integer64) {
		return _SYDNEY_DYNAMIC_CAST(const Common::Integer64Data&, *m_pData);
	}
	_SYDNEY_THROW0(Exception::ClassCast);
}

// FUNCTION public
//	Action::NoTypeData::operator const Common::IntegerData& -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Common::IntegerData&
//
// EXCEPTIONS

NoTypeData::
operator const Common::IntegerData&() const
{
	if (m_pData.get() && m_pData->getType() == Common::DataType::Integer) {
		return _SYDNEY_DYNAMIC_CAST(const Common::IntegerData&, *m_pData);
	}
	_SYDNEY_THROW0(Exception::ClassCast);
}

// FUNCTION public
//	Action::NoTypeData::operator const Common::ObjectIDData& -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Common::ObjectIDData&
//
// EXCEPTIONS

NoTypeData::
operator const Common::ObjectIDData&() const
{
	if (m_pData.get() && m_pData->getType() == Common::DataType::ObjectID) {
		return _SYDNEY_DYNAMIC_CAST(const Common::ObjectIDData&, *m_pData);
	}
	_SYDNEY_THROW0(Exception::ClassCast);
}

// FUNCTION public
//	Action::NoTypeData::operator const Common::StringData& -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Common::StringData&
//
// EXCEPTIONS

NoTypeData::
operator const Common::StringData&() const
{
	if (m_pData.get() && m_pData->getType() == Common::DataType::String) {
		return _SYDNEY_DYNAMIC_CAST(const Common::StringData&, *m_pData);
	}
	_SYDNEY_THROW0(Exception::ClassCast);
}

// FUNCTION public
//	Action::NoTypeData::operator const Common::UnsignedInteger64Data& -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Common::UnsignedInteger64Data&
//
// EXCEPTIONS

NoTypeData::
operator const Common::UnsignedInteger64Data&() const
{
	if (m_pData.get() && m_pData->getType() == Common::DataType::UnsignedInteger64) {
		return _SYDNEY_DYNAMIC_CAST(const Common::UnsignedInteger64Data&, *m_pData);
	}
	_SYDNEY_THROW0(Exception::ClassCast);
}

// FUNCTION public
//	Action::NoTypeData::operator const Common::UnsignedIntegerData& -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Common::UnsignedIntegerData&
//
// EXCEPTIONS

NoTypeData::
operator const Common::UnsignedIntegerData&() const
{
	if (m_pData.get() && m_pData->getType() == Common::DataType::UnsignedInteger) {
		return _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData&, *m_pData);
	}
	_SYDNEY_THROW0(Exception::ClassCast);
}

// FUNCTION private
//	Action::NoTypeData::getSource -- get source data from operator operand
//
// NOTES
//
// ARGUMENTS
//	const Common::Data* r
//	
// RETURN
//	const Common::Data*
//
// EXCEPTIONS

const Common::Data*
NoTypeData::
getSource(const Common::Data* r) const
{
	if (r && r->Super::getType() == Common::DataType::Data) {
		const This* p = _SYDNEY_DYNAMIC_CAST(const This*, r);
		; _SYDNEY_ASSERT(p);
		return p->m_pData.get();
	}
	return r;
}

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
