// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OperatorTermNode.h --
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

#ifndef __SYDNEY_FULLTEXT2_OPERATORTERMNODE_H
#define __SYDNEY_FULLTEXT2_OPERATORTERMNODE_H

#include "FullText2/Module.h"
#include "FullText2/OperatorNode.h"

#include "ModUnicodeString.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class ScoreCalculator;

//
//	CLASS
//	FullText2::OperatorTermNode -- 末端のノードをあらわすクラス
//
//	NOTES
//
class OperatorTermNode : public OperatorNode
{
public:
	//
	//	CLASS
	//	FullText2::OperatorTermNode::Frequency
	//
	class Frequency
	{
	public:
		// コンストラクタ
		Frequency()
			: m_uiDocumentFrequency(0),
			  m_ulTotalTermFrequency(0) {}

		// コピーコンストラクタ
		Frequency(const Frequency& s)
			: m_uiDocumentFrequency(s.m_uiDocumentFrequency),
			  m_ulTotalTermFrequency(s.m_ulTotalTermFrequency) {}

		// 子ノード分を得る
		ModVector<Frequency>& getChild(SearchInformation& cSearchInfo_);

		// マージする
		void merge(const Frequency& frequency_);

		ModUInt32 m_uiDocumentFrequency;	// 文書頻度
		ModUInt64 m_ulTotalTermFrequency;	// 総文書内頻度
		
		ModVector<Frequency> m_vecChild;	// 子ノード分
	};
	
	// コンストラクタ
	OperatorTermNode(const ModUnicodeString& cString_);
	// デストラクタ
	virtual ~OperatorTermNode();
	// コピーコンストラクタ
	OperatorTermNode(const OperatorTermNode& src_);

	// 文字列表記を得る(tea構文)
	const ModUnicodeString& getString() { return m_cString; }

	// 文書内頻度を得る
	virtual ModSize getTermFrequency() = 0;
	// スコア計算器を設定する
	virtual void setScoreCalculator(ScoreCalculator* pCalculator_) = 0;

	// 文書頻度を得る
	virtual void getDocumentFrequency(SearchInformation& cSearchInfo_,
									  DocumentID bid_,	// 開始文書ID
									  DocumentID eid_,	// 終端文書ID
									  Frequency& cValue_,
									  bool bGetTotalTermFrequency_) = 0;

protected:
	// 文字列表記
	ModUnicodeString m_cString;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_OPERATORTERMNODE_H

//
//	Copyright (c) 2010, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
