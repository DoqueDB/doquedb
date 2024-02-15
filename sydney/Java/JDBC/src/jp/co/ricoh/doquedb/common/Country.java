// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Country.java -- 国・地域関連
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
 * 国・地域関連。
 * ModCountry::iso3166 の各国・地域コードと同値のフィールドをもつ。
 *
 */
public final class Country
{
	/** 未知の国・地域コード (=0) */
	public final static int	UNDEFINED = 0;

	/** AFGHANISTAN を示す国・地域コード (=1) */
	public final static int	AF = 1;
	/** ALBANIA を示す国・地域コード (=2) */
	public final static int	AL = 2;
	/** ALGERIA を示す国・地域コード (=3) */
	public final static int	DZ = 3;
	/** AMERICAN SAMOA を示す国・地域コード (=4) */
	public final static int	AS = 4;
	/** ANDORRA を示す国・地域コード (=5) */
	public final static int	AD = 5;
	/** ANGOLA を示す国・地域コード (=6) */
	public final static int	AO = 6;
	/** ANGUILLA を示す国・地域コード (=7) */
	public final static int	AI = 7;
	/** ANTARCTICA を示す国・地域コード (=8) */
	public final static int	AQ = 8;
	/** ANTIGUA AND BARBUDA を示す国・地域コード (=9) */
	public final static int	AG = 9;
	/** ARGENTINA を示す国・地域コード (=10) */
	public final static int	AR = 10;
	/** ARMENIA を示す国・地域コード (=11) */
	public final static int	AM = 11;
	/** ARUBA を示す国・地域コード (=12) */
	public final static int	AW = 12;
	/** AUSTRALIA を示す国・地域コード (=13) */
	public final static int	AU = 13;
	/** AUSTRIA を示す国・地域コード (=14) */
	public final static int	AT = 14;
	/** AZERBAIJAN を示す国・地域コード (=15) */
	public final static int	AZ = 15;
	/** BAHAMAS を示す国・地域コード (=16) */
	public final static int	BS = 16;
	/** BAHRAIN を示す国・地域コード (=17) */
	public final static int	BH = 17;
	/** BANGLADESH を示す国・地域コード (=18) */
	public final static int	BD = 18;
	/** BARBADOS を示す国・地域コード (=19) */
	public final static int	BB = 19;
	/** BELARUS を示す国・地域コード (=20) */
	public final static int	BY = 20;
	/** BELGIUM を示す国・地域コード (=21) */
	public final static int	BE = 21;
	/** BELIZE を示す国・地域コード (=22) */
	public final static int	BZ = 22;
	/** BENIN を示す国・地域コード (=23) */
	public final static int	BJ = 23;
	/** BERMUDA を示す国・地域コード (=24) */
	public final static int	BM = 24;
	/** BHUTAN を示す国・地域コード (=25) */
	public final static int	BT = 25;
	/** BOLIVIA を示す国・地域コード (=26) */
	public final static int	BO = 26;
	/** BOSNIA AND HERZEGOWINA を示す国・地域コード (=27) */
	public final static int	BA = 27;
	/** BOTSWANA を示す国・地域コード (=28) */
	public final static int	BW = 28;
	/** BOUVET ISLAND を示す国・地域コード (=29) */
	public final static int	BV = 29;
	/** BRAZIL を示す国・地域コード (=30) */
	public final static int	BR = 30;
	/** BRITISH INDIAN OCEAN TERRITORY を示す国・地域コード (=31) */
	public final static int	IO = 31;
	/** BRUNEI DARUSSALAM を示す国・地域コード (=32) */
	public final static int	BN = 32;
	/** BULGARIA を示す国・地域コード (=33) */
	public final static int	BG = 33;
	/** BURKINA FASO を示す国・地域コード (=34) */
	public final static int	BF = 34;
	/** BURUNDI を示す国・地域コード (=35) */
	public final static int	BI = 35;
	/** CAMBODIA を示す国・地域コード (=36) */
	public final static int	KH = 36;
	/** CAMEROON を示す国・地域コード (=37) */
	public final static int	CM = 37;
	/** CANADA を示す国・地域コード (=38) */
	public final static int	CA = 38;
	/** CAPE VERDE を示す国・地域コード (=39) */
	public final static int	CV = 39;
	/** CAYMAN ISLANDS を示す国・地域コード (=40) */
	public final static int	KY = 40;
	/** CENTRAL AFRICAN REPUBLIC を示す国・地域コード (=41) */
	public final static int	CF = 41;
	/** CHAD を示す国・地域コード (=42) */
	public final static int	TD = 42;
	/** CHILE を示す国・地域コード (=43) */
	public final static int	CL = 43;
	/** CHINA を示す国・地域コード (=44) */
	public final static int	CN = 44;
	/** CHRISTMAS ISLAND を示す国・地域コード (=45) */
	public final static int	CX = 45;
	/** COCOS (KEELING) ISLANDS を示す国・地域コード (=46) */
	public final static int	CC = 46;
	/** COLOMBIA を示す国・地域コード (=47) */
	public final static int	CO = 47;
	/** COMOROS を示す国・地域コード (=48) */
	public final static int	KM = 48;
	/** CONGO, Democratic Republic of (was Zaire) を示す国・地域コード (=49) */
	public final static int	CD = 49;
	/** CONGO, People's Republic of を示す国・地域コード (=50) */
	public final static int	CG = 50;
	/** COOK ISLANDS を示す国・地域コード (=51) */
	public final static int	CK = 51;
	/** COSTA RICA を示す国・地域コード (=52) */
	public final static int	CR = 52;
	/** COTE D'IVOIRE を示す国・地域コード (=53) */
	public final static int	CI = 53;
	/** CROATIA (local name: Hrvatska) を示す国・地域コード (=54) */
	public final static int	HR = 54;
	/** CUBA を示す国・地域コード (=55) */
	public final static int	CU = 55;
	/** CYPRUS を示す国・地域コード (=56) */
	public final static int	CY = 56;
	/** CZECH REPUBLIC を示す国・地域コード (=57) */
	public final static int	CZ = 57;
	/** DENMARK を示す国・地域コード (=58) */
	public final static int	DK = 58;
	/** DJIBOUTI を示す国・地域コード (=59) */
	public final static int	DJ = 59;
	/** DOMINICA を示す国・地域コード (=60) */
	public final static int	DM = 60;
	/** DOMINICAN REPUBLIC を示す国・地域コード (=61) */
	public final static int	DO = 61;
	/** EAST TIMOR を示す国・地域コード (=62) */
	public final static int	TL = 62;
	/** ECUADOR を示す国・地域コード (=63) */
	public final static int	EC = 63;
	/** EGYPT を示す国・地域コード (=64) */
	public final static int	EG = 64;
	/** EL SALVADOR を示す国・地域コード (=65) */
	public final static int	SV = 65;
	/** EQUATORIAL GUINEA を示す国・地域コード (=66) */
	public final static int	GQ = 66;
	/** ERITREA を示す国・地域コード (=67) */
	public final static int	ER = 67;
	/** ESTONIA を示す国・地域コード (=68) */
	public final static int	EE = 68;
	/** ETHIOPIA を示す国・地域コード (=69) */
	public final static int	ET = 69;
	/** FALKLAND ISLANDS (MALVINAS) を示す国・地域コード (=70) */
	public final static int	FK = 70;
	/** FAROE ISLANDS を示す国・地域コード (=71) */
	public final static int	FO = 71;
	/** FIJI を示す国・地域コード (=72) */
	public final static int	FJ = 72;
	/** FINLAND を示す国・地域コード (=73) */
	public final static int	FI = 73;
	/** FRANCE を示す国・地域コード (=74) */
	public final static int	FR = 74;
	/** FRANCE, METROPOLITAN を示す国・地域コード (=75) */
	public final static int	FX = 75;
	/** FRENCH GUIANA を示す国・地域コード (=76) */
	public final static int	GF = 76;
	/** FRENCH POLYNESIA を示す国・地域コード (=77) */
	public final static int	PF = 77;
	/** FRENCH SOUTHERN TERRITORIES を示す国・地域コード (=78) */
	public final static int	TF = 78;
	/** GABON を示す国・地域コード (=79) */
	public final static int	GA = 79;
	/** GAMBIA を示す国・地域コード (=80) */
	public final static int	GM = 80;
	/** GEORGIA を示す国・地域コード (=81) */
	public final static int	GE = 81;
	/** GERMANY を示す国・地域コード (=82) */
	public final static int	DE = 82;
	/** GHANA を示す国・地域コード (=83) */
	public final static int	GH = 83;
	/** GIBRALTAR を示す国・地域コード (=84) */
	public final static int	GI = 84;
	/** GREECE を示す国・地域コード (=85) */
	public final static int	GR = 85;
	/** GREENLAND を示す国・地域コード (=86) */
	public final static int	GL = 86;
	/** GRENADA を示す国・地域コード (=87) */
	public final static int	GD = 87;
	/** GUADELOUPE を示す国・地域コード (=88) */
	public final static int	GP = 88;
	/** GUAM を示す国・地域コード (=89) */
	public final static int	GU = 89;
	/** GUATEMALA を示す国・地域コード (=90) */
	public final static int	GT = 90;
	/** GUINEA を示す国・地域コード (=91) */
	public final static int	GN = 91;
	/** GUINEA-BISSAU を示す国・地域コード (=92) */
	public final static int	GW = 92;
	/** GUYANA を示す国・地域コード (=93) */
	public final static int	GY = 93;
	/** HAITI を示す国・地域コード (=94) */
	public final static int	HT = 94;
	/** HEARD AND MC DONALD ISLANDS を示す国・地域コード (=95) */
	public final static int	HM = 95;
	/** HONDURAS を示す国・地域コード (=96) */
	public final static int	HN = 96;
	/** HONG KONG を示す国・地域コード (=97) */
	public final static int	HK = 97;
	/** HUNGARY を示す国・地域コード (=98) */
	public final static int	HU = 98;
	/** ICELAND を示す国・地域コード (=99) */
	public final static int	IS = 99;
	/** INDIA を示す国・地域コード (=100) */
	public final static int	IN = 100;
	/** INDONESIA を示す国・地域コード (=101) */
	public final static int	ID = 101;
	/** IRAN (ISLAMIC REPUBLIC OF) を示す国・地域コード (=102) */
	public final static int	IR = 102;
	/** IRAQ を示す国・地域コード (=103) */
	public final static int	IQ = 103;
	/** IRELAND を示す国・地域コード (=104) */
	public final static int	IE = 104;
	/** ISRAEL を示す国・地域コード (=105) */
	public final static int	IL = 105;
	/** ITALY を示す国・地域コード (=106) */
	public final static int	IT = 106;
	/** JAMAICA を示す国・地域コード (=107) */
	public final static int	JM = 107;
	/** JAPAN を示す国・地域コード (=108) */
	public final static int	JP = 108;
	/** JORDAN を示す国・地域コード (=109) */
	public final static int	JO = 109;
	/** KAZAKHSTAN を示す国・地域コード (=110) */
	public final static int	KZ = 110;
	/** KENYA を示す国・地域コード (=111) */
	public final static int	KE = 111;
	/** KIRIBATI を示す国・地域コード (=112) */
	public final static int	KI = 112;
	/** KOREA, DEMOCRATIC PEOPLE'S REPUBLIC OF を示す国・地域コード (=113) */
	public final static int	KP = 113;
	/** KOREA, REPUBLIC OF を示す国・地域コード (=114) */
	public final static int	KR = 114;
	/** KUWAIT を示す国・地域コード (=115) */
	public final static int	KW = 115;
	/** KYRGYZSTAN を示す国・地域コード (=116) */
	public final static int	KG = 116;
	/** LAO PEOPLE'S DEMOCRATIC REPUBLIC を示す国・地域コード (=117) */
	public final static int	LA = 117;
	/** LATVIA を示す国・地域コード (=118) */
	public final static int	LV = 118;
	/** LEBANON を示す国・地域コード (=119) */
	public final static int	LB = 119;
	/** LESOTHO を示す国・地域コード (=120) */
	public final static int	LS = 120;
	/** LIBERIA を示す国・地域コード (=121) */
	public final static int	LR = 121;
	/** LIBYAN ARAB JAMAHIRIYA を示す国・地域コード (=122) */
	public final static int	LY = 122;
	/** LIECHTENSTEIN を示す国・地域コード (=123) */
	public final static int	LI = 123;
	/** LITHUANIA を示す国・地域コード (=124) */
	public final static int	LT = 124;
	/** LUXEMBOURG を示す国・地域コード (=125) */
	public final static int	LU = 125;
	/** MACAU を示す国・地域コード (=126) */
	public final static int	MO = 126;
	/** MACEDONIA, THE FORMER YUGOSLAV REPUBLIC OF を示す国・地域コード (=127) */
	public final static int	MK = 127;
	/** MADAGASCAR を示す国・地域コード (=128) */
	public final static int	MG = 128;
	/** MALAWI を示す国・地域コード (=129) */
	public final static int	MW = 129;
	/** MALAYSIA を示す国・地域コード (=130) */
	public final static int	MY = 130;
	/** MALDIVES を示す国・地域コード (=131) */
	public final static int	MV = 131;
	/** MALI を示す国・地域コード (=132) */
	public final static int	ML = 132;
	/** MALTA を示す国・地域コード (=133) */
	public final static int	MT = 133;
	/** MARSHALL ISLANDS を示す国・地域コード (=134) */
	public final static int	MH = 134;
	/** MARTINIQUE を示す国・地域コード (=135) */
	public final static int	MQ = 135;
	/** MAURITANIA を示す国・地域コード (=136) */
	public final static int	MR = 136;
	/** MAURITIUS を示す国・地域コード (=137) */
	public final static int	MU = 137;
	/** MAYOTTE を示す国・地域コード (=138) */
	public final static int	YT = 138;
	/** MEXICO を示す国・地域コード (=139) */
	public final static int	MX = 139;
	/** MICRONESIA, FEDERATED STATES OF を示す国・地域コード (=140) */
	public final static int	FM = 140;
	/** MOLDOVA, REPUBLIC OF を示す国・地域コード (=141) */
	public final static int	MD = 141;
	/** MONACO を示す国・地域コード (=142) */
	public final static int	MC = 142;
	/** MONGOLIA を示す国・地域コード (=143) */
	public final static int	MN = 143;
	/** MONTSERRAT を示す国・地域コード (=144) */
	public final static int	MS = 144;
	/** MOROCCO を示す国・地域コード (=145) */
	public final static int	MA = 145;
	/** MOZAMBIQUE を示す国・地域コード (=146) */
	public final static int	MZ = 146;
	/** MYANMAR を示す国・地域コード (=147) */
	public final static int	MM = 147;
	/** NAMIBIA を示す国・地域コード (=148) */
	public final static int	NA = 148;
	/** NAURU を示す国・地域コード (=149) */
	public final static int	NR = 149;
	/** NEPAL を示す国・地域コード (=150) */
	public final static int	NP = 150;
	/** NETHERLANDS を示す国・地域コード (=151) */
	public final static int	NL = 151;
	/** NETHERLANDS ANTILLES を示す国・地域コード (=152) */
	public final static int	AN = 152;
	/** NEW CALEDONIA を示す国・地域コード (=153) */
	public final static int	NC = 153;
	/** NEW ZEALAND を示す国・地域コード (=154) */
	public final static int	NZ = 154;
	/** NICARAGUA を示す国・地域コード (=155) */
	public final static int	NI = 155;
	/** NIGER を示す国・地域コード (=156) */
	public final static int	NE = 156;
	/** NIGERIA を示す国・地域コード (=157) */
	public final static int	NG = 157;
	/** NIUE を示す国・地域コード (=158) */
	public final static int	NU = 158;
	/** norfolk ISLAND を示す国・地域コード (=159) */
	public final static int	NF = 159;
	/** NORTHERN MARIANA ISLANDS を示す国・地域コード (=160) */
	public final static int	MP = 160;
	/** NORWAY を示す国・地域コード (=161) */
	public final static int	NO = 161;
	/** OMAN を示す国・地域コード (=162) */
	public final static int	OM = 162;
	/** PAKISTAN を示す国・地域コード (=163) */
	public final static int	PK = 163;
	/** PALAU を示す国・地域コード (=164) */
	public final static int	PW = 164;
	/** PALESTINIAN TERRITORY, Occupied を示す国・地域コード (=165) */
	public final static int	PS = 165;
	/** PANAMA を示す国・地域コード (=166) */
	public final static int	PA = 166;
	/** PAPUA NEW GUINEA を示す国・地域コード (=167) */
	public final static int	PG = 167;
	/** PARAGUAY を示す国・地域コード (=168) */
	public final static int	PY = 168;
	/** PERU を示す国・地域コード (=169) */
	public final static int	PE = 169;
	/** PHILIPPINES を示す国・地域コード (=170) */
	public final static int	PH = 170;
	/** PITCAIRN を示す国・地域コード (=171) */
	public final static int	PN = 171;
	/** POLAND を示す国・地域コード (=172) */
	public final static int	PL = 172;
	/** PORTUGAL を示す国・地域コード (=173) */
	public final static int	PT = 173;
	/** PUERTO RICO を示す国・地域コード (=174) */
	public final static int	PR = 174;
	/** QATAR を示す国・地域コード (=175) */
	public final static int	QA = 175;
	/** REUNION を示す国・地域コード (=176) */
	public final static int	RE = 176;
	/** ROMANIA を示す国・地域コード (=177) */
	public final static int	RO = 177;
	/** RUSSIAN FEDERATION を示す国・地域コード (=178) */
	public final static int	RU = 178;
	/** RWANDA を示す国・地域コード (=179) */
	public final static int	RW = 179;
	/** SAINT KITTS AND NEVIS を示す国・地域コード (=180) */
	public final static int	KN = 180;
	/** SAINT LUCIA を示す国・地域コード (=181) */
	public final static int	LC = 181;
	/** SAINT VINCENT AND THE GRENADINES を示す国・地域コード (=182) */
	public final static int	VC = 182;
	/** SAMOA を示す国・地域コード (=183) */
	public final static int	WS = 183;
	/** SAN MARINO を示す国・地域コード (=184) */
	public final static int	SM = 184;
	/** SAO TOME AND PRINCIPE を示す国・地域コード (=185) */
	public final static int	ST = 185;
	/** SAUDI ARABIA を示す国・地域コード (=186) */
	public final static int	SA = 186;
	/** SENEGAL を示す国・地域コード (=187) */
	public final static int	SN = 187;
	/** SEYCHELLES を示す国・地域コード (=188) */
	public final static int	SC = 188;
	/** SIERRA LEONE を示す国・地域コード (=189) */
	public final static int	SL = 189;
	/** SINGAPORE を示す国・地域コード (=190) */
	public final static int	SG = 190;
	/** SLOVAKIA (Slovak Republic) を示す国・地域コード (=191) */
	public final static int	SK = 191;
	/** SLOVENIA を示す国・地域コード (=192) */
	public final static int	SI = 192;
	/** SOLOMON ISLANDS を示す国・地域コード (=193) */
	public final static int	SB = 193;
	/** SOMALIA を示す国・地域コード (=194) */
	public final static int	SO = 194;
	/** SOUTH AFRICA を示す国・地域コード (=195) */
	public final static int	ZA = 195;
	/** SOUTH GEORGIA AND THE SOUTH SANDWICH ISLANDS を示す国・地域コード (=196) */
	public final static int	GS = 196;
	/** SPAIN を示す国・地域コード (=197) */
	public final static int	ES = 197;
	/** SRI LANKA を示す国・地域コード (=198) */
	public final static int	LK = 198;
	/** ST. HELENA を示す国・地域コード (=199) */
	public final static int	SH = 199;
	/** ST. PIERRE AND MIQUELON を示す国・地域コード (=200) */
	public final static int	PM = 200;
	/** SUDAN を示す国・地域コード (=201) */
	public final static int	SD = 201;
	/** SURINAME を示す国・地域コード (=202) */
	public final static int	SR = 202;
	/** SVALBARD AND JAN MAYEN ISLANDS を示す国・地域コード (=203) */
	public final static int	SJ = 203;
	/** SWAZILAND を示す国・地域コード (=204) */
	public final static int	SZ = 204;
	/** SWEDEN を示す国・地域コード (=205) */
	public final static int	SE = 205;
	/** SWITZERLAND を示す国・地域コード (=206) */
	public final static int	CH = 206;
	/** SYRIAN ARAB REPUBLIC を示す国・地域コード (=207) */
	public final static int	SY = 207;
	/** TAIWAN を示す国・地域コード (=208) */
	public final static int	TW = 208;
	/** TAJIKISTAN を示す国・地域コード (=209) */
	public final static int	TJ = 209;
	/** TANZANIA, UNITED REPUBLIC OF を示す国・地域コード (=210) */
	public final static int	TZ = 210;
	/** THAILAND を示す国・地域コード (=211) */
	public final static int	TH = 211;
	/** TOGO を示す国・地域コード (=212) */
	public final static int	TG = 212;
	/** TOKELAU を示す国・地域コード (=213) */
	public final static int	TK = 213;
	/** TONGA を示す国・地域コード (=214) */
	public final static int	TO = 214;
	/** TRINIDAD AND TOBAGO を示す国・地域コード (=215) */
	public final static int	TT = 215;
	/** TUNISIA を示す国・地域コード (=216) */
	public final static int	TN = 216;
	/** TURKEY を示す国・地域コード (=217) */
	public final static int	TR = 217;
	/** TURKMENISTAN を示す国・地域コード (=218) */
	public final static int	TM = 218;
	/** TURKS AND CAICOS ISLANDS を示す国・地域コード (=219) */
	public final static int	TC = 219;
	/** TUVALU を示す国・地域コード (=220) */
	public final static int	TV = 220;
	/** UGANDA を示す国・地域コード (=221) */
	public final static int	UG = 221;
	/** UKRAINE を示す国・地域コード (=222) */
	public final static int	UA = 222;
	/** UNITED ARAB EMIRATES を示す国・地域コード (=223) */
	public final static int	AE = 223;
	/** UNITED KINGDOM を示す国・地域コード (=224) */
	public final static int	GB = 224;
	/** UNITED STATES を示す国・地域コード (=225) */
	public final static int	US = 225;
	/** UNITED STATES MINOR OUTLYING ISLANDS を示す国・地域コード (=226) */
	public final static int	UM = 226;
	/** URUGUAY を示す国・地域コード (=227) */
	public final static int	UY = 227;
	/** UZBEKISTAN を示す国・地域コード (=228) */
	public final static int	UZ = 228;
	/** VANUATU を示す国・地域コード (=229) */
	public final static int	VU = 229;
	/** VATICAN CITY STATE (HOLY SEE) を示す国・地域コード (=230) */
	public final static int	VA = 230;
	/** VENEZUELA を示す国・地域コード (=231) */
	public final static int	VE = 231;
	/** VIET NAM を示す国・地域コード (=232) */
	public final static int	VN = 232;
	/** VIRGIN ISLANDS (BRITISH) を示す国・地域コード (=233) */
	public final static int	VG = 233;
	/** VIRGIN ISLANDS (U.S.) を示す国・地域コード (=234) */
	public final static int	VI = 234;
	/** WALLIS AND FUTUNA ISLANDS を示す国・地域コード (=235) */
	public final static int	WF = 235;
	/** WESTERN SAHARA を示す国・地域コード (=236) */
	public final static int	EH = 236;
	/** YEMEN を示す国・地域コード (=237) */
	public final static int	YE = 237;
	/** YUGOSLAVIA を示す国・地域コード (=238) */
	public final static int	YU = 238;
	/** ZAMBIA を示す国・地域コード (=239) */
	public final static int	ZM = 239;
	/** ZIMBABWE を示す国・地域コード (=240) */
	public final static int	ZW = 240;

	/** 国・地域コードの範囲外開始を示す値 (=241) */
	public final static int	LAST = 241;

	/** 各国・地域コードに対応する文字列（シンボル）が格納されている配列。 */
	private final static String	CodeSymbol[] = {
		"undefined",
		"af",		//   1. AFGHANISTAN
		"al",		//   2. ALBANIA
		"dz",		//   3. ALGERIA
		"as",		//   4. AMERICAN SAMOA
		"ad",		//   5. ANDORRA
		"ao",		//   6. ANGOLA
		"ai",		//   7. ANGUILLA
		"aq",		//   8. ANTARCTICA
		"ag",		//   9. ANTIGUA AND BARBUDA
		"ar",		//  10. ARGENTINA
		"am",		//  11. ARMENIA
		"aw",		//  12. ARUBA
		"au",		//  13. AUSTRALIA
		"at",		//  14. AUSTRIA
		"az",		//  15. AZERBAIJAN
		"bs",		//  16. BAHAMAS
		"bh",		//  17. BAHRAIN
		"bd",		//  18. BANGLADESH
		"bb",		//  19. BARBADOS
		"by",		//  20. BELARUS
		"be",		//  21. BELGIUM
		"bz",		//  22. BELIZE
		"bj",		//  23. BENIN
		"bm",		//  24. BERMUDA
		"bt",		//  25. BHUTAN
		"bo",		//  26. BOLIVIA
		"ba",		//  27. BOSNIA AND HERZEGOWINA
		"bw",		//  28. BOTSWANA
		"bv",		//  29. BOUVET ISLAND
		"br",		//  30. BRAZIL
		"io",		//  31. BRITISH INDIAN OCEAN TERRITORY
		"bn",		//  32. BRUNEI DARUSSALAM
		"bg",		//  33. BULGARIA
		"bf",		//  34. BURKINA FASO
		"bi",		//  35. BURUNDI
		"kh",		//  36. CAMBODIA
		"cm",		//  37. CAMEROON
		"ca",		//  38. CANADA
		"cv",		//  39. CAPE VERDE
		"ky",		//  40. CAYMAN ISLANDS
		"cf",		//  41. CENTRAL AFRICAN REPUBLIC
		"td",		//  42. CHAD
		"cl",		//  43. CHILE
		"cn",		//  44. CHINA
		"cx",		//  45. CHRISTMAS ISLAND
		"cc",		//  46. COCOS (KEELING) ISLANDS
		"co",		//  47. COLOMBIA
		"km",		//  48. COMOROS
		"cd",		//  49. CONGO, Democratic Republic of (was Zaire)
		"cg",		//  50. CONGO, People's Republic of
		"ck",		//  51. COOK ISLANDS
		"cr",		//  52. COSTA RICA
		"ci",		//  53. COTE D'IVOIRE
		"hr",		//  54. CROATIA (local name: Hrvatska)
		"cu",		//  55. CUBA
		"cy",		//  56. CYPRUS
		"cz",		//  57. CZECH REPUBLIC
		"dk",		//  58. DENMARK
		"dj",		//  59. DJIBOUTI
		"dm",		//  60. DOMINICA
		"do",		//  61. DOMINICAN REPUBLIC
		"tl",		//  62. EAST TIMOR
		"ec",		//  63. ECUADOR
		"eg",		//  64. EGYPT
		"sv",		//  65. EL SALVADOR
		"gq",		//  66. EQUATORIAL GUINEA
		"er",		//  67. ERITREA
		"ee",		//  68. ESTONIA
		"et",		//  69. ETHIOPIA
		"fk",		//  70. FALKLAND ISLANDS (MALVINAS)
		"fo",		//  71. FAROE ISLANDS
		"fj",		//  72. FIJI
		"fi",		//  73. FINLAND
		"fr",		//  74. FRANCE
		"fx",		//  75. FRANCE, METROPOLITAN
		"gf",		//  76. FRENCH GUIANA
		"pf",		//  77. FRENCH POLYNESIA
		"tf",		//  78. FRENCH SOUTHERN TERRITORIES
		"ga",		//  79. GABON
		"gm",		//  80. GAMBIA
		"ge",		//  81. GEORGIA
		"de",		//  82. GERMANY
		"gh",		//  83. GHANA
		"gi",		//  84. GIBRALTAR
		"gr",		//  85. GREECE
		"gl",		//  86. GREENLAND
		"gd",		//  87. GRENADA
		"gp",		//  88. GUADELOUPE
		"gu",		//  89. GUAM
		"gt",		//  90. GUATEMALA
		"gn",		//  91. GUINEA
		"gw",		//  92. GUINEA-BISSAU
		"gy",		//  93. GUYANA
		"ht",		//  94. HAITI
		"hm",		//  95. HEARD AND MC DONALD ISLANDS
		"hn",		//  96. HONDURAS
		"hk",		//  97. HONG KONG
		"hu",		//  98. HUNGARY
		"is",		//  99. ICELAND
		"in",		// 100. INDIA
		"id",		// 101. INDONESIA
		"ir",		// 102. IRAN (ISLAMIC REPUBLIC OF)
		"iq",		// 103. IRAQ
		"ie",		// 104. IRELAND
		"il",		// 105. ISRAEL
		"it",		// 106. ITALY
		"jm",		// 107. JAMAICA
		"jp",		// 108. JAPAN
		"jo",		// 109. JORDAN
		"kz",		// 110. KAZAKHSTAN
		"ke",		// 111. KENYA
		"ki",		// 112. KIRIBATI
		"kp",		// 113. KOREA, DEMOCRATIC PEOPLE'S REPUBLIC OF
		"kr",		// 114. KOREA, REPUBLIC OF
		"kw",		// 115. KUWAIT
		"kg",		// 116. KYRGYZSTAN
		"la",		// 117. LAO PEOPLE'S DEMOCRATIC REPUBLIC
		"lv",		// 118. LATVIA
		"lb",		// 119. LEBANON
		"ls",		// 120. LESOTHO
		"lr",		// 121. LIBERIA
		"ly",		// 122. LIBYAN ARAB JAMAHIRIYA
		"li",		// 123. LIECHTENSTEIN
		"lt",		// 124. LITHUANIA
		"lu",		// 125. LUXEMBOURG
		"mo",		// 126. MACAU
		"mk",		// 127. MACEDONIA, THE FORMER YUGOSLAV REPUBLIC OF
		"mg",		// 128. MADAGASCAR
		"mw",		// 129. MALAWI
		"my",		// 130. MALAYSIA
		"mv",		// 131. MALDIVES
		"ml",		// 132. MALI
		"mt",		// 133. MALTA
		"mh",		// 134. MARSHALL ISLANDS
		"mq",		// 135. MARTINIQUE
		"mr",		// 136. MAURITANIA
		"mu",		// 137. MAURITIUS
		"yt",		// 138. MAYOTTE
		"mx",		// 139. MEXICO
		"fm",		// 140. MICRONESIA, FEDERATED STATES OF
		"md",		// 141. MOLDOVA, REPUBLIC OF
		"mc",		// 142. MONACO
		"mn",		// 143. MONGOLIA
		"ms",		// 144. MONTSERRAT
		"ma",		// 145. MOROCCO
		"mz",		// 146. MOZAMBIQUE
		"mm",		// 147. MYANMAR
		"na",		// 148. NAMIBIA
		"nr",		// 149. NAURU
		"np",		// 150. NEPAL
		"nl",		// 151. NETHERLANDS
		"an",		// 152. NETHERLANDS ANTILLES
		"nc",		// 153. NEW CALEDONIA
		"nz",		// 154. NEW ZEALAND
		"ni",		// 155. NICARAGUA
		"ne",		// 156. NIGER
		"ng",		// 157. NIGERIA
		"nu",		// 158. NIUE
		"nf",		// 159. norfolk ISLAND
		"mp",		// 160. NORTHERN MARIANA ISLANDS
		"no",		// 161. NORWAY
		"om",		// 162. OMAN
		"pk",		// 163. PAKISTAN
		"pw",		// 164. PALAU
		"ps",		// 165. PALESTINIAN TERRITORY, Occupied
		"pa",		// 166. PANAMA
		"pg",		// 167. PAPUA NEW GUINEA
		"py",		// 168. PARAGUAY
		"pe",		// 169. PERU
		"ph",		// 170. PHILIPPINES
		"pn",		// 171. PITCAIRN
		"pl",		// 172. POLAND
		"pt",		// 173. PORTUGAL
		"pr",		// 174. PUERTO RICO
		"qa",		// 175. QATAR
		"re",		// 176. REUNION
		"ro",		// 177. ROMANIA
		"ru",		// 178. RUSSIAN FEDERATION
		"rw",		// 179. RWANDA
		"kn",		// 180. SAINT KITTS AND NEVIS
		"lc",		// 181. SAINT LUCIA
		"vc",		// 182. SAINT VINCENT AND THE GRENADINES
		"ws",		// 183. SAMOA
		"sm",		// 184. SAN MARINO
		"st",		// 185. SAO TOME AND PRINCIPE
		"sa",		// 186. SAUDI ARABIA
		"sn",		// 187. SENEGAL
		"sc",		// 188. SEYCHELLES
		"sl",		// 189. SIERRA LEONE
		"sg",		// 190. SINGAPORE
		"sk",		// 191. SLOVAKIA (Slovak Republic)
		"si",		// 192. SLOVENIA
		"sb",		// 193. SOLOMON ISLANDS
		"so",		// 194. SOMALIA
		"za",		// 195. SOUTH AFRICA
		"gs",		// 196. SOUTH GEORGIA AND THE SOUTH SANDWICH ISLANDS
		"es",		// 197. SPAIN
		"lk",		// 198. SRI LANKA
		"sh",		// 199. ST. HELENA
		"pm",		// 200. ST. PIERRE AND MIQUELON
		"sd",		// 201. SUDAN
		"sr",		// 202. SURINAME
		"sj",		// 203. SVALBARD AND JAN MAYEN ISLANDS
		"sz",		// 204. SWAZILAND
		"se",		// 205. SWEDEN
		"ch",		// 206. SWITZERLAND
		"sy",		// 207. SYRIAN ARAB REPUBLIC
		"tw",		// 208. TAIWAN
		"tj",		// 209. TAJIKISTAN
		"tz",		// 210. TANZANIA, UNITED REPUBLIC OF
		"th",		// 211. THAILAND
		"tg",		// 212. TOGO
		"tk",		// 213. TOKELAU
		"to",		// 214. TONGA
		"tt",		// 215. TRINIDAD AND TOBAGO
		"tn",		// 216. TUNISIA
		"tr",		// 217. TURKEY
		"tm",		// 218. TURKMENISTAN
		"tc",		// 219. TURKS AND CAICOS ISLANDS
		"tv",		// 220. TUVALU
		"ug",		// 221. UGANDA
		"ua",		// 222. UKRAINE
		"ae",		// 223. UNITED ARAB EMIRATES
		"gb",		// 224. UNITED KINGDOM
		"us",		// 225. UNITED STATES
		"um",		// 226. UNITED STATES MINOR OUTLYING ISLANDS
		"uy",		// 227. URUGUAY
		"uz",		// 228. UZBEKISTAN
		"vu",		// 229. VANUATU
		"va",		// 230. VATICAN CITY STATE (HOLY SEE)
		"ve",		// 231. VENEZUELA
		"vn",		// 232. VIET NAM
		"vg",		// 233. VIRGIN ISLANDS (BRITISH)
		"vi",		// 234. VIRGIN ISLANDS (U.S.)
		"wf",		// 235. WALLIS AND FUTUNA ISLANDS
		"eh",		// 236. WESTERN SAHARA
		"ye",		// 237. YEMEN
		"yu",		// 238. YUGOSLAVIA
		"zm",		// 239. ZAMBIA
		"zw",		// 240. ZIMBABWE
	};

	/**
	 * 国・地域コードから対応するシンボルを返す。
	 *
	 * @param	code_
	 *			国・地域コード
	 * @return	国・地域コードを示す文字列（シンボル）。
	 */
	public static String toSymbol(int	code_)
	{
		int	code = isValid(code_) ? code_ : Country.UNDEFINED;
		return Country.CodeSymbol[code];
	}

	/**
	 * シンボルから対応する国・地域コードを返す。
	 *
	 * @param	symbol_
	 *			国・地域コードを示す文字列（シンボル）。
	 * @return	国・地域コード。
	 */
	public static int toCode(String	symbol_)
	{
		if (symbol_.length() != 2) return Country.UNDEFINED;

		// よく使うものを先に調べる
		// CN(CHINA), TW(TAIWAN), JP(JAPAN), US(UNITED STATES),
		//  GB(UNITED KINGDOM), FR(FRANCE), DE(GERMANY), IT(ITALY),
		// ES(SPAIN), NL(NETHERLANDS)
		if (symbol_.compareToIgnoreCase(Country.CodeSymbol[Country.CN]) == 0) {
			return Country.CN;
		} else if (
			symbol_.compareToIgnoreCase(Country.CodeSymbol[Country.TW]) == 0) {
			return Country.TW;
		} else if (
			symbol_.compareToIgnoreCase(Country.CodeSymbol[Country.JP]) == 0) {
			return Country.JP;
		} else if (
			symbol_.compareToIgnoreCase(Country.CodeSymbol[Country.US]) == 0) {
			return Country.US;
		} else if (
			symbol_.compareToIgnoreCase(Country.CodeSymbol[Country.GB]) == 0) {
			return Country.GB;
		} else if (
			symbol_.compareToIgnoreCase(Country.CodeSymbol[Country.FR]) == 0) {
			return Country.FR;
		} else if (
			symbol_.compareToIgnoreCase(Country.CodeSymbol[Country.DE]) == 0) {
			return Country.DE;
		} else if (
			symbol_.compareToIgnoreCase(Country.CodeSymbol[Country.IT]) == 0) {
			return Country.IT;
		} else if (
			symbol_.compareToIgnoreCase(Country.CodeSymbol[Country.ES]) == 0) {
			return Country.ES;
		} else if (
			symbol_.compareToIgnoreCase(Country.CodeSymbol[Country.NL]) == 0) {
			return Country.NL;
		}

		for (int i = 1; i < Country.LAST; i++) {
			if (symbol_.compareToIgnoreCase(Country.CodeSymbol[i]) == 0) {
				return i;
			}
		}

		return Country.UNDEFINED;
	}

	/**
	 * 国・地域コードが範囲内であるかどうかを確認する。
	 *
	 * @param	code_
	 *			国・地域コード。
	 * @return	国・地域コードが範囲内の場合は <code>true</code> 、
	 *			範囲外の場合は <code>false</code> を返す。
	 */
	public static boolean isValid(int	code_)
	{
		if (code_ <= Country.UNDEFINED ||
			code_ >= Country.LAST) {
			return false;
		}
		return true;
	}
}

//
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
