// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
//	ModTermElement.cpp -- ModTermElement の実装
// 
// Copyright (c) 2000, 2005, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Utility";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Utility/ModTermElement.h"

#include "ModUnicodeOstrStream.h"
#include "ModCharString.h"	 // 数値から文字列への変換

_TRMEISTER_USING
_TRMEISTER_UTILITY_USING

// 検索語語形の区切り文字
const ModUnicodeChar ModTermElement::termSeparator(' '); // ASCIIのスペース

//
// FUNCTION static
// _double2string -- 倍精度浮動小数を文字列に変換
//
// NOTES
//	 倍精度浮動少数を文字列に変換
//
// ARGUMENTS
//	 double value	倍精度浮動小数値
//
// RETURN
//	 ModUnicodeString	作成した文字列
//
// EXCEPTIONS
//
static ModUnicodeString _double2string(double value)
{
	ModUnicodeString string;
	string.clear();

	if(value < 0) {
		string.append('-');
		value *= -1;
	}

	int n1 = (int)value;
	double d = value - (double)n1;
	int n2 = (int)(1000000 * d);

	ModCharString charString;
	charString.format("%d.%06d", n1, n2);
	ModUnicodeString u(charString);

	string.append(u);
	return string;
}

//
// FUNCTION public
// ModTermElement::ModTermElement -- デフォルトのコンストラクタ
//
// NOTES
//   デフォルトのコンストラクタ。
//   出現頻度および選択値は1に初期化される。他は0に初期化される。
//
// ARGUMENTS
//   なし
//
// RETURN
//   なし
//
// EXCEPTIONS
//   なし
//
ModTermElement::ModTermElement()
{
	type		   = 0;
	scale          = 0;
	weight         = 0;
	tf             = 1;
	tsv            = 1;
	df             = 0;
	sdf            = 0;
	twv			   = 0;
	paramWeight    = 0;
	paramScore     = 0;
	paramLength    = 0;
	paramProximity = 0;
	matchMode	   = voidMatch;
	position	   = -1;
}

//
// FUNCTION public
// ModTermElement::ModTermElement -- 単独語のコンストラクタ
//
// NOTES
//   単独語のコンストラクタ。
//   出現頻度および選択値は1に初期化される。他は0に初期化される。
//
// ARGUMENTS
//   const ModUnicodeString& _string  単独語の語形
//   const ModTermType _type          単独語のパタン種別
//   const double _twv                単独語のパタン重み
//
// RETURN
//   なし
//
// EXCEPTIONS
//   なし
//
ModTermElement::ModTermElement(
	const ModUnicodeString& _string,
	const ModTermType _type,
	const double _twv)
	:
	type(_type), string(_string), twv(_twv), originalString(_string)
{
	scale          = 0;
	weight         = 0;
	tf             = 1;
	tsv            = 1;
	df             = 0;
	sdf            = 0;
	paramWeight    = 0;
	paramScore     = 0;
	paramLength    = 0;
	paramProximity = 0;
	// langSpecはデフォルトコンストラクタによる
	matchMode	   = voidMatch;
	position	   = -1;
}

//
// FUNCTION public
// ModTermElement::ModTermElement -- 隣接語のコンストラクタ
//
// NOTES
//   隣接語のコンストラクタ
//   出現頻度および選択値は1に初期化される。他は0に初期化される。
// 
//   隣接語の語形は前置語と後置語を区切り文字を介して連接したもの
//   隣接語のタイプは前置語/後置語のパタン種別(t1,t2)を基に合成(100*t1+t2)
//   隣接語の重みづけ値は後置語のパタン重み
//
// ARGUMENTS
//   const ModUnicodeString& _string1  前置語の語形
//   const ModTermType _type1          前置語のパタン種別
//   const double _twv1                前置語のパタン重み
//   const ModUnicodeString& _string2  後置語の語形
//   const ModTermType _type2          後置語のパタン種別
//   const double _twv2                後置語のパタン重み
//
// RETURN
//   なし
//
// EXCEPTIONS
//   なし
//
ModTermElement::ModTermElement(
	const ModUnicodeString& _string1,
	const ModTermType _type1,
	const double _twv1,
	const ModUnicodeString& _string2,
	const ModTermType _type2,
	const double _twv2)
	: string(_string1), type(-(100 * _type1 + _type2)), twv(_twv2)
{
	string.append(ModTermElement::termSeparator);
	string.append(_string2);
	originalString = string;
	scale          = 0;
	weight         = 0;
	tf             = 1;
	tsv            = 1;
	df             = 0;
	sdf            = 0;
	paramWeight    = 0;
	paramScore     = 0;
	paramLength    = 0;
	paramProximity = 0;
	// langSpecはデフォルトコンストラクタによる
	matchMode	   = voidMatch;
	position	   = -1;
}

//
// FUNCTION public
// ModTermElement::~ModTermElement -- デストラクタ
//
// NOTES
//   デストラクタ
//   operatorNode にノードが設定されていたら解放する
//
// ARGUMENTS
//   なし
//
// RETURN
//   なし
//
// EXCEPTIONS
//   なし
//
ModTermElement::~ModTermElement()
{
}

//
//	FUNCTION public
//	ModTermElement::ModTermElement -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const ModTermElement& src
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ModTermElement::ModTermElement(const ModTermElement& src)
{
	operator = (src);
}

//
//	FUNCTION public
//	ModTermElement::operator =
//		-- 代入演算子
//
//	NOTES
//
//	ARGUMENTS
//	const ModTermElement& src
//		代入元
//
//	RETURN
//	ModTermElement&
//		自身への参照
//
//	EXCEPTIONS
//
ModTermElement&
ModTermElement::operator = (const ModTermElement& src)
{
	string = src.string;
	originalString = src.originalString;
	type = src.type;
	scale = src.scale;
	weight = src.weight;
	tf = src.tf;
	df = src.df;
	sdf = src.sdf;
	twv = src.twv;
	tsv = src.tsv;
	paramWeight = src.paramWeight;
	paramScore = src.paramScore;
	paramLength = src.paramLength;
	paramProximity = src.paramProximity;
	langSpec = src.langSpec; 
	matchMode = src.matchMode;
	formula = src.formula;
	position = src.position;

	return *this;
}

//
// FUNCTION public
// ModTermElement::getFormula -- 検索語式の獲得
//
// NOTES
//	ModQueryParser用の検索語式を生成して返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	検索式。	例。#window[1,10,u](#term(pen),#term(sword))
//
// EXCEPTIONS
//	なし
//
const ModUnicodeString&
ModTermElement::getFormula()
{
	if (formula.getLength() == 0)
	{
		// まだ作成されていないので、作成する
		
		ModUnicodeOstrStream s;
		const ModUnicodeChar* b = string;
		
		// #windowオペレータを利用したか？
		bool isWindow = false;

		if (paramProximity != 0)
		{
			// 近傍検索を利用するかチェックする
			const ModUnicodeChar* p = b;
			while (*p != 0)
			{
				if (*p == termSeparator)
				{
					isWindow = true;
					break;
				}
			}

			if (isWindow)
			{
				if (paramProximity > 0) {
					// 順序指定(ordered)
					s << "#window[1," << paramProximity << ",o](";
				} else {
					// 順序不定(unorderd)
					s << "#window[1," << -paramProximity << ",u](";
				}
			}
		}
		
		// #termオペレータ
		ModUnicodeString operatorTermMode(ModUnicodeString("#term"));

		// 検索語照合法
		switch (matchMode) {
		case exactMatch:
			operatorTermMode.append(ModUnicodeString("[e,,"));
			break;
		case simpleMatch:
			operatorTermMode.append(ModUnicodeString("[s,,"));
			break;
		case stringMatch:
			operatorTermMode.append(ModUnicodeString("[n,,"));
			break;
		case multiMatch:
			operatorTermMode.append(ModUnicodeString("[m,,"));
			break;
		case headMatch:
			operatorTermMode.append(ModUnicodeString("[h,,"));
			break;
		case tailMatch:
			operatorTermMode.append(ModUnicodeString("[t,,"));
			break;
		default:
			operatorTermMode.append(ModUnicodeString("[v,,"));	// 無効な値
		}

		// 言語
		operatorTermMode.append(langSpec.getName());
		operatorTermMode.append(']');

		s << operatorTermMode << "(";

		// 語形の処理
		const ModUnicodeChar* p = b;
	
		while (*p != 0)
		{
			if (*p == ',' ||
				*p == ')' ||
				*p == '(' ||
				*p == ']' ||
				*p == '[' ||
				*p == '#' ||
				*p == '\\')
			{
				s << '\\' << *p;
			}
			else if (*p == termSeparator)
			{
				if (paramProximity == 0) {
					// 近接演算子が付加されない場合
				
					if (p == b || *(p+1) == 0) {
						// 先頭または末尾のセパレータを削除
						;
					} else {
						// 文字列中のセパレータ
					
						if (ModUnicodeCharTrait::isAlphabet(*(p-1))
							&& ModUnicodeCharTrait::isAlphabet(*(p+1)))
						{
							// 英文字 + セパレータ + 英文字 の場合は
							// セパレータを残す
							s << *p;
						}
						else if (ModUnicodeCharTrait::isDigit(*(p-1))
								 && ModUnicodeCharTrait::isDigit(*(p+1)))
						{
							// 数字 + セパレータ + 数字 の場合は
							// セパレータを残す
							s << *p;
						}
						else
						{
							// それ以外はセパレータを削除
							;
						}
					}
				} else {
					
					// 近接演算子が付加された場合
				
					// 一つの#termの終わり
					s << "),";
					// 次の#term演算子
					s << operatorTermMode << "(";
				}
			}
			else
			{
				s << *p;
			}

			++p;
		}
	
		// termの終わり
		s << ")";

		// 検索式の完成
		if (isWindow) s << ")";

		// メンバー変数に設定する
		formula = s.getString();
	}

	return formula;
}

//
//	FUNCTION public
//	ModTermElement::getCalculator -- スコア計算器を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		スコア計算器
//
//	EXCEPTIONS
//
ModUnicodeString
ModTermElement::getCalculator() const
{
	ModUnicodeString parameter;
	
	if (weight) {
		parameter.append(ModUnicodeString("NormalizedOkapiTf:"));
		parameter.append(_double2string(paramScore));
		parameter.append(':');
		parameter.append(_double2string(paramLength));
	} else {
		parameter.append(ModUnicodeString("NormalizedOkapiTfIdf:"));
		parameter.append(_double2string(paramScore));
		parameter.append(':');
		parameter.append(_double2string(paramWeight));
		parameter.append(':');
		parameter.append(_double2string(paramLength));
	}

	return parameter;
}

//
// FUNCTION public
// ModTermPool::insertTerm -- 検索語の登録
//
// NOTES
//	 検索語プールへの登録を行う。
//	・既に登録済みの検索語の場合
//		- 検索語の出現頻度および選択値に1を加える
//	・新規の検索語の場合
//		- 要素数が maxSize に満たない場合
//			・検索語を登録する
//		- 要素数が maxSize に達した場合
//			・isHeapがModFalseの場合は、ヒープを構成し最小重みを獲得。
//			・登録語の選択値がマップ中の最小選択値より小さい場合は登録しない
//			・それ以外は最小選択値を持つ検索語と登録語を置換する。すなわち
//				- ヒープをポップしそのインデックスが指す検索語をマップから削除。
//				- 登録語をマップに登録しそのインデックスをヒープにプッシュ。
//
//	 同一語形の検索語群をプールに登録しない時、
//	 各検索語の属性は登録により変化しない。
//	 例えば検索語マップから検索語プールに検索語を移すときなど。
//
//	 同一語形の検索語群をプールに登録する場合、
//	 検索語の出現頻度と選択値は登録回数-1だけ増加する。
//
// ARGUMENTS
//	 const ModTermElement& element 登録語
//
// RETURN
//	 なし
//
// EXCEPTIONS
//	 なし
//
void
ModTermPool::insertTerm(const ModTermElement& element) {

	// 空のプールの場合はなにもしない
	if(maxSize == 0) {
		return;
	}

	// mapのinsertの戻り値
	ModPair<ModTermPoolMap::Iterator, ModBoolean> ret;

	// 登録用キー
	const ModUnicodeString& key = element.getString();

	// map検索用Iterator
	ModTermPoolMap::Iterator iterator = map.find(key);

	// mapには登録されていない
	if(iterator == map.end()) {

		// maxSizeに達していない
		if(map.getSize() < maxSize) {

			// マップに登録
			ret = map.insert(key, element);

			// 自分(Vector)にtermIndexを登録
			ModTermIndex termIndex(&((*(ret.first)).second));
			this->pushBack(termIndex);

			element.isBiGram() == ModTrue ? ++numBiGram : ++numUniGram;

		// maxSizeに達している
		} else {

			// 初回、あるいは、sortByTsv() が実施された場合など
			if(isHeap == ModFalse) {

				// ヒープを作成。
				ModMakeHeap(this->begin(), this->end(), ModGreater<ModTermIndex>());
				isHeap = ModTrue;

				// プール内の最小重みを minTsv にセットする。
				minTsv = (this->begin())->getTsv();
			}

			// 登録語の選択値がプールの最小選択値より小さい場合
			if(minTsv > element.getTsv()) {
				return; // 登録しない
			}

			// ポップ
			ModPopHeap(this->begin(), this->end(), ModGreater<ModTermIndex>());

			// ポップの結果 end - 1 の位置の選択値最小要素を削除する
			ModVector<ModTermIndex>::Iterator del = this->end() - 1;

			// mapを検索
			ModTermPoolMap::Iterator delIterator = map.find(del->getString());

			// mapから削除
			map.erase(delIterator);

			// 自分(Vectorから)
			this->erase(del);

			// mapに登録
			ret = map.insert(key, element);
			ModTermIndex termIndex(&((*(ret.first)).second));
			this->pushBack(termIndex);

			element.isBiGram() == ModTrue ? ++numBiGram : ++numUniGram;

			// Heapを再構成
			ModPushHeap(this->begin(), maxSize-1 , maxSize-1, termIndex,
								ModGreater<ModTermIndex>());

			// Heapを再構成したので minTsv を更新
			minTsv= (this->begin())->getTsv();
		}

	// 登録されているなら出現頻度と選択値を加算。
	} else {
		(*iterator).second.setTf(
			(*iterator).second.getTf() + element.getTf());
		(*iterator).second.setTsv(
			(*iterator).second.getTsv() + element.getTsv());
	}
}

//
// FUNCTION public
// ModTermPool::eraseTerm -- 指定選択値以下の検索語の消去
//
// NOTES
//	 引数で指定した選択値以下の検索語をプール中から削除する。
//
// ARGUMENTS
//	 double tsv	検索語の選択値
//
// RETURN
//	 なし
//
// EXCEPTIONS
//	 なし
//
void
ModTermPool::eraseTerm(double tsv) {

	// 空のプールの場合はなにもしない
	if(maxSize == 0) {
		return;
	}

	// 選択値が大きい順に
	this->sortByTsv();
	ModSize numErase = 0;
	for(ModTermPool::Iterator t = this->begin(); t != this->end(); t++) {
		// 削除対象ならば
		if(t->getTsv() <= tsv) {
			// 検索語数を更新
			if(t->isBiGram() == ModTrue) {
				--numBiGram;
			} else {
				--numUniGram;
			}
			++numErase;
			// マップから削除
			ModTermPoolMap::Iterator m = map.find(t->getString());
			map.erase(m);
		}
	}

	// ベクトル要素を削除
	if(numErase > 0) {
		this->erase(this->end() - numErase, this->end());
	}
}

//
// FUNCTION public
// ModTermPool::setDf -- DF値を設定する
//
// NOTES
//
// ARGUMENTS
//	 const ModUnicodeString& term -- 検索語(ModTermPoolMap のキー値)
//	 ModSize df -- 文書頻度
//
// RETURN
//	 bool
//		対象が存在した場合はtrue、存在しない場合はfalse
//
// EXCEPTIONS
//	 なし
//
bool
ModTermPool::setDf(const ModUnicodeString& term,
				   ModSize df)
{
	ModTermPoolMap::Iterator i = map.find(term);
	if (i == map.end()) return false;
	(*i).second.setDf(df);
	return true;
}

//
// Copyright (c) 2000, 2005, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
