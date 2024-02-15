// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
//  ModTermElement.h 
//   - ModTermElement の宣言
//   - ModTermIndex   の宣言
//   - ModTermPool    の宣言
// 
// Copyright (c) 2000, 2009, 2023 Ricoh Company, Ltd.
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
#include "ModAlgorithm.h" // ModSort
#ifdef V1_6
#include "ModLanguageSet.h"
#endif // V1_6

// 検索語タイプ
typedef int ModTermType;

// 検索語の照合法
typedef int ModTermMatch;

//
// CLASS ModTermElement -- 検索語を表すクラス
//
// NOTES
//   検索語を表すクラス。
//
class ModTermElement
{
public:

  // 検索語の照合法
  enum {
    voidMatch = 0,     // 初期化用無効値

    stringMatch,       // 文字列照合   #term[n]
    exactMatch,        // 厳格単語照合 #term[e]
    simpleMatch,       // 単純単語照合 #term[s]
    approximateMatch,  // 曖昧単語照合 #term[a]
	multiMatch,		   // マルチ言語照合 #term[m]
	headMatch,		   // 先頭単語照合 #term[h]
	tailMatch		   // 末尾単語照合 #term[t]
  };

  // コンストラクタ - ダミー
  ModTermElement(){}

  // 単独語用コンストラクタ
  ModTermElement(
    const ModUnicodeString& _string,    // 単独語の語形
    const ModTermType       _type = 0,  // 単独語のパタン種別
    const double            _twv  = 1); // 単独語のパタン重み

  // 隣接語用コンストラクタ
  ModTermElement(
    const ModUnicodeString& _string1,   // 前置語の語形
    const ModTermType       _type1,     // 前置語のパタン種別
    const double            _twv1,      // 前置語のパタン重み
    const ModUnicodeString& _string2,   // 後置語の語形
    const ModTermType       _type2,     // 後置語のパタン種別
    const double            _twv2);     // 後置語のパタン重み

  // 検索語式の獲得
  // (1) scale(=n)が非零の場合 #scale[n] を生成
  // (2) paramProximity(=d)が非零の場合
  //     - 正値の場合 #window[1,d,o] を生成
  //     - 負値の場合 #window[1,d,u] を生成
  //     空白の扱いの違いにより
  //     - 英語の場合   d = 1
  //     - 日本語の場合 d = 2  と指定する。
  // (3) 検索語の照合法match(=m), 語形(=f)に対し #term[m,p](f) を生成
  //     ここで p はスコア計算機の指定で
  //     - weightが零の場合    NormalizedOkapiTfIdf:x:y:z
  //     - weightが非零の場合  NormalizedOkapiTf:x:z
  //     上記 x, y, z はパラメタparamScore, paramWeight, paramLength
  //     また語形 f が a + b (+はセパレータ)の場合
  //     - paramProximity が非零の場合 #term[m,p](a),#term[m,p](b) を生成
  //     - paramProximity が零の場合
  //       - aの末尾とbの先頭が共に英数字  #term[m,p](a b) を生成
  //       - 上記以外                      #term[m,p](ab)  を生成
  ModUnicodeString getFormula(
	ModTermMatch match,           // 検索語の照合法
	const ModUnicodeString& calculator_,	// スコア計算器
    ModBoolean ranking = ModTrue) // 真ならランキング検索用、偽ならブーリアン検索用
    const;  

  // 隣接語か否かの判定
  ModBoolean isBiGram () const { return (type < 0) ? ModTrue : ModFalse; }

  // stringのアクセサ
  const ModUnicodeString& getString() const { return string; }

  // stringのセット
  void setString(const ModUnicodeString& _string) { string = _string; }

  // originalStringのアクセサ
  const ModUnicodeString& getOriginalString() const { return originalString; }

  // originalStringのセット
  void setOriginalString(const ModUnicodeString& _originalString) 
	{ originalString = _originalString; }

  // typeのアクセサ
  ModTermType getType() const { return type; }

  // typeのセット
  void setType(const ModTermType _type) { type = _type; }

  // scaleのアクセサ
  double getScale() const { return scale; }

  // scaleのセット
  void setScale(const double _scale) { scale = _scale; }

  // weightのアクセサ
  double getWeight() const { return weight; }

  // weightのセット
  void setWeight(const double _weight) { weight = _weight; }

  // tfのアクセサ
  double getTf() const { return tf; }

  // tfのセット
  void setTf(const double _tf) { tf = _tf; }

  // dfのアクセサ
  double getDf() const { return df; }

  // dfのセット
  void setDf(const double _df) { df = _df; }

  // sdfのアクセサ
  double getSdf() const { return sdf; }

  // sdfのセット
  void setSdf(const double _sdf) { sdf = _sdf; }

  // tsvのアクセサ
  double getTsv() const { return tsv; }

  // tsvのセット
  void setTsv(const double _tsv) { tsv = _tsv; }

  // twvのアクセサ
  double getTwv() const { return twv; }

  // twvのセット
  void setTwv(const double _twv) { twv = _twv; }

  // paramWeightのアクセサ
  double getParamWeight() const { return paramWeight; }

  // paramWeightのセット
  void setParamWeight(const double _paramWeight) { paramWeight = _paramWeight; }

  // paramScoreのアクセサ
  double getParamScore() const { return paramScore; }

  // paramScoreのセット
  void setParamScore(const double _paramScore) { paramScore = _paramScore; }

  // paramLengthのアクセサ
  double getParamLength() const { return paramLength; }

  // paramLengthのセット
  void setParamLength(const double _paramLength) { paramLength = _paramLength; }

  // paramProximityのアクセサ
  int getParamProximity() const { return paramProximity; }

  // paramProximityのセット
  void setParamProximity(const int _paramProximity) 
    { paramProximity = _paramProximity; }

#ifdef V1_6
  // langSpecのアクセサ
  const ModLanguageSet& getLangSpec() const { return langSpec; }

  // langSpecのセット
  void setLangSpec(const ModLanguageSet& _langSpec) 
    { langSpec = _langSpec; }
#endif

  // matchModeのアクセサ
  ModTermMatch getMatchMode() const { return matchMode; }

  // matchModeのセット
  void setMatchMode(ModTermMatch _matchMode) { matchMode = _matchMode; }

  // 検索語語形の区切り文字
  static const ModUnicodeChar termSeparator;

protected:

private:

  // 語形 (正規化あり)
  ModUnicodeString string;

  // 語形 (正規化なし) 
  // - コンストラクト時には上記stringと同一
  // - コンストラクト後にsetが必要
  ModUnicodeString originalString;

  // タイプ
  ModTermType type;
  // スケール
  double scale;
  // 重み
  double weight;

  // 出現頻度 (term frequency)
  double tf;
  // 文書頻度 (document frequency)
  double df;
  // シード文書頻度 (seed document frequency)
  double sdf;
  // 重みづけ値 (term weighting value)
  double twv;
  // 選択値 (term selection value)
  double tsv;

  // 重みパラメタ
  double paramWeight;
  // 文書スコアパラメタ
  double paramScore;
  // 文書長パラメタ
  double paramLength;
  // 近接パラメタ
  int paramProximity;

#ifdef V1_6
  // 言語指定
  ModLanguageSet langSpec; 
#endif

  // 一致条件
  ModTermMatch matchMode;

  // 属性群の変更時には、コンストラクタ, アクセサ, ModTermIndexのアクセサ
  // の変更も必要。
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
  ModTermElement& getTerm() const { return *index; }

  // 検索式の取得
  ModUnicodeString getFormula(
		ModTermMatch match,
		const ModUnicodeString& calculator,
		ModBoolean ranking = ModTrue) const
    { return index->getFormula(match, calculator, ranking); }

  // 大小関係 (重みの降順ソート用)
  ModBoolean operator>(const ModTermIndex& _other) const 
    { return index->getTsv() > _other.getTsv() ? ModTrue : ModFalse; }

  // 隣接語か否かの判定
  ModBoolean isBiGram () const { return index->isBiGram(); }

  // stringのアクセサ
  const ModUnicodeString& getString() const { return index->getString(); }

  // stringのセット
  void setString(const ModUnicodeString& _string) 
	{ index->setString(_string); }

  // originalStringのアクセサ
  const ModUnicodeString& getOriginalString() const 
	{ return index->getOriginalString(); }

  // originalStringのセット
  void setOriginalString(const ModUnicodeString& _originalString) 
	{ index->setOriginalString(_originalString); }

  // typeのアクセサ
  ModTermType getType() const { return index->getType(); }

  // typeのセット
  void setType(const ModTermType _type) { index->setType(_type); }

  // scaleのアクセサ
  double getScale() const { return index->getScale(); }

  // scaleのセット
  void setScale(const double _scale) { index->setScale(_scale); }

  // weightのアクセサ
  double getWeight() const { return index->getWeight(); }

  // weightのセット
  void setWeight(const double _weight) { index->setWeight(_weight); }

  // tfのアクセサ
  double getTf() const { return index->getTf(); }

  // tfのセット
  void setTf(const double _tf) { index->setTf(_tf); }

  // dfのアクセサ
  double getDf() const { return index->getDf(); }

  // dfのセット
  void setDf(const double _df) { index->setDf(_df); }

  // sdfのアクセサ
  double getSdf() const { return index->getSdf(); }

  // sdfのセット
  void setSdf(const double _sdf) { index->setSdf(_sdf); }

  // tsvのアクセサ
  double getTsv() const { return index->getTsv(); }

  // tsvのセット
  void setTsv(const double _tsv) { index->setTsv(_tsv); }

  // twvのアクセサ
  double getTwv() const { return index->getTwv(); }

  // twvのセット
  void setTwv(const double _twv) { index->setTwv(_twv); }

  // paramWeightのアクセサ
  double getParamWeight() const 
    { return index->getParamWeight(); }

  // paramWeightのセット
  void setParamWeight(const double _paramWeight) 
    { index->setParamWeight(_paramWeight); }

  // paramScoreのアクセサ
  double getParamScore() const 
    { return index->getParamScore(); }

  // paramScoreのセット
  void setParamScore(const double _paramScore) 
    { index->setParamScore(_paramScore); }

  // paramLengthのアクセサ
  double getParamLength() const 
    { return index->getParamLength(); }

  // paramLengthのセット
  void setParamLength(const double _paramLength) 
    { index->setParamLength(_paramLength); }

  // paramProximityのアクセサ
  int getParamProximity() const 
    { return index->getParamProximity(); }

  // paramProximityのセット
  void setParamProximity(const int _paramProximity) 
    { index->setParamProximity(_paramProximity); }

#ifdef V1_6
  // langSpecのアクセサ
  const ModLanguageSet& getLangSpec() const { return index->getLangSpec(); }

  // langSpecのセット
  void setLangSpec(const ModLanguageSet& _langSpec) 
    { index->setLangSpec(_langSpec); }
#endif

	// matchModeのアクセサ
	ModTermMatch getMatchMode() const { return index->getMatchMode(); }

	// matchModeのセット
	void setMatchMode(ModTermMatch _matchMode)
		{ index->setMatchMode(_matchMode); }

protected:

private:

  // 検索語へのポインタ
  ModTermElement* index;
};

// 検索語プールのマップ
typedef ModMap< ModUnicodeString, ModTermElement, ModLess<ModUnicodeString> > ModTermPoolMap;

//
// CLASS ModTermPool -- 検索語プール
//
// NOTES
//   検索語プールを表すクラス。
//   検索語インデックスの配列。
//
class ModTermPool : public ModVector<ModTermIndex> 
{
public:

  // コンストラクタ
  ModTermPool(const ModSize _maxSize) 
  : isHeap(ModFalse), maxSize(_maxSize), minTsv(0), numUniGram(0), numBiGram(0)
    { this->reserve(_maxSize); };

  // 検索語の選択値の降順にソートする
  void sortByTsv()
    { ModSort(this->begin(),this->end(), ModGreater<ModTermIndex>());
      isHeap = ModFalse; };

  // 検索語の登録
  void insertTerm(const ModTermElement& element);

  // 指定選択値以下の検索語の消去
  void eraseTerm(double tsv);

  // 検索語の最大登録数のアクセサ
  ModSize getMaxSize() const { return maxSize; };

  // 最大登録数の変更(空の状態で)
  void reSize(const ModSize newSize) { maxSize = newSize; };

  // 単独語の総数のアクセサ
  ModSize getNumUniGram() const { return numUniGram; };

  // 隣接語の総数のアクセサ
  ModSize getNumBiGram()  const { return numBiGram;  };

  // 登録を高速化用のマップ
  ModTermPoolMap map;

protected:

private:

  // 検索語プールの最大登録数
  ModSize maxSize;

  // ヒープが形成されている場合は ModTrue。
  ModBoolean isHeap;

  // プール中の最小選択値。最大登録数に達した後は、選択値最小のものから捨てられる。
  double minTsv;

  // プール中の単独語の総数
  ModSize numUniGram;

  // プール中の隣接語の総数
  ModSize numBiGram;
};

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
inline
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
#ifdef V1_6
  // langSpecはデフォルトコンストラクタによる
#endif
  matchMode		 = voidMatch;
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
inline
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
#ifdef V1_6
  // langSpecはデフォルトコンストラクタによる
#endif
  matchMode		 = voidMatch;
}

#endif // __ModTermElement_H__

//
// Copyright (c) 2000, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
