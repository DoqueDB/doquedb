// -*-mOde: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModLanguageSet.h --
//		複数の言語タグ(以下言語セット)を扱うクラスの定義
// 
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModLanguageSet_H__
#define __ModLanguageSet_H__

#include "ModTypes.h"
#include "ModCommonDLL.h"
#include "ModSerial.h"
#include "ModDefaultManager.h"
#include "ModUnicodeString.h"
#include "ModLanguage.h"
#include "ModVector.h"

// CLASS
// ModLanguageSet -- 言語セットを表現するクラス
//
// NOTES
//

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する
//
//【注意】
//	public 関数ごとに ModCommonDLL を付けると Win版でうまく動かない

class ModCommonDLL ModLanguageSet
	: public ModSerializer,
	  public ModDefaultObject
{
public:

	//
	// コンストラクタ
	//

	// コンストラクタ(1)
	ModLanguageSet();

	// コンストラクタ(2)
	ModLanguageSet(ModLanguageCode	langCode_);

	// コンストラクタ(3)
	ModLanguageSet(const ModUnicodeString&	langSetSymbol_);

	// コンストラクタ(4)
	ModLanguageSet(ModLanguageTag	langTag_);

	//
	// マニピュレータ
	//

	// = 演算オペレータ(1)
	ModLanguageSet& operator=(ModLanguageCode	langCode_);

	// = 演算オペレータ(2)
	ModLanguageSet& operator=(const ModUnicodeString&	langSetSymbol_);

	// = 演算オペレータ(3)
	ModLanguageSet& operator=(const ModLanguageTag&	langTag_);

	// = 演算オペレータ(4)
	ModLanguageSet& operator=(const ModLanguageSet&	langSet_);

	// == 演算オペレータ
	ModBoolean operator==(const ModLanguageSet&	langSet_) const;

	// != 演算オペレータ
	ModBoolean operator!=(const ModLanguageSet&	langSet_) const;

	// < 演算オペレータ
	ModBoolean operator<(const ModLanguageSet&	langSet_) const;

	// <= 演算オペレータ
	ModBoolean operator<=(const ModLanguageSet&	langSet_) const;

	// > 演算オペレータ
	ModBoolean operator>(const ModLanguageSet&	langSet_) const;

	// >= 演算オペレータ
	ModBoolean operator>=(const ModLanguageSet&	langSet_) const;

	// 言語セットをクリアします
	void clear();

	// 指定言語を追加します
	void add(ModLanguageCode langCode_);

	// 指定言語タグを追加します
	void add(const ModLanguageTag&	langTag_);

	// 指定言語が含まれているかどうかを調べます
	ModBoolean isContained(ModLanguageCode	langCode) const;

	// 指定言語タグが含まれているかどうかを調べます
	ModBoolean isContained(const ModLanguageTag&	langTag_) const;

	// 言語タグから国・地域コードを除いた言語セットを取得します
	ModLanguageSet round() const;

	// 言語タグ数を取得します
	ModSize getSize() const;

	// 言語セットの文字列表現を取得します
	ModUnicodeString getName() const; 

	// シリアル化します
	void serialize(ModArchive&	archiver_);

	// 言語セット同士を比較します
	static int compare(const ModLanguageSet&	src_,
					   const ModLanguageSet&	dst_);

private:

	// 言語タグを積むベクター
	ModVector<ModLanguageTag>	_v;

}; // end of class ModLanguageSet

#endif	// __ModLanguageSet_H__

//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
