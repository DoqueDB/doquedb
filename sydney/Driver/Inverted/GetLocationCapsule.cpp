// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// GetLocationCapsule.cpp --
// 
// Copyright (c) 2002, 2004, 2005, 2006, 2009, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Inverted/GetLocationCapsule.h"
#include "Inverted/OpenOption.h"
#include "Inverted/InvertedFile.h"
#include "Inverted/IndexFile.h"
#include "Inverted/IndexFileSet.h"
#include "Inverted/Types.h"
#include "FileCommon/OpenOption.h"
#include "Exception/NotSupported.h"

#include "ModInvertedFileCapsule.h"
#include "ModInvertedQuery.h"
#include "ModInvertedQueryParser.h"
#include "ModInvertedSearchResult.h"
#include "ModInvertedLocationListIterator.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//  FUNCTION public
//  Inverted::GetLocationCapsule::GetLocationCapsule -- コンストラクタ
//
//  NOTES
//  コンストラクタ
//
//  ARGUMENTS
//  ModInvertedTokenizer* tokenizer_
//	  トークナイザー
//  Inverted::SearchCapsule& cSearchCapsule_
//	  検索カプセル
//  Inverted::InvertedFile* pFile_
//	  大転置ファイル
//  Inverted::InvertedFile* pInsertFile1_
//	  挿入用小転置1 (default 0)
//  Inverted::InvertedFile* pInsertFile2_
//	  挿入用小転置2 (default 0)
//  Inverted::InvertedFile* pExpungeFile1_
//	  削除用小転置1 (default 0)
//  Inverted::InvertedFile* pExpungeFile2_
//	  削除用小転置2 (default 0)
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
GetLocationCapsule::GetLocationCapsule(
	SearchCapsule& cSearchCapsule_)
	:m_cSearchCapsule(cSearchCapsule_),	m_pQuery(0),
	 m_uiUpperLimitPerTerm(0), m_uiUpperTermLimit(0)
{
}

//
//  FUNCTION public
//  Inverted::GetLocationCapsule::~GetLocationCapsule -- デストラクタ
//
//  NOTES
//  デストラクタ
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
GetLocationCapsule::~GetLocationCapsule()
{
	delete m_pQuery;
}

//
//  FUNCTION public
//  Inverted::GetLocationCapsule::execute -- 検索を実行する
//
//  NOTES
//  この検索では、削除用の小転置を参照していないので、
//  削除されたものもヒットしてしまうことがあるので、
//  呼び出し側で考慮する必要がある。
//
//  ARGUMENTS
//  ModUInt32 uiRowID_
//	  RowID
//
//  RETURN
//  const Inverted::GetLocationCapsule::ResultSet&
//	  検索結果集合
//
//  EXCEPTIONS
//
const GetLocationCapsule::ResultSet&
GetLocationCapsule::execute(ModUInt32 uiRowID_,
							ModSize& uiTermCount_)
{
	uiTermCount_ = 0;
	
	try
	{
	// cSearchCapsulleの中にある転置ファイル群を利用する

		if (m_pQuery == 0)
		{
			// 検索条件を得る
			ModUnicodeString cstrTea = m_cSearchCapsule.getCondition();
			if (cstrTea.getLength() == 0)
				_SYDNEY_THROW0(Exception::NotSupported);

			m_pQuery = new ModInvertedQuery;
			// パースする
			ModInvertedQueryParser cParser;
			cParser.parse(cstrTea, *m_pQuery);
		}

		ModUInt32 uiDocumentID;
		IndexFileSet  *pIndexFileSet = m_cSearchCapsule.getIndexFileSet();

		for(IndexFileSet::Iterator iter = pIndexFileSet->end() - 1 ; iter >= pIndexFileSet->begin() ; iter--)
		{
			if(iter->signature() & Inverted::Sign::INSERT_MASK)
			{
				if(iter->getCount() > 0){
					uiDocumentID = iter->convertToDocumentID(uiRowID_);
					if(uiDocumentID != UndefinedDocumentID)
					{
						search(iter->getInvertedFile(), uiDocumentID, uiTermCount_);
						break;
					}
				}
			}
		}
	}
	catch (...)
	{
		detach();
		_SYDNEY_RETHROW;
	}

	detach();

	return m_cResultSet;
}

//
//  FUNCTION private
//  Inverted::GetLocationCapsule::search -- 1つのファイルの位置情報を得る
//
//  NOTES
//
//  ARGUMENTS
//  Inverted::InvertedFile* pFile_
//	  転置ファイル
//  ModUInt32 uiDocumentID_
//	  文書ID
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
GetLocationCapsule::search(InvertedFile* pFile_,
						   ModUInt32 uiDocumentID_,
						   ModSize& uiTermCount_)
{
	// [NOTE] m_pQueryから得られる全てのSimpleTokenLeafNodeを取得し、
	//  各Nodeに対応する索引語の位置(開始位置と終了位置のペア)を全て取得し、
	//  m_cResultSetに設定する。

	; _TRMEISTER_ASSERT(uiTermCount_ == 0);
	
	m_cResultSet.clear();

	ModInvertedLocationListIterator* iterator = 0;

	ModInvertedQuery cQuery(*m_pQuery); // コピー
	cQuery.validate(pFile_,
					ModInvertedFileCapsuleGetMatchLocations::defaultValidateMode
					| ModInvertedQuery::rankingMode);
	if (cQuery.evaluate(
			uiDocumentID_,
			iterator,
			ModInvertedFileCapsuleGetMatchLocations::defaultEvaluateMode)
		== ModFalse)
	{
		// 検索条件を満たさなかったので終了
		return;
	}

	iterator->release(), iterator = 0;

	// 検索語ノードを収集する
	// 中間ノードを inode にためながら、索引語ノードを tnode にためる
	// [NOTE] ようは全てのSimpleTokeLeafNodeを収集する。
	ModInvertedQueryNode* node = cQuery.getRoot();
	ModVector<ModInvertedQueryNode*> inode;
	ModVector<ModInvertedQueryNode*> tnode;
	if (node->isConvertedFromTermLeafNode() == ModTrue
		|| node->getType() == ModInvertedQueryNode::simpleTokenLeafNode)
	{
		// ルートノードが検索語ノードに対応するの場合
		tnode.pushBack(node);
	}
	else if (node->getType() & ModInvertedQueryNode::internalNode)
	{
		// ルートノードが中間ノードの場合
		inode.pushBack(node);
		ModSize from = 0;
		ModSize to = 1;
		ModSize num;
		while (from < to)
		{
			num = 0;
			// 前のループで収集した未処理の中間ノードを処理する
			for (ModSize x = from; x < to; ++x)
			{
				ModVector<ModInvertedQueryNode*>* children
					= inode[x]->getChildren();
				for (ModSize n = 0; n < children->getSize(); ++n)
				{
					if ((*children)[n]->isConvertedFromTermLeafNode() == ModTrue
						|| (*children)[n]->getType()
						== ModInvertedQueryNode::simpleTokenLeafNode)
					{
						// 子ノードが検索語ノードの場合
						tnode.pushBack((*children)[n]);
					}
					else if ((*children)[n]->getType()&
							 ModInvertedQueryNode::internalNode)
					{
						// 子ノードが中間ノードの場合
						inode.pushBack((*children)[n]);
						++num;
					}
				}
			}
			from = to;
			to += num;
		}
	}

	// 検索語ノードごとに照合位置を集める
	ModSize uiID = 0;
	ModVector<ModInvertedQueryNode*>::Iterator entry = tnode.begin();
	for (; entry != tnode.end(); ++entry)
	{
		node = *entry;
		if (node->evaluate(
				uiDocumentID_,
				iterator,
				ModInvertedFileCapsuleGetMatchLocations::defaultEvaluateMode)
			== ModTrue)
		{
			// 照合位置をベクターにセットする
			ModSize uiCount = 0;
			iterator->reset();
			while (iterator->isEnd() == ModFalse)
			{
				// ModInvertedの位置情報は文字単位の1オリジンなので、1を引く
				m_cResultSet.pushBack(
					Result(iterator->getLocation() - 1,
						   SubResult(iterator->getEndLocation() - 1,
									 uiID)));
				// [NOTE] m_uiUpperLimitPerTerm == 0 の場合、全て取得する。
				if (++uiCount == m_uiUpperLimitPerTerm)
				{
					break;
				}
				iterator->next();
			}
			iterator->release();
			if (uiCount > 0)
			{
				++uiID;
			}

			// m_uiUpperTermLimit == 0 の場合、全て取得する。
			if (uiID == m_uiUpperTermLimit)
			{
				iterator = 0;
				break;
			}
		}
		iterator = 0;
	}

	inode.clear();
	tnode.clear();

	// 取得結果をソート
	// [NOTE] ラフKWICの領域計算は出現位置でソートされていることを仮定している
	ModSort(m_cResultSet.begin(), m_cResultSet.end());

	uiTermCount_ = uiID;
}

//
//  FUNCTION private
//  Inverted::GetLocationCapsule::detach -- すべてのファイルのページをdetachする
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
GetLocationCapsule::detach()
{
	m_cSearchCapsule.getIndexFileSet()->detachAllPages();
}

//
//  Copyright (c) 2002, 2004, 2005, 2006, 2009, 2011, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
