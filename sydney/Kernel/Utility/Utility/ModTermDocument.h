// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
//  ModTermDocument.h	-- ModTermDocument	の宣言
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

#ifndef __SYDNEY_UTILITY_MODTERMDOCUMENT_H__
#define __SYDNEY_UTILITY_MODTERMDOCUMENT_H__

#include "Utility/Module.h"

#include "ModUnicodeString.h"

_TRMEISTER_BEGIN
_TRMEISTER_UTILITY_BEGIN

//
// CLASS ModTermDocument -- 検索語を表すクラス
//
// NOTES
//	シード文書をあらわすクラス。
//
class ModTermDocument {
public:

	// シード文書の分類
	enum ModTermDocType {
		// 初期検索文書
		ModTermDocRetrieved,
		// 適合文書
		ModTermDocRelevant,
		// 非適合文書
		ModTermDocIrrelevant
	};

	// コンストラクタ
	ModTermDocument(
			ModTermDocType _type, ModUnicodeString& _text, ModSize _poolSize);

	// コンストラクタ(ダミー：これがないとvector使用でコンパイルエラー)
	ModTermDocument(){/* ダミー */}

	// type のアクセサ
	ModTermDocType getType() const;

	// text のアクセサ
	const ModUnicodeString& getText() const;

	// poolSizeのアクセサ
	ModSize getPoolSize() const;

#ifdef DEBUG
	// デバック用表示関数
	void showDocument();
#endif // DEBUG

protected:

private:

	// シード文書の分類
	ModTermDocType type;

	// シード文書のテキスト
	ModUnicodeString text;

	// クエリ生成で用いられる検索語プールの最大サイズ
	ModSize poolSize;

};

//
// FUNCTION public
// ModTermDocument::ModTermDocument -- コンストラクタ
//
// NOTES
//	コンストラクタ。シード文書を生成する。
//	文書分類、テキスト、プールサイズをセットする。
//
// ARGUMENTS
//	ModTermDocType _type
//		文書分類
//	ModUnicodeString& _text
//		テキスト
//	ModSize _poolSize
//		プールサイズ
//
// RETURN
//  なし
//
// EXCEPTIONS
//  なし
//
inline
ModTermDocument::ModTermDocument(
	ModTermDocType _type,
	ModUnicodeString& _text,
	ModSize _poolSize) :
	type(_type), text(_text), poolSize(_poolSize)
{ }


//
// FUNCTION public
// ModTermDocument::getType -- typeのアクセサ
//
// NOTES
//	typeのアクセサ。
//
// ARGUMENTS
//	なし
//
// RETURN
//  const ModTermDocType	文書種別を返す
//
// EXCEPTIONS
//  なし
//
inline ModTermDocument::ModTermDocType 
ModTermDocument::getType() const
{
	return type;
}


//
// FUNCTION public
// ModTermDocument::getText -- textのアクセサ
//
// NOTES
//	textのアクセサ。
//
// ARGUMENTS
//	なし
//
// RETURN
//	const ModUnicodeString&	シード文書を返す
//
// EXCEPTIONS
//  なし
//
inline const ModUnicodeString&
ModTermDocument::getText() const
{
	return text;
}


//
// FUNCTION public
// ModTermDocument::getPoolSize -- poolSizeのアクセサ
//
// NOTES
//	poolSizeのアクセサ。
//
// ARGUMENTS
//	なし
//
// RETURN
//	const ModSize	poolSizeの値を返す
//
// EXCEPTIONS
//  なし
//
inline ModSize
ModTermDocument::getPoolSize() const
{
	return poolSize;
}

_TRMEISTER_UTILITY_END
_TRMEISTER_END

#endif // __SYDNEY_UTILITY_MODTERMDOCUMENT_H__

//
// Copyright (c) 2000, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

