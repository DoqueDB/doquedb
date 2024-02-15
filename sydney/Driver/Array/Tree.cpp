// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Tree.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Array";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Array/FileID.h"
#include "Array/Tree.h"

#include "Common/Assert.h"
#include "Exception/Unexpected.h"

_SYDNEY_USING
_SYDNEY_ARRAY_USING

//
//	FUNCTION public
//	Array::Tree::Tree -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
Tree::Tree(const FileID& cFileID_)
	: m_cFileID(cFileID_)
{
}

//
//	FUNCTION public
//	Array::Tree::~Tree -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Tree::~Tree()
{
}

//
//	FUNCTION public
//	Array::Tree::initialize --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Tree::initialize()
{
	// Set the count of the fields.
	const int count = getFieldCount();
	m_cLeafData.setTypeCount(count);
	m_cNodeData.setTypeCount(count + 1);	// +1 is for PageID.
	m_cCompare.setTypeCount(count);
	m_cCompare.setUnique(true);
	const int keyCount = getKeyFieldCount();
	m_cKeyCompare.setTypeCount(keyCount);
	m_cKeyCompare.setUnique(false);

	// Set the type of fields.
	ModSize fixedSize = 0;
	ModSize variableSize = 0;
	for (int i = 0; i < count; ++i)
	{
		const Data::Type::Value type = getFieldType(i);
		m_cLeafData.setType(type, i);
		m_cNodeData.setType(type, i);
		m_cCompare.setType(type, i);
		if (i < keyCount)
		{
			m_cKeyCompare.setType(type, i);
		}

		const ModSize size = getFieldSize(i);
		if (isFixedField(i) == true)
		{
			fixedSize += size;
		}
		else
		{
			variableSize += size;
		}
	}
	m_cNodeData.setType(getPageIDType(), count);	// For PageID

	// Set the total of the fixed fields and the variable fields. [unit size]
	m_cLeafData.setSize(fixedSize, variableSize);
	m_cNodeData.setSize(fixedSize + getPageIDSize(), variableSize);
}

//
//	FUNCTION public
//	Array::Tree::setKey --
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32* p_
//		データ
//
//	RETURN
//
//	EXCEPTIONS
//
void
Tree::setKey(Common::DataArrayData& cData_, const ModUInt32* p) const
{
	// Nothing to do.
}

//
//	FUNCTION public
//	Array::Tree::setKey -- Set Tuple's Value to Common::DataArrayData
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cData_
//		データ
//
//	RETURN
//
//	EXCEPTIONS
//
void
Tree::setKey(Common::DataArrayData& cData_,
			 const Common::Data::Pointer pValue_) const
{
	// Nothing to do
}

//
//	FUNCTION public
//	Array::Tree::getRowID --
//
//	NOTES
//	Get the RowID from the Leaf's entry which is pointed by 'p'.
//
//	ARGUMENTS
//	const ModUInt32* p
//		メモリ
//	Common::DataArrayData& cTuple_
//		RowID is set at the top of the cTuple_.
//
//	RETURN
//	unsigned int
//
//	EXCEPTIONS
//
unsigned int
Tree::getRowID(const ModUInt32* p, Common::DataArrayData& cTuple_) const
{
	// For Leaf
	return static_cast<unsigned int>(
		m_cLeafData.getModUInt32Data(p, getRowIDPosition(), cTuple_));
}

//
//	FUNCTION public
//	Array::Tree::getRowIDByBitSet --
//
//	NOTES
//	Get the RowID from the Leaf's entry which is pointed by 'p'.
//
//	ARGUMENTS
//	const ModUInt32* p
//		メモリ
//	Common::BitSet& cBitSet_
//		RowID is set in the cBitSet_.
//
//	RETURN
//	unsigned int
//
//	EXCEPTIONS
//
unsigned int
Tree::getRowIDByBitSet(const ModUInt32* p, Common::BitSet& cBitSet_) const
{
	// For Leaf
	return static_cast<unsigned int>(
		m_cLeafData.getModUInt32DataByBitSet(p, getRowIDPosition(), cBitSet_));
}

//
//	FUNCTION public
//	Array::Tree::setIndex --
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cData_
//		データ
//
//	RETURN
//
//	EXCEPTIONS
//
void
Tree::setIndex(Common::DataArrayData& cData_, int iIndex_) const
{
	// Nothing to do.
}

//
//	FUNCTION public
//	Array::Tree::getPageID -- ページIDを得る
//
//	NOTES
//	Get the PageID from the Node's entry which is pointed by 'p'.
//
//	ARGUMENTS
//	const ModUInt32* p
//		メモリ
//
//	RETURN
//	PhysicalFile::PageID
//
//	EXCEPTIONS
//
PhysicalFile::PageID
Tree::getPageID(const ModUInt32* p) const
{
	// For Node
	
	// PageID is stored in the Node's last field.
	return static_cast<PhysicalFile::PageID>(
		m_cNodeData.getModUInt32Data(p, getPageIDPosition()));
}

//
//	FUNCTION public
//	Array::Tree::setPageID --
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32* pEntry_
//		データ
//	ModSize uiSize_ [unit size]
//		The size of the entry which include PageID
//	PhysicalFile::PageID uiPageID_
//		Child's PageID
//
//	RETURN
//
//	EXCEPTIONS
//
void
Tree::setPageID(AutoPointer<ModUInt32>& pEntry_,
				ModSize uiSize_,
				PhysicalFile::PageID uiPageID_) const
{
	// For Node

	; _SYDNEY_ASSERT(uiSize_ - getPageIDSize() > 0);
	
	ModUInt32* pPageID = pEntry_.get() + uiSize_ - getPageIDSize();
	syd_reinterpret_cast<Data::UnsignedInteger*>(pPageID)->m_value = uiPageID_;
}

//
//	FUNCTION public
//	Array::Tree::makeDataArrayData --
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData& cData_
//
//	RETURN
//
//	EXCEPTIONS
//
void
Tree::makeDataArrayData(Common::DataArrayData& cData_) const
{
	// For Leaf
	
	const int count = m_cLeafData.getTypeCount();
	cData_.reserve(count);

	for (int i = 0; i < count; ++i)
	{
		// Get the field parameter in the point of FileID
		int iPosition = i;
		bool isArray = false;
		getFieldParameter(iPosition, isArray);

		// Make a Common::Data.
		Common::Data::Pointer pData = Data::makeData(
			m_cFileID, iPosition, m_cLeafData.getType(i), isArray);

		// Set the Common::Data to Common::DataArrayData.
		cData_.setElement(i, pData);
	}
}

//
//	FUNCTION public
//	Array::Tree::makeLeafEntry --
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cValue_
//		データ
//	ModSize& uiSize_ [unit size]
//
//	RETURN
//	Array::AutoPointer<ModUInt32>
//		データ
//
//	EXCEPTIONS
//
AutoPointer<ModUInt32>
Tree::makeLeafEntry(const Common::DataArrayData& cData_, ModSize& uiSize_) const
{
	// For Leaf

	// Allocate buffer.
	uiSize_ = getLeafEntrySize(cData_);
	AutoPointer<ModUInt32> p = static_cast<ModUInt32*>(
		Os::Memory::allocate(uiSize_ * sizeof(ModUInt32)));

	// Dump the data.
	m_cLeafData.dump(p, cData_);

	return p;
}

//
//	FUNCTION public
//	Array::Tree::makeNodeEntry --
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32* pEntry_
//		データ
//	ModSize& uiSize_ [unit size]
//
//	RETURN
//	Array::AutoPointer<ModUInt32>
//		データ
//
//	EXCEPTIONS
//
AutoPointer<ModUInt32>
Tree::makeNodeEntry(const ModUInt32* pEntry_, ModSize& uiSize_) const
{
	// For Node

	// Allocate buffer.
	uiSize_ = getNodeEntrySize(pEntry_);
	AutoPointer<ModUInt32> p
		= syd_reinterpret_cast<ModUInt32*>(
			Os::Memory::allocate(uiSize_ * sizeof(ModUInt32)));

	// Copy the entry except PageID.
	Os::Memory::copy(p, pEntry_,
					 (uiSize_ - getPageIDSize()) * sizeof(ModUInt32));
	
	return p;
}

//
//	FUNCTION public
//	Array::Tree::getAverageEntryCount --
//		Get the avarage number of the entries par one tuple
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
double
Tree::getAverageEntryCount(ModSize uiTupleCount_,
						   ModSize uiOneEntryTupleCount_) const
{
	; _SYDNEY_ASSERT(uiTupleCount_ >= uiOneEntryTupleCount_);
	
	const ModSize entry = m_pHeader->getCount();
	const ModSize tuple = uiTupleCount_ - uiOneEntryTupleCount_;

	double result = 1.0;
	if (tuple != 0)
	{
		result = entry / tuple;
	}
	return result;
}

//
//	FUNCTION protected
//	Array::Tree::getDefaultSize --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	ModSize	[unit size]
//
//	EXCEPTIONS
//
ModSize
Tree::getDefaultSize() const
{
	// The size of PageID, RowID and Index are stored in UnsignedInteger.

	ModUInt32* dummy = 0;
	return Data::UnsignedInteger::getSize(dummy);
}

//
//	FUNCTION protected
//	Array::Tree::setDefaultData --
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cData_
//		データ
//
//	RETURN
//
//	EXCEPTIONS
//
void
Tree::setDefaultData(Common::DataArrayData& cData_,
					 int iPosition_,
					 unsigned int uiData_) const
{
	; _SYDNEY_ASSERT(iPosition_ >= 0 && iPosition_ < getFieldCount());
	
	Common::Data::Pointer p = cData_.getElement(iPosition_);
	; _SYDNEY_ASSERT(p->getType() == Common::DataType::UnsignedInteger);

	Common::UnsignedIntegerData* d =
		_SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData*, p.get());
	d->setValue(uiData_);
}

//
//	FUNCTION private
//	Array::Tree::getFieldType --
//
//	NOTES
//
//	ARGUMENTS
//	int iPosition_
//
//	RETURN
//	Data::Type::Value
//
//	EXCEPTIONS
//
Data::Type::Value
Tree::getFieldType(int iPosition_) const
{
	; _SYDNEY_ASSERT(iPosition_ >= 0 && iPosition_ < getFieldCount());
	return getDefaultType();
}

//
//	FUNCTION private
//	Array::Tree::getFieldSize --
//
//	NOTES
//
//	ARGUMENTS
//	int iPosition_
//
//	RETURN
//	ModSize
//
//	EXCEPTIONS
//
ModSize
Tree::getFieldSize(int iPosition_) const
{
	; _SYDNEY_ASSERT(iPosition_ >= 0 && iPosition_ < getFieldCount());
	return getDefaultSize();
}

//
//	FUNCTION private
//	Array::Tree::isFixedField --
//
//	NOTES
//
//	ARGUMENTS
//	int iPosition_
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool
Tree::isFixedField(int iPosition_) const
{
	; _SYDNEY_ASSERT(iPosition_ >= 0 && iPosition_ < getFieldCount());
	return true;
}

//
//	FUNCTION private
//	Array::Tree::getValuePosition -- Get the position of Value.
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
int
Tree::getValuePosition() const
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Array::Tree::getIndexPosition -- Get the position of RowID.
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
int
Tree::getIndexPosition() const
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Array::Tree::getFieldParameter --
//		Get the field parameter in the point of FileID
//
//	NOTES
//
//	ARGUMENTS
//	int& iPosition_
//	bool& isArray_
//
//	RETURN
//
//	EXCEPTIONS
//
void
Tree::getFieldParameter(int& iPosition_, bool& isArray_) const
{
	// Nothing to do
}

//
//	Copyright (c) 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
