// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
//  ModTermElement.h 
//   - ModTermElement の宣言
//   - ModTermIndex   の宣言
//   - ModTermPool    の宣言
// 
// Copyright (c) 2000, 2009, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_UTILITY_MODTERMELEMENT_H__
#define __SYDNEY_UTILITY_MODTERMELEMENT_H__

#include "Utility/Module.h"

#include "ModVector.h"
#include "ModMap.h"
#include "ModUnicodeString.h"
#include "ModAlgorithm.h" // ModSort
#include "ModLanguageSet.h"

_TRMEISTER_BEGIN
_TRMEISTER_UTILITY_BEGIN

class ModTerm;

// 統合品詞コード
typedef int ModUnifyTag;

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
class SYD_UTILITY_FUNCTION ModTermElement
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
	ModTermElement();

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

	// デストラクタ
	~ModTermElement();

	// コピーコンストラクタ
	ModTermElement(const ModTermElement& src);

	// 代入演算子
	ModTermElement& operator = (const ModTermElement& src);

	// 検索語式の獲得
	// (1) paramProximity(=d)が非零の場合
	//     - 正値の場合 #window[1,d,o] を生成
	//     - 負値の場合 #window[1,d,u] を生成
	//     空白の扱いの違いにより
	//     - 英語の場合   d = 1
	//     - 日本語の場合 d = 2  と指定する。
	// (2) 検索語の照合法match(=m), 語形(=f)に対し #term[m,,l](f) を生成
	//     ここで p はスコア計算機の指定で
	//     また語形 f が a + b (+はセパレータ)の場合
	//     - paramProximity が非零の場合 #term[m,,l](a),#term[m,,l](b) を生成
	//     - paramProximity が零の場合
	//       - aの末尾とbの先頭が共に英数字  #term[m,,l](a b) を生成
	//       - 上記以外                      #term[m,,l](ab)  を生成
	const ModUnicodeString& getFormula();

	// スコア計算器を得る
	//  - weightが零の場合    NormalizedOkapiTfIdf:x:y:z
	//  - weightが非零の場合  NormalizedOkapiTf:x:z
	// 上記 x, y, z はパラメタparamScore, paramWeight, paramLength
	ModUnicodeString getCalculator() const;

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
	void setParamWeight(const double _paramWeight)
		{ paramWeight = _paramWeight; }

	// paramScoreのアクセサ
	double getParamScore() const { return paramScore; }

	// paramScoreのセット
	void setParamScore(const double _paramScore) { paramScore = _paramScore; }

	// paramLengthのアクセサ
	double getParamLength() const { return paramLength; }

	// paramLengthのセット
	void setParamLength(const double _paramLength)
		{ paramLength = _paramLength; }

	// paramProximityのアクセサ
	int getParamProximity() const { return paramProximity; }

	// paramProximityのセット
	void setParamProximity(const int _paramProximity) 
		{ paramProximity = _paramProximity; }

	// langSpecのアクセサ
	const ModLanguageSet& getLangSpec() const { return langSpec; }
	// langSpecのセット
	void setLangSpec(const ModLanguageSet& _langSpec) 
		{ langSpec = _langSpec; }

	// matchModeのアクセサ
	ModTermMatch getMatchMode() const { return matchMode; }
	// matchModeのセット
	void setMatchMode(ModTermMatch _matchMode) { matchMode = _matchMode; }

	// 位置のアクセサ
	int getPosition() const { return position; }
	// 位置のセット
	void setPosition(int _position) { position = _position; }

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

	// 言語指定
	ModLanguageSet langSpec; 
	// 一致条件
	ModTermMatch matchMode;

	// tea構文
	ModUnicodeString formula;

	// 位置
	int position;

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
class SYD_UTILITY_FUNCTION ModTermIndex
{
public:
	// コンストラクタ - ダミー (ModVector<ModTermIndex>用)
	ModTermIndex() {}

	// コンストラクタ
	ModTermIndex(ModTermElement* _index) : index(_index) {}

	// 検索語の取得
	ModTermElement& getTerm() const { return *index; }

	// 検索式の取得
	const ModUnicodeString& getFormula()
		{ return index->getFormula(); }

	// スコア計算器を得る
	ModUnicodeString getCalculator() const
		{ return index->getCalculator(); }

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

	// langSpecのアクセサ
	const ModLanguageSet& getLangSpec() const { return index->getLangSpec(); }
	// langSpecのセット
	void setLangSpec(const ModLanguageSet& _langSpec) 
		{ index->setLangSpec(_langSpec); }

	// matchModeのアクセサ
	ModTermMatch getMatchMode() const { return index->getMatchMode(); }
	// matchModeのセット
	void setMatchMode(ModTermMatch _matchMode)
		{ index->setMatchMode(_matchMode); }

	// 位置のアクセサ
	int getPosition() const { return index->getPosition(); }
	// 位置のセット
	void setPosition(int _position)
		{ index->setPosition(_position); }

protected:

private:

	// 検索語へのポインタ
	ModTermElement* index;
};

// 検索語プールのマップ
typedef ModMap<ModUnicodeString, ModTermElement, ModLess<ModUnicodeString> >
ModTermPoolMap;

//
// CLASS ModTermPool -- 検索語プール
//
// NOTES
//   検索語プールを表すクラス。
//   検索語インデックスの配列。
//
class ModTermPool : public ModVector<ModTermIndex> 
{
	friend class ModTerm;
	
public:

	// コンストラクタ
	ModTermPool(const ModSize _maxSize) 
		: isHeap(ModFalse), maxSize(_maxSize), minTsv(0),
		  numUniGram(0), numBiGram(0)
		{ this->reserve(_maxSize); }

	// 検索語の選択値の降順にソートする
	void sortByTsv()
		{ ModSort(this->begin(),this->end(), ModGreater<ModTermIndex>());
		isHeap = ModFalse; }

	// 検索語の登録
	SYD_UTILITY_FUNCTION
	void insertTerm(const ModTermElement& element);

	// 指定選択値以下の検索語の消去
	SYD_UTILITY_FUNCTION
	void eraseTerm(double tsv);

	// 検索語の最大登録数のアクセサ
	ModSize getMaxSize() const { return maxSize; }

	// 最大登録数の変更(空の状態で)
	void reSize(const ModSize newSize) { maxSize = newSize; }

	// 単独語の総数のアクセサ
	ModSize getNumUniGram() const { return numUniGram; }

	// 隣接語の総数のアクセサ
	ModSize getNumBiGram()  const { return numBiGram;  }

	// DF値を設定する
	SYD_UTILITY_FUNCTION
	bool setDf(const ModUnicodeString& term, ModSize df);

protected:

private:
	// 登録を高速化用のマップ
	ModTermPoolMap map;

	// 検索語プールの最大登録数
	ModSize maxSize;

	// ヒープが形成されている場合は ModTrue。
	ModBoolean isHeap;

	// プール中の最小選択値
	// 最大登録数に達した後は、選択値最小のものから捨てられる。
	double minTsv;

	// プール中の単独語の総数
	ModSize numUniGram;

	// プール中の隣接語の総数
	ModSize numBiGram;
};

_TRMEISTER_UTILITY_END
_TRMEISTER_END

#endif // __SYDNEY_UTILITY_MODTERMELEMENT_H__

//
// Copyright (c) 2000, 2009, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
