// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Condition.h -- 
// 
// Copyright (c) 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VECTOR2_CONDITION_H
#define __SYDNEY_VECTOR2_CONDITION_H

#include "Vector2/Module.h"
#include "LogicalFile/TreeNodeInterface.h"

#include "ModTypes.h"
#include "ModVector.h"

_SYDNEY_BEGIN

namespace Common
{
	class Data;
}

_SYDNEY_VECTOR2_BEGIN

//
//	CLASS
//	Vector2::Condition -- Vector2の検索条件を解析する
//
//	NOTES
//	【オープンオプション】
//	条件は、等号や不等号などの関係演算子、AND、OR の3種類がある。
//	木構造の出現位置はルートから見て OR AND 関係演算子 の順番である。
//	つまりAND条件の中にOR条件が入れ子で存在することはない。
//	また OR や AND がないこともある。
//	※ いちおうROOT-AND-ORもサポートしている。
//
//	・関係演算子
//	Type==Equalsなど
//		Operand[0] Type==Field, Value==フィールド番号
//		Operand[1] Type==Variable or ConstantValue Value==値
//
//	・AND条件
//	AND条件の下には関係演算子がある。
//	Type==And
//		Operand[0] 関係演算子 1
//		Operand[1] 関係演算子 2
//		...
//
//	・OR条件
//	OR条件の直下にはAND条件と関係演算子がある。それらの順番は不定。
//	Type==Or
//		Operand[0] ANDまたは関係演算子 1
//		Operand[1] ANDまたは関係演算子 2
//		...
//
//	【解析結果】
//	Btree2は、解析できる条件に対しては、0件取得条件でもtrueを返す。
//	解析できない条件に対しては、Exceptionではなくてfalseを返す。
//	Vector2も、これに従うことにする。
//	※ ROWIDの範囲以外だったり、整数でなくても扱える。
//
//	【条件の表現】
//	初期状態	min=max=IllegalKey
//
//	k			min=max=k
//	k以上		min=k, max=IllegalKey
//	k以下		min=0, max=k
//	全件取得	min=0, max=IllegalKey
//	0件取得		min=max=IllegalKey
//
//	not k		未サポート
//
//	【条件の解析とマージ】
//	解析は、解析済み条件から以降の条件のマージが不要とわかっても、
//	文法チェックのために最後まで続ける。
//	ただし、全件取得条件が確定した場合は、その時点でfalseを返す。
//	マージは、できるだけ各条件の解析が終わる毎に実行する。
// 	最後にまとめてマージすると、
// 	条件がたくさんあった時に、メモリがたくさん必要になるため。
//	マージの実行判定も、merge関数の中で判定する。
//
//	【今後の課題】
//	1. 無駄なANDマージをしない
//	ORノードから呼ばれるANDノードのマージは、関係演算子の数によらずに
//	ANDノードが解析し終わってからマージしている。
//	しかし、ORノードから呼ばれるANDノードの関係演算子ノード数が多い時は、
//	適当な間隔でその時点までに解析されたAND条件が、
//	OR条件に含まれているか確認した方がよい。
//	もし含まれていれば、以降のANDマージは不要になるため。
//	ただし、OR条件の数が多いと含まれているかの確認コストも高くなるので、
//	OR条件とAND条件の数を比較して、動的に処理を変えるのが望ましい。
//
//	2. 全件取得条件はfalseを返す。
//	-> 詳しくはLogicalInterface.h
//
//	3. 複数条件を持ったand条件同士のマージ
//	条件の先頭同士からマージするのではなく、
//	真ん中同士を比較して、二分探索しながら比較を繰り返す。
//
class Condition
{
public:

	// コンストラクタ
	Condition(const LogicalFile::TreeNodeInterface* pCondition_,
			  LogicalFile::TreeNodeInterface::Type iParent_
			  = LogicalFile::TreeNodeInterface::Undefined,
			  int iHierarchy_ = 0);
	// デストラクタ
	~Condition();

	// 検索条件を解析する
	bool parseRoot();

	// 解析された検索条件の数を返す
	int getConditionSize() const;

	// 解析結果を取得する
	ModUInt32 getMin(int pos_ = 0) const;
	ModUInt32 getMax(int pos_ = 0) const;
	
private:

	// ORノードを解析して結果を返す
	bool parseOrNode();

	// ANDノードを解析して結果を返す
	bool parseAndNode();

	// 関係演算子リーフを解析して結果を返す
	bool parseOperatorLeaf(const LogicalFile::TreeNodeInterface* pCondition_);
	
	// Alternate two terms.
	void alternateTerm(const LogicalFile::TreeNodeInterface*& pFirst_,
					   const LogicalFile::TreeNodeInterface*& pSecond_,
					   LogicalFile::TreeNodeInterface::Type& eMatch_);
	
	// OR条件でマージする
	void mergeOrCondition(ModUInt32 newMin_,
						  ModUInt32 newMax_);

	// AND条件でマージする
	void mergeAndCondition(ModUInt32 newMin_,
						   ModUInt32 newMax_);
	// 条件数がわからない条件をAND条件でマージする
	void mergeAndCondition(Condition& other_);
	// 複数の条件をAND条件でマージする
	void mergeAndConditions(Condition& other_);

	// 一つの条件で表せるか調べる
	void checkCondition();
	
	// 関係演算子リーフかどうか
	static bool isOperatorLeaf(const LogicalFile::TreeNodeInterface* pCondition_);

	// 整数に丸める
	static ModInt64 round(const Common::Data& cData_,
						  bool& integer_);

	// 条件を二分探索する
	ModUInt32 binsearch(ModUInt32 key_,
						bool& range_)
		{ return binsearch(key_, range_, 0, getConditionSize()-1); }
	ModUInt32 binsearch(ModUInt32 key_,
						bool& range_,
						ModUInt32 low_,
						ModUInt32 high_);


	// 条件を表す木構造へのポインタ
	const LogicalFile::TreeNodeInterface* m_pNode;
	// 親ノードの種類
	LogicalFile::TreeNodeInterface::Type m_iParent;
	// Rootノードからの階層
	int m_iHierarchy;
	
	// 単純な検索条件か
	bool m_bSimple;
	// 全件取得条件がOR条件にある
	bool m_bAll;
	// 0件取得条件がAND条件にある
	bool m_bZero;

	// 最小値と最大値
	ModUInt32 m_uiMin;
	ModUInt32 m_uiMax;
	// 最小値と最大値の配列
	ModVector<ModUInt32> m_vecMin;
	ModVector<ModUInt32> m_vecMax;
};

_SYDNEY_VECTOR2_END
_SYDNEY_END

#endif // __SYDNEY_VECTOR2_CONDITION_H

//
//	Copyright (c) 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
