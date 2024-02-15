// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedBlockedNgramTokenizer.h -- ブロック化 Ngram 分割器
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedBlockedNgramTokenizer_H__
#define __ModInvertedBlockedNgramTokenizer_H__

#include "ModInvertedNgramTokenizer.h"

class ModCharString;

class ModInvertedUnicodeCharBlocker;


//
// CLASS
// ModInvertedBlockedNgramTokenizer --- 日本語化 Ngram 分割器
//
// NOTES
// 日本語化した Ngram による分割器
//
class
ModInvertedBlockedNgramTokenizer : public ModInvertedNgramTokenizer
{
	friend class ModInvertedTokenizer;
public:
	ModInvertedBlockedNgramTokenizer(const ModCharString&,
									 ModInvertedFile*,
									 const ModBoolean = ModFalse);
	virtual ~ModInvertedBlockedNgramTokenizer();

	virtual void getDescription(ModCharString& description_,
								const ModBoolean withName_ = ModTrue) const;

protected:

	virtual void set(const ModUnicodeString&, const TokenizeMode,
					 const ModVector<ModSize>*, ModVector<ModSize>*);

	virtual ModBoolean tokenizeSub(const ModUnicodeString&,
								   const TokenizeMode,
								   ModInvertedLocationListMap&,
								   ModSize&,
								   ModUnicodeString&,
								   ModUnicodeString&,
								   ModUnicodeString&
#ifdef V1_6
								   , const ModLanguageSet&
#endif // V1_6
								   );

	ModBoolean yield(ModUnicodeString&, Occurrence&);

	// getTokenizedEnd() は NgramTokenizer のものをそのまま使用できる

private:
	void parse(const ModCharString&);

	// 以下は parse() の下請関数
	int getCharType(const ModCharString& stringCharType);
	ModSize divideIntoToken(ModCharString* token, const ModCharString& flase);
	void setArgument(const ModSize setCharType1, const ModSize setCharType2,
					 const ModSize minValue, const ModSize maxValue);

	ModVector<ModSize> blockMinLength;
	ModVector<ModSize> blockMaxLength;
	ModVector<ModBoolean> validPair;

	ModBoolean checkPair(const ModSize, const ModSize) const;
	void setPair(const ModSize, const ModSize);

	ModSize blockNum;
	ModInvertedUnicodeCharBlocker* blocker;

	ModSize headBlock;
	ModSize tailBlock;
	ModBoolean isNewBlock;

	ModBoolean pairValid;		// validPair を出力した場合、それを記憶する

	static const char* blockName[];
	static const char tokenizerName[];
};

#endif	// __ModInvertedBlockedNgramTokenizer_H__

//
// Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

