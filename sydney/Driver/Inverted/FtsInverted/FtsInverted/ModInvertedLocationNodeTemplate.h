// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedLocationNodeTemplate.h -- 近傍演算ノードインタフェイスファイル
// 
// Copyright (c) 1999, 2000, 2001, 2002, 2004, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedLocationNodeTemplate_H__
#define __ModInvertedLocationNodeTemplate_H__

#include "ModInvertedWordBaseNode.h"

#ifdef SYD_INVERTED
#include "Inverted/ModInvertedFile.h"
#include "Inverted/ModInvertedDocumentLengthFile.h"
#else
#include "ModInvertedFile.h"
#include "ModInvertedDocumentLengthFile.h"
#endif

//
// CLASS
// ModInvertedWindowNodeTemplate -- Sinple/Operator Windowノードのテンプレートクラス
//
// NOTES
//
template<class Type1, ModInvertedQueryNode::NodeType nodeType>
class
ModInvertedLocationNodeTemplate
	: public ModInvertedWordBaseNode
{
public :

	// コンストラクタ
	ModInvertedLocationNodeTemplate(const ModSize location_, const NodeType,const ModUInt32 resultType_);
	//ModInvertedLocationNodeTemplate(const NodeType);

	// 与えられた文書が検索条件を満たすかどうかの検査 BOOL
	virtual ModBoolean evaluate(DocumentID documentID,
								Query::EvaluateMode mode);

	// 文書内頻度を得る
	virtual ModSize getTermFrequency(DocumentID documentID,
									 Query::EvaluateMode mode);

	// 自分のコピーを作成する
	virtual QueryNode* duplicate(const ModInvertedQuery& query);

	// 演算子を表わす文字列を返す
	virtual void prefixString(ModUnicodeString& prefix,
							  const ModBoolean withCalOrCombName,
							  const ModBoolean withCalOrCombParam) const;

	ModInvertedDocumentLengthFile* getDocumentLengthFile() const;

	virtual void validate(InvertedFile* invertedFile,
						  const ModInvertedQuery::ValidateMode mode,
						  ModInvertedQuery* rQuery);

	// 粗い evaluate 満足を前提とした、正確な再評価
	virtual ModBoolean reevaluate(DocumentID documentID);
	virtual ModBoolean reevaluate(DocumentID documentID,
								  LocationIterator*& location,
								  ModSize& uiTF_,
								  ModInvertedQueryNode* givenEndNode = 0);

	ModSize getLocation();

	// Location では location , End では distance
	// を統一したメンバ変数
	ModSize location;

	ModInvertedDocumentLengthFile* documentLengthFile;
};

//
// FUNCTION protected
// ModInvertedLocationNodeTemplate::ModInvertedLocationNodeTemplate -- コンストラクタ
//
// NOTES
//
// ARGUMENTS
// const ModSize location_
// 		Location では location , End では distanceに当たる値     
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
template<class Type1, ModInvertedQueryNode::NodeType nodeType>
inline
ModInvertedLocationNodeTemplate<Type1, nodeType>
	::ModInvertedLocationNodeTemplate(const ModSize location_, 
									  const NodeType nType,
									  const ModUInt32 resultType_
							)
	: ModInvertedWordBaseNode(nType,resultType_)
{
	location = location_;
}

//
// FUNCTION public
// ModInvertedLocationNodeTemplate<Type1, nodeType>::evaluate -- 与えられた文書が検索条件式を満たすかどうかの検査
//
// NOTES
// 与えられた文書が検索式を満たすかどうか検査する。
//
// ARGUMENTS
// ModInvertedDocumentID documentID	
//		評価する文書ID
// ModInvertedQuery::EvaluateMode mode
//		評価モード
//
// RETURN
// 与えられた文書がが条件を満たす場合ModTrue、満たさない場合ModFalse
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
template<class Type1, ModInvertedQueryNode::NodeType nodeType>
inline ModBoolean 
ModInvertedLocationNodeTemplate<Type1, nodeType>::evaluate(DocumentID documentID, 
													Query::EvaluateMode mode)
{
	// 既に調査済み？
	if (documentID >= this->lower) {
		if (documentID == this->upper) {
			return ModTrue;
		} else if (documentID < this->upper || this->upper == -1) {
			return ModFalse;
		}
	}

	LocationIterator* childLocation = 0;

	if (children[0]->evaluate(documentID, childLocation, mode) == ModFalse) {
		return ModFalse;
	}

	ModSize endLocation(0);

	if(nodeType == AtomicNode::operatorEndNode) {
		// 子ノードの位置情報を取得
		documentLengthFile->search(documentID, endLocation);
	}

	// 検索語の出現位置を調べる
	Type1* iterator = static_cast<Type1*>(getFreeList());
	if (iterator == 0)
	{
		iterator = new Type1(this);
	}
	LocationIterator::AutoPointer p = iterator;
	
	iterator->initialize(childLocation, endLocation, location);
	if(iterator->isEnd() != ModTrue) {
		this->upper = this->lower = documentID;
		return ModTrue;
	}
	return ModFalse;
}

//
// FUNCTION public
// ModInvertedLocationNodeTemplate<Type1, nodeType>::duplicate -- 自分のコピーを作成する
//
// NOTES
// 自分のコピーを作成する。Query::から呼ばれ、検索木を上からたどり、
// 検索木のコピーを作成する際に使用される。
// QueryNode::dupulicateをオーバーライド。LocationNodeのコピーを作る。
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
template<class Type1, ModInvertedQueryNode::NodeType nodeType>
inline ModInvertedQueryNode*
ModInvertedLocationNodeTemplate<Type1, nodeType>::duplicate(const ModInvertedQuery& rQuery)
{
	//Type2* node = new Type2(location);
	ModInvertedLocationNodeTemplate<Type1, nodeType>* node 
		= new ModInvertedLocationNodeTemplate<Type1, nodeType>(location, nodeType,firstStepResult->getType());

	if (this->scoreCalculator != 0) {
		node->scoreCalculator = this->scoreCalculator->duplicate();
	}
	
	// totalDucumentFrequencyのコピー
	node->setTotalDocumentFrequency(totalDocumentFrequency);

	// 子ノードのduplicateの結果
	ModInvertedQueryNode* newChild;

	newChild = children[0]->duplicate(rQuery);

	// 子ノードを追加
	node->insertChild(newChild);

	return static_cast<ModInvertedQueryInternalNode*>(node);
}

//
// FUNCTION public
// ModInvertedLocationNodeTemplate<Type1, nodeType>::validate -- 正規表現ノードの有効化
//
// NOTES
// 初期化としてベクターファイルのオープン，ヒープファイルのオープン，
// 正規表現のコンパイルを行う。
//
// ARGUMENTS
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
template<class Type1, ModInvertedQueryNode::NodeType nodeType>
inline void
ModInvertedLocationNodeTemplate<Type1, nodeType>::validate(InvertedFile* invertedFile,
										  const Query::ValidateMode mode,
										  ModInvertedQuery* rQuery)
{
	if(nodeType == AtomicNode::operatorEndNode) {
		// 転置ファイルからDocumentLengthFileを取得
		documentLengthFile = invertedFile->getDocumentLengthFile();
	}

	// ランキング検索の場合はスコア計算器をセットする
	if ((mode & Query::rankingMode) != 0) {
		if (this->scoreCalculator == 0) {
			// QueryNodeには必ずデフォルトの計算器をセットするように
			// なったので、ここではduplicateだけ
			ScoreCalculator* calculator = rQuery->getDefaultScoreCalculator();
			;ModAssert(calculator != 0);
			this->scoreCalculator = calculator->duplicate();
		}
	}

	// totalDocumentFrequencyのセット
	setTotalDocumentFrequency(rQuery->getTotalDocumentFrequency());

	// 子ノードの有効化
	ModInvertedQueryInternalNode::validate(invertedFile, mode, rQuery);
}

//
// FUNCTION public
// ModInvertedLocationNodeTemplate<Type1, nodeType>::prefixString -- 演算子を表わす文字列を返す
//
// NOTES
// LocationNodeで定義された内容をオーバライドする。
// 演算子を表わす文字列を返す
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
template<class Type1, ModInvertedQueryNode::NodeType nodeType>
inline void
ModInvertedLocationNodeTemplate<Type1, nodeType>::prefixString(
	ModUnicodeString& prefix,
	const ModBoolean withCalOrCombName,
	const ModBoolean withCalOrCombParam) const
{
	if(nodeType == AtomicNode::operatorEndNode) {
		prefix += "#end[";
	} else {
		prefix += "#location[";
	}	
	ModOstrStream os;
	os << location;
	prefix += os.getString();

	if (withCalOrCombName == ModTrue) {
		ModUnicodeString calculatorName;
		getCalculatorOrCombinerName(calculatorName, withCalOrCombParam);
		if (calculatorName.getLength() > 0) {
			prefix += ',';
			prefix += calculatorName;
		}
	}

	prefix += ']';
}

//
// FUNCTION protected
// ModInvertedLocationNodeTemplate<Type1, nodeType>::reevaluate -- 正確な再評価
//
// NOTES
// 粗い evaluate 満足を前提とした、正確な再評価
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
//
// RETURN
// 与えられた文書が検索式を満たす場合 ModTrue、満たさない場合 ModFalse
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
template<class Type1, ModInvertedQueryNode::NodeType nodeType>
inline ModBoolean
ModInvertedLocationNodeTemplate<Type1, nodeType>::reevaluate(DocumentID documentID)
{
	LocationIterator* childLocation = 0;
	if (children[0]->reevaluate(documentID, childLocation) == ModFalse) {
		return ModFalse;
	}
	if (childLocation == 0)
	{
		// 位置情報リストを取得できなかったので、
		// 位置の確認はできないが成功したとみなす。
		return ModTrue;
	}

	ModSize endLocation(0);

	if(nodeType == AtomicNode::operatorEndNode) {
		documentLengthFile->search(documentID, endLocation);
	}

	// 検索語の出現位置を調べる
	Type1* iterator = static_cast<Type1*>(getFreeList());
	if (iterator == 0)
	{
		iterator = new Type1(this);
	}
	LocationIterator::AutoPointer p = iterator;
	
	iterator->initialize(childLocation, endLocation, location);
	ModBoolean result = (iterator->isEnd() == ModFalse) ? ModTrue : ModFalse;

	return result;
}

//
// FUNCTION protected
// ModInvertedLocationNodeTemplate<Type1, nodeType>::reevaluate -- 正確な再評価
//
// NOTES
// 粗い evaluate 満足を前提とした、正確な再評価
//
// 引数 locations には
// ModInvertedLocationNodeLocationListIterator をセットする。
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
// ModInvertedLocationListIterator*& locations
//		位置情報（結果格納用）
// ModSize& uiTF_
//		(位置情報リストを取得できない場合) TF
// ModInvertedQueryNode* giveEndNode
//		ここでは未使用。orderedDistanceでのみ使用。
//
// RETURN
// 与えられた文書が検索式を満たす場合 ModTrue、満たさない場合 ModFalse
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
template<class Type1, ModInvertedQueryNode::NodeType nodeType>
inline ModBoolean 
ModInvertedLocationNodeTemplate<Type1, nodeType>::reevaluate(
	DocumentID documentID,
	LocationIterator*& locations,
	ModSize& uiTF_,
	ModInvertedQueryNode* givenEndNode)
{
	// 先頭子ノードを再評価する
	LocationIterator* childLocation = 0;
	if (children[0]->reevaluate(documentID, childLocation, uiTF_) == ModFalse) {
		return ModFalse;
	}
	if (childLocation == 0)
	{
		// 位置情報リストを取得できなかったので、
		// 位置の確認はできないが成功したとみなす。
		// TFは常に取得できるはず。
		//; ModAssert(uiTF_ > 0);
		return ModTrue;
	}

	// 文書長(データの末尾の位置)を取得
	ModSize endLocation(0);
	if(nodeType == AtomicNode::operatorEndNode) {
		documentLengthFile->search(documentID, endLocation);
	}

	// 検索語の出現位置を調べる
	Type1* iterator = static_cast<Type1*>(getFreeList());
	if (iterator == 0)
	{
		iterator = new Type1(this);
	}
	LocationIterator::AutoPointer p = iterator;

	iterator->initialize(childLocation, endLocation, location);
	if(iterator->isEnd() == ModFalse) {
		locations = p.release();
		return ModTrue;
	}
	return ModFalse;
}

//
// FUNCTION public
// ModInvertedOperatorEndNode::getDocumentLengthFile -- documentLengthFileのアクセサ関数
//
// NOTES
// メンバ変数documentLengthのアクセサ関数。
//
// ARGUMENTS
// なし
//
// RETURN
// documentLengthFile
//
// EXCEPTIONS
// なし
//
template<class Type1, ModInvertedQueryNode::NodeType nodeType>
inline ModInvertedDocumentLengthFile*
ModInvertedLocationNodeTemplate<Type1, nodeType>::getDocumentLengthFile() const
{
	return documentLengthFile;
}

//
// FUNCTION public
// ModInvertedOperatorEndNode::getTermFrequency -- 文書内頻度の取得
//
// NOTES
// 条件を満たす語句の文書内出現頻度を求める。QueryNode の
// getTermFrequency をオーバライドしている。
//
// もし子ノードが全て SimpleTokenLeafNode であった場合は、
// SimpleTokenLeafNode の Iterator から文書内出現頻度を取り出して掛け
// 合わせることにより自分の文書内出現頻度を求める。
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//      文書ID
// Query::EvaluateMode mode
//      評価モード
//
// RETURN
// 求めた文書内頻度
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
template<class Type1, ModInvertedQueryNode::NodeType nodeType>
inline ModSize
ModInvertedLocationNodeTemplate<Type1, nodeType>
	::getTermFrequency(ModInvertedDocumentID documentID,
					   Query::EvaluateMode mode)
{
	return (evaluate(documentID, mode) == ModTrue) ? 1 : 0;
}

//
// FUNCTION public
// ModInvertedOperatorLocationNode::getLocation -- locationの値を返す
//
// NOTES
// メンバ変数locaitonのアクセサ関数。locationの値を返す
//
// ARGUMENTS
// なし
//
// RETURN
// locaiton
//
// EXCEPTIONS
// なし
//
template<class Type1, ModInvertedQueryNode::NodeType nodeType>
inline ModSize 
ModInvertedLocationNodeTemplate<Type1, nodeType>::getLocation()
{
	return location;
}

#endif //__ModInvertedLocationNodeTemplate_H__

//
// Copyright (c) 1999, 2000, 2001, 2002, 2004, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
