// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Tree.h --
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

#ifndef __SYDNEY_ARRAY_TREE_H
#define __SYDNEY_ARRAY_TREE_H

#include "Array/Module.h"

#include "Array/AutoPointer.h"
#include "Array/Compare.h"
#include "Array/Data.h"
#include "Array/TreeHeader.h"

#include "PhysicalFile/Types.h"		//PhysicalFile::PageID

#include "ModTypes.h"

_SYDNEY_BEGIN
_SYDNEY_ARRAY_BEGIN

class FileID;

//
//	CLASS
//	Array::Tree --
//
//	NOTES
//
//
class Tree
{
public:
	//
	//	STRUCT
	//	Array::Tree::Type --
	//
	struct Type
	{
		enum Value
		{
			Data,
			NullData,
			NullArray,
			Undefined
		};
	};
	
	//コンストラクタ
	Tree(const FileID& cFileID_);
	//デストラクタ
	virtual ~Tree();
	
	// Initialize header
	void initializeHeader() { m_pHeader->initialize(); }

	// Initialize
	virtual void initialize();

	// Get the type of tree.
	virtual Type::Value getType() const = 0;

	//
	// Data
	//
	
	// Get Data class.
	const Data& getNodeData() const { return m_cNodeData; }
	const Data& getLeafData() const { return m_cLeafData; }

	// Set the buffer to DataArrayData.
	virtual void setKey(Common::DataArrayData& cData_,
						 const ModUInt32* p) const;
	// Set Common::Data to DataArrayData.
	virtual void setKey(Common::DataArrayData& cData_,
						 const Common::Data::Pointer pValue_) const;

	// Get RowID from the buffer.
	unsigned int getRowID(const ModUInt32* p,
						  Common::DataArrayData& cTuple_) const;
	// Get RowID by BitSet from the buffer.
	unsigned int getRowIDByBitSet(const ModUInt32* p,
								  Common::BitSet& cBitSet_) const;
	// Set RowID to DataArrayData.
	void setRowID(Common::DataArrayData& cData_,
				  unsigned int uiRowID_) const
		{ setDefaultData(cData_, getRowIDPosition(), uiRowID_); }

	// Set Index to the DataArrayData.
	virtual void setIndex(Common::DataArrayData& cData_,
						  int iIndex_) const;
	
	// Get PageID from the buffer.
	PhysicalFile::PageID getPageID(const ModUInt32* p) const;
	// Set PageID to the buffer.
	void setPageID(AutoPointer<ModUInt32>& pEntry_,
				   ModSize uiSize_,
				   PhysicalFile::PageID uiPageID_) const;

	// Make Common::DataArrayData.
	void makeDataArrayData(Common::DataArrayData& cData_) const;

	// Make an entry from DataArrayData.
	AutoPointer<ModUInt32>
	makeLeafEntry(const Common::DataArrayData& cData_, ModSize& uiSize_) const;

	// Make an entry from the buffer.
	AutoPointer<ModUInt32>
	makeNodeEntry(const ModUInt32* pEntry_, ModSize& uiSize_) const;

	//
	// Compare
	//

	// Is this compare class for unique search?
	bool isForUniqueSearch(const Compare& cCompare_) const
		{ return cCompare_.isUnique(); }
	
	// Get Compare class.
	const Compare& getCompare() const { return m_cCompare; }
	const Compare& getKeyCompare() const { return m_cKeyCompare; }

	//
	// Header
	//
	void setHeader(TreeHeader* pTreeHeader_) { m_pHeader = pTreeHeader_; }

	// The count of the entry.
	ModSize getCount() const { return m_pHeader->getCount(); }
	void incrementCount() { m_pHeader->incrementCount(); }
	void decrementCount() { m_pHeader->decrementCount(); }
	
	// The step of the Tree.
	ModSize getStepCount() const { return m_pHeader->getStepCount(); }
	void incrementStepCount() { m_pHeader->incrementStepCount(); }
	void decrementStepCount() { m_pHeader->decrementStepCount(); }

	// RootPageID
	PhysicalFile::PageID getRootPageID() const
		{ return m_pHeader->getRootPageID(); }
	void setRootPageID(PhysicalFile::PageID uiPageID_)
		{ m_pHeader->setRootPageID(uiPageID_); }

	// LeftLeafPageID
	PhysicalFile::PageID getLeftLeafPageID() const
		{ return m_pHeader->getLeftLeafPageID(); }
	void setLeftLeafPageID(PhysicalFile::PageID uiPageID_)
		{ m_pHeader->setLeftLeafPageID(uiPageID_); }

	// RightLeafPageID
	PhysicalFile::PageID getRightLeafPageID() const
		{ return m_pHeader->getRightLeafPageID(); }
	void setRightLeafPageID(PhysicalFile::PageID uiPageID_)
		{ m_pHeader->setRightLeafPageID(uiPageID_); }
	
	// Get the avarage number of the entries par one tuple.
	virtual double getAverageEntryCount(ModSize uiTupleCount_,
										ModSize uiOneEntryTupleCount_) const;
	
protected:
	// Get the default type.
	Data::Type::Value getDefaultType() const
		{ return Data::Type::UnsignedInteger; }
	// Get the default size [unit size].
	ModSize getDefaultSize() const;

	// Set default data to DataArrayData.
	void setDefaultData(Common::DataArrayData& cData_,
						int iPosition_,
						unsigned int uiData_) const;
	
	// FileID
	const FileID& m_cFileID;
	
	// Data class
	Data m_cNodeData;
	Data m_cLeafData;

	// Compare class
	Compare m_cCompare;
	Compare m_cKeyCompare;

	// Header
	TreeHeader* m_pHeader;

private:
	// Get the number of the fields.
	virtual int getFieldCount() const = 0;
	// Get the number of the key fields.
	virtual int getKeyFieldCount() const = 0;
	// Get the type of the field.
	virtual Data::Type::Value getFieldType(int iPosition_) const;
	// Get the size of the field. [unit size]
	virtual ModSize getFieldSize(int iPosition_) const;
	// is the size of the field fixed?
	virtual bool isFixedField(int iPosition_) const;

	// Get the PageID type.
	Data::Type::Value getPageIDType() const { return getDefaultType(); }
	// Get the PageID size [unit size]. For Node.
	ModSize getPageIDSize() const { return getDefaultSize(); }

	// Get the position of Value.
	virtual int getValuePosition() const;
	// Get the position of RowID.
	virtual int getRowIDPosition() const = 0;
	// Get the position of RowID.
	virtual int getIndexPosition() const;
	// Get the position of RowID.
	int getPageIDPosition() const { return getFieldCount(); }

	// Get the entry size [unit size]. For Leaf.
	virtual ModSize getLeafEntrySize(const Common::DataArrayData& cData_) const
		{ return m_cLeafData.getFixedSize(); }
	// Get the entry size [unit size]. For Node.
	virtual ModSize getNodeEntrySize(const ModUInt32* pEntry_) const
		{ return m_cNodeData.getFixedSize(); }

	// Get the field parameter in the point of FileID.
	virtual void getFieldParameter(int& iPosition_, bool& isArray_) const;
};

_SYDNEY_ARRAY_END
_SYDNEY_END

#endif //__SYDNEY_ARRAY_TREE_H

//
//	Copyright (c) 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
