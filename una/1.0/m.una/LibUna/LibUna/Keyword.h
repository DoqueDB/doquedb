// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Keyword.h -- Keyword の定義ファイル
// 
// Copyright (c) 2004-2010, 2023 Ricoh Company, Ltd.
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

#ifndef __UNA_LIBUNA_KEYWORD_H
#define __UNA_LIBUNA_KEYWORD_H

#define UNA_FUNCTION

#include "UnaDLL.h"
#include "ModCommonDLL.h"
#include "Type.h"
#include "ModVector.h"
#include "ModMap.h"
#include "ModUnicodeString.h"

// MACRO
// KeywordDLL --
//
// NOTES
//

namespace UNA {

class Morph;

//
//	CLASS
//		Keyword -- キーワードクラス
//
class UNA_LOCAL_FUNCTION Keyword
{
public:
	//	TYPEDEF
	//		KeywordRange
	typedef Type::Range<const Morph*>		KeywordRange;

	//	TYPEDEF
	//		CalcInfo -- score ルールで使用する計算情報と計算回数のペア
	typedef	ModPair< Type::CalcMaterial, unsigned int >	CalcInfo;

	// コンストラクタ、デストラクタ
	Keyword();
	Keyword(ModVector< Type::Range<const Morph*> >& words_);
	Keyword(const Keyword& keyword_);
	~Keyword(){}

	// キーワードの文字列(異表記正規化前)を得る
	ModUnicodeString		getOrg() const;
	ModUnicodeString getOrgEuroStyle() const;

	// キーワードの文字列(異表記正規化後)を得る
	ModUnicodeString		getNorm() const;
	ModUnicodeString getNormEuroStyle() const;

	// キーワードを構成する形態素(異表記正規化前)を文字列として返す。デバッグ用
	ModUnicodeString		getOrgMessage() const;

	// キーワードを構成する形態素(異表記正規化後)を文字列として返す。デバッグ用
	ModUnicodeString		getNormMessage() const;

	// 構成数を得る
	unsigned int			getSize() const;

	// 出現位置の追加
	void				pushBackPosition(const Morph* pos_, ModSize len_);

	// キーワードの文字長を取得する
	ModSize				getLength(int idx_ = 0) const;

	// Word へのアクセサ
	const Morph&			operator[](int i_) const;

	// 出現位置の取得
	const ModVector<KeywordRange>&	getPosition() const;

	// 代入演算子
	Keyword&			operator=(const Keyword& keyword_);
	Keyword&			operator+=(const Keyword& keyword_);

	// 比較演算子
	ModBoolean			operator < (const Keyword& keyword_) const;
	ModBoolean			operator > (const Keyword& keyword_) const;
	ModBoolean			operator == (const Keyword& keyword_) const;
	ModBoolean			operator != (const Keyword& keyword_) const;

	ModBoolean			operator == (const ModUnicodeString& str_) const;
	ModBoolean			operator != (const ModUnicodeString& str_) const;
	ModBoolean			operator < (const ModUnicodeString& str_) const;
	ModBoolean			operator > (const ModUnicodeString& str_) const;

protected:
private:
	// 出現位置
	ModVector<KeywordRange> _position;

	// スコア
	double					_score;

	// score ルールの為の計算情報
	ModVector< CalcInfo > _calcoperation;

};

// Keyword クラスのベクトル列(ModVector実装)
typedef		ModVector<Keyword> KeywordVec;
// Keyword クラスのベクトル列へのイテレータ
typedef		ModVector<Keyword>::Iterator KeywordVecIt;
// Keyword クラスのベクトル列へのイテレータ(const版)
typedef		ModVector<Keyword>::ConstIterator KeywordVecCIt;
// keyword クラス格納コンテナ
typedef		ModMap<ModUnicodeString, Keyword, ModLess<ModUnicodeString> > KeywordMap;
}

#endif // __UNA_LIBUNA_KEYWORD_H

//
// Copyright (c) 2004-2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//


