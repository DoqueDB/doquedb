// -*-Mode: C++; tab-width: 4; c-basic-offmap: 4;-*-
// vi:map ts=4 sw=4:
//
// FileMap.h -- スキーマオブジェクトのマップを表すクラス定義、関数宣言
// 
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_FILE_MAP_H
#define	__SYDNEY_SCHEMA_FILE_MAP_H

#include "Schema/ObjectMap.h"
#include "Schema/File.h"
#include "Schema/AreaCategory.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

//	CLASS public
//	Schema::FileMap --
//
//	NOTES

class FileMap
	: public ObjectMap<File, FilePointer>
{
public:
	FileMap();

	// FileMapからオブジェクトを得るのに使用する比較関数
	static bool findByAreaCategory(File* pFile_, AreaCategory::Value eCategory_);
};

//	FUNCTION public
//	Schema::FileMap::findByAreaCategory -- エリアカテゴリーでオブジェクトを探すための比較関数
//
//	NOTES
//
//	ARGUMENTS
//		Schema::File* pFile_
//			比較対象のオブジェクト
//		Schema::AreaCategory::Value eCategory_
//			条件となる値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
inline bool
FileMap::
findByAreaCategory(File* pFile_, AreaCategory::Value eCategory_)
{
	return (pFile_->getAreaCategory() == eCategory_);
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif // __SYDNEY_SCHEMA_FILE_MAP_H

//
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
