// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortLeafNodeCompatible.cpp
//	-- 古いDUAL(BNG)トークナイザーの文頭処理による弊害を吸収するためのクラス
//	   新しいトークナイザーでは不要
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
#include "FullText2/ShortLeafNodeCompatible.h"
#include "FullText2/ShortLeafLocationListIterator.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::ShortLeafNodeCompatible::ShortLeafNodeCompatible
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	int iLength_
//	   	検索文字列の長さ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ShortLeafNodeCompatible::ShortLeafNodeCompatible(int iLength_)
	: ShortLeafNode(iLength_)
{
}

//
//	FUNCTION public
//	FullText2::ShortLeafNodeCompatible::~ShortLeafNodeCompatible -- デストラクタ
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
ShortLeafNodeCompatible::~ShortLeafNodeCompatible()
{
}

//
//	FUNCTION public
//	FullText2::ShortLeafNodeCompatible::ShortLeafNodeCompatible
//		-- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::ShortLeafNodeCompatible& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ShortLeafNodeCompatible::
ShortLeafNodeCompatible(const ShortLeafNodeCompatible& src_)
	: ShortLeafNode(src_)
{
}

//
//	FUNCTION public
//	FullText2::ShortLeafNodeCompatible::getTermFrequency -- 文書内頻度を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		文書内頻度
//
//	EXCEPTIONS
//
ModSize
ShortLeafNodeCompatible::getTermFrequency()
{
	if (m_uiTermFrequency == 0)
	{
		// 文頭処理のため、同じ位置の文字が複数の索引単位に現れる場合があり、
		// ショートワードのTF値は、単純に全索引単位のTF値の合計ではない
		//
		// だから、ショートワードのランキング検索は遅い
	
		ShortLeafLocationListIterator* location = getLocationListIteratorImpl();
		LocationListIterator::AutoPointer p = location;
		m_uiTermFrequency = location->getTermFrequencyImpl();
	}
	return m_uiTermFrequency;
}

//
//	FUNCTION public
//	FullText2::ShortLeafNodeCompatible::copy -- コピーを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::LeafNode*
//		コピー
//
//	EXCEPTIONS
//
LeafNode*
ShortLeafNodeCompatible::copy() const
{
	return new ShortLeafNodeCompatible(*this);
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
