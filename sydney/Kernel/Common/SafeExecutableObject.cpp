// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SafeExecutableObject.cpp -- エグゼキュータが扱うオブジェクト
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

namespace {
const char moduleName[] = "Common";
const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Common/SafeExecutableObject.h"
#include "Common/Message.h"
#include "Os/AutoCriticalSection.h"

_TRMEISTER_USING

//
//	FUNCTION public
//	Common::SafeExecutableObject -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Common::SafeExecutableObject::SafeExecutableObject()
{
}

//
//	FUNCTION public
//	Common::SafeExecutableObject::~SafeExecutableObject -- デストラクタ
//
//	NOTES
//	デストラクタ
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Common::SafeExecutableObject::~SafeExecutableObject()
{
}

//
//	FUNCTION public
//	Common::SafeExecutableObject::SafeExecutableObject -- コピーコンストラクタ
//
//	NOTES
//	コピーコンストラクタ
//
//	ARGUMENTS
//	const Common::SafeExecutableObject& cObject_
//		コピー元のオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Common::SafeExecutableObject::SafeExecutableObject(const SafeExecutableObject& cObject_)
: ExecutableObject(cObject_)
{
}

//
//	FUNCTION public
//	Common::SafeExecutableObject::operator= -- 代入演算子
//
//	NOTES
//	代入演算子。
//
//	ARGUMENTS
//	const Common::SafeExecutableObject& cObject_
//		代入するオブジェクト
//
//	RETURN
//	Common::SafeExecutableObject&
//		自分自身の参照
//
//	EXCEPTIONS
//	なし
//
Common::SafeExecutableObject&
Common::SafeExecutableObject::operator= (const SafeExecutableObject& cObject_)
{
	ExecutableObject::operator= (cObject_);
	return *this;
}

//
//	FUNCTION public
//	Common::SafeExecutableObject::incrementReferenceCounter -- 参照を増やす
//
//	NOTES
//	参照を増やす。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Common::SafeExecutableObject::incrementReferenceCounter()
{
	Os::AutoCriticalSection lock(m_cLatch);
	ExecutableObject::incrementReferenceCounter();
}

//
//	FUNCTION public
//	Common::ExecuableObject::decrementReferenceCounter -- 参照を減らす
//
//	NOTES
//	参照を減らす。
//	publicであるが、ObjectPointer以外でこのメソッドを呼んではいけない。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
int
Common::SafeExecutableObject::decrementReferenceCounter()
{
	Os::AutoCriticalSection lock(m_cLatch);
	return ExecutableObject::decrementReferenceCounter();
}

//
//	Copyright (c) 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

