// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
//  ModTermPattern.h  
//   - ModTermPattern の宣言
//   - ModTermPatternFile の宣言
// 
// Copyright (c) 2000, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __ModTermPattern_H__
#define __ModTermPattern_H__

#include "ModUnicodeRegularExpression.h"  // 正規表現を使用
#include "ModTermElement.h"
#include "ModTermStringFile.h"

// typedefs
typedef int ModTermPatternID;  // パタン識別子の型

//
// CLASS ModTermPatternResource -- パターンリソースを表すクラス
//
// NOTES
//  パターンリソースを表すクラス。
//  識別子、種別、重み、コンパイル前の正規表現、置換をもつ。
//
//  ModTermPatternFileで使用される。
//
class ModTermPatternResource
{
public:
	// コンストラクタ
	ModTermPatternResource(
		const ModUnicodeString& record,         // パタンの文字列
		const ModUnicodeChar fieldSep = 0x19);  // フィールド区切子

	// コンストラクタ - ダミー
	ModTermPatternResource(){}

	// idのアクセサ
	ModTermPatternID getID() const { return id; };

	// typeのアクセサ
	ModTermType getType() const { return type; };

	// weightのアクセサ
	double getWeight() const { return weight; };

	// replacement のアクセサ
	const ModUnicodeString& getReplacement() const { return replacement; };

	// patternのアクセサ
	const ModUnicodeString& getPattern() const { return pattern; };

protected:

private:

	// パターン識別子
	ModTermPatternID id;

	// パターン種別
	ModTermType  type;

	// パターン重み
	double weight;

	// 置換
	ModUnicodeString replacement;

	// コンパイル前の正規表現
	ModUnicodeString pattern;
};

//
// CLASS ModTermPattern -- パターンを表すクラス
//
// NOTES
//  パターンを表すクラス。
//	パターンリソース、正規表現ハンドルをもつ。
//
//  ModTermPatternFileで使用される。
//
class ModTermPattern
{
public:
	// コンストラクタ
	ModTermPattern()
		: resource(0), isCompile(ModFalse) {}

	// コピーコンストラクタ
	ModTermPattern(const ModTermPattern& other_)
		: resource(other_.resource), isCompile(other_.isCompile) {}

	// operator =
	ModTermPattern& operator = (const ModTermPattern& other_)
	{
		resource = other_.resource;
		isCompile = other_.isCompile;
		return *this;
	}

	// リソースを設定する
	void setResource(const ModTermPatternResource* resource_)
		{ resource = resource_; } 

	// idのアクセサ
	ModTermPatternID getID() const { return resource->getID(); };

	// typeのアクセサ
	ModTermType getType() const { return resource->getType(); };

	// weightのアクセサ
	double getWeight() const { return resource->getWeight(); };

	// 照合
	ModBoolean match(const ModUnicodeString& string, const ModSize offset);

	// 置換文字列の生成 (照合が行われている事が前提)
	ModUnicodeString replace();

protected:

private:

	// パターンリソース
	const ModTermPatternResource* resource;

	// 正規表現のハンドル。patternをrxCompile関数に与えて得る
	ModUnicodeRegularExpression handle;

	// 正規表現ハンドルをコンパイルしたかどうか
	ModBoolean isCompile;
};

//
// CLASS ModTermPatternFile -- パターンファイル
//
// NOTES
//  パターンファイル。
//  指定ファイル中の各パタンへのポインタを要素とするベクトル。
//
class ModTermPatternFile : public ModVector<ModTermPatternResource*>
{
public:

	// コンストラクタ
	ModTermPatternFile(
		const ModUnicodeString& path,         // パタン辞書ファイルのパス
		const char recordSep = 0x1A);  // パタン(レコード)の区切子

	// デストラクタ
	~ModTermPatternFile();

	// パターン配列を得る
	void getPattern(ModVector<ModTermPattern>& pattern_);

protected:

private:

};

#endif // __ModTermPattern_H__

//
// Copyright (c) 2000, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
