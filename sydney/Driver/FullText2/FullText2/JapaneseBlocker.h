// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// JapaneseBlocker -- 日本語用のブロック化器
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_JAPANESEBLOCKER_H
#define __SYDNEY_FULLTEXT2_JAPANESEBLOCKER_H

#include "FullText2/Module.h"
#include "FullText2/Blocker.h"

#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::JapaneseBlocker --
//
//	NOTES
//
class JapaneseBlocker : public Blocker
{
public:
	// ブロック名
	enum Name {
		Undefined = 0,			// 未定義 (0である)
		
		Ascii,
		Symbol,
		Digit,
		Alphabet,
		Hiragana,
		Katakana,
		Greek,
		Russian,
		Kanji,
		Gaiji,
		Other,					// その他
		
		ValueNum				// 異なり数
	};
	
	// コンストラクタ
	JapaneseBlocker();
	// デストラクタ
	virtual ~JapaneseBlocker();

	// パースする
	void parse(const ModUnicodeChar* parameter_);

protected:
	// ブロック数
	ModSize getBlockCount() const { return m_uiBlockCount; }
	// ブロック種別を得る
	virtual ModSize getBlock(ModUnicodeChar c_) const;

	// ペアかどうか
	bool checkPair(ModSize b1, ModSize b2)
		{ return m_pCheckPair[b1 * m_uiBlockCount + b2]; }
	// 値を得る
	void getBlockSize(ModSize b,
					  ModSize& min, ModSize& max)
		{
			min = m_pBlockMin[b];
			max = m_pBlockMax[b];
		}

private:
	// ２つのブロック種別がペアかどうか
	// m_pCheckPair[b1 * m_uiBlockCount + b2] とアクセスされる
	bool* m_pCheckPair;
	// ブロックの最小値
	ModSize* m_pBlockMin;
	// ブロックの最大値
	ModSize* m_pBlockMax;
	// ブロック数
	ModSize m_uiBlockCount;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_JAPANESEBLOCKER_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
