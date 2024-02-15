// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModLanguage.h -- ModLanguage のクラス定義
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

#ifndef	__ModLanguage_H__
#define __ModLanguage_H__

#include "ModCommonDLL.h"
#include "ModTypes.h"
#include "ModUnicodeChar.h"
#include "ModCountry.h"
#include "ModPair.h"

// CLASS
// ModLanguage -- 言語種別関連
//
// NOTES

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する


class ModLanguage
{
public:

	enum iso639
	{
		undefined =		0,

		// 以下は ftp://dkuug.dk/i18n/ISO_639 から入手した
		// ISO639 のシンボル

		aa,		// Afar
		ab,		// Abkhazian
		af,		// Afrikaans
		am,		// Amharic
		ar,		// Arabic
		as,		// Assamese
		ay,		// Aymara
		az,		// Azerbaijani

		ba,		// Bashkir
		be,		// Byelorussian
		bg,		// Bulgarian
		bh,		// Bihari
		bi,		// Bislama
		bn,		// Bengali; Bangla
		bo,		// Tibetan
		br,		// Breton

		ca,		// Catalan
		co,		// Corsican
		cs,		// Czech
		cy,		// Welsh

		da,		// Danish
		de,		// German
		dz,		// Bhutani

		el,		// Greek
		en,		// English
		eo,		// Esperanto
		es,		// Spanish
		et,		// Estonian
		eu,		// Basque

		fa,		// Persian
		fi,		// Finnish
		fj,		// Fiji
		fo,		// Faroese
		fr,		// French
		fy,		// Frisian

		ga,		// Irish
		gd,		// Scots Gaelic
		gl,		// Galician
		gn,		// Guarani
		gu,		// Gujarati

		ha,		// Hausa
		he,		// Hebrew (formerly iw)
		hi,		// Hindi
		hr,		// Croatian
		hu,		// Hungarian
		hy,		// Armenian

		ia,		// Interlingua
		id,		// Indonesian (formerly in)
		ie,		// Interlingue
		ik,		// Inupiak
		is,		// Icelandic
		it,		// Italian
		iu,		// Inuktitut

		ja,		// Japanese
		jw,		// Javanese

		ka,		// Georgian
		kk,		// Kazakh
		kl,		// Greenlandic
		km,		// Cambodian
		kn,		// Kannada
		ko,		// Korean
		ks,		// Kashmiri
		ku,		// Kurdish
		ky,		// Kirghiz

		la,		// Latin
		ln,		// Lingala
		lo,		// Laothian
		lt,		// Lithuanian
		lv,		// Latvian, Lettish

		mg,		// Malagasy
		mi,		// Maori
		mk,		// Macedonian
		ml,		// Malayalam
		mn,		// Mongolian
		mo,		// Moldavian
		mr,		// Marathi
		ms,		// Malay
		mt,		// Maltese
		my,		// Burmese

		na,		// Nauru
		ne,		// Nepali
		nl,		// Dutch
		no,		// Norwegian

		oc,		// Occitan
		om,		// (Afan) Oromo
		OR,		// Oriya

		pa,		// Punjabi
		pl,		// Polish
		ps,		// Pashto, Pushto
		pt,		// Portuguese

		qu,		// Quechua

		rm,		// Rhaeto-Romance
		rn,		// Kirundi
		ro,		// Romanian
		ru,		// Russian
		rw,		// Kinyarwanda

		sa,		// Sanskrit
		sd,		// Sindhi
		sg,		// Sangho
		sh,		// Serbo-Croatian
		si,		// Sinhalese
		sk,		// Slovak
		sl,		// Slovenian
		sm,		// Samoan
		sn,		// Shona
		so,		// Somali
		sq,		// Albanian
		sr,		// Serbian
		ss,		// Siswati
		st,		// Sesotho
		su,		// Sundanese
		sv,		// Swedish
		sw,		// Swahili

		ta,		// Tamil
		te,		// Telugu
		tg,		// Tajik
		th,		// Thai
		ti,		// Tigrinya
		tk,		// Turkmen
		tl,		// Tagalog
		tn,		// Setswana
		to,		// Tonga
		tr,		// Turkish
		ts,		// Tsonga
		tt,		// Tatar
		tw,		// Twi

		ug,		// Uighur
		uk,		// Ukrainian
		ur,		// Urdu
		uz,		// Uzbek

		vi,		// Vietnamese
		vo,		// Volapuk

		wo,		// Wolof

		xh,		// Xhosa

		yi,		// Yiddish (formerly ji)
		yo,		// Yoruba

		za,		// Zhuang
		zh,		// Chinese
		zu,		// Zulu
		last	// End of this enum
	};

	// コードからシンボルを得る
	ModCommonDLL
	static const ModUnicodeChar* toSymbol(iso639 code);

	// シンボルからコードを得る
	ModCommonDLL
	static ModLanguage::iso639	toCode(const ModUnicodeChar* symbol,
									   ModSize len = 0);

	// コードが範囲内であるかどうか
	ModCommonDLL
	static ModBoolean			isValid(iso639 code);
};

// TYPEDEF
// ModLanguageCode -- 言語種別を表す型
//
// NOTES

typedef ModLanguage::iso639	ModLanguageCode;

//	TYPEDEF
//	ModLanguageTag -- 言語タグを表す型
//
//	NOTES
//	言語種別と国・地域種別のペアを言語タグとする。

typedef ModPair<ModLanguageCode, ModCountryCode>	ModLanguageTag;

#endif	// __ModLanguage_H__

//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
