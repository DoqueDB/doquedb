// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FieldMask.h --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2009, 2023 Ricoh Company, Ltd.
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
#ifndef __SYDNEY_FULLTEXT_FIELDMASK_H
#define __SYDNEY_FULLTEXT_FIELDMASK_H

#include "FullText/Module.h"

#include "Inverted/SortParameter.h"
#include "Inverted/FileIDNumber.h"
#include "Inverted/FieldType.h"

#include "ModMap.h"
#include "ModVector.h"
#include "ModHashMap.h"
#include "ModPair.h"

#include "FileCommon/OpenOption.h"


_SYDNEY_BEGIN
_SYDNEY_FULLTEXT_BEGIN

class FieldMask
{
public:

	const static ModUInt32 SCAN_BIT        = 0x01;
	const static ModUInt32 GetByBitSet_BIT = 0x02;
	const static ModUInt32 BASIC_BIT       = 0x04;  // 基本BIT

	typedef _SYDNEY::Inverted::FieldType::Value FieldType;

	//	同時に取得できる固定フィールドは以下のグループに分けられる。
	//	* normal
	//	* word
	//	* length
	//	各グループの詳細はコンストラクタを参照。
	//
	//	補足
	//	* ビットセットで取得できるのは、ROWIDだけを取得するときのみ。
	
	// コンストラクタ
	FieldMask(bool bLang=false,bool bScore=false);
	FieldMask(LogicalFile::OpenOption& cOpenOption_,bool bLang,bool bScore,bool bSection = false);

	// フィールド番号とフィールド型が一致するか？
	bool check(int n_,FieldType type_) const;
	// フィールド番号は適切な範囲内か？
	bool checkValueRangeValidity(int n_) const;
	// 調査済みのフィールド群と同じグループか？
	bool checkGroupExclusiveness(int n_);

	// フィールド型からフィールド番号を取得
	int	 getField(FieldType type) const {return base + type;}
	// フィールド番号からフィールド型を取得
	FieldType getFieldType(int n_) const
	{
		if(n_ - base >= _SYDNEY::Inverted::FieldType::Rowid && 
		   n_ - base <= _SYDNEY::Inverted::FieldType::Last - 1)
		{
			return (FieldType)(n_ - base);
		}
		// 該当するフィールド型が定義されていない
		return (FieldType)-1;
	}

private:
	// [YET] shift処理用？
	enum FieldGroupMaskID
	{
		LANGUAGE = 1,
		SCORE,
		SECTION
	};
	
	// 固定フィールドの位置を移動
	void shift();
	// [YET] 関数名と処理が合っていない？
	void shift(FieldGroupMaskID groupMaskID);

	// status var
	ModUInt32 stat;
	// vars for range check
	ModInt32 lower;
	ModInt32 upper;
	// group mask
	// groupは互いに排他
	ModUInt32 normal;
	ModUInt32 word;
	ModUInt32 length;
	// checked bits
	ModUInt32 bit;
	// works vars
	ModInt32 section;
	ModInt32 base;
};

_SYDNEY_FULLTEXT_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT_FIELDMASK_H

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

