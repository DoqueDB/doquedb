// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FieldData.h --
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

#ifndef __SYDNEY_RECORD2_FIELDDATA_H
#define __SYDNEY_RECORD2_FIELDDATA_H

#include "Common/DataArrayData.h"

#include "Record2/FileID.h"

_SYDNEY_BEGIN

_SYDNEY_RECORD2_BEGIN

//
//	CLASS
//	Record2::FieldData -- 
//		FieldData
//	NOTES
// Class for common fields operation. It's super class including fixed and variable data.
//
class FieldData : public Common::Object
{
public:

    //constructor
    FieldData(const FileID::TargetFields& cTargetFields_);

	//destructor
	~FieldData();

    ////read
    //virtual void read(Common::DataArrayData* pTuple_, 
				//const char* pPointer_, 
				//Utility::AreaSize uiSize_ = 0) = 0;

    ////update
    //virtual void update(const Common::DataArrayData* pTuple_, 
				//char* pPointer_, 
				//Utility::AreaSize uiSize_ = 0) = 0;

	//get FieldInfos
	const FileID::TargetFields& getTargetFields() const;

	//field num operation
	//void setFieldNum(const Utility::FieldNum iFieldNum_) ;
	Utility::FieldNum getFieldNum() const;

	// A necessary number of bytes for bitmap is obtained. 
	static Utility::Size getBitmapSize(Utility::Size uiCount_);

	// read fixed length field value
	static const char* readFixedField(const char* pAreaPointer_,
										const DataManagerType& cDataType_,
										Common::Data& cCommonData_);

	// write field value. (Data should be a fixed length. )
	static char* writeFixedField(char* pAreaPointer_,
								 const DataManagerType& cDataType_,
								 const Common::Data& cCommonData_);

	// The size before and after the compression of the variable length field is obtained. 
	static void getVariableSize(const Common::Data& cData_,
								const DataManagerType& cDataType_,
								Utility::FieldLength& uiUncompressedSize_,
								Utility::FieldLength& uiFieldLength_);
protected:

	//read/write Null Bitmap
	const char* readNullBitMap(const char* pPointer_, 
				const Utility::FieldNum uiCount_);
	char* writeNullBitMap(char* pPointer_, 
				const Utility::FieldNum uiCount_);
	//makeNullBitMap from data
	void makeNullBitMap(const Common::DataArrayData* pData_, 
				bool bIsArray_ = false, bool bForUpdate_ = false);

    //each field correspond to a bit
    bool* m_pNulls;

    //field or element count
    Utility::FieldNum m_iFieldNum;
    //fields meta
    //const FileID::TargetFields* m_pTargetFields;
	const FileID::TargetFields& m_cTargetFields;

};

//get FieldInfos
inline
const FileID::TargetFields&
FieldData::
getTargetFields() const
{ 
	return m_cTargetFields;
}

//getFieldNum
inline
Utility::FieldNum 
FieldData::
getFieldNum() const
{ 
	return m_iFieldNum;
}

_SYDNEY_RECORD2_END
_SYDNEY_END

#endif // __SYDNEY_RECORD2_FIELDDATA_H

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
