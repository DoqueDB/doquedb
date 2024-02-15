// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Compare.h -- 比較クラス
// 
// Copyright (c) 2003, 2004, 2007, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE2_COMPARE_H
#define __SYDNEY_BTREE2_COMPARE_H

#include "Btree2/Module.h"
#include "Btree2/Data.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_BTREE2_BEGIN

//
//	CLASS
//	Btree2::Compare -- 比較クラス
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
	void setType(const ModVector<Data::Type::Value>& vecType_,
				 bool bUnique_, bool bHeader_);
	// 型を得る
	const ModVector<Data::Type::Value>& getType() const
	{
		return m_vecType;
	}

	// インテグリティーチェックで使用するフィールド数を設定する
	void setUsingIntegrityCheckField(ModSize num_)
	{
		m_uiUsingIntegrityCheckField = num_;
	}

	// 比較する -- nullがないもの
	int operator () (const ModUInt32* p1, const ModUInt32* p2) const;
	// 比較する -- nullがあるもの
	int operator () (const ModUInt32* p1, unsigned char nullBitmap1_,
					 const ModUInt32* p2, unsigned char nullBitmap2_) const;

	// インテグリティーチェック
	bool integrityCheck(const ModUInt32* p1, const ModUInt32* p2) const;
	// インテグリティーチェック
	bool integrityCheck(const ModUInt32* p1, unsigned char nullBitmap1_,
						const ModUInt32* p2, unsigned char nullBitmap2_) const;


	// 比較クラスが同じかどうか
	bool operator == (const Compare& other_)
	{
		// サイズが同じなら同じはず
		return m_vecType.getSize() == other_.m_vecType.getSize();
	}
	bool operator != (const Compare& other_)
	{
		return !(*this == other_);
	}

	// この比較でエントリがユニークになるかどうか
	bool isUnique() const { return m_bUnique; }

	// setTypeされているかどうか
	bool isSetType() const { return m_vecType.getSize() != 0; }

	// 1つ比較する -- nullがないもの
    int compare(const ModUInt32*& p1,
				const ModUInt32*& p2, Data::Type::Value eType_) const;
	
	// 1つ比較する -- nullがあるもの
	int compare(const ModUInt32*& p1, bool isNull1_,
				const ModUInt32*& p2, bool isNull2_,
				Data::Type::Value eType_) const;

	// Likeで1つ比較する -- nullがないもの
    bool like(const ModUInt32*& p1,
			  const ModUInt32*& p2, Data::Type::Value eType_,
			  ModUnicodeChar escape_) const;
	
private:	
	// 型の配列
	ModVector<Data::Type::Value> m_vecType;
	// この比較でエントリがユニークか？
	bool m_bUnique;
	// エントリヘッダーがあるか否か
	bool m_bHeader;

	// インテグリティーチェック時に使用するフィールド数
	ModSize m_uiUsingIntegrityCheckField;
};

_SYDNEY_BTREE2_END
_SYDNEY_END

#endif //__SYDNEY_BTREE2_COMPARE_H

//
//	Copyright (c) 2003, 2004, 2007, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
