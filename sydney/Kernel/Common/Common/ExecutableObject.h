// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExecutableObject.h -- エグゼキュータが扱うオブジェクト
// 
// Copyright (c) 1999, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_EXECUTABLEOBJECT_H
#define __TRMEISTER_COMMON_EXECUTABLEOBJECT_H

#include "Common/Object.h"

_TRMEISTER_BEGIN

namespace Common
{

//
//	CLASS
//	Common::ExecutableObject -- エグゼキュータが扱うオブジェクト
//
//	NOTES
//	エグゼキュータが扱うオブジェクト共通の基底クラス。
//	プログラム等に格納するオブジェクトはすべてのこのクラスの
//	派生クラスでなければならない。
//
class SYD_COMMON_FUNCTION ExecutableObject : public Common::Object
{
public:
	//コンストラクタ
	ExecutableObject();
	//デストラクタ
	virtual ~ExecutableObject();

	//コピーコンストラクタ
	ExecutableObject(const ExecutableObject& cObject_);
	//代入演算子
	ExecutableObject& operator= (const ExecutableObject& cObject_);
	
	//文字列をプリントする
	virtual void print() const;

	//
	// !!!!!! 以下のメソッドはObjectPointerでしか呼んではいけない !!!!!
	//
	//参照を増やす
	virtual void incrementReferenceCounter();
	//参照を減らす
	virtual int decrementReferenceCounter();

protected:
	//参照カウンタ
	int m_iReference;
};

}

_TRMEISTER_END

#endif //__TRMEISTER_COMMON_EXECUTABLEOBJECT_H

//
//	Copyright (c) 1999, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
