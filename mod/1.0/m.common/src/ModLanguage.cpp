// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModLanguage.cpp -- 言語情報関連
// 
// Copyright (c) 1999, 2002, 2003, 2023 Ricoh Company, Ltd.
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


#include "ModLanguage.h"
#include "ModUnicodeCharTrait.h"

//	VARIABLE 
//	ModLanguageCodeSymbol -- コードとシンボルのテーブル
//
//	NOTES
//		
//
static const ModUnicodeChar UndefinedSymbol[] = {
	ModUnicodeChar('u'),
	ModUnicodeChar('n'),
	ModUnicodeChar('d'),
	ModUnicodeChar('e'),
	ModUnicodeChar('f'),
	ModUnicodeChar('i'),
	ModUnicodeChar('n'),
	ModUnicodeChar('e'),
	ModUnicodeChar('d'),
	ModUnicodeChar(0x0000)
};

static const ModUnicodeChar ModLanguageCodeSymbol[][3] = {
	{ModUnicodeChar(0x0000)},
	{ModUnicodeChar('a'), ModUnicodeChar('a')},		// Afar
	{ModUnicodeChar('a'), ModUnicodeChar('b')},		// Abkhazian
	{ModUnicodeChar('a'), ModUnicodeChar('f')},		// Afrikaans
	{ModUnicodeChar('a'), ModUnicodeChar('m')},		// Amharic
	{ModUnicodeChar('a'), ModUnicodeChar('r')},		// Arabic
	{ModUnicodeChar('a'), ModUnicodeChar('s')},		// Assamese
	{ModUnicodeChar('a'), ModUnicodeChar('y')},		// Aymara
	{ModUnicodeChar('a'), ModUnicodeChar('z')},		// Azerbaijani

	{ModUnicodeChar('b'), ModUnicodeChar('a')},		// Bashkir
	{ModUnicodeChar('b'), ModUnicodeChar('e')},		// Byelorussian
	{ModUnicodeChar('b'), ModUnicodeChar('g')},		// Bulgarian
	{ModUnicodeChar('b'), ModUnicodeChar('h')},		// Bihari
	{ModUnicodeChar('b'), ModUnicodeChar('i')},		// Bislama
	{ModUnicodeChar('b'), ModUnicodeChar('n')},		// Bengali; Bangla
	{ModUnicodeChar('b'), ModUnicodeChar('o')},		// Tibetan
	{ModUnicodeChar('b'), ModUnicodeChar('r')},		// Breton

	{ModUnicodeChar('c'), ModUnicodeChar('a')},		// Catalan
	{ModUnicodeChar('c'), ModUnicodeChar('o')},		// Corsican
	{ModUnicodeChar('c'), ModUnicodeChar('s')},		// Czech
	{ModUnicodeChar('c'), ModUnicodeChar('y')},		// Welsh

	{ModUnicodeChar('d'), ModUnicodeChar('a')},		// Danish
	{ModUnicodeChar('d'), ModUnicodeChar('e')},		// German
	{ModUnicodeChar('d'), ModUnicodeChar('z')},		// Bhutani

	{ModUnicodeChar('e'), ModUnicodeChar('l')},		// Greek
	{ModUnicodeChar('e'), ModUnicodeChar('n')},		// English
	{ModUnicodeChar('e'), ModUnicodeChar('o')},		// Esperanto
	{ModUnicodeChar('e'), ModUnicodeChar('s')},		// Spanish
	{ModUnicodeChar('e'), ModUnicodeChar('t')},		// Estonian
	{ModUnicodeChar('e'), ModUnicodeChar('u')},		// Basque

	{ModUnicodeChar('f'), ModUnicodeChar('a')},		// Persian
	{ModUnicodeChar('f'), ModUnicodeChar('i')},		// Finnish
	{ModUnicodeChar('f'), ModUnicodeChar('j')},		// Fiji
	{ModUnicodeChar('f'), ModUnicodeChar('o')},		// Faroese
	{ModUnicodeChar('f'), ModUnicodeChar('r')},		// French
	{ModUnicodeChar('f'), ModUnicodeChar('y')},		// Frisian

	{ModUnicodeChar('g'), ModUnicodeChar('a')},		// Irish
	{ModUnicodeChar('g'), ModUnicodeChar('d')},		// Scots Gaelic
	{ModUnicodeChar('g'), ModUnicodeChar('l')},		// Galician
	{ModUnicodeChar('g'), ModUnicodeChar('n')},		// Guarani
	{ModUnicodeChar('g'), ModUnicodeChar('u')},		// Gujarati

	{ModUnicodeChar('h'), ModUnicodeChar('a')},		// Hausa
	{ModUnicodeChar('h'), ModUnicodeChar('e')},		// Hebrew (formerly iw)
	{ModUnicodeChar('h'), ModUnicodeChar('i')},		// Hindi
	{ModUnicodeChar('h'), ModUnicodeChar('r')},		// Croatian
	{ModUnicodeChar('h'), ModUnicodeChar('u')},		// Hungarian
	{ModUnicodeChar('h'), ModUnicodeChar('y')},		// Armenian

	{ModUnicodeChar('i'), ModUnicodeChar('a')},		// Interlingua
	{ModUnicodeChar('i'), ModUnicodeChar('d')},		// Indonesian (formerly in)
	{ModUnicodeChar('i'), ModUnicodeChar('e')},		// Interlingue
	{ModUnicodeChar('i'), ModUnicodeChar('k')},		// Inupiak
	{ModUnicodeChar('i'), ModUnicodeChar('s')},		// Icelandic
	{ModUnicodeChar('i'), ModUnicodeChar('t')},		// Italian
	{ModUnicodeChar('i'), ModUnicodeChar('u')},		// Inuktitut

	{ModUnicodeChar('j'), ModUnicodeChar('a')},		// Japanese
	{ModUnicodeChar('j'), ModUnicodeChar('w')},		// Javanese

	{ModUnicodeChar('k'), ModUnicodeChar('a')},		// Georgian
	{ModUnicodeChar('k'), ModUnicodeChar('k')},		// Kazakh
	{ModUnicodeChar('k'), ModUnicodeChar('l')},		// Greenlandic
	{ModUnicodeChar('k'), ModUnicodeChar('m')},		// Cambodian
	{ModUnicodeChar('k'), ModUnicodeChar('n')},		// Kannada
	{ModUnicodeChar('k'), ModUnicodeChar('o')},		// Korean
	{ModUnicodeChar('k'), ModUnicodeChar('s')},		// Kashmiri
	{ModUnicodeChar('k'), ModUnicodeChar('u')},		// Kurdish
	{ModUnicodeChar('k'), ModUnicodeChar('y')},		// Kirghiz

	{ModUnicodeChar('l'), ModUnicodeChar('a')},		// Latin
	{ModUnicodeChar('l'), ModUnicodeChar('n')},		// Lingala
	{ModUnicodeChar('l'), ModUnicodeChar('o')},		// Laothian
	{ModUnicodeChar('l'), ModUnicodeChar('t')},		// Lithuanian
	{ModUnicodeChar('l'), ModUnicodeChar('v')},		// Latvian, Lettish

	{ModUnicodeChar('m'), ModUnicodeChar('g')},		// Malagasy
	{ModUnicodeChar('m'), ModUnicodeChar('i')},		// Maori
	{ModUnicodeChar('m'), ModUnicodeChar('k')},		// Macedonian
	{ModUnicodeChar('m'), ModUnicodeChar('l')},		// Malayalam
	{ModUnicodeChar('m'), ModUnicodeChar('n')},		// Mongolian
	{ModUnicodeChar('m'), ModUnicodeChar('o')},		// Moldavian
	{ModUnicodeChar('m'), ModUnicodeChar('r')},		// Marathi
	{ModUnicodeChar('m'), ModUnicodeChar('s')},		// Malay
	{ModUnicodeChar('m'), ModUnicodeChar('t')},		// Maltese
	{ModUnicodeChar('m'), ModUnicodeChar('y')},		// Burmese

	{ModUnicodeChar('n'), ModUnicodeChar('a')},		// Nauru
	{ModUnicodeChar('n'), ModUnicodeChar('e')},		// Nepali
	{ModUnicodeChar('n'), ModUnicodeChar('l')},		// Dutch
	{ModUnicodeChar('n'), ModUnicodeChar('o')},		// Norwegian

	{ModUnicodeChar('o'), ModUnicodeChar('c')},		// Occitan
	{ModUnicodeChar('o'), ModUnicodeChar('m')},		// (Afan) Oromo
	{ModUnicodeChar('o'), ModUnicodeChar('r')},		// Oriya

	{ModUnicodeChar('p'), ModUnicodeChar('a')},		// Punjabi
	{ModUnicodeChar('p'), ModUnicodeChar('l')},		// Polish
	{ModUnicodeChar('p'), ModUnicodeChar('s')},		// Pashto, Pushto
	{ModUnicodeChar('p'), ModUnicodeChar('t')},		// Portuguese

	{ModUnicodeChar('q'), ModUnicodeChar('u')},		// Quechua

	{ModUnicodeChar('r'), ModUnicodeChar('m')},		// Rhaeto-Romance
	{ModUnicodeChar('r'), ModUnicodeChar('n')},		// Kirundi
	{ModUnicodeChar('r'), ModUnicodeChar('o')},		// Romanian
	{ModUnicodeChar('r'), ModUnicodeChar('u')},		// Russian
	{ModUnicodeChar('r'), ModUnicodeChar('w')},		// Kinyarwanda

	{ModUnicodeChar('s'), ModUnicodeChar('a')},		// Sanskrit
	{ModUnicodeChar('s'), ModUnicodeChar('d')},		// Sindhi
	{ModUnicodeChar('s'), ModUnicodeChar('g')},		// Sangho
	{ModUnicodeChar('s'), ModUnicodeChar('h')},		// Serbo-Croatian
	{ModUnicodeChar('s'), ModUnicodeChar('i')},		// Sinhalese
	{ModUnicodeChar('s'), ModUnicodeChar('k')},		// Slovak
	{ModUnicodeChar('s'), ModUnicodeChar('l')},		// Slovenian
	{ModUnicodeChar('s'), ModUnicodeChar('m')},		// Samoan
	{ModUnicodeChar('s'), ModUnicodeChar('n')},		// Shona
	{ModUnicodeChar('s'), ModUnicodeChar('o')},		// Somali
	{ModUnicodeChar('s'), ModUnicodeChar('q')},		// Albanian
	{ModUnicodeChar('s'), ModUnicodeChar('r')},		// Serbian
	{ModUnicodeChar('s'), ModUnicodeChar('s')},		// Siswati
	{ModUnicodeChar('s'), ModUnicodeChar('t')},		// Sesotho
	{ModUnicodeChar('s'), ModUnicodeChar('u')},		// Sundanese
	{ModUnicodeChar('s'), ModUnicodeChar('v')},		// Swedish
	{ModUnicodeChar('s'), ModUnicodeChar('w')},		// Swahili

	{ModUnicodeChar('t'), ModUnicodeChar('a')},		// Tamil
	{ModUnicodeChar('t'), ModUnicodeChar('e')},		// Telugu
	{ModUnicodeChar('t'), ModUnicodeChar('g')},		// Tajik
	{ModUnicodeChar('t'), ModUnicodeChar('h')},		// Thai
	{ModUnicodeChar('t'), ModUnicodeChar('i')},		// Tigrinya
	{ModUnicodeChar('t'), ModUnicodeChar('k')},		// Turkmen
	{ModUnicodeChar('t'), ModUnicodeChar('l')},		// Tagalog
	{ModUnicodeChar('t'), ModUnicodeChar('n')},		// Setswana
	{ModUnicodeChar('t'), ModUnicodeChar('o')},		// Tonga
	{ModUnicodeChar('t'), ModUnicodeChar('r')},		// Turkish
	{ModUnicodeChar('t'), ModUnicodeChar('s')},		// Tsonga
	{ModUnicodeChar('t'), ModUnicodeChar('t')},		// Tatar
	{ModUnicodeChar('t'), ModUnicodeChar('w')},		// Twi

	{ModUnicodeChar('u'), ModUnicodeChar('g')},		// Uighur
	{ModUnicodeChar('u'), ModUnicodeChar('k')},		// Ukrainian
	{ModUnicodeChar('u'), ModUnicodeChar('r')},		// Urdu
	{ModUnicodeChar('u'), ModUnicodeChar('z')},		// Uzbek

	{ModUnicodeChar('v'), ModUnicodeChar('i')},		// Vietnamese
	{ModUnicodeChar('v'), ModUnicodeChar('o')},		// Volapuk

	{ModUnicodeChar('w'), ModUnicodeChar('o')},		// Wolof

	{ModUnicodeChar('x'), ModUnicodeChar('h')},		// Xhosa

	{ModUnicodeChar('y'), ModUnicodeChar('i')},		// Yiddish (formerly ji)
	{ModUnicodeChar('y'), ModUnicodeChar('o')},		// Yoruba

	{ModUnicodeChar('z'), ModUnicodeChar('a')},		// Zhuang
	{ModUnicodeChar('z'), ModUnicodeChar('h')},		// Chinese
	{ModUnicodeChar('z'), ModUnicodeChar('u')}		// Zulu
};

//	FUNCTION public
//	ModLanguage::toSymbol -- コードからシンボルを得る
//
//	NOTES
//		コードからシンボルを得る
//		不正なコードが与えられた場合には undefined を返す
//
//	ARGUMENTS
//		ModLanguageCode
//
//	RETURN
//		const ModUnicodeChar*
//
//	EXCEPTIONS
//		なし

// static
const ModUnicodeChar*
ModLanguage::toSymbol(ModLanguageCode code)
{
	return (ModLanguage::isValid(code)) ?
		ModLanguageCodeSymbol[code] : UndefinedSymbol;
}

//	FUNCTION public
//	ModLanguage::toCode -- シンボルからコードを得る
//
//	NOTES
//		シンボルからコードを得る
//		コードにない文字列が指定された場合には undefined を返す
//
//	ARGUMENTS
//		const ModUnicodeChar*
//		ModSize	len = 0
//
//	RETURN
//		ModLanguageCode
//
//	EXCEPTIONS
//		なし

// static
ModLanguageCode
ModLanguage::toCode(const ModUnicodeChar* symbol, ModSize len)
{
	if (len != 2) return ModLanguage::undefined;

	// よく使うものを先に調べる
	//	ja, en, fr, de, it, es, nl, zh
	if (ModUnicodeCharTrait::compare(ModLanguageCodeSymbol[ModLanguage::ja], symbol, ModFalse, len) == 0)
		return ModLanguage::ja;
	if (ModUnicodeCharTrait::compare(ModLanguageCodeSymbol[ModLanguage::en], symbol, ModFalse, len) == 0)
		return ModLanguage::en;
	if (ModUnicodeCharTrait::compare(ModLanguageCodeSymbol[ModLanguage::fr], symbol, ModFalse, len) == 0)
		return ModLanguage::fr;
	if (ModUnicodeCharTrait::compare(ModLanguageCodeSymbol[ModLanguage::de], symbol, ModFalse, len) == 0)
		return ModLanguage::de;
	if (ModUnicodeCharTrait::compare(ModLanguageCodeSymbol[ModLanguage::it], symbol, ModFalse, len) == 0)
		return ModLanguage::it;
	if (ModUnicodeCharTrait::compare(ModLanguageCodeSymbol[ModLanguage::es], symbol, ModFalse, len) == 0)
		return ModLanguage::es;
	if (ModUnicodeCharTrait::compare(ModLanguageCodeSymbol[ModLanguage::nl], symbol, ModFalse, len) == 0)
		return ModLanguage::nl;
	if (ModUnicodeCharTrait::compare(ModLanguageCodeSymbol[ModLanguage::zh], symbol, ModFalse, len) == 0)
		return ModLanguage::zh;

	// テーブルを調べる
	//	とりあえす線形サーチ
	for (int i = 1; i < ModLanguage::last; i++) {
		if (ModUnicodeCharTrait::compare(ModLanguageCodeSymbol[i], symbol, ModFalse, len) == 0)
			return (ModLanguageCode)i;
	}
	return ModLanguage::undefined;
}

//	FUNCTION public
//	ModLanguage::isValid -- コードが範囲内にあるか調べる
//
//	NOTES
//
//	ARGUMENTS
//		ModLanguageCode
//
//	RETURN
//		ModBoolean
//
//	EXCEPTIONS
//		なし

// static
ModBoolean
ModLanguage::isValid(ModLanguageCode	code)
{
	return (code <= ModLanguage::undefined || code >= ModLanguage::last) ?
			ModFalse : ModTrue;
}

//
// Copyright (c) 1999, 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
