// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BitmapTable.h --
//		空き領域管理機能付き物理ファイル用領域率ビットマップ変換表関連
// 
// Copyright (c) 2000, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PHYSICALFILE_BITMAPTABLE_H
#define __SYDNEY_PHYSICALFILE_BITMAPTABLE_H

#include "Common/Common.h"
#include "Common/Internal.h"

_SYDNEY_BEGIN

namespace PhysicalFile
{

//
//	CLASS
//	PhysicalFile::BitmapTable --
//		空き領域管理機能付き物理ファイル用領域率ビットマップ変換表クラス
//
//	NOTES
//	空き領域管理機能付き物理ファイル用領域率ビットマップ変換表クラス。
//	領域率ビットマップと未使用領域率／空き領域率[％]間での変換に
//	用いる変換表。
//
class BitmapTable
{
public:

	//
	//	ENUM
	//	PhysicalFile::BitmapTable::Rate --
	//		領域率を表す列挙型
	//
	//	NOTES
	//	領域率を表す列挙型
	//
	enum Rate
	{
		Unuse = 0,			// 0 : 未使用の物理ページ
		Rate_04,			// 1 : 4％以下
		Rate05_09,			// 2 : 5〜9％
		Rate10_14,			// 3 : 10〜14％
		Rate15_19,			// 4 : 15〜19％
		Rate20_39,			// 5 : 20〜39％
		Rate40_59,			// 6 : 40〜59％
		Rate60_79,			// 7 : 60〜79％
		Rate80_,			// 8 : 80％以上
		RateNum,			// 有効な領域率数
		InvalidRate = -1	// 無効な領域率
	};

	// 無効な領域率ビットマップの値
	static const unsigned char	InvalidBitmapValue;

	// 未使用領域率[％]と空き領域率[％]から
	// 領域率ビットマップの値（8ビット）への変換表
	static const unsigned char	ToBitmapValue[RateNum][RateNum];

	// 領域率ビットマップの値（8ビット）から未使用領域率[％]への変換表
	static const Rate	ToUnuseAreaRate[256];

	// 領域率ビットマップの値（8ビット）から空き領域率[％]への変換表
	static const Rate	ToFreeAreaRate[256];

	// 領域率を表す列挙型への変換表
	static const Rate	ToRate[101];

private:

};

}

_SYDNEY_END

#endif //__SYDNEY_PHYSICALFILE_BITMAPTABLE_H

//
//	Copyright (c) 2000, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
