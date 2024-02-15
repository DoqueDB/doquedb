// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// JapaneseBlocker1 -- 日本語用のブロック化器
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

#ifndef __SYDNEY_FULLTEXT2_JAPANESEBLOCKER1_H
#define __SYDNEY_FULLTEXT2_JAPANESEBLOCKER1_H

#include "FullText2/Module.h"
#include "FullText2/JapaneseBlocker.h"

#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::JapaneseBlocker1 --
//
//	NOTES
//
class JapaneseBlocker1 : public JapaneseBlocker
{
public:
	// コンストラクタ
	JapaneseBlocker1();
	// デストラクタ
	virtual ~JapaneseBlocker1();

	// トークナイズする文字列を設定する
	void set(const ModUnicodeString& cTarger_,	// 対象文字列
			 bool bSearch_);					// 検索 or 登録

	// トークンの取得
	bool yield(ModUnicodeString& cToken_,	// トークン
			   ModSize& uiOffset_,		// オフセット
			   bool& isShort_,			// ショートかどうか
			   ModSize& uiMinLength_);	// ショートの場合、その文字種の最短長

	// 文字種跨ぎの先頭処理を行っているかどうか
	bool isPrefixProcessing() { return true; }

private:
	// 現在のトークナイズのモードが検索用かどうか
	bool m_bSearch;
	
	// トークナイズする文字列へのポインタ
	const ModUnicodeChar* m_pTarget;
	// トークナイズする文字列の長さ
	ModSize m_uiTargetLength;

	ModSize m_uiTailOffset;			// 末尾部分 (文書の先頭に近い側)
	ModSize m_uiHeadOffset;			// 先頭部分 (文書の末尾に近い側)
	ModSize m_uiMaxLength;			// 最大長
	ModSize m_uiMinLength;			// 最小長
	ModSize m_uiCurrentLength;		// 現トークンの必要な長さ
	ModSize m_uiCurrentMaxLength;	// 現トークンの必要な最大長
	bool m_bShortWord;				// ショートワードかどうか

	ModSize m_uiHeadBlock;			// 文字種1
	ModSize m_uiTailBlock;			// 文字種2
	bool m_bPairValid;				// ペアのブロックかどうか
	bool m_bNewBlock;				// 新しい文字種かどうか
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_JAPANESEBLOCKER1_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
