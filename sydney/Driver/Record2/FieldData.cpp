// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FieldData.cpp -- 
// 
// Copyright (c) 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"

#include "Common/CompressedData.h"

#include "Record2/FieldData.h"


_SYDNEY_USING

_SYDNEY_RECORD2_USING

//	FUNCTION public
//	Record2::FieldData::FieldData -- constructor
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

FieldData::
FieldData(const FileID::TargetFields& cTargetFields_)
	 :m_cTargetFields(cTargetFields_),
 	 m_pNulls(0)
{
	//may be 
	m_iFieldNum = cTargetFields_.getSize();

	//m_cFieldInfos = m_pTargetFields->m_vecFieldInfo;
	//m_iFieldNum = m_pTargetFields->m_uiFieldNum;
}

//	FUNCTION public
//	Record2::FieldData::~FieldData -- destructor
//
//	NOTES
//		-- destructor
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

FieldData::
~FieldData()
{
	if(m_pNulls)
	{
		delete[] m_pNulls;
		m_pNulls = 0;
	}
}

//
//	FUNCTION public
//	Record2::Utility::readFixedField -- Data corresponding to the fixed-length field is read. 
//
//	NOTES
//
//	ARGUMENTS
//	const char* pAreaPointer_
//		Read address
//	const DataManagerType&		cDataType_
//		Data kind of data corresponding to field(what acquired from meta data etc.)
// 	Common::Data& cCommonData_
//		The return value is put. 
//
//	RETURN
//	Pointer that indicates the following of read data
//
//	EXCEPTIONS

// static
const char*
FieldData::readFixedField(const char* pPointer_,
					  const DataManagerType& cDataType_,
					  Common::Data& cCommonData_)
{
	; _SYDNEY_ASSERT(pPointer_);
	; _SYDNEY_ASSERT(cCommonData_.getType() == cDataType_._name);

	const char*	pPointer = pPointer_;

	if (Common::Data::isFixedSize(cDataType_._name))
	{
		// If the type is a fixed length, it only has to read as it is. 
		pPointer = FileCommon::DataManager::readFixedCommonData(cCommonData_, pPointer);
	}
	else
	{
		/*
		// If the type is variable-length, it is data after the size. 
		Utility::FieldLength uiLength;
		Os::Memory::copy(&uiLength, pPointer, Utility::m_FieldLengthArchiveSize);
		pPointer += Utility::m_FieldLengthArchiveSize;

		// A special method is used at the character string. 
		if (cDataType_._name == Common::DataType::String)
		{
			FileCommon::DataManager::readStringData(
				cCommonData_, cDataType_._encodingForm, pPointer, uiLength);
		}
		else FileCommon::DataManager::readCommonData(cCommonData_, pPointer, uiLength);

		pPointer += cDataType_._length;
		*/
	}
	
	return pPointer;
}

//
//	FUNCTION public
//	Record2::FieldData::writeFixedField -- Data corresponding to the field is written. 
//
//	NOTES
//	The field value is written (Data should be a fixed length). 
//	Information being written in the field is "Fixed length data. "
//
//	   ┌─── Field of fixed length data ───┐
//	   │┌─────────────────┐│
//	   ││Fixed length data               ││
//	   │└─────────────────┘│
//	   └───────────────────┘
//
//	ARGUMENTS
//	char* pAreaPointer_
//		Pointer that points at position in which data is written
//	Common::Data* 						pCommonData_
//		Value written in field
//
//	RETURN
//	Pointer that indicates the following of written data
//
//	EXCEPTIONS
//

// static
char*
FieldData::writeFixedField(char* pAreaPointer_,
					   const DataManagerType& cDataType_,
					   const Common::Data& cCommonData_)
{
	; _SYDNEY_ASSERT(pAreaPointer_);

	char*	pPointer = pAreaPointer_;

	if (cCommonData_.isNull())
	{
		// Only the pointer advances it. 
		pPointer += cDataType_._length;
	}
	else if (cDataType_.isVariable()) 
	{
		/*
		// If the type is variable-length, it is data after the size. 
		; _SYDNEY_ASSERT(!(cCommonData_.getFunction() & Common::Data::Function::Compressed));

		Utility::FieldLength uiLength;
		if (cDataType_._name == Common::DataType::String)
		{
			uiLength = _SYDNEY_DYNAMIC_CAST(const Common::StringData&,
										 cCommonData_).getDumpSize(
										 cDataType_._encodingForm);
		}
		else uiLength = cCommonData_.getDumpSize();

		if (uiLength > cDataType_._length) 
		{
			SydErrorMessage << "Can't write over-length data. max=" << cDataType_._length
							<< " length=" << uiLength << ModEndl;
			_SYDNEY_THROW0(Exception::BadArgument);
		}

		Os::Memory::copy(pPointer, &uiLength, Utility::m_FieldLengthArchiveSize);
		pPointer += Utility::m_FieldLengthArchiveSize;

		if (cDataType_._name == Common::DataType::String)
		{
			FileCommon::DataManager::writeStringData(
				cCommonData_, cDataType_._encodingForm, pPointer);
		}
		else FileCommon::DataManager::writeCommonData(cCommonData_, pPointer);

		pPointer += cDataType_._length;
		*/
	} 
	else
	{
		// The fixed length data is written. 
		pPointer = FileCommon::DataManager::writeFixedCommonData(cCommonData_, pPointer);
	}

	return pPointer;
}

//	FUNCTION public
//	Record2::FieldData::getVariableSize -- 
//		The size before and after the compression of the variable length field is obtained. 
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& pData_
//		Examined data
//	Record2::FieldData::FieldLength& uiUncompressedSize_
//	Record2::FieldData::FieldLength& uiFieldLength_
//		[OUT] Size before and after compression
//
//	RETURN
// 		None
//
//	EXCEPTIONS

//static
void
FieldData::
getVariableSize(const Common::Data& cData_,
				const DataManagerType& cDataType_,
				Utility::FieldLength& uiUncompressedSize_,
				Utility::FieldLength& uiFieldLength_)
{
	Common::Data::Function::Value eFunction = Common::Data::Function::Compressed;
	if (!(cData_.getFunction() & eFunction)) 
	{
		// It is not compressed. 
		if (cData_.getType() == Common::DataType::String)
		{
			const Common::StringData& cStringData
				= _SYDNEY_DYNAMIC_CAST(const Common::StringData&, cData_);
			
			//; _SYDNEY_ASSERT(cStringData.getEncodingForm() == cDataType_._encodingForm);

			uiFieldLength_ = uiUncompressedSize_
				= cStringData.getDumpSize(cDataType_._encodingForm);
		}
		else
		{
			uiFieldLength_ = uiUncompressedSize_ = cData_.getDumpSize();
		}

		return;
	}
	; _SYDNEY_ASSERT(cData_.getFunction() & eFunction);

	// The size before it compresses it is obtained. 
	// Because static_cast cannot be used here, dynamic_cast is always used. 
	const Common::CompressedData* pCompressedData =
			dynamic_cast<const Common::CompressedData*>(&cData_);

	; _SYDNEY_ASSERT(pCompressedData);

	uiUncompressedSize_ = pCompressedData->getValueSize();
	uiFieldLength_ = pCompressedData->getCompressedSize();
}


//	FUNCTION private
//	Record2::FieldData::setNullBitMap --
//
//	NOTES
//		No adjustment of the data type is checked at the same time as looking for NullData. 
//
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void 
FieldData::
makeNullBitMap(const Common::DataArrayData* pData_, bool bIsArray_, bool bForUpdate_)
{
	//only used in inserting mode
	; _SYDNEY_ASSERT(pData_);

	//get null bi count
	Utility::FieldNum uiCount = bIsArray_ ? pData_->getCount() : m_cTargetFields.getSize();

	//initialize memory space
	if(!bForUpdate_)
	{
		if(m_pNulls) delete[] m_pNulls;
		m_pNulls = new bool[uiCount];
	}

	; _SYDNEY_ASSERT(m_pNulls);

	for(Utility::FieldNum i = 0; i < uiCount; ++i)
	{
		//get the data index
		int index = bIsArray_ ? i : m_cTargetFields[i]->getDataIndex(); 

		if(!bForUpdate_ || m_cTargetFields[i]->isSelected())
		{	
			if(i == 0 && !bIsArray_ || 
				!pData_->getElement(index)->isNull())
			{
				m_pNulls[i] = false;
			}
			else
			{
				m_pNulls[i] = true;
			}
		}
	}
}

//	FUNCTION public
//	Record2::FieldData::readNullBitMap -- readNullBitMap
//
//	NOTES
//		-- readNullBitMap
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

const char*
FieldData::
readNullBitMap(const char* pPointer_, const Utility::FieldNum uiCount_)
{
	; _SYDNEY_ASSERT(pPointer_);

	const char* pBitmap = pPointer_;
	// Null Bitmap is stored from the head of the object. 
	if(m_pNulls) delete[] m_pNulls;
	m_pNulls = new bool[uiCount_];

	//because the first i == 0 but needn't += 1
	pBitmap--;
	for(Utility::FieldNum i = 0; i < uiCount_; ++i )
	{
		pBitmap += !(i % 8) ? 1 : 0;
		m_pNulls[i] = *pBitmap & (0x80 >> (i % 8));
	}
	//for next byte will store other data
	pBitmap++;

	return pBitmap;
}

//	FUNCTION public
//	Record2::FieldData::writeNullBitMap -- writeNullBitMap
//
//	NOTES
//		-- writeNullBitMap
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

char*
FieldData::
writeNullBitMap(char* pPointer_, const Utility::FieldNum uiCount_)
{
	; _SYDNEY_ASSERT(pPointer_ && m_pNulls);

	char* pBitmap = pPointer_;
	// Null Bitmap is stored from the head of the object. 

	//because the first i == 0 but needn't += 1
	pBitmap--;
	for(Utility::FieldNum i = 0; i < uiCount_; ++i )
	{
		pBitmap += !(i % 8) ? 1 : 0;
		if(m_pNulls[i]) *pBitmap |= (0x80 >> (i % 8));
		else *pBitmap &= ~(0x80 >> (i % 8));
	}
	//for next byte will store other data
	pBitmap++;
	
	return pBitmap;
}

//	FUNCTION public
//	Record2::FieldData::getBitmapSize -- getBitmapSize
//
//	NOTES
//		-- getBitmapSize
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

Utility::Size
FieldData::
getBitmapSize(Utility::Size uiCount_)
{
	return (uiCount_ + 7) / 8;
}

//
//	Copyright (c) 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
