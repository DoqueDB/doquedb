// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileOption.h -- 論理ファイル共通オプションのヘッダーファイル
// 
// Copyright (c) 2000, 2002, 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FILECOMMON_FILEOPTION_H
#define __SYDNEY_FILECOMMON_FILEOPTION_H

#include "FileCommon/Module.h"
#include "LogicalFile/FileID.h"
#include "Os/Memory.h"

_SYDNEY_BEGIN

namespace FileCommon
{

namespace FileOption
{
//------------------------
//	FileID の型変更に関するマクロ
//------------------------

namespace Parameter {
	typedef	LogicalFile::Parameter::Key		KeyType;
	typedef	ModUnicodeString				ValueType;
}

#define _SYDNEY_FILE_PARAMETER_KEY(key_)              FileCommon::FileOption::Parameter::KeyType((key_), 0)
#define _SYDNEY_FILE_PARAMETER_FORMAT_KEY(key_, arg_) FileCommon::FileOption::Parameter::KeyType((key_), (arg_))
#define _SYDNEY_FILE_PARAMETER_VALUE(value_)          (value_)

//------------------------------------
//	物理ファイルの生成に関するオプション
//------------------------------------

namespace Mounted
{

//
//	CONST
//	FileCommon::FileOption::Mounted::Key --
//		論理ファイルがマウントされているかどうかを指定するパラメータのキー
//
//	NOTES
//	論理ファイルがマウントされているかどうかを指定するパラメータのキー。
//	マウントされているのであれば true を指定し、
//	マウントされていないのであれば false を指定する。
//
//const char*	const Key = "Mounted";
const int Key = LogicalFile::FileID::KeyNumber::Mounted;

} // end of namespace Mounted

namespace Area
{

//
//	CONST
//	FileCommon::FileOption::Area::Key --
//		論理ファイル格納先ディレクトリパスを指定するパラメータのキー
//
//	NOTES
//	論理ファイル格納先ディレクトリパスを指定するパラメータのキー。
//	ファイル生成時には、このパラメータのバリューで指定される
//	ディレクトリ内に物理ファイルが生成される。
//	このパラメータのバリューには文字列を設定する。
//
//const char*	const Key = "Area[%d]";
const int Key = LogicalFile::FileID::KeyNumber::Area;

} // end of namespace Area

namespace Temporary
{

//
//	CONST
//	FileCommon::FileOption::Temporary::Key --
//		一時ファイル作成を指定するパラメータのキー
//
//	NOTES
//	一時ファイルとして作成するかどうかを指定するパラメータのキー。
//	一時ファイルを作成する場合には true を指定する。
//	このパラメータのバリューには bool を設定する。
//
//const char*	const Key = "Temporary";
const int Key = LogicalFile::FileID::KeyNumber::Temporary;

} // end of namespace Temporary

namespace PageSize
{

//
//	CONST
//	FileCommon::FileOption::PageSize::Key --
//		物理ページサイズを指定するパラメータのキー
//
//	NOTES
//	論理ファイルがもつ物理ファイル内の物理ページサイズを指定する
//	パラメータのキー。
//	このパラメータのバリューには整数値を設定する。
//	単位は Kbyte で、2 のべき乗の値を指定しなければならない。
//
//const char*	const Key = "PageSize";
const int Key = LogicalFile::FileID::KeyNumber::PageSize;

//
//	FUNCTION
//	FileCommon::FileOption::PageSize::getDefault --
//		デフォルトのページサイズを得る
//
//	NOTES
//	パラメータ FileCommon_DefaultPageSize で指定される
//	デフォルトのページサイズを得る(単位はbyte)
//	パラメータが指定されていない場合は、Os::SysConf::PageSize::get()の値を返す
//
SYD_FILECOMMON_FUNCTION
Os::Memory::Size getDefault();

} // end of namespace PageSize


//----------------------------
//	フィールドに関するオプション
//----------------------------

namespace FieldNumber
{

//
//	CONST
//	FileCommon::FileOption::FieldNumber::Key --
//		フィールド数を指定するパラメータのキー
//
//	NOTES
//	ファイルへ挿入するオブジェクトを構成するフィールド数を
//	指定するパラメータのキー。オブジェクト ID フィールドの分も含まれる。
//	このパラメータのバリューには整数値を設定する。
//
//const char*	const Key = "FieldNumber";
const int Key = LogicalFile::FileID::KeyNumber::FieldNumber;

} // end of namespace FieldNumber

namespace FieldType
{

//
//	CONST
//	FileCommon::FileOption::FieldType::Key --
//		フィールドのデータ型を指定するパラメータのキー
//
//	NOTES
//	フィールドのデータ型を指定するパラメータのキー。
//	このパラメータのバリューには Common::DataType::Type のいずれかを設定する。
//
//const char*	const Key = "FieldType[%d]";
const int Key = LogicalFile::FileID::KeyNumber::FieldType;

} // end of namespace FieldType

namespace FieldLength
{

//
//	CONST
//	FileCommon::FileOption::FieldLength::Key --
//		可変長フィールドの最大長を指定するパラメータのキー
//
//	NOTES
//	文字列型フィールドなどの可変長フィールドの最大長、
//	または配列フィールドの要素数の最大値を指定する
//	パラメータのキー。
//	現在、可変長フィールドは、
//		・ Common::String 型フィールド
//		・ Common::Binary 型フィールド
//	がある。
//	これら以外のフィールドについては、このパラメータを設定する必要はない。
//	このパラメータのバリューには整数値を設定する。
//	String 型フィールドの場合	バイト数
//								文字数->バイト数変換は呼び出し側で行うこと
//	Binary 型フィールドの場合	バイト数
//	配列型フィールドの場合		配列要素の個数
//
//const char*	const Key = "FieldLength[%d]";
const int Key = LogicalFile::FileID::KeyNumber::FieldLength;

} // end of namespace FieldLength

namespace FieldFraction
{

//
//	CONST
//	FileCommon::FileOption::FieldFraction::Key --
//		小数点以下の桁数を指定するパラメータのキー
//
//	NOTES
//	Common::Decimal 型フィールドの小数点以下の桁数を指定するパラメータのキー。
//	このパラメータのバリューには整数値を設定する。
//
//const char*	const Key = "FieldFraction[%d]";
const int Key = LogicalFile::FileID::KeyNumber::FieldFraction;

} // end of namespace FieldFraction

namespace FieldEncodingForm
{
	//	CONST
	//	FileCommon::FileOption::FieldEncodingForm::Key --
	//		フィールドの符号化形式を指定するパラメータのキー
	//
	//	NOTES
	//		このパラメータのバリューには整数値を設定する

	const int Key = LogicalFile::FileID::KeyNumber::FieldEncodingForm;
}

namespace FieldFixed
{
	//
	//	CONST
	//	FileCommon::FileOption::FieldFixed::Key --
	//		フィールドが固定長か否かを指定するパラメータのキー
	//
	//	NOTES
	//	このパラメータのバリューには bool を設定する。
	//

	const int Key = LogicalFile::FileID::KeyNumber::FieldFixed;

} // end of namespace FieldFixed

namespace FieldCollation
{
	//
	//	CONST
	//	FileCommon::FileOption::FieldCollation::Key --
	//		The key of the collation in the field
	//
	//	NOTES
	//	Set Integer to this parameter.
	//

	const int Key = LogicalFile::FileID::KeyNumber::FieldCollation;

} // end of namespace FieldCollation

namespace ElementType
{

//
//	CONST
//	FileCommon::FileOption::ElementType::Key --
//		配列フィールドの要素のデータ型を指定するパラメータのキー
//
//	NOTES
//	配列フィールドの要素のデータ型を指定するパラメータのキー。
//	このパラメータのバリューには Common::DataType::Type のいずれかを設定する。
//	%d で指定されたフィールドの FieldType[%d] は Common::DataType::Array で
//	なければならない。
//
//const char*	const Key = "ElementType[%d]";
const int Key = LogicalFile::FileID::KeyNumber::ElementType;

} // end of namespace ElementType

namespace ElementLength
{

//
//	CONST
//	FileCommon::FileOption::ElementLength::Key --
//		配列フィールドの要素の最大長を指定するパラメータのキー
//
//	NOTES
//	配列フィールドの要素の型が文字列型などの可変長の場合に最大長を指定する
//	パラメータのキー。
//	現在、可変長フィールドは、
//		・ Common::String 型フィールド
//		・ Common::Binary 型フィールド
//	がある。
//	これら以外のフィールドについては、このパラメータを設定する必要はない。
//	このパラメータのバリューには整数値を設定する。
//	String 型フィールドの場合	文字数 (ModUnicodeChar)
//	Binary 型フィールドの場合	バイト数
//
//const char*	const Key = "ElementLength[%d]";
const int Key = LogicalFile::FileID::KeyNumber::ElementLength;

} // end of namespace ElementLength

namespace ElementEncodingForm
{
	//	CONST
	//	FileCommon::FileOption::ElementEncodingForm::Key --
	//		配列フィールドの要素の符号化形式を指定するパラメータのキー
	//
	//	NOTES
	//		このパラメータのバリューには整数値を設定する

	const int Key = LogicalFile::FileID::KeyNumber::ElementEncodingForm;
}

namespace ElementFixed
{
	//
	//	CONST
	//	FileCommon::FileOption::ElementFixed::Key --
	//		配列フィールドの要素が固定長か否かを指定するパラメータのキー
	//
	//	NOTES
	//	このパラメータのバリューには bool を設定する。
	//

	const int Key = LogicalFile::FileID::KeyNumber::ElementFixed;

} // end of namespace ElementFixed

namespace ReadOnly
{

//	CONST
//	FileCommon::FileOption::ReadOnly::Key --
//		ファイルの書き込み属性を指定するパラメータのキー
//
//	NOTES
//	ファイルの書き込み属性を指定するパラメータのキー。
//	このパラメータの値にはboolを設定する
//	(trueはReadOnly, falseはReadWriteとする)。
//
//const char*	const Key = "ReadOnly";
const int Key = LogicalFile::FileID::KeyNumber::ReadOnly;

} // end of namespace ReadOnly

//------------------------
//	ヒントに関するオプション
//------------------------

namespace AreaHint
{
//const char* const Key = "AreaHint";
const int Key = LogicalFile::FileID::KeyNumber::AreaHint;
}

namespace FileHint
{
//const char* const Key = "FileHint";
const int Key = LogicalFile::FileID::KeyNumber::FileHint;
}

namespace FieldHint
{
//const char* const Key = "FieldHint[%d]";
const int Key = LogicalFile::FileID::KeyNumber::FieldHint;
}


//------------------------
//	ファイルのロック名取得に関するオプション
//------------------------
namespace DatabaseID
{

//
//	CONST
//	FileCommon::FileOption::DatabaseID::Key --
//		スキーマのデータベースID を示すパラメータのキー
//
//	NOTES
//	このパラメータのバリューには整数値を設定する。
//
//const char*	const Key = "SchemaDatabaseID";
const int Key = LogicalFile::FileID::KeyNumber::SchemaDatabaseID;

} // end of namespace DatabaseID

namespace TableID
{

//
//	CONST
//	FileCommon::FileOption::TableID::Key --
//		スキーマの索引 ID を示すパラメータのキー
//
//	NOTES
//	このパラメータのバリューには整数値を設定する。
//
//const char*	const Key = "SchemaTableID";
const int Key = LogicalFile::FileID::KeyNumber::SchemaTableID;

} // end of namespace TableID

namespace FileObjectID
{

//
//	CONST
//	FileCommon::FileOption::FileObjectID::Key --
//		スキーマのオブジェクトID を示すパラメータのキー
//
//	NOTES
//	このパラメータのバリューには整数値を設定する。
//
//const char*	const Key = "SchemaFileObjectID";
const int Key = LogicalFile::FileID::KeyNumber::SchemaFileObjectID;

} // end of namespace FileObjectID

namespace KeyFieldNumber
{

//
//	CONST
//	FileCommon::FileOption::KeyFieldNumber::Key --
//		キーフィールドの数を示すパラメータのキー
//
//	NOTES
//	このパラメータのバリューには整数値を設定する。
//	論理ファイルへ挿入するオブジェクトを構成するフィールドのうちの
//	キーフィールド数を指定するパラメータのキー。
//	このパラメータのバリューのデータ型は
//	Common::Parameter::TypeInteger で符号付き 32 ビット整数値を設定する。
//	省略不可。
//	※ 「キーフィールド数」には「オブジェクト ID フィールド」分は含まない。
//
const int Key = LogicalFile::FileID::KeyNumber::KeyFieldNumber;

} // end of namespace KeyFieldNumber

namespace VirtualFieldNumber
{

//
//	CONST
//	FileCommon::FileOption::VirtualFieldNumber::Key --
//		仮想フィールドの数を示すパラメータのキー
//
//	NOTES
//	このパラメータのバリューには整数値を設定する。
//	論理ファイルから取得するオブジェクトを構成するフィールドのうちの
//	仮想フィールド数を指定するパラメータのキー。
//	このパラメータのバリューのデータ型は
//	Common::Parameter::TypeInteger で符号付き 32 ビット整数値を設定する。
//	省略可。
//
const int Key = LogicalFile::FileID::KeyNumber::VirtualFieldNumber;

} // end of namespace VirtualFieldNumber

namespace Unique
{

//
//	CONST
//	FileCommon::FileOption::Unique::Key --
//		ユニーク指定用パラメータのキー
//
//	NOTES
//	このパラメータのバリューには整数値を設定する。
//	Unique 属性をもたせる場合、
//		・オブジェクト全体を Unique とするか
//		・キーフィールドを Unique とするか
//	いずれかを指定する。
//	このパラメータのバリューには、
//		・FileCommon::FileOption::Unique::Object
//		・FileCommon::FileOption::Unique::KeyField
//	いずれかの文字列を指定する。
//	省略時には、ユニーク指定がされていないと判断する。
//
const int Key = LogicalFile::FileID::KeyNumber::Unique;

//
//	CONST
//	FileCommon::FileOption::Unique::Object --
//		オブジェクト全体を Unique とするユニーク指定用パラメータのバリュー
//
const int Object = LogicalFile::FileID::Unique::Object;

//
//	CONST
//	FileCommon::FileOption::Unique::KeyField --
//		キーフィールドを Unique とするユニーク指定用パラメータのバリュー
//
const int KeyField = LogicalFile::FileID::Unique::KeyField;

} // end of namespace Unique

namespace Version
{

//
//	CONST
//	FileCommon::FileOption::Version::Key --
//		ドライバーのバージョンを示すパラメータのキー
//
//	NOTES
//	このパラメータのバリューには整数値を設定する。
//	ドライバのバージョンを1から	
//	Common::Parameter::TypeInteger で符号付き 32 ビット整数値を設定する。
//	create後にファイルドライバーが設定する。
//
const int Key = LogicalFile::FileID::KeyNumber::Version;

} // end of namespace Version

} // end of namespace FileOption

} // end of namespace FileCommon

_SYDNEY_END

#endif // __SYDNEY_FILECOMMON_FILEOPTION_H

//
//	Copyright (c) 2000, 2002, 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
