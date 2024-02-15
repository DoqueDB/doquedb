// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
//  ModTermMap.cpp -- ModTermMap の実装
// 
// Copyright (c) 2000, 2005, 2023 Ricoh Company, Ltd.
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
#include "ModTermMap.h"

//
// FUNCTION public
// ModTermMap::insertTerm -- 検索語マップへの登録
//
// NOTES
//   文書への検索語の出現を検索語マップに登録する。
//   同一語形の検索語が登録された場合、そのタイプは最初に登録されたものとなる。
// 
// ARGUMENTS
//   const ModSize         docID  文書ID
//   const ModTermElement&  term  検索語
//
// RETURN
//   なし
//
// EXCEPTIONS
//   なし
//
void
ModTermMap::insertTerm(const ModSize docID, const ModTermElement& term)
{
  // ポスティングの要素(文書IDと出現頻度のペア)を生成
  ModTermPostingElement element(docID, term.getTf());

  // マップの検索
  ModTermMap::Iterator iterator = findTerm(term);

  // 登録されていない
  if(iterator == this->end()) {

    // ポスティングを生成する
    ModTermPosting posting(term);

    // ポスティングに要素を追加
    posting.pushBack(element);

    // 検索語マップにキー(検索語語形)と値(ポスティング)を登録
    (void)(this->insert(term.getString(), posting));

  // すでに登録済み
  } else {
    // ポスティングに要素を追加
    ((*iterator).second).pushBack(element);
  }
}

//
// FUNCTION public
// ModTermTable::ModTermTable -- 検索語テーブルのコンストラクタ
// 
// NOTES
//   検索語テーブルのコンストラクタ
//
// ARGUMENTS
//   const ModSize 		_width	文脈幅
//
// RETURN
//   なし
//
// EXCEPTIONS
//   なし
//
ModTermTable::ModTermTable(const ModSize _width)
{
  width = _width;
  nth   = _width + 1;  // 隣接語分として１を加算

  // 出現リストのパディング
  for(ModSize i = 0; i <= width; i++) {
    ModTermElement dummy("", 0, 0);
    termList.pushBack(dummy);
  }
}

//
// FUNCTION public
// ModTermTable::switchDocument -- 文書の切替え
// 
// NOTES
//   検索語テーブルへの登録で文書を切替える。
//   その文書の処理が終った後に実行すること。
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
void
ModTermTable::switchDocument()
{
  nth += width + 1;  // 隣接語分として１を加算

  // 出現リストのパディング
  for(ModSize i = 0; i <= width; i++) {
    ModTermElement dummy("", 0, 0);
    termList.pushBack(dummy);
  }
}

//
// FUNCTION public
// ModTermTable::insertTerm -- 検索語テーブルへの登録
// 
// NOTES
//   検索語テーブルへ検索語を登録する。
//   任意長の複合語に対応できるように語長を指定する。
//
//   ModTerm::poolTerm でこの登録が呼出される。 
//   n-gramは末尾語(単独語)が生成された時点で登録され、
//   その後、その末尾語が登録される。
// 
//
// ARGUMENTS
//   const ModTermElement&  term      検索語
//   ModSize                length    語長 (形態素数)
//
// RETURN
//   なし
//
// EXCEPTIONS
//   なし
//
void
ModTermTable::insertTerm(
  const ModTermElement& term,
  ModSize length)
{
  // ポスティングの要素(開始位置と終了位置のペア)を生成
  ModTermPostingElement element(nth - length + 1, nth);

  // 単独語の場合のみ出現番号を更新し語形を出現リストに加える
  if(length == 1) {
    termList.pushBack(term);
    nth++;
  }

  // ダミーはテーブルに登録しない
  if(term.getString().getLength() == 0) {
    return;
  }

  // テーブルの検索
  ModTermTable::Iterator iterator = findTerm(term);

  // 登録されていない
  if(iterator == this->end()) {

    // ポスティングを生成する
    ModTermPosting posting(term);

    // ポスティングに要素を追加
    posting.pushBack(element);

    // 検索語テーブルにキー(検索語語形)と値(出現番号ベクトル)を登録
    (void)(this->insert(term.getString(), posting));

  // すでに登録済み
  } else {
    // 出現番号を追加・更新
    ((*iterator).second).pushBack(element);
  }
}

//
// Robertson and Sparck Jones Relevance Weight
//
double _getWeight(ModSize r, ModSize R, ModSize n, ModSize N)
{
   double a = (r + 0.5) / (R - r + 0.5);
   double b = (n - r + 0.5) / (N - n - R + r + 0.5);
   double max = ((R + 0.5)/0.5)/(0.5/(N + 0.5));
   return log(a/b) / log(max);
}

//
// FUNCTION public
// ModTermTable::makeContext -- 対象語群の文脈生成
// 
// NOTES
//   対象語群の文脈を生成する。
//
// ARGUMENTS
//   ModTermPool&  target          対象語プール
//   ModTermPool&  leftContext     左文脈プール
//   ModTermPool&  rightContext    右文脈プール
//
// RETURN
//   なし
//
// EXCEPTIONS
//   なし
//
void
ModTermTable::makeContext(
	ModTermPool& target,
	ModTermPool& leftContext,
	ModTermPool& rightContext)
{
  // 対象語毎に
  for(ModTermPool::Iterator t = target.begin(); t != target.end(); t++) {

    // テーブルの検索
    ModTermTable::Iterator m = findTerm(t->getTerm());
    if(m == this->end()) continue;

    // 出現位置毎に
    ModTermPosting& posting = (*m).second;
    for(ModTermPosting::Iterator p = posting.begin(); p != posting.end(); p++) {

      ModSize s = (*p).first;   // 開始出現番号
      ModSize e = (*p).second;  // 終了出現番号

      // 左右文脈語の候補(単独語のみ)をセット
      for(ModSize i = 1; i <= width; i++) {
        leftContext.insertTerm(termList[s - i]);
        rightContext.insertTerm(termList[e + i]);
      }
    }
  }
}

//
// FUNCTION public
// ModTermTable::matchContext -- 文脈の照合
// 
// NOTES
//   テーブル内の登録語と文脈照合し照合度と共にプールする。
//
// ARGUMENTS
//   ModTermPool&  leftContext     左文脈プール
//   ModTermPool&  rightContext    右文脈プール
//   ModTermPool&  result          照合結果
//
// RETURN
//   なし
//
// EXCEPTIONS
//   なし
//
void
ModTermTable::matchContext(
	ModTermPool& leftContext,
	ModTermPool& rightContext,
    ModTermPool& left,  // 左文脈の右側出現語
    ModTermPool& right, // 右文脈の左側出現語
    ModSize minLeft, 
    ModSize minRight)
{
  // 左文脈語毎に
  ModTermPool::Iterator c;
  for(c = leftContext.begin(); c != leftContext.end(); c++) {

    // 低頻度のものは除外
    if(c->getTf() <= minLeft) continue;

    // テーブルの検索
    ModTermTable::Iterator m = findTerm(c->getTerm());
    if(m == this->end()) continue;

    // 出現位置毎に
    ModTermPosting& posting = (*m).second;
    for(ModTermPosting::Iterator p = posting.begin(); p != posting.end(); p++) {

      ModSize e = (*p).second;                // 終了出現番号

      // 右側の出現語を登録
      for(ModSize i = 1; i <= width; i++) {
        // 単独語
        ModTermElement& t1 = termList[e + i];
        left.insertTerm(t1);

        // 隣接語
        ModTermElement& t2 = termList[e + i + 1];
        ModTermElement bt(t1.getString(),t1.getType(),t1.getTwv(),
                          t2.getString(),t2.getType(),t2.getTwv());
		ModUnicodeString ostr = t1.getOriginalString();
		ostr.append(ModTermElement::termSeparator);
		ostr.append(t2.getOriginalString());
		bt.setOriginalString(ostr);

        // 登録されている隣接語のみを採用
        if(findTerm(bt) != this->end()) {
          left.insertTerm(bt);
        }
      }
    }
  }

  // 右文脈語毎に
  for(c = rightContext.begin(); c != rightContext.end(); c++) {

    // 低頻度のものは除外
    if(c->getTf() <= minRight) continue;

    // テーブルの検索
    ModTermTable::Iterator m = findTerm(c->getTerm());
    if(m == this->end()) continue;

    // 出現位置毎に
    ModTermPosting& posting = (*m).second;
    for(ModTermPosting::Iterator p = posting.begin(); p != posting.end(); p++) {

      ModSize s = (*p).first;                 // 開始出現番号

      // 左側の出現語を登録
      for(ModSize i = 1; i <= width; i++) {
        // 単独語
        ModTermElement& t1 = termList[s - i];
        right.insertTerm(t1);

        // 隣接語
        ModTermElement& t2 = termList[s - i - 1];
        ModTermElement bt(t2.getString(),t2.getType(),t2.getTwv(),
                          t1.getString(),t1.getType(),t1.getTwv());
		ModUnicodeString ostr = t2.getOriginalString();
		ostr.append(ModTermElement::termSeparator);
		ostr.append(t1.getOriginalString());
		bt.setOriginalString(ostr);

        // 登録されている隣接語のみを採用
        if(findTerm(bt) != this->end()) {
          right.insertTerm(bt);
        }
      }
    }
  }
}

//
// Copyright (c) 2000, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
