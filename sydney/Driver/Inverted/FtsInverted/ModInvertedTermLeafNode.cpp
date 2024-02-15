// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedTermLeafNode.cpp -- 検索語に対応する末端ノード実装
// 
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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
namespace
{
const char	srcFile[] = __FILE__;
const char	moduleName[] = "Inverted";
}
#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "ModAutoPointer.h"
#endif

#include "ModAlgorithm.h"

#include "ModInvertedTypes.h"
#ifdef	SYD_INVERTED // SYDNEY 対応
#include "Inverted/ModInvertedFile.h"
#include "Inverted/ModInvertedList.h"
#include "Inverted/Parameter.h"
#else
#include "ModInvertedFile.h"
#include "ModInvertedList.h"
#endif

#include "ModInvertedTokenizer.h"
#include "ModInvertedTermLeafNode.h"
#include "ModInvertedOperatorAndNode.h"
#ifdef V1_4 // 単語単位検索
#include "ModInvertedOperatorWordNode.h"
#endif // V1_4	単語単位検索
#include "ModInvertedOperatorOrNode.h"
#include "ModInvertedOrderedDistanceNode.h"
#include "ModInvertedWordOrderedDistanceNode.h"
#include "ModInvertedSimpleTokenLeafNode.h"
#include "ModInvertedQuery.h"
#include "ModInvertedAtomicOrNode.h"
#include "ModInvertedRankingScoreCombiner.h"

#ifdef SYD_INVERTED
#include "Exception/Object.h"
#endif

#define MOD_INV_USEROUGH_IN_EVALUATE

_SYDNEY_USING

namespace {
	// ModInvertedTermMultiLangaugeMode を有効にするかどうか
	Inverted::ParameterBoolean _cIsMulti("Inverted_MultiMatchMode", true);
}

//
// FUNCTION
// ModInvertedTermLeafNode::former -- 出現位置による比較
//
// NOTES
// 出現位置と検索語ノードのペアを検索語ノードの出現位置による比較を行う。
//
// ARGUMENTS
// const ModInvertedTermLeafNode::LocationQueryPair& x
//		比較データ１
// const ModInvertedTermLeafNode::LocationQueryPair& y
//		比較データ２
//
// RETURN
// 比較データ１が小さければ ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// なし
//
/*static*/ inline ModBoolean
ModInvertedTermLeafNode::former(
	const ModInvertedTermLeafNode::LocationQueryPair& x,
	const ModInvertedTermLeafNode::LocationQueryPair& y)
{
	return x.first < y.first ? ModTrue : ModFalse;
}

//
// FUNCTION
// ModInvertedTermLeafNode::former -- 文書頻度による比較
//
// NOTES
// 出現位置と検索語ノードのペアを検索語ノードの文書頻度による比較を行う。
//
// ARGUMENTS
// const ModInvertedTermLeafNode::LocationQueryPair& x
//		比較データ１
// const ModInvertedTermLeafNode::LocationQueryPair& y
//		比較データ２
//
// RETURN
// 比較データ１が小さければ ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// なし
//
/*static*/ inline ModBoolean
ModInvertedTermLeafNode::lessFrequent(
	const ModInvertedTermLeafNode::LocationQueryPair& x,
	const ModInvertedTermLeafNode::LocationQueryPair& y)
{
	return (x.second->estimateDocumentFrequency()
			< y.second->estimateDocumentFrequency() ?
			ModTrue : ModFalse);
}

//
// FUNCTION public
// ModInvertedTermLeafNode::~ModInvertedTermLeafNode -- 検索語に対応するノードの廃棄
//
// NOTES
// 検索語に対応する検索式内部表現ノードオブジェクトを廃棄する。
//
// queryNodeForPreciseEvaluation,queryNodeForRoughEvaluationは、
// eraceTermLeafNodeで0がセットされるのでここではなにもしなくてよい
//
// ARGUMENTS
// なし
//
// EXCEPTIONS
// なし
//
ModInvertedTermLeafNode::~ModInvertedTermLeafNode()
{
}

//
// FUNCTION public
// ModInvertedQueryNode::duplicate -- 自分のコピーを作成する
//
// NOTES
// 自分のコピーを作成する。Query::から呼ばれ、検索木を上からたどり、
// 検索木のコピーを作成する際に使用される。
// QueryNode::dupulicateをオーバーライド。TermLeafNodeのコピーを作る。
//
// ARGUMENTS
// const ModInvertedQuery&
//		クエリ。
//
// RETURN
// 生成したコピーのノードのポインタ
//
// EXCEPTIONS
// ModInvertedErrorQueryValidateFail
//
ModInvertedQueryNode*
ModInvertedTermLeafNode::duplicate(const ModInvertedQuery& rQuery)
{
	// 新しいTermLeafNodeを作成
	ModInvertedTermLeafNode* node
		= new ModInvertedTermLeafNode(this->termString,
									  firstStepResult->getType(),
#ifdef V1_6
									  langSet,
#endif // V1_6
									  this->matchMode);

	if (this->scoreCalculator != 0) {
		node->scoreCalculator = this->scoreCalculator->duplicate();
	}

	// totalDocumentFrequencyのセット
	node->setTotalDocumentFrequency(totalDocumentFrequency);

	return node;
}

//
// FUNCTION public
// ModInvertedTermLeafNode::contentString -- ノードの内容を表わす文字列を返す
//
// NOTES
// ノードの内容を表わす文字列を返す。QueryNodeで定義された内容をオーバ
// ライドする。
//
// ARGUMENTS
// ModString& prefix
//		ノードの内容を表わす文字列
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedTermLeafNode::contentString(ModUnicodeString& content) const
{
	content = this->termString;
}

//
// FUNCTION public
// ModInvertedTermLeafNode::prefixString -- 演算子を表わす文字列(#term)を返す
//
// NOTES
// QueryNodeで定義された内容をオーバライドする
// 演算子を表わす文字列(#term)を返す
//
// ARGUMENTS
// ModString& prefix
//		演算子を表わす文字列を返す(結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedTermLeafNode::prefixString(ModUnicodeString& prefix,
									  const ModBoolean withCalOrCombName,
									  const ModBoolean withCalOrCombParam) const
{
	ModBoolean isAddString = ModFalse;

	prefix += "#term";

	if( matchMode != 0 ) {
		// マッチモードを出力する
		ModUnicodeString matchModeName;
		switch(matchMode) {
		case ModInvertedTermWordHead:
			matchModeName = 'h';
			break;
		case ModInvertedTermWordTail:
			matchModeName = 't';
			break;
		case ModInvertedTermSimpleWordMode:
			matchModeName = 's';
			break;
		case ModInvertedTermExactWordMode:
			matchModeName = 'e';
			break;
#ifndef MOD_DIST // APPMODE
		case ModInvertedTermApproximateMode:
			matchModeName = 'a';
			break;
#endif
#ifdef V1_4
		case ModInvertedTermMultiLanguageMode:
			matchModeName = 'm';
			break;
#endif // V1_4
		default:
			matchModeName = 'n';
			break;
		}

		prefix += '[';
		prefix += matchModeName;

		isAddString = ModTrue;

#ifdef V1_6
		prefix += ',';
#endif // V1_6
	}

	if (withCalOrCombName == ModTrue) {
		// 計算器名を出力する
		ModUnicodeString calculatorName;
		getCalculatorOrCombinerName(calculatorName, withCalOrCombParam);
		if(calculatorName.getLength() > 0) {
#ifdef V1_6
			if (isAddString == ModFalse) {
				prefix += '[';
				isAddString = ModTrue;
			}
#else
			if (isAddString == ModTrue) {
				prefix += ',';
			} else {
				prefix += '[';
				isAddString = ModTrue;
			}
#endif // V1_6
			prefix += calculatorName;
		}
	}

#ifdef V1_6
	if (isAddString == ModTrue) {
		// 多言語対応の言語指定コードを追加
		ModUnicodeString langSetName;
		langSetName = langSet.getName();
		if(langSetName.getLength() > 0) {
			prefix += ',';
			prefix += langSetName;
		}
	}
#endif // V1_6

	if (isAddString == ModTrue) {
		prefix += ']';
	}
}

//
// FUNCTION public
// ModInvertedTermLeafNode::getSearchTermList --
//
// NOTES
//
// ARGUMENTS
//
// RETURN
// 
// EXCEPTIONS
//
void
ModInvertedTermLeafNode::getSearchTermList(
	ModInvertedQuery::SearchTermList& vecSearchTerm_,
	ModSize uiSynonymID_) const
{
	if (termString.getLength() > 0)
	{
		ModInvertedQuery::SearchTerm t;
		t.m_cstrSearchTerm = termString;
		t.m_cLanguageSet = langSet;
		t.m_eMatchMode = matchMode;
		t.m_uiSynonymID = uiSynonymID_;
		
		vecSearchTerm_.pushBack(t);
	}
}

//
// FUNCTION protected
// ModInvertedTermLeafNode::validate -- 検索語ノードの有効化
//
// NOTES
// 検索語をトークン分割関数 tokenizeMulti() により分割し、その結果から
// queryNodeForPreciseEvaluation と queryNodeForRoughEvaluation を設定
// する。
//
// トークン分割の際に引数modeを参照し、tokenizeQueryのBITがオンの時
// queryを使ったトークン分割を行ない、オフであった場合simpleQueryを使っ
// たトークン分割を行なう。
//
// また tokenizeMulti() が ModFalse を返してきた時は、検索語が short word
// であった場合となる。
//
// その場合、セットされたTokenMap の getSize() の結果が 0 であった場合
// は short word のみの検索語であったことを表わし、
// ModInvertedQuery::validateForShortWord()を呼び出して有効化を行なう。
// TokenMap の getSize() の結果が 1 以上であった場合は short word と通
// 常の検索語の混合であった場合なので、short word の部分は
// ModInvertedQuery::validateForShortWord() を呼び出し、通常の検索語の
// 部分は validateForNormalWord() を呼び出して有効化を行なう。
//
// 検索語が short word でなかった(通常の検索語)場合は、
// validateForNormalWord() 関数を呼び出し有効化を行なう。
//
// ARGUMENTS
// ModInvertedQuery& query
//		検索式内部表現オブジェクト
// ModInvertedFile* invertedFile
//		転置ファイルへのポインタ
// const Query::ValidateMode mode
//		有効化モード
// ModInvertedQuery* rQuery
//		クエリ
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedTermLeafNode::validate(ModInvertedFile* invertedFile,
								  const Query::ValidateMode mode,
								  Query* rQuery)
{
	// totalDocumentFrequencyの取得
	setTotalDocumentFrequency(rQuery->getTotalDocumentFrequency());

	// Check match mode
	matchMode = checkMatchMode(invertedFile->getIndexingType(), matchMode);

	// トークン分割結果
	ModInvertedQueryTokenizedResult* tokenizedResults = 0;

	try
	{
		// Get tokenize mode
		ModInvertedTokenizer::TokenizeMode tokenizeMode =
			getTokenizeMode(invertedFile->getIndexingType(), matchMode, mode);
		
		//
		// トークン分割
		//
		
		ModSize tokenizeResultSize;
		try {
			tokenizeResultSize = invertedFile-> tokenizer->tokenizeMulti(
				termString, tokenizeMode, tokenizedResults
#ifdef V1_6
				, langSet
#endif // V1_6
				);

		} catch (ModException& e) {
			ModErrorMessage << "tokenize failed: " << e << ModEndl;
			ModRethrow(e);
		}
		
		//
		// トークン分割の各結果を有効化し、preciseNodes/roughNodesに格納する
		//

		// 有効化結果格納用vector
		ModVector<ModInvertedQueryNode*> preciseNodes;
		ModVector<ModInvertedQueryNode*> roughNodes;

		ModInvertedQueryInternalNode* parent = 0;
		ModInvertedQueryTokenizedResult* tokenizedResult = tokenizedResults;

		// 検索語の長さ比較用テンポラリ変数
		ModSize tmpTermLength = 0;

		for (ModSize loop(0); loop < tokenizeResultSize;
			 ++loop, ++tokenizedResult) {

			//
			// 分割単位を順番に有効化する。
			//

			// 分割単位は以下の二つに分類される
			// * short word
			// * normal word

			if (tokenizedResult->isNormalWord == ModFalse) {
				//
				// 検索語が short word の場合
				//

				// ShortWordのときはtermLengthにModSizeMaxをセット
				this->termLength = ModSizeMax;

				//
				// shortWordの有効化
				//
				ModInvertedQueryNode* shortWordNode = 0;
				validateForShortWord(tokenizedResult->shortWord,
									 tokenizedResult->fromWord,
									 tokenizedResult->toWord,
									 invertedFile, shortWordNode,
									 *rQuery, mode);

				//
				// 単語境界処理
				//

				ModInvertedSmartLocationList* tokenBoundaries = 0;

				if ((tokenizedResult->locationListMap).getSize() == 0) {
					// short word のみで文字列検索の場合 -- ここでは何もしない
				}
#ifdef V1_4 // 単語単位検索
				else if (
					(tokenizedResult->locationListMap).getSize() == 1 &&
					(matchMode == ModInvertedTermExactWordMode
#ifndef MOD_DIST // APPMODE
					 || matchMode == ModInvertedTermApproximateMode
#endif
						) &&
					invertedFile->indexingType == ModInvertedFileDualIndexing
					) {
					// short word のみで単語単位検索（head/tail以外）の場合
					// 単語境界を得る

					if (shortWordNode != QueryNode::emptySetNode) {

						ModInvertedLocationListMap::Iterator
							p(tokenizedResult->locationListMap.begin());
						if (invertedFile->tokenizer->getTokenLength((*p).first)
							!= 0) {
							ModErrorMessage << "no boundaries for short word"
											<< ModEndl;
							ModThrowInvertedFileError(
								ModInvertedErrorInternal);
						}

						tokenBoundaries =
							new ModInvertedSmartLocationList((*p).second);
					} else {
						if (tokenBoundaries != 0) {
							delete tokenBoundaries;
						}
					}
				}
#endif	// V1_4
				else {
					; ModAssert(0);
				}

				//
				// priciseNodesに格納する
				//

				// validateForNormalWordの結果 emptySetNodeがセットされて
				// いる可能性がある。emptySetNode の場合は preciseNodes
				// にセットしない
				if (shortWordNode != QueryNode::emptySetNode) {
#ifdef V1_4 // 単語単位検索
					if (getMatchMode() != ModInvertedTermStringMode) {
						// 単語単位検索のための有効化
						validateForWordMode(
							shortWordNode,
							invertedFile->tokenizer->getTokenLength(
								tokenizedResult->shortWord),
							*rQuery, invertedFile, mode,
							tokenBoundaries);
					}
#endif
					preciseNodes.pushBack(shortWordNode);
				}
				queryNodeForRoughEvaluation = 0; // Roughは作らない
				
				//
				// short word の場合の処理が終わった
				//

			} else {
				//
				// Normal word の場合
				//

				//
				// 分割単位長をチェック
				//

				// すべての検索語の長さを比較し、すべて同じなら検索語の長さ、
				// 違えば0、shortWordが混じっていればModSizeMaxをセットする
				// (shortWordがひとつでもあればTermLengthはModSizeMaxになるので以下の比較不要)
				if (this->termLength != ModSizeMax) {

					// 有効化済みのトークンには、shortWordがない
					
					if (tmpTermLength == 0) {
						
						// 最初のnormalWordだった
						
						// 最初の検索語の長さを取り、長さの比較の基準とする
						tmpTermLength = tokenizedResult->tokenizedEnd;
						this->termLength = tmpTermLength;
					}
					else if (tmpTermLength != ModSizeMax){

						// 最初のnormalWordではない
						// また、今までの有効化済み分割単位長は基準と同じだった
						
						if (tmpTermLength == tokenizedResult->tokenizedEnd) {
							
							// 長さが基準と同じ
							
							// [YET] 同じ値を再設定している…
							this->termLength = tokenizedResult->tokenizedEnd;
							
						} else {
							
							// 長さが基準と異なる
							
							// TermLengthに0を格納
							this->termLength = 0;
							// 以降比較不要なのでtmpTermLengthにModSizeMaxを設定しておく
							tmpTermLength = ModSizeMax;
						}
					}
				}

				//
				// 不要な有効化はしない
				//

				if ((tokenizedResult->locationListMap).getSize() == 0
#ifdef V1_4 // 単語単位検索
					||
					((tokenizedResult->locationListMap).getSize() == 1 &&
					 (matchMode == ModInvertedTermExactWordMode
#ifndef MOD_DIST // APPMODE
					  || matchMode == ModInvertedTermApproximateMode
#endif
						 ) &&
					 invertedFile->indexingType == ModInvertedFileDualIndexing)
#endif
					) {
					// 抽出された n-gram がないときは何もしない
					continue;
				}

				//
				// normal wordの有効化
				//
				
				ModInvertedQueryNode* tmpPreciseNode = 0;
				ModInvertedQueryNode* tmpRoughNode = 0;
				ModSize dummy = 0;

				ModInvertedSmartLocationList* tokenBoundaries = 0;

				// tokenizedResult->tokenizedEnd には normalWord の長さが
				// セットされている

				// - 複数の異表記がある場合、共通の n-gram をラフにセット
				//	 するが、共通の n-gram が見つかりやすいように、
				//	 分割結果の n-gram を全てを使用するモードとする
				validateForNormalWord(
					*rQuery, invertedFile,
					(tokenizeResultSize <= 1) ? mode :
					(mode&(~(ModInvertedQuery::tokenMapToRough|ModInvertedQuery::bestPathToRough))),
					tokenizedResult->locationListMap,
					tokenizedResult->tokenizedEnd,
					tmpPreciseNode, tmpRoughNode, dummy,
					tokenBoundaries);

				//
				// priciseNodes/roughNodesに格納する
				//

				// validateForNormalWordの結果 emptySetNodeがセットされて
				// いる可能性がある。emptySetNode の場合は preciseNodes
				// にセットしない
				if (tmpPreciseNode != QueryNode::emptySetNode) {
#ifdef V1_4 // 単語単位検索
					if (getMatchMode() != ModInvertedTermStringMode &&
						invertedFile->indexingType !=
						ModInvertedFileWordIndexing) {
						// 単語単位検索のための有効化
						validateForWordMode(tmpPreciseNode,
											tokenizedResult->tokenizedEnd,
											*rQuery, invertedFile, mode,
											tokenBoundaries);
					}
#endif V1_4 // 単語単位検索

					// preciseNodesに有効化結果のpreciseノードを格納
					preciseNodes.pushBack(tmpPreciseNode);

					// roughNodesに有効化結果のラフノードを格納
					if (tmpRoughNode != 0) {
						// tmpRoughNodeが0の場合はroughNodesには登録しない
						roughNodes.pushBack(tmpRoughNode);
					}
				} else {
					if (tokenBoundaries != 0) {
						delete tokenBoundaries;
					}
				}
				//
				// Normal word の場合の処理が終わった
				//
			}
			
			// 次の分割単位へ
		}
		// 全ての分割単位の有効化が終わった
		
		
		//
		// queryNodeForPreciseEvaluation/queryNodeForRoughEvaluation を設定
		//
		
		// preciseNodes用iterator
		ModVector<ModInvertedQueryNode*>::Iterator iteratorForPreciseNodes
			= preciseNodes.begin();
		// roughNodes用iterator
		ModVector<ModInvertedQueryNode*>::Iterator iteratorForRoughNodes
			= roughNodes.begin();

		if (preciseNodes.getSize() < 1) {
			setQueryNodeForPreciseEvaluation(const_cast<ModInvertedQueryNode*>(QueryNode::emptySetNode));
			queryNodeForRoughEvaluation = 0;
			// preciseNodesが1より小さい→ empty
		} else if (preciseNodes.getSize() == 1) {
			// preciseNodeが1つ以下
			setQueryNodeForPreciseEvaluation(*iteratorForPreciseNodes);
			if ((roughNodes.getSize() > 0) && (*iteratorForRoughNodes != 0)) {
				setQueryNodeForRoughEvaluation(*iteratorForRoughNodes);
			} else {
				queryNodeForRoughEvaluation = 0;
			}
		} else {
			// アトミックORを生成する
			//		ブーリアン検索→ブーリアンのOR
			//		ランキング検索→アトミックOR
			//
			// トークン分割で複数の結果が返された場合
			// termLeafNodeのqueryNodeForPreciseEvaluationは
			//
			// OrNode ---- 分割結果1
			//		 |-- 分割結果2
			//		 |-- 分割結果3
			//		 :		 :
			//		 |-- 分割結果n
			//
			// とする。
			//
			// また、ラフノードは分割結果の各ラフノード(Andノード)の子ノード
			// のうち、すべてに共通なもののAndとする
			//

			// preciseノード用orNode
			ModInvertedOperatorOrNode* orNode = 0;

			// 検索タイプに応じたorNode生成
			orNode = createOrNode(*rQuery, invertedFile, mode);

			//
			// preciseノードの生成
			//
			// orNodeにpreciseNodesに格納していたQueryNodeをinsertChild
			while (iteratorForPreciseNodes != preciseNodes.end()) {
				// EmptySetNode以外のものをpushBack
				orNode ->insertChild(*iteratorForPreciseNodes);
				++iteratorForPreciseNodes;
			}

			ModVector<ModInvertedQueryNode*>* tmpOrChildren;
			tmpOrChildren = orNode->getChildren();
			if (tmpOrChildren->getSize() < 1) {
				ModAssert(0);
			} else {
				// orNodeをtermLeafNodeのqueryNodeForPreciseEvaluationにセット

				setQueryNodeForPreciseEvaluation(orNode);
			}

			//
			// roughノードの生成
			//
			if (roughNodes.getSize() >= 2) {
				createRoughNodeForTermLeafNode2(*rQuery,
												iteratorForRoughNodes,
												roughNodes.end(),
												roughNodes.getSize(),
												mode);
			}
			else if (roughNodes.getSize() == 1) {
				setQueryNodeForRoughEvaluation(*iteratorForRoughNodes);
			} else {
				queryNodeForRoughEvaluation = 0;
			}
		}

		//
		// 後処理
		//

		// tokenizedResults の削除
		try {
			delete[] tokenizedResults;
			tokenizedResults = 0;
		} catch (ModException& e) {
			ModErrorMessage << "delete failed: " << e << ModEndl;
			ModRethrow(e);
		}

	} catch (ModException& e) {
		ModErrorMessage << "validate failed: " << e << ModEndl;
		delete[] tokenizedResults;
		ModRethrow(e);
	} catch (...) {
/* purecov:begin deadcode */
		delete[] tokenizedResults;
#ifdef SYD_INVERTED
		_SYDNEY_RETHROW;
#else
		ModUnexpectedThrow(ModModuleInvertedFile);
#endif
/* purecov:end */
	}

#ifdef DEBUG
	{
		ModUnicodeString s;
		queryNodeForPreciseEvaluation->getQueryString(s);
		ModDebugMessage << "PreciseNode :" << s << ModEndl;

		if (queryNodeForRoughEvaluation != 0) {
			ModUnicodeString s;
			queryNodeForRoughEvaluation->getQueryString(s);
			ModDebugMessage << "RoughNode :" << s << ModEndl;
		} else {
			ModDebugMessage << "RoughNode : 0" << ModEndl;
		}
	}
#endif
}

#ifdef V1_4 // 単語単位検索
//
// FUNCTION private
// ModInvertedTermLeafNode::validateWordMode -- 単語単位検索のための有効化
//
// NOTES
// 単語単位検索のための有効化。MatchModeがセットされている場合にコールされる
//
// ARGUMENTS
// ModInvertedQueryNode*& node
//		有効化するノード
// const ModSize tokenLength
//		有効化するノードの元になったtokenの長さ。"末尾が単語境界と一致" の
//		時に作成するOrederedDistanceの距離条件に使う
// ModInvertedQuery& query
//		検索式内部表現オブジェクト
// ModInvertedFile* invertedFile
//		転置ファイルへのポインタ
// validateMode mode
//		roughNode 作成 mode（有効化のモード）
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedTermLeafNode::validateForWordMode(
	ModInvertedQueryNode*& node,
	const ModSize tokenLength,
	ModInvertedQuery& query,
	ModInvertedFile* invertedFile,
	const Query::ValidateMode mode,
	ModInvertedSmartLocationList* emptyLocationList)
{
	// 単語単位索引はすべて ModInvertedOperatorWordNode で処理する
	
	ModInvertedOperatorWordNode* wordNode;

	wordNode = new ModInvertedOperatorWordNode(
		node, tokenLength, getMatchMode(), query, invertedFile, mode,
		firstStepResult->getType(),emptyLocationList);

	node = static_cast<InternalNode*>(wordNode);

	if ((mode&Query::rankingMode) != 0) {
		// TermLeafNodeにスコア計算器があればそれをセット。
		// なければQueryNodeのデフォルトのものをセットする。
		if (this->scoreCalculator != 0) {
			node->setScoreCalculator(this->scoreCalculator->duplicate());
		}
		else {
			// QueryNodeには必ずデフォルトの計算機をセットするように
			// なったので、ここではduplicateだけ
			ModInvertedRankingScoreCalculator* calculator =
				query.getDefaultScoreCalculator();
			; ModAssert(calculator != 0);
			node->setScoreCalculator(calculator->duplicate());
		}

		node->getScoreCalculator()->setDocumentLengthFile(
			invertedFile->getDocumentLengthFile());
		node->getScoreCalculator()->setAverageDocumentLength(
			query.getAverageDocumentLength());
	}
}
#endif // V1_4 単語単位検索

//
// FUNCTION private
// ModInvertedTermLeafNode::validateForNormalWord -- 通常の検索語ノードの有効化(validateの下請け関数)
//
// NOTES
// 検索語のshort word でない部分の有効化処理を行なう。
//
// 有効化の結果は queryNodeForPreciseEvaluation 用 OrderedDistanceNode
// を引数 preciseNode にセットし、queryNodeForRoughEvaluation 用
// OPeratorAndNode は引数 roughNode にセットする。
//
// roughNode を作成する時に引数modeを参照し、tokenMapToRough のBITがオ
// ンの場合 引数 tokenMap で与えられた全てのtokenを用いて
// queryNodeForRoughEvaluationをセットし、オフの場合getBestPath()の結
// 果をセットする。
//
// ARGUMENTS
// ModInvertedQuery& query
//		検索式内部表現オブジェクト
// ModInvertedFile* invertedFile
//		転置ファイルへのポインタ
// validateMode mode
//		roughNode 作成 mode（有効化のモード）
// ModInvertedLocationListMap tokenMap
//		検索語分割情報
// ModSize length
//		通常検索語（normal word）の長さ
// ModInvertedQueryNode*& preciseNode
//		queryNodeForPreciseEvaluation 用ノードポインタ
// ModInvertedQueryNode*& roughNode
//		queryNodeForRoughEvaluation 用ノードポインタ
// ModSize& location
//		preciseNode の top node の位置情報
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedTermLeafNode::validateForNormalWord(
	ModInvertedQuery& query,
	ModInvertedFile* invertedFile,
	const Query::ValidateMode mode,
	const ModInvertedLocationListMap& tokenMap,
	ModSize length,
	ModInvertedQueryNode*& preciseNode,
	ModInvertedQueryNode*& roughNode,
	ModSize& location,
	ModInvertedSmartLocationList*& newEmptyLocationList
	)
{
	// 索引語トークンを出現位置と共に並べる
	ModSize tokenLength;
	LocationQueryPairVector allLeafVector;
	ModInvertedQueryNode* tmpNode = 0;
	LocationQueryPair leaf1;
	LocationQueryPair leaf2;
	ModSize tailLocation = ModUInt32Max;

	//
	// WordHead処理
	//
	if (getMatchMode() == ModInvertedTermWordHead &&
		invertedFile->indexingType == ModInvertedFileWordIndexing)
	{
		// 単語単位索引でwordheadの場合、位置が最後のノードを
		// validateForShortWord の結果と置き換える

		//
		// 一番最後のtokenを調べる
		//
		
		ModUnicodeString term;
		for (ModInvertedLocationListMap::ConstIterator p0(tokenMap.begin());
			 p0 != tokenMap.end(); ++p0) {

			//
			// 各トークンを順番に調べる
			//
			
#ifdef INV_TOK_USETOKEN
			const ModUnicodeString& tokenString = (*p0).first.getString();
#else
			const ModUnicodeString& tokenString = (*p0).first;
#endif
			const ModInvertedSmartLocationList& locationList = (*p0).second;

			ModAutoPointer<ModInvertedLocationListIterator>
				p1(locationList.begin());
			while (p1->isEnd() == ModFalse) {

				//
				// 各トークンの出現位置を順番に調べる
				//
				
				if (tailLocation == ModUInt32Max ||
					p1->getLocation() > tailLocation)
				{
					// 一回目のチェックか、
					// 今までの最後尾よりさらに後ろのトークンが見つかったか。
					
					tailLocation = p1->getLocation();
					term = tokenString;
				}

				// 次の位置へ
				p1->next();
			}
			// 次のトークンへ
		}
		// 最後のトークンとその位置を調べた。

		//
		// short wordで有効化する
		//
			
		// validateForShortWord で AtomicOrNode または OperatorOrNode に変換する

		// [NOTE] 文字列の比較はNO PADで行われている。
		//  従って、term < term+0x01 という順番なので、
		//  term から term+0xFFFF の範囲で有効化すれば良い。
		//  もしPAD SPACEで比較されるなら、
		//  term+0x01 < term という順番になるので、
		//  term+0x01 から term+0xFFFFの範囲で有効化する必要がある。
		//  LeafPage::Area::Less, BtreePage::Entry を参照。
		
		// ModUnicodeString from = term;
		// from += ModUnicodeChar(1);
		ModUnicodeString to = term;
		to += ModUnicodeChar(0xffff);		// Unicodeに0xffffはない
		ModInvertedQueryNode* newNode = 0;
		validateForShortWord(term, term, to, invertedFile,
							 newNode, query, mode);

		//
		// 得られたノードを設定する
		//
		
		leaf2.first = tailLocation;
		leaf2.second = newNode;
		
		// length も短くなる
		length -= 1;
	}
	// WordHead処理が終わった
	

	//
	// トークンが転置索引に存在することを確認
	//
	ModBoolean bFirstCheck = ModTrue;
	ModBoolean bNolocation = ModFalse;
	for (ModInvertedLocationListMap::ConstIterator p0(tokenMap.begin());
		 p0 != tokenMap.end(); ++p0) {

		//
		// 各トークンを順番に調べる
		//
		
#ifdef INV_TOK_USETOKEN
		const ModUnicodeString& tokenString = (*p0).first.getString();
#else
		const ModUnicodeString& tokenString = (*p0).first;
#endif

		//
		// 空文字列の処理
		//

#ifdef V1_4 // 単語単位検索
		if (invertedFile->tokenizer->getTokenLength(tokenString) == 0) {
			if (matchMode != ModInvertedTermExactWordMode
#ifndef MOD_DIST // APPMODE
				&& matchMode != ModInvertedTermApproximateMode
#endif
				) {
				// exactWordMode以外で空文字列があるのはおかしい
				; ModAssert(0);
				return;
			}

			newEmptyLocationList =
				new ModInvertedSmartLocationList((*p0).second);

			continue;
		}
#endif // V1_4 単語単位検索


		//
		// トークンを転置索引から取得
		//

		const ModInvertedSmartLocationList& locationList = (*p0).second;
#ifdef SYD_INVERTED
		ModInvertedList* invertedList = invertedFile->getInvertedList();
#else
		ModInvertedList* invertedList = new ModInvertedList();
#endif
		if (invertedFile->getInvertedList(tokenString,
										  *invertedList,
										  ModInvertedListSearchMode)
			== ModFalse) {

			// 転置索引になかった
			
			delete invertedList;

			if (tailLocation != ModUInt32Max)
			{
				// WordHead処理している
				if (locationList.getSize() == 1 &&
					locationList[0] == tailLocation)
				{
					// 最後のトークンなので転置索引になくてもよい
					continue;
				}
			}
			
			// tokenStringが転置ファイルになかったため
			// SimpleTokenLeafNodeは取得できない。
			// それで空集合のNodeをセットする
			preciseNode = const_cast<ModInvertedQueryNode*>(
				QueryNode::emptySetNode);
			roughNode = 0;
			return;
		}

		tokenLength = invertedFile->tokenizer->getTokenLength(tokenString);

		//
		// 転置索引は位置情報を格納しているか？
		// pathを作成する際に利用する。
		//
		if (bFirstCheck == ModTrue)
		{
			; ModAssert(invertedList != 0);
			bNolocation = invertedList->isNolocation();
			bFirstCheck = ModFalse;
		}

		//
		// シンプルトークンノードを作成
		//

		// map登録用のキー文字列取得
		ModUnicodeString mapKey;
		prefixString(mapKey, ModTrue, ModTrue);
		mapKey += '(';
		mapKey += tokenString;
		mapKey += ')';

		if (query.getSimpleTokenNode(tokenString, tokenLength,
									 mapKey, invertedList,
									 tmpNode, mode) == ModTrue) {
			delete invertedList;
		}

		ModAutoPointer<ModInvertedLocationListIterator>
			p1(locationList.begin());
		while (p1->isEnd() == ModFalse) {

			//
			// 各位置を順番にベクタに登録
			//
			
			if (p1->getLocation() != tailLocation) {
				
				// WordHead処理していた場合、
				// 最後の位置のトークンは追加しない。
				
				leaf1.first = p1->getLocation();
				leaf1.second = tmpNode;
				allLeafVector.pushBack(leaf1);
			}

			// 次の位置へ
			p1->next();
		}

		// 次のトークンへ
	}
	// 全てのトークンのチェックが終わった

	
	//
	// 最適の索引語列を得る。
	//

	// 位置の順に並べ替える
	ModSort(allLeafVector.begin(), allLeafVector.end(), former);
			
	LocationPathPairVector pathVector;

	Path tmpPath;
	Path* path = &tmpPath;

	if (allLeafVector.getSize())
	{
		if (bNolocation == ModFalse)
		{
			// 転置索引に位置情報が格納されている場合
			
			if (getBestPath(allLeafVector.begin(), allLeafVector.end(),
							1, length, pathVector) == ModFalse) {
				// bestPathの取得に失敗
				ModErrorMessage << "getBestPath failed" << ModEndl;
				ModThrowInvertedFileError(ModInvertedErrorQueryValidateFail);
			}
		}
		else
		{
			// 転置索引に位置情報が格納されていない場合

			// 位置の付き合わせができないので、最適なパスを取得する意味がない。
			// 単純に相異なるトークンのANDで代用する。
			// [NOTE] 正確に言えばパスではないが、他と言葉の使い方と合わせる。
			
			// 位置情報が格納されていない場合、WordHead検索は未サポート。
			// 参照 Inverted::OpenOption::convertContains
			// したがってWordHead処理でlengthが変わることは考慮しなくてよい。
			; ModAssert(getMatchMode() != ModInvertedTermWordHead);

			// [NOTE] 直接pathを設定しても良いが、erasePathVectorを使うので、
			//  一度pathVectorに設定する。
			
			if (getSimplePath(allLeafVector, pathVector) == ModFalse)
			{
				ModErrorMessage << "getSimplePath failed" << ModEndl;
				ModThrowInvertedFileError(ModInvertedErrorQueryValidateFail);
			}
		}
		
		// 先頭位置をpathに設定する
		path = findPath(pathVector, 1);
	}

	if (tailLocation != ModUInt32Max)
	{
		// WordHead処理をしている
		
		// shortWordを最後に加える
		path->pushBack(leaf2);
 	}


	//
	// preciseNodeの設定
	//
 
	// IDF 順に並べ替える
	// [YET] IDF順に並べるのはTFが少ないことを期待して？
	ModSort(path->begin(), path->end(), lessFrequent);

	if (path->getSize() > 1)
	{
		//
		// トークンが二つ以上
		//

		if (bNolocation == ModFalse)
		{
			// 位置情報が格納されている場合
			
			// 各トークンの位置関係を確認する
			
			// OrderedDistanceNodeの生成
			//		ブーリアン検索の時はブーリアン用
			//		ランキング検索の時はランキング用のノードを生成する
			ModInvertedOrderedDistanceNode* node
				= createOrderedDistanceNode(query, invertedFile, mode);
			LocationQueryPairVector::Iterator p2 = path->begin();
			for (; p2 != path->end(); ++p2)
			{
				if (((*p2).second)->getType() == simpleTokenLeafNode)
				{
					static_cast<SimpleTokenLeafNode*>((*p2).second)->setIterator();
				}
				node->insertChild((*p2).first, (*p2).second);
			}
			
			preciseNode = static_cast<InternalNode*>(node);
		}
		else
		{
			// 位置情報が格納されていない

			// 位置関係を確認しないのでただのAND検索

			// [NOTE] 関数名と異なる用途で使っている。
			preciseNode = createRoughNodeForTermLeafNode(query, mode);
			
			LocationQueryPairVector::Iterator p2 = path->begin();
			for (; p2 != path->end(); ++p2)
			{
				//
				// 各ノードをANDノードの子ノードに追加する
				//

				ModInvertedQueryNode* pQueryNode = (*p2).second;
				// [YET] SimpleTokenLeafNode以外の時の処理は？
				if (pQueryNode->getType() == simpleTokenLeafNode)
				{
					setSimpleTokenLeafNodeForNormalWord(
						pQueryNode, query, invertedFile, mode);
				}
#ifdef CC_SUN4_2
				static_cast<AndNode*&>(preciseNode)
					->insertChild(pQueryNode);
#else
				static_cast<AndNode*>(preciseNode)
					->insertChild(pQueryNode);
#endif
			}
		}
	}
	else
	{
		//
		// トークンが一つ
		// トークン間の位置関係を確認する必要がない
		//
		
		ModInvertedQueryNode* pQueryNode = (*path->begin()).second;
		if (pQueryNode->getType() == simpleTokenLeafNode)
		{
			setSimpleTokenLeafNodeForNormalWord(
				pQueryNode, query, invertedFile, mode);
		}
		preciseNode = pQueryNode;
	}
	

	//
	// roughNodeの設定
	//
	
	if (!(getMatchMode() == ModInvertedTermWordHead &&
		  invertedFile->indexingType == ModInvertedFileWordIndexing))
	{
		// 単語単位索引以外、または、wordhead以外の場合

		// roughEvaluation では、位置のつき合わせをしないが、
		// 検索語に含まれる索引語を全部使って #and を取る。
		
		ModSize s = path->getSize();
		if (s == 1)
		{
			// ひとつの token で構成されている場合
			// SimpleTokenNode を1つ設定しておく

			roughNode = preciseNode;

		} else if (s > 1) {

			// 二つ以上のトークンで構成されている場合
						
			if ((mode & ModInvertedQuery::tokenMapToRough) != 0) {

				// トークンマップを使う場合 ※NOTESを参照のこと
				
				//トークン分割で得られたtokenのうち前後のtokenと出現頻度を
				//比較して、頻度の低い場合だけrough pointerにセットする
				tokenMapToRoughPointer(tokenMap, allLeafVector, *path,
									   roughNode, query, preciseNode, mode,
									   invertedFile);
				
			}
			else if ((mode & ModInvertedQuery::bestPathToRough) != 0 ||
					 bNolocation == ModTrue)
			{
				// ベストパスを使う場合

				// [NOTE] 位置情報がない場合もベストパスを使う。
				//  位置情報がない場合のベストパスは、基本的には全てのN-gramを
				//  使っているため。
				//  参照 getSimplePath(), validate()
				
				// getBestPathの結果をRoughPointerにセットする
				// ラフノードの作成(検索タイプに合わせてANDノード作成)
				roughNode = createRoughNodeForTermLeafNode(query, mode);
				LocationQueryPairVector::Iterator p2 = path->begin();
				for (; p2 != path->end(); ++p2) {
					//
					// 各ノードをANDノードの子ノードに追加する
					//

					// [YET] setIterator()は不要？
#ifdef CC_SUN4_2
					static_cast<AndNode*&>(roughNode)
						->insertChild((*p2).second);
#else
					static_cast<AndNode*>(roughNode)
						->insertChild((*p2).second);
#endif
				}
			} else {

				// 全てのトークンを使う場合
				
				// トークン分割で得られた全てのtokenをrough pointerにセットする
				// ラフノードの作成(検索タイプに合わせてANDノード作成)
				roughNode = createRoughNodeForTermLeafNode(query, mode);

				for (ModInvertedLocationListMap::ConstIterator
						 p0(tokenMap.begin());
					 p0 != tokenMap.end(); ++p0) {

					//
					// 各トークンを取得
					//
#ifdef INV_TOK_USETOKEN
					const ModUnicodeString& tokenString
						= (*p0).first.getString();
#else
					const ModUnicodeString& tokenString
						= (*p0).first;
#endif
					tokenLength
						= invertedFile->tokenizer->getTokenLength((*p0).first);
					if (tokenLength == 0) {
						continue;
					}
					// [YET] 宣言の場所と使い始める場所が離れている…
					ModInvertedQueryNode* node = 0;

					//
					// トークンを転置索引から取得
					//
#ifdef SYD_INVERTED
					ModInvertedList* invertedList
						= invertedFile->getInvertedList();
#else
					ModInvertedList* invertedList
						= new ModInvertedList();
#endif
					if (invertedFile->getInvertedList(tokenString,
													  *invertedList,
													  ModInvertedListSearchMode)
						== ModFalse) {
						delete invertedList;
						// SimpleTokenLeafNodeが取得できないのは異常事態
						ModAssert(0);
					}

					//
					// シンプルトークンノードを作成
					//

					// map登録用のキー文字列取得
					ModUnicodeString mapKey;
					prefixString(mapKey, ModTrue, ModTrue);
					mapKey += '(';
					mapKey += tokenString;
					mapKey += ')';

					if (query.getSimpleTokenNode(
							tokenString,
							invertedFile->tokenizer->getTokenLength(
								tokenString),
							mapKey, invertedList, node, mode) == ModTrue) {
						delete invertedList;
					}

					//
					// ラフノードに挿入
					//
					static_cast<SimpleTokenLeafNode*>(node)->setIterator();
#ifndef CC_SUN4_2
					static_cast<AndNode*>(roughNode)->insertChild(node);
#else
					static_cast<AndNode*&>(roughNode)->insertChild(node);
#endif
					
					// 次のトークンへ
				}
				// 全てのトークンを取得した

				// queryNodeForRoughEvaluationの内容をsortしておく
				roughNode->sortChildren(mode);

				// 全てのトークンを使う場合の処理が終わった
			}

			// 二つ以上のトークンで構成されている場合の処理が終わった
		}

		// ラフノードのラフポインタを自分自身にしておく
		roughNode->setQueryNodeForRoughEvaluation(roughNode);

		// 単語単位索引以外、または、wordhead以外の場合の処理が終わった
	}
	// roughNodeの設定が終わった

	
	//
	// 後処理
	//

	// pathVector の後始末
	// [NOTE] pathVectorの先頭要素がpathに設定されて使われているので、
	//  後処理は最後に実行する必要がある。
	ModInvertedTermLeafNode::erasePathVector(pathVector);
}

//
// FUNCTION protected
// ModInvertedTermLeafNode::validateForShortWord -- 検索語が short word だった場合の有効化関数
//
// NOTES
// 引数 term に対応する SimpleTokenLeafNode および from から to までの
// 文字列に対応する SimpleTokenLeafNode を作りそれらをORノード(ORノード
// は検索タイプにしたがいModInvertedOperatorOrNodeまたは
//	ModInvertedRankingOrNodeまたはModInvertedAtomicOrNodeを作成する)の
// 子ノードとする木構造としそれを返す。ただし SimpleTokenLeafNode が1つ
// しか存在しない場合はORノードはなくSimpleTokenLeafNodeがひとつある構
// 造を返す。さらに form , to の範囲に転置リストが存在しない場合は空集
// 合を示すノードを返す。
//
// ARGUMENTS
// const ModUnicodeString& term
//		short word に対応する文字列
// const ModUnicodeString& from
//		short word に対応する文字列範囲の開始を示す
// const ModUnicodeString& to
//		short word に対応する文字列範囲の終了の次を示す
// ModInvertedFile* invertedFile
//		転置ファイル
// ModInvertedQueryNode*& node
//		生成したノード（結果格納用）
// ModInvertedQuery& query
//		検索式内部表現オブジェクト
// validateMode mode
//		有効化のモード ランキングの場合に作成するOrノード
//		(アトミック/ランキング)の判断に用いる
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedTermLeafNode::validateForShortWord(const ModUnicodeString& term,
											  const ModUnicodeString& from,
											  const ModUnicodeString& to,
											  ModInvertedFile* invertedFile,
											  ModInvertedQueryNode*& node,
											  ModInvertedQuery& query,
											  const Query::ValidateMode mode)
{
	ModInvertedQueryNode* tmpNode = 0;
	ModBoolean deleteInvertedListFlag = ModFalse;
#ifdef SYD_INVERTED
	ModInvertedList* invertedList = invertedFile->getInvertedList();
#else
	ModInvertedList* invertedList = new ModInvertedList();
#endif
	ModUnicodeString invertedListKey;
	ModInvertedOperatorOrNode* orNode = 0;
	ModBoolean loop = ModTrue;

	// shortWord検索時にassert ランキング/ブーリアンに関わらず、
	// ModInvertedOperatorOrNodeを作成しているため。

	if ((mode & Query::rankingMode) != 0) {
		// ランキング検索ではアトミックORを作成する
		orNode = new ModInvertedAtomicOrNode(firstStepResult->getType());

		orNode->setTotalDocumentFrequency(query.getTotalDocumentFrequency());

		// TermLeafNodeにスコア計算器があればそれをセット。
		// なければQueryNodeのデフォルトのものをセットする。
		if (this->scoreCalculator != 0) {
			orNode->setScoreCalculator(scoreCalculator->duplicate());
		}
		else {
			// QueryNodeには必ずデフォルトの計算機をセットするように
			// なったので、ここではduplicateだけ
			ModInvertedRankingScoreCalculator* calculator =
				query.getDefaultScoreCalculator();
			; ModAssert(calculator != 0);
			orNode->setScoreCalculator(calculator->duplicate());
		}

		orNode->getScoreCalculator()->setDocumentLengthFile(
			invertedFile->getDocumentLengthFile());
		orNode->getScoreCalculator()->setAverageDocumentLength(
			query.getAverageDocumentLength());

	} else {
		// ブーリアンでもショートワード用のOrの場合はAtomicOrを使う
		orNode = new ModInvertedOperatorOrNode(firstStepResult->getType());
	}

	// term の InvertedList を取得する
	if (invertedFile->getInvertedList(term, *invertedList,
									  ModInvertedListSearchMode) == ModTrue) {
		// 出現頻度を得る
		ModSize documentFrequency = invertedList->getDocumentFrequency();
		// 出現頻度が 0 か検査する
		if (documentFrequency == 0) {
			// 出現頻度が0なので転置リストを破棄する
			delete invertedList;
		} else {
			// 出現頻度が0以外なのでSimpleTokenLeafNodeを作りにいく
			invertedListKey = invertedList->getKey();

			// map登録用のキー文字列取得
			ModUnicodeString mapKey;
			prefixString(mapKey, ModTrue, ModTrue);
			mapKey += '(';
			mapKey += invertedListKey;
			mapKey += ')';

			if (query.getSimpleTokenNode(
					invertedListKey,
					invertedFile->tokenizer->getTokenLength(invertedListKey),
					mapKey, invertedList, tmpNode, mode) == ModTrue) {
				// getSimpleTokenNode()はsimpleTokenLeafNodeMapに既に登録
				// されていたSimpleTokenLeafNodeをtmpNodeにセットして返し
				// た場合はTrueを返す。この場合invertedListをdeleteしない
				// とmemory leakを起すことになる。
				delete invertedList;
			}

			static_cast<SimpleTokenLeafNode*>(tmpNode)->setIterator();
			orNode->insertChild(tmpNode);
		}
		// 新しい InvertedList を用意
#ifdef SYD_INVERTED
		invertedList = invertedFile->getInvertedList();
#else
		invertedList = new ModInvertedList();
#endif
	}

	// まず最初の転置リスト(from より大きい最初のリスト)を取得する
	if (invertedFile->getInvertedList(from, *invertedList,
									  ModInvertedListLowerBoundMode)
		!= ModTrue) {
		// from , to で示される範囲の転置リストは存在しなかった。
		loop = ModFalse;				// while loop は skip する
		delete invertedList;			// 転置リストは破棄
	} else {
		// invertedListの取得に成功 to と比較する
		invertedListKey = invertedList->getKey();
		if (to <= invertedListKey) {
			// getInvertedListで得た転置リストはto以上である場合(範囲外)
			delete invertedList;		// 転置リストは破棄
			loop = ModFalse;			// while loop は skip する

		} else {
			// to より小さいので Or の子ノードに加える(範囲内)

			// map登録用のキー文字列取得
			ModUnicodeString mapKey;
			prefixString(mapKey, ModTrue, ModTrue);
			mapKey += '(';
			mapKey += invertedListKey;
			mapKey += ')';

			deleteInvertedListFlag = query.getSimpleTokenNode(
				invertedListKey,
				invertedFile->tokenizer->getTokenLength(invertedListKey),
				mapKey, invertedList, tmpNode, mode);

			static_cast<SimpleTokenLeafNode*>(tmpNode)->setIterator();
			orNode->insertChild(tmpNode);
		}
	}

	// 最初の SimpleTokenLeafNodeができあがる
	// 2個目以降は以下のloop内で生成
	while (loop) {
		// 次の転置リストを取得
#ifdef SYD_INVERTED
		ModInvertedList* nextInvertedList = invertedList->clone();
		if (nextInvertedList->next() != ModTrue)
#else
		ModInvertedList* nextInvertedList = new ModInvertedList();
		if (invertedList->getNextInvertedList(*nextInvertedList) != ModTrue)
#endif
		{
			if (deleteInvertedListFlag == ModTrue) {
				delete invertedList;
			}
			delete nextInvertedList;	// 転置リストは破棄
			break;						// 終了
		}
		// to と比較
		invertedListKey = nextInvertedList->getKey();
		if (to <= invertedListKey) {
			// getInvertedListで得た転置リストはto以上である場合
			if (deleteInvertedListFlag == ModTrue) {
				delete invertedList;
			}
			delete nextInvertedList;	// 転置リストは破棄
			break;						// 終了
		}
		if (deleteInvertedListFlag == ModTrue) {
			delete invertedList;
			deleteInvertedListFlag = ModFalse;
		}
		invertedList = nextInvertedList;

		// map登録用のキー文字列取得
		ModUnicodeString mapKey;
		prefixString(mapKey, ModTrue, ModTrue);
		mapKey += '(';
		mapKey += invertedListKey;
		mapKey += ')';

		deleteInvertedListFlag = query.getSimpleTokenNode(
			invertedListKey,
			invertedFile->tokenizer->getTokenLength(invertedListKey),
			mapKey, invertedList, tmpNode, mode);

		static_cast<SimpleTokenLeafNode*>(tmpNode)->setIterator();
		orNode->insertChild(tmpNode);
	}

	// Orノードへ子ノードを追加する処理は終了
	ModVector<ModInvertedQueryNode*>* children = orNode->getChildren();
	// 子ノードの数をチェック
	switch(children->getSize()) {
	case 0:
		// Or の子ノードが0
		node = const_cast<ModInvertedQueryNode*>(QueryNode::emptySetNode);
		delete orNode;
		break;
	default:
		// shortWordであるという情報をorNodeが持っているため、
		// ここのorNodeの削除をやめる
		(void)orNode->sortChildren(mode);

		// short word の長さを求めORノードにセットする
		if (invertedFile->indexingType == ModInvertedFileWordIndexing)
		{
			orNode->setShortWordLength(1);
		}
		else
		{
			orNode->setShortWordLength(
				invertedFile->tokenizer->getTokenLength(term));
		}

		orNode->setTotalDocumentFrequency(invertedFile->getDocumentFrequency());

		node = orNode;

		// ショートワードあり
		query.setShortWord();
		break;
	}
}

//
// FUNCTION private
// ModInvertedTermLeafNode::tokenMapToRoughPointer -- tokenMapの内容をTermLeafNodeのroughPointerにセット
//
// NOTES
// tokenaizeの結果であるtokenMapの内容をTermLeafNodeのrough pointerに
// セットする。ただしgetBestPathの結果選ばれなかったtokenについては、
// 前後のtokenと出現頻度を比較し、頻度が低い場合のみrough pointerにセッ
// トする。出現頻度が高い場合は候補文書を絞る効果が小さいと考えられる
// からrough pointerには含めない。
//
// ARGUMENTS
// const ModInvertedLocationListMap& tokenMap
//		tokenaize結果（参照用）
// const LocationQueryPairVector& allLeaf
//		tokenMapを位置情報にもとずいてソートしたもの（参照用）
//const LocationQueryPairVector& bestPath
//		getBestPath()の結果選らばれたtoken（参照用）
// ModInvertedQueryNode*& roughNode
//		rough pointer （結果格納用）
// ModInvertedQuery& query
//		検索式内部表現オブジェクト
// const Query::ValidateMode mode
//		評価モード
// ModInvertedQueryNode*& preciseNode
//		precise Pointer (ブーリアン/ランキング/アトミック判定のため)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedTermLeafNode::tokenMapToRoughPointer(
	const ModInvertedLocationListMap& tokenMap,
	const LocationQueryPairVector& allLeaf,
	const LocationQueryPairVector& bestPath,
	ModInvertedQueryNode*& roughNode,
	ModInvertedQuery& query,
	ModInvertedQueryNode*& preciseNode,
	const Query::ValidateMode mode,
	ModInvertedFile* file)
{
	ModSize allLeafSize = allLeaf.getSize();
	ModSize bestPathSize = bestPath.getSize();

	if (2 == allLeafSize) {
		// 2つのTokenでTermが構成されている場合1つのTokenに単純化でき
		// るかチェックする
		if (allLeaf[0].second == allLeaf[1].second) {
			// QueryNodeへのポインタが同じなので1のTokenに単純化できる
			roughNode = allLeaf[0].second;
			static_cast<SimpleTokenLeafNode* const>(
				allLeaf[0].second)->setIterator();
			return;					 // 単純化できたので終了
		} else {
			// Tokenが2つの場合は出現頻度を比較せず2つともrough
			// pointerに入れる

			// ラフノードの作成(検索タイプに合わせてANDノード作成)
			roughNode = createRoughNodeForTermLeafNode(query, mode);

#ifndef CC_SUN4_2
			AndNode* andNode = static_cast<AndNode*>(roughNode);
#else
			AndNode* andNode = static_cast<AndNode*&>(roughNode);
#endif
			andNode->insertChild(allLeaf[0].second);
			andNode->insertChild(allLeaf[1].second);

			static_cast<SimpleTokenLeafNode* const>(
				allLeaf[0].second)->setIterator();
			static_cast<SimpleTokenLeafNode* const>(
				allLeaf[1].second)->setIterator();
			roughNode->sortChildren(mode);
			return;
		}
	}

	ModVector<ModSize>* selectedPath = 0;

	LocationQueryPairVector::ConstIterator ap = allLeaf.begin();
	LocationQueryPairVector::ConstIterator aend = allLeaf.end();

	// 位置情報ModSizeMax からselectedPathの添字(index)を求められるVector
	//		サイズは最後のTokenの位置情報，初期値は ModSizeMax
	ModVector<ModSize> indexTable(allLeaf.back().first, ModSizeMax);
	// indexMap
	ModSize index = 0;
	while (ap != aend) {
		indexTable[(*ap).first-1] = index; // 位置情報を-1しておく
		++index;
		++ap;
	}

	if (allLeafSize == bestPathSize) {
		// allLeaf と bestPath のサイズが同じ場合はselectedPathの内容はModSizeMaxで
		// 初期化し、出現頻度を比較処理はしないい
		selectedPath = new ModVector<ModSize>(allLeafSize, ModSizeMax);
	} else {
		// allLeafと同じサイズで内容が0のVectorを用意する
		selectedPath = new ModVector<ModSize>(allLeafSize, 0);

		// selectedPath を設定する(選択されたTokenに0以外の値をセットする
		ModSize i;
		for (i = 0; i < bestPathSize; ++i) {
			(*selectedPath)[indexTable[bestPath[i].first-1]]
				= bestPath[i].first;
		}

		// selectedPathを使って選択外のTokenの前後Tokenの出現頻度を見て頻度が
		// 低い場合だけselectedPathにセットする
		for (i = 0; i < allLeafSize; ++i) { // iはallLeafのindex
			if ((*selectedPath)[indexTable[allLeaf[i].first-1]] == 0) {
				// 選択外のTokenを発見
				int counter = 0;
				// getBeatPathで選ばれなかったToken前後の出現頻度と比較し
				// てrough pointerに入れるかどうか判断する
				if (i != 0) {			 // 先頭以外か？
					// 前のTokenと出現頻度を比較
					if (allLeaf[i].second->estimateDocumentFrequency()
						< allLeaf[i-1].second->estimateDocumentFrequency()) {
						++counter;			// 出現頻度が低かった
					}
				} else {
					++counter;
				}
				if (i != (allLeafSize-1)) { // 末尾以外か？
					// 後のTokenと出現頻度を比較
					if (allLeaf[i].second->estimateDocumentFrequency()
						< allLeaf[i+1].second->estimateDocumentFrequency()) {
						++counter;
					}
				} else {
					++counter;				// 出現頻度が低かった
				}
				if (counter == 2) {
					// 前後よりも出現頻度が低かったのでroughに加えることにする
					(*selectedPath)[indexTable[allLeaf[i].first-1]]
						= allLeaf[i].first;
				}
			}
		}
	}

	// selectedPath の中に同じTokenが含まれているかチェックする
	ModInvertedLocationListMap::ConstIterator p0(tokenMap.begin());
	ModInvertedLocationListMap::ConstIterator end0(tokenMap.end());
	while (p0 != end0) {
		const ModInvertedSmartLocationList& locationList = (*p0).second;
#ifdef V1_4 // 単語単位検索
		// 単語単位検索のexactWordModeの場合tokenMapに単語境界(空文字)
		// も含まれる。これはここでは無視する
		if (file->tokenizer->getTokenLength((*p0).first) == 0) {
			p0++;
			continue;
		}
#endif // V1_4 単語単位検索
		if (locationList.getSize() > 1) {
			ModAutoPointer<ModInvertedLocationListIterator>
				p1(locationList.begin());
			p1->next();					 // 2番目のデーターから使う
			while (p1->isEnd() == ModFalse) {
				// 同じTokenは使わない
				(*selectedPath)[indexTable[p1->getLocation() - 1]] = 0;
				p1->next();
			}
		}
		++p0;
	}

	// selectedPathからrough pointerを作る

	// ラフノードの作成(検索タイプに合わせてANDノード作成)
	roughNode = createRoughNodeForTermLeafNode(query, mode);

	for (ModSize i(0); i < allLeafSize; ++i) {
		if ((*selectedPath)[i] != 0) {
#ifdef CC_SUN4_2
			static_cast<AndNode*&>(roughNode)->insertChild(allLeaf[i].second);
#else
			static_cast<AndNode*>(roughNode)->insertChild(allLeaf[i].second);
#endif
			static_cast<SimpleTokenLeafNode* const>(
				allLeaf[i].second)->setIterator();
		}
	}
	roughNode->sortChildren(mode);
	delete selectedPath;
}

//
// FUNCTION private
// ModInvertedTermLeafNode::getBestPath -- 最適の索引語列を得る
//
// NOTES
// 最適の索引語列を得る。(validate() の補助関数)
// 最小被覆の内、構成トークンの文書頻度がなるべく小さくなるように選ぶ。
// (例) 目標補正量 − tokenize → {目標, 標補, 補正, 正量}
//					 − getBestPath → (目標, 標補, 正量)
// この例の場合、最小被覆は (目標, 標補, 正量) と (目標, 補正, 正量) の
// ２つで、標補の方が補正より文書頻度が小さい (= あまり出現しない) ので、
// (目標, 標補, 正量) を選ぶ。
//
//	長い文字列の場合、再起呼び出しによりStackOverflowになるので、
//	leafIteratorとendIteratorの長さが31?より大きかったら、DFとか調べずに
//	ただ、先頭から索引語長分の重なりがないようにpathVectorにpushする。
//	長い文字列の場合、コストをかけて最適なパスを見つけるより早いはず。
//
// ARGUMENTS
// LocationQueryPairVector::Iterator leafIterator
//		Path の開始位置
//		[NOTE] BestPathを求めるのに使われる索引語列の探索範囲の開始位置
// const LocationQueryPairVector::Iterator endIterator
//		Path の終了位置
//		[NOTE] BestPathを求めるのに使われる索引語列の探索範囲の終了位置
// ModSize startPosition
//		BestPath を求める開始位置
// const ModSize endPosition
//		BestPath を求める終了位置
// LocationPathPairVector& pathVector
//		結果格納用
//		[NOTE] ある位置と、その位置から検索語の最後までを
//		 最小被覆する索引語列（パス）とのペアのベクタ
//		[NOTE] 着目範囲がチェック済みかどうかを確認する際にendPositionは
//		 使われていない。つまりpathVectorに格納されるパスの終了位置は、
//		 全てendPositionで固定されている。
//		 再帰呼び出しの引数を見ても常にendPositionが渡されている。
//		[NOTE] Pathには、前方に位置する索引語が後方に格納される。
//		 NOTESの例だと、正量, 標補, 目標 の順番になる。
//
// RETURN
// 正常な場合 ModTrue 異常な場合 ModFalse
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedTermLeafNode::getBestPath(
	LocationQueryPairVector::Iterator leafIterator,
	const LocationQueryPairVector::Iterator endIterator,
	ModSize startPosition,
	const ModSize endPosition,
	LocationPathPairVector& pathVector)
{
	//
	// 前処理
	//
	
	if (findPath(pathVector, startPosition) != 0) {
		// 着目範囲は調べ済みだった
		return ModTrue;
	}

	//
	// leafIteratorをstartPositionまで進める
	//
	
	// [NOTE] leafIteratorのLocationQueryPairVector（索引語群）は、
	//  索引語の出現位置の順序で並べられているはず。
	//  着目範囲の開始位置であるstartPositionと
	//  索引語の出現位置とが一致するまでleafIteratorを進める。
	//  ちなみに(*leafIterator).firstは索引語の出現位置である。

	while (leafIterator != endIterator &&
		   (*leafIterator).first < startPosition) {
		++leafIterator;
	}

	//
	// ベストパスを取得する
	//

	if (leafIterator != endIterator &&
		(*leafIterator).first == startPosition) {

		//
		// 着目範囲の開始位置に出現する索引語が存在する場合
		//

		//
		// 同じ位置に複数の索引語があるなら、最長のものを選ぶ。
		//
		
		// 同じ位置にある索引語の内、最長のものに対応する LocationQueryPair
		LocationQueryPair leaf0 = *leafIterator;
		// 同じ位置にある索引語の内、最長のものの長さ
		ModSize length0;
		static_cast<SimpleTokenLeafNode*>((*leafIterator).second)->
			getTermLength(length0);
		
		ModSize length;
		// 次の索引語との比較から始める
		++leafIterator;
		while (leafIterator != endIterator &&
			   (*leafIterator).first == startPosition) {
			//
			// startPositionに出現する索引語を順番に調べる。
			//
			
			// 索引語長を取得
			static_cast<SimpleTokenLeafNode*>((*leafIterator).second)->
				getTermLength(length);
			
			if (length0 < length) {
				
				// 今までの索引語より長い

				// 索引語を更新
				leaf0 = *leafIterator;
				length0 = length;
			}

			// 次の索引語へ
			++leafIterator;
		}

		//
		// 以降、得られた索引語や、残りの索引語数を見て、処理を分岐する。
		//
		
		if (startPosition + length0 > endPosition) {

			//
			// 最長の索引語１つで着目範囲を覆うことができた。
			//

			Path* path = new Path(1, leaf0);
			pathVector.pushBack(LocationPathPair(startPosition, path));
			return ModTrue;
		}

		// [YET] 並列する条件ではないがelse ifを使っている
		else if (endIterator - leafIterator > 30)
		{
			//
			// 残りの索引語数が31?を超える場合 ※[NOTES]を参照
			//

			// ただ先頭からつなげる

			Path path;
			// 最長の索引語を設定
			ModSize len = length0;
			path.pushBack(leaf0);
			
			// 最長の索引語と隣合う索引語を、順番につなげるだけ
			// ベストパスは探索しない。
			ModSize sp = startPosition + len;
			while (leafIterator != endIterator)
			{
				if ((*leafIterator).first == sp)
				{
					path.pushBack(*leafIterator);
					static_cast<SimpleTokenLeafNode*>((*leafIterator).second)->
						getTermLength(len);
					sp += len;
				}
				++leafIterator;
			}
			--leafIterator;
			if ((*leafIterator).first != sp)
				path.pushBack(*leafIterator);
			
			// 順番を逆転させる
			// (呼び出し側で、最初が最後のパスであることが想定されている)
			Path* path0 = new Path;
			path0->reserve(path.getSize());
			Path::Iterator i = path.end();
			while (i != path.begin()) {
				--i;
				path0->pushBack(*i);
			}
			
			pathVector.pushBack(LocationPathPair(startPosition, path0));
			return ModTrue;
		}

		if (leafIterator != endIterator &&
			(*leafIterator).first <= startPosition + length0) {

			//
			// 最長索引語と、重なり合うか隣接するかしている索引語が存在する場合
			//

			// 最長索引語長を持つ索引語は、ベストパスの一部として確定する。
			// 最長索引語長を持つ索引語の開始位置 + 最長索引語長 までの範囲の
			// 各位置を開始位置とした部分ベストパスを取得し、
			// その中で最も良いパスを選択する。

			//
			// 部分ベストパスの初期値を得る
			//

			// 部分ベストパスの初期開始位置
			ModSize	 foremostPosition = (*leafIterator).first;
			// 部分ベストパスの初期値
			Path* path0;
			// 再帰的呼び出し
			if (getBestPath(leafIterator, endIterator,
							foremostPosition, endPosition, pathVector)
				== ModFalse) {
				return ModFalse;
			}
			path0 = findPath(pathVector, foremostPosition);

			//
			// 最良な部分ベストパスを得る
			//
			
			// 比較は次の位置とから。
			// （正確には次の位置に存在する索引語から）
			while (leafIterator != endIterator &&
				   (*leafIterator).first == foremostPosition) {
				// 同じ位置に存在する索引語は調査済みなので飛ばす。
				++leafIterator;
			}

			Path* path;
			while (leafIterator != endIterator &&
				   (*leafIterator).first <= startPosition + length0) {
				
				// 
				// 各開始位置を順番に処理する。
				//

				// 開始位置を更新
				; ModAssert(foremostPosition < (*leafIterator).first);
				foremostPosition = (*leafIterator).first;
				// 再帰的呼び出し
				if (getBestPath(leafIterator, endIterator,
								foremostPosition, endPosition, pathVector)
					== ModFalse) {
					return ModFalse;
				}
				path = findPath(pathVector, foremostPosition);

				//
				// 今までの部分ベストパスと比較
				//
				if (betterPath(*path, *path0) == ModTrue) {
					// pathの方が良いので、部分ベストパスを更新
					path0 = path;
				}
				
				// 次の位置へ
				while (leafIterator != endIterator &&
					   (*leafIterator).first == foremostPosition) {
					++leafIterator;
				}
			}

			// 最良の部分ベストパスが得られた。
			path = new Path(*path0);
			// 既に確定している最長索引語長を持つ索引語を追加
			// [NOTE] Pathには、前方に位置する索引語が後方に格納される。
			path->pushBack(leaf0);
			// ベストパスを格納
			pathVector.pushBack(LocationPathPair(startPosition, path));
			
			return ModTrue;
		}
		// 最長索引語に続く索引語が存在しなかった場合、ここに来る。
	}
	// 着目範囲の開始位置に出現する索引語が存在しなかった場合、ここに来る。

	//
	// この位置から始まるパスは取得できなかった。
	//

	// 後処理
	erasePathVector(pathVector);

	return ModFalse;
}

//
// FUNCTION private
// ModInvertedTermLeafNode::getSimplePath -- シンプルな索引語列を得る
//
// NOTES
// 相異なるトークンからなる索引語列を得る。
// つまり最小被覆を考慮しない。
// (例) 千葉県千葉市 − tokenize → {千葉, 葉県, 県千, 千葉, 葉市}
//					 − getSimplePath → (千葉, 葉県, 県千, 葉市)
// ただし包含関係のある索引語がある場合は、大きい方のみ選択する。
//
// ARGUMENTS
// const LocationQueryPairVector& vecPairLocationQuery_
//		トークナイズ結果
// LocationPathPairVecotr& vecPairLocationPath_
//		結果格納用
//		[NOTE] Pathに格納される索引語の順番はトークナイズ順
//		[NOTE] 空文字列からなる索引語はPathに含めない。条件に加えても
//		 ANDで検索するだけなので検索結果の絞り込みに有用ではないため。
//
// RETURN
// ModBoolean
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedTermLeafNode::getSimplePath(
	const LocationQueryPairVector& vecPairLocationQuery_,
	LocationPathPairVector& vecPairLocationPath_)
{
	LocationQueryPairVector::ConstIterator ite = vecPairLocationQuery_.begin();
	const LocationQueryPairVector::ConstIterator endIte =
		vecPairLocationQuery_.end();

	// LocationPathPairVectorに格納するPath
	Path* pPath = 0;
	// Pathに追加済みの索引語集合
	ModVector<ModUnicodeString> vecTerm;
	// 索引語の文字列を取得するための一時変数
	ModUnicodeString cstrTerm;
	
	// 一つ前に格納した索引語の開始位置
	ModSize uiPrevPos = 0;
	// 一つ前に格納した索引語の長さ
	ModSize uiPrevLength = 0;
	
	// 現在調べている位置
	ModSize uiPos = 0;
	// 現在調べている位置から始まる索引語の中で、最長索引語長
	ModSize uiMaxLength;
	// 現在調べている位置から始まる索引語の中で、最長索引語長のペア
	LocationQueryPair pairLocationQuery;
	
	// 参照中の索引語の長さ
	ModSize uiTempLength;

	while (ite != endIte)
	{
		//
		// 各LocationQueryPairを順番に処理する
		//

		if (uiPos == 0)
		{
			// 基準値が未設定の場合
			
			static_cast<SimpleTokenLeafNode*>(
				(*ite).second)->getTermLength(uiTempLength);
			if (uiTempLength > 0 &&
				(*ite).first + uiTempLength > uiPrevPos + uiPrevLength)
			{
				// 空文字列ではなく、一つ前に格納した索引語に包含されない場合

				// 例えば abc に対して、abc, bc, c とトークナイズされた場合、
				// bcとcはabcに包含されると考える。
			
				// [NOTE] 空文字列はPathに含めない

				// 基準値に設定する。
				uiPos = (*ite).first;
				uiMaxLength = uiTempLength;
				pairLocationQuery = *ite;
			}

			// 次の索引語へ
			++ite;
		}
		else if (uiPos == (*ite).first)
		{
			// 基準値と同じ位置から始まる索引語の場合
			
			static_cast<SimpleTokenLeafNode*>(
				(*ite).second)->getTermLength(uiTempLength);
			if (uiMaxLength < uiTempLength)
			{
				// もっと長い索引語がみつかった。

				// 最長索引語を更新
				pairLocationQuery = *ite;
				uiMaxLength = uiTempLength;
			}
			
			// 次の索引語へ
			++ite;
		}
		else
		{
			// 基準値と異なる位置から始まる索引語の場合

			if (addPairLocationQuery(pairLocationQuery, cstrTerm, vecTerm,
									 pPath) == ModTrue)
			{
				// 最長索引語を追加できた場合

				// 一つ前に格納した索引語を更新する
				uiPrevPos = uiPos;
				uiPrevLength = uiMaxLength;
				
				// 格納済みの索引語集合に追加する
				vecTerm.pushBack(cstrTerm);
			}

			// 基準値を未設定とする
			uiPos = 0;

			// この索引語が新しい基準値となるかどうか再評価するので、
			// 次の索引語に進まない。
		}
		
		// 次の評価へ
	}
	//最後の索引語をパスに追加する。
	if (uiPos != 0)
	{
		addPairLocationQuery(pairLocationQuery, cstrTerm, vecTerm, pPath);
	}

	// 実行結果を返す
	if (pPath != 0)
	{
		// 得られたPathを格納する
		// [NOTE] 開始位置は1で固定
		vecPairLocationPath_.pushBack(LocationPathPair(1, pPath));
		return ModTrue;
	}
	else
	{
		return ModFalse;
	}
}

//
// FUNCTION private static
// ModInvertedTermLeafNode::addPairLocationQuery -- LocationPathPairを追加する
//
// NOTES
//
// ARGUMENTS
// const LocationQueryPair& pairLocationQuery_
// ModUnicodeString& cstrTerm_
// const ModVector<ModUnicodeString>& vecTerm_
// LocationPathPairVector& vecPairLocationPath_
//
// RETURN
//
// EXCEPTIONS
//
ModBoolean
ModInvertedTermLeafNode::addPairLocationQuery(
	const LocationQueryPair& pairLocationQuery_,
	ModUnicodeString& cstrTerm_,
	const ModVector<ModUnicodeString>& vecTerm_,
	Path*& pPath_)
{
	// 索引語の取得
	cstrTerm_.clear();
	static_cast<SimpleTokenLeafNode*>(
		pairLocationQuery_.second)->contentString(cstrTerm_);
	
	if (vecTerm_.find(cstrTerm_) == vecTerm_.end())
	{
		// まだ格納されていない索引語の場合
		
		// 最長索引語長を持つ索引語をパスに追加する。
		if (pPath_ == 0)
		{
			// パスに一件も格納されていない場合
			
			pPath_ = new Path(1, pairLocationQuery_);
		}
		else
		{
			pPath_->pushBack(pairLocationQuery_);
		}

		return ModTrue;
	}
	return ModFalse;
}

//
// FUNCTION private
// ModInvertedTermLeafNode::findPath -- 索引語列を得る
//
// NOTES
// path を取り出す (validate() と getBestPath() の補助関数)
//
// ARGUMENTS
// LocationPathPairVector& pathVector
//		結果格納用
// ModSize startPosition
//		開始位置
//
// RETURN
// path
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModVector< ModPair<ModSize, ModInvertedQueryNode*> > *
ModInvertedTermLeafNode::findPath(LocationPathPairVector& pathVector,
								  ModSize startPosition)
{
	for (LocationPathPairVector::Iterator p(pathVector.begin());
		 p != pathVector.end(); ++p) {
		if ((*p).first == startPosition) {
			return (*p).second;
		}
	}
	return 0;
}

//
// FUNCTION private
// ModInvertedTermLeafNode::erasePathVector -- 索引語列を消す
//
// NOTES
// pathVector を消す (validate() と getBestPath() の補助関数)
//
// ARGUMENTS
// LocationPathPairVector& pathVector
//		path
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedTermLeafNode::erasePathVector(LocationPathPairVector& pathVector)
{
	for (LocationPathPairVector::Iterator p(pathVector.begin());
		 p != pathVector.end(); ++p) {
		delete (*p).second;
	}
	pathVector.clear();
}

//
// FUNCTION private
// ModInvertedTermLeafNode::betterPath -- ふたつのpathを比較する
//
// NOTES
// 引数 x と y を比較するして、x が良いと判断したら ModTrue を返し、そ
// うでなければ ModFalse を返す
//
// ARGUMENTS
// const LocationQueryPairVector& x
//		パス１
// const LocationQueryPairVector& y
//		パス１
//
// RETURN
// x が良い場合 ModTrue、悪い場合 ModFalse
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedTermLeafNode::betterPath(const LocationQueryPairVector& x,
									const LocationQueryPairVector& y)
{
	if (x.getSize() < y.getSize()) {
		return ModTrue;
	} else if (x.getSize() > y.getSize() || x.getSize() == 0) {
		return ModFalse;
	} else {
		LocationQueryPairVector::ConstIterator p = x.begin();
		ModSize minimalDf = (*p).second->estimateDocumentFrequency();
		ModSize totalDf = minimalDf;
		for (++p; p != x.end(); ++p) {
			totalDf += (*p).second->estimateDocumentFrequency();
			if (minimalDf > (*p).second->estimateDocumentFrequency()) {
				minimalDf = (*p).second->estimateDocumentFrequency();
			}
		}
		for (p = y.begin(); p != y.end(); ++p) {
			if (minimalDf > (*p).second->estimateDocumentFrequency()) {
				return ModFalse;
			}
			totalDf -= (*p).second->estimateDocumentFrequency();
			if (totalDf < 0) {
				return ModTrue;
			}
		}
	}
	return ModFalse;
}

//
// FUNCTION private
// ModInvertedTermLeafNode::createOrderedDistanceNode -- RankingOrderedDistanceNodeの生成
//
// NOTES
// 引数distanceを使ってOrderedDistanceNodeを生成する。
//
// ARGUMENTS
// const ModInvertedQuery& query
//		Queryの参照(未使用
//		RankingTermLeafNode::createOrderedDistanceNode()に合すために付
//		けただけ)
//
// RETURN
// 生成したOrderedDistanceNode
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModInvertedOrderedDistanceNode*
ModInvertedTermLeafNode::createOrderedDistanceNode(
	const ModInvertedQuery& query,
	const ModInvertedFile* invertedFile,
	const Query::ValidateMode mode) const
{
	ModInvertedOrderedDistanceNode* node
		= new ModInvertedOrderedDistanceNode(query.getTotalDocumentFrequency(),firstStepResult->getType());

	if ((mode&Query::rankingMode) != 0) {
		// TermLeafNodeにスコア計算器があればそれをセット。
		// なければQueryNodeのデフォルトのものをセットする。
		if (this->scoreCalculator != 0) {
			node->setScoreCalculator(this->scoreCalculator->duplicate());
		}
		else {
			// QueryNodeには必ずデフォルトの計算機をセットするように
			// なったので、ここではduplicateだけ
			ModInvertedRankingScoreCalculator* calculator =
				query.getDefaultScoreCalculator();
			; ModAssert(calculator != 0);
			node->setScoreCalculator(calculator->duplicate());
		}

		node->getScoreCalculator()->setDocumentLengthFile(
			invertedFile->getDocumentLengthFile());
		node->getScoreCalculator()->setAverageDocumentLength(
			query.getAverageDocumentLength());
	}

	return node;
}

//
// FUNCTION private
// ModInvertedTermLeafNode::createWordOrderedDistanceNode -- RankingWordOrderedDistanceNodeの生成
//
// NOTES
// 引数distanceを使ってWordOrderedDistanceNodeを生成する。
//
// ARGUMENTS
// const ModInvertedQuery& query
//		Queryの参照(未使用
//		RankingTermLeafNode::createOrderedDistanceNode()に合すために付
//		けただけ)
//
// RETURN
// 生成したOrderedDistanceNode
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModInvertedWordOrderedDistanceNode*
ModInvertedTermLeafNode::createWordOrderedDistanceNode(
	const ModInvertedQuery& query,
	const ModInvertedFile* invertedFile,
	const Query::ValidateMode mode) const
{
	ModInvertedWordOrderedDistanceNode* node
		= new ModInvertedWordOrderedDistanceNode(
			query.getTotalDocumentFrequency(),firstStepResult->getType());

	if ((mode&Query::rankingMode) != 0) {
		// TermLeafNodeにスコア計算器があればそれをセット。
		// なければQueryNodeのデフォルトのものをセットする。
		if (this->scoreCalculator != 0) {
			node->setScoreCalculator(this->scoreCalculator->duplicate());
		}
		else {
			// QueryNodeには必ずデフォルトの計算機をセットするように
			// なったので、ここではduplicateだけ
			ModInvertedRankingScoreCalculator* calculator =
				query.getDefaultScoreCalculator();
			; ModAssert(calculator != 0);
			node->setScoreCalculator(calculator->duplicate());
		}

		node->getScoreCalculator()->setDocumentLengthFile(
			invertedFile->getDocumentLengthFile());
		node->getScoreCalculator()->setAverageDocumentLength(
			query.getAverageDocumentLength());
	}

	return node;
}

//
// FUNCTION private
// ModInvertedTermLeafNode::createRoughNodeForTermLeafNode -- TermLeafNodeのラフノード作成
//
// NOTES
// TermLeafNodeの裸婦ノード作成。検索タイプにあわせて、BooleanAnd/RankingAnd/
// AtomicAndを作成する。本関数がコールされるのはブーリアン検索の場合なので、
// BooleanAndを作成。
//
// ARGUMENTS
// const ModInvertedQuery& query
//		Queryの参照(未使用)
// const Query::ValidateMode mode
//		評価モード(未使用)
//
// RETURN
// 生成したAndNode
//
// EXCEPTIONS
// なし
//
ModInvertedQueryNode*
ModInvertedTermLeafNode::createRoughNodeForTermLeafNode(
	const ModInvertedQuery& query,
	const Query::ValidateMode mode)
{
	ModInvertedOperatorAndNode* tmpRoughNode
		= new ModInvertedOperatorAndNode(firstStepResult->getType());

	if ((mode & Query::rankingMode) != 0) {

		// QueryNodeには必ずデフォルトの合成機をセットするように
		// なったので、ここではduplicateだけ
		ModInvertedRankingScoreCombiner* combiner
					= query.getDefaultAndScoreCombiner();
		;ModAssert(combiner != 0);
		tmpRoughNode->setScoreCombiner(combiner->duplicate());
	}

	return tmpRoughNode;
}

// ModInvertedTermLeafNode::createRoughNodeForTermLeafNode2 -- ラフノード作成
//
// NOTES
// ラフノードを作成してvectorに格納する。
//
// ARGUMENTS
// ModVector<ModInvertedQueryNode*>::Iterator iteratorForRoughNodes
//		roughNodes用iterator
// const ModVector<ModInvertedQueryNode*> roughNodes
//		ラフノード格納用vector
// const ModInvertedQuery& query
//		Queryの参照
// const Query::ValidateMode mode
//		評価モード
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void ModInvertedTermLeafNode::createRoughNodeForTermLeafNode2(
	const ModInvertedQuery& query,
	ModVector<ModInvertedQueryNode*>::Iterator iteratorForRoughNodes,
	const ModVector<ModInvertedQueryNode*>::Iterator roughEnd,
	const ModSize roughSize,
	const Query::ValidateMode mode)
{
	// 各子ノードがいくつのラフノードに出現するか調べ、
	// roughNodes.getSize()と同じ数のラフノードに出現する子ノードを
	// すべてのラフノードに共通な子ノードと見なす。

	// 出現回数カウント用Map
	// pair(queryNode*(子ノード), int(出現回数))
	QueryNodeCountMap roughGrandChildCountMap;

	// ラフノードの子ノード格納用
	ModVector<ModInvertedQueryNode*>* grandChildren;
	ModVector<ModInvertedQueryNode*>::Iterator iteratorGrandChild;

	// 初めに一番目のラフノードをMapに登録する
	// ここにくるのはAndNodeかsimpletokenLeafNodeなので
	// 中間ノードかどうかはAndNodeかどうかで判断する
	if( ModInvertedAtomicMask((*iteratorForRoughNodes)
			->getType()) == ModInvertedQueryNode::operatorAndNode ) {

		// 中間ノード(andNode)の場合
		// ラフノードの子ノードを取得
		grandChildren = static_cast<InternalNode*>(
								*iteratorForRoughNodes)->getChildren();

		iteratorGrandChild = (grandChildren)->begin();

		// 子ノードすべてをMapに登録
		while (iteratorGrandChild != (grandChildren)->end()) {
			// 出現回数 = 1
			roughGrandChildCountMap[*iteratorGrandChild] = 1;
			++iteratorGrandChild;
		}
	} else {

		// 中間ノード以外(simpleTokeLeafNode)
		// そのままMapに登録
		roughGrandChildCountMap[*iteratorForRoughNodes] = 1;
	}

	// andNodeは不要
	if (ModInvertedAtomicMask((*iteratorForRoughNodes)
			->getType()) != ModInvertedQueryNode::simpleTokenLeafNode) {
		delete *iteratorForRoughNodes;
	}
	++iteratorForRoughNodes;

	// 二つ目以降のラフノード
	QueryNodeCountMap::Iterator p ;

	while (iteratorForRoughNodes != roughEnd) {

		if( ModInvertedAtomicMask((*iteratorForRoughNodes)
				->getType()) == ModInvertedQueryNode::operatorAndNode ) {

			// 中間ノード(andNode)の場合
			// ラフノードの子ノードを取得
			grandChildren = static_cast<InternalNode*>(
					*iteratorForRoughNodes)->getChildren();

			iteratorGrandChild = grandChildren->begin();

			while (iteratorGrandChild != grandChildren->end()) {
				// Mapと取得した子ノードを比較
				p = roughGrandChildCountMap.find(*iteratorGrandChild);
				if (p != roughGrandChildCountMap.end()) {
					// 子ノードがMapに存在した
					// 出現回数++
					roughGrandChildCountMap[*iteratorGrandChild]++;
				}
				// Mapに存在しなかった子ノードを登録する必要はない
				// 既にすべてのラフノードに存在するという条件を
				// 満たしていない
				++iteratorGrandChild;
			}
		} else {
			// 中間ノード以外(simpleTokeLeafNode)
			// そのままMapに登録

			// Mapとラフノードを比較
			p = roughGrandChildCountMap.find(*iteratorForRoughNodes);
			if (p != roughGrandChildCountMap.end()) {
				// 子ノードがMapに存在した
				// 出現回数++
				roughGrandChildCountMap[*iteratorForRoughNodes]++;
			}
		}

		// andNodeは不要
		if (ModInvertedAtomicMask(
				(*iteratorForRoughNodes)->getType())
				!= ModInvertedQueryNode::simpleTokenLeafNode) {
			delete *iteratorForRoughNodes;
		}
		++iteratorForRoughNodes;
	}

	ModInvertedQueryNode* tmpRoughNode = 0;

	// ラフノードを生成
	tmpRoughNode = createRoughNodeForTermLeafNode(query, mode);

	// Mapに登録されているラフノードの子ノードのうち全ラフノード
	// に出現しているもの(出現回数がroughNodes.getSize()と同じ)を
	// tmpRoughNodeにinsertChild
	for (p = roughGrandChildCountMap.begin();
		p != roughGrandChildCountMap.end(); ++p) {
		if (ModSize((*p).second) == roughSize) {
		// すべてのラフノードに出現
		static_cast<InternalNode*>(tmpRoughNode)->insertChild((*p).first);
		}
	}

	// 子ノードの数を調べるために使う
	ModVector<ModInvertedQueryNode*>* tmpNodes
		= static_cast<InternalNode*>(tmpRoughNode)->getChildren();

	if (tmpNodes->getSize() > 0) {
		// tmpRoughNodeの子ノードの数が0以上

		// QueryNodeForPreciseEvaluationにtmpRoughNodeをセット
		setQueryNodeForRoughEvaluation(tmpRoughNode);
		queryNodeForRoughEvaluation->setQueryNodeForRoughEvaluation(
			queryNodeForRoughEvaluation);
	} else {
		// tmpRoughNodeの子ノードの数が0
		// セットしない
		queryNodeForRoughEvaluation = 0;
		delete tmpRoughNode;
	}
}

//
// ModInvertedTermLeafNode::createOrNode -- 検索タイプに応じたorNode生成
//
// NOTES
// ブーリアン検索のときはOperatorOrNode、
// ランキング検索のときはAtomicOrNodeを生成する。
//
// ARGUMENTS
// const ModInvertedQuery& query
//		Queryの参照
// const Query::ValidateMode mode
//		評価モード
// ModInvertedOperatorOrNode& orNode
//		生成するOrNode（中でnewしている）
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
ModInvertedOperatorOrNode*
ModInvertedTermLeafNode::createOrNode(const ModInvertedQuery& query,
										const ModInvertedFile* invertedFile,
										const Query::ValidateMode mode)
{
	// preciseノード用orNode
	ModInvertedOperatorOrNode* tmpNode = 0;

	if ((mode&ModInvertedQuery::rankingMode) == 0) {
		// ブーリアンの場合はOperatorOrを生成する
		tmpNode = new ModInvertedOperatorOrNode(firstStepResult->getType());

		tmpNode->setTotalDocumentFrequency(
			query.getTotalDocumentFrequency());
	} else {
		// 2000/02/29
		// ランキング検索の場合はAtomicOrを生成する
		// コンストラクタの修正が必要 ！！！
		tmpNode = new ModInvertedAtomicOrNode(firstStepResult->getType());

		tmpNode->setTotalDocumentFrequency(query.getTotalDocumentFrequency());

		// TermLeafNodeにスコア計算器があればそれをセット。
		// なければQueryNodeのデフォルトのものをセットする。
		if (this->scoreCalculator != 0) {
			tmpNode->setScoreCalculator(scoreCalculator->duplicate());
		}
		else {
			// QueryNodeには必ずデフォルトの計算機をセットするように
			// なったので、ここではduplicateだけ
			ModInvertedRankingScoreCalculator* calculator =
				query.getDefaultScoreCalculator();
			; ModAssert(calculator != 0);
			tmpNode->setScoreCalculator(calculator->duplicate());
		}

		tmpNode->getScoreCalculator()->setDocumentLengthFile(
			invertedFile->getDocumentLengthFile());
		tmpNode->getScoreCalculator()->setAverageDocumentLength(
			query.getAverageDocumentLength());
	}

	return tmpNode;
}

//
// FUNCTION private
// ModInvertedTermLeafNode::setSimpleTokenLeafNodeForNormalWord --
//
// NOTES
//
// ARGUMENTS
// ModInvertedQueryNode* pQueryNode_
// const ModInvertedQuery& cQuery_,
// const ModInvertedFile* pInvertedFile_,
// Query::ValidateMode uiValidateMode_,
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedTermLeafNode::setSimpleTokenLeafNodeForNormalWord(
	ModInvertedQueryNode* pQueryNode_,
	const ModInvertedQuery& cQuery_,
	const ModInvertedFile* pInvertedFile_,
	Query::ValidateMode uiValidateMode_)
{
	; ModAssert(pQueryNode_->getType() == simpleTokenLeafNode);
	ModInvertedSimpleTokenLeafNode* pSimpleTokenLeafNode =
		static_cast<ModInvertedSimpleTokenLeafNode*>(pQueryNode_);
	
	pSimpleTokenLeafNode->setIterator();

	if ((uiValidateMode_ & ModInvertedQuery::rankingMode) != 0 &&
		pSimpleTokenLeafNode->getScoreCalculator() == 0) {

		//
		// ランキングモードで、スコア計算器がない場合
		//

		// スコア計算器を設定
		ModInvertedRankingScoreCalculator* tmpCalculator = 0;
		if (this->scoreCalculator != 0) {
			tmpCalculator = this->scoreCalculator->duplicate();
		} else {
			; ModAssert(cQuery_.defaultScoreCalculator != 0);
			tmpCalculator = cQuery_.defaultScoreCalculator->duplicate();
		}
		pSimpleTokenLeafNode->setScoreCalculator(tmpCalculator);

		// スコア計算器の前準備
		// [NOTE] tmpCalculatorは上記でメンバ変数とduplicateされているので、
		//  これに対する実行結果は保持される。
		tmpCalculator->setDocumentLengthFile(
			pInvertedFile_->getDocumentLengthFile());
		tmpCalculator->setAverageDocumentLength(
			cQuery_.getAverageDocumentLength());
		tmpCalculator->prepare(
			this->totalDocumentFrequency,
			pSimpleTokenLeafNode->getInvertedList()->getDocumentFrequency());
	}
}

//
// FUNCTION private
// ModInvertedTermLeafNode::checkMatchMode --
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//
ModInvertedTermMatchMode
ModInvertedTermLeafNode::checkMatchMode(
	ModInvertedFileIndexingType iIndexingType_,
	MatchMode iMatchMode_) const
{
	MatchMode iMatchMode = iMatchMode_;
	
	//
	// トークン分割モード決定の前処理
	//
	
#ifdef V1_4
	if(iMatchMode == ModInvertedTermMultiLanguageMode)
	{
		if (_cIsMulti.get() == true) {
			// 多言語対応モード
			// 文字列にCJKが含まれるときは文字列検索
			// 含まれないときは厳格単語検索
			iMatchMode = ModInvertedTermExactWordMode;
			int size = (int)termString.getLength();
			for(int i = 0; i < size ; ++i)
			{
				if( ModUnicodeCharTrait::isAlphabet(termString[i]) == ModTrue ||
					ModUnicodeCharTrait::isDigit(termString[i]) == ModTrue ||
					ModUnicodeCharTrait::isSymbol(termString[i]) == ModTrue ||
					ModUnicodeCharTrait::isSpace(termString[i]) == ModTrue )
				{
					continue;
				}
				else
				{
					iMatchMode = ModInvertedTermStringMode;
					break;
				}
			}
		} else {
			// 多言語対応モードは利用しない -> すべて文字列検索
			iMatchMode = ModInvertedTermStringMode;
		}
	}
#endif // V1_4

	//
	// トークン分割モードの検査
	//
	
	if ((iMatchMode == ModInvertedTermStringMode &&
		 (iIndexingType_&ModInvertedFileNgramIndexing) == 0) ||
		((iMatchMode == ModInvertedTermExactWordMode ||
		  iMatchMode == ModInvertedTermWordHead
#ifndef MOD_DIST // APPMODE
			|| iMatchMode == ModInvertedTermApproximateMode
#endif
			) &&
		 (iIndexingType_&ModInvertedFileWordIndexing) == 0) ||
		((iMatchMode == ModInvertedTermSimpleWordMode ||
		  iMatchMode == ModInvertedTermWordTail) &&
		 iIndexingType_ != ModInvertedFileDualIndexing))
	{
		ModErrorMessage << "invalid match mode: " << iMatchMode << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorInvalidMatchMode);
	}

	return iMatchMode;
}

//
// FUNCTION private
// ModInvertedTermLeafNode::getTokenizeMode --
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//
ModInvertedTokenizer::TokenizeMode
ModInvertedTermLeafNode::getTokenizeMode(
	ModInvertedFileIndexingType iIndexingType_,
	MatchMode iMatchMode_,
	Query::ValidateMode uiValidateMode_) const
{
	ModInvertedTokenizer::TokenizeMode tokenizeMode;
	
	if (iIndexingType_ == ModInvertedFileWordIndexing)
	{
		tokenizeMode = ModInvertedTokenizer::TokenizeMode(
			ModInvertedTokenizer::query |
			ModInvertedTokenizer::wordIndexingOnly |
			(((uiValidateMode_ & ModInvertedQuery::skipNormalizing) != 0) ?
			 ModInvertedTokenizer::skipNormalizing : 0) |
			(((uiValidateMode_ & ModInvertedQuery::skipExpansion) != 0) ?
			 ModInvertedTokenizer::skipExpansion : 0));
	}
	else
	{
		tokenizeMode = ModInvertedTokenizer::TokenizeMode(
			(((uiValidateMode_ & ModInvertedQuery::tokenizeQuery) != 0) ?
			 ModInvertedTokenizer::query : ModInvertedTokenizer::simpleQuery) |
			(((iMatchMode_ != ModInvertedTermExactWordMode)
#ifndef MOD_DIST // APPMODE
			  && (iMatchMode_ != ModInvertedTermApproximateMode)
#endif
				) ?
			 ModInvertedTokenizer::ngramIndexingOnly : 0) |
			(((uiValidateMode_ & ModInvertedQuery::skipNormalizing) != 0) ?
			 ModInvertedTokenizer::skipNormalizing : 0) |
			(((uiValidateMode_ & ModInvertedQuery::skipExpansion) != 0) ?
			 ModInvertedTokenizer::skipExpansion : 0));
	}

	return tokenizeMode;
}

//
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
