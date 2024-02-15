// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OptionDataFile.h --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_OPTIONDATAFILE_H
#define __SYDNEY_FULLTEXT2_OPTIONDATAFILE_H

#include "FullText2/Module.h"
#include "FullText2/FeatureSet.h"
#include "Common/Data.h"
#include "ModTypes.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::OptionDataFile -- オプションデータを取得するファイル
//
//	NOTES
//
class OptionDataFile
{
public:
	// スコア調整用の値があるかどうか
	virtual bool isModifierValue() = 0;
	// スコア調整用の値を得る
	virtual bool getModifierValue(ModUInt32 uiRowiD_, double& modifier_) = 0;

	// 特徴語データがあるかどうか
	virtual bool isFeatureValue() = 0;
	// 特徴語データを得る
	virtual bool getFeatureValue(ModUInt32 uiRowID_,
								 FeatureSetPointer& pFeatureSet_) = 0;
	
	// すべてのページをdetachする
	virtual void detachAllPages() = 0;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_OPTIONDATAFILE_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
