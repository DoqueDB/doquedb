// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModTermMap.h
//  - ModTermPosting の宣言
//	- ModTermMap     の宣言
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
#ifndef __ModTermMap_H__
#define __ModTermMap_H__

#include "LibUna/ModTermElement.h"

_UNA_BEGIN

//
// CLASS ModTermMap -- 検索語マップ
//
// NOTES
//   検索語マップ。検索語の語形をキーとし、ポスティングを値とする。
//
class ModTermMap 
//  : public ModMap < ModUnicodeString, ModTermPosting, ModLess<ModUnicodeString> >
  : public ModMap < ModUnicodeString, ModLess<ModUnicodeString> >
{
public:

  // コンストラクタ
  ModTermMap(){};

  // マップへの登録
  void insertTerm(const ModSize docID, const ModTermElement& term);

  // マップの検索
  ModTermMap::Iterator findTerm(const ModTermElement& term)
    { return this->find(term.getString()); };

protected:

private:
};

//
// CLASS ModTermTable -- 検索語テーブル
//
// NOTES
//   検索語テーブル。検索語の語形をキーとし、出現番号のベクトルを値とする。
//
class ModTermTable : public ModMap < ModUnicodeString, ModLess<ModUnicodeString> >
{
public:

  // コンストラクタ
  ModTermTable(const ModSize _width = 5);

  // 検索語の登録
  void insertTerm(const ModTermElement& term, ModSize length = 1);

  // テーブルの検索
  ModTermTable::Iterator findTerm(const ModTermElement& term)
    { return this->find(term.getString()); };

protected:

private:

  // 検索語の出現リスト
  ModVector<ModTermElement> termList;
};

_UNA_END

#endif __ModTermMap_H__

//
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.

