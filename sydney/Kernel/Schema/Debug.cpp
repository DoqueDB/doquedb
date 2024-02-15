// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Debug.cpp -- スキーマモジュールのデバッグ関連の関数定義
// 
// Copyright (c) 2001, 2007, 2012, 2023 Ricoh Company, Ltd.
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

#ifdef DEBUG

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Schema/Debug.h"
#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace {
	// Object::Category::Valueから文字列を得るためのテーブル
	const char* const _ObjectCategoryTable[] =
	{
		"Unknown",
		"Database",
		"Table",
		"Column",
		"Constraint",
		"Index",
		"Key",
		"File",
		"Field",
		"Area",
		"AreaContent",
		"Privilege",
		"Cascade",
		"Partition",
		"Function",
		"ValueNum"
	};
	// AreaCategory::Valueから文字列を得るためのテーブル
	const char* const _AreaCategoryTable[] =
	{
		"Default",
		"LogicalLog",
		"PhysicalLog",
		"FileMin",
		"Heap",
		"Index",
		"FullText",
		"ValueNum"
	};
	// Database::Path::Category::Valueから文字列を得るためのテーブル
	const char* const _DatabasePathCategoryTable[] =
	{
		"Data",
		"LogicalLog",
		"System",
		"ValueNum"
	};
	// Column::Category::Valueから文字列を得るためのテーブル
	const char* const _ColumnCategoryTable[] =
	{
		"Unknown",
		"TupleID",
		"Normal",
		"ValueNum"
	};
	// Constraint::Category::Valueから文字列を得るためのテーブル
	const char* const _ConstraintCategoryTable[] =
	{
		"Unknown",
		"PrimaryKey",
		"Unique",
		"ForeignKey",
		"ValueNum"
	};
	// Default::Category::Valueから文字列を得るためのテーブル
	const char* const _DefaultCategoryTable[] =
	{
		"Unknown",
		"Constant",
		"ValueNum"
	};
	// Externalizable::Category::Valueから文字列を得るためのテーブル
	const char* const _ExternalizableCategoryTable[] =
	{
		"Unknown",
		"BtreeFile",
		"Column",
		"Constraint",
		"Database",
		"Default",
		"Field",
		"Index",
		"Key",
		"LogData",
		"Object",
		"RecordFile",
		"Table",
		"TempFile",
		"FullTextFile",
		"Area",
		"AreaContent",
		"ObjectID",
		"Hint",
		"VectorFile",
		"ValueNum"
	};
	// Field::Category::Valueから文字列を得るためのテーブル
	const char* const _FieldCategoryTable[] =
	{
		"Unknown",
		"ObjectID",
		"Key",
		"Data",
		"Function",
		"ValueNum"
	};
	// Index::Category::Valueから文字列を得るためのテーブル
	const char* const _IndexCategoryTable[] =
	{
		"Unknown",
		"Normal",
		"FullText",
		"Bitmap",
		"Array",
		"ValueNum"
	};
	// File::Category::Valueから文字列を得るためのテーブル
	const char* const _FileCategoryTable[] =
	{
		"Unknown",
		"Record",
		"Btree",
		"FullText",
		"Vector",
		"Lob",
		"ValueNum"
	};
}

//---------------------------------------------
//メッセージ出力でenum型を文字列で見るための定義
//---------------------------------------------

ModMessageStream& operator<<(ModMessageStream& cStream_,
							 Object::Category::Value eValue_)
{
	; _SYDNEY_ASSERT(sizeof(_ObjectCategoryTable) / sizeof(const char*) == Object::Category::ValueNum + 1);
	return cStream_ << _ObjectCategoryTable[eValue_];
}

ModOstream& operator<<(ModOstream& cStream_,
					   Object::Category::Value eValue_)
{
	; _SYDNEY_ASSERT(sizeof(_ObjectCategoryTable) / sizeof(const char*) == Object::Category::ValueNum + 1);
	return cStream_ << _ObjectCategoryTable[eValue_];
}

ModMessageStream& operator<<(ModMessageStream& cStream_,
							 AreaCategory::Value eValue_)
{
	; _SYDNEY_ASSERT(sizeof(_AreaCategoryTable) / sizeof(const char*) == AreaCategory::ValueNum + 1);
	return cStream_ << _AreaCategoryTable[eValue_];
}

ModOstream& operator<<(ModOstream& cStream_,
					   AreaCategory::Value eValue_)
{
	; _SYDNEY_ASSERT(sizeof(_AreaCategoryTable) / sizeof(const char*) == AreaCategory::ValueNum + 1);
	return cStream_ << _AreaCategoryTable[eValue_];
}

ModMessageStream& operator<<(ModMessageStream& cStream_,
							 Database::Path::Category::Value eValue_)
{
	; _SYDNEY_ASSERT(sizeof(_DatabasePathCategoryTable) / sizeof(const char*) == Database::Path::Category::ValueNum + 1);
	return cStream_ << _DatabasePathCategoryTable[eValue_];
}

ModOstream& operator<<(ModOstream& cStream_,
					   Database::Path::Category::Value eValue_)
{
	; _SYDNEY_ASSERT(sizeof(_DatabasePathCategoryTable) / sizeof(const char*) == Database::Path::Category::ValueNum + 1);
	return cStream_ << _DatabasePathCategoryTable[eValue_];
}

ModMessageStream& operator<<(ModMessageStream& cStream_,
							 _SYDNEY::Schema::Column::Category::Value eValue_)
{
	; _SYDNEY_ASSERT(sizeof(_ColumnCategoryTable) / sizeof(const char*) == Column::Category::ValueNum + 1);
	return cStream_ << _ColumnCategoryTable[eValue_];
}

ModOstream& operator<<(ModOstream& cStream_,
					   _SYDNEY::Schema::Column::Category::Value eValue_)
{
	; _SYDNEY_ASSERT(sizeof(_ColumnCategoryTable) / sizeof(const char*) == Column::Category::ValueNum + 1);
	return cStream_ << _ColumnCategoryTable[eValue_];
}

ModMessageStream& operator<<(ModMessageStream& cStream_,
							 _SYDNEY::Schema::Constraint::Category::Value eValue_)
{
	; _SYDNEY_ASSERT(sizeof(_ConstraintCategoryTable) / sizeof(const char*) == Constraint::Category::ValueNum + 1);
	return cStream_ << _ConstraintCategoryTable[eValue_];
}

ModOstream& operator<<(ModOstream& cStream_,
					   _SYDNEY::Schema::Constraint::Category::Value eValue_)
{
	; _SYDNEY_ASSERT(sizeof(_ConstraintCategoryTable) / sizeof(const char*) == Constraint::Category::ValueNum + 1);
	return cStream_ << _ConstraintCategoryTable[eValue_];
}

ModMessageStream& operator<<(ModMessageStream& cStream_,
							 _SYDNEY::Schema::Default::Category::Value eValue_)
{
	; _SYDNEY_ASSERT(sizeof(_DefaultCategoryTable) / sizeof(const char*) == Default::Category::ValueNum + 1);
	return cStream_ << _DefaultCategoryTable[eValue_];
}

ModOstream& operator<<(ModOstream& cStream_,
					   _SYDNEY::Schema::Default::Category::Value eValue_)
{
	; _SYDNEY_ASSERT(sizeof(_DefaultCategoryTable) / sizeof(const char*) == Default::Category::ValueNum + 1);
	return cStream_ << _DefaultCategoryTable[eValue_];
}

ModMessageStream& operator<<(ModMessageStream& cStream_,
							 _SYDNEY::Schema::Externalizable::Category::Value eValue_)
{
	; _SYDNEY_ASSERT(sizeof(_ExternalizableCategoryTable) / sizeof(const char*) == Externalizable::Category::ValueNum + 1);
	return cStream_ << _ExternalizableCategoryTable[eValue_];
}

ModOstream& operator<<(ModOstream& cStream_,
					   _SYDNEY::Schema::Externalizable::Category::Value eValue_)
{
	; _SYDNEY_ASSERT(sizeof(_ExternalizableCategoryTable) / sizeof(const char*) == Externalizable::Category::ValueNum + 1);
	return cStream_ << _ExternalizableCategoryTable[eValue_];
}

ModMessageStream& operator<<(ModMessageStream& cStream_,
							 _SYDNEY::Schema::Field::Category::Value eValue_)
{
	; _SYDNEY_ASSERT(sizeof(_FieldCategoryTable) / sizeof(const char*) == Field::Category::ValueNum + 1);
	return cStream_ << _FieldCategoryTable[eValue_];
}

ModOstream& operator<<(ModOstream& cStream_,
					   _SYDNEY::Schema::Field::Category::Value eValue_)
{
	; _SYDNEY_ASSERT(sizeof(_FieldCategoryTable) / sizeof(const char*) == Field::Category::ValueNum + 1);
	return cStream_ << _FieldCategoryTable[eValue_];
}

ModMessageStream& operator<<(ModMessageStream& cStream_,
							 _SYDNEY::Schema::Index::Category::Value eValue_)
{
	; _SYDNEY_ASSERT(sizeof(_IndexCategoryTable) / sizeof(const char*) == Index::Category::ValueNum + 1);
	return cStream_ << _IndexCategoryTable[eValue_];
}

ModOstream& operator<<(ModOstream& cStream_,
					   _SYDNEY::Schema::Index::Category::Value eValue_)
{
	; _SYDNEY_ASSERT(sizeof(_IndexCategoryTable) / sizeof(const char*) == Index::Category::ValueNum + 1);
	return cStream_ << _IndexCategoryTable[eValue_];
}

ModMessageStream& operator<<(ModMessageStream& cStream_,
							 _SYDNEY::Schema::File::Category::Value eValue_)
{
	; _SYDNEY_ASSERT(sizeof(_FileCategoryTable) / sizeof(const char*) == File::Category::ValueNum + 1);
	return cStream_ << _FileCategoryTable[eValue_];
}

ModOstream& operator<<(ModOstream& cStream_,
					   _SYDNEY::Schema::File::Category::Value eValue_)
{
	; _SYDNEY_ASSERT(sizeof(_FileCategoryTable) / sizeof(const char*) == File::Category::ValueNum + 1);
	return cStream_ << _FileCategoryTable[eValue_];
}
#endif

//
// Copyright (c) 2001, 2007, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
