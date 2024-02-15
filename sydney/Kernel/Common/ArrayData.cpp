// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ArrayData.cpp -- 配列データ型をあらわすクラス
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Common";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/Assert.h"
#include "Common/ArrayData.h"

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

//
//	FUNCTION public
//	Common::ArrayData::ArrayData -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	Common::DataType::Type eElementType_
//		要素のデータ型
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ArrayData::
ArrayData(DataType::Type eElementType_)
: Data(DataType::Array), m_eElementType(eElementType_), m_iCount( 0 )
{
}

//
//	FUNCTION public
//	Common::ArrayData::~ArrayData -- デストラクタ
//
//	NOTES
//	デストラクタ
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ArrayData::
~ArrayData()
{
}

//	FUNCTION private
//	Common::ArrayData::serialize_NotNull -- シリアル化
//
//	NOTES
//	シリアル化を行う
//
//	ARGUMENTS
//	ModArchive& cArchiver_
//		アーカイバー
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
ArrayData::serialize_NotNull(ModArchive& cArchiver_)
{
	; _TRMEISTER_ASSERT(!isNull());

	Data::serialize_NotNull(cArchiver_);

	if (cArchiver_.isStore())
	{
		//書き出し
		//	要素の型を書いていたが、意味ないので書くのをやめた
		m_iCount = getCount();
		cArchiver_ << m_iCount;
	}
	else
	{
		//読み出し
		cArchiver_ >> m_iCount;
	}
}

//
//	FUNCTION public
//	Common::ArrayData::getElementType -- 要素のデータ型を得る
//
//	NOTES
//	要素のデータ型を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::DataType::Type
//		要素のデータ型
//
//	EXCEPTIONS
//	なし
//
DataType::Type
ArrayData::
getElementType() const
{
	return m_eElementType;
}

// FUNCTION public
//	Common::ArrayData::getSQLTypeByValue -- データに対応するSQLDataを得る
//
// NOTES
//
// ARGUMENTS
//	SQLData& cResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ArrayData::
getSQLTypeByValue(SQLData& cResult_)
{
	// ArrayDataの場合は上書きしない限り失敗を表すfalseを返す
	return false;
}


//
//	Copyright (c) 1999, 2004, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
