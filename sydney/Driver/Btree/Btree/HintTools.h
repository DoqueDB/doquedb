// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// HintTools.h -- 更新系や検索系のヒント用ツールのヘッダーファイル
// 
// Copyright (c) 2001, 2004, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE_HINTTOOLS_H
#define __SYDNEY_BTREE_HINTTOOLS_H

#include "Btree/Module.h"

#include "Common/Common.h"
#include "Common/DataType.h"

_SYDNEY_BEGIN

namespace Common
{
	class Data;
}

_SYDNEY_BTREE_BEGIN

//
//	CLASS
//	Btree::HintTools --
//		更新系や検索系のヒント用ツールクラス
//
//	NOTES
//	更新系や検索系のヒント用ツールクラス。
//
class HintTools 
{
public:

	// 固定長フィールドに対する検索条件の配列上での記録サイズを返す [byte]
	static int
		getFixedDataSize(const Common::DataType::Type	DataType_);

	// 固定長フィールドに対する検索条件を設定する
	static int setFixedData(const Common::DataType::Type	DataType_,
							const Common::Data*				Data_,
							void*							SetPos_);

	// ヒントに設定されている固定長フィールドへの検索条件と
	// フィールド値を比較する
	static int
		compareToFixedData(const Common::DataType::Type	DataType_,
						   const char*					Condition_,
						   const int*					OffsetArray_,
						   const int*					MultiNumberArray_,
						   const int					ArrayIndex_,
						   const void*					DataValue_);

}; // end of class Btree::HintTools

_SYDNEY_BTREE_END
_SYDNEY_END

#endif //__SYDNEY_BTREE_HINTTOOLS_H

//
//	Copyright (c) 2001, 2004, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
