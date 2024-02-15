// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModTermMap.h
//  - ModTermPosting の宣言
//	- ModTermMap     の宣言
// 
// Copyright (c) 2000, 2012, 2013, 2023 Ricoh Company, Ltd.
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
#ifndef __SYDNEY_UTILITY_MODTERMMAP_H__
#define __SYDNEY_UTILITY_MODTERMMAP_H__

#include "Utility/Module.h"
#include "Utility/ModTermElement.h"

#include "ModMap.h"
#include "ModVector.h"

_TRMEISTER_BEGIN
_TRMEISTER_UTILITY_BEGIN

//
// CLASS ModTermPosting -- 検索語ポスティング
//
// NOTES
//	 検索語ポスティング。文書番号と検索語の出現頻度のペアを要素とする
//   ベクトル。さらに属性として代表検索語を有する。
//
typedef ModPair<ModSize, ModSize> ModTermPostingElement;

class ModTermPosting : public ModVector<ModTermPostingElement>
{
public:

  // コンストラクタ：ダミー
  ModTermPosting() {}

  // コンストラクタ
  ModTermPosting(const ModTermElement& _term) : term(_term) {}

  // 代表検索語のアクセサ
  ModTermElement& getTerm() { return term; }

protected:

private:
  // 代表検索語 (語形とタイプのみが有意)
  ModTermElement term;
};

//
// CLASS ModTermMap -- 検索語マップ
//
// NOTES
//   検索語マップ。検索語の語形をキーとし、ポスティングを値とする。
//
class ModTermMap 
  : public ModMap < ModUnicodeString, ModTermPosting, ModLess<ModUnicodeString> >
{
public:

  // コンストラクタ
  ModTermMap() : numDocs(0), confidence(0) {}

  // マップへの登録
  void insertTerm(const ModSize docID, const ModTermElement& term);

  // マップの検索
  ModTermMap::Iterator findTerm(const ModTermElement& term)
    { return this->find(term.getString()); }

  // 文書数のアクセサ
  ModSize getNumDocs() const { return numDocs; }

  // 文書数のセット
  void setNumDocs(const ModSize _numDocs) { numDocs = _numDocs; }

  // 信頼値のアクセサ
  double getConfidence() const { return confidence; }

  // 信頼値のセット
  void setConfidence(const double _confidence) { confidence= _confidence; }

protected:

private:

  // 文書数
  ModSize  numDocs;

  // 信頼値
  double confidence;
};

//
// CLASS ModTermTable -- 検索語テーブル
//
// NOTES
//   検索語テーブル。検索語の語形をキーとし、出現番号のベクトルを値とする。
//
class ModTermTable : public ModMap < ModUnicodeString, ModTermPosting, ModLess<ModUnicodeString> >
{
public:

  // コンストラクタ
  ModTermTable(const ModSize _width = 5);

  // 検索語の登録
  void insertTerm(const ModTermElement& term, ModSize length = 1);

  // テーブルの検索
  ModTermTable::Iterator findTerm(const ModTermElement& term)
    { return this->find(term.getString()); }

  // 文書の切り替え
  void switchDocument();

  // 文脈の生成
  void makeContext(ModTermPool& target, ModTermPool& left, ModTermPool& right);

  // 文脈の照合
  void matchContext(
		ModTermPool& leftContext, ModTermPool& rightContext, 
		ModTermPool& left, 
        ModTermPool& right,
		ModSize minLeft  = 0,
        ModSize minRight = 0);

  // 文脈の照合
  ModSize getNth() { return nth; }

protected:

private:

  // 検索語の出現番号カウンタ
  ModSize  nth;

  // 検索語の出現文脈幅 
  ModSize  width;

  // 検索語の出現リスト
  ModVector<ModTermElement> termList;
};

_TRMEISTER_UTILITY_END
_TRMEISTER_END

#endif __SYDNEY_UTILITY_MODTERMMAP_H__

//
// Copyright (c) 2000, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.

