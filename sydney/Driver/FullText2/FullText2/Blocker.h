// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Blocker -- ブロック化器
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

#ifndef __SYDNEY_FULLTEXT2_BLOCKER_H
#define __SYDNEY_FULLTEXT2_BLOCKER_H

#include "FullText2/Module.h"

#include "ModVector.h"
#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::Blocker --
//
//	NOTES
//
class Blocker
{
public:
	// コンストラクタ
	Blocker();
	// デストラクタ
	virtual ~Blocker();

	// パースする
	virtual void parse(const ModUnicodeChar* parameter_) = 0;

	// トークナイズする文字列を設定する
	virtual void set(const ModUnicodeString& cTarget_,	// 対象文字列
					 bool bSearch_) = 0;				// 検索 or 登録
	// トークンの取得
	virtual bool yield(ModUnicodeString& cToken_,		// トークン
					   ModSize& uiOffset_,				// オフセット
					   bool& isShort_,					// ショートかどうか
					   ModSize& uiMinLength_) = 0;		// ショートの場合、
													    // その文字種の最短長

	//
	// 文字種跨ぎの先頭処理を行っているかどうか
	// これが true の場合、同じ位置のトークンが複数切り出されることになるので、
	// ショートワード検索の場合に位置合わせが必要となる(重複排除が必要)
	//
	virtual bool isPrefixProcessing() = 0;
	
protected:
	// トークン種別
	struct Token
	{
		enum Type
		{
			// Number と Keyword しかないが、増やす場合でも 15 まで
			// また合計で32ビット以内なので、組み合わせ数も最大8個まで
			
			Number					= 1,
			Number_Number			= (Number << 4) | Number,
			Keyword					= 2,
			Keyword_Number			= (Keyword << 4) | Number,
			Keyword_Number_Number	= (Keyword << 8) | (Number << 4) | Number,
			Keyword_Keyword			= (Keyword << 4) | Keyword
		};
	};
	
	// 次のトークンを得る
	static Token::Type nextToken(const ModUnicodeChar*& target_,
								 ModVector<const ModUnicodeChar*>& element_);
	
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_BLOCKER_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
