// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// WordLeafLocationListIterator.cpp --
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
#include "FullText2/WordLeafLocationListIterator.h"

#include "FullText2/WordLeafNode.h"

#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::WordLeafLocationListIterator::WordLeafLocationListIterator
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	WordLeafNode& cNode_
//		ノード
//	ModVector<ModSize>& cWordPosition_
//		単語境界を確認する位置
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
WordLeafLocationListIterator::
WordLeafLocationListIterator(WordLeafNode& cNode_,
							 ModVector<ModSize>& cWordPosition_)
	: LocationListIterator(&cNode_), m_cWordPosition(cWordPosition_)
{
}

//
//	FUNCTION public
//	FullText2::WordLeafLocationListIterator::~WordLeafLocationListIterator
//		-- デストラクタ
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
WordLeafLocationListIterator::~WordLeafLocationListIterator()
{
}

//
//	FUNCTION public
//	FullText2::WordLeafLocationListIterator::release
//		-- 解放する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
// 	bool
//		フリーリストにつなげた場合は true、
//		呼び出し側で delete しなくてはならない場合は false を返す
//
//	EXCEPTIONS
//
bool
WordLeafLocationListIterator::release()
{
	m_pTerm = 0;
	m_pSeparator = 0;
	return LocationListIterator::release();
}

//
//	FUNCTION protected
//	FullText2::WordLeafLocationListIterator::resetImpl
//		-- カーソルをリセットする
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
void
WordLeafLocationListIterator::resetImpl()
{
	// 子ノードをリセット
	if (m_pTerm.get()) m_pTerm->reset();
	if (m_pSeparator.get()) m_pSeparator->reset();
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
