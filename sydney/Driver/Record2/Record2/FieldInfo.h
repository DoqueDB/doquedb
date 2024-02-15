// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FieldInfo.h --
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

#ifndef __SYDNEY_RECORD2_FIELDINFO_H
#define __SYDNEY_RECORD2_FIELDINFO_H

#include "FileCommon/DataManager.h"
#include "Record2/Module.h"
#include "Record2/Utility.h"

_SYDNEY_BEGIN

_SYDNEY_RECORD2_BEGIN

// Data type used in record file
typedef	FileCommon::DataManager::DataType DataManagerType;

//
//	CLASS
//	Record2::FieldInfo -- 
//		FieldInfo
//	NOTES
// Stored filed information instead of metadata.
//
class FieldInfo
{
public:

	struct AdditionInfo	//Additional information
	{
		// array: Number of elements
		// Usually: The size before it compresses it. 
		union 
		{
			Utility::ElementNum m_uiDataElementNum;		// Number of array elements
			Utility::FieldLength m_iUncompressedSize;	// Size before it compresses it
		};
	};

	//default constructor
	//FieldInfo();

	//constructor
	FieldInfo(const int iFieldID_, 
			const Common::DataType::Type& cDataType_,
			const Utility::FieldLength uiLength_,
			const Common::StringData::EncodingForm::Value& eEncodingForm_,
			const bool bIsVariable_ = false, 
			const bool bCompress_ = false);

	// destructor
	~FieldInfo();
	
    //get identity of this field
    int getFieldID() const;

    //get identity of this field
	void setFieldIndex(const int iFieldIndex_);
    int getFieldIndex() const;

    //get the position of this field
    void setDataIndex(const int iDataIndex_);
    int getDataIndex() const;

 	//setElementInfo
	void setElementInfo(int iType_, 
						int iLength_, 
						int iCount_, 
						Common::StringData::EncodingForm::Value& eEncodingForm_);

    //The field data type of an arbitrary field is acquired. 
    const DataManagerType& getDataType() const;

    //The data type of the array element of an arbitrary field is acquired. 
    const DataManagerType& getElementType() const;

	//selected operation
	void setIsSelected(bool bSelected_);
	bool isSelected() const;

	//for decimal operation
	void setFraction(Utility::FieldLength uiFraction_);
	Utility::FieldLength getFraction() const;

	//variable operation
	bool isVariable() const;

    //isArray
    bool isArray() const;

    //Is it an array of a variable-length element?
    bool isVariableArray() const;

    //Whether it stores it by compressing it or not?
    bool hasCompress() const;

    //set field length, if bSchema_ == false, set really length
	void setLength(const Utility::FieldLength uiLength_, bool bSchema_ = false);
	//get data/schema length
    Utility::FieldLength getLength();

	//including 2 parts: element count and uncompressed size 
	void setAdditionInfo(const AdditionInfo uiAdditionInfo_);
	const AdditionInfo& getAdditionInfo() const;

    //getElementNumber
    Utility::ElementNum getElementNumber() const;

	//get element length (fixed array)
    Utility::FieldLength getFixedElementLength() const;

private:

	//make knowable(fixed) length, 
	Utility::FieldLength makeKnowableLength();

	//identity of this field
    int m_iFieldID;

	//identity of this field
    int m_iFieldIndex;

    //position of this field
    int m_iDataIndex;

    //datatype of this field
    DataManagerType m_cDataType;

	//data really length
	Utility::FieldLength m_uiDataLength;

    //if it is array, the element type
    DataManagerType m_cElementType;

    //how many elements
	Utility::ElementNum m_uiSchemaElementNum;

	//ddditional information
	AdditionInfo m_cAdditionInfo;

    //has compress?
    bool m_bCompress;

	//is variable? needn't
	bool m_bIsVariable;

	//is selected, used in select and update
	bool m_bSelected;
};

//getFieldID
inline
int
FieldInfo::
getFieldID() const
{
	return m_iFieldID;
}

//setFieldIndex
inline
void
FieldInfo::
setFieldIndex(const int iFieldIndex_)
{
	m_iFieldIndex = iFieldIndex_;
}

//getFieldIndex
inline
int
FieldInfo::
getFieldIndex() const
{
	return m_iFieldIndex;
}

//set the position of this field
inline
void
FieldInfo::
setDataIndex(const int iDataIndex_)
{
	m_iDataIndex = iDataIndex_;
}

//get the position of this field
inline
int
FieldInfo::
getDataIndex() const
{
	return m_iDataIndex;
}

//The field data type of an arbitrary field is acquired. 
inline 
const DataManagerType&
FieldInfo::
getDataType() const
{
	return m_cDataType;
}

//The data type of the array element of an arbitrary field is acquired.
inline 
const DataManagerType&
FieldInfo::
getElementType() const
{
	return m_cElementType;
}

//isSelected
inline
void
FieldInfo::
setIsSelected(bool bSelected_)
{
	m_bSelected = bSelected_;
}

//isSelected
inline
bool
FieldInfo::
isSelected() const
{
	return m_bSelected;
}

//isVariable
inline
bool
FieldInfo::
isVariable() const
{
	return m_bIsVariable;
}

//Whether it stores it by compressing it or not?
inline
bool
FieldInfo::
hasCompress() const
{
	return m_bCompress;
}

//setAdditionInfo
inline
void 
FieldInfo::
setAdditionInfo(const AdditionInfo uiAdditionInfo_)
{
	m_cAdditionInfo = uiAdditionInfo_;
}

//getAdditionInfo
inline
const FieldInfo::AdditionInfo& 
FieldInfo::
getAdditionInfo() const
{
	return m_cAdditionInfo;
}

_SYDNEY_RECORD2_END
_SYDNEY_END

#endif // __SYDNEY_RECORD2_FIELDINFO_H

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
