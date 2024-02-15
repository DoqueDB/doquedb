// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
//  ModTermElement.h 
//   - ModTermElement の宣言
//   - ModTermIndex   の宣言
//   - ModTermPool    の宣言
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

#ifndef __ModTermElement_H__
#define __ModTermElement_H__

#include "ModVector.h"
#include "ModMap.h"
#include "ModUnicodeString.h"
#include "Module.h"

_UNA_BEGIN

// 検索語タイプ
typedef int ModTermType;

//
// CLASS ModTermElement -- 検索語を表すクラス
//
// NOTES
//   検索語を表すクラス。
//
class ModTermElement
{
public:

  // コンストラクタ - ダミー
  ModTermElement(){}

  // コンストラクタ
  ModTermElement(
    const ModUnicodeString& _termNorm);		// 名詞句（正規化済み）

  ModTermElement(
    const ModUnicodeString& _termNorm,		// 名詞句（正規化済み）
    const ModUnicodeString& _termOrig);		// 名詞句（元表記）

  // normのアクセサ
  const ModUnicodeString& getTermNorm() const { return termNorm; }

  // normのセット
  void setTermNorm(const ModUnicodeString& _termNorm) { termNorm = _termNorm; }

  // origのアクセサ
  const ModUnicodeString& getTermOrig() const { return termOrig; }

  // origのセット
  void setTermOrig(const ModUnicodeString& _termOrig) 
	{ termOrig = _termOrig; }

  // normVectorのアクセサ
  ModVector<ModUnicodeString>& getNormVector() { return normVector; }

  // normVectorのセット
  void setNormVector(const ModVector<ModUnicodeString>& _normVector) { normVector = _normVector; }

  // origVectorのアクセサ
  ModVector<ModUnicodeString>& getOrigVector() { return origVector; }

  // origVectorのセット
  void setOrigVector(const ModVector<ModUnicodeString>& _origVector) { origVector = _origVector; }

  // posVectorのアクセサ
  ModVector<int>& getPosVector() { return posVector; }

  // posVectorのセット
  void setPosVector(const ModVector<int>& _posVector) { posVector = _posVector; }

  // costVectorのアクセサ
  ModVector<int>& getCostVector() { return costVector; }

  // costVectorのセット
  void setCostVector(const ModVector<int>& _costVector) { costVector = _costVector; }

  // 検索語語形の区切り文字
  static const ModUnicodeChar termSeparator;

protected:

private:

  // 語形 (正規化あり)
  ModUnicodeString termNorm;

  // 語形 (正規化なし) 
  // - コンストラクト時には上記stringと同一
  // - コンストラクト後にsetが必要
  ModUnicodeString termOrig;

  // タイプ
  ModTermType type;

  // normVector
  ModVector<ModUnicodeString> normVector;

  // origVector
  ModVector<ModUnicodeString> origVector;

  // posVector
  ModVector<int> posVector;

  // costVector
  ModVector<int> costVector;
};

//
// CLASS
// ModTermIndex -- 順序つき検索語インデックス
//
// NOTES
//   順序つき検索語インデックス。
//   検索語へのポインタ。検索語の実体は検索語プールのマップに格納。
//
class ModTermIndex
{
public:
  // コンストラクタ - ダミー (ModVector<ModTermIndex>用)
  ModTermIndex() {}

  // コンストラクタ
  ModTermIndex(ModTermElement* _index) : index(_index) {}

  // 検索語の取得
  ModTermElement& getTermElement() const { return *index; }

  // normのアクセサ
  const ModUnicodeString& getTermNorm() const { return index->getTermNorm(); }

  // normのセット
  void setTermNorm(const ModUnicodeString& _termNorm) 
	{ index->setTermNorm(_termNorm); }

  // origのアクセサ
  const ModUnicodeString& getTermOrig() const 
	{ return index->getTermOrig(); }

  // origのセット
  void setTermOrig(const ModUnicodeString& _termOrig) 
	{ index->setTermOrig(_termOrig); }

  // normVectorのアクセサ
  ModVector<ModUnicodeString>& getNormVector() const { return index->getNormVector(); }

  // normVectorのセット
  void setNormVector(const ModVector<ModUnicodeString>& _normVector) { index->setNormVector(_normVector); }

  // origVectorのアクセサ
  ModVector<ModUnicodeString>& getOrigVector() const { return index->getOrigVector(); }

  // origVectorのセット
  void setOrigVector(const ModVector<ModUnicodeString>& _origVector) { index->setOrigVector(_origVector); }

  // posVectorのアクセサ
  ModVector<int>& getPosVector() const { return index->getPosVector(); }

  // posVectorのセット
  void setPosVector(const ModVector<int>& _posVector) { index->setPosVector(_posVector); }

  // costVectorのアクセサ
  ModVector<int>& getCostVector() const { return index->getCostVector(); }

  // costVectorのセット
  void setCostVector(const ModVector<int>& _costVector) { index->setCostVector(_costVector); }

//#ifdef V1_6
//  // langSpecのアクセサ
//  const ModLanguageSet& getLangSpec() const { return index->getLangSpec(); }
//
//  // langSpecのセット
//  void setLangSpec(const ModLanguageSet& _langSpec) 
//    { index->setLangSpec(_langSpec); }
//#endif

protected:

private:

  // 検索語へのポインタ
  ModTermElement* index;
};

// 検索語プールのマップ
typedef ModMap< ModUnicodeString, ModTermElement, ModLess<ModUnicodeString> > ModTermPoolMap;

//
// CLASS ModTermPool -- 名詞句プール
//
// NOTES
//   検索語プールを表すクラス。
//   検索語インデックスの配列。
//
class ModTermPool : public ModVector<ModTermIndex>
{
public:

  // コンストラクタ
  ModTermPool() {};

  // 名詞句の登録
  void insertTerm(const ModTermElement& element);

  // 登録を高速化用のマップ
  ModTermPoolMap map;

protected:

private:

};

//
// FUNCTION public
// ModTermElement::ModTermElement -- コンストラクタ
//
// NOTES
//   コンストラクタ。
//
// ARGUMENTS
//   const ModUnicodeString&	_termNorm			名詞句（正規化済み）
//
// RETURN
//   なし
//
// EXCEPTIONS
//   なし
//
inline
ModTermElement::ModTermElement(
  const ModUnicodeString& _termNorm)
  :
  termNorm(_termNorm)
{
//#ifdef V1_6
//  // langSpecはデフォルトコンストラクタによる
//#endif
}

//
// FUNCTION public
// ModTermElement::ModTermElement -- コンストラクタ
//
// NOTES
//   コンストラクタ。
//
// ARGUMENTS
//   const ModUnicodeString&	_termNorm			名詞句（正規化済み）
//   const ModUnicodeString&	_termOrig			名詞句（元表記）
//
// RETURN
//   なし
//
// EXCEPTIONS
//   なし
//
inline
ModTermElement::ModTermElement(
  const ModUnicodeString& _termNorm,
  const ModUnicodeString& _termOrig)
  :
  termNorm(_termNorm), termOrig(_termOrig)
{
//#ifdef V1_6
//  // langSpecはデフォルトコンストラクタによる
//#endif
}

_UNA_END

#endif // __ModTermElement_H__

//
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
