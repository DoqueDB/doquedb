// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Meta.h -- システム表関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2005, 2007, 2012, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_META_H
#define	__SYDNEY_SCHEMA_META_H

#include "Schema/Module.h"

#include "Common/DataType.h"

#include "ModUnicodeString.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

namespace Meta
{
	struct MemberType
	{
		enum Value {
			Unknown = 0,
			FileOID,					// ファイル上のOID
			ObjectID,					// ObjectのID
			ParentID,					// 親オブジェクトのID
			Name,						// 名称
			Timestamp,					// タイムスタンプ
			UseObjectMax,				// ここまではObjectの情報を使う

			ID = UseObjectMax,			// その他のID
			IDArray,					// ID配列
			Integer,					// int
			UnsignedInteger,			// unsigned int
			String,						// ModUnicodeString
			StringArray,				// ModVector<ModUnicodeString>
			Binary,						// Binaryにserializeして格納する
			BigInt,						// BigInt
			UnsignedIntegerArray,		// ModVector<unsigned int>
			ValueNum
		};
	};

	// Meta::MemberTypeから対応するCommon::DataTypeを得る
	Common::DataType::Type getFieldType(MemberType::Value eType_);
	Common::DataType::Type getFieldElementType(MemberType::Value eType_);

	// 定義に使うテンプレート構造体
	template <class _Object_>
	struct Definition {

		typedef int (_Object_::* GetInteger)() const;
		typedef unsigned int (_Object_::* GetUnsignedInteger)() const;
		typedef const ModVector<unsigned int>& (_Object_::* GetUnsignedIntegers)() const;
		typedef const ModUnicodeString& (_Object_::* GetString)() const;
		typedef const ModVector<ModUnicodeString>& (_Object_::* GetStrings)() const;
		typedef void (_Object_::* SetInteger)(int);
		typedef void (_Object_::* SetUnsignedInteger)(unsigned int);
		typedef void (_Object_::* SetUnsignedIntegers)(const ModVector<unsigned int>&);
		typedef void (_Object_::* SetString)(const ModUnicodeString&);
		typedef void (_Object_::* SetStrings)(const ModVector<ModUnicodeString>&);

		typedef GetUnsignedInteger GetID;
		typedef SetUnsignedInteger SetID;
		typedef GetUnsignedIntegers GetIDs;
		typedef SetUnsignedIntegers SetIDs;

		union Get {
			void* _pointer;
			GetID _id;
			GetIDs _ids;
			GetInteger _integer;
			GetUnsignedInteger _unsignedInteger;
			GetUnsignedIntegers _unsignedIntegers;
			GetString _string;
			GetStrings _strings;
		};
		union Set {
			void* _pointer;
			SetID _id;
			SetIDs _ids;
			SetInteger _integer;
			SetUnsignedInteger _unsignedInteger;
			SetUnsignedIntegers _unsignedIntegers;
			SetString _string;
			SetStrings _strings;
		};

		Meta::MemberType::Value m_eType;
		Get m_funcGet;
		Set m_funcSet;

		// コンストラクター
		Definition(Meta::MemberType::Value eType_)
			: m_eType(eType_)
		{m_funcGet._pointer = 0, m_funcSet._pointer = 0;}
		Definition(Meta::MemberType::Value eType_, GetInteger get_, SetInteger set_)
			: m_eType(eType_)
		{m_funcGet._integer = get_, m_funcSet._integer = set_;}
		Definition(Meta::MemberType::Value eType_, GetUnsignedInteger get_, SetUnsignedInteger set_)
			: m_eType(eType_)
		{m_funcGet._unsignedInteger = get_, m_funcSet._unsignedInteger = set_;}
		Definition(Meta::MemberType::Value eType_, GetUnsignedIntegers get_, SetUnsignedIntegers set_)
			: m_eType(eType_)
		{m_funcGet._ids = get_, m_funcSet._ids = set_;}
		Definition(Meta::MemberType::Value eType_, GetString get_, SetString set_)
			: m_eType(eType_)
		{m_funcGet._string = get_, m_funcSet._string = set_;}
		Definition(Meta::MemberType::Value eType_, GetStrings get_, SetStrings set_)
			: m_eType(eType_)
		{m_funcGet._strings = get_, m_funcSet._strings = set_;}
	};

	////////////////////////////////////////
	// MemberIDの定義
	////////////////////////////////////////

	namespace Database
	{
		// メタデータベースにおける「データベース」表の構造は以下のとおり
		// create table Database (
		//		ID			id,
		//		name		nvarchar,
		//		path		<string array>, -- パス名の配列
		//		time		timestamp,
		//		unsigned int				-- フラグ
		//			0:bool	ReadOnly,		-- 読取属性
		//			1:bool	Online			-- オンライン属性
		//		master_url	string,
		// )
		enum MemberID {
			FileOID = 0,
			ID,
			Name,
			Path,
			Timestamp,
			Flag,
			MemberNum,
			MasterURL, // only for system_database
		};
	} // namespace Database

	namespace Area
	{
		// メタデータベースにおける「エリア」表の構造は以下のとおり
		// create table Area (
		//		ID		id,
		//		name	nvarchar,
		//		path	<path array>, -- StringData or DataArrayData
		//		time	timestamp
		// )
		enum MemberID {
			FileOID = 0,
			ID,
			Name,
			Path,
			Timestamp,
			MemberNum
		};
	} // namespace Area

	namespace AreaContent
	{
		// メタデータベースにおける「エリア格納関係」表の構造は以下のとおり
		// create table AreaContent (
		//		ID			id,
		//		area		id,
		//		objID		id,
		//		objCategory int,	-- Category::Value
		//		areaCategory int,	-- AreaCategory::Value
		//		time	timestamp
		// )
		enum MemberID {
			FileOID = 0,
			ID,
			AreaID,
			ObjectID,
			ObjectCategory,
			AreaCategory,
			Timestamp,
			MemberNum
		};
	} // namespace AreaContent

	namespace Column
	{
		// メタデータベースにおける「列」表の構造は以下のとおり
		// create table Column_DBXXXX (
		//		ID			id,
		//		parent		id,
		//		name		nvarchar,
		//		category	int,
		//		position	int,
		//		field		id,
		//		default		<data>, -- Common::Data
		//		flag		int,
		//		type		<type>, -- Statement::DataType
		//		precision	int,
		//		scale		int,
		//		characterSet	int,
		//		hint		<data>, -- Common::Data
		//		time		timestamp
		// )
		enum MemberID {
			FileOID = 0,
			ID,
			ParentID,
			Name,
			Category,
			Position,
			FieldID,
			Default,
			Flag,
			Type,
			Precision,
			Scale,
			CharacterSet,
			Hint,
			Timestamp,
			MemberNum,
			// 以下は関数列のID(通常は使われない)
			MetaData
		};
	} // namespace Column

	namespace Constraint
	{
		// メタデータベースにおける「制約」表の構造は以下のとおり
		// create table Constraint_DBXXXX (
		//		ID			id,
		//		parent		id,
		//		name		nvarchar,
		//		category	int,
		//		position	int,
		//		columnIDs	<binary>,
		//		time		timestamp
		// )
		enum MemberID {
			FileOID = 0,
			ID,
			ParentID,
			Name,
			Category,
			Position,
			ColumnIDs,
			Timestamp,
			MemberNum
		};
	} // namespace Constraint

	namespace Field
	{
		// メタデータベースにおける「フィールド」表の構造は以下のとおり
		// create table Field_DBXXXX (
		//		ID			id,
		//		parent		id,
		//		category	int,
		//		function	int,
		//		position	int,
		//		permission	int,
		//		source		id,
		//		column		id,
		//		key			id,
		//		type		int,
		//		elementType	int,
		//		length		int,
		//		elementLength	int,
		//		default		<binary>,
		//		time		timestamp
		// )
		enum MemberID {
			FileOID = 0,
			ID,
			ParentID,
			Category,
			Function,
			Position,
			Permission,
			SourceID,
			ColumnID,
			KeyID,
			Type,
			ElementType,
			Length,
			ElementLength,
			Default,
			Timestamp,
			MemberNum
		};
	} // namespace Field

	namespace File
	{
		// メタデータベースにおける「ファイル」表の構造は以下のとおり
		// create table File_DBXXXX (
		//		ID			id,
		//		parent		id,
		//		name		nvarchar,
		//		category	int,
		//		index		id,
		//		FileID		nvarchar,
		//		hint		nvarchar,
		//		area		{id, id},
		//		areaHint	nvarchar,
		//		option		<any>,		 -- サブクラスの付加情報
		//		time		timestamp
		// )
		enum MemberID {
			FileOID = 0,
			ID,
			ParentID,
			Name,
			Category,
			IndexID,
			FileID,
			Hint,
			AreaIDs,
			AreaHint,
			Option,
			Timestamp,
			MemberNum,
			// 以下は関数列のID(通常は使われない)
			FileSize
		};
	} // namespace File

	namespace Index
	{
		// メタデータベースにおける「索引」表の構造は以下のとおり
		// create table Index_DBXXXX (
		//		ID			id,
		//		parent		id,
		//		name		nvarchar,
		//		category	int,
		//		flag		int,
		//		file		id,
		//		hint		nvarchar,
		//		area		<id array>, -- エリアIDの配列
		//		areaHint	nvarchar,
		//		time		timestamp
		// )
		enum MemberID {
			FileOID = 0,
			ID,
			ParentID,
			Name,
			Category,
			Flag,
			FileID,
			Hint,
			AreaIDs,
			AreaHint,
			Timestamp,
			MemberNum,
			// 以下は関数列のID(通常は使われない)
			HintString
		};
	} // namespace Index

	namespace Key
	{
		// メタデータベースにおける「キー」表の構造は以下のとおり
		// create table Key_DBXXXX (
		//		ID			id,
		//		parent		id,
		//		position	int,
		//		order		int,
		//		column		id,
		//		field		id,
		//		time		timestamp
		// )
		enum MemberID {
			FileOID,
			ID,
			ParentID,
			Position,
			Order,
			ColumnID,
			FieldID,
			Timestamp,
			MemberNum
		};
	} // namespace Key

	namespace Privilege
	{
		// System table schema for privilege is as follows;
		// create table Privilege_DBXXXX (
		//		ID		id,
		//		userID	int,
		//		value	<unsigned int array>, -- UnsignedIntegerArrayData
		//		type	int,
		//		objectID <id array>,
		//		time	timestamp
		// )
		enum MemberID {
			FileOID,
			ID,
			UserID,
			Flags,
			Type,
			ObjectIDs,
			Timestamp,
			MemberNum
		};
	} // namespace Privilege

	namespace Table
	{
		// メタデータベースにおける「表」表の構造は以下のとおり
		// create table Table_DBXXXX (
		//		ID			id,
		//		name		nvarchar,
		//		type		int,
		//		area		<id array>, -- エリアIDの配列
		//		hint		nvarchar,
		//		time		timestamp
		// )
		enum MemberID {
			FileOID,
			ID,
			Name,
			Type,
			AreaIDs,
			Hint,
			Timestamp,
			MemberNum
		};
	} // namespace Table

	namespace Cascade
	{
		// メタデータベースにおける「子サーバー」表の構造は以下のとおり
		// create table Cascade_DBXXXX (
		//		ID			id,
		//		name		nvarchar,
		//		version		int,
		//		target		string[],
		//		time		timestamp
		// )
		enum MemberID {
			FileOID,
			ID,
			Name,
			Version,
			Target,
			Timestamp,
			MemberNum
		};
	} // namespace Cascade

	namespace Partition
	{
		// メタデータベースにおける「ルール」表の構造は以下のとおり
		// create table Partition_DBXXXX (
		//		ID			id,
		//		name		nvarchar,
		//		tableID		int,
		//		category	int,
		//		function	string,
		//		columnIDs	id[],
		//		time		timestamp
		// )
		enum MemberID {
			FileOID,
			ID,
			Name,
			TableID,
			Category,
			FunctionName,
			ColumnIDs,
			Timestamp,
			MemberNum,
		};
	} // namespace Cascade

	namespace Function
	{
		// メタデータベースにおける「関数」表の構造は以下のとおり
		// create table Function_DBXXXX (
		//		ID			id,
		//		name		nvarchar,
		//		version		int,
		//		returntype	binary,
		//		routine		string,
		//		time		timestamp
		// )
		enum MemberID {
			FileOID,
			ID,
			Name,
			Version,
			ReturnType,
			Routine,
			Timestamp,
			MemberNum
		};
	} // namespace Function

} // namespace Meta

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif // __SYDNEY_SCHEMA_META_H

//
// Copyright (c) 2001, 2005, 2007, 2012, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
