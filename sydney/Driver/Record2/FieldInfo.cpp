// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FieldInfo.cpp -- 
// 
// Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Record2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/DecimalData.h"

#include "Record2/FieldInfo.h"
#include "Record2/VariableData.h"


_SYDNEY_USING

_SYDNEY_RECORD2_USING


//	FUNCTION public
//	Record2::FieldInfo::FieldInfo -- constructor
//
//	NOTES
//		-- constructor
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

//FieldInfo::
//FieldInfo()
//	:m_iFieldID(0),
//	m_iFieldIndex(0),
//	m_iDataIndex(0),
//	m_bCompress(false),
//	m_bIsVariable(false),
//	m_bSelected(false),
//	m_uiSchemaElementNum(0)
//{
//	m_cDataType._name = Common::DataType::Undefined;
//	m_cDataType._length = 0;
//	m_cDataType._scale = 0;
//	m_cDataType._encodingForm = Common::StringData::EncodingForm::UCS2;
//
//	//set element type default value
//	m_cElementType._name = Common::DataType::Undefined;
//	m_cElementType._length = 0;
//	m_cElementType._scale = 0;
//	m_cElementType._encodingForm = Common::StringData::EncodingForm::UCS2;
//
//	m_cDataSize.m_iFieldSize = 0;
//	m_cAdditionInfo.m_uiDataElementNum = 0;
//}


//	FUNCTION public
//	Record2::FieldInfo::FieldInfo -- constructor
//
//	NOTES
//		-- constructor
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

FieldInfo::
FieldInfo(const int iFieldID_,
		const Common::DataType::Type& cDataType_,
		const Utility::FieldLength uiLength_,
		const Common::StringData::EncodingForm::Value& eEncodingForm_,
		const bool bIsVariable_, 
		const bool bCompress_)
	:m_iFieldID(iFieldID_),
	m_iFieldIndex(iFieldID_),
	m_iDataIndex(iFieldID_),
	m_bCompress(bCompress_),
	m_bIsVariable(bIsVariable_),
	m_bSelected(false),
	m_uiDataLength(0),
	m_uiSchemaElementNum(0)
{
	//set field datatype
	m_cDataType._name = cDataType_;
	m_cDataType._length = uiLength_;
	m_cDataType._scale = 0;
	m_cDataType._encodingForm = eEncodingForm_;

	//set element type default value
	m_cElementType._name = Common::DataType::Undefined;
	m_cElementType._length = 0;
	m_cElementType._scale = 0;
	m_cElementType._encodingForm = Common::StringData::EncodingForm::UCS2;

	//set additional info
	m_cAdditionInfo.m_uiDataElementNum = 0;

	//have to coincident
	; _SYDNEY_ASSERT(bIsVariable_ == m_cDataType.isVariable());
}

//
//	FUNCTION public
//	Record2::FieldInfo::~FieldInfo -- destructor
//
//	NOTES
//		destructor
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//	
//
FieldInfo::
~FieldInfo()
{
}

//	FUNCTION public
//	Record2::FieldInfo::setElementInfo -- setElementInfo
//
//	NOTES
//		-- setElementInfo
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void 
FieldInfo::
setElementInfo(int iType_, 
				int iLength_, 
				int iCount_, 
				Common::StringData::EncodingForm::Value& eEncodingForm_)
{
	; _SYDNEY_ASSERT(isArray());

	m_cElementType._name = static_cast<Common::DataType::Type>(iType_);
	m_cElementType._length = static_cast<Utility::FieldLength>(iLength_);
	m_cElementType._encodingForm = eEncodingForm_;

	m_uiSchemaElementNum = static_cast<Utility::ElementNum>(iCount_);
}

//	FUNCTION public
//	Record2::FieldInfo::setFraction -- setFraction
//
//	NOTES
//		-- setFraction
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
FieldInfo::
setFraction(Utility::FieldLength uiFraction_)
{
	m_cDataType._scale = uiFraction_;
	m_cElementType._scale = uiFraction_;
}

//	FUNCTION public
//	Record2::FieldInfo::isArray -- isArray
//
//	NOTES
//		-- isArray
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

bool
FieldInfo::
isArray() const
{
	return m_cDataType._name == Common::DataType::Array;
}

//	FUNCTION public
//	Record2::FieldInfo::isVariableArray -- isVariableArray
//
//	NOTES
//		-- Is it an array of a variable-length element?
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

bool
FieldInfo::
isVariableArray() const
{
	return m_cDataType._name == Common::DataType::Array
		&& FileCommon::DataManager::isVariable(m_cElementType._name);
}

//	FUNCTION public
//	Record2::FieldInfo::setLength -- setLength
//
//	NOTES
//		-- set field length, if bSchema_ == false, set really length
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void 
FieldInfo::
setLength(const Utility::FieldLength uiLength_, bool bSchema_)
{
	if(bSchema_)
	{
		m_cDataType._length = uiLength_;
	}
	else
	{
		m_uiDataLength = uiLength_;
	}
}

//	FUNCTION public
//	Record2::FieldInfo::makeKnowableLength -- makeKnowableLength
//
//	NOTES
//		-- get knowable(fixed) length, 
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS
//
Utility::FieldLength 
FieldInfo::
makeKnowableLength()
{
	if(!m_bIsVariable)	//fixed data
	{
		if (m_cDataType._name == Common::DataType::Decimal)
			m_uiDataLength = Common::DecimalData::getDumpSizeBy(m_cDataType._length, m_cDataType._scale);
		else m_uiDataLength = FileCommon::DataManager::getFixedCommonDataArchiveSize(m_cDataType._name);
	}
	else
	{
		//initialize length : uncompress or element count length
		m_uiDataLength = Utility::m_FieldLengthArchiveSize;
		if(isArray())	//simple variable length
		{
			Utility::ElementNum uiElementNum = getElementNumber();
			//variable array
			if(FileCommon::DataManager::isVariable(m_cElementType._name))
			{
				//header size
				Utility::Size iSize = VariableData::getPositionSize(uiElementNum, true);
				//uncompressed length
				iSize += Utility::m_FieldLengthArchiveSize * uiElementNum;
				m_uiDataLength += iSize;
			}
			else //fixed array
			{
				m_uiDataLength += FieldData::getBitmapSize(uiElementNum)
								+ (uiElementNum * getFixedElementLength());				
			}
		}
	}
	
	return m_uiDataLength;
}

//	FUNCTION public
//	Record2::FieldInfo::getFieldSize -- getFieldSize
//
//	NOTES
//		-- The area size of one field (field kind + field value) is returned.
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

Utility::FieldLength 
FieldInfo::
getLength()
{
	return (m_uiDataLength != 0) 
	? 
	m_uiDataLength
	:
	makeKnowableLength();	//make knowable length first
}

//	FUNCTION public
//	Record2::FieldInfo::getElementNumber -- getElementNumber
//
//	NOTES
//		-- getElementNumber
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

Utility::ElementNum
FieldInfo::
getElementNumber() const
{
	//if had set real element count
	return (m_cAdditionInfo.m_uiDataElementNum != 0) 
		? 
		m_cAdditionInfo.m_uiDataElementNum
		:
		m_uiSchemaElementNum;
}

//	FUNCTION public
//	Record2::FieldInfo::getElementLength -- get element length (fixed array)
//
//	NOTES
//		-- get element length (fixed array)
//
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

Utility::FieldLength 
FieldInfo::
getFixedElementLength() const
{
	; _SYDNEY_ASSERT(isArray());
	; _SYDNEY_ASSERT(!isVariableArray());

	//is fixed array
	if (m_cElementType._name == Common::DataType::Decimal)
		return	Common::DecimalData::getDumpSizeBy(m_cElementType._length, m_cElementType._scale);
	else //if(!FileCommon::DataManager::isVariable(m_cElementType._name))
		return FileCommon::DataManager::getFixedCommonDataArchiveSize(m_cElementType._name);
	//else return m_cElementType._length;

}

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
