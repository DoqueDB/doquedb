// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ObjectIterator.cpp -- オブジェクト反復子の実装ファイル
// 
// Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Vector";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/Assert.h"
#include "Exception/IllegalFileAccess.h"
#include "FileCommon/VectorKey.h"
#include "Vector/FileInformation.h"
#include "Vector/Object.h"
#include "Vector/ObjectIterator.h"
#include "Vector/PageManager.h"

_SYDNEY_USING

using namespace Vector;

// コンストラクタはprotected

//
//	FUNCTION
//	Vector::ObjectIterator::~ObjectIterator -- デストラクタ
//
//	NOTE
//		デストラクタ。
//		他のクラスの基底クラスなのでvirtualである。
//
//	ARGUMENTS
//		なし
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//
ObjectIterator::~ObjectIterator()
{
}

// PROTECTED FUNCTIONS

//
//	FUNCTION
//	Vector::ObjectIterator::ObjectIterator -- コンストラクタ
//
//	NOTE
//		コンストラクタ
//			外部から直接使わせないためにprotectedにした。
//
//	ARGUMENTS
//		FileParameter&		rFileParameter_
//			
//		OpenParameter&		rOpenParameter_
//			
//		PageManager&		rPageManager_
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//			
ObjectIterator::ObjectIterator(FileParameter&		rFileParameter_,
							   OpenParameter&		rOpenParameter_,
							   PageManager&		rPageManager_)
	: m_rFileParameter(rFileParameter_),
	  m_rOpenParameter(rOpenParameter_),
	  m_rPageManager(rPageManager_)
{
}

//
//	Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
