// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// HintTools.cpp -- 更新系や検索系のヒント用ツールの実現ファイル
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Btree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Btree/File.h"
#include "Btree/HintTools.h"

#include "Common/Assert.h"
#include "Exception/BadArgument.h"
#include "FileCommon/DataManager.h"

_SYDNEY_USING
_SYDNEY_BTREE_USING

//////////////////////////////////////////////////
//
//	PUBLIC METHOD
//
//////////////////////////////////////////////////

//	FUNCTION public
//	Btree::HintTools::getFixedDataSize --
//		固定長フィールドに対する検索条件の配列上での記録サイズを返す
//
//	NOTES
//	固定長フィールドに対する検索条件の配列上での記録サイズを返す。
//
//	ARGUMENTS
//	const Common::DataType::Type	DataType_
//		固定長フィールドのデータタイプ
//
//	RETURN
//	int
//		検索条件の配列上での記録サイズ [byte]
//
//	EXCEPTIONS

// static
int
HintTools::getFixedDataSize(const Common::DataType::Type DataType_)
{
	return static_cast<int>(Common::Data::getDumpSize(DataType_));
}

//	FUNCTION public
//	Btree::HintTools::setFixedData --
//		固定長フィールドに対する検索条件を設定する
//
//	NOTES
//	固定長フィールドに対する検索条件を設定する。
//
//	ARGUMENTS
//	const Common::DataType::Type	DataType_
//		検索対象の固定長フィールドのデータタイプ
//	const Common::Data*				Data_
//		固定長フィールドに対する検索条件が設定されているデータへのポインタ
//	void*							SetPos_
//		設定先へのポインタ
//
//	RETURN
//	int
//		設定したサイズ [byte]
//
//	EXCEPTIONS

// static
int
HintTools::setFixedData(const Common::DataType::Type	DataType_,
						const Common::Data*				Data_,
						void*							SetPos_)
{
	; _SYDNEY_ASSERT(Data_);

	return (Data_->isFixedSize()) ?
		Data_->dumpValue(static_cast<char*>(SetPos_)) : 0;
}

//	FUNCTION public
//	Btree::HintTools::compareToFixedData --
//		ヒントに設定されている固定長フィールドへの検索条件と
//		フィールド値を比較する
//
//	NOTES
//	ヒントに設定されている固定長フィールドへの検索条件と
//	フィールド値を比較する。
//
//	ARGUMENTS
//	const Common::DataType::Type	DataType_
//		固定長フィールドのデータタイプ
//	const char*						Condition_
//		検索条件配列へのポインタ
//	const int*						OffsetArray_
//		検索条件オフセット配列へのポインタ
//	const int*						MultiNumberArray_
//		比較結果との乗数配列へのポインタ
//	const int						ArrayIndex_
//		ヒントの配列のインデックス
//	const void*						DataValue_
//		フィールド値が記録されている領域へのポインタ
//
//	RETURN
//	int
//		比較結果
//			< 0 : ヒントに設定されてる検索条件の方がキー値順で前方
//			= 0 : ヒントに設定されてる検索条件とキーフィールドの値が等しい
//			> 0 : ヒントに設定されてる検索条件の方がキー値順で後方
//
//	EXCEPTIONS

// static
int
HintTools::compareToFixedData(const Common::DataType::Type DataType_,
							  const char* Condition_,
							  const int* OffsetArray_,
							  const int* MultiNumberArray_,
							  const int ArrayIndex_,
							  const void* DataValue_)
{
	if (OffsetArray_)
		Condition_ += *(OffsetArray_ + ArrayIndex_);

	int result;

	switch (DataType_) {
	case Common::DataType::Integer:
		result = File::compareIntegerField(Condition_, DataValue_);
		break;
	case Common::DataType::UnsignedInteger:
		result = File::compareUnsignedIntegerField(Condition_, DataValue_);
		break;
	case Common::DataType::Integer64:
		result = File::compareInteger64Field(Condition_, DataValue_);
		break;
	case Common::DataType::UnsignedInteger64:
		result = File::compareUnsignedInteger64Field(Condition_, DataValue_);
		break;
	case Common::DataType::Float:
		result = File::compareFloatField(Condition_, DataValue_);
		break;
	case Common::DataType::Double:
		result = File::compareDoubleField(Condition_, DataValue_);
		break;
	case Common::DataType::Date:
		result = File::compareDateField(Condition_, DataValue_);
		break;
	case Common::DataType::DateTime:
		result = File::compareTimeField(Condition_, DataValue_);
		break;
	case Common::DataType::ObjectID:
		result = File::compareObjectIDField(Condition_, DataValue_);
		break;
	default:
		; _SYDNEY_ASSERT(false);
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	return result * *(MultiNumberArray_ + ArrayIndex_);
}

//
//	Copyright (c) 2001, 2004, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
