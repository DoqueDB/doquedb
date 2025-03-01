// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NullDataTree.h --
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

#ifndef __SYDNEY_ARRAY_NULLDATATREE_H
#define __SYDNEY_ARRAY_NULLDATATREE_H

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
class NullDataTree : public Tree
{
public:
	
	//コンストラクタ
	NullDataTree(const FileID& cFileID_);
	//デストラクタ
	virtual ~NullDataTree();

	// Get the type of tree.
	Type::Value getType() const { return Type::NullData; }

	//
	// Data
	//

	// Set Index to the DataArrayData.
	void setIndex(Common::DataArrayData& cData_, int iIndex_) const;
	
private:
	//
	//	CONST
	//	FieldCount --
	//
	static const int FieldCount = 2;

	//
	//	CONST
	//	KeyFieldCount --
	//
	static const int KeyFieldCount = 0;

	//
	//	STRUCT
	//	FieldPosition -- The position of the field in the point of NullDataTree
	//
	struct FieldPosition
	{
		enum Value
		{
			RowID = 0,
			Index
		};
	};
	
	// Get the count of the fields.
	int getFieldCount() const { return FieldCount; }
	// Get the count of the key fields.
	int getKeyFieldCount() const { return KeyFieldCount; }
	
	// Get the position of RowID.
	int getRowIDPosition() const { return FieldPosition::RowID; }
	// Get the position of Index.
	int getIndexPosition() const { return FieldPosition::Index; }
};

_SYDNEY_ARRAY_END
_SYDNEY_END

#endif //__SYDNEY_ARRAY_NULLDATATREE_H

//
//	Copyright (c) 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
