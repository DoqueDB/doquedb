// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Compare.h -- 比較クラス
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

#ifndef __SYDNEY_ARRAY_COMPARE_H
#define __SYDNEY_ARRAY_COMPARE_H

#include "Array/Module.h"
#include "Array/Data.h"

#include "ModTypes.h"

_SYDNEY_BEGIN
_SYDNEY_ARRAY_BEGIN

class Tree;

//
//	CLASS
//	Array::Compare -- 比較クラス
//
//	NOTES
//	二つのエントリを比較する。
//	比較するときは、エントリのメモリが渡されるだけで、
//	エントリの構成情報—どんなフィールド型列から構成されているか—は渡されない。
//	エントリの構成情報は、事前にsetTypeを使って設定しておく。
//
//	設定されるフィールドは、
//	各エントリの大小関係の判定に必要十分なフィールド列になる。
//	それはリーフのフィールド列に等しいので、
//	ノードに付加されるPageIDフィールドは不用。
//
class Compare
{
public:
	// コンストラクタ
	Compare();
	// デストラクタ
	virtual ~Compare();

	// Set the number of the fields.
	void setTypeCount(int iFieldCount_);
	// Get the number of the fields.
	int getTypeCount() const { return m_iFieldCount; }
	
	// Set whether this comparison is for an unique search.
	void setUnique(bool isUnique_) { m_bUnique = isUnique_; }
	// この比較でエントリがユニークになるかどうか
	bool isUnique() const { return m_bUnique; }

	// 型を設定する
	void setType(Data::Type::Value eFieldType_, int iFieldPosition_);

	// 比較する
	int operator () (const ModUInt32* p1, const ModUInt32* p2) const;

	// 1つ比較する
    static int compare(const ModUInt32*& p1, const ModUInt32*& p2,
					   Data::Type::Value eType_);

	// Compare one data with like operator.
	static bool like(const ModUInt32*& p1, const ModUInt32*& p2,
					 Data::Type::Value eType_, ModUnicodeChar escape_);
	
private:
	//
	//	CONST
	//	MaxFieldCount -- The Maximum number of fields
	//
	//	NOTES
	//	Value, RowID, Index
	//	PageID is not used for the comparison.
	//	[YET!] It may be better to define this number in Tree or FileID.
	//
	static const int MaxFieldCount = 3;
	
	// The array of the types
	Data::Type::Value m_cFieldTypes[MaxFieldCount];
	// The number of the types
	int m_iFieldCount;
	// この比較でエントリがユニークか？
	bool m_bUnique;
};

_SYDNEY_ARRAY_END
_SYDNEY_END

#endif //__SYDNEY_ARRAY_COMPARE_H

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
