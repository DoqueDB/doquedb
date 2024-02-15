// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Compare.h -- 比較クラス
// 
// Copyright (c) 2005, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BITMAP_COMPARE_H
#define __SYDNEY_BITMAP_COMPARE_H

#include "Bitmap/Module.h"
#include "Bitmap/Data.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

//
//	CLASS
//	Bitmap::Compare -- 比較クラス
//
//	NOTES
//
class Compare
{
public:
	// コンストラクタ
	Compare();
	// デストラクタ
	virtual ~Compare();

	// 型を設定する
	void setType(Data::Type::Value eType_,
				 bool bUnique_);
	// 型を得る
	Data::Type::Value getType() const
	{
		return m_eType;
	}

	// 比較する -- nullがないもの
	int operator () (const ModUInt32* p1, const ModUInt32* p2) const;
	
	// 比較クラスが同じかどうか
	bool operator == (const Compare& other_)
	{
		// 型が同じなら同じ
		return m_eType == other_.m_eType;
	}
	bool operator != (const Compare& other_)
	{
		return !(*this == other_);
	}

	// この比較でエントリがユニークになるかどうか
	bool isUnique() const { return m_bUnique; }

	// setTypeされているかどうか
	bool isSetType() const { return m_eType != Data::Type::Unknown; }

	// 1つ比較する -- nullがないもの
    int compare(const ModUInt32*& p1,
				const ModUInt32*& p2, Data::Type::Value eType_) const;
	
	// Likeで1つ比較する -- nullがないもの
    bool like(const ModUInt32*& p1,
			  const ModUInt32*& p2, Data::Type::Value eType_,
			  ModUnicodeChar escape_) const;
	
private:
	// 型
	Data::Type::Value m_eType;
	// ユニークかどうか
	bool m_bUnique;
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif //__SYDNEY_BITMAP_COMPARE_H

//
//	Copyright (c) 2005, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
