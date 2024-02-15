// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExecutableObject.cpp -- エグゼキュータが扱うオブジェクト
// 
// Copyright (c) 1999, 2001, 2002, 2023 Ricoh Company, Ltd.
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
#include "Common/ExecutableObject.h"
#include "Common/Message.h"

_TRMEISTER_USING

//
//	FUNCTION public
//	Common::ExecutableObject -- コンストラクタ
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
Common::ExecutableObject::ExecutableObject()
: m_iReference(0)
{
}

//
//	FUNCTION public
//	Common::ExecutableObject::~ExecutableObject -- デストラクタ
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
Common::ExecutableObject::~ExecutableObject()
{
}

//
//	FUNCTION public
//	Common::ExecutableObject::ExecutableObject -- コピーコンストラクタ
//
//	NOTES
//	コピーコンストラクタ
//
//	ARGUMENTS
//	const Common::ExecutableObject& cObject_
//		コピー元のオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Common::ExecutableObject::ExecutableObject(const ExecutableObject& cObject_)
: m_iReference(0)
{
}

//
//	FUNCTION public
//	Common::ExecutableObject::operator= -- 代入演算子
//
//	NOTES
//	代入演算子。
//
//	ARGUMENTS
//	const Common::ExecutableObject& cObject_
//		代入するオブジェクト
//
//	RETURN
//	Common::ExecutableObject&
//		自分自身の参照
//
//	EXCEPTIONS
//	なし
//
Common::ExecutableObject&
Common::ExecutableObject::operator= (const ExecutableObject& cObject_)
{
	//何にもしない
	return *this;
}

//
//	FUNCTION public
//	Common::ExecutableObject::incrementReferenceCounter -- 参照を増やす
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
Common::ExecutableObject::incrementReferenceCounter()
{
	++m_iReference;
}

//
//	FUNCTION public
//	Common::ExecuableObject::decrementReferenceCounter -- 参照を減らす
//
//	NOTES
//	参照を減らす
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
Common::ExecutableObject::decrementReferenceCounter()
{
	return --m_iReference;
}

//	FUNCTION public
//	Common::ExecutableObject::print -- 文字列をプリントする
//
//	NOTES
//	文字列をプリントする
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Common::ExecutableObject::print() const
{
#ifdef DEBUG
	SydDebugMessage << "Please overwrite print() method." << ModEndl;
#endif
}

//
//	Copyright (c) 1999, 2001, 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

