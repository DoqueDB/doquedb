// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedWindowNodeTemplate.h -- Simple/Operator Window のテンプレートクラス
// 
// Copyright (c) 2002, 2004, 2005, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedWindowNodeTemplate_H__
#define __ModInvertedWindowNodeTemplate_H__

#include "ModInvertedWindowBaseNode.h"
#include "ModInvertedLocationListIterator.h"

//
// CLASS
// ModInvertedWindowNodeTemplate -- Sinple/Operator Windowノードのテンプレートクラス
//
// NOTES
//
template<ModSize OScore, ModSize UScore, char Prefix, class Type1, class Type2>
class
ModInvertedWindowNodeTemplate
	: public ModInvertedWindowBaseNode
{
public :

	ModInvertedWindowNodeTemplate();
	ModInvertedWindowNodeTemplate(const NodeType nType,
				const ModUInt32 resultType_);

	// sortFactor の計算
	virtual ModSize calcSortFactor();

	// 演算子を表わす文字列を返す
	virtual void prefixString(ModUnicodeString& prefix,
							  const ModBoolean withCalOrCombName,
							  const ModBoolean withCalOrCombParam) const;

	virtual ModBoolean reevaluate(ModInvertedDocumentID);
	virtual ModBoolean reevaluate(DocumentID,
								  LocationIterator*&,
								  ModSize& uiTF_,
								  ModInvertedQueryNode* = 0);
};

//
// FUNCTION protected
// ModInvertedWindowTempalte::ModInvertedWindowNodeTemplate -- コンストラクタ
//
// NOTES
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
template<ModSize OScore, ModSize UScore, char Prefix, class Type1, class Type2>
inline
ModInvertedWindowNodeTemplate<OScore, UScore, Prefix, Type1, Type2>
	::ModInvertedWindowNodeTemplate()
{}

//
// FUNCTION protected
// ModInvertedWindowNodeTemplate::ModInvertedWindowNodeTemplate -- コンストラクタ
//
// NOTES
// 積集合(AND)ノードを生成する。NodeType引数付き。
// OrderedDistanceNode や WindowNode から呼び出される。
//
// ARGUMENTS
// const NodeType nType
//      ノードタイプ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
template<ModSize OScore, ModSize UScore, char Prefix, class Type1, class Type2>
inline
ModInvertedWindowNodeTemplate<OScore, UScore, Prefix, Type1, Type2>
	::ModInvertedWindowNodeTemplate(const NodeType nType,
		const ModUInt32 resultType_)
		: ModInvertedWindowBaseNode(nType,resultType_)
{}

//
// FUNCTION public
// ModInvertedWindowNodeTemplate::calcSortFactor -- sortFactor の計算
//
// NOTES
// sortChildren() 関数で使用する sortFactor メンバ変数を計算する。
//
// QueryOperatorAndNode::calcSortFactor() をオーバライドしている。
// 各子ノードの sortFactor の和が sortFactor となる。
//
// ARGUMENTS
// なし
//
// RETURN
// 計算した sortFactor 値。
//
template<ModSize OScore, ModSize UScore, char Prefix, class Type1, class Type2>
inline ModSize
ModInvertedWindowNodeTemplate<OScore, UScore, Prefix, Type1, Type2>
	::calcSortFactor()
{
	if (this->sortFactor == 0) {
		// まだ計算していないのなら、計算する
		AndNode::calcSortFactor();
		if (this->sortFactor == AndNode::MaxSortFactor) {
			// 自分以下のノードにRegexが含まれる。
			// 既に上限に達しているのでそのまま返す
			return this->sortFactor;
		}

		// 位置検査のための係数
		if (BaseNode::getType() & 
				ModInvertedAtomicNode::unorderedNode) {
			this->sortFactor = ModSize((UScore/10)*this->sortFactor);
		} else {
			this->sortFactor = ModSize((OScore/10)*this->sortFactor);
		}
	}
	return this->sortFactor;
}

//
// FUNCTION public
// ModInvertedWindowNodeTemplate::prefixString -- 演算子を表わす文字列を返す
//
// NOTES
// QueryNodeで定義された内容をオーバライドする
//
// ARGUMENTS
// ModString& prefix
//  演算子を表わす文字列を返す(結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
template<ModSize OScore, ModSize UScore, char Prefix, class Type1, class Type2>
inline void
ModInvertedWindowNodeTemplate<OScore, UScore, Prefix, Type1, Type2>
	::prefixString(
		ModUnicodeString& prefix,
		const ModBoolean withCalOrCombName,
		const ModBoolean withCalOrCombParam) const
{
	ModOstrStream out;

	out << '#' << Prefix << "window[" << this->minimalDistance << ','

		<< this->maximalDistance << ',';
	if (ModInvertedAtomicMask(BaseNode::getType()) &
			ModInvertedAtomicNode::unorderedNode) {
		out << 'u';
	} else {
		out << 'o';
	}

	prefix = out.getString();

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
// ModInvertedWindowtemplate::reevaluate -- 正確な再 evaluate
//
// NOTES
// 粗い evaluate 後の、正確な再 evaluate。出現位置の検査のみを行なう。
//
// nodeTypeが orderedOperatorWindowNode の場合は
// OrderedOperatorWindowLocationListIterator で順序付き位置検査を行なう。
//
// nodeTypeが unorderedOperatorWindowNode の場合は
// UnorderedOperatorWindowLocationListIterator で順序無視位置検査を行なう。
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//  文書ID
//
// RETURN
// 出現位置検査を満足すれば ModTrue、しなければ ModFalse
//
// EXCEPTIONS
// ModInvertedErrorRetrieveFail
//ノードタイプが、orderedOperatorWindowNode でも
// UnorderedOperatorLocationIterator でもなかった。
//
template<ModSize OScore, ModSize UScore, char Prefix, class Type1, class Type2>
inline ModBoolean
ModInvertedWindowNodeTemplate<OScore, UScore, Prefix, Type1, Type2>
	::reevaluate(ModInvertedDocumentID documentID)
{
	NodeType myType(ModInvertedAtomicMask(BaseNode::getType()));
	ModBoolean result = ModFalse;

	// Type1とType2で完全に実装を分けた
	
	if (myType & ModInvertedAtomicNode::unorderedNode)
	{
		Type2* iterator = static_cast<Type2*>(getFreeList());
		if (iterator == 0)
		{
			iterator = new Type2(this);
			iterator->reserve(children.getSize());
		}
		LocationIterator::AutoPointer p = iterator;

		// 常に document-at-a-time
		// 子ノードそれぞれについて正確に再評価すると共に、出現位置も得る
		ModSize childSize = children.getSize();
		ModInvertedLocationListIterator* childLocation = 0;
		for (ModSize i = 0 ; i < childSize ; ++i)
		{
			if (children[i]->reevaluate(documentID, childLocation)
				!= ModTrue)
			{
				return ModFalse;
			}
			iterator->pushIterator(childLocation);
		}

		iterator->initialize(minimalDistance, maximalDistance);
		result = (iterator->isEnd() == ModFalse) ? ModTrue : ModFalse;
	}
	else
	{
		Type1* iterator = static_cast<Type1*>(getFreeList());
		if (iterator == 0)
		{
			iterator = new Type1(this);
			iterator->reserve(children.getSize());
		}
		LocationIterator::AutoPointer p = iterator;

		// 常に document-at-a-time
		// 子ノードそれぞれについて正確に再評価すると共に、出現位置も得る
		ModSize childSize = children.getSize();
		ModInvertedLocationListIterator* childLocation = 0;
		for (ModSize i = 0 ; i < childSize ; ++i)
		{
			if (children[i]->reevaluate(documentID, childLocation)
				!= ModTrue)
			{
				return ModFalse;
			}
			iterator->pushIterator(childLocation);
		}

		iterator->initialize(minimalDistance, maximalDistance);
		result = (iterator->isEnd() == ModFalse) ? ModTrue : ModFalse;
	}
	
	return result;
}

//
// FUNCTION protected
// ModInvertedWindowNodeTemplate::reevaluate -- 正確な再 evaluate と位置情報の獲得
//
// NOTES
// 粗い evaluate を前提として、正確な再 evaluate を行ない、満足の場合、
// 与えられた出現位置情報オブジェクトに出現位置情報を格納する。
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//      文書ID
// ModInvertedLocationListIterator*& locations
//      出現位置反復子へのポインタ (結果格納用)
// ModSize& uiTF_
//		(位置情報リストを取得できない場合) TF
// ModInvertedQueryNode* givenEndNode
//      ここでは未使用。orderedDistanceでのみ使用。
//
// RETURN
// 出現位置検査を満足すれば ModTrue、しなければ ModFalse
//
// EXCEPTIONS
// ModInvertedErrorRetrieveFail
//      ノードタイプが、orderedOperatorWindowNode でも
//       UnorderedOperatorLocationIterator でもなかった。
//
template<ModSize OScore, ModSize UScore, char Prefix, class Type1, class Type2>
inline ModBoolean
ModInvertedWindowNodeTemplate<OScore, UScore, Prefix, Type1, Type2>
	::reevaluate(ModInvertedDocumentID documentID,
				 LocationIterator*& locations,
				 ModSize& uiTF_,
				 ModInvertedQueryNode* givenEndNode)
{
	NodeType myType(ModInvertedAtomicMask(BaseNode::getType()));
	ModBoolean result = ModFalse;

	// Type1とType2で完全に実装を分けた
	
	if (myType & ModInvertedAtomicNode::unorderedNode)
	{
		Type2* iterator = static_cast<Type2*>(getFreeList());
		if (iterator == 0)
		{
			iterator = new Type2(this);
			iterator->reserve(children.getSize());
		}
		LocationIterator::AutoPointer p = iterator;

		// 常に document-at-a-time
		// 子ノードそれぞれについて正確に再評価すると共に、出現位置も得る
		ModSize childSize = children.getSize();
		ModInvertedLocationListIterator* childLocation = 0;
		for (ModSize i = 0 ; i < childSize ; ++i)
		{
			if (children[i]->reevaluate(documentID, childLocation)
				!= ModTrue)
			{
				return ModFalse;
			}
			iterator->pushIterator(childLocation);
		}

		iterator->initialize(minimalDistance, maximalDistance);
		if (iterator->isEnd() == ModFalse)
		{
			locations = p.release();
			result = ModTrue;
		}
	}
	else
	{
		Type1* iterator = static_cast<Type1*>(getFreeList());
		if (iterator == 0)
		{
			iterator = new Type1(this);
			iterator->reserve(children.getSize());
		}
		LocationIterator::AutoPointer p = iterator;

		// 常に document-at-a-time
		// 子ノードそれぞれについて正確に再評価すると共に、出現位置も得る
		ModSize childSize = children.getSize();
		ModInvertedLocationListIterator* childLocation = 0;
		for (ModSize i = 0 ; i < childSize ; ++i)
		{
			if (children[i]->reevaluate(documentID, childLocation)
				!= ModTrue)
			{
				return ModFalse;
			}
			iterator->pushIterator(childLocation);
		}

		iterator->initialize(minimalDistance, maximalDistance);
		if (iterator->isEnd() == ModFalse)
		{
			locations = p.release();
			result = ModTrue;
		}
	}
	
	return result;
}

#endif //__ModInvertedWindowNodeTemplate_H__

//
// Copyright (c) 2002, 2004, 2005, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
