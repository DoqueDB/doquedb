//
// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
//	ModTermElement.cpp -- ModTermElement の実装
// 
// Copyright (c) 2000, 2005, 2009, 2023 Ricoh Company, Ltd.
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

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "ModCharString.h"	 // 数値から文字列への変換
#include "ModTermElement.h"

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
// ModTermElement::getFormula -- 検索語式の獲得
//
// NOTES
//	 ModQueryParser用の検索語式を生成して返す。
//
// ARGUMENTS
//	 ModTermMatch match	検索語の照合法
//	 ModBoolean ranking	真ならランキング検索用
//
// RETURN
//	 検索式。	例。#scale[1.0](#window[1,10,u](pen,sword))
//
// EXCEPTIONS
//	 なし
//
ModUnicodeString
ModTermElement::getFormula(
	ModTermMatch match,			 // 検索語の照合法
	const ModUnicodeString& calculator_,	// スコア計算器
	ModBoolean ranking) const // 真ならランキング検索用
{
	ModUnicodeString formula;	// 検索式
	formula.clear();

	// #scaleオペレータ
	ModUnicodeString operatorScale;
	operatorScale.clear();

	// 一致条件
	if (matchMode != voidMatch)
		// 一致条件はメンバー変数の値が優先される
		match = matchMode;

	if(ranking == ModTrue) {
		// スケール値は scale と weight 属性の積。
	double scaleValue;
	if (calculator_.getLength() == 0)
		scaleValue = (scale ? scale : 1) * (weight ? weight : 1);
	else
		scaleValue = (scale ? scale : 1);
		if(scaleValue != 1) {
			operatorScale.append(ModUnicodeString("#scale["));
			operatorScale.append(_double2string(scaleValue));
			operatorScale.append(ModUnicodeString("]("));
			formula.append(operatorScale);
		}
	}

	// #windowオペレータ
	ModUnicodeString operatorWindow;
	operatorWindow.clear();
	// 順序指定(ordered)
	if(paramProximity > 0) {
		ModCharString c;
		c.format("#window[1,%d,o](", paramProximity);
		ModUnicodeString u(c);
		operatorWindow.append(u);
		formula.append(operatorWindow);
	// 順序不定(unorderd)
	} else if(paramProximity < 0) {
		ModCharString c;
		c.format("#window[1,%d,u](", -paramProximity);
		ModUnicodeString u(c);
		operatorWindow.append(u);
		formula.append(operatorWindow);
	} else {
			;
	}

	// #termオペレータ
	ModUnicodeString operatorTermMode("#term");
	ModUnicodeString termMode;

	// 検索語照合法
	if(match == exactMatch) {
		termMode = "[e";
	} else if(match == simpleMatch) {
		termMode = "[s";
	} else if(match == stringMatch) {
		termMode = "[n";
	} else if(match == approximateMatch) {
		termMode = "[a";
	} else if(match == multiMatch) {
		termMode = "[m";
	} else if(match == headMatch) {
		termMode = "[h";
	} else if(match == tailMatch) {
		termMode = "[t";
	} else { // 無効値
		termMode = "[v";
	}

	// スコア計算器とパラメタ
	ModUnicodeString parameter;
	parameter.clear();
	if (calculator_.getLength() == 0) {
		if(weight) {
			parameter.append(ModUnicodeString(",NormalizedOkapiTf:"));
			parameter.append(_double2string(paramScore));
			parameter.append(':');
			parameter.append(_double2string(paramLength));
		} else {
			parameter.append(",NormalizedOkapiTfIdf:");
			parameter.append(_double2string(paramScore));
			parameter.append(':');
			parameter.append(_double2string(paramWeight));
			parameter.append(':');
			parameter.append(_double2string(paramLength));
		}
	} else {
		parameter.append(',');
		parameter.append(calculator_);
	}
#ifdef V1_6
	ModUnicodeString name = langSpec.getName();
	if(ranking == ModTrue) {
		termMode = termMode + parameter + ModUnicodeString(",") + name + ModUnicodeString("]");
	} else {
		termMode = termMode + ModUnicodeString(",,") + name + ModUnicodeString("]");
	}
#else
	if(ranking == ModTrue) {
		termMode = termMode + parameter + ModUnicodeString("]");
	} else {
		termMode = termMode + ModUnicodeString("]");
	}
#endif
	operatorTermMode += termMode;

	//
	// エスケープ文字等
	//
	const ModUnicodeChar charComma = ',';
	const ModUnicodeChar charRP = ')';
	const ModUnicodeChar charLP = '(';
	const ModUnicodeChar charRB = ']';
	const ModUnicodeChar charLB = '[';
	const ModUnicodeChar charSharp = '#';
	const ModUnicodeChar charBackSlash = '\\';

	ModUnicodeString tmpStr;
	tmpStr.clear();
	tmpStr.append(operatorTermMode);
	tmpStr.append(charLP);

	// 語形の処理
	ModSize len = string.getLength();
	for(ModSize j = 0; j < len; j++) {

		// エスケープ文字の処理 ","→ "\,"
		if(string[j] == charComma) {
			tmpStr.append(charBackSlash);
			tmpStr.append(charComma);

		// エスケープ文字の処理 ")"→ "\)"
		} else if(string[j] == charRP) {
			tmpStr.append(charBackSlash);
			tmpStr.append(charRP);

		// エスケープ文字の処理 "("→ "\("
		} else if(string[j] == charLP) {
			tmpStr.append(charBackSlash);
			tmpStr.append(charLP);

		// エスケープ文字の処理 "]"→ "\]"
		} else if(string[j] == charRB) {
			tmpStr.append(charBackSlash);
			tmpStr.append(charRB);

		// エスケープ文字の処理 "["→ "\["
		} else if(string[j] == charLB) {
			tmpStr.append(charBackSlash);
			tmpStr.append(charLB);

		// エスケープ文字の処理 "#"→ "\#"
		} else if(string[j] == charSharp) {
			tmpStr.append(charBackSlash);
			tmpStr.append(charSharp);

		// エスケープ文字の処理 "\"→ "\\"
		} else if(string[j] == charBackSlash) {
			tmpStr.append(charBackSlash);
			tmpStr.append(charBackSlash);

		// 検索語セパレータ
		} else if(string[j] == termSeparator) {

				// 近接演算子が付加されない場合
			if(paramProximity == 0) {
				// 先頭または末尾のセパレータを削除
				if(j == 0 || j == len - 1) {
					;
				// 文字列中のセパレータ
				} else {
					// 英文字 + セパレータ + 英文字 の場合はセパレータを残す
					if(ModUnicodeCharTrait::isAlphabet(string[j-1])
					&& ModUnicodeCharTrait::isAlphabet(string[j+1])) {
						tmpStr.append(string[j]);
					// 数字 + セパレータ + 数字 の場合はセパレータを残す
					} else if(ModUnicodeCharTrait::isDigit(string[j-1])
						&& ModUnicodeCharTrait::isDigit(string[j+1])) {
						tmpStr.append(string[j]);

					// それ以外はセパレータを削除
					} else {
						;
					}
				}
				// 近接演算子が付加された場合
			} else {
				// 一つの#termの終わり
				tmpStr.append(charRP);
				tmpStr.append(charComma);
				// 次の#term演算子
				tmpStr.append(operatorTermMode);
				tmpStr.append(charLP);
			}
		} else {
			tmpStr.append(string[j]);
		}
	}
	// termの終わり
	tmpStr.append(charRP);

	// 検索式の完成
	formula.append(tmpStr);
	if(operatorWindow.getLength() != 0) formula.append(')');
	if(operatorScale.getLength() != 0)	formula.append(')');

	return formula;
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
	ModUnicodeString key = element.getString();

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

	// 登録されているなら出現頻度と選択値に１を加算。
	} else {
#if 1
		(map[key]).setTf((map[key]).getTf() + element.getTf());
		(map[key]).setTsv((map[key]).getTsv() + element.getTsv());
#else
		(map[key]).setTf((map[key]).getTf()+1);
		(map[key]).setTsv((map[key]).getTsv()+1);
#endif
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
// Copyright (c) 2000, 2005, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
