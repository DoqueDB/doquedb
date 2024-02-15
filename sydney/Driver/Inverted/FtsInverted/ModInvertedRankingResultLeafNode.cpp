// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedRankingResultLeafNode.cpp -- ModInvertedRankingResultLeafNodeの実装
// 
// Copyright (c) 1999, 2000, 2001, 2002, 2003, 2005, 2006, 2023 Ricoh Company, Ltd.
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

#ifdef SYD_INVERTED // SYDNEY 対応
#include "SyDefault.h"
#include "SyReinterpretCast.h"
#endif

#include "ModInvertedQuery.h"
#include "ModInvertedSearchResult.h"
#include "ModInvertedBooleanResultLeafNode.h"
#include "ModInvertedRankingResultLeafNode.h"
#include "ModInvertedFileCapsule.h"

#ifdef	SYD_INVERTED // SYDNEY 対応
#include "Inverted/ModInvertedFile.h"
#include "Inverted/ModInvertedDocumentLengthFile.h"
#else
#include "ModInvertedFile.h"
#include "ModInvertedDocumentLengthFile.h"
#endif

///////////////////////////////////////////////////////////////////////
//
//	RankingResultLeafNode
//
ModInvertedRankingResultLeafNode::ModInvertedRankingResultLeafNode(
	const ModInvertedRankingResultLeafNode* originalNode,const ModUInt32 resultType_)
	: ModInvertedBooleanResultLeafNode(originalNode,resultType_)
{
	retrieved = ModTrue;

	this->lower = 1;
	this->upper = 0;
}
ModInvertedRankingResultLeafNode::ModInvertedRankingResultLeafNode(
	const ModInvertedSearchResult* result,const  ModUInt32 resultType_)
	: ModInvertedBooleanResultLeafNode(result,resultType_)
{
	retrieved = ModTrue;

	this->lower = 1;
	this->upper = 0;
}

void
ModInvertedRankingResultLeafNode::contentString(ModUnicodeString& content) const
{

	ModOstrStream out;
	ModSize i = this->begin;
	if (i != this->end) {
		out << '{' << searchResult->getDocID(i) << ',';
		out << searchResult->getScore(i) << '}';
		for (++i; i != this->end; ++i) {
			out << '{' << searchResult->getDocID(i) << ',';
			out << searchResult->getScore(i) << '}';
		}
	}

	// ModOstrStream::getString(),ModUnicodeString()とも
	// デフォルト漢字コードはutf8のため引数なし
	content = ModUnicodeString(out.getString());
}

#if 0
void
ModInvertedRankingResultLeafNode::removeFromFirstStepResult(
	const ModInvertedSearchResult* bresult)
{
	ModInvertedQueryNode::removeFromFirstStepResult(bresult);
	ModInvertedSearchResult* result = getRankingResult();
	begin = 0;
	end = estimatedDocumentFrequency = result->getSize();
}
#endif
void
ModInvertedRankingResultLeafNode::checkQueryNode(
	ModInvertedQuery* query_,
	const ModBoolean setStringInChildren_,
	const ModBoolean needDF_
	)
{
	if(needDF_ == ModTrue) {
		setNeedDF(needDF_);
	}

	ModUnicodeString token,tmp;
	this->prefixString(token, ModFalse, ModFalse);
	this->contentString(tmp);
	token += '(';
	token += tmp;
	token += ')';
	query_->insertTermNode(token,
							 const_cast<ModInvertedRankingResultLeafNode*>(this));
}

//
// Copyright (c) 1999, 2000, 2001, 2002, 2003, 2005, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
