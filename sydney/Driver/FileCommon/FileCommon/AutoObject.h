// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoObject.h -- 
// 
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FILECOMMON_AUTOOBJECT_H
#define __SYDNEY_FILECOMMON_AUTOOBJECT_H

#include "FileCommon/Module.h"

#ifndef PURIFY
#include "ModDefaultManager.h"
#endif

_SYDNEY_BEGIN
_SYDNEY_FILECOMMON_BEGIN

// 自動オブジェクトまたは他オブジェクトのメンバ
// としてのみ作成されるオブジェクト
class AutoObject
{
private://auto のみで作成させる為の処置(private 宣言のみ)
	void* operator new(size_t);
	void operator delete(void*);
	void* operator new[](size_t);
	void operator delete[](void*);
};

// 他オブジェクトのメンバまたは Vectorのエレメント
// としてのみ作成されるオブジェクト
// ∵ Vectorの内部で new[]/delete[] が実行される。
class ElementObject
#ifndef PURIFY
	: public ModDefaultObject
#endif//PURIFY
{
private://auto のみで作成させる為の処置(private 宣言のみ)
	void* operator new(size_t);
	//void operator delete(void*);	//リンクエラー対応（delete[]で使用？）
};

_SYDNEY_FILECOMMON_END
_SYDNEY_END

#endif // __SYDNEY_FILECOMMON_AUTOOBJECT_H

//
//	Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
