// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Exernalizable.h -- シリアル化可能なオブジェクト関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2005, 2007, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_EXTERNALIZABLE_H
#define	__SYDNEY_SCHEMA_EXTERNALIZABLE_H

#include "Schema/Module.h"
#include "Common/Externalizable.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

//	CLASS
//	Schema::Externalizable -- シリアル化可能なオブジェクトを表すクラス
//
//	NOTES

class CommonExternalizable
	: public Common::Externalizable
{ };

class SYD_SCHEMA_FUNCTION Externalizable
	: public	CommonExternalizable
{
public:
	struct Category
	{
		//	ENUM
		//	Schema::Externalizable::Category::Value --
		//		シリアル化可能なオブジェクトの種別の値を表す列挙型
		//
		//	NOTES

		enum Value
		{
			Unknown =		0,					// 不明
			BtreeFile,							// Schema::BtreeFile
			Column,								// Schema::Column
			Constraint,							// Schema::Constraint
			Database,							// Schema::Database
			Default,							// Schema::Default
			Field,								// Schema::Field
			Index,								// Schema::Index
			Key,								// Schema::Key
			LogData,							// Schema::LogData
			Object,								// Schema::Object
			RecordFile,							// Schema::RecordFile
			Table,								// Schema::Table
			TempFile,							// Schema::TempFile
			FullTextFile,						// Schema::FullTextFile
			Area,								// Schema::Area
			AreaContent,						// Schema::AreaContent
#ifdef OBSOLETE
			ObjectID,							// Schema::ObjectID
#else
			Dummy_ObjectID,						// Schema::ObjectID
#endif
			Hint,								// Schema::Hint
			VectorFile,							// Schema::VectorFile
			File,								// Schema::File
			BtreeIndex,							// Schema::BtreeIndex
			FullTextIndex,						// Schema::FullTextIndex
			ColumnType,							// Schema::Column::Type
			LobFile,							// Schema::LobFile
			BitmapFile,							// Schema::BitmapFile
			BitmapIndex,						// Schema::BitmapIndex
			ArrayFile,							// Schema::ArrayFile
			ArrayIndex,							// Schema::ArrayIndex
			Privilege,							// Schema::Privilege
			KdTreeFile,							// Schema::KdTreeFile
			KdTreeIndex,						// Schema::KdTreeIndex
			ValueNum							// 種別数
		};
	};

	static Common::Externalizable* getClassInstance(int classID);
												// クラス ID からそれの表す
												// クラスを確保する
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_EXTERNALIZABLE_H

//
// Copyright (c) 2000, 2005, 2007, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
