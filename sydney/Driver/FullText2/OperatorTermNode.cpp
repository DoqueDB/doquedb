// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OperatorTermNode.cpp --
// 
// Copyright (c) 2010, 2013, 2023 Ricoh Company, Ltd.
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
#include "FullText2/OperatorTermNode.h"
#include "FullText2/SearchInformation.h"

#include "Exception/BadArgument.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::OperatorTermNode::Frequency::getChild
//		-- 子ノード分の領域を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModVector<FullText2::OperatorTermNode::Frequency>&
//		子ノードのDF値等を格納するクラス
//
//	EXCEPTIONS
//
ModVector<OperatorTermNode::Frequency>&
OperatorTermNode::Frequency::getChild(SearchInformation& cSearchInfo_)
{
	if (m_vecChild.getSize() == 0)
	{
		// 子ノード分加える
		m_vecChild.assign(cSearchInfo_.getElementSize());
	}

	return m_vecChild;
}

//
//	FUNCTION public
//	FullText2::OperatorTermNode::merge -- マージする
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::OperatorTermNode::Frequency& frequency_
//		頻度情報
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OperatorTermNode::Frequency::merge(const Frequency& frequency_)
{
	if (m_vecChild.getSize() != frequency_.m_vecChild.getSize())
		// 子コードの数が同じじゃないとエラー
		_TRMEISTER_THROW0(Exception::BadArgument);
	
	// 自身
	m_uiDocumentFrequency += frequency_.m_uiDocumentFrequency;
	m_ulTotalTermFrequency += frequency_.m_ulTotalTermFrequency;

	// 子ノード
	ModVector<Frequency>::Iterator i = m_vecChild.begin();
	ModVector<Frequency>::ConstIterator j = frequency_.m_vecChild.begin();
	for (; i != m_vecChild.end(); ++i, ++j)
	{
		(*i).m_uiDocumentFrequency += (*j).m_uiDocumentFrequency;
		(*i).m_ulTotalTermFrequency += (*j).m_ulTotalTermFrequency;
	}
}

//
//	FUNCTION public
//	FullText2::OperatorTermNode::OperatorTermNode -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrString_
//		文字列表記
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OperatorTermNode::OperatorTermNode(const ModUnicodeString& cString_)
	: OperatorNode(), m_cString(cString_)
{
}

//
//	FUNCTION public
//	FullText2::OperatorTermNode::~OperatorTermNode -- デストラクタ
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
OperatorTermNode::~OperatorTermNode()
{
}

//
//	FUCNTION public
//	FullText2::OperatorTermNode::OperatorTermNode -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::OperatorTermNode& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OperatorTermNode::OperatorTermNode(const OperatorTermNode& src_)
	: OperatorNode(src_)
{
	m_cString = src_.m_cString;
}

//
//	Copyright (c) 2010, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
