// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SimpleResultSet.h --
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

#ifndef __SYDNEY_FULLTEXT2_SIMPLERESULTSET_H
#define __SYDNEY_FULLTEXT2_SIMPLERESULTSET_H

#include "FullText2/Module.h"
#include "FullText2/Types.h"

#include "Common/LargeVector.h"

#include "ModPair.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::SimpleResultSet -- 文書IDとスコアの組み合わせの検索結果クラス
//
//	NOTES
//
class SimpleResultSet
	: public Common::LargeVector<ModPair<DocumentID, DocumentScore> >
{
public:
	// 基底クラス
	typedef Common::LargeVector<ModPair<DocumentID, DocumentScore> > Super;

	// 比較クラス
	class IDLess
	{
	public:
		ModBoolean operator () (const ModPair<DocumentID, DocumentScore>& a1,
								const ModPair<DocumentID, DocumentScore>& a2)
		{
			return (a1.first < a2.first) ? ModTrue : ModFalse;
		}
	};
	class IDGreater
	{
	public:
		ModBoolean operator () (const ModPair<DocumentID, DocumentScore>& a1,
								const ModPair<DocumentID, DocumentScore>& a2)
		{
			return (a1.first > a2.first) ? ModTrue : ModFalse;
		}
	};

	class ScoreLess
	{
	public:
		ModBoolean operator () (const ModPair<DocumentID, DocumentScore>& a1,
								const ModPair<DocumentID, DocumentScore>& a2)
		{
			return (a1.second < a2.second) ? ModTrue : ModFalse;
		}
	};
	class ScoreGreater
	{
	public:
		ModBoolean operator () (const ModPair<DocumentID, DocumentScore>& a1,
								const ModPair<DocumentID, DocumentScore>& a2)
		{
			return (a1.second > a2.second) ? ModTrue : ModFalse;
		}
	};
	
	// コンストラクタ
	SimpleResultSet();
	// デストラクタ
	virtual ~SimpleResultSet();
	// コピーコンストラクタ
	SimpleResultSet(const SimpleResultSet& o);

	// 代入演算子
	SimpleResultSet& operator = (const SimpleResultSet& o);

	// ソートする
	void sort(SortKey::Value eKey_, Order::Value eOrder_);
	void sort(int iStart_, int iEnd_,
			  SortKey::Value eKey_, Order::Value eOrder_);
	
	// マージする
	void merge(const SimpleResultSet& cResultSet_,
			   SortKey::Value eKey_, Order::Value eOrder_);

private:
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_SIMPLERESULTSET_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
