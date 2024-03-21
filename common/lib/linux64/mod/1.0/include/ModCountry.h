// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModCountry.h -- ModCountry のクラス定義
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

#ifndef __ModCountry_H__
#define __ModCountry_H__

#include "ModCommonDLL.h"
#include "ModTypes.h"
#include "ModUnicodeChar.h"

//	CLASS
//	ModCountry -- 国・地域関連
//
//	NOTES

class ModCountry
{
public:

	//	ENUM
	//	ModCountry::iso3166 -- ISO3166 で定義される国・地域コード
	//
	//	NOTES

	enum iso3166 {

		undefined = 0,

		// 以下は
		// http://userpage.chemie.fu-berlin.de/diverse/doc/ISO_3166.html
		// から入手した ISO3166 で定義される国・地域コード

		//		   Country                                           A2 A3  No.
		//---------------------------------------------------------------------
		af,		//   1. AFGHANISTAN                                  AF AFG 004
		al,		//   2. ALBANIA                                      AL ALB 008
		dz,		//   3. ALGERIA                                      DZ DZA 012
		as,		//   4. AMERICAN SAMOA                               AS ASM 016
		ad,		//   5. ANDORRA                                      AD AND 020
		ao,		//   6. ANGOLA                                       AO AGO 024
		ai,		//   7. ANGUILLA                                     AI AIA 660
		aq,		//   8. ANTARCTICA                                   AQ ATA 010
		ag,		//   9. ANTIGUA AND BARBUDA                          AG ATG 028
		ar,		//  10. ARGENTINA                                    AR ARG 032
		am,		//  11. ARMENIA                                      AM ARM 051
		aw,		//  12. ARUBA                                        AW ABW 533
		au,		//  13. AUSTRALIA                                    AU AUS 036
		at,		//  14. AUSTRIA                                      AT AUT 040
		az,		//  15. AZERBAIJAN                                   AZ AZE 031
		bs,		//  16. BAHAMAS                                      BS BHS 044
		bh,		//  17. BAHRAIN                                      BH BHR 048
		bd,		//  18. BANGLADESH                                   BD BGD 050
		bb,		//  19. BARBADOS                                     BB BRB 052
		by,		//  20. BELARUS                                      BY BLR 112
		be,		//  21. BELGIUM                                      BE BEL 056
		bz,		//  22. BELIZE                                       BZ BLZ 084
		bj,		//  23. BENIN                                        BJ BEN 204
		bm,		//  24. BERMUDA                                      BM BMU 060
		bt,		//  25. BHUTAN                                       BT BTN 064
		bo,		//  26. BOLIVIA                                      BO BOL 068
		ba,		//  27. BOSNIA AND HERZEGOWINA                       BA BIH 070
		bw,		//  28. BOTSWANA                                     BW BWA 072
		bv,		//  29. BOUVET ISLAND                                BV BVT 074
		br,		//  30. BRAZIL                                       BR BRA 076
		io,		//  31. BRITISH INDIAN OCEAN TERRITORY               IO IOT 086
		bn,		//  32. BRUNEI DARUSSALAM                            BN BRN 096
		bg,		//  33. BULGARIA                                     BG BGR 100
		bf,		//  34. BURKINA FASO                                 BF BFA 854
		bi,		//  35. BURUNDI                                      BI BDI 108
		kh,		//  36. CAMBODIA                                     KH KHM 116
		cm,		//  37. CAMEROON                                     CM CMR 120
		ca,		//  38. CANADA                                       CA CAN 124
		cv,		//  39. CAPE VERDE                                   CV CPV 132
		ky,		//  40. CAYMAN ISLANDS                               KY CYM 136
		cf,		//  41. CENTRAL AFRICAN REPUBLIC                     CF CAF 140
		td,		//  42. CHAD                                         TD TCD 148
		cl,		//  43. CHILE                                        CL CHL 152
		cn,		//  44. CHINA                                        CN CHN 156
		cx,		//  45. CHRISTMAS ISLAND                             CX CXR 162
		cc,		//  46. COCOS (KEELING) ISLANDS                      CC CCK 166
		co,		//  47. COLOMBIA                                     CO COL 170
		km,		//  48. COMOROS                                      KM COM 174
		cd,		//  49. CONGO, Democratic Republic of (was Zaire)    CD COD 180
		cg,		//  50. CONGO, People's Republic of                  CG COG 178
		ck,		//  51. COOK ISLANDS                                 CK COK 184
		cr,		//  52. COSTA RICA                                   CR CRI 188
		ci,		//  53. COTE D'IVOIRE                                CI CIV 384
		hr,		//  54. CROATIA (local name: Hrvatska)               HR HRV 191
		cu,		//  55. CUBA                                         CU CUB 192
		cy,		//  56. CYPRUS                                       CY CYP 196
		cz,		//  57. CZECH REPUBLIC                               CZ CZE 203
		dk,		//  58. DENMARK                                      DK DNK 208
		dj,		//  59. DJIBOUTI                                     DJ DJI 262
		dm,		//  60. DOMINICA                                     DM DMA 212
		DO,		//  61. DOMINICAN REPUBLIC                           DO DOM 214
		tl,		//  62. EAST TIMOR                                   TL TLS 626
		ec,		//  63. ECUADOR                                      EC ECU 218
		eg,		//  64. EGYPT                                        EG EGY 818
		sv,		//  65. EL SALVADOR                                  SV SLV 222
		gq,		//  66. EQUATORIAL GUINEA                            GQ GNQ 226
		er,		//  67. ERITREA                                      ER ERI 232
		ee,		//  68. ESTONIA                                      EE EST 233
		et,		//  69. ETHIOPIA                                     ET ETH 231
		fk,		//  70. FALKLAND ISLANDS (MALVINAS)                  FK FLK 238
		fo,		//  71. FAROE ISLANDS                                FO FRO 234
		fj,		//  72. FIJI                                         FJ FJI 242
		fi,		//  73. FINLAND                                      FI FIN 246
		fr,		//  74. FRANCE                                       FR FRA 250
		fx,		//  75. FRANCE, METROPOLITAN                         FX FXX 249
		gf,		//  76. FRENCH GUIANA                                GF GUF 254
		pf,		//  77. FRENCH POLYNESIA                             PF PYF 258
		tf,		//  78. FRENCH SOUTHERN TERRITORIES                  TF ATF 260
		ga,		//  79. GABON                                        GA GAB 266
		gm,		//  80. GAMBIA                                       GM GMB 270
		ge,		//  81. GEORGIA                                      GE GEO 268
		de,		//  82. GERMANY                                      DE DEU 276
		gh,		//  83. GHANA                                        GH GHA 288
		gi,		//  84. GIBRALTAR                                    GI GIB 292
		gr,		//  85. GREECE                                       GR GRC 300
		gl,		//  86. GREENLAND                                    GL GRL 304
		gd,		//  87. GRENADA                                      GD GRD 308
		gp,		//  88. GUADELOUPE                                   GP GLP 312
		gu,		//  89. GUAM                                         GU GUM 316
		gt,		//  90. GUATEMALA                                    GT GTM 320
		gn,		//  91. GUINEA                                       GN GIN 324
		gw,		//  92. GUINEA-BISSAU                                GW GNB 624
		gy,		//  93. GUYANA                                       GY GUY 328
		ht,		//  94. HAITI                                        HT HTI 332
		hm,		//  95. HEARD AND MC DONALD ISLANDS                  HM HMD 334
		hn,		//  96. HONDURAS                                     HN HND 340
		hk,		//  97. HONG KONG                                    HK HKG 344
		hu,		//  98. HUNGARY                                      HU HUN 348
		is,		//  99. ICELAND                                      IS ISL 352
		in,		// 100. INDIA                                        IN IND 356
		id,		// 101. INDONESIA                                    ID IDN 360
		ir,		// 102. IRAN (ISLAMIC REPUBLIC OF)                   IR IRN 364
		iq,		// 103. IRAQ                                         IQ IRQ 368
		ie,		// 104. IRELAND                                      IE IRL 372
		il,		// 105. ISRAEL                                       IL ISR 376
		it,		// 106. ITALY                                        IT ITA 380
		jm,		// 107. JAMAICA                                      JM JAM 388
		jp,		// 108. JAPAN                                        JP JPN 392
		jo,		// 109. JORDAN                                       JO JOR 400
		kz,		// 110. KAZAKHSTAN                                   KZ KAZ 398
		ke,		// 111. KENYA                                        KE KEN 404
		ki,		// 112. KIRIBATI                                     KI KIR 296
		kp,		// 113. KOREA, DEMOCRATIC PEOPLE'S REPUBLIC OF       KP PRK 408
		kr,		// 114. KOREA, REPUBLIC OF                           KR KOR 410
		kw,		// 115. KUWAIT                                       KW KWT 414
		kg,		// 116. KYRGYZSTAN                                   KG KGZ 417
		la,		// 117. LAO PEOPLE'S DEMOCRATIC REPUBLIC             LA LAO 418
		lv,		// 118. LATVIA                                       LV LVA 428
		lb,		// 119. LEBANON                                      LB LBN 422
		ls,		// 120. LESOTHO                                      LS LSO 426
		lr,		// 121. LIBERIA                                      LR LBR 430
		ly,		// 122. LIBYAN ARAB JAMAHIRIYA                       LY LBY 434
		li,		// 123. LIECHTENSTEIN                                LI LIE 438
		lt,		// 124. LITHUANIA                                    LT LTU 440
		lu,		// 125. LUXEMBOURG                                   LU LUX 442
		mo,		// 126. MACAU                                        MO MAC 446
		mk,		// 127. MACEDONIA, THE FORMER YUGOSLAV REPUBLIC OF   MK MKD 807
		mg,		// 128. MADAGASCAR                                   MG MDG 450
		mw,		// 129. MALAWI                                       MW MWI 454
		my,		// 130. MALAYSIA                                     MY MYS 458
		mv,		// 131. MALDIVES                                     MV MDV 462
		ml,		// 132. MALI                                         ML MLI 466
		mt,		// 133. MALTA                                        MT MLT 470
		mh,		// 134. MARSHALL ISLANDS                             MH MHL 584
		mq,		// 135. MARTINIQUE                                   MQ MTQ 474
		mr,		// 136. MAURITANIA                                   MR MRT 478
		mu,		// 137. MAURITIUS                                    MU MUS 480
		yt,		// 138. MAYOTTE                                      YT MYT 175
		mx,		// 139. MEXICO                                       MX MEX 484
		fm,		// 140. MICRONESIA, FEDERATED STATES OF              FM FSM 583
		md,		// 141. MOLDOVA, REPUBLIC OF                         MD MDA 498
		mc,		// 142. MONACO                                       MC MCO 492
		mn,		// 143. MONGOLIA                                     MN MNG 496
		ms,		// 144. MONTSERRAT                                   MS MSR 500
		ma,		// 145. MOROCCO                                      MA MAR 504
		mz,		// 146. MOZAMBIQUE                                   MZ MOZ 508
		mm,		// 147. MYANMAR                                      MM MMR 104
		na,		// 148. NAMIBIA                                      NA NAM 516
		nr,		// 149. NAURU                                        NR NRU 520
		np,		// 150. NEPAL                                        NP NPL 524
		nl,		// 151. NETHERLANDS                                  NL NLD 528
		an,		// 152. NETHERLANDS ANTILLES                         AN ANT 530
		nc,		// 153. NEW CALEDONIA                                NC NCL 540
		nz,		// 154. NEW ZEALAND                                  NZ NZL 554
		ni,		// 155. NICARAGUA                                    NI NIC 558
		ne,		// 156. NIGER                                        NE NER 562
		ng,		// 157. NIGERIA                                      NG NGA 566
		nu,		// 158. NIUE                                         NU NIU 570
		nf,		// 159. norfolk ISLAND                               NF NFK 574
		mp,		// 160. NORTHERN MARIANA ISLANDS                     MP MNP 580
		no,		// 161. NORWAY                                       NO NOR 578
		om,		// 162. OMAN                                         OM OMN 512
		pk,		// 163. PAKISTAN                                     PK PAK 586
		pw,		// 164. PALAU                                        PW PLW 585
		ps,		// 165. PALESTINIAN TERRITORY, Occupied              PS PSE 275
		pa,		// 166. PANAMA                                       PA PAN 591
		pg,		// 167. PAPUA NEW GUINEA                             PG PNG 598
		py,		// 168. PARAGUAY                                     PY PRY 600
		pe,		// 169. PERU                                         PE PER 604
		ph,		// 170. PHILIPPINES                                  PH PHL 608
		pn,		// 171. PITCAIRN                                     PN PCN 612
		pl,		// 172. POLAND                                       PL POL 616
		pt,		// 173. PORTUGAL                                     PT PRT 620
		pr,		// 174. PUERTO RICO                                  PR PRI 630
		qa,		// 175. QATAR                                        QA QAT 634
		re,		// 176. REUNION                                      RE REU 638
		ro,		// 177. ROMANIA                                      RO ROU 642
		ru,		// 178. RUSSIAN FEDERATION                           RU RUS 643
		rw,		// 179. RWANDA                                       RW RWA 646
		kn,		// 180. SAINT KITTS AND NEVIS                        KN KNA 659
		lc,		// 181. SAINT LUCIA                                  LC LCA 662
		vc,		// 182. SAINT VINCENT AND THE GRENADINES             VC VCT 670
		ws,		// 183. SAMOA                                        WS WSM 882
		sm,		// 184. SAN MARINO                                   SM SMR 674
		st,		// 185. SAO TOME AND PRINCIPE                        ST STP 678
		sa,		// 186. SAUDI ARABIA                                 SA SAU 682
		sn,		// 187. SENEGAL                                      SN SEN 686
		sc,		// 188. SEYCHELLES                                   SC SYC 690
		sl,		// 189. SIERRA LEONE                                 SL SLE 694
		sg,		// 190. SINGAPORE                                    SG SGP 702
		sk,		// 191. SLOVAKIA (Slovak Republic)                   SK SVK 703
		si,		// 192. SLOVENIA                                     SI SVN 705
		sb,		// 193. SOLOMON ISLANDS                              SB SLB 090
		so,		// 194. SOMALIA                                      SO SOM 706
		za,		// 195. SOUTH AFRICA                                 ZA ZAF 710
		gs,		// 196. SOUTH GEORGIA AND THE SOUTH SANDWICH ISLANDS GS SGS 239
		es,		// 197. SPAIN                                        ES ESP 724
		lk,		// 198. SRI LANKA                                    LK LKA 144
		sh,		// 199. ST. HELENA                                   SH SHN 654
		pm,		// 200. ST. PIERRE AND MIQUELON                      PM SPM 666
		sd,		// 201. SUDAN                                        SD SDN 736
		sr,		// 202. SURINAME                                     SR SUR 740
		sj,		// 203. SVALBARD AND JAN MAYEN ISLANDS               SJ SJM 744
		sz,		// 204. SWAZILAND                                    SZ SWZ 748
		se,		// 205. SWEDEN                                       SE SWE 752
		ch,		// 206. SWITZERLAND                                  CH CHE 756
		sy,		// 207. SYRIAN ARAB REPUBLIC                         SY SYR 760
		tw,		// 208. TAIWAN                                       TW TWN 158
		tj,		// 209. TAJIKISTAN                                   TJ TJK 762
		tz,		// 210. TANZANIA, UNITED REPUBLIC OF                 TZ TZA 834
		th,		// 211. THAILAND                                     TH THA 764
		tg,		// 212. TOGO                                         TG TGO 768
		tk,		// 213. TOKELAU                                      TK TKL 772
		to,		// 214. TONGA                                        TO TON 776
		tt,		// 215. TRINIDAD AND TOBAGO                          TT TTO 780
		tn,		// 216. TUNISIA                                      TN TUN 788
		tr,		// 217. TURKEY                                       TR TUR 792
		tm,		// 218. TURKMENISTAN                                 TM TKM 795
		tc,		// 219. TURKS AND CAICOS ISLANDS                     TC TCA 796
		tv,		// 220. TUVALU                                       TV TUV 798
		ug,		// 221. UGANDA                                       UG UGA 800
		ua,		// 222. UKRAINE                                      UA UKR 804
		ae,		// 223. UNITED ARAB EMIRATES                         AE ARE 784
		gb,		// 224. UNITED KINGDOM                               GB GBR 826
		us,		// 225. UNITED STATES                                US USA 840
		um,		// 226. UNITED STATES MINOR OUTLYING ISLANDS         UM UMI 581
		uy,		// 227. URUGUAY                                      UY URY 858
		uz,		// 228. UZBEKISTAN                                   UZ UZB 860
		vu,		// 229. VANUATU                                      VU VUT 548
		va,		// 230. VATICAN CITY STATE (HOLY SEE)                VA VAT 336
		ve,		// 231. VENEZUELA                                    VE VEN 862
		vn,		// 232. VIET NAM                                     VN VNM 704
		vg,		// 233. VIRGIN ISLANDS (BRITISH)                     VG VGB 092
		vi,		// 234. VIRGIN ISLANDS (U.S.)                        VI VIR 850
		wf,		// 235. WALLIS AND FUTUNA ISLANDS                    WF WLF 876
		eh,		// 236. WESTERN SAHARA                               EH ESH 732
		ye,		// 237. YEMEN                                        YE YEM 887
		yu,		// 238. YUGOSLAVIA                                   YU YUG 891
		zm,		// 239. ZAMBIA                                       ZM ZMB 894
		zw,		// 240. ZIMBABWE                                     ZW ZWE 716
		last	// end of this enum
	};

	// コードからシンボルを取得します
	ModCommonDLL
	static const ModUnicodeChar* toSymbol(iso3166	code_);

	// シンボルからコードを取得します
	ModCommonDLL
	static iso3166 toCode(const ModUnicodeChar*	symbol_,
						  ModSize				len_ = 0);

	// コードが範囲内であるかどうかを調べます
	ModCommonDLL
	static ModBoolean isValid(iso3166	code_);
};

//	TYPEDEF
//	ModCountryCode -- 国・地域コードを表す型
//
//	NOTES

typedef ModCountry::iso3166	ModCountryCode;

#endif	// __ModCountry_H__

//
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
