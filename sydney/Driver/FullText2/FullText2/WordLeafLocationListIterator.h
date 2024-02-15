// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// WordLeafLocationListItertor.h --
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

#ifndef __SYDNEY_FULLTEXT2_WORDLEAFLOCATIONLISTITERATOR_H
#define __SYDNEY_FULLTEXT2_WORDLEAFLOCATIONLISTITERATOR_H

#include "FullText2/Module.h"
#include "FullText2/LocationListIterator.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class WordLeafNode;

//
//	CLASS
//	FullText2::WordLeafLocationListIterator
//		-- 検索時に位置情報を走査するクラス
//
//	NOTES
//
class WordLeafLocationListIterator : public LocationListIterator
{
public:
	// コンストラクタ
	WordLeafLocationListIterator(WordLeafNode& cNode_,
								 ModVector<ModSize>& cWordPosition_);
	// デストラクタ
	virtual ~WordLeafLocationListIterator();

	// 検索語を設定する
	void setTerm(LocationListIterator::AutoPointer term)
		{ m_pTerm = term; }
	// 単語境界を設定する
	void setSeparator(LocationListIterator::AutoPointer sep)
		{ m_pSeparator = sep; }

	// 解放する
	bool release();

protected:
	// カーソルをリセットする
	void resetImpl();
	
	// 単語境界の確認する位置
	ModVector<ModSize>& m_cWordPosition;
	
	// 検索語
	LocationListIterator::AutoPointer m_pTerm;
	// 単語境界
	LocationListIterator::AutoPointer m_pSeparator;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_WORDLEAFLOCATIONLISTITERATOR_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
