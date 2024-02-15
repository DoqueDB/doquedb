// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataTree.h --
// 
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_ARRAY_DATATREE_H
#define __SYDNEY_ARRAY_DATATREE_H

#include "Array/Module.h"

#include "Array/Tree.h"

_SYDNEY_BEGIN
_SYDNEY_ARRAY_BEGIN

//
//	CLASS
//	Array::Tree --
//
//	NOTES
//
//
class DataTree : public Tree
{
public:
	//コンストラクタ
	DataTree(const FileID& cFileID_);
	//デストラクタ
	virtual ~DataTree();

	// Get the type of tree.
	Type::Value getType() const { return Type::Data; }

	//
	// Data
	//
	
	// Set the buffer to DataArrayData.
	void setKey(Common::DataArrayData& cData_,
				 const ModUInt32* p) const;
	// Set Common::Data to DataArrayData.
	void setKey(Common::DataArrayData& cData_,
				 const Common::Data::Pointer pElement_) const;

	// Set Index to DataArrayData.
	void setIndex(Common::DataArrayData& cData_, int iIndex_) const;

private:
	//
	//	CONST
	//	FieldCount --
	//
	static const int FieldCount = 3;

	//
	//	CONST
	//	KeyFieldCount --
	//
	static const int KeyFieldCount = 1;

	//
	//	STRUCT
	//	FieldPosition -- The position of the field in the point of DataTree
	//
	struct FieldPosition
	{
		enum Value
		{
			Key = 0,
			RowID,
			Index
		};
	};

	// Get the count of the fields.
	int getFieldCount() const { return FieldCount; }
	// Get the count of the key fields.
	int getKeyFieldCount() const { return KeyFieldCount; }
	// Get the type of the field.
	Data::Type::Value getFieldType(int iPosition_) const;
	// Get the size of the field. [unit size]
	ModSize getFieldSize(int iPosition_) const;
	// is the size of the field fixed?
	bool isFixedField(int iPosition_) const;

	// Get the position of Value.
	int getValuePosition() const { return FieldPosition::Key; }
	// Get the position of RowID.
	int getRowIDPosition() const { return FieldPosition::RowID; }
	// Get the position of Index.
	int getIndexPosition() const { return FieldPosition::Index; }

	// Get the entry size [unit size]. For Leaf.
	ModSize getLeafEntrySize(const Common::DataArrayData& cData_) const;
	// Get the entry size [unit size]. For Node.
	ModSize getNodeEntrySize(const ModUInt32* pEntry_) const;

	// Get the field parameter in the point of FileID.
	void getFieldParameter(int& iPosition_, bool& isArray_) const;
};

_SYDNEY_ARRAY_END
_SYDNEY_END

#endif //__SYDNEY_ARRAY_DATATREE_H

//
//	Copyright (c) 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
