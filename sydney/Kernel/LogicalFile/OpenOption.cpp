// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenOption.cpp -- ファイルオープンオプション
// 
// Copyright (c) 2002, 2004, 2005, 2009, 2012, 2014, 2023 Ricoh Company, Ltd.
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

#include "Common/Assert.h"
#include "Exception/BadArgument.h"
#include "LogicalFile/OpenOption.h"
#include "Os/AutoCriticalSection.h"
#include "ModUnicodeString.h"
#include "ModMap.h"

_SYDNEY_USING
using namespace LogicalFile;

namespace {
	//	VARIABLE
	//	$$$::_bMapInitialized -- LayoutMapが初期化されたかを示す
	//
	//	NOTES
	bool _bMapInitialized = false;

	//	VARIABLE
	//	$$$::_cLayoutMap -- OpenOptionのLayoutMap
	//
	//	NOTES
	Parameter::LayoutMap _cLayoutMap;

	//	VARIABLE
	//	$$$::_latch -- 以下の変数を保護するためのクリティカルセクション
	//
	//	NOTES
	Os::CriticalSection _latch;

#ifdef OBSOLETE
	//	VARIABLE
	//	$$$::_cOldLayoutMap -- 旧バージョンのLayoutMap
	//
	//	NOTES
	typedef ModMap<int, Parameter::LayoutMap, ModLess<int> > _LayoutMapMap;
	_LayoutMapMap _cOldLayoutMap; // 旧バージョンのレイアウトマップ
#endif

	//	CONST
	//	$$$::_VersionNumber::_Value -- レイアウトマップのバージョン番号
	//
	//	NOTES
	// Versionが追加されたときはcreateLayoutMap()およびgetLayoutMap()を修正する
	// -> OpenOptionは永続化されないのでVersionをあげなくてよい
	namespace _VersionNumber {
		enum _Value {
			Version1 = 0,
			CurrentVersion = Version1,
			ValueNum
		};
	}
}

//	FUNCTION
//	LogicalFile::OpenOption::initialize -- OpenOptionの初期化
//
//	NOTES
//		CurrentVersionのOpenOption用のレイアウトマップを登録する
//		★注意★
//		呼び出し側で排他する必要がある

//static
void
OpenOption::
initialize()
{
	// 最新バージョンのエントリー配列を作っておく
	if (!_bMapInitialized) {
		createLayoutMap(_VersionNumber::CurrentVersion, _cLayoutMap);
		_bMapInitialized = true;
	}
}

OpenOption::
OpenOption()
	: Base()
{
	Base::setLayoutMap(_VersionNumber::CurrentVersion);
}

OpenOption::
OpenOption(const OpenOption& cOther_)
	: Base(cOther_)
{}

OpenOption::
~OpenOption()
{}

int
OpenOption::
getClassID() const
{
	return Common::Externalizable::LogicalFileClasses
		+ Externalizable::Category::OpenOption;
}

// バージョン番号に対応したレイアウトマップを取得する
//virtual
const Parameter::LayoutMap*
OpenOption::
getLayoutMap(int iVersion_) const
{
#if 0 //Versionが追加されたときに修正する
	switch (iVersion_) {
	case _VersionNumber::CurrentVersion:
		return &_cLayoutMap;
	default:
		return getOldLayoutMap(iVersion_);
	}
	// never reach
#else
	_SYDNEY_ASSERT(iVersion_ == _VersionNumber::CurrentVersion);
	return &_cLayoutMap;
#endif //OBSOLETE
}

#ifdef OBSOLETE
// 古いバージョンのレイアウトマップを取得する
// static
const Parameter::LayoutMap*
OpenOption::
getOldLayoutMap(int iVersion_)
{
	Os::AutoCriticalSection m(_latch);

	// なければ新たにLayoutMapを挿入して、あればその位置を得る
	ModPair<_LayoutMapMap::Iterator, ModBoolean> e = _cOldLayoutMap.insert(iVersion_, Parameter::LayoutMap());
	; _SYDNEY_ASSERT(e.first != _cOldLayoutMap.end());

	if (e.second == ModTrue) {
		// 新たに挿入されている場合は値を代入する
		createLayoutMap(iVersion_, (*e.first).second);
	}
	return &((*e.first).second);
}
#endif //OBSOLETE

// バージョン番号に対応したレイアウトマップを生成する
//	★注意★
//	呼び出し側で排他する必要がある

// static
void
OpenOption::
createLayoutMap(int iVersion_, Parameter::LayoutMap& cMap_)
{
#define S Parameter::LayoutMapEntry::Type::String
#define I Parameter::LayoutMapEntry::Type::Integer
#define L Parameter::LayoutMapEntry::Type::LongLong
#define B Parameter::LayoutMapEntry::Type::Boolean
#define D Parameter::LayoutMapEntry::Type::Double
#define O Parameter::LayoutMapEntry::Type::ObjectPointer
#define OFFSET_OF(_T_, _m_) (intptr_t)(&(((_T_*)0)->_m_))
#define ENTRY(_v_, _t_, _b_, _m_) ModPair<int, Parameter::LayoutMapEntry>(KeyNumber::_v_, Parameter::LayoutMapEntry(_t_, _b_, OFFSET_OF(OpenOption, _m_)))

	// Version1
	const ModPair<int, Parameter::LayoutMapEntry> _cOpenOptionEntries_Version1[] =
	{
		ENTRY(OpenMode,				I, false, m_piOpenMode),
		ENTRY(ReadSubMode,			I, false, m_piReadSubMode),
		ENTRY(Estimate,				B, false, m_pbEstimate),
		ENTRY(FieldSelect,			B, false, m_pbFieldSelect),
		ENTRY(TargetFieldNumber,	I, false, m_piTargetFieldNumber),
		ENTRY(TargetFieldIndex,		I, true , m_pvecTargetFieldIndex),
		ENTRY(GetByBitSet,			B, false, m_pbGetByBitSet),
		ENTRY(CacheAllObject,		B, false, m_pbCacheAllObject),
		ENTRY(SearchByBitSet,		O, false, m_ppSearchByBitSet),
		ENTRY(GroupBy,				B, false, m_pbGroupBy),
		ENTRY(RankByBitSet,			O, false, m_ppRankByBitSet),
		ENTRY(GetForConstraintLock,	B, false, m_pbGetForConstraintLock),
	};

//------------------------------------------
// 追加するときは以下のように定義を追加する
//------------------------------------------
//	// Version2
//	const ModPair<int, Parameter::LayoutMapEntry> _cOpenOptionEntries_Version2[] =
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
				  &_cOpenOptionEntries_Version1[0],
				  &_cOpenOptionEntries_Version1[
						 sizeof(_cOpenOptionEntries_Version1) /
						 sizeof(ModPair<int, Parameter::LayoutMapEntry>)]);
		break;
//------------------------------------------
// Versionが追加されたときはcase節を追加する
//------------------------------------------
//	case _VersionNumber::Version2:
//		assignMap(cMap_, KeyNumber::ValueNum,
//				  &_cOpenOptionEntries_Version2[0],
//				  &_cOpenOptionEntries_Version2[
//						 sizeof(_cOpenOptionEntries_Version2) /
//						 sizeof(ModPair<int, Parameter::LayoutMapEntry>)]);
//		break;
	default:
		_SYDNEY_THROW0(Exception::BadArgument);
	}
}

//
//	Copyright (c) 2002, 2004, 2005, 2009, 2012, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
