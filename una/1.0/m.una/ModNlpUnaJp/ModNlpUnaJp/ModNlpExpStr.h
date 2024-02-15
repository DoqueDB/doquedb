//
// ModNlpExpStr.h - header file of ModNlpExpStr class
// 
// Copyright (c) 2009,2012, 2023 Ricoh Company, Ltd.
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

#ifndef MODNLPEXPSTR_H
#define MODNLPEXPSTR_H

#include "ModVector.h"
#include "ModNlpUnaJp/ModNormalizer.h"

_UNA_BEGIN
_UNA_UNAJP_BEGIN

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CLASS
// EXP_Table - class for a table of search object morphemes to store to list
//
// NOTES
// 表に格納する検索対象形態素用テーブル
//
class EXP_Table {
public:
	EXP_Table(){
	}

	ModUnicodeString onDicStr;	// 検索対象の形態素
	ModUnicodeString onDicPat;	// 検索対象の形態素に対する展開パターン群
	bool onDic;					// 辞書引き結果 true:登録用語
	int topPos;					// 形態素vectorの登録位置 １〜
	int count;					// 形態素vectorの登録位置 １〜
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CLASS
// ModNlpExpStr - class for expanding character string
//
// NOTES
// class for expanding character string
//
class ModNlpExpStr : public ModDefaultObject
{
public:
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION public
//	ModNlpExpStr::ModNlpExpStr -- constructor
//
// NOTES
//	default constructor of ModNlpExpStr
//
// ARGUMENTS
//	ModNormalizer* const normalizer_
//		I:instance of ModNormalizer
//
// RETURN
//	none
//
// EXCEPTIONS
//	none
//
	ModNlpExpStr(ModNormalizer* const normalizer_);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION public
//	ModNlpExpStr::~ModNlpExpStr -- destructor
//
// NOTES
//	default destructor of ModNlpExpStr
//
// ARGUMENTS
//	none
//
// RETURN
//	none
//
// EXCEPTIONS
//	none
//
	~ModNlpExpStr();

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION public
//	ModNlpExpStr::expandStrings -- get expand character string patterns
//
// NOTES
//	This function acquires expand character string patterns
//  only in the ModNlpUnaJp module.
//
// ARGUMENTS
//	ModVector<ModUnicodeString>& form_
//		I:target strings
//	ModVector<ModUnicodeString>& expanded_
//		O:expanded strings patterns
//	ModSize maxExpPatternNum_
//		I:number of maximum expanded character string patterns
//	ModBoolean normFlag_
//		I:the flag of whether normalizer is on or off
//
// RETURN
//	ModSize
//		number of expanded strings patterns
//
// EXCEPTIONS
//
	ModSize expandStrings(ModVector<ModUnicodeString>& form_,
							ModVector<ModUnicodeString>& expanded_,
							ModSize maxExpPatternNum_,
							ModBoolean normFlag_ = ModFalse);

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////
// //
// // FUNCTION public
// //	ModNlpExpStr::expandStrings -- get expand character string patterns for user mode
// //
// // NOTES
// //	This function acquires expand character string patterns
// //  only in the ModNlpUnaJp module.
// //
// // ARGUMENTS
// //	ModUnicodeString& cstrOutStr_
// //		I:normalized target string
// //	ModVector<ModUnicodeString>& expanded_
// //		O:expanded strings patterns
// //	ModSize maxExpPatternNum_
// //		I:number of maximum expanded character string patterns
// //
// // RETURN
// //	ModSize
// //		number of expanded strings patterns
// //
// // EXCEPTIONS
// //
// 	ModSize expandStrings(const ModUnicodeString& cstrInputStr_,
// 							ModVector<ModUnicodeString>& expanded_,
// 							ModSize maxExpPatternNum_);


private:
	ModNormalizer* normalizer;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION private
//	ModNlpExpStr::searchExpnadStrings -- search expanded character string patterns
//
// NOTES
//	This fanction is used to search expanded character string patterns of ModlpExpStrData::search.
//
// ARGUMENTS
//	ModVector<ModUnicodeString>& form_
//		I:target strings
//	ModVector<ModUnicodeString>& expanded_
//		O:expanded strings patterns
//	ModSize maxExpPatternNum_
//		I:number of maximum expanded character string patterns
//	ModBoolean normFlag_
//		I:the flag of whether normalizer is on or off
//
// RETURN
//	ModSize
//		number of expanded character string patterns
//
// EXCEPTIONS
//
	ModSize searchExpandStrings(ModVector<ModUnicodeString>& form_,
									ModVector<ModUnicodeString>& expanded_,
									ModSize maxExpPatternNum_,
									ModBoolean& normFlag_);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION private
//      ModNlpExpStr::makeExpnadStrings --  Put a expanded character string pattern together 
//											and make character string.
//
// NOTES
//
// ARGUMENTS
//	ModVector<ModUnicodeString>& searched_pattern
//		I:expanded character strings
//	ModVector<ModUnicodeString>& expanded_
//		O:expanded character strings patterns
//
// RETURN
//	ModBoolean
//		ModTrue	 : the acquisition of the result succeeds
//		ModFalse : no input expanded character strings patterns
// 
// EXCEPTIONS
//
	ModBoolean expandJoin(ModList< ModVector<ModUnicodeString> > in_,
										ModVector<ModUnicodeString>& expanded_);
};

_UNA_UNAJP_END
_UNA_END

#endif // MODNLPEXPSTR_H

//
// Copyright (c) 2009,2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
