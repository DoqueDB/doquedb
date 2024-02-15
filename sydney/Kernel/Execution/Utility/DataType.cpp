// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Utility/DataType.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Utility";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Utility/DataType.h"

#include "Common/DataArrayData.h"
#include "Common/DataInstance.h"
#include "Common/DecimalData.h"

#include "Exception/Unexpected.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_UTILITY_BEGIN

namespace
{
	// table to get data type with doubled precision
	Common::DataType::Type _DoubleTable[] =
	{
		Common::DataType::Integer64,	// Common::DataType::Integer
		Common::DataType::UnsignedInteger64, // Common::DataType::UnsignedInteger
		Common::DataType::Decimal,		// Common::DataType::Integer64
		Common::DataType::Decimal,		// Common::DataType::UnsignedInteger64
		Common::DataType::Undefined,	// Common::DataType::String
		Common::DataType::Double,		// Common::DataType::Float
		Common::DataType::Undefined,	// Common::DataType::Double
		Common::DataType::Decimal,		// Common::DataType::Decimal
		Common::DataType::Undefined,	// Common::DataType::Date
		Common::DataType::Undefined,	// Common::DataType::DateTime
		Common::DataType::Undefined,	// Common::DataType::Binary
		Common::DataType::Undefined,	// Common::DataType::BitSet
		Common::DataType::Undefined,	// Common::DataType::ObjectID
		Common::DataType::Undefined,	// Common::DataType::Language
		Common::DataType::Undefined,	// Common::DataType::ColumnMetaData
		Common::DataType::Undefined		// Common::DataType::Word
	};
}

////////////////////////////////////
// Execution::Utility::DataType

// FUNCTION public
//	Utility::DataType::getDoublePrecision -- get data with doubled precision
//
// NOTES
//
// ARGUMENTS
//	const Common::Data* pData_
//	
// RETURN
//	Common::Data::Pointer
//
// EXCEPTIONS

Common::Data::Pointer
DataType::
getDoublePrecision(const Common::Data* pData_)
{
	// result data
	Common::Data::Pointer pResult;

	if (pData_->isNull() == false
		&& pData_->getType() >= Common::DataType::MinScalar
		&& pData_->getType() < Common::DataType::MaxScalar) {
		Common::DataType::Type eNewType =
			_DoubleTable[pData_->getType() - Common::DataType::MinScalar];

		if (eNewType != Common::DataType::Decimal) {
			pResult = Common::DataInstance::create(eNewType);

		} else {
			switch (pData_->getType()) {
			case Common::DataType::Decimal:
				{
					const Common::DecimalData* pDecimalData =
						_SYDNEY_DYNAMIC_CAST(const Common::DecimalData*, pData_);
					pResult = new Common::DecimalData(pDecimalData->getPrecision() * 2,
													  pDecimalData->getScale());
					break;
				}
			case Common::DataType::Integer64:
				{
					pResult = new Common::DecimalData(19 * 2, 0);
					break;
				}
			case Common::DataType::UnsignedInteger64:
				{
					pResult = new Common::DecimalData(20 * 2, 0);
					break;
				}
			default:
				{
					_SYDNEY_THROW0(Exception::Unexpected);
				}
			}
		}

		if (pResult.get()) {
			// assign original data
			pResult->assign(pData_);
		}
	}

	return pResult;
}

// FUNCTION public
//	Utility::DataType::assignElements -- assign each element of array
//
// NOTES
//
// ARGUMENTS
//	Common::DataArrayData* pTarget_
//	const Common::DataArrayData* pSource_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
DataType::
assignElements(Common::DataArrayData* pTarget_,
			   const Common::DataArrayData* pSource_)
{
	int n = pTarget_->getCount();
	if (n != pSource_->getCount()) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	for (int i = 0; i < n; ++i) {
		pTarget_->getElement(i)->assign(pSource_->getElement(i).get());
	}
}

// FUNCTION public
//	Utility::DataType::setNullElements -- set null each element of array
//
// NOTES
//
// ARGUMENTS
//	Common::DataArrayData* pTarget_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
DataType::
setNullElements(Common::DataArrayData* pTarget_)
{
	int n = pTarget_->getCount();
	for (int i = 0; i < n; ++i) {
		pTarget_->getElement(i)->setNull();
	}
}

_SYDNEY_EXECUTION_UTILITY_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
