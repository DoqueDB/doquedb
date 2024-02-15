// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataTree.cpp --
// 
// Copyright (c) 2007, 2009, 2023 Ricoh Company, Ltd.
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

#include "Array/DataTree.h"
#include "Array/FileID.h"

#include "Common/Assert.h"
#include "Exception/BadArgument.h"
#include "Utility/CharTrait.h"

_SYDNEY_USING
_SYDNEY_ARRAY_USING

//
//	FUNCTION public
//	Array::DataTree::DataTree -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
DataTree::DataTree(const FileID& cFileID_)
	: Tree(cFileID_)
{
}

//
//	FUNCTION public
//	Array::DataTree::~DataTree -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
DataTree::~DataTree()
{
}

//
//	FUNCTION public
//	Array::DataTree::setKey --
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
DataTree::setKey(Common::DataArrayData& cData_,
				  const ModUInt32* p) const
{
	// Make a Common::Data.
	Common::Data::Pointer pValue =
		Data::makeData(m_cFileID, FileID::FieldPosition::Array,
					   m_cFileID.getKeyType(), true);

	// Set the buffer to the Common::Data.
	m_cLeafData.getData(p, getValuePosition(), pValue);

	// Set the Common::Data to DataArrayData.
	setKey(cData_, pValue);
}

//
//	FUNCTION public
//	Array::DataTree::setKey --
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
DataTree::setKey(Common::DataArrayData& cData_,
				  const Common::Data::Pointer pElement_) const
{
	const int i = getValuePosition();

	// Check the types.
	if (cData_.getElement(i)->getType() != pElement_->getType())
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// Set the data.
	if (m_cFileID.isNormalized() == true)
	{
		// [NOTE] pElement_はconstデータなので、
		//  データをコピーしたうえで正規化データを上書きする必要がある。
		// [YET] 正規化で文字列が変更されたことを確認してから
		//  copyしても遅くないのでは？
		Common::Data::Pointer pCopyData = pElement_->copy();
		; _SYDNEY_ASSERT(pElement_->getType() == Common::DataType::String);
		Common::StringData* pStringData
			= _SYDNEY_DYNAMIC_CAST(Common::StringData*, pCopyData.get());
		Utility::CharTrait::normalize(
			pStringData, m_cFileID.getNormalizingMethod());
		cData_.setElement(i, pCopyData);
	}
	else
	{
		cData_.setElement(i, pElement_);
	}
}

//
//	FUNCTION public
//	Array::DataTree::setIndex --
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
DataTree::setIndex(Common::DataArrayData& cData_, int iIndex_) const
{
	; _SYDNEY_ASSERT(iIndex_ >= 0);

	setDefaultData(
		cData_, getIndexPosition(), static_cast<unsigned int>(iIndex_));
}

//
//	FUNCTION private
//	Array::DataTree::getFieldType --
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
DataTree::getFieldType(int iPosition_) const
{
	; _SYDNEY_ASSERT(iPosition_ >= 0 && iPosition_ < getFieldCount());

	Data::Type::Value type;
	if(iPosition_ == FieldPosition::Key)
	{
		type = m_cFileID.getKeyType();
	}
	else
	{
		type = getDefaultType();
	}
	return type;
}

//
//	FUNCTION private
//	Array::DataTree::getFieldSize --
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
DataTree::getFieldSize(int iPosition_) const
{
	; _SYDNEY_ASSERT(iPosition_ >= 0 && iPosition_ < getFieldCount());

	ModSize size = 0; // [unit size]
	if (iPosition_ == FieldPosition::Key)
	{
		size = m_cFileID.getKeySize() / sizeof(ModUInt32);
	}
	else
	{
		size = getDefaultSize();
	}
	return size;
}

//
//	FUNCTION private
//	Array::DataTree::isFixedField --
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
DataTree::isFixedField(int iPosition_) const
{
	; _SYDNEY_ASSERT(iPosition_ >= 0 && iPosition_ < getFieldCount());

	bool result = true;
	if (iPosition_ == FieldPosition::Key)
	{
		result = m_cFileID.isFixed();
	}
	return result;
}

//
//	FUNCTION private
//	Array::DataTree::getLeafEntrySize --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	ModSize [unit size]
//
//	EXCEPTIONS
//
ModSize
DataTree::getLeafEntrySize(const Common::DataArrayData& cData_) const
{
	// For Leaf
	
	ModSize size = 0;
	if (m_cLeafData.isFixed() == false)
	{
		size = m_cLeafData.getSize(
			*(cData_.getElement(FieldPosition::Key)), FieldPosition::Key);
		if (size > m_cLeafData.getVariableSize())
		{
			_SYDNEY_THROW0(Exception::BadArgument);
		}
	}
	return size + m_cLeafData.getFixedSize();
}

//
//	FUNCTION private
//	Array::DataTree::getNodeEntrySize --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	ModSize [unit size]
//
//	EXCEPTIONS
//
ModSize
DataTree::getNodeEntrySize(const ModUInt32* pEntry_) const
{
	// For Node
	
	ModSize size = 0;
	if (m_cNodeData.isFixed() == false)
	{
		const int count = m_cNodeData.getTypeCount();
		for (int i = 0; i < count; ++i)
		{
			size = Data::getSize(pEntry_, m_cNodeData.getType(i));
			if (i == FieldPosition::Key)
			{
				break;
			}
		}
		if (size > m_cNodeData.getVariableSize())
		{
			_SYDNEY_THROW0(Exception::BadArgument);
		}
	}
	return size + m_cNodeData.getFixedSize();
}

//
//	FUNCTION private
//	Array::DataTree::getFieldParameter --
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
DataTree::getFieldParameter(int& iPosition_, bool& isArray_) const
{
	if (iPosition_ == getValuePosition())
	{
		iPosition_ = FileID::FieldPosition::Array;
		isArray_ = true;
	}
#ifdef DEBUG
	else if (iPosition_ == getRowIDPosition())
	{
		iPosition_ = FileID::FieldPosition::RowID;
	}
	else if (iPosition_ == getIndexPosition())
	{
		// Index does NOT exist in the point of FileID
		iPosition_ = -1;
	}
#endif
}

//
//	Copyright (c) 2007, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
