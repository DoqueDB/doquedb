// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModCountry.cpp -- 国・地域関連
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


#include "ModCountry.h"
#include "ModUnicodeCharTrait.h"

//	VARIABLE
//	ModCountryCodeSymbol -- コードとシンボルのテーブル
//
//	NOTES

#define UCH(x)	ModUnicodeChar(x)

static const ModUnicodeChar	ModCountryUndefinedSymbol[] = {
	UCH('u'),
	UCH('n'),
	UCH('d'),
	UCH('e'),
	UCH('f'),
	UCH('i'),
	UCH('n'),
	UCH('e'),
	UCH('d'),
	UCH(0x0000)
};

static const ModUnicodeChar ModCountryCodeSymbol[][3] = {
	{UCH(0x0000)},
	{UCH('a'), UCH('f')}, //   1. AFGHANISTAN
	{UCH('a'), UCH('l')}, //   2. ALBANIA
	{UCH('d'), UCH('z')}, //   3. ALGERIA
	{UCH('a'), UCH('s')}, //   4. AMERICAN SAMOA
	{UCH('a'), UCH('d')}, //   5. ANDORRA
	{UCH('a'), UCH('o')}, //   6. ANGOLA
	{UCH('a'), UCH('i')}, //   7. ANGUILLA
	{UCH('a'), UCH('q')}, //   8. ANTARCTICA
	{UCH('a'), UCH('g')}, //   9. ANTIGUA AND BARBUDA
	{UCH('a'), UCH('r')}, //  10. ARGENTINA
	{UCH('a'), UCH('m')}, //  11. ARMENIA
	{UCH('a'), UCH('w')}, //  12. ARUBA
	{UCH('a'), UCH('u')}, //  13. AUSTRALIA
	{UCH('a'), UCH('t')}, //  14. AUSTRIA
	{UCH('a'), UCH('z')}, //  15. AZERBAIJAN
	{UCH('b'), UCH('s')}, //  16. BAHAMAS
	{UCH('b'), UCH('h')}, //  17. BAHRAIN
	{UCH('b'), UCH('d')}, //  18. BANGLADESH
	{UCH('b'), UCH('b')}, //  19. BARBADOS
	{UCH('b'), UCH('y')}, //  20. BELARUS
	{UCH('b'), UCH('e')}, //  21. BELGIUM
	{UCH('b'), UCH('z')}, //  22. BELIZE
	{UCH('b'), UCH('j')}, //  23. BENIN
	{UCH('b'), UCH('m')}, //  24. BERMUDA
	{UCH('b'), UCH('t')}, //  25. BHUTAN
	{UCH('b'), UCH('o')}, //  26. BOLIVIA
	{UCH('b'), UCH('a')}, //  27. BOSNIA AND HERZEGOWINA
	{UCH('b'), UCH('w')}, //  28. BOTSWANA
	{UCH('b'), UCH('v')}, //  29. BOUVET ISLAND
	{UCH('b'), UCH('r')}, //  30. BRAZIL
	{UCH('i'), UCH('o')}, //  31. BRITISH INDIAN OCEAN TERRITORY
	{UCH('b'), UCH('n')}, //  32. BRUNEI DARUSSALAM
	{UCH('b'), UCH('g')}, //  33. BULGARIA
	{UCH('b'), UCH('f')}, //  34. BURKINA FASO
	{UCH('b'), UCH('i')}, //  35. BURUNDI
	{UCH('k'), UCH('h')}, //  36. CAMBODIA
	{UCH('c'), UCH('m')}, //  37. CAMEROON
	{UCH('c'), UCH('a')}, //  38. CANADA
	{UCH('c'), UCH('v')}, //  39. CAPE VERDE
	{UCH('k'), UCH('y')}, //  40. CAYMAN ISLANDS
	{UCH('c'), UCH('f')}, //  41. CENTRAL AFRICAN REPUBLIC
	{UCH('t'), UCH('d')}, //  42. CHAD
	{UCH('c'), UCH('l')}, //  43. CHILE
	{UCH('c'), UCH('n')}, //  44. CHINA
	{UCH('c'), UCH('x')}, //  45. CHRISTMAS ISLAND
	{UCH('c'), UCH('c')}, //  46. COCOS (KEELING) ISLANDS
	{UCH('c'), UCH('o')}, //  47. COLOMBIA
	{UCH('k'), UCH('m')}, //  48. COMOROS
	{UCH('c'), UCH('d')}, //  49. CONGO, Democratic Republic of (was Zaire)
	{UCH('c'), UCH('g')}, //  50. CONGO, People's Republic of
	{UCH('c'), UCH('k')}, //  51. COOK ISLANDS
	{UCH('c'), UCH('r')}, //  52. COSTA RICA
	{UCH('c'), UCH('i')}, //  53. COTE D'IVOIRE
	{UCH('h'), UCH('r')}, //  54. CROATIA (local name: Hrvatska)
	{UCH('c'), UCH('u')}, //  55. CUBA
	{UCH('c'), UCH('y')}, //  56. CYPRUS
	{UCH('c'), UCH('z')}, //  57. CZECH REPUBLIC
	{UCH('d'), UCH('k')}, //  58. DENMARK
	{UCH('d'), UCH('j')}, //  59. DJIBOUTI
	{UCH('d'), UCH('m')}, //  60. DOMINICA
	{UCH('d'), UCH('o')}, //  61. DOMINICAN REPUBLIC
	{UCH('t'), UCH('l')}, //  62. EAST TIMOR
	{UCH('e'), UCH('c')}, //  63. ECUADOR
	{UCH('e'), UCH('g')}, //  64. EGYPT
	{UCH('s'), UCH('v')}, //  65. EL SALVADOR
	{UCH('g'), UCH('q')}, //  66. EQUATORIAL GUINEA
	{UCH('e'), UCH('r')}, //  67. ERITREA
	{UCH('e'), UCH('e')}, //  68. ESTONIA
	{UCH('e'), UCH('t')}, //  69. ETHIOPIA
	{UCH('f'), UCH('k')}, //  70. FALKLAND ISLANDS (MALVINAS)
	{UCH('f'), UCH('o')}, //  71. FAROE ISLANDS
	{UCH('f'), UCH('j')}, //  72. FIJI
	{UCH('f'), UCH('i')}, //  73. FINLAND
	{UCH('f'), UCH('r')}, //  74. FRANCE
	{UCH('f'), UCH('x')}, //  75. FRANCE, METROPOLITAN
	{UCH('g'), UCH('f')}, //  76. FRENCH GUIANA
	{UCH('p'), UCH('f')}, //  77. FRENCH POLYNESIA
	{UCH('t'), UCH('f')}, //  78. FRENCH SOUTHERN TERRITORIES
	{UCH('g'), UCH('a')}, //  79. GABON
	{UCH('g'), UCH('m')}, //  80. GAMBIA
	{UCH('g'), UCH('e')}, //  81. GEORGIA
	{UCH('d'), UCH('e')}, //  82. GERMANY
	{UCH('g'), UCH('h')}, //  83. GHANA
	{UCH('g'), UCH('i')}, //  84. GIBRALTAR
	{UCH('g'), UCH('r')}, //  85. GREECE
	{UCH('g'), UCH('l')}, //  86. GREENLAND
	{UCH('g'), UCH('d')}, //  87. GRENADA
	{UCH('g'), UCH('p')}, //  88. GUADELOUPE
	{UCH('g'), UCH('u')}, //  89. GUAM
	{UCH('g'), UCH('t')}, //  90. GUATEMALA
	{UCH('g'), UCH('n')}, //  91. GUINEA
	{UCH('g'), UCH('w')}, //  92. GUINEA-BISSAU
	{UCH('g'), UCH('y')}, //  93. GUYANA
	{UCH('h'), UCH('t')}, //  94. HAITI
	{UCH('h'), UCH('m')}, //  95. HEARD AND MC DONALD ISLANDS
	{UCH('h'), UCH('n')}, //  96. HONDURAS
	{UCH('h'), UCH('k')}, //  97. HONG KONG
	{UCH('h'), UCH('u')}, //  98. HUNGARY
	{UCH('i'), UCH('s')}, //  99. ICELAND
	{UCH('i'), UCH('n')}, // 100. INDIA
	{UCH('i'), UCH('d')}, // 101. INDONESIA
	{UCH('i'), UCH('r')}, // 102. IRAN (ISLAMIC REPUBLIC OF)
	{UCH('i'), UCH('q')}, // 103. IRAQ
	{UCH('i'), UCH('e')}, // 104. IRELAND
	{UCH('i'), UCH('l')}, // 105. ISRAEL
	{UCH('i'), UCH('t')}, // 106. ITALY
	{UCH('j'), UCH('m')}, // 107. JAMAICA
	{UCH('j'), UCH('p')}, // 108. JAPAN
	{UCH('j'), UCH('o')}, // 109. JORDAN
	{UCH('k'), UCH('z')}, // 110. KAZAKHSTAN
	{UCH('k'), UCH('e')}, // 111. KENYA
	{UCH('k'), UCH('i')}, // 112. KIRIBATI
	{UCH('k'), UCH('p')}, // 113. KOREA, DEMOCRATIC PEOPLE'S REPUBLIC OF
	{UCH('k'), UCH('r')}, // 114. KOREA, REPUBLIC OF
	{UCH('k'), UCH('w')}, // 115. KUWAIT
	{UCH('k'), UCH('g')}, // 116. KYRGYZSTAN
	{UCH('l'), UCH('a')}, // 117. LAO PEOPLE'S DEMOCRATIC REPUBLIC
	{UCH('l'), UCH('v')}, // 118. LATVIA
	{UCH('l'), UCH('b')}, // 119. LEBANON
	{UCH('l'), UCH('s')}, // 120. LESOTHO
	{UCH('l'), UCH('r')}, // 121. LIBERIA
	{UCH('l'), UCH('y')}, // 122. LIBYAN ARAB JAMAHIRIYA
	{UCH('l'), UCH('i')}, // 123. LIECHTENSTEIN
	{UCH('l'), UCH('t')}, // 124. LITHUANIA
	{UCH('l'), UCH('u')}, // 125. LUXEMBOURG
	{UCH('m'), UCH('o')}, // 126. MACAU
	{UCH('m'), UCH('k')}, // 127. MACEDONIA, THE FORMER YUGOSLAV REPUBLIC OF
	{UCH('m'), UCH('g')}, // 128. MADAGASCAR
	{UCH('m'), UCH('w')}, // 129. MALAWI
	{UCH('m'), UCH('y')}, // 130. MALAYSIA
	{UCH('m'), UCH('v')}, // 131. MALDIVES
	{UCH('m'), UCH('l')}, // 132. MALI
	{UCH('m'), UCH('t')}, // 133. MALTA
	{UCH('m'), UCH('h')}, // 134. MARSHALL ISLANDS
	{UCH('m'), UCH('q')}, // 135. MARTINIQUE
	{UCH('m'), UCH('r')}, // 136. MAURITANIA
	{UCH('m'), UCH('u')}, // 137. MAURITIUS
	{UCH('y'), UCH('t')}, // 138. MAYOTTE
	{UCH('m'), UCH('x')}, // 139. MEXICO
	{UCH('f'), UCH('m')}, // 140. MICRONESIA, FEDERATED STATES OF
	{UCH('m'), UCH('d')}, // 141. MOLDOVA, REPUBLIC OF
	{UCH('m'), UCH('c')}, // 142. MONACO
	{UCH('m'), UCH('n')}, // 143. MONGOLIA
	{UCH('m'), UCH('s')}, // 144. MONTSERRAT
	{UCH('m'), UCH('a')}, // 145. MOROCCO
	{UCH('m'), UCH('z')}, // 146. MOZAMBIQUE
	{UCH('m'), UCH('m')}, // 147. MYANMAR
	{UCH('n'), UCH('a')}, // 148. NAMIBIA
	{UCH('n'), UCH('r')}, // 149. NAURU
	{UCH('n'), UCH('p')}, // 150. NEPAL
	{UCH('n'), UCH('l')}, // 151. NETHERLANDS
	{UCH('a'), UCH('n')}, // 152. NETHERLANDS ANTILLES
	{UCH('n'), UCH('c')}, // 153. NEW CALEDONIA
	{UCH('n'), UCH('z')}, // 154. NEW ZEALAND
	{UCH('n'), UCH('i')}, // 155. NICARAGUA
	{UCH('n'), UCH('e')}, // 156. NIGER
	{UCH('n'), UCH('g')}, // 157. NIGERIA
	{UCH('n'), UCH('u')}, // 158. NIUE
	{UCH('n'), UCH('f')}, // 159. NORFOLK ISLAND
	{UCH('m'), UCH('p')}, // 160. NORTHERN MARIANA ISLANDS
	{UCH('n'), UCH('o')}, // 161. NORWAY
	{UCH('o'), UCH('m')}, // 162. OMAN
	{UCH('p'), UCH('k')}, // 163. PAKISTAN
	{UCH('p'), UCH('w')}, // 164. PALAU
	{UCH('p'), UCH('s')}, // 165. PALESTINIAN TERRITORY, Occupied
	{UCH('p'), UCH('a')}, // 166. PANAMA
	{UCH('p'), UCH('g')}, // 167. PAPUA NEW GUINEA
	{UCH('p'), UCH('y')}, // 168. PARAGUAY
	{UCH('p'), UCH('e')}, // 169. PERU
	{UCH('p'), UCH('h')}, // 170. PHILIPPINES
	{UCH('p'), UCH('n')}, // 171. PITCAIRN
	{UCH('p'), UCH('l')}, // 172. POLAND
	{UCH('p'), UCH('t')}, // 173. PORTUGAL
	{UCH('p'), UCH('r')}, // 174. PUERTO RICO
	{UCH('q'), UCH('a')}, // 175. QATAR
	{UCH('r'), UCH('e')}, // 176. REUNION
	{UCH('r'), UCH('o')}, // 177. ROMANIA
	{UCH('r'), UCH('u')}, // 178. RUSSIAN FEDERATION
	{UCH('r'), UCH('w')}, // 179. RWANDA
	{UCH('k'), UCH('n')}, // 180. SAINT KITTS AND NEVIS
	{UCH('l'), UCH('c')}, // 181. SAINT LUCIA
	{UCH('v'), UCH('c')}, // 182. SAINT VINCENT AND THE GRENADINES
	{UCH('w'), UCH('s')}, // 183. SAMOA
	{UCH('s'), UCH('m')}, // 184. SAN MARINO
	{UCH('s'), UCH('t')}, // 185. SAO TOME AND PRINCIPE
	{UCH('s'), UCH('a')}, // 186. SAUDI ARABIA
	{UCH('s'), UCH('n')}, // 187. SENEGAL
	{UCH('s'), UCH('c')}, // 188. SEYCHELLES
	{UCH('s'), UCH('l')}, // 189. SIERRA LEONE
	{UCH('s'), UCH('g')}, // 190. SINGAPORE
	{UCH('s'), UCH('k')}, // 191. SLOVAKIA (Slovak Republic)
	{UCH('s'), UCH('i')}, // 192. SLOVENIA
	{UCH('s'), UCH('b')}, // 193. SOLOMON ISLANDS
	{UCH('s'), UCH('o')}, // 194. SOMALIA
	{UCH('z'), UCH('a')}, // 195. SOUTH AFRICA
	{UCH('g'), UCH('s')}, // 196. SOUTH GEORGIA AND THE SOUTH SANDWICH ISLANDS
	{UCH('e'), UCH('s')}, // 197. SPAIN
	{UCH('l'), UCH('k')}, // 198. SRI LANKA
	{UCH('s'), UCH('h')}, // 199. ST. HELENA
	{UCH('p'), UCH('m')}, // 200. ST. PIERRE AND MIQUELON
	{UCH('s'), UCH('d')}, // 201. SUDAN
	{UCH('s'), UCH('r')}, // 202. SURINAME
	{UCH('s'), UCH('j')}, // 203. SVALBARD AND JAN MAYEN ISLANDS
	{UCH('s'), UCH('z')}, // 204. SWAZILAND
	{UCH('s'), UCH('e')}, // 205. SWEDEN
	{UCH('c'), UCH('h')}, // 206. SWITZERLAND
	{UCH('s'), UCH('y')}, // 207. SYRIAN ARAB REPUBLIC
	{UCH('t'), UCH('w')}, // 208. TAIWAN
	{UCH('t'), UCH('j')}, // 209. TAJIKISTAN
	{UCH('t'), UCH('z')}, // 210. TANZANIA, UNITED REPUBLIC OF
	{UCH('t'), UCH('h')}, // 211. THAILAND
	{UCH('t'), UCH('g')}, // 212. TOGO
	{UCH('t'), UCH('k')}, // 213. TOKELAU
	{UCH('t'), UCH('o')}, // 214. TONGA
	{UCH('t'), UCH('t')}, // 215. TRINIDAD AND TOBAGO
	{UCH('t'), UCH('n')}, // 216. TUNISIA
	{UCH('t'), UCH('r')}, // 217. TURKEY
	{UCH('t'), UCH('m')}, // 218. TURKMENISTAN
	{UCH('t'), UCH('c')}, // 219. TURKS AND CAICOS ISLANDS
	{UCH('t'), UCH('v')}, // 220. TUVALU
	{UCH('u'), UCH('g')}, // 221. UGANDA
	{UCH('u'), UCH('a')}, // 222. UKRAINE
	{UCH('a'), UCH('e')}, // 223. UNITED ARAB EMIRATES
	{UCH('g'), UCH('b')}, // 224. UNITED KINGDOM
	{UCH('u'), UCH('s')}, // 225. UNITED STATES
	{UCH('u'), UCH('m')}, // 226. UNITED STATES MINOR OUTLYING ISLANDS
	{UCH('u'), UCH('y')}, // 227. URUGUAY
	{UCH('u'), UCH('z')}, // 228. UZBEKISTAN
	{UCH('v'), UCH('u')}, // 229. VANUATU
	{UCH('v'), UCH('a')}, // 230. VATICAN CITY STATE (HOLY SEE)
	{UCH('v'), UCH('e')}, // 231. VENEZUELA
	{UCH('v'), UCH('n')}, // 232. VIET NAM
	{UCH('v'), UCH('g')}, // 233. VIRGIN ISLANDS (BRITISH)
	{UCH('v'), UCH('i')}, // 234. VIRGIN ISLANDS (U.S.)
	{UCH('w'), UCH('f')}, // 235. WALLIS AND FUTUNA ISLANDS
	{UCH('e'), UCH('h')}, // 236. WESTERN SAHARA
	{UCH('y'), UCH('e')}, // 237. YEMEN
	{UCH('y'), UCH('u')}, // 238. YUGOSLAVIA
	{UCH('z'), UCH('m')}, // 239. ZAMBIA
	{UCH('z'), UCH('w')}  // 240. ZIMBABWE
};

//	FUNCTION public
//	ModCountry::toSymbol -- コードからシンボルを取得します
//
//	NOTES
//	不正なコードが与えられた場合には "undefined" を返します。
//
//	ARGUMENTS
//	ModCountryCode	code_
//		国・地域コード
//
//	RETURN
//	const ModUnicodeChar*
//		国・地域コードを示す文字列（シンボル）
//
//	EXCEPTIONS
//	なし

// static
const ModUnicodeChar*
ModCountry::toSymbol(ModCountryCode	code_)
{
	return
		(ModCountry::isValid(code_) == ModTrue) ?
			ModCountryCodeSymbol[code_] : ModCountryUndefinedSymbol;
}

//	FUNCTION public
//	ModCountry::toCode -- シンボルからコードを取得します
//
//	NOTES
//	コードにないシンボルが指定された場合には
//	ModCountryCode::undefined を返します。
//
//	ARGUMENTS
//	const ModUnicodeChar*	symbol_
//		国・地域コードを示す文字列（シンボル）
//	ModSize					len_ = 0
//		シンボルを構成する文字数
//
//	RETURN
//	ModCountryCode
//		国・地域コード
//
//	EXCEPTIONS
//	なし

// static
ModCountryCode
ModCountry::toCode(const ModUnicodeChar*	symbol_,
				   ModSize					len_)
{
	if (len_ != 2) return ModCountry::undefined;

	// よく使うものを先に調べる
	// CN(CHINA), TW(TAIWAN), JP(JAPAN), US(UNITED STATES), GB(UNITED KINGDOM),
	// FR(FRANCE), DE(GERMANY), IT(ITALY), ES(SPAIN), NL(NETHERLANDS)

	if (ModUnicodeCharTrait::compare(
			ModCountryCodeSymbol[ModCountry::cn],
			symbol_,
			ModFalse, // ← 大文字と小文字を区別せず
			len_) == 0) return ModCountry::cn;
	if (ModUnicodeCharTrait::compare(
			ModCountryCodeSymbol[ModCountry::tw],
			symbol_,
			ModFalse,
			len_) == 0) return ModCountry::tw;
	if (ModUnicodeCharTrait::compare(
			ModCountryCodeSymbol[ModCountry::jp],
			symbol_,
			ModFalse,
			len_) == 0) return ModCountry::jp;
	if (ModUnicodeCharTrait::compare(
			ModCountryCodeSymbol[ModCountry::us],
			symbol_,
			ModFalse,
			len_) == 0) return ModCountry::us;
	if (ModUnicodeCharTrait::compare(
			ModCountryCodeSymbol[ModCountry::gb],
			symbol_,
			ModFalse,
			len_) == 0) return ModCountry::gb;
	if (ModUnicodeCharTrait::compare(
			ModCountryCodeSymbol[ModCountry::fr],
			symbol_,
			ModFalse,
			len_) == 0) return ModCountry::fr;
	if (ModUnicodeCharTrait::compare(
			ModCountryCodeSymbol[ModCountry::de],
			symbol_,
			ModFalse,
			len_) == 0) return ModCountry::de;
	if (ModUnicodeCharTrait::compare(
			ModCountryCodeSymbol[ModCountry::it],
			symbol_,
			ModFalse,
			len_) == 0) return ModCountry::it;
	if (ModUnicodeCharTrait::compare(
			ModCountryCodeSymbol[ModCountry::es],
			symbol_,
			ModFalse,
			len_) == 0) return ModCountry::es;
	if (ModUnicodeCharTrait::compare(
			ModCountryCodeSymbol[ModCountry::nl],
			symbol_,
			ModFalse,
			len_) == 0) return ModCountry::nl;

	int	i;

	for (i = 1; i < ModCountry::last; i++) {

		if (ModUnicodeCharTrait::compare(symbol_,
										 ModCountryCodeSymbol[i],
										 ModFalse,
										 len_)
			== 0) {

			return (ModCountryCode)i;
		}
										 
	}

	return ModCountry::undefined;
}

//	FUNCTION public
//	ModCountry::isValid -- コードが範囲内であるかどうかを調べます
//
//	NOTES
//
//	ARGUMENTS
//	ModCountryCode	code_
//		国・地域コード
//
//	RETURN
//	コードが範囲内である場合には ModTrue 、範囲外の場合には ModFalse 。
//
//	EXCEPTIONS
//	なし

// static
ModBoolean
ModCountry::isValid(ModCountryCode	code_)
{
	return
		(code_ > ModCountry::undefined && code_ < ModCountry::last) ?
			ModTrue : ModFalse;
}

//
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
