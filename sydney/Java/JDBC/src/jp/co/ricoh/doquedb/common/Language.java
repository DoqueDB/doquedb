// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Language.java -- 言語種別関連
//
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.common;

/**
 * 言語種別関連。
 * ModLanguage::iso639 の各言語種別コードと同値のフィールドをもつ。
 *
 */
public final class Language
{
	/** 未知の言語種別コード (=0) */
	public final static int	UNDEFINED = 0;

	/** Afar を示す言語種別コード (=1) */
	public final static int	AA = 1;
	/** Abkhazian を示す言語種別コード (=2) */
	public final static int	AB = 2;
	/** Afrikaans を示す言語種別コード (=3) */
	public final static int	AF = 3;
	/** Amharic を示す言語種別コード (=4) */
	public final static int	AM = 4;
	/** Arabic を示す言語種別コード (=5) */
	public final static int	AR = 5;
	/** Assamese を示す言語種別コード (=6) */
	public final static int	AS = 6;
	/** Aymara を示す言語種別コード (=7) */
	public final static int	AY = 7;
	/** Azerbaijani を示す言語種別コード (=8) */
	public final static int	AZ = 8;
	/** Bashkir を示す言語種別コード (=9) */
	public final static int	BA = 9;
	/** Byelorussian を示す言語種別コード (=10) */
	public final static int	BE = 10;
	/** Bulgarian を示す言語種別コード (=11) */
	public final static int	BG = 11;
	/** Bihari を示す言語種別コード (=12) */
	public final static int	BH = 12;
	/** Bislama を示す言語種別コード (=13) */
	public final static int	BI = 13;
	/** Bengali; Bangla を示す言語種別コード (=14) */
	public final static int	BN = 14;
	/** Tibetan を示す言語種別コード (=15) */
	public final static int	BO = 15;
	/** Breton を示す言語種別コード (=16) */
	public final static int	BR = 16;
	/** Catalan を示す言語種別コード (=17) */
	public final static int	CA = 17;
	/** Corsican を示す言語種別コード (=18) */
	public final static int	CO = 18;
	/** Czech を示す言語種別コード (=19) */
	public final static int	CS = 19;
	/** Welsh を示す言語種別コード (=20) */
	public final static int	CY = 20;
	/** Danish を示す言語種別コード (=21) */
	public final static int	DA = 21;
	/** German を示す言語種別コード (=22) */
	public final static int	DE = 22;
	/** Bhutani を示す言語種別コード (=23) */
	public final static int	DZ = 23;
	/** Greek を示す言語種別コード (=24) */
	public final static int	EL = 24;
	/** English を示す言語種別コード (=25) */
	public final static int	EN = 25;
	/** Esperanto を示す言語種別コード (=26) */
	public final static int	EO = 26;
	/** Spanish を示す言語種別コード (=27) */
	public final static int	ES = 27;
	/** Estonian を示す言語種別コード (=28) */
	public final static int	ET = 28;
	/** Basque を示す言語種別コード (=29) */
	public final static int	EU = 29;
	/** Persian を示す言語種別コード (=30) */
	public final static int	FA = 30;
	/** Finnish を示す言語種別コード (=31) */
	public final static int	FI = 31;
	/** Fiji を示す言語種別コード (=32) */
	public final static int	FJ = 32;
	/** Faroese を示す言語種別コード (=33) */
	public final static int	FO = 33;
	/** French を示す言語種別コード (=34) */
	public final static int	FR = 34;
	/** Frisian を示す言語種別コード (=35) */
	public final static int	FY = 35;
	/** Irish を示す言語種別コード (=36) */
	public final static int	GA = 36;
	/** Scots Gaelic を示す言語種別コード (=37) */
	public final static int	GD = 37;
	/** Galician を示す言語種別コード (=38) */
	public final static int	GL = 38;
	/** Guarani を示す言語種別コード (=39) */
	public final static int	GN = 39;
	/** Gujarati を示す言語種別コード (=40) */
	public final static int	GU = 40;
	/** Hausa を示す言語種別コード (=41) */
	public final static int	HA = 41;
	/** Hebrew (formerly iw) を示す言語種別コード (=42) */
	public final static int	HE = 42;
	/** Hindi を示す言語種別コード (=43) */
	public final static int	HI = 43;
	/** Croatian を示す言語種別コード (=44) */
	public final static int	HR = 44;
	/** Hungarian を示す言語種別コード (=45) */
	public final static int	HU = 45;
	/** Armenian を示す言語種別コード (=46) */
	public final static int	HY = 46;
	/** Interlingua を示す言語種別コード (=47) */
	public final static int	IA = 47;
	/** Indonesian (formerly in) を示す言語種別コード (=48) */
	public final static int	ID = 48;
	/** Interlingue を示す言語種別コード (=49) */
	public final static int	IE = 49;
	/** Inupiak を示す言語種別コード (=50) */
	public final static int	IK = 50;
	/** Icelandic を示す言語種別コード (=51) */
	public final static int	IS = 51;
	/** Italian を示す言語種別コード (=52) */
	public final static int	IT = 52;
	/** Inuktitut を示す言語種別コード (=53) */
	public final static int	IU = 53;
	/** Japanese を示す言語種別コード (=54) */
	public final static int	JA = 54;
	/** Javanese を示す言語種別コード (=55) */
	public final static int	JW = 55;
	/** Georgian を示す言語種別コード (=56) */
	public final static int	KA = 56;
	/** Kazakh を示す言語種別コード (=57) */
	public final static int	KK = 57;
	/** Greenlandic を示す言語種別コード (=58) */
	public final static int	KL = 58;
	/** Cambodian を示す言語種別コード (=59) */
	public final static int	KM = 59;
	/** Kannada を示す言語種別コード (=60) */
	public final static int	KN = 60;
	/** Korean を示す言語種別コード (=61) */
	public final static int	KO = 61;
	/** Kashmiri を示す言語種別コード (=62) */
	public final static int	KS = 62;
	/** Kurdish を示す言語種別コード (=63) */
	public final static int	KU = 63;
	/** Kirghiz を示す言語種別コード (=64) */
	public final static int	KY = 64;
	/** Latin を示す言語種別コード (=65) */
	public final static int	LA = 65;
	/** Lingala を示す言語種別コード (=66) */
	public final static int	LN = 66;
	/** Laothian を示す言語種別コード (=67) */
	public final static int	LO = 67;
	/** Lithuanian を示す言語種別コード (=68) */
	public final static int	LT = 68;
	/** Latvian, Lettish を示す言語種別コード (=69) */
	public final static int	LV = 69;
	/** Malagasy を示す言語種別コード (=70) */
	public final static int	MG = 70;
	/** Maori を示す言語種別コード (=71) */
	public final static int	MI = 71;
	/** Macedonian を示す言語種別コード (=72) */
	public final static int	MK = 72;
	/** Malayalam を示す言語種別コード (=73) */
	public final static int	ML = 73;
	/** Mongolian を示す言語種別コード (=74) */
	public final static int	MN = 74;
	/** Moldavian を示す言語種別コード (=75) */
	public final static int	MO = 75;
	/** Marathi を示す言語種別コード (=76) */
	public final static int	MR = 76;
	/** Malay を示す言語種別コード (=77) */
	public final static int	MS = 77;
	/** Maltese を示す言語種別コード (=78) */
	public final static int	MT = 78;
	/** Burmese を示す言語種別コード (=79) */
	public final static int	MY = 79;
	/** Nauru を示す言語種別コード (=80) */
	public final static int	NA = 80;
	/** Nepali を示す言語種別コード (=81) */
	public final static int	NE = 81;
	/** Dutch を示す言語種別コード (=82) */
	public final static int	NL = 82;
	/** Norwegian を示す言語種別コード (=83) */
	public final static int	NO = 83;
	/** Occitan を示す言語種別コード (=84) */
	public final static int	OC = 84;
	/** (Afan) Oromo を示す言語種別コード (=85) */
	public final static int	OM = 85;
	/** Oriya を示す言語種別コード (=86) */
	public final static int	OR = 86;
	/** Punjabi を示す言語種別コード (=87) */
	public final static int	PA = 87;
	/** Polish を示す言語種別コード (=88) */
	public final static int	PL = 88;
	/** Pashto, Pushto を示す言語種別コード (=89) */
	public final static int	PS = 89;
	/** Portuguese を示す言語種別コード (=90) */
	public final static int	PT = 90;
	/** Quechua を示す言語種別コード (=91) */
	public final static int	QU = 91;
	/** Rhaeto-Romance を示す言語種別コード (=92) */
	public final static int	RM = 92;
	/** Kirundi を示す言語種別コード (=93) */
	public final static int	RN = 93;
	/** Romanian を示す言語種別コード (=94) */
	public final static int	RO = 94;
	/** Russian を示す言語種別コード (=95) */
	public final static int	RU = 95;
	/** Kinyarwanda を示す言語種別コード (=96) */
	public final static int	RW = 96;
	/** Sanskrit を示す言語種別コード (=97) */
	public final static int	SA = 97;
	/** Sindhi を示す言語種別コード (=98) */
	public final static int	SD = 98;
	/** Sangho を示す言語種別コード (=99) */
	public final static int	SG = 99;
	/** Serbo-Croatian を示す言語種別コード (=100) */
	public final static int	SH = 100;
	/** Sinhalese を示す言語種別コード (=101) */
	public final static int	SI = 101;
	/** Slovak を示す言語種別コード (=102) */
	public final static int	SK = 102;
	/** Slovenian を示す言語種別コード (=103) */
	public final static int	SL = 103;
	/** Samoan を示す言語種別コード (=104) */
	public final static int	SM = 104;
	/** Shona を示す言語種別コード (=105) */
	public final static int	SN = 105;
	/** Somali を示す言語種別コード (=106) */
	public final static int	SO = 106;
	/** Albanian を示す言語種別コード (=107) */
	public final static int	SQ = 107;
	/** Serbian を示す言語種別コード (=108) */
	public final static int	SR = 108;
	/** Siswati を示す言語種別コード (=109) */
	public final static int	SS = 109;
	/** Sesotho を示す言語種別コード (=110) */
	public final static int	ST = 110;
	/** Sundanese を示す言語種別コード (=111) */
	public final static int	SU = 111;
	/** Swedish を示す言語種別コード (=112) */
	public final static int	SV = 112;
	/** Swahili を示す言語種別コード (=113) */
	public final static int	SW = 113;
	/** Tamil を示す言語種別コード (=114) */
	public final static int	TA = 114;
	/** Telugu を示す言語種別コード (=115) */
	public final static int	TE = 115;
	/** Tajik を示す言語種別コード (=116) */
	public final static int	TG = 116;
	/** Thai を示す言語種別コード (=117) */
	public final static int	TH = 117;
	/** Tigrinya を示す言語種別コード (=118) */
	public final static int	TI = 118;
	/** Turkmen を示す言語種別コード (=119) */
	public final static int	TK = 119;
	/** Tagalog を示す言語種別コード (=120) */
	public final static int	TL = 120;
	/** Setswana を示す言語種別コード (=121) */
	public final static int	TN = 121;
	/** Tonga を示す言語種別コード (=122) */
	public final static int	TO = 122;
	/** Turkish を示す言語種別コード (=123) */
	public final static int	TR = 123;
	/** Tsonga を示す言語種別コード (=124) */
	public final static int	TS = 124;
	/** Tatar を示す言語種別コード (=125) */
	public final static int	TT = 125;
	/** Twi を示す言語種別コード (=126) */
	public final static int	TW = 126;
	/** Uighur を示す言語種別コード (=127) */
	public final static int	UG = 127;
	/** Ukrainian を示す言語種別コード (=128) */
	public final static int	UK = 128;
	/** Urdu を示す言語種別コード (=129) */
	public final static int	UR = 129;
	/** Uzbek を示す言語種別コード (=130) */
	public final static int	UZ = 130;
	/** Vietnamese を示す言語種別コード (=131) */
	public final static int	VI = 131;
	/** Volapuk を示す言語種別コード (=132) */
	public final static int	VO = 132;
	/** Wolof を示す言語種別コード (=133) */
	public final static int	WO = 133;
	/** Xhosa を示す言語種別コード (=134) */
	public final static int	XH = 134;
	/** Yiddish (formerly ji) を示す言語種別コード (=135) */
	public final static int	YI = 135;
	/** Yoruba を示す言語種別コード (=136) */
	public final static int	YO = 136;
	/** Zhuang を示す言語種別コード (=137) */
	public final static int	ZA = 137;
	/** Chinese を示す言語種別コード (=138) */
	public final static int	ZH = 138;
	/** Zulu を示す言語種別コード (=139) */
	public final static int	ZU = 139;

	/** 言語種別コードの範囲外開始を示す値 (=140) */
	public final static int	LAST = 140;

	/** 各言語種別コードに対応する文字列（シンボル）が格納されている配列。 */
	private final static String CodeSymbol[] = {
		"undefined",	// 		undefined
		"aa",			// 1.	Afar
		"ab",			// 2.	Abkhazian
		"af",			// 3.	Afrikaans
		"am",			// 4.	Amharic
		"ar",			// 5.	Arabic
		"as",			// 6.	Assamese
		"ay",			// 7.	Aymara
		"az",			// 8.	Azerbaijani

		"ba",			// 9.	Bashkir
		"be",			// 10.	Byelorussian
		"bg",			// 11.	Bulgarian
		"bh",			// 12.	Bihari
		"bi",			// 13.	Bislama
		"bn",			// 14.	Bengali; Bangla
		"bo",			// 15.	Tibetan
		"br",			// 16.	Breton

		"ca",			// 17.	Catalan
		"co",			// 18.	Corsican
		"cs",			// 19.	Czech
		"cy",			// 20.	Welsh

		"da",			// 21.	Danish
		"de",			// 22.	German
		"dz",			// 23.	Bhutani

		"el",			// 24.	Greek
		"en",			// 25.	English
		"eo",			// 26.	Esperanto
		"es",			// 27.	Spanish
		"et",			// 28.	Estonian
		"eu",			// 29.	Basque

		"fa",			// 30.	Persian
		"fi",			// 31.	Finnish
		"fj",			// 32.	Fiji
		"fo",			// 33.	Faroese
		"fr",			// 34.	French
		"fy",			// 35.	Frisian

		"ga",			// 36.	Irish
		"gd",			// 37.	Scots Gaelic
		"gl",			// 38.	Galician
		"gn",			// 39.	Guarani
		"gu",			// 40.	Gujarati

		"ha",			// 41.	Hausa
		"he",			// 42.	Hebrew (formerly iw)
		"hi",			// 43.	Hindi
		"hr",			// 44.	Croatian
		"hu",			// 45.	Hungarian
		"hy",			// 46.	Armenian

		"ia",			// 47.	Interlingua
		"id",			// 48.	Indonesian (formerly in)
		"ie",			// 49.	Interlingue
		"ik",			// 50.	Inupiak
		"is",			// 51.	Icelandic
		"it",			// 52.	Italian
		"iu",			// 53.	Inuktitut

		"ja",			// 54.	Japanese
		"jw",			// 55.	Javanese

		"ka",			// 56.	Georgian
		"kk",			// 57.	Kazakh
		"kl",			// 58.	Greenlandic
		"km",			// 59.	Cambodian
		"kn",			// 60.	Kannada
		"ko",			// 61.	Korean
		"ks",			// 62.	Kashmiri
		"ku",			// 63.	Kurdish
		"ky",			// 64.	Kirghiz

		"la",			// 65.	Latin
		"ln",			// 66.	Lingala
		"lo",			// 67.	Laothian
		"lt",			// 68.	Lithuanian
		"lv",			// 69.	Latvian, Lettish

		"mg",			// 70.	Malagasy
		"mi",			// 71.	Maori
		"mk",			// 72.	Macedonian
		"ml",			// 73.	Malayalam
		"mn",			// 74.	Mongolian
		"mo",			// 75.	Moldavian
		"mr",			// 76.	Marathi
		"ms",			// 77.	Malay
		"mt",			// 78.	Maltese
		"my",			// 79.	Burmese

		"na",			// 80.	Nauru
		"ne",			// 81.	Nepali
		"nl",			// 82.	Dutch
		"no",			// 83.	Norwegian

		"oc",			// 84.	Occitan
		"om",			// 85.	(Afan) Oromo
		"or",			// 86.	Oriya

		"pa",			// 87.	Punjabi
		"pl",			// 88.	Polish
		"ps",			// 89.	Pashto, Pushto
		"pt",			// 90.	Portuguese

		"qu",			// 91.	Quechua

		"rm",			// 92.	Rhaeto-Romance
		"rn",			// 93.	Kirundi
		"ro",			// 94.	Romanian
		"ru",			// 95.	Russian
		"rw",			// 96.	Kinyarwanda

		"sa",			// 97.	Sanskrit
		"sd",			// 98.	Sindhi
		"sg",			// 99.	Sangho
		"sh",			// 100.	Serbo-Croatian
		"si",			// 101.	Sinhalese
		"sk",			// 102.	Slovak
		"sl",			// 103.	Slovenian
		"sm",			// 104.	Samoan
		"sn",			// 105.	Shona
		"so",			// 106.	Somali
		"sq",			// 107.	Albanian
		"sr",			// 108.	Serbian
		"ss",			// 109.	Siswati
		"st",			// 110.	Sesotho
		"su",			// 111.	Sundanese
		"sv",			// 112.	Swedish
		"sw",			// 113.	Swahili

		"ta",			// 114.	Tamil
		"te",			// 115.	Telugu
		"tg",			// 116.	Tajik
		"th",			// 117.	Thai
		"ti",			// 118.	Tigrinya
		"tk",			// 119.	Turkmen
		"tl",			// 120.	Tagalog
		"tn",			// 121.	Setswana
		"to",			// 122.	Tonga
		"tr",			// 123.	Turkish
		"ts",			// 124.	Tsonga
		"tt",			// 125.	Tatar
		"tw",			// 126.	Twi

		"ug",			// 127.	Uighur
		"uk",			// 128.	Ukrainian
		"ur",			// 129.	Urdu
		"uz",			// 130.	Uzbek

		"vi",			// 131.	Vietnamese
		"vo",			// 132.	Volapuk

		"wo",			// 133.	Wolof

		"xh",			// 134.	Xhosa

		"yi",			// 135.	Yiddish (formerly ji)
		"yo",			// 136.	Yoruba

		"za",			// 137.	Zhuang
		"zh",			// 138.	Chinese
		"zu"			// 139.	Zulu
	};

	/**
	 * 言語種別コードから対応するシンボルを返す。
	 *
	 * @param	code_
	 *			言語種別コード。
	 * @return	シンボル。
	 */
	public static String toSymbol(int	code_)
	{
		int	code = isValid(code_) ? code_ : Language.UNDEFINED;
		return Language.CodeSymbol[code];
	}

	/**
	 * シンボルから対応する言語種別コードを返す。
	 *
	 * @param	symbol_
	 *			 シンボル。
	 * @return	言語種別コード。
	 */
	public static int toCode(String	symbol_)
	{
		if (symbol_.length() != 2) return Language.UNDEFINED;

		// よく使うものを先に調べる
		//	ja, en, fr, de, it, es, nl, zh
		if (symbol_.compareToIgnoreCase(Language.CodeSymbol[Language.JA]) == 0)
		{
			return Language.JA;
		} else if (
			symbol_.compareToIgnoreCase(Language.CodeSymbol[Language.EN]) == 0)
		{
			return Language.EN;
		} else if (
			symbol_.compareToIgnoreCase(Language.CodeSymbol[Language.FR]) == 0)
		{
			return Language.FR;
		} else if (
			symbol_.compareToIgnoreCase(Language.CodeSymbol[Language.DE]) == 0)
		{
			return Language.DE;
		} else if (
			symbol_.compareToIgnoreCase(Language.CodeSymbol[Language.IT]) == 0)
		{
			return Language.IT;
		} else if (
			symbol_.compareToIgnoreCase(Language.CodeSymbol[Language.ES]) == 0)
		{
			return Language.ES;
		} else if (
			symbol_.compareToIgnoreCase(Language.CodeSymbol[Language.NL]) == 0)
		{
			return Language.NL;
		} else if (
			symbol_.compareToIgnoreCase(Language.CodeSymbol[Language.ZH]) == 0)
		{
			return Language.ZH;
		}

		// テーブルを調べる

		int	startIndex = Language.UNDEFINED;
		int	endIndex = Language.UNDEFINED;
		char	firstChar = symbol_.toLowerCase().charAt(0);
		switch (firstChar) {
		case 'a':
			startIndex	= Language.AA;
			endIndex	= Language.BA;
			break;
		case 'b':
			startIndex	= Language.BA;
			endIndex	= Language.CA;
			break;
		case 'c':
			startIndex	= Language.CA;
			endIndex	= Language.DA;
			break;
		case 'd':
			startIndex	= Language.DA;
			endIndex	= Language.EL;
			break;
		case 'e':
			startIndex	= Language.EL;
			endIndex	= Language.FA;
			break;
		case 'f':
			startIndex	= Language.FA;
			endIndex	= Language.GA;
			break;
		case 'g':
			startIndex	= Language.GA;
			endIndex	= Language.HA;
			break;
		case 'h':
			startIndex	= Language.HA;
			endIndex	= Language.IA;
			break;
		case 'i':
			startIndex	= Language.IA;
			endIndex	= Language.JA;
			break;
		case 'j':
			// Japanese は最初にチェックしているのでとばす。
			startIndex	= Language.JW;
			endIndex	= Language.KA;
			break;
		case 'k':
			startIndex	= Language.KA;
			endIndex	= Language.LA;
			break;
		case 'l':
			startIndex	= Language.LA;
			endIndex	= Language.MG;
			break;
		case 'm':
			startIndex	= Language.MG;
			endIndex	= Language.NA;
			break;
		case 'n':
			startIndex	= Language.NA;
			endIndex	= Language.OC;
			break;
		case 'o':
			startIndex	= Language.OC;
			endIndex	= Language.PA;
			break;
		case 'p':
			startIndex	= Language.PA;
			endIndex	= Language.QU;
			break;
		case 'q':
			startIndex	= Language.QU;
			endIndex	= Language.RM;
			break;
		case 'r':
			startIndex	= Language.RM;
			endIndex	= Language.SA;
			break;
		case 's':
			startIndex	= Language.SA;
			endIndex	= Language.TA;
			break;
		case 't':
			startIndex	= Language.TA;
			endIndex	= Language.UG;
			break;
		case 'u':
			startIndex	= Language.UG;
			endIndex	= Language.VI;
			break;
		case 'v':
			startIndex	= Language.VI;
			endIndex	= Language.WO;
			break;
		case 'w':
			startIndex	= Language.WO;
			endIndex	= Language.XH;
			break;
		case 'x':
			startIndex	= Language.XH;
			endIndex	= Language.YI;
			break;
		case 'y':
			startIndex	= Language.YI;
			endIndex	= Language.ZA;
			break;
		case 'z':
			startIndex	= Language.ZA;
			endIndex	= Language.LAST;
			break;
		}

		for (int i = startIndex; i < endIndex; i++) {

			if (symbol_.compareToIgnoreCase(Language.CodeSymbol[i]) == 0) {
				return i;
			}
		}

		return Language.UNDEFINED;
	}

	/**
	 * 言語種別コードが範囲内であるかどうかを確認する。
	 *
	 * @param	code_
	 *			言語種別コード。
	 * @return	言語種別コードが範囲内の場合は <code>true</code> 、
	 *        	範囲外の場合は <code>false</code> を返す。
	 */
	public static boolean isValid(int	code_)
	{
		if (code_ <= Language.UNDEFINED ||
			code_ >= Language.LAST) {
			return false;
		}
		return true;
	}
}

//
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
