// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedQueryParser.h -- 検索式パーザインタフェイス
// 
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedQueryParser_H__
#define __ModInvertedQueryParser_H__

#include "ModTypes.h"
class ModUnicodeString;

#include "ModInvertedTypes.h"
#include "ModInvertedManager.h"
#include "ModInvertedQuery.h"
#include "ModInvertedTermLeafNode.h"

//
// CLASS
// ModInvertedQueryParser -- 検索式パーザ
//
// NOTES
// 検索式文字列をパーズして、検索式内部表現に変換する。
// 検索式文字列以外に、以前の検索結果を渡して、絞り込み検索や
// 拡張検索のためのパーズもできる。
//
class
ModInvertedQueryParser : public ModInvertedObject
{
public:
	// コンストラクタ
	ModInvertedQueryParser();
	ModInvertedQueryParser(const  ModUInt32 resultType_);

	// デストラクタ
	~ModInvertedQueryParser();

	// パーズ
	void parse(const ModUnicodeString& queryString, ModInvertedQuery& query);

	static void convertTermString(const ModUnicodeString&, ModUnicodeString&);

private:
	// パーズ (下請)
	void parse(ModInvertedQueryNode*& queryNode);

#ifdef MOD_INV_SKIPSPACE
	// 空白文字の読み飛ばし
	void skipWhiteSpaces();
#endif

	// トークンの切り出し (演算子名などのASCII文字列)
	void yieldAsciiToken(ModUnicodeString& token,
						 const ModUnicodeString& delimiters,
						 ModBoolean ignoreCaseFlag = ModFalse);

	// トークンの切り出し (一般文字列)
	void yieldGeneralToken(ModUnicodeString& token,
						   const ModUnicodeString& delimiters);

	// 子ノードリストのパーズ
	void parseAndInsertChildren(ModInvertedQueryNode* parentNode);

	// #term のパーズ (孫請)
	void parseAsTermString(ModInvertedQueryNode*& queryNode
#ifdef V1_6
						   , const ModLanguageSet& langSet_	
#endif // V1_6
	                       );
	void parseAsTermString(ModInvertedQueryNode*& queryNode, 
						   const ModInvertedTermMatchMode,
						   ModUnicodeString& combinerName
#ifdef V1_6
						   , const ModLanguageSet& langSet_	
#endif // V1_6
						   );
	void parseAsTermNode(ModInvertedQueryNode*& queryNode);
	// #token のパーズ (孫請)
	void parseAsTokenNode(ModInvertedQueryNode*& queryNode);
	// #and のパーズ (孫請)
	void parseAsOperatorAnd(ModInvertedQueryNode*& queryNode);
	// #or のパーズ (孫請)
	void parseAsOperatorOr(ModInvertedQueryNode*& queryNode);
	// #syn(A,B) = #or[Atomic](A,B)
	void parseAsOperatorSyn(ModInvertedQueryNode*& queryNode);
	// #and-not のパーズ (孫請)
	void parseAsOperatorAndNot(ModInvertedQueryNode*& queryNode);
	// #window のパーズ (孫請)
	void parseAsOperatorWindow(ModInvertedQueryNode*& queryNode);
	// #window のパーズ (孫請)
	void parseAsRegex(ModInvertedQueryNode*& queryNode);
	// #bresult のパーズ
	void parseAsBooleanResult(ModInvertedQueryNode*& queryNode);
	// #rresult のパーズ
	void parseAsRankingResult(ModInvertedQueryNode*& queryNode);
	// #scale のパーズ
	void parseAsOperatorScale(ModInvertedQueryNode*& queryNode);
	// #location のパーズ
	void parseAsOperatorLocation(ModInvertedQueryNode*& queryNode);
	// #end のパーズ
	void parseAsOperatorEnd(ModInvertedQueryNode*& queryNode);

	// データメンバー
	const ModUnicodeString* target;
	const  ModUInt32 resultType;
	ModSize current;
	ModSize length;

	static const ModUnicodeChar escapeChar;
	static const ModUnicodeChar escapee[];
#ifdef V1_6
	static const ModLanguageSet defaultLangSet;
#endif // V1_6

	// 現在windowノードの子ノードかどうかの判定用
	ModBoolean isWindow;

};

#endif	// __ModInvertedQueryParser_H__

//
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
