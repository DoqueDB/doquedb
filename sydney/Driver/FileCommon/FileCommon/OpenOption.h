// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenOption.h -- 論理ファイル共通オープンオプションのヘッダーファイル
// 
// Copyright (c) 2000, 2001, 2002, 2005, 2008, 2012, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FILECOMMON_OPENOPTION_H
#define __SYDNEY_FILECOMMON_OPENOPTION_H

#include "Common/Common.h"
#include "LogicalFile/OpenOption.h"

_SYDNEY_BEGIN

namespace FileCommon
{

namespace OpenOption
{
//------------------------
//	OpenParameter の型変更に関するマクロ
//------------------------

namespace Parameter {
typedef	LogicalFile::Parameter::Key		KeyType;
typedef	ModUnicodeString				ValueType;
}

#define _SYDNEY_OPEN_PARAMETER_KEY(key_)              FileCommon::OpenOption::Parameter::KeyType((key_))
#define _SYDNEY_OPEN_PARAMETER_FORMAT_KEY(key_, arg_) FileCommon::OpenOption::Parameter::KeyType((key_), (arg_))
#define _SYDNEY_OPEN_PARAMETER_VALUE(value_)          (value_)

namespace OpenMode
{

//
//	CONST
//	FileCommon::OpenOption::OpenMode::Key --
//		オープンモードを指定するパラメータのキー
//
//	NOTES
//	ファイルのオープンモードを指定するパラメータのキー。
//	このパラメータのバリューには、
//		・FileCommon::OpenOption::OpenMode::Read
//		・FileCommon::OpenOption::OpenMode::Update
//	のいずれかを設定する。
//
const int Key = LogicalFile::OpenOption::KeyNumber::OpenMode;

//
//	CONST
//	FileCommon::OpenOption::OpenMode::Unknown --
//		不定値のオープンモードパラメータのバリュー
//
//	NOTES
//	不定値のオープンモードパラメータのバリュー。この値が得られたらNG
//
const int Unknown = LogicalFile::OpenOption::OpenMode::Unknown;

//
//	CONST
//	FileCommon::OpenOption::OpenMode::Read --
//		オブジェクト取得モードを示すオープンモードパラメータのバリュー
//
//	NOTES
//	オブジェクト取得モードを示すオープンモードパラメータのバリュー。
//
const int Read = LogicalFile::OpenOption::OpenMode::Read;

//
//	CONST
//	FileCommon::OpenOption::OpenMode::Search --
//		オブジェクト検索モードを示すオープンモードパラメータのバリュー
//
//	NOTES
//	オブジェクト検索モードを示すオープンモードパラメータのバリュー。
//
const int Search = LogicalFile::OpenOption::OpenMode::Search;

//
//	CONST
//	FileCommon::OpenOption::OpenMode::Update --
//	オブジェクト更新モードを示すオープンモードパラメータのバリュー
//
//	NOTES
//	オブジェクト更新モードを示すオープンモードパラメータのバリュー。
//
const int Update = LogicalFile::OpenOption::OpenMode::Update;

//
//	CONST
//	FileCommon::OpenOption::OpenMode::Initialize --
//	システム初期化時のオープンモードを示す
//	オープンモードパラメータのバリュー
//
//	NOTES
//	システム初期化時のオープンモードを示す
//	オープンモードパラメータのバリュー。
//
const int Initialize = LogicalFile::OpenOption::OpenMode::Initialize;

//
//	CONST
//	FileCommon::OpenOption::OpenMode::Batch --
//	バッチインサートモードを示すオープンモードパラメータのバリュー
//
//	NOTES
//	バッチインサートモードを示すオープンモードパラメータのバリュー
//
const int Batch = LogicalFile::OpenOption::OpenMode::Batch;

} // end of namespace OpenMode


// オブジェクト取得サブモードを指定するパラメータ
namespace ReadSubMode // 2000-12-13追加
{

//
//	CONST
//	FileCommon::OpenOption::ReadSubMode::Key --
//		オブジェクト取得サブモードを指定するパラメータのキー
//
//	NOTES
//	関数 getData でオブジェクトを取得する際の
//	取得モードを指定するパラメータのキー。
//	このパラメータの値には
//		・FileCommon::OpenOption::ReadSubMode::Scan
//		・FileCommon::OpenOption::ReadSubMode::Fetch
//	のいずれかの文字列を設定する。
//	省略時には、Scan モードでのオブジェクト取得となる
//
//const char*	const Key = "ReadSubMode";
const int Key = LogicalFile::OpenOption::KeyNumber::ReadSubMode;

//
//	CONST
//	FileCommon::OpenOption::ReadSubMode::Scan --
//		Scan モードを示すオブジェクト取得サブモードパラメータのバリュー
//
//	NOTES
//	Scan モードを示すオブジェクト取得サブモードパラメータのバリュー。
//	オブジェクト取得サブモードパラメータにこの値（文字列）を設定すると
//	オブジェクト取得時にオプションの指定ができなくなる。
//
//const char*	const Scan = "Scan";
const int Scan = LogicalFile::OpenOption::ReadSubMode::Scan;

//
//	CONST
//	FileCommon::OpenOption::ReadSubMode::Fetch --
//		Fetch モードを示すオブジェクト取得サブモードパラメータのバリュー
//
//	NOTES
//	Fetch モードを示すオブジェクト取得サブモードパラメータのバリュー。
//	オブジェクト取得サブモードパラメータにこの値（文字列）を設定すると
//	オブジェクト取得時には毎回オプションを指定する必要がある。
//
//const char*	const Fetch = "Fetch";
const int Fetch = LogicalFile::OpenOption::ReadSubMode::Fetch;

} // end of namespace ReadSubMode

// 見積りフラグを指定するパラメータ
namespace Estimate
{

//
//	CONST
//	FileCommon::OpenOption::Estimate::Key -- 見積りフラグを指定するパラメータのキー
//
//	NOTES
//	見積りフラグを指定するパラメータのキー。
//	ファイルサイズや挿入されているオブジェクト数などを返す見積り関数を
//	呼び出すためにファイルをオープンする場合には、
//	このパラメータのバリューに true を、そうでない場合には false を設定する。
//
//const char*	const Key = "Estimate";
const int Key = LogicalFile::OpenOption::KeyNumber::Estimate;

} // end of namespace Estimate


// フィールド選択フラグを指定するパラメータ
namespace FieldSelect
{

//
//	CONST
//	FileCommon::OpenOption::FieldSelect::Key --
//		フィールド選択フラグを指定するパラメータのキー
//
//	NOTES
//	フィールド選択フラグを指定するパラメータのキー。
//	このパラメータは以下に示す 2 通りの使われ方をする。
//	#1
//	  オブジェクト取得のためにオープンする場合に
//	  このパラメータに true が設定されているとファイルは、
//	  FileCommon::OpenOption::TargetFieldIndex パラメータで指定されている
//	  フィールドのみでオブジェクトを生成し、ユーザに返す。
//	#2
//	  オブジェクト更新のためにオープンする場合に
//	  このパラメータに true が設定されているとファイルは、
//	  FileCommon::OpenOption::TargetFieldIndex パラメータで指定されている
//	  フィールドのみの更新を行なう。
//
//const char*	const Key = "FieldSelect";
const int Key = LogicalFile::OpenOption::KeyNumber::FieldSelect;

} // end of namespace FieldSelect


// 対象フィールド数を指定するパラメータ
namespace TargetFieldNumber
{

//
//	CONST
//	FileCommon::OpenOption::TargetFieldNumber::Key --
//		処理対象フィールド数を指定するパラメータのキー
//
//	NOTES
//	処理対象フィールド数を指定するパラメータのキー。
//
//const char*	const Key = "TargetFieldNumber";
const int Key = LogicalFile::OpenOption::KeyNumber::TargetFieldNumber;

} // end of namespace TargetFieldNumber


// 対象フィールドインデックスを指定するパラメータ
namespace TargetFieldIndex
{

//
//	CONST
//	FileCommon::OpenOption::TargetFieldIndex::Key --
//		対象フィールドインデックスを指定するパラメータのキー
//
//	NOTES
//	FileCommon::OpenOption::FieldSelect::Key のコメント参照。
//	このパラメータのバリューには整数値を設定する。
//
//const char*	const Key = "TargetFieldIndex[%d]";
const int Key = LogicalFile::OpenOption::KeyNumber::TargetFieldIndex;

} // end of namespace TargetFieldIndex

namespace GetByBitSet
{

//	CONST
//	FileCommon::OpenOption::GetByBitSet::Key --
//		BitSetで値を返すことを指定するパラメータのキー
//
//	NOTES
//	BitSetで値を返すことを指定するパラメータのキー。
//	このパラメータの値にはboolを設定する。
//
//const char*	const Key = "GetByBitSet";
const int Key = LogicalFile::OpenOption::KeyNumber::GetByBitSet;

} // end of namespace GetByBitSet

namespace CacheAllObject
{

//
//	CONST
//	FileCommon::OpenOption::CacheAllObject::Key --
//		初回File::get()で検索条件と一致するすべてのオブジェクトを
//		ドライバが保持するかどうかを示すパラメータのキー
//
//	NOTES
//	初回File::get()で検索条件と一致するすべてのオブジェクトを
//	ドライバが保持するかどうかを示すパラメータのキー。
//	このパラメータの値は、ドライバがboolを設定する。
//
//const char*	const Key = "CacheAllObject";
const int Key = LogicalFile::OpenOption::KeyNumber::CacheAllObject;

} // end of namespace CacheAllObject

namespace SearchByBitSet
{

//	CONST
//	FileCommon::OpenOption::SearchByBitSet::Key --
//		検索時にBitSetで絞り込み対象を指定するパラメータのキー
//
//	NOTES
//	このパラメータの値にはCommon::Object*(実体はCommon::BitSet)を設定する。
//
const int Key = LogicalFile::OpenOption::KeyNumber::SearchByBitSet;

} // end of namespace SearchByBitSet

namespace GroupBy
{
//	CONST
//	FileCommon::OpenOption::GroupBy::Key --
//		BitSetで値を返すことを指定するパラメータのキー
//
//	NOTES
//	GroupByをBitSetで値を返すことを意味するパラメータのキー。
//	値はFileDriver側でsetする。
//	このパラメータの値にはboolを設定する。
//

const int Key = LogicalFile::OpenOption::KeyNumber::GroupBy;

} // end of namespace GroupBy

namespace RankByBitSet
{

//	CONST
//	FileCommon::OpenOption::RankByBitSet::Key --
//		絞った集合でのランキング検索をする時の集合を指定するパラメータのキー
//
//	NOTES
//	このパラメータの値にはCommon::Object*(実体はCommon::BitSet)を設定する。
//
const int Key = LogicalFile::OpenOption::KeyNumber::RankByBitSet;

} // end of namespace RankByBitSet

namespace GetForConstraintLock
{

//	CONST
//	FileCommon::OpenOption::GetForConstraintLock::Key --
//		制約ロックのための検索か否かを指定するパラメータのキー
//
//	NOTES
//	このパラメータの値にはboolを設定する。
//
const int Key = LogicalFile::OpenOption::KeyNumber::GetForConstraintLock;

} // end of namespace GetForConstraintLock

} // end of namespace OpenOption

} // end of namespace FileCommon

_SYDNEY_END

#endif // __SYDNEY_FILECOMMON_OPENOPTION_H

//
//	Copyright (c) 2000, 2001, 2002, 2005, 2008, 2012, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
