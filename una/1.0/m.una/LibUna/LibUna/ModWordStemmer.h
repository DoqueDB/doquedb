// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModWordStemmer.h -- 英単語正規化器
// 
// Copyright (c) 2002-2005, 2023 Ricoh Company, Ltd.
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
#ifndef	__ModWordStemmer_H__
#define __ModWordStemmer_H__

#include "UnaNameSpace.h"
#include "ModVector.h"
#include "ModLanguageSet.h"
#include "LibUna/ModStemDLL.h"

_UNA_BEGIN

//
// CLASS
//	ModWordStemmer -- 欧州語正規化器
//
// NOTES
//	正規化器の本体。以下の処理を行なう。
//		look
//			辞書引き（見出し登録されているかどうかの確認）
//		stem
//			正規化
//		expand
//			正規化＆展開
//
class ModStemDLL ModWordStemmer
	: public ModDefaultObject
{
protected:
	ModSize langNum;
	ModLanguageSet lang;
public:
	ModWordStemmer()
	{
		langNum = 0; 
	}
	virtual ~ModWordStemmer();
	ModWordStemmer(const ModLanguageSet& languageSet_);
	// 辞書引き
	virtual ModBoolean setLanguageSet(const ModLanguageSet& languageSet_);

	// 辞書引き
	virtual ModBoolean look(const ModUnicodeString& target);
	// 正規化
	virtual ModBoolean stem(const ModUnicodeString& target,
					ModUnicodeString& result);
	// 正規化＆展開
	virtual ModBoolean expand(const ModUnicodeString& target,
					  ModVector<ModUnicodeString>& result);
};
_UNA_END
#endif	// __ModWordStemmer_H__
//
// Copyright (c) 2002-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
