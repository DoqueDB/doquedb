// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedQueryParser.cpp -- 検索式パーザ実装
// 
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2023 Ricoh Company, Ltd.
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

#ifdef SYD_INVERTED // SYDNEY 対応
#include "SyDefault.h"
#include "SyReinterpretCast.h"
#endif

#include "ModAutoPointer.h"
#include "ModUnicodeCharTrait.h"
#include "ModUnicodeString.h"

#include "ModInvertedException.h"
#include "ModInvertedQueryParser.h"
#include "ModInvertedOperatorAndNode.h"
#include "ModInvertedOrderedDistanceNode.h"
#include "ModInvertedSimpleTokenLeafNode.h"
#include "ModInvertedBooleanResultLeafNode.h"
#include "ModInvertedTermLeafNode.h"
#include "ModInvertedBooleanResultLeafNode.h"
#include "ModInvertedOperatorAndNode.h"
#include "ModInvertedOperatorOrNode.h"
#include "ModInvertedAtomicOrNode.h"
#include "ModInvertedOperatorAndNotNode.h"
#include "ModInvertedOperatorWindowNode.h"
#include "ModInvertedOperatorLocationNode.h"
#include "ModInvertedOperatorEndNode.h"
#include "ModInvertedSimpleWindowNode.h"
#include "ModInvertedOperatorScaleNode.h"
#include "ModInvertedRankingResultLeafNode.h"

#ifndef SYD_INVERTED
#include "ModInvertedRegexLeafNode.h" // Sydneyには不要
#endif

//
// MACRO
// MOD_INV_ACCEPT_NULL
//
// NOTES
// 空文字列を検索文字列として受理する。定義しない場合は例外となる。
// デフォルトは定義する。
//
#define MOD_INV_ACCEPT_NULL

//
// CNOST
// ModInvertedQueryParser::escapeChar -- エスケープ文字
//
// NOTES
// クエリパーザにとってのエスケープ文字
//
/*static*/ const ModUnicodeChar
ModInvertedQueryParser::escapeChar('\\');

//
// CNOST
// ModInvertedQueryParser::escapee -- エスケープされるべき文字群
//
// NOTES
// 検索文字列としてエスケープされるべき文字群
//
/*static*/ const ModUnicodeChar
ModInvertedQueryParser::escapee[] = {
	ModUnicodeChar('#'),
	ModUnicodeChar('('),
	ModUnicodeChar(')'),
	ModUnicodeChar('['),
	ModUnicodeChar(']'),
	ModUnicodeChar(','),
	ModUnicodeChar('\\')
	};

#ifdef V1_6
//
// CONST
// ModInvertedQueryParser:defaultLangSet -- デフォルトの言語セット
//
// NOTES
// デフォルトの言語セットを表す
//
/*static*/
const ModLanguageSet ModInvertedQueryParser::defaultLangSet;
#endif // V1_6

//
// FUNCTION public
// ModInvertedQueryParser::ModInvertedQueryParser -- 検索式パーザの生成
//
// NOTES
// 検索式パーザオブジェクトを新しく生成する。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
ModInvertedQueryParser:: ModInvertedQueryParser()
	: target(0), current(0),resultType(0)
{
}

ModInvertedQueryParser:: ModInvertedQueryParser(const ModUInt32 resultType_)
	: target(0), current(0),resultType(resultType_)
{
}

//
// FUNCTION public
// ModInvertedQueryParser::~ModInvertedQueryParser -- 検索式パーザの廃棄
//
// NOTES
// 検索式パーザオブジェクトを廃棄する。
//
// ARGUMENTS
// なし
// 
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
ModInvertedQueryParser::~ModInvertedQueryParser()
{}

#ifdef MOD_INV_SKIPSPACE
//
// FUNCTION protected
// ModInvertedQueryParser::skipWhiteSpaces -- 空白文字の読み飛ばし
//
// NOTES
// 検索式文字列から ASCII の空白文字を読み飛ばす。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
void
ModInvertedQueryParser::skipWhiteSpaces()
{
	ModUnicodeChar	wc;

	while (!this->iterator.isEnd()) {
		wc = (*this->target)[this->current];
		if (ModUnicodeCharTrait::isAscii(wc) && wc == '\\') {
			++(this->iterator);
			break;
		}
		else if (!ModUnicodeCharTrait::isAscii(wc) ||
				 !ModUnicodeCharTrait::isSpace(wc)) {
			break;
		}
		++(this->iterator);
	}
}
#endif

//
// FUNCTION protected
// ModInvertedQueryParser::yieldAsciiToken -- トークンの切り出し(ASCII文字列)
//
// NOTES
// 検索式文字列から ASCII 文字列をトークンとして切り出す。
// \ をエスケープ文字とし、直前に \ がある ASCII 文字は、区切り文字であっても
// 無条件にトークンに加える。エスケープ文字自身はトークンに加えないので、
// \ をトークンに含みたい場合は \\ と書く必要がある。
//
// ARGUMENTS
// ModString& token
//		結果格納用文字列
// const ModUnicodeString& delimiters
//		区切り文字を列挙したワイド文字列
// ModBoolean ignoreCaseFlag
//		欧文字の大文字小文字の別を無視するかどうかのフラグ
//		(省略可で、デフォルトは ModFalse)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryParser::yieldAsciiToken(ModUnicodeString& token,
										const ModUnicodeString& delimiters,
										ModBoolean ignoreCaseFlag)
{
	token.clear();

	while (this->current < this->length) {
		ModUnicodeChar uc = (*this->target)[this->current];
		if (uc == escapeChar) {
#if 1
			ModErrorMessage << "escape cannot be used" << ModEndl;
			ModThrowInvertedFileError(ModInvertedErrorInternal);
#else
			if (++this->current >= this->length) {
				token += escapeChar;
				break;
			}
			uc = (*this->target)[this->current];
			if (ModUnicodeCharTrait::isAscii(uc) == ModFalse) {
				token += escapeChar;
				break;
			}
#endif
		} else if (ModUnicodeCharTrait::isAscii(uc) == ModFalse ||
				   delimiters.search(uc) != 0) {
			return;
		}
		token += (ignoreCaseFlag ? ModUnicodeCharTrait::toLower(uc) : uc);
		++this->current;
	}
	ModErrorMessage << "eos" << ModEndl;
	ModThrowInvertedFileError(ModInvertedErrorInternal);
}

//
// FUNCTION protected
// ModInvertedQueryParser::yieldGeneralToken -- トークンの切り出し(一般文字列)
//
// NOTES
// 検索式文字列から ASCII に限らない文字列をトークンとして切り出す。
// \ をエスケープ文字とし、直前に \ がある文字は、区切り文字であっても
// 無条件にトークンに加える。エスケープ文字自身はトークンに加えないので、
// \ をトークンに含みたい場合は \\ と書く必要がある。
//
// ARGUMENTS
// ModString& token
//		結果格納用文字列
// const ModUnicodeString& delimiters
//		区切り文字を列挙したワイド文字列
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryParser::yieldGeneralToken(ModUnicodeString& token,
										  const ModUnicodeString& delimiters)
{
	token.clear();

	while (this->current < this->length) {
		ModUnicodeChar uc = (*this->target)[this->current];
		if (uc == escapeChar) {
			if (++this->current >= this->length) {
				token += escapeChar;
				break;
			}
			uc = (*this->target)[this->current];
		} else {
			if (delimiters.search(uc) != 0) {
				break;
			}
		}
		token += uc;
		++this->current;
	}
	// 検索語のみが与えられることがあるので、ここに来てもエラーではない
}

//
// FUNCTION protected
// ModInvertedQueryParser::parseAndInsertChildren -- 子ノードリストのパーズ
//
// NOTES
// 子ノードリストを表す文字列をパーズして、与えられた親ノードの子ノード
// リストをセットする。
//
// ARGUMENTS
// ModInvertedQueryNode* parent
//		親ノード
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryParser::parseAndInsertChildren(ModInvertedQueryNode* parent_)
{
	ModInvertedQueryInternalNode* parent
		= static_cast<ModInvertedQueryInternalNode*>(parent_);

	ModUnicodeChar uc;
	if (this->current < this->length &&
		(uc = (*this->target)[this->current]) == '(') {
		++this->current;
		// ) までの各引数をパーズ
		ModInvertedQueryNode* child = 0;
		while (this->current < this->length) {
			uc = (*this->target)[this->current];
			if (uc == ')') {
				if (child == 0) {
					ModErrorMessage << "no node found" << ModEndl;
					ModThrowInvertedFileError(ModInvertedErrorInternal);
				}
				++this->current;
				return;
			}
			child = 0;

			parse(child);

			if (child != 0) {
				parent->insertChild(child);
			}

			uc = (*this->target)[this->current];
			if (uc == ',') {
				if (child == 0) {
					ModErrorMessage << "no node found" << ModEndl;
					ModThrowInvertedFileError(ModInvertedErrorInternal);
				}
				child = 0;
				++this->current;
			} else if (uc != ')') {
				ModErrorMessage << "no delimiter" << ModEndl;
				ModThrowInvertedFileError(ModInvertedErrorInternal);
			}
		}

	}
	ModErrorMessage << "no node found" << ModEndl;
	ModThrowInvertedFileError(ModInvertedErrorInternal);
}

#if 0
//
// FUNCTION protected
// ModInvertedQueryParser::parseScaleInsertChildren -- 子ノードリストのパーズ
//
// NOTES
// 子ノードリストを表す文字列をパーズして、与えられた親ノードの子ノード
// リストをセットする。
//
// ARGUMENTS
// ModInvertedQueryInternalNode* parent
//		親ノード
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryParser::parseScaleInsertChildren(ModInvertedQueryInternalNode* parent)
{
	if (this->current < this->length) {
		++this->current;
		ModUnicodeChar wc = (*this->target)[this->current];
		if (wc == '(') {
			++this->current;
			ModInvertedQueryNode* child=0;
			int loopCounter = 0;

			while (this->current < this->length) {
				wc = (*this->target)[this->current];
				if (loopCounter > 0) {
					ModThrowInvertedFileError(ModInvertedErrorInternal);
				}
				parse(child);


				if (child != 0) {
					parent->insertChild(child);
				} else {
					ModThrowInvertedFileError(ModInvertedErrorInternal);
				}

				wc = (*this->target)[this->current];
				if (wc == ')') {
					++this->current;
					break;
				} else {
					// 子ノードの数が１以上
					ModThrowInvertedFileError(ModInvertedErrorInternal);
				}

				loopCounter++;
			}
		} else {
			ModThrowInvertedFileError(ModInvertedErrorInternal);
		}
	} else {
		ModThrowInvertedFileError(ModInvertedErrorInternal);
	}
}
#endif

//
// FUNCTION public
// ModInvertedQueryParser::parse -- 検索式のパーズ
//
// NOTES
// 検索式文字列をパーズして、検索式オブジェクトを作る。
//
// ARGUMENTS
// const ModString& queryString
//		検索式文字列
// ModInvertedQuery& query
//		検索式オブジェクト (結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryParser::parse(const ModUnicodeString& queryString,
							  ModInvertedQuery& query)
{
	// データメンバーの初期化
	this->target = &queryString;
	current = 0;
	length = target->getLength();
	isWindow = ModFalse;

	// 下請関数の呼び出し
	ModInvertedQueryNode* root = 0;
	this->parse(root);

	if (this->current != this->length) {
		ModErrorMessage << "invalid query" << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorInternal);
	}
	query.setRoot(root);
}

//
// FUNCTION protected
// ModInvertedQueryParser::parse -- パーズの実行 (下請)
//
// NOTES
// 下請関数として、検索式文字列の着目部分をパーズして、
// 検索オブジェクトを作る。
//
// ARGUMENTS
// ModInvertedQueryNode*& queryNode
//		検索式オブジェクトへのポインタ(結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryParser::parse(ModInvertedQueryNode*& queryNode)
{
	ModUnicodeString token;

	// 先頭文字による判別
	//   #	   演算子
	//   , )	 区切り子
	//   その他  キー文字列
	switch ((*this->target)[this->current]) {
	case '#':	// 演算子
		++this->current;
		// 非 ASCII 文字か空白文字か開き括弧かの前までを切り出し
		this->yieldAsciiToken(token, "([", ModTrue);

		// 演算子ごとの孫請関数の呼び出し
		if (token == "and") {		// 積集合
			parseAsOperatorAnd(queryNode);
		} else if (token == "or") {			// 和集合
			parseAsOperatorOr(queryNode);
		} else if (token == "syn") {		// orのatomic用構文
			parseAsOperatorSyn(queryNode);
		} else if (token == "and-not") { 	// 差集合
			parseAsOperatorAndNot(queryNode);
		} else if (token == "window" || token == "swindow" || token == "owindow") {		// 位置条件
			isWindow = ModTrue;
			parseAsOperatorWindow(queryNode);
#ifndef SYD_INVERTED	// 定義されてなかったら
		} else if (token == "regex") {		// 正規表現
			parseAsRegex(queryNode);
#endif
		} else if (token == "list" || token == "bresult") {
											// ブーリアン結果
			if (isWindow == ModTrue) {
				// windowの中にbresultが指定されたら例外を投げる
				ModThrowInvertedFileError(ModInvertedErrorInternal);
			}
			parseAsBooleanResult(queryNode);
		} else if (token == "rresult") {	// ランキング結果
			if (isWindow == ModTrue) {
				// windowの中にrresultが指定されたら例外を投げる
				ModThrowInvertedFileError(ModInvertedErrorInternal);	
			}
			parseAsRankingResult(queryNode);
		} else if (token == "scale") {		// 中間結果
			parseAsOperatorScale(queryNode);
		} else if (token == "location") {	// 前方一致
			parseAsOperatorLocation(queryNode);
		} else if (token == "end") {		// 前方一致
			parseAsOperatorEnd(queryNode);
		} else if (token == "term") {		// termLeafNode
			parseAsTermNode(queryNode);
		} else if (token == "token") {		// simpleTokenLeafNode
			parseAsTokenNode(queryNode);
		} else {							// その他 (未定義)
			ModThrowInvertedFileError(ModInvertedErrorInternal);
		}
		break;
	case ')':
	case ',':	// 区切り子
		break;
	default:	// キー文字列
		parseAsTermString(queryNode
#ifdef V1_6
						  , defaultLangSet
#endif // V1_6
			     );
		break;
	}
}

//
// FUNCTION private
// ModInvertedQueryParser::parseAsTermString -- パーズの実行 (孫請: キー文字列)
//
// NOTES
// 検索式文字列中の着目部分をキー文字列としてトークンに分割して、
// 間隔演算内部表現ノードオブジェクトを作る。
// ( , ) および ASCII の空白文字を区切り文字として扱う。この解釈を避けたい
// 場合は、直前に \ を入れてエスケープすること。\ をエスケープ文字として
// でなくその文字として扱いたい場合も同様。
//
// ARGUMENTS
// ModInvertedQueryNode*& queryNode
//		検索式オブジェクトへのポインタ(結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryParser::parseAsTermString(ModInvertedQueryNode*& queryNode
#ifdef V1_6
										  , const ModLanguageSet& langSet_
#endif // V1_6
											)
{
	ModUnicodeString termString;

	// 区切り文字が現れるか文字列が終わるまで切り出す
	this->yieldGeneralToken(termString, ",)");
	if (termString.getLength() == 0) {
		ModErrorMessage << "no termString" << ModEndl;
#ifndef MOD_INV_ACCEPT_NULL
		ModThrowInvertedFileError(ModInvertedErrorInternal);
#endif
	}

#ifdef DEBUG
	ModDebugMessage << "termString=" << termString << ModEndl;
#endif	
	queryNode = new ModInvertedTermLeafNode(termString,
											resultType,		
#ifdef V1_6
											 langSet_,
#endif // V1_6
									ModInvertedTermStringMode
									);
}

void
ModInvertedQueryParser::parseAsTermString(
	ModInvertedQueryNode*& queryNode,
	const ModInvertedTermMatchMode matchMode,
	ModUnicodeString& calculator
#ifdef V1_6
	, const ModLanguageSet& langSet_
#endif // V1_6
	)
{
	ModUnicodeString termString;

	// 区切り文字が現れるか文字列が終わるまで切り出す
	this->yieldGeneralToken(termString, ",)");
	if (termString.getLength() == 0) {
		ModErrorMessage << "no termString" << ModEndl;
#ifndef MOD_INV_ACCEPT_NULL
		ModThrowInvertedFileError(ModInvertedErrorInternal);
#endif
	}

#ifdef DEBUG
	ModDebugMessage << "termString=" << termString << ModEndl;
#endif	
	ModAutoPointer<ModInvertedTermLeafNode>
		tmpNode(new ModInvertedTermLeafNode(termString,
											resultType,
#ifdef V1_6
											langSet_,
#endif // V1_6
											matchMode
											));
	if (calculator.getLength() > 0) {
#ifdef DEBUG
		ModDebugMessage << "calculator=" << calculator << ModEndl;
#endif	
		// tmpNode->ModInvertedQueryNode::setScoreCalculator(calculator);
		tmpNode->setScoreCalculator(calculator);
	}

	queryNode = tmpNode.release();
}

//
// FUNCTION private
// ModInvertedQueryParser::parseAsTermNode -- パーズの実行 (孫請: 明示的に指定されたtermNode)
//
// NOTES
// 明示的に指定されたtermNode(#term(...))のパーズ
//
// ARGUMENTS
// ModInvertedQueryNode*& queryNode
//		検索式オブジェクトへのポインタ(結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryParser::parseAsTermNode(ModInvertedQueryNode*& queryNode)
{
	ModUnicodeChar uc;
	ModUnicodeString matchmode, calculator;
#ifdef V1_6
	ModUnicodeString lang;
#endif // V1_6

	if (this->current < this->length &&
		(uc = (*this->target)[this->current]) == '[') {
		++this->current;

		this->yieldAsciiToken(matchmode, ",]");

		if (this->current < this->length &&
			(*this->target)[this->current] == ',') {
			++this->current;

			this->yieldAsciiToken(calculator, ",]");

		}

#ifdef V1_6
		if (this->current < this->length &&
			((*this->target)[this->current] == ',' )) {
			++this->current;
			this->yieldAsciiToken(lang, "]");
#ifdef DEBUG
			ModDebugMessage << "languageSet=" << lang << ModEndl;
#endif
		}
#endif // V1_6

		++this->current;
	}
	// matchmode, calculator は省略可

	if (this->current < this->length &&
		(uc = (*this->target)[this->current]) == '(') {
		++this->current;

		if (matchmode.getLength() == 0) {
#ifdef V1_6
			if(lang.getLength() > 0) {
				ModLanguageSet langSet(lang);
				parseAsTermString(queryNode, langSet);
			} else {
				parseAsTermString(queryNode, defaultLangSet);
			}
#else
			parseAsTermString(queryNode);
#endif // V1_6
		} else {
			ModInvertedTermMatchMode mm;
			if (matchmode == "w") {
				// 単語検索（現在、＝単純単語検索）
				mm = ModInvertedTermWordMode;
			} else if (matchmode == "s") {
				// 単純単語検索
				mm = ModInvertedTermSimpleWordMode;
			} else if (matchmode == "e") {
				// 厳格単語検索
				mm = ModInvertedTermExactWordMode;
			} else if (matchmode == "h") {
				// 先頭が単語境界と一致
				mm = ModInvertedTermWordHead;
			} else if (matchmode == "t"){
				// 末尾が単語境界と一致
				mm = ModInvertedTermWordTail;
#ifdef V1_4
			} else if (matchmode == "m"){
				// 多言語対応モード
				// 文字列にCJKが含まれるときは単純単語検索
				// 含まれないときは厳格単語検索
				mm = ModInvertedTermMultiLanguageMode;
#endif // V1_4
#ifndef MOD_DIST // APPMODE
			} else if (matchmode == "a") {
				// approximateMode
				// 照合位置はwordModeと同じ
				// WordNodeLocationListIterator::getCurrentMatchType()
				// で照合タイプを取得できる
				// wordNode::getTermFrequency()でつかう
				mm = ModInvertedTermApproximateMode;
#endif
			} else {
				// 'w','s','e','h','t' 以外の文字が指定された
				// stringModeとする
				mm = ModInvertedTermStringMode;
			}
#ifdef V1_6
			if(lang.getLength() > 0) {
				ModLanguageSet langSet(lang);
				parseAsTermString(queryNode, mm, calculator, langSet);
			} else {
				parseAsTermString(queryNode, mm, calculator, defaultLangSet);
			}
#else
			parseAsTermString(queryNode, mm, calculator);
#endif // V1_6
		}

		if (this->current < this->length &&
			(*this->target)[this->current] == ')') {
			++this->current;
		} else {
			ModErrorMessage << "no close parenthesis" << ModEndl;
			ModThrowInvertedFileError(ModInvertedErrorInternal);
		}
	}
	else {
		// termString がない
		ModErrorMessage << "no termString" << ModEndl;
#ifndef MOD_INV_ACCEPT_NULL
		ModThrowInvertedFileError(ModInvertedErrorInternal);
#endif
	}
}

//
// FUNCTION private
// ModInvertedQueryParser::parseAsTokenNode -- パーズの実行 (孫請: 明示的に指定されたtokenNode)
//
// NOTES
// 明示的に指定されたtokenNode(#token(...))のパーズ
//
// ARGUMENTS
// ModInvertedQueryNode*& queryNode
//		検索式オブジェクトへのポインタ(結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryParser::parseAsTokenNode(ModInvertedQueryNode*& queryNode) 
{
	ModUnicodeChar uc;
	ModUnicodeString calculator;
	ModSize tokenLength(0);
	ModUnicodeString tokenString;


	if (this->current < this->length &&
		(uc = (*this->target)[this->current]) == '[') {
		++this->current;

		this->yieldAsciiToken(tokenString, ",]");
		tokenLength = ModUnicodeCharTrait::toInt(tokenString);

		if (this->current < this->length &&
			(*this->target)[this->current] == ',') {
			++this->current;

			this->yieldAsciiToken(calculator, "]");
		}
		++this->current;
	}
	// calculator は省略可

	if (this->current < this->length &&
		(uc = (*this->target)[this->current]) == '(') {
		++this->current;

		// 区切り文字が現れるか文字列が終わるまで切り出す
		this->yieldGeneralToken(tokenString, ",)");
		if (tokenString.getLength() == 0) {
			ModErrorMessage << "no tokenString" << ModEndl;
			ModThrowInvertedFileError(ModInvertedErrorInternal);
		}

		if (tokenLength == 0) {
			tokenLength = tokenString.getLength();
		}

#ifdef DEBUG
		ModDebugMessage << "tokenString=" << tokenString << ' '
						<< tokenLength << ModEndl;
#endif	
		ModAutoPointer<ModInvertedSimpleTokenLeafNode>
			tmpNode(new ModInvertedSimpleTokenLeafNode(tokenString,
													   tokenLength, 0,
														resultType));

		if (calculator.getLength() > 0) {
#ifdef DEBUG
			ModDebugMessage << "calculator=" << calculator << ModEndl;
#endif	
			tmpNode->setScoreCalculator(calculator);
		}

		if (this->current < this->length &&
			(*this->target)[this->current] == ')') {
			++this->current;
		} else {
			ModErrorMessage << "no close parenthesis" << ModEndl;
			ModThrowInvertedFileError(ModInvertedErrorInternal);
		}
		queryNode = tmpNode.release();

	}
	else {
		// tokenString がない
		ModErrorMessage << "no tokenString" << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorInternal);
	}
}

//
// FUNCTION private
// ModInvertedQueryParser::parseAsOperatorAnd -- パーズの実行 (孫請: AND)
//
// NOTES
// Boolean検索式文字列の着目部分を積集合演算の引数リストとしてパーズして、
// 積集合演算内部表現ノードオブジェクトを作る。
//
// ARGUMENTS
// ModInvertedQueryNode*& queryNode
//		検索式オブジェクトへのポインタ(結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryParser::parseAsOperatorAnd(ModInvertedQueryNode*& queryNode)
{
	ModUnicodeChar uc;
	ModUnicodeString combiner;

	if (this->current < this->length &&
		(uc = (*this->target)[this->current]) == '[') {
		++this->current;

		this->yieldAsciiToken(combiner, "]");
		++this->current;
	}
	// combiner は省略可

	ModAutoPointer<ModInvertedOperatorAndNode>
		tmpNode(new ModInvertedOperatorAndNode(resultType));

	if (combiner.getLength() > 0) {
#ifdef DEBUG
		ModDebugMessage << "combiner=" << combiner << ModEndl;
#endif	
		// tmpNode->ModInvertedQueryNode::setScoreCombiner(combiner);
		tmpNode->setScoreCombiner(combiner);
	}

	this->parseAndInsertChildren(tmpNode);

	queryNode = tmpNode.release();
}

//
// FUNCTION private
// ModInvertedQueryParser::parseAsOperatorOr -- パーズの実行 (孫請: OR)
//
// NOTES
// Boolean検索式文字列の着目部分を和集合演算の引数リストとしてパーズして、
// 和集合演算内部表現ノードオブジェクトを作る。
//
// ARGUMENTS
// ModInvertedQueryNode*& queryNode
//		検索式オブジェクトへのポインタ(結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryParser::parseAsOperatorOr(ModInvertedQueryNode*& queryNode)
{
	ModUnicodeChar uc;
	ModUnicodeString combiner;

	if (this->current < this->length &&
		(uc = (*this->target)[this->current]) == '[') {
		++this->current;

		this->yieldAsciiToken(combiner, "]");
		++this->current;
	}
	// combiner は省略可

	ModAutoPointer<ModInvertedOperatorOrNode>
		tmpNode(new ModInvertedOperatorOrNode(resultType));

	if (combiner.getLength() > 0) {
#ifdef DEBUG
		ModDebugMessage << "combiner=" << combiner << ModEndl;
#endif	
		// tmpNode->ModInvertedQueryNode::setScoreCombiner(combiner);
		tmpNode->setScoreCombiner(combiner);
	}

	this->parseAndInsertChildren(tmpNode);

	queryNode = tmpNode.release();
}

//
// FUNCTION private
// ModInvertedQueryParser::parseAsOperatorAndNot -- パーズの実行 (孫請: AndNot)
//
// NOTES
// Boolean検索式文字列の着目部分を差集合演算の引数リストとしてパーズして、
// 差集合演算内部表現ノードオブジェクトを作る。
//
// ARGUMENTS
// ModInvertedQueryNode*& queryNode
//		検索式オブジェクトへのポインタ(結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryParser::parseAsOperatorAndNot(ModInvertedQueryNode*& queryNode)
{
	ModUnicodeChar uc;
	ModUnicodeString combiner, negator;

	if (this->current < this->length &&
		(uc = (*this->target)[this->current]) == '[') {
		++this->current;

		this->yieldAsciiToken(combiner, ",]");

		if (this->current < this->length &&
			(*this->target)[this->current] == ',') {
			++this->current;

			this->yieldAsciiToken(negator, "]");
		}
		++this->current;
	}
	// combiner は省略可

	ModAutoPointer<ModInvertedOperatorAndNotNode>
		tmpNode(new ModInvertedOperatorAndNotNode(resultType));

	if (combiner.getLength() > 0) {
#ifdef DEBUG
		ModDebugMessage << "combiner=" << combiner << ModEndl;
#endif	
		// tmpNode->ModInvertedQueryNode::setScoreCombiner(combiner);
		tmpNode->setScoreCombiner(combiner);
	}
	if (negator.getLength() > 0) {
#ifdef DEBUG
		ModDebugMessage << "negator=" << negator << ModEndl;
#endif	
		tmpNode->setScoreNegator(negator);
	}

	this->parseAndInsertChildren(tmpNode);

	queryNode = tmpNode.release();
}

//
// FUNCTION private
// ModInvertedQueryParser::parseAsOperatorWindow -- パーズの実行 (孫請: Window)
//
// NOTES
// Boolean検索式文字列の着目部分を近傍演算の引数リストとしてパーズして、
// 近傍演算内部表現ノードオブジェクトを作る。
//
// 書式:
// (1) #window[min](children)
//	 例: #window[5](東京,特許,許可)
// (2) #window[min,max](children)
//	 例: #window[5,20](東京,特許,許可)
// (3) #window[min,order](children)
//	 例: #window[5,unordered](東京,特許,許可)
// (4) #window[min,max,order](children)
//	 例: #window[5,20,unordered](東京,特許,許可)
// (5) #window[min,calculator](children)
//	 例: #window[5](東京,特許,許可)
// (6) #window[min,max,calculator](children)
//	 例: #window[5,20](東京,特許,許可)
// (7) #window[min,order,calculator](children)
//	 例: #window[5,unordered](東京,特許,許可)
// (8) #window[min,max,order,calculator](children)
//	 例: #window[5,20,unordered](東京,特許,許可)
//
//
// ARGUMENTS
// ModInvertedQueryNode*& queryNode
//		検索式オブジェクトへのポインタ(結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryParser::parseAsOperatorWindow(ModInvertedQueryNode*& queryNode)
{
	ModSize paramnum(0), numnum(0);
	ModSize minimalDistance(1);
	ModSize maximalDistance(ModSizeMax);
	ModBoolean ordered(ModTrue);
	ModUnicodeChar uc;
	ModUnicodeString calculator, token;

	if (this->current < this->length &&
		(uc = (*this->target)[this->current]) == '[') {

		do {
			// トークンの切り出し
			++this->current;
			this->yieldAsciiToken(token, ",]");

			if (token.getLength() > 0) {
				if (token == "u" || token == "unodered") {
					// unordered
					ordered = ModFalse;
				} else if (token == "o" || token == "ordered") {
					// ordered
					ordered = ModTrue;
				} else if (ModUnicodeCharTrait::isDigit(token[0])) {
					if (numnum == 0) {
						// ２つ目の数値がある場合
						minimalDistance = ModUnicodeCharTrait::toInt(token);
					} else if (numnum == 1) {
						// ２つ目の数値がある場合
						maximalDistance = ModUnicodeCharTrait::toInt(token);
					} else {
						ModErrorMessage << "too much distance spec" << ModEndl;
						ModThrowInvertedFileError(ModInvertedErrorInternal);
					}
					++numnum;
				} else if (calculator.getLength() == 0) {
					// calculator と仮定する
					calculator = token;
				} else {
					ModErrorMessage << "invalid spec" << ModEndl;
					ModThrowInvertedFileError(ModInvertedErrorInternal);
				}
				token = "";
				++paramnum;
			} else {
				ModErrorMessage << "no window spec" << ModEndl;
				ModThrowInvertedFileError(ModInvertedErrorInternal);
			}

			// 区切り文字を調べる
			uc = (*this->target)[this->current];

		} while (uc == ',');

		uc = (*this->target)[this->current];
		if (uc == ']') {
			++this->current;
		} else {
			ModThrowInvertedFileError(ModInvertedErrorInternal);
		}
	}
	if (paramnum < 1) {
		ModErrorMessage << "too few window spec" << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorInternal);
	}
#ifdef DEBUG
	ModDebugMessage << minimalDistance << ' ' << maximalDistance << ' '
					<< int(ordered) << ' ' << calculator << ModEndl;
#endif

	// WINDOW ノードの作成
	ModAutoPointer<ModInvertedOperatorWindowNode>
		tmpNode(new ModInvertedOperatorWindowNode(minimalDistance,
												  maximalDistance,
													resultType));

	if (ordered == ModTrue) {
		tmpNode->setOrderedType();
	} else {
		tmpNode->setUnorderedType();
	}
	if (calculator.getLength() > 0) {
#ifdef DEBUG
		ModDebugMessage << "calculator=" << calculator << ModEndl;
#endif	
		// tmpNode->ModInvertedQueryNode::setScoreCombiner(calculator);
		tmpNode->setScoreCalculator(calculator);
	}

	queryNode = static_cast<ModInvertedQueryInternalNode*>(tmpNode.get());
	this->parseAndInsertChildren(queryNode);
	tmpNode.release();
}

#ifndef SYD_INVERTED	// 定義されてなかったら

//
// FUNCTION private
// ModInvertedQueryParser::parseAsRegex -- パーズの実行 (孫請:正規表現)
//
// NOTES
// 検索式文字列中の着目部分をキー文字列としてトークンに分割して、正規
// 表現ノードオブジェクトを作る。
//
// ARGUMENTS
// ModInvertedQueryNode*& queryNode
//		検索式オブジェクトへのポインタ(結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryParser::parseAsRegex(ModInvertedQueryNode*& queryNode)
{
	ModUnicodeChar uc;
	ModUnicodeString calculator, pattern;

	if (this->current < this->length &&
		(uc = (*this->target)[this->current]) == '[') {
		++this->current;

		this->yieldAsciiToken(calculator, "]");
		++this->current;
	}
	// calculator は省略可

	if (this->current < this->length &&
		(uc = (*this->target)[this->current]) == '(') {
		++this->current;

		// 区切り文字が現れるか文字列が終わるまで切り出す
		// （パターンに ')' を含める場合にはエスケープすること）
		this->yieldGeneralToken(pattern, ")");
		++this->current;
	}
	if (pattern.getLength() == 0) {
		// pattern は指定されなければならない
		ModErrorMessage << "no pattern" << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorInternal);
	}

#ifdef DEBUG
		ModDebugMessage << "pattern=" << pattern << ModEndl;
#endif	

	ModAutoPointer<ModInvertedRegexLeafNode>
		tmpNode(new ModInvertedRegexLeafNode(pattern,resultType));

	if (calculator.getLength() > 0) {
#ifdef DEBUG
		ModDebugMessage << "calculator=" << calculator << ModEndl;
#endif	
		tmpNode->setScoreCalculator(calculator);
	}

	queryNode = static_cast<ModInvertedQueryLeafNode*>(tmpNode.release());
}

#endif // SYD_INVERTED

//
// FUNCTION private
// ModInvertedQueryParser::parseAsBooleanResult -- パーズの実行 (Boolean)
//
// NOTES
// Boolean検索式文字列の着目部分を文書IDリストとしてパーズして、
// 中間結果内部表現ノードオブジェクトを作る。
//
// ARGUMENTS
// ModInvertedQueryNode*& queryNode
//		検索式オブジェクトへのポインタ(結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryParser::parseAsBooleanResult(ModInvertedQueryNode*& queryNode)
{
	ModInvertedBooleanResult result;
	ModUnicodeString token;
	ModUnicodeChar uc;
	ModBoolean existData(ModTrue);		// 空集合のためにここでは true

	// 引数リストのパーズ
	if (this->current < this->length &&
		(uc = (*this->target)[this->current]) == '(') {
		++this->current;
		// ) までの各引数をパーズ
		while (this->current < this->length) {
			uc = (*this->target)[this->current];
			if (uc == ')') {
				// ソートはコンストラクタで行われる
				if (existData == ModFalse) {
					ModErrorMessage << "no node found" << ModEndl;
					ModThrowInvertedFileError(ModInvertedErrorInternal);
				}
				queryNode = new ModInvertedBooleanResultLeafNode(&result,resultType);
				++this->current;
				return;
			}
			existData = ModFalse;

			yieldAsciiToken(token, ",)");
			if (token.getLength() > 0) {
				result.pushBack(ModUnicodeCharTrait::toInt(token));
				existData = ModTrue;
			} else {
				ModErrorMessage << "no node found" << ModEndl;
				ModThrowInvertedFileError(ModInvertedErrorInternal);
			}

			uc = (*this->target)[this->current];
			if (uc == ',') {
				++this->current;
				existData = ModFalse;
			}
		}
	}

	ModErrorMessage << "eos" << ModEndl;
	ModThrowInvertedFileError(ModInvertedErrorInternal);
}

//
// FUNCTION private
// ModInvertedQueryParser::parseAsRankingResult -- パーズの実行 (Ranking)
//
// NOTES
// Boolean検索式文字列の着目部分を文書IDリストとしてパーズして、
// 中間結果内部表現ノードオブジェクトを作る。
//
// ARGUMENTS
// ModInvertedQueryNode*& queryNode
//		検索式オブジェクトへのポインタ(結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryParser::parseAsRankingResult(ModInvertedQueryNode*& queryNode)
{
	ModUnicodeChar uc;
	ModUnicodeString token;
	ModInvertedSearchResultScore result;
	ModInvertedDocumentID documentID;
	ModInvertedDocumentScore documentScore;
	ModBoolean existData(ModTrue);		// 空集合のためにここでは true

	// 引数リストのパーズ
	if (this->current < this->length &&
		(uc = (*this->target)[this->current]) == '(') {
		++this->current;
		// ) までの各引数をパーズ
		while (this->current < this->length) {
			uc = (*this->target)[this->current];
			if (uc == ')') {
				if (existData == ModFalse) {
					ModErrorMessage << "no node found" << ModEndl;
					ModThrowInvertedFileError(ModInvertedErrorInternal);
				}
				result.sort();
				queryNode = new ModInvertedRankingResultLeafNode(&result,resultType);
				++this->current;
				return;
			}
			existData = ModFalse;

			if (uc == '{') {
				++this->current;
				yieldAsciiToken(token, ",)}");

				documentID = ModUnicodeCharTrait::toInt(token);

				uc = (*this->target)[this->current];
				if (uc != ',') {
					ModErrorMessage << "no score" << ModEndl;
					ModThrowInvertedFileError(ModInvertedErrorInternal);
				} else {
					++this->current;
				}

				yieldAsciiToken(token, "}");

				uc = (*this->target)[this->current];
				; ModAssert(uc == '}');
				++this->current;

				documentScore = ModUnicodeCharTrait::toFloat(token);

				result.pushBack(documentID,documentScore);
				existData = ModTrue;
			} else {
				ModErrorMessage << "no node found" << ModEndl;
				ModThrowInvertedFileError(ModInvertedErrorInternal);
			}

			uc = (*this->target)[this->current];
			if (uc == ',') {
				++this->current;
				existData = ModFalse;
			}
		}
	}

	ModErrorMessage << "eos" << ModEndl;
	ModThrowInvertedFileError(ModInvertedErrorInternal);
}

//
// FUNCTION private
// ModInvertedQueryParser::parseAsOperatorScale -- パーズの実行(scale)
//
// NOTES
// Boolean検索式文字列の着目部分を文書IDリストとしてパーズして、
// 中間結果内部表現ノードオブジェクトを作る。
//
// ARGUMENTS
// ModInvertedQueryNode*& queryNode
//		検索式オブジェクトへのポインタ(結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryParser::parseAsOperatorScale(ModInvertedQueryNode*& queryNode)
{
	ModUnicodeString scale;
	ModUnicodeChar uc;

	if (this->current < this->length &&
		(uc = (*this->target)[this->current]) == '[') {
		++this->current;

		this->yieldAsciiToken(scale, "]");
		++this->current;
	}
	else {
		// SCALE では値が指定されなければならない
		ModErrorMessage << "no scale value" << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorInternal);
	}
	
	ModAutoPointer<ModInvertedOperatorScaleNode>
		tmpNode(new ModInvertedOperatorScaleNode(
			ModUnicodeCharTrait::toFloat(scale),resultType));

	this->parseAndInsertChildren(tmpNode);

	queryNode = tmpNode.release();
}

//
// FUNCTION private
// ModInvertedQueryParser::parseAsOperatorLocation -- パーズの実行(location)
//
// NOTES
//
// ARGUMENTS
// ModInvertedQueryNode*& queryNode
//		検索式オブジェクトへのポインタ(結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryParser::parseAsOperatorLocation(ModInvertedQueryNode*& queryNode)
{
	ModUnicodeChar uc;
	ModUnicodeString calculator, location;

	if (this->current < this->length &&
		(uc = (*this->target)[this->current]) == '[') {
		++this->current;

		this->yieldAsciiToken(location, ",]");

		if (this->current < this->length &&
			(*this->target)[this->current] == ',') {
			++this->current;

			this->yieldAsciiToken(calculator, "]");
		}
		++this->current;
	}
	// location, calculator は省略可

	ModAutoPointer<ModInvertedOperatorLocationNode> tmpNode;
	if (location.getLength() == 0) {
		tmpNode = new ModInvertedOperatorLocationNode(1,resultType);
	} else {
		tmpNode = new ModInvertedOperatorLocationNode(
			ModUnicodeCharTrait::toInt(location),resultType);
	}
	if (calculator.getLength() > 0) {
#ifdef DEBUG
		ModDebugMessage << "calculator=" << calculator << ModEndl;
#endif	
		tmpNode->setScoreCalculator(calculator);
	}

	queryNode = static_cast<ModInvertedQueryInternalNode*>(tmpNode.get());
	this->parseAndInsertChildren(queryNode);
	tmpNode.release();
}

//
// FUNCTION private
// ModInvertedQueryParser::parseAsOperatorEnd -- パーズの実行(end)
//
// NOTES
//
// ARGUMENTS
// ModInvertedQueryNode*& queryNode
//		検索式オブジェクトへのポインタ(結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryParser::parseAsOperatorEnd(ModInvertedQueryNode*& queryNode)
{
	ModUnicodeChar uc;
	ModUnicodeString calculator, distance;

	if (this->current < this->length &&
		(uc = (*this->target)[this->current]) == '[') {
		++this->current;

		this->yieldAsciiToken(distance, ",]");

		if (this->current < this->length &&
			(*this->target)[this->current] == ',') {
			++this->current;

			this->yieldAsciiToken(calculator, "]");
		}
		++this->current;
	}
	// distance, calculator は省略可

	ModAutoPointer<ModInvertedOperatorEndNode> tmpNode;
	if (distance.getLength() == 0) {
		tmpNode = new ModInvertedOperatorEndNode(0,resultType);
	} else {
		tmpNode = new ModInvertedOperatorEndNode(
			ModUnicodeCharTrait::toInt(distance),resultType);
	}

	if (calculator.getLength() > 0) {
#ifdef DEBUG
		ModDebugMessage << "calculator=" << calculator << ModEndl;
#endif	
		tmpNode->setScoreCalculator(calculator);
	}

	queryNode = static_cast<ModInvertedQueryInternalNode*>(tmpNode.get());
	this->parseAndInsertChildren(queryNode);
	tmpNode.release();
}

//
// FUNCTION private
// ModInvertedQueryParser::parseAsOperatorSyn -- パーズの実行(Syn)
//
// NOTES
// orのatomic版を表すSynのパーズ。
//
// ARGUMENTS
// ModInvertedQueryNode*& queryNode
//		検索式オブジェクトへのポインタ(結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedQueryParser::parseAsOperatorSyn(ModInvertedQueryNode*& queryNode)
{
	ModUnicodeChar uc;
	ModUnicodeString calculator;

	if (this->current < this->length &&
		(uc = (*this->target)[this->current]) == '[') {
		++this->current;

		this->yieldAsciiToken(calculator, "]");
		++this->current;
	}
	// combiner は省略可

	ModAutoPointer<ModInvertedAtomicOrNode>
		tmpNode(new ModInvertedAtomicOrNode(resultType));

	if (calculator.getLength() > 0) {
#ifdef DEBUG
		ModDebugMessage << "calculator=" << calculator << ModEndl;
#endif	
		tmpNode->setScoreCalculator(calculator);
	}

	queryNode = static_cast<ModInvertedOperatorOrNode*>(tmpNode.get());

	this->parseAndInsertChildren(queryNode);

	// queryNode は呼び出し側の制御に移るので、デストラクトされないようにする
	tmpNode.release();
}

//
// FUNCTION public
// ModInvertedQueryParser::convertTermString -- 検索文字列の変換
//
// NOTES
// 検索文字列をパースしても大丈夫なようにエスケープすべき文字（escapee）を
// escapeChar でエスケープする。
//
// ARGUMENTS
// const ModUnicodeString& in_
//		変換すべき検索文字列
// ModUnicodeString& out_
//		変換した検索文字列
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
/*static*/ void
ModInvertedQueryParser::convertTermString(const ModUnicodeString& in_,
										  ModUnicodeString& out_)
{
	// out_.reserve(in_.getLength());
	for (ModSize n(0); n < in_.getLength(); ++n) {
		if (ModUnicodeCharTrait::find(escapee, in_[n]) != 0) {
			out_.append(escapeChar);
		}
		out_.append(in_[n]);
	}
}
		
//
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
