// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileOption.h -- Ｂ＋木ファイルオプションのヘッダーファイル
// 
// Copyright (c) 2000,2002, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE_FILEOPTION_H
#define __SYDNEY_BTREE_FILEOPTION_H

#include "FileCommon/FileOption.h"
#include "LogicalFile/FileID.h"

_SYDNEY_BEGIN

namespace Btree
{

namespace FileOption
{

//------------------------
//	FileID の型変更に関するマクロ
//------------------------

namespace Parameter {
	typedef	LogicalFile::Parameter::Key	KeyType;
	typedef	ModUnicodeString			ValueType;

	namespace KeyNumber {
		enum Value {
			KeyObjectPerNode = LogicalFile::FileID::DriverNumber::Btree,
			FieldSortOrder,
			ValueNum
		};
	}
}

#define _SYDNEY_BTREE_FILE_PARAMETER_KEY(key_)              Btree::FileOption::Parameter::KeyType((key_))
#define _SYDNEY_BTREE_FILE_PARAMETER_FORMAT_KEY(key_, arg_) Btree::FileOption::Parameter::KeyType((key_), (arg_))
#define _SYDNEY_BTREE_FILE_PARAMETER_VALUE(value_)          value_

// 1 ノードページあたりのキーオブジェクト数を指定するパラメータ
namespace KeyObjectPerNode
{

//
//	CONST
//	Btree::FileOption::KeyObjectPerNode::Key --
//		1 ノードページあたりのキーオブジェクト数を指定するパラメータのキー
//
//	NOTES
//	リーフページを含む 1 ノードページあたりのキーオブジェクト数を指定する
//	パラメータのキー。
//	このパラメータのバリューのデータ型は
//	Common::Parameter::TypeInteger で
//	2 以上の無符号 32 ビット整数値を設定する。
//	省略時には、デフォルト値 100 が設定され、「 100 次のＢ＋木」となる。
//
//const char* const Key = "KeyObjectPerNode";
const int Key = Parameter::KeyNumber::KeyObjectPerNode;

} // end of namespace KeyObjectPerNode

// キーフィールドのソート順を指定するパラメータ	
namespace FieldSortOrder
{

//
//	CONST
//	Btree::FileOption::FieldSortOrder::Key --
//		キーフィールドのソート順を指定するパラメータのキー
//
//	NOTES
//	キーフィールドのソート順を指定するパラメータのキー。
//	昇順の場合にはこのパラメータのバリューに false を、
//	降順の場合にはこのパラメータのバリューに true を設定する。
//	省略したキーフィールドは昇順指定となる。
//	インデックスはキーフィールドでのインデックスではなく、
//	オブジェクト内でのインデックスである。オブジェクト ID フィールド
//	およびバリューフィールドに対してこのパラメータを指定しても
//	Ｂ＋木ファイルは認識しない。
//
//const char* const Key = "FieldSortOrder[%d]";
const int Key = Parameter::KeyNumber::FieldSortOrder;

} // end of namespace FieldSortOrder


} // end of namespace FileOption

} // end of namespace Btree

_SYDNEY_END

#endif // __SYDNEY_BTREE_FILEOPTION_H

//
//	Copyright (c) 2000,2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
