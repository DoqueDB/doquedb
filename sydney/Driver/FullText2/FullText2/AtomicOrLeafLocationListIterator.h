// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AtomicOrLeafLocationListItertor.h --
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

#ifndef __SYDNEY_FULLTEXT2_ATOMICORLEAFLOCATIONLISTITERATOR_H
#define __SYDNEY_FULLTEXT2_ATOMICORLEAFLOCATIONLISTITERATOR_H

#include "FullText2/Module.h"
#include "FullText2/ShortLeafLocationListIterator.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::AtomicOrLeafLocationListIterator
//		-- 検索時に位置情報を走査するクラス
//
//	NOTES
//
class AtomicOrLeafLocationListIterator : public ShortLeafLocationListIterator
{
	friend class AtomicOrLeafNode;
	
public:
	// コンストラクタ
	AtomicOrLeafLocationListIterator(LeafNode& cNode_,
									 ModSize reserve_)
		: ShortLeafLocationListIterator(cNode_, -1, reserve_) {}
	// デストラクタ
	virtual ~AtomicOrLeafLocationListIterator() {}
	
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_ATOMICORLEAFLOCATIONLISTITERATOR_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
