// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MergeReserve.h --
// 
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT_MERGERESERVE_H
#define __SYDNEY_FULLTEXT_MERGERESERVE_H

#include "FullText/Module.h"

#include "Lock/Name.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT_BEGIN

//
//	CLASS
//	FullText::MergeReserve -- マージ処理へのエントリを管理する
//
//	NOTES
//
class MergeReserve
{
public:
	// 末尾にエントリを追加する
	static bool pushBack(const Lock::FileName& cFileName_);
	// 先頭のエントリを得る
	static bool getFront(Lock::FileName& cFileName_,
						 unsigned int msec_);
	// 先頭のエントリを削除する
	static void popFront();
};

_SYDNEY_FULLTEXT_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT_MERGERESERVE_H

//
//	Copyright (c) 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
