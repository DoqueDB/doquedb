// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.h --	論理ファイルに関する定義
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

#ifndef __SYDNEY_SCHEMA_FILEID_H
#define __SYDNEY_SCHEMA_FILEID_H

#include "Schema/Module.h"

_SYDNEY_BEGIN

namespace LogicalFile
{
	class FileID;
	namespace Parameter
	{
		class Key;
	}
}

_SYDNEY_SCHEMA_BEGIN

namespace FileID
{

//	TYPEDEF
//	Schema::FileID::KeyType -- キーの型
//
//	NOTES

typedef LogicalFile::Parameter::Key KeyType;

//	TYPEDEF
//	Schema::FileID::ModeType -- 選択肢の型
//
//	NOTES

typedef int ModeType;

//	FUNCTION
//	Schema::FileID::setMode -- ファイルIDで選択肢から選ぶオプションをセットする
//
//	NOTES

void	setMode(LogicalFile::FileID& cOption_,
				const KeyType& cKey_, const ModeType& cValue_);
	
} // namespace FileID
_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif //__SYDNEY_SCHEMA_FILEID_H

//
//	Copyright (c) 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
