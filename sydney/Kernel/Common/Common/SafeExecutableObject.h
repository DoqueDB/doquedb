// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SafeExecutableObject.h -- エグゼキュータが扱うオブジェクト(MT-safe版)
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

#ifndef __TRMEISTER_COMMON_SAFEEXECUTABLEOBJECT_H
#define __TRMEISTER_COMMON_SAFEEXECUTABLEOBJECT_H

#include "Common/ExecutableObject.h"
#include "Os/CriticalSection.h"

_TRMEISTER_BEGIN

namespace Common
{

//
//	CLASS
//	Common::SafeExecutableObject -- エグゼキュータが扱うオブジェクト(MT-safe版)
//
//	NOTES
//	エグゼキュータが扱うオブジェクト共通の基底クラス。
//	プログラム等に格納するオブジェクトはすべてのこのクラスの
//	派生クラスでなければならない。
//
class SYD_COMMON_FUNCTION SafeExecutableObject : public Common::ExecutableObject
{
public:
	//コンストラクタ
	SafeExecutableObject();
	//デストラクタ
	virtual ~SafeExecutableObject();

	//コピーコンストラクタ
	SafeExecutableObject(const SafeExecutableObject& cObject_);
	//代入演算子
	SafeExecutableObject& operator= (const SafeExecutableObject& cObject_);
	
	//
	// !!!!!! 以下のメソッドはObjectPointerでしか呼んではいけない !!!!!
	//
	//参照を増やす
	void incrementReferenceCounter();
	//参照を減らす
	int decrementReferenceCounter();

private:
	//排他制御用
	Os::CriticalSection m_cLatch;
};

}

_TRMEISTER_END

#endif //__TRMEISTER_COMMON_SAFEEXECUTABLEOBJECT_H

//
//	Copyright (c) 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
