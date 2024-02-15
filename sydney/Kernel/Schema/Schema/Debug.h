// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Debug.h -- スキーマモジュールでデバッグに関する定義
// 
// Copyright (c) 2001, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_DEBUG_H
#define	__SYDNEY_SCHEMA_DEBUG_H

#ifdef DEBUG

#include "Schema/Module.h"
#include "Schema/Message.h"
#include "Schema/Object.h"
#include "Schema/Area.h"
#include "Schema/Column.h"
#include "Schema/Constraint.h"
#include "Schema/Database.h"
#include "Schema/Default.h"
#include "Schema/Externalizable.h"
#include "Schema/Field.h"
#include "Schema/Index.h"
#include "Schema/File.h"

#include "ModMessage.h"

// enum型に対するメッセージ出力関数の宣言
ModMessageStream& operator<<(ModMessageStream& cStream_,
							 _SYDNEY::Schema::Object::Category::Value eValue_);
ModOstream& operator<<(ModOstream& cStream_,
					   _SYDNEY::Schema::Object::Category::Value eValue_);
ModMessageStream& operator<<(ModMessageStream& cStream_,
							 _SYDNEY::Schema::AreaCategory::Value eValue_);
ModOstream& operator<<(ModOstream& cStream_,
					   _SYDNEY::Schema::AreaCategory::Value eValue_);
ModMessageStream& operator<<(ModMessageStream& cStream_,
							 _SYDNEY::Schema::Database::Path::Category::Value eValue_);
ModOstream& operator<<(ModOstream& cStream_,
					   _SYDNEY::Schema::Database::Path::Category::Value eValue_);
ModMessageStream& operator<<(ModMessageStream& cStream_,
							 _SYDNEY::Schema::Column::Category::Value eValue_);
ModOstream& operator<<(ModOstream& cStream_,
					   _SYDNEY::Schema::Column::Category::Value eValue_);
ModMessageStream& operator<<(ModMessageStream& cStream_,
							 _SYDNEY::Schema::Constraint::Category::Value eValue_);
ModOstream& operator<<(ModOstream& cStream_,
					   _SYDNEY::Schema::Constraint::Category::Value eValue_);
ModMessageStream& operator<<(ModMessageStream& cStream_,
							 _SYDNEY::Schema::Default::Category::Value eValue_);
ModOstream& operator<<(ModOstream& cStream_,
					   _SYDNEY::Schema::Default::Category::Value eValue_);
ModMessageStream& operator<<(ModMessageStream& cStream_,
							 _SYDNEY::Schema::Externalizable::Category::Value eValue_);
ModOstream& operator<<(ModOstream& cStream_,
					   _SYDNEY::Schema::Externalizable::Category::Value eValue_);
ModMessageStream& operator<<(ModMessageStream& cStream_,
							 _SYDNEY::Schema::Field::Category::Value eValue_);
ModOstream& operator<<(ModOstream& cStream_,
					   _SYDNEY::Schema::Field::Category::Value eValue_);
ModMessageStream& operator<<(ModMessageStream& cStream_,
							 _SYDNEY::Schema::Index::Category::Value eValue_);
ModOstream& operator<<(ModOstream& cStream_,
					   _SYDNEY::Schema::Index::Category::Value eValue_);
ModMessageStream& operator<<(ModMessageStream& cStream_,
							 _SYDNEY::Schema::File::Category::Value eValue_);
ModOstream& operator<<(ModOstream& cStream_,
					   _SYDNEY::Schema::File::Category::Value eValue_);

#endif

#endif	// __SYDNEY_SCHEMA_DEBUG_H

//
// Copyright (c) 2001, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
