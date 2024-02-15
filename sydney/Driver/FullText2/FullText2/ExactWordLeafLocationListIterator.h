// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExactWordLeafLocationListItertor.h --
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

#ifndef __SYDNEY_FULLTEXT2_EXACTWORDLEAFLOCATIONLISTITERATOR_H
#define __SYDNEY_FULLTEXT2_EXACTWORDLEAFLOCATIONLISTITERATOR_H

#include "FullText2/Module.h"
#include "FullText2/WordLeafLocationListIterator.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::ExactWordLeafLocationListIterator
//		-- 検索時に位置情報を走査するクラス
//
//	NOTES
//
class ExactWordLeafLocationListIterator : public WordLeafLocationListIterator
{
public:
	// コンストラクタ
	ExactWordLeafLocationListIterator(WordLeafNode& cNode_,
									  ModVector<ModSize>& cWordPosition_);
	// デストラクタ
	virtual ~ExactWordLeafLocationListIterator();

	// 位置情報を検索する
	bool find(ModSize location_, int& length_);
	ModSize lowerBound(ModSize location_, int& length_);
	// カーソルを先頭に戻す
	void reset() { resetImpl(); }
	// 次の値を得る
	ModSize next(int& length_) { return nextImpl(length_); }

	// 解放する
	bool release();
	// 文書内頻度を得る
	ModSize getTermFrequency();

private:
	// 位置情報を検索する
	ModSize lowerBoundImpl(ModSize location_, int& length_);
	// カーソルをリセットする
	void resetImpl();
	// 次の値を得る
	ModSize nextImpl(int& length_);
	// 文書内頻度を得る
	ModSize getTermFrequencyImpl();

	// 現在位置
	ModSize m_uiCurrentLocation;
	// 現在長
	int m_iCurrentLength;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_EXACTWORDLEAFLOCATIONLISTITERATOR_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
