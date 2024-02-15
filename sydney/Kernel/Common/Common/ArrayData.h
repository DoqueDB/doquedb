// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ArrayData.h -- 配列データ型をあらわすクラス
// 
// Copyright (c) 1999, 2004, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_ARRAYDATA_H
#define __TRMEISTER_COMMON_ARRAYDATA_H

#include "Common/Module.h"
#include "Common/Data.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//	CLASS
//	Common::ArrayData -- 配列データ型を表すクラスの基底クラス
//
//	NOTES

class SYD_COMMON_FUNCTION ArrayData
	: public	Data
{
public:
	//コンストラクタ
	ArrayData(Common::DataType::Type eElementType_);
	//デストラクタ
	virtual ~ArrayData();

	// シリアル化する
//	Common::Data
//	virtual void
//	serialize(ModArchive& archiver);

	//要素のデータ型を得る
	Common::DataType::Type getElementType() const;

	//要素数を得る
	virtual int getCount() const = 0;

	//配列要素が2の時にその範囲が重なっているかどうかチェックする
	virtual bool overlaps(const Common::Data* pData_) const = 0;

protected:
	// シリアル化する(自分自身が NULL 値でない)
	virtual void
	serialize_NotNull(ModArchive& archiver);

	//要素数(アーカイバーからの読み出しのみに使用する)
	int m_iCount;

private:
	virtual bool getSQLTypeByValue(SQLData& cResult_);

	//要素のデータ型
	Common::DataType::Type m_eElementType;
};

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_ARRAYDATA_H

//
//	Copyright (c) 1999, 2004, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
