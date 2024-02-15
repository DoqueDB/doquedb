// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenOption.h -- Ｂ＋木ファイルオープンオプションのヘッダーファイル
// 
// Copyright (c) 2000,2001,2002, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE_OPENOPTION_H
#define __SYDNEY_BTREE_OPENOPTION_H

#include "FileCommon/OpenOption.h"
#include "LogicalFile/OpenOption.h"

_SYDNEY_BEGIN

namespace Btree
{

namespace OpenOption
{
//------------------------
//	OpenParameter の型変更に関するマクロ
//------------------------

namespace Parameter {
	typedef	LogicalFile::Parameter::Key		KeyType;
	typedef	ModUnicodeString				ValueType;

	namespace KeyNumber {
		enum Value {
			FetchFieldNumber = LogicalFile::OpenOption::DriverNumber::Btree,
			FetchFieldIndex,			// array
			SearchFieldNumber,
			SearchFieldIndex,			// array
			SearchStart,				// array
			SearchStop,					// array
			SearchStartOpe,				// array
			SearchStopOpe,				// array
			SortKey,
			SortReverse,
			Escape,
			ValueNum
		};
	}

	namespace Operator {
		enum Value {
			Equals = 0,
			GreaterThan,
			GreaterThanEquals,
			LessThan,
			LessThanEquals,
			EqualsToNull,
			Like,
			ValueNum
		};
	}

	namespace SortKey {
		enum Value {
			ObjectID,
			KeyField,
			ValueNum
		};
	}
}

#define _SYDNEY_BTREE_OPEN_PARAMETER_KEY(key_)              Btree::OpenOption::Parameter::KeyType((key_))
#define _SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(key_, arg_) Btree::OpenOption::Parameter::KeyType((key_), (arg_))
#define _SYDNEY_BTREE_OPEN_PARAMETER_VALUE(value_)          value_

// Fetch 対象フィールド数を指定するパラメータ
namespace FetchFieldNumber
{

//
//	CONST
//	Btree::OpenOption::FetchFieldNumber::Key --
//		Fetch 対象フィールド数を指定するパラメータのキー
//
//	NOTES
//	Fetch 対象フィールド数を指定するパラメータのキー
//	パラメータ ReadSubMode のバリューが
//	"Fetch" の場合に有効となり、省略不可。
//	（ "Scan" の場合はもちろん省略可能。）
//	このパラメータのバリューのデータ型は
//	Common::Parameter::TypeInteger で
//	符号付き 32 ビット整数値を設定する。
//
//const char*	const Key = "FetchFieldNumber";
const int Key = Parameter::KeyNumber::FetchFieldNumber;

} // end of namespace Btree::OpenOption::FetchFieldNumber

// Fetch 対象フィールドインデックスを指定するパラメータ
namespace FetchFieldIndex
{

//
//	CONST
//	Btree::OpenOption::FetchFieldIndex::Key --
//		Fetch 対象フィールドインデックスを指定するパラメータ
//
//	NOTES
//	Fetch 対象フィールドインデックスを指定するパラメータ。
//	このパラメータのバリューのデータ型は
//	Common::Parameter::TypeInteger で
//	符号付き 32 ビット整数値を設定する。
//	"Fetch" の場合は省略不可。
//
//const char*	const Key = "FetchFieldIndex[%d]";
const int Key = Parameter::KeyNumber::FetchFieldIndex;

} // end of namespace Btree::OpenOption::FetchFieldIndex



// 検索対象キーフィールド数を指定するパラメータ
namespace SearchFieldNumber
{

//
//	CONST
//	Btree::OpenOption::SearchFieldNumber::Key --
//		検索対象キーフィールド数を指定するパラメータのキー
//
//	NOTES
//	キーフィールドに対する検索を行なう場合、このパラメータのバリューで
//	検索対象となるキーフィールド数を指定する。
//	また、オブジェクト ID での検索を行なう場合には
//	    ┌──── key  ────┬ value  ┐
//	    │ "SearchFieldNumber"  │   1    │
//		└───────────┴────┘
//	のように指定する。
//	このパラメータのバリューのデータ型は
//	Common::Parameter::TypeInteger で
//	符号付き 32 ビット整数値を指定する。
//
//const char* const Key = "SearchFieldNumber";
const int Key = Parameter::KeyNumber::SearchFieldNumber;

} // end of namespace Btree::OpenOption::SearchFieldNumber

// 検索対象キーフィールドインデックスを指定するパラメータ
namespace SearchFieldIndex
{

//
//	CONST
//	Btree::OpenOption::SearchFieldIndex::Key --
//		検索対象キーフィールドインデックスを指定するパラメータのキー
//
//	NOTES
//	キーフィールドに対する検索を行なう場合、このパラメータのバリューで
//	検索対象となるキーフィールドのインデックス（オブジェクト内での
//	インデックス）を指定する。
//	また、オブジェクト ID での検索を行なう場合には
//	    ┌──── key  ────┬ value  ┐
//	    │"SearchFieldIndex[0]" │   0    │
//		└───────────┴────┘
//	のように指定する。（ value の "0" という値は、
//	オブジェクト ID フィールドのインデックスである。）
//	このパラメータのバリューには整数値を指定する。
//	
//
//const char* const Key = "SearchFieldIndex[%d]";
const int Key = Parameter::KeyNumber::SearchFieldIndex;

} // end of namespace Btree::OpenOption::SearchFieldIndex

// 検索対象キーフィールド開始値を指定するパラメータ
namespace SearchStart
{

//
//	CONST
//	Btree::OpenOption::SearchStart::Key --
//		検索対象キーフィールド開始値を指定するパラメータのキー
//
//	NOTES
//	キーフィールドに対する検索を行なう場合、このパラメータのバリューで
//	検索対象となるキーフィールドの検索範囲開始値を文字列表現で指定する。
//	また、オブジェクト ID での検索を行なう場合には
//	このパラメータのバリューでオブジェクト ID の範囲開始値（こちらは
//	整数値）を指定する。
//
//const char* const Key = "SearchStart[%d]";
const int Key = Parameter::KeyNumber::SearchStart;

} // end of namespace Btree::OpenOption::SearchStart

// 検索対象キーフィールド終了値を指定するパラメータ
namespace SearchStop
{

//
//	CONST
//	Btree::OpenOption::SearchStop::Key --
//		検索対象キーフィールド終了値を指定するパラメータのキー
//
//	NOTES
//	キーフィールドに対する検索を行なう場合、このパラメータのバリューで
//	検索対象となるキーフィールドの検索範囲終了値を文字列表現で指定する。
//	また、オブジェクト ID での検索を行なう場合には
//	このパラメータのバリューでオブジェクト ID の範囲終了値（こちらは
//	整数値）を指定する。
//
//const char* const Key = "SearchStop[%d]";
const int Key = Parameter::KeyNumber::SearchStop;

} // end of namespace Btree::OpenOption::SearchStop

// 検索条件の比較演算子を指定するパラメータ
namespace Ope
{

//
//	CONST
//	Btree::OpenOption::Ope::StartKey --
//		検索対象キーフィールド開始比較演算子を指定するパラメータのキー
//
//	NOTES
//	キーフィールドに対する検索を行なう場合、このパラメータのバリューで
//	検索対象となるキーフィールドの検索範囲開始値に対する比較演算子を指定する。
//	また、オブジェクト ID での検索を行なう場合には、
//	オブジェクト ID の範囲開始値に対する比較演算子を指定する。
//	ただし、オブジェクト ID が設定されていないオブジェクトは存在しないので、
//	"EqualsToNull" 演算子は指定できない。
//	このパラメータのバリューには、
//		・Btree::OpenOption::Ope::Equals
//		・Btree::OpenOption::Ope::GreaterThan
//		・Btree::OpenOption::Ope::GreaterThanEquals
//		・Btree::OpenOption::Ope::LessThan
//		・Btree::OpenOption::Ope::LessThanEquals
//		・Btree::OpenOption::Ope::EqualsToNull
//	のいずれかを設定する。
//		・Btree::OpenOption::Ope::NotEquals
//	はない。
//
//const char* const StartKey = "SearchStartOpe[%d]";
const int StartKey = Parameter::KeyNumber::SearchStartOpe;

//
//	CONST
//	Btree:OpenOption::Ope::StopKey --
//		検索対象キーフィールド終了比較演算子を指定するパラメータのキー
//
//	NOTES
//	キーフィールドに対する検索を行なう場合、このパラメータのバリューで
//	検索対象となるキーフィールドの検索範囲終了値に対する比較演算子を指定する。
//	また、オブジェクト ID での検索を行なう場合には、
//	オブジェクト ID の範囲終了値に対する比較演算子を指定する。
//	ただし、オブジェクト ID が設定されていないオブジェクトは存在しないので、
//	"EqualsToNull" 演算子は指定できない。
//	このパラメータのバリューに設定できる比較演算子は、
//	Btree::OpenOption::Ope::StartKey と同じである。
//	
//const char* const StopKey = "SearchStopOpe[%d]";
const int StopKey = Parameter::KeyNumber::SearchStopOpe;

//
//	CONST
//	Btree::OpenOption::Ope::Equals
//		比較演算子「＝」を指定するパラメータのバリュー
//
//	NOTES
//	比較演算子「＝」を指定するパラメータのバリュー。
//
//const char* const Equals = "EQ";
const int Equals = Parameter::Operator::Equals;

//
//	CONST
//	Btree::OpenOption::Ope::GreaterThan
//		比較演算子「＞」を指定するパラメータのバリュー
//
//	NOTES
//	比較演算子「＞」を指定するパラメータのバリュー。
//
//const char* const GreaterThan = "GT";
const int GreaterThan = Parameter::Operator::GreaterThan;

//
//	CONST
//	Btree::OpenOption::Ope::GreaterThanEquals
//		比較演算子「≧」を指定するパラメータのバリュー
//
//	NOTES
//	比較演算子「≧」を指定するパラメータのバリュー。
//
//const char* const GreaterThanEquals = "GE";
const int GreaterThanEquals = Parameter::Operator::GreaterThanEquals;

//
//	CONST
//	Btree::OpenOption::Ope::LessThan
//		比較演算子「＜」を指定するパラメータのバリュー
//
//	NOTES
//	比較演算子「＜」を指定するパラメータのバリュー。
//
//const char* const LessThan = "LT";
const int LessThan = Parameter::Operator::LessThan;

//
//	CONST
//	Btree::OpenOption::Ope::LessThan
//		比較演算子「≦」を指定するパラメータのバリュー
//
//	NOTES
//	比較演算子「≦」を指定するパラメータのバリュー。
//
//const char* const LessThanEquals = "LE";
const int LessThanEquals = Parameter::Operator::LessThanEquals;

//
//	CONST
//	Btree::OpenOption::Ope::EqualsToNull
//		ヌル値との比較演算子を指定するパラメータのバリュー
//
//	NOTES
//	ヌル値との比較演算子を指定するパラメータのバリュー。
//
//const char* const EqualsToNull = "EN";
const int EqualsToNull = Parameter::Operator::EqualsToNull;

//
//	CONST
//	Btree::OpenOption::Ope::Like
//		likeを指定するパラメータのバリュー
//
//	NOTES
//	likeを指定するパラメータのバリュー。
//
//const char*	const Like = "LK";
const int Like = Parameter::Operator::Like;

} // end of namespace Btree::OpenOption::Ope

// ソートキーを選択するパラメータ
namespace SortKey
{

//
//	CONST
//	Btree::OpenOption::SortKey::Key --
//		ソートキーを選択するパラメータのキー
//
//	NOTES
//	Ｂ＋木ファイルに挿入されているオブジェクトの
//	ソートキーを選択するパラメータのキー。
//	このパラメータのバリューには、
//		・Btree::OpenOption::SortKey::ObjectID
//		・Btree::OpenOption::SortKey::KeyField
//	のいずれかを設定する。
//
//const char* const Key = "SortKey";
const int Key = Parameter::KeyNumber::SortKey;

//
//	CONST
//	Btree::OpenOption::SortKey::ObjectID --
//		オブジェクト ID をソートキーとするパラメータのバリュー
//
//	NOTES
//	オブジェクト ID をソートキーとするパラメータのバリュー。
//
//const char* const ObjectID = "ObjectID";
const int ObjectID = Parameter::SortKey::ObjectID;

//
//	CONST
//	Btree::OpenOption::SortKey::KeyField --
//		キーフィールドをソートキーとするパラメータのバリュー
//
//	NOTES
//	キーフィールドをソートキーとするパラメータのバリュー。
//
//const char* const KeyField = "KeyField";
const int KeyField = Parameter::SortKey::KeyField;

} // end of namespace Btree::OpenOption::SortKey

// ソート順を指定するパラメータ
namespace SortReverse
{

//
//	CONST
//	Btree::OpenOption::SortReverse::Key --
//		ソート順を指定するパラメータのキー
//
//	NOTES
//	オブジェクト取得時のソート順を指定する。
//	Btree::OpenOption::SortKey のバリューで指定されるソートキーにより、
//	意味合いが異なる。
//	Btree::OpenOption::SortKey のバリューに
//	Btree::OpenOption::SortKey::ObjectID が指定されている場合...
//		このパラメータのバリューに false を設定すると、
//		オブジェクトはオブジェクト ID の昇順で取得でき、
//		true を設定すると、降順で取得できる。
//	Btree::OpenOption::SortKey のバリューに
//	Btree::OpenOption::SortKey::KeyField が指定されている場合...
//		このパラメータのバリューに false を設定すると、
//		オブジェクトは挿入されているキー値順に取得でき、
//		true を設定すると、キー値順の逆順に取得できる。
//
//const char* const Key = "SortReverse";
const int Key = Parameter::KeyNumber::SortReverse;

} // end of namespace Btree::OpenOption::SortReverse

// ビットセット取得を指定するパラメータ
namespace GetByBitSet
{

//
//	CONST
//	Btree::OpenOption::GetByBitSet::Key --
//		ビットセット取得を指定するパラメータのキー
//
//	NOTES
//	オブジェクトを取得する際に、複数のオブジェクトを
//	ビットセットとして一括取得する場合には
//	このパラメータのバリューに true を設定する。
//
//const char* const Key = "GetByBitSet";
const int Key = LogicalFile::OpenOption::KeyNumber::GetByBitSet;

} // end of namespace Btree::OpenOption::GetByBitSet

// エスケープ文字を指定するパラメータ
namespace Escape
{

//
//	CONST
//	Btree::OpenOption::Escape::Key --
//		エスケープ文字を指定するパラメータのキー
//
//	NOTES
//	エスケープ文字を指定するパラメータのキー。
//
//const char*	const Key = "Escape";
const int Key = Parameter::KeyNumber::Escape;

} // end of namespace Btree::OpenOption::Escape

} // end of namespace Btree::OpenOption

} // end of namespace Btree

_SYDNEY_END

#endif // __SYDNEY_BTREE_OPENOPTION_H

//
//	Copyright (c) 2000,2001,2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
