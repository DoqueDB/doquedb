// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NullDataTree.cpp --
// 
// Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Array";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Array/NullDataTree.h"

#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_ARRAY_USING

//
//	FUNCTION public
//	Array::NullDataTree::NullDataTree -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
NullDataTree::NullDataTree(const FileID& cFileID_)
	: Tree(cFileID_)
{
}

//
//	FUNCTION public
//	Array::NullDataTree::~NullDataTree -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
NullDataTree::~NullDataTree()
{
}

//
//	FUNCTION public
//	Array::NullDataTree::setIndex --
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cData_
//		データ
//
//	RETURN
//
//	EXCEPTIONS
//
void
NullDataTree::setIndex(Common::DataArrayData& cData_, int iIndex_) const
{
	// [YET!] DataTree::setIndexと同じ
	
	; _SYDNEY_ASSERT(iIndex_ >= 0);

	setDefaultData(
		cData_, getIndexPosition(), static_cast<unsigned int>(iIndex_));
}

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
