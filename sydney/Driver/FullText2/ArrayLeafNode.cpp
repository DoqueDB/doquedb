// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ArrayLeafNode.cpp --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/ArrayLeafNode.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::ArrayLeafNode::ArrayLeafNode -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ArrayLeafNode::ArrayLeafNode()
	: LeafNode()
{
}

//
//	FUNCTION public
//	FullText2::ArrayLeafNode::~ArrayLeafNode -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ArrayLeafNode::~ArrayLeafNode()
{
}

//
//	FUNCTION public
//	FullText2::ArrayLeafNode::ArrayLeafNode
//		-- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::ArrayLeafNode& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ArrayLeafNode::ArrayLeafNode(const ArrayLeafNode& src_)
	: LeafNode(src_)
{
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
