//
// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
//	ModTermElement.cpp -- ModTermElement の実装
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

#include "ModCharString.h"	 // 数値から文字列への変換
#include "LibUna/ModTermElement.h"

_UNA_USING

// 検索語語形の区切り文字
const ModUnicodeChar ModTermElement::termSeparator(' '); // ASCIIのスペース

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

	// mapのinsertの戻り値
	ModPair<ModTermPoolMap::Iterator, ModBoolean> ret;

	// 登録用キー
	ModUnicodeString key = element.getTermNorm();

	// マップに登録
	ret = map.insert(key, element);

	// 自分(Vector)にtermIndexを登録
	ModTermIndex termIndex(&((*(ret.first)).second));
	this->pushBack(termIndex);
}

//
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
