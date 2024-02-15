// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.cpp -- 論理ファイルID
// 
// Copyright (c) 2001, 2002, 2004, 2005, 2007, 2009, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "LogicalFile";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "SyInclude.h"

#include "Common/Common.h"
#include "Common/Assert.h"
#include "Exception/BadArgument.h"
#include "LogicalFile/FileID.h"
#include "Os/AutoCriticalSection.h"

#include "ModMap.h"
#include "ModPair.h"

_SYDNEY_USING
_SYDNEY_LOGICALFILE_USING

namespace {
	//	VARIABLE
	//	$$$::_bMapInitialized -- LayoutMapが初期化されたかを示す
	//
	//	NOTES
	bool _bMapInitialized = false;

	//	VARIABLE
	//	$$$::_cLayoutMap -- FileIDのLayoutMap
	//
	//	NOTES
	Parameter::LayoutMap _cLayoutMap;

	//	VARIABLE
	//	$$$::_latch -- 以下の変数を保護するためのクリティカルセクション
	//
	//	NOTES
	Os::CriticalSection _latch;

	//	VARIABLE
	//	$$$::_cOldLayoutMap -- 旧バージョンのLayoutMap
	//
	//	NOTES
	typedef ModMap<int, Parameter::LayoutMap, ModLess<int> > _LayoutMapMap;
	_LayoutMapMap _cOldLayoutMap;

	//	CONST
	//	$$$::_VersionNumber::_Value -- レイアウトマップのバージョン番号
	//
	//	NOTES
	// Versionが追加されたときはcreateLayoutMap()およびgetLayoutMap()を修正する
	namespace _VersionNumber {
		enum _Value {
			Version1 = 0,
			Version2,
			Version3,
			Version4,
			Version5,
			CurrentVersion = Version5,
			ValueNum
		};
	}
}

//	FUNCTION
//	LogicalFile::FileID::initialize -- FileIDの初期化
//
//	NOTES
//		CurrentVersionのFileID用のレイアウトマップを登録する
//		★注意★
//		呼び出し側で排他する必要がある

//static
void
FileID::
initialize()
{
	// 最新バージョンのエントリー配列を作っておく
	if (!_bMapInitialized) {
		createLayoutMap(_VersionNumber::CurrentVersion, _cLayoutMap);
		_bMapInitialized = true;
	}
}

FileID::
FileID()
	: Base()
{
	Base::setLayoutMap(_VersionNumber::CurrentVersion);
}

FileID::
FileID(const FileID& cOther_)
	: Base(cOther_)
{}

FileID::
~FileID()
{}

int
FileID::
getClassID() const
{
	return Common::Externalizable::LogicalFileClasses
		+ Externalizable::Category::FileID;
}

// バージョン番号に対応したレイアウトマップを取得する
//virtual
const Parameter::LayoutMap*
FileID::
getLayoutMap(int iVersion_) const
{
	return (iVersion_ == _VersionNumber::CurrentVersion) ?
		&_cLayoutMap : getOldLayoutMap(iVersion_);
}

// 古いバージョンのレイアウトマップを取得する
// static
const Parameter::LayoutMap*
FileID::
getOldLayoutMap(int iVersion_)
{
	Os::AutoCriticalSection latch(_latch);

	// なければ新たにLayoutMapを挿入して、あればその位置を得る
	ModPair<_LayoutMapMap::Iterator, ModBoolean> e =
		_cOldLayoutMap.insert(iVersion_, Parameter::LayoutMap());
	; _SYDNEY_ASSERT(e.first != _cOldLayoutMap.end());

	if (e.second == ModTrue) {
		// 新たに挿入されている場合は値を代入する
		createLayoutMap(iVersion_, (*e.first).second);
	}
	return &((*e.first).second);
}

// バージョン番号に対応したレイアウトマップを生成する
//	★注意★
//	呼び出し側で排他する必要がある

//static
void
FileID::
createLayoutMap(int iVersion_, Parameter::LayoutMap& cMap_)
{
#define S Parameter::LayoutMapEntry::Type::String
#define I Parameter::LayoutMapEntry::Type::Integer
#define L Parameter::LayoutMapEntry::Type::LongLong
#define B Parameter::LayoutMapEntry::Type::Boolean
#define D Parameter::LayoutMapEntry::Type::Double
#define O Parameter::LayoutMapEntry::Type::ObjectPointer
#define OFFSET_OF(_T_, _m_) (intptr_t)(&(((_T_*)0)->_m_))
#define ENTRY(_v_, _t_, _b_, _m_) ModPair<int, Parameter::LayoutMapEntry>(KeyNumber::_v_, Parameter::LayoutMapEntry(_t_, _b_, OFFSET_OF(FileID, _m_)))

	// Version1
	const ModPair<int, Parameter::LayoutMapEntry> _cFileIDEntries_Version1[] =
	{ 
		ENTRY(Mounted,			B, false, m_pbMounted),
		ENTRY(Area,				S, true , m_pvecArea),
		ENTRY(Temporary,		B, false, m_pbTemporary),
		ENTRY(PageSize,			I, false, m_piPageSize),
		ENTRY(FieldNumber,		I, false, m_piFieldNumber),
		ENTRY(FieldType,		I, true , m_pvecFieldType),
		ENTRY(FieldLength,		I, true , m_pvecFieldLength),
		ENTRY(FieldFraction,	I, true , m_pvecFieldFraction),
		ENTRY(ElementType,		I, true , m_pvecElementType),
		ENTRY(ElementLength,	I, true , m_pvecElementLength),
		ENTRY(ReadOnly,			B, false, m_pbReadOnly),
		ENTRY(AreaHint,			S, false, m_pcstrAreaHint),
		ENTRY(FileHint,			S, false, m_pcstrFileHint),
		ENTRY(FieldHint,		S, true , m_pvecFieldHint),
		ENTRY(SchemaDatabaseID,	I, false, m_piDatabaseID),
		ENTRY(SchemaTableID,	I, false, m_piTableID),
		ENTRY(SchemaFileObjectID,I, false, m_piFileID),
		ENTRY(KeyFieldNumber,	I, false, m_piKeyFieldNumber),
		ENTRY(Unique,			I, false, m_piUnique),
		ENTRY(Version,			I, false, m_piVersion)
	};

	// Version2
	const ModPair<int, Parameter::LayoutMapEntry> _cFileIDEntries_Version2[] =
	{
		ENTRY(Mounted,			B, false, m_pbMounted),				// Version1
		ENTRY(Area,				S, true , m_pvecArea),				// Version1
		ENTRY(Temporary,		B, false, m_pbTemporary),			// Version1
		ENTRY(PageSize,			I, false, m_piPageSize),			// Version1
		ENTRY(FieldNumber,		I, false, m_piFieldNumber),			// Version1
		ENTRY(FieldType,		I, true , m_pvecFieldType),			// Version1
		ENTRY(FieldLength,		I, true , m_pvecFieldLength),		// Version1
		ENTRY(FieldEncodingForm,I, true , m_pvecFieldEncodingForm),
		ENTRY(FieldFraction,	I, true , m_pvecFieldFraction),		// Version1
		ENTRY(ElementType,		I, true , m_pvecElementType),		// Version1
		ENTRY(ElementLength,	I, true , m_pvecElementLength),		// Version1
		ENTRY(ElementEncodingForm,I, true , m_pvecElementEncodingForm),
		ENTRY(ReadOnly,			B, false, m_pbReadOnly),			// Version1
		ENTRY(AreaHint,			S, false, m_pcstrAreaHint),			// Version1
		ENTRY(FileHint,			S, false, m_pcstrFileHint),			// Version1
		ENTRY(FieldHint,		S, true , m_pvecFieldHint),			// Version1
		ENTRY(SchemaDatabaseID,	I, false, m_piDatabaseID),			// Version1
		ENTRY(SchemaTableID,	I, false, m_piTableID),				// Version1
		ENTRY(SchemaFileObjectID,I, false, m_piFileID),				// Version1
		ENTRY(KeyFieldNumber,	I, false, m_piKeyFieldNumber),		// Version1
		ENTRY(Unique,			I, false, m_piUnique),				// Version1
		ENTRY(Version,			I, false, m_piVersion)				// Version1
	};

	// Version3
	const ModPair<int, Parameter::LayoutMapEntry> _cFileIDEntries_Version3[] =
	{
		ENTRY(Mounted,			B, false, m_pbMounted),				// Version1
		ENTRY(Area,				S, true , m_pvecArea),				// Version1
		ENTRY(Temporary,		B, false, m_pbTemporary),			// Version1
		ENTRY(PageSize,			I, false, m_piPageSize),			// Version1
		ENTRY(FieldNumber,		I, false, m_piFieldNumber),			// Version1
		ENTRY(FieldType,		I, true , m_pvecFieldType),			// Version1
		ENTRY(FieldLength,		I, true , m_pvecFieldLength),		// Version1
		ENTRY(FieldEncodingForm,I, true , m_pvecFieldEncodingForm),	// Version2
		ENTRY(FieldFraction,	I, true , m_pvecFieldFraction),		// Version1
		ENTRY(FieldFixed,		B, true , m_pvecFieldFixed),
		ENTRY(ElementType,		I, true , m_pvecElementType),		// Version1
		ENTRY(ElementLength,	I, true , m_pvecElementLength),		// Version1
		ENTRY(ElementEncodingForm,I, true , m_pvecElementEncodingForm),	// Version2
		ENTRY(ElementFixed,		B, true , m_pvecElementFixed),
		ENTRY(ReadOnly,			B, false, m_pbReadOnly),			// Version1
		ENTRY(AreaHint,			S, false, m_pcstrAreaHint),			// Version1
		ENTRY(FileHint,			S, false, m_pcstrFileHint),			// Version1
		ENTRY(FieldHint,		S, true , m_pvecFieldHint),			// Version1
		ENTRY(SchemaDatabaseID,	I, false, m_piDatabaseID),			// Version1
		ENTRY(SchemaTableID,	I, false, m_piTableID),				// Version1
		ENTRY(SchemaFileObjectID,I, false, m_piFileID),				// Version1
		ENTRY(KeyFieldNumber,	I, false, m_piKeyFieldNumber),		// Version1
		ENTRY(Unique,			I, false, m_piUnique),				// Version1
		ENTRY(Version,			I, false, m_piVersion)				// Version1
	};

	// Version4
	const ModPair<int, Parameter::LayoutMapEntry> _cFileIDEntries_Version4[] =
	{
		ENTRY(Mounted,			B, false, m_pbMounted),				// Version1
		ENTRY(Area,				S, true , m_pvecArea),				// Version1
		ENTRY(Temporary,		B, false, m_pbTemporary),			// Version1
		ENTRY(PageSize,			I, false, m_piPageSize),			// Version1
		ENTRY(FieldNumber,		I, false, m_piFieldNumber),			// Version1
		ENTRY(FieldType,		I, true , m_pvecFieldType),			// Version1
		ENTRY(FieldLength,		I, true , m_pvecFieldLength),		// Version1
		ENTRY(FieldEncodingForm,I, true , m_pvecFieldEncodingForm),	// Version2
		ENTRY(FieldFraction,	I, true , m_pvecFieldFraction),		// Version1
		ENTRY(FieldFixed,		B, true , m_pvecFieldFixed),		// Version3
		ENTRY(ElementType,		I, true , m_pvecElementType),		// Version1
		ENTRY(ElementLength,	I, true , m_pvecElementLength),		// Version1
		ENTRY(ElementEncodingForm,I, true , m_pvecElementEncodingForm),	// Version2
		ENTRY(ElementFixed,		B, true , m_pvecElementFixed),		// Version3
		ENTRY(ReadOnly,			B, false, m_pbReadOnly),			// Version1
		ENTRY(AreaHint,			S, false, m_pcstrAreaHint),			// Version1
		ENTRY(FileHint,			S, false, m_pcstrFileHint),			// Version1
		ENTRY(FieldHint,		S, true , m_pvecFieldHint),			// Version1
		ENTRY(SchemaDatabaseID,	I, false, m_piDatabaseID),			// Version1
		ENTRY(SchemaTableID,	I, false, m_piTableID),				// Version1
		ENTRY(SchemaFileObjectID,I, false, m_piFileID),				// Version1
		ENTRY(KeyFieldNumber,	I, false, m_piKeyFieldNumber),		// Version1
		ENTRY(VirtualFieldNumber,	I, false, m_piVirtualFieldNumber),
		ENTRY(Unique,			I, false, m_piUnique),				// Version1
		ENTRY(Version,			I, false, m_piVersion)				// Version1
	};

	// Version5
	const ModPair<int, Parameter::LayoutMapEntry> _cFileIDEntries_Version5[] =
	{
		ENTRY(Mounted,			B, false, m_pbMounted),				// Version1
		ENTRY(Area,				S, true , m_pvecArea),				// Version1
		ENTRY(Temporary,		B, false, m_pbTemporary),			// Version1
		ENTRY(PageSize,			I, false, m_piPageSize),			// Version1
		ENTRY(FieldNumber,		I, false, m_piFieldNumber),			// Version1
		ENTRY(FieldType,		I, true , m_pvecFieldType),			// Version1
		ENTRY(FieldLength,		I, true , m_pvecFieldLength),		// Version1
		ENTRY(FieldEncodingForm,I, true , m_pvecFieldEncodingForm),	// Version2
		ENTRY(FieldFraction,	I, true , m_pvecFieldFraction),		// Version1
		ENTRY(FieldFixed,		B, true , m_pvecFieldFixed),		// Version3
		ENTRY(FieldCollation,	I, true , m_pvecFieldCollation),
		ENTRY(ElementType,		I, true , m_pvecElementType),		// Version1
		ENTRY(ElementLength,	I, true , m_pvecElementLength),		// Version1
		ENTRY(ElementEncodingForm,I, true , m_pvecElementEncodingForm),	// Version2
		ENTRY(ElementFixed,		B, true , m_pvecElementFixed),		// Version3
		ENTRY(ReadOnly,			B, false, m_pbReadOnly),			// Version1
		ENTRY(AreaHint,			S, false, m_pcstrAreaHint),			// Version1
		ENTRY(FileHint,			S, false, m_pcstrFileHint),			// Version1
		ENTRY(FieldHint,		S, true , m_pvecFieldHint),			// Version1
		ENTRY(SchemaDatabaseID,	I, false, m_piDatabaseID),			// Version1
		ENTRY(SchemaTableID,	I, false, m_piTableID),				// Version1
		ENTRY(SchemaFileObjectID,I, false, m_piFileID),				// Version1
		ENTRY(KeyFieldNumber,	I, false, m_piKeyFieldNumber),		// Version1
		ENTRY(VirtualFieldNumber,	I, false, m_piVirtualFieldNumber),// Version4
		ENTRY(Unique,			I, false, m_piUnique),				// Version1
		ENTRY(Version,			I, false, m_piVersion)				// Version1
	};

//------------------------------------------
// 追加するときは以下のように定義を追加する
//------------------------------------------
//	// Version6
//	const ModPair<int, Parameter::LayoutMapEntry> _cFileIDEntries_Version6[] =
//		{...}
//	};

#undef ENTRY
#undef OFFSET_OF
#undef S
#undef I
#undef L
#undef B
#undef D
#undef O

	switch (iVersion_) {
	case _VersionNumber::Version1:
		assignMap(cMap_, KeyNumber::ValueNum,
				  &_cFileIDEntries_Version1[0],
				  &_cFileIDEntries_Version1[
						 sizeof(_cFileIDEntries_Version1) /
						 sizeof(ModPair<int, Parameter::LayoutMapEntry>)]);
		break;

	case _VersionNumber::Version2:
		assignMap(cMap_, KeyNumber::ValueNum,
				  &_cFileIDEntries_Version2[0],
				  &_cFileIDEntries_Version2[
						 sizeof(_cFileIDEntries_Version2) /
						 sizeof(ModPair<int, Parameter::LayoutMapEntry>)]);
		break;

	case _VersionNumber::Version3:
		assignMap(cMap_, KeyNumber::ValueNum,
				  &_cFileIDEntries_Version3[0],
				  &_cFileIDEntries_Version3[
						 sizeof(_cFileIDEntries_Version3) /
						 sizeof(ModPair<int, Parameter::LayoutMapEntry>)]);
		break;

	case _VersionNumber::Version4:
		assignMap(cMap_, KeyNumber::ValueNum,
				  &_cFileIDEntries_Version4[0],
				  &_cFileIDEntries_Version4[
						 sizeof(_cFileIDEntries_Version4) /
						 sizeof(ModPair<int, Parameter::LayoutMapEntry>)]);
		break;

	case _VersionNumber::Version5:
		assignMap(cMap_, KeyNumber::ValueNum,
				  &_cFileIDEntries_Version5[0],
				  &_cFileIDEntries_Version5[
						 sizeof(_cFileIDEntries_Version5) /
						 sizeof(ModPair<int, Parameter::LayoutMapEntry>)]);
		break;

//------------------------------------------
// Versionが追加されたときはcase節を追加する
//------------------------------------------
//	case _VersionNumber::Version6:
//		assignMap(cMap_, KeyNumber::ValueNum,
//				  &_cFileIDEntries_Version6[0],
//				  &_cFileIDEntries_Version6[
//						 sizeof(_cFileIDEntries_Version6) /
//						 sizeof(ModPair<int, Parameter::LayoutMapEntry>)]);
//		break;

	default:
		_SYDNEY_THROW0(Exception::BadArgument);
	}
}

//
//	Copyright (c) 2001, 2002, 2004, 2005, 2007, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
