// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Externalizable.cpp -- シリアル化可能なオブジェクト関連の関数定義
// 
// Copyright (c) 2000, 2004, 2005, 2007, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Schema/Area.h"
#include "Schema/AreaContent.h"
#include "Schema/ArrayFile.h"
#include "Schema/ArrayIndex.h"
#include "Schema/BitmapFile.h"
#include "Schema/BitmapIndex.h"
#include "Schema/BtreeFile.h"
#include "Schema/BtreeIndex.h"
#include "Schema/Column.h"
#include "Schema/Constraint.h"
#include "Schema/Database.h"
#include "Schema/Default.h"
#include "Schema/Externalizable.h"
#include "Schema/Field.h"
#include "Schema/FullTextFile.h"
#include "Schema/FullTextIndex.h"
#include "Schema/Hint.h"
#include "Schema/Index.h"
#include "Schema/KdTreeFile.h"
#include "Schema/KdTreeIndex.h"
#include "Schema/Key.h"
#include "Schema/LogData.h"
#include "Schema/Privilege.h"
#include "Schema/RecordFile.h"
#include "Schema/Table.h"
#include "Schema/VectorFile.h"

#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::Externalizable::getClassInstance --
//		指定された種別のシリアル化可能なオブジェクトを確保する
//
//	NOTES
//
//	ARGUMENTS
//		int					classID
//			確保するオブジェクトの種別を表す値
//
//	RETURN
//		確保したシリアル化可能なオブジェクトを格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
Common::Externalizable*
Externalizable::
getClassInstance(int classID)
{
	; _SYDNEY_ASSERT(classID >= Common::Externalizable::SchemaClasses);

	Externalizable*	object = 0;

	Externalizable::Category::Value	category =
		static_cast<Externalizable::Category::Value>(
			classID - Common::Externalizable::SchemaClasses);

	switch (category) {
	case Category::Database:
		object = new Database();		break;
	case Category::Table:
		object = new Table();			break;
	case Category::Column:
		object = new Column();			break;
	case Category::Constraint:
		object = new Constraint();		break;
	case Category::BtreeIndex:
		object = new BtreeIndex();		break;
	case Category::FullTextIndex:
		object = new FullTextIndex();	break;
	case Category::Key:
		object = new Key();				break;
	case Category::BtreeFile:
		object = new BtreeFile();		break;
	case Category::RecordFile:
		object = new RecordFile();		break;
	case Category::FullTextFile:
		object = new FullTextFile();	break;
	case Category::VectorFile:
		object = new VectorFile();		break;
	case Category::Field:
		object = new Field();			break;
	case Category::Area:
		object = new Area();			break;
	case Category::AreaContent:
		object = new AreaContent();		break;
	case Category::Default:
		object = new Default();			break;
	case Category::LogData:
		object = new LogData();			break;
	case Category::Hint:
		object = new Hint();			break;
	case Category::ColumnType:
		object = new Column::DataTypeOld();	break;
	case Category::BitmapFile:
		object = new BitmapFile();		break;
	case Category::BitmapIndex:
		object = new BitmapIndex();		break;
	case Category::ArrayFile:
		object = new ArrayFile();		break;
	case Category::ArrayIndex:
		object = new ArrayIndex();		break;
	case Category::Privilege:
		object = new Privilege();		break;
	case Category::KdTreeFile:
		object = new KdTreeFile();		break;
	case Category::KdTreeIndex:
		object = new KdTreeIndex();		break;
	default:
		; _SYDNEY_ASSERT(false);
	}

	return object;
}

//
// Copyright (c) 2000, 2004, 2005, 2007, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
