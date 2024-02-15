// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ScalarData.h -- スカラーデータ共通の基底クラス
// 
// Copyright (c) 1999, 2001, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_SCALARDATA_H
#define __TRMEISTER_COMMON_SCALARDATA_H

#include "Common/Module.h"
#include "Common/Data.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//	CLASS
//	Common::ScalarData -- スカラーデータ共通の基底クラス
//
//	NOTES

class SYD_COMMON_FUNCTION ScalarData
	: public	Data
{
public:
	// コンストラクタ
	ScalarData(DataType::Type type);
	// デストラクタ
	virtual ~ScalarData();

	// スカラデータか調べる
	virtual bool
	isScalar() const;
};

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_SCALARDATA_H

//
//	Copyright (c) 1999, 2001, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
