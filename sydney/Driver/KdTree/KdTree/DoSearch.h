// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DoSearch.h --
// 
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_KDTREE_DOSEARCH_H
#define __SYDNEY_KDTREE_DOSEARCH_H

#include "KdTree/Module.h"
#include "Utility/OpenMP.h"

#include "KdTree/Node.h"

#include "ModPair.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

class Entry;
class KdTreeIndex;

//
//	CLASS
//	KdTree::DoSearch -- 最近傍検索を実行する
//
//	NOTES
//
class DoSearch : public Utility::OpenMP
{
public:
	// コンストラクタ
	DoSearch(const KdTreeIndex* pMain_,
			 const KdTreeIndex* pSmall1_,
			 const KdTreeIndex* pSmall2_,
			 const ModVector<ModVector<float> >& vecCondition_,
			 Node::Status* pMainStatus_,
			 Node::Status* pSmall1Status_,
			 Node::Status* pSmall2Status_);
	// デストラクタ
	virtual ~DoSearch();

	// 検索結果を得る
	const ModVector<ModVector<ModPair<ModUInt32, double> > >& getResult() const
		{ return m_vecResult; }

	// 検索を実行する
	void parallel();

protected:
	// 次に処理すべき検索条件と格納すべき結果を得る
	Entry* getNext(
		ModVector<ModVector<ModPair<ModUInt32, double> > >::Iterator& i_);
	
	// エントリを作成する
	Entry* makeEntry(const ModVector<float>& vecValue_);

	// KD-Tree
	const KdTreeIndex* m_pMain;
	const KdTreeIndex* m_pSmall1;
	const KdTreeIndex* m_pSmall2;

	// ステータス
	Node::Status* m_pMainStatus;
	Node::Status* m_pSmall1Status;
	Node::Status* m_pSmall2Status;
	
	// 検索条件
	const ModVector<ModVector<float> >& m_vecCondition;
	// 検索結果
	ModVector<ModVector<ModPair<ModUInt32, double> > > m_vecResult;

	// 処理した検索条件の位置
	int m_iElement;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_DOSEARCH_H

//
//  Copyright (c) 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
