/*
 * EUCキリル文字とその翻字形との変換表
 *
 * Copyright (c) 2023 Ricoh Company, Ltd.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

struct _ambRussianTransTbl {
	unsigned short		russiancode;
	unsigned char		cnvdata[9];
}ambRussianTransTbl[] =
{
	{ 0xa7a1,  "Ａ"  },   /* А */
	{ 0xa7a2,  "Ｂ"  },   /* Б */
	{ 0xa7a3,  "Ｖ"  },   /* В */
	{ 0xa7a4,  "Ｇ"  },   /* Г */
	{ 0xa7a5,  "Ｄ"  },   /* Д */
	{ 0xa7a6,  "Ｅ"  },   /* Е */
	{ 0xa7a7,  "Ｅ"  },   /* Ё */
	{ 0xa7a8,  "Ｚｈ" },   /* Ж */
	{ 0xa7a9,  "Ｚ"  },   /* З */
	{ 0xa7aa,  "Ｉ"  },   /* И */
	{ 0xa7ab,  "Ｉ"  },   /* Й */
	{ 0xa7ac,  "Ｋ"  },   /* К */
	{ 0xa7ad,  "Ｌ"  },   /* Л */
	{ 0xa7ae,  "Ｍ"  },   /* М */
	{ 0xa7af,  "Ｎ"  },   /* Н */
	{ 0xa7b0,  "Ｏ"  },   /* О */
	{ 0xa7b1,  "Ｐ"  },   /* П */
	{ 0xa7b2,  "Ｒ"  },   /* Р */
	{ 0xa7b3,  "Ｓ"  },   /* С */
	{ 0xa7b4,  "Ｔ"  },   /* Т */
	{ 0xa7b5,  "Ｕ"  },   /* У */
	{ 0xa7b6,  "Ｆ"  },   /* Ф */
	{ 0xa7b7,  "Ｋｈ" },   /* Х */
	{ 0xa7b8,  "ＴＳ" },   /* Ц */
	{ 0xa7b9,  "Ｃｈ" },   /* Ч */
	{ 0xa7ba,  "Ｓｈ" },   /* Ш */
	{ 0xa7bb,  "Ｓｈｃｈ" }, /* Щ */
	{ 0xa7bc,  "″" },   /* Ъ */
	{ 0xa7bd,  "Ｙ"  },   /* Ы */
	{ 0xa7be,  "′" },   /* Ь */
	{ 0xa7bf,  "Ｅ"  },   /* Э */
	{ 0xa7c0,  "ＩＵ" },   /* Ю */
	{ 0xa7c1,  "ＩＡ" },   /* Я */
	{ 0xa7d1,  "ａ"  },   /* а */
	{ 0xa7d2,  "ｂ"  },   /* б */
	{ 0xa7d3,  "ｖ"  },   /* в */
	{ 0xa7d4,  "ｇ"  },   /* г */
	{ 0xa7d5,  "ｄ"  },   /* д */
	{ 0xa7d6,  "ｅ"  },   /* е */
	{ 0xa7d7,  "ｅ"  },   /* ё */
	{ 0xa7d8,  "ｚｈ" },   /* ж */
	{ 0xa7d9,  "ｚ"  },   /* з */
	{ 0xa7da,  "ｉ"  },   /* и */
	{ 0xa7db,  "ｉ"  },   /* й */
	{ 0xa7dc,  "ｋ"  },   /* к */
	{ 0xa7dd,  "ｌ"  },   /* л */
	{ 0xa7de,  "ｍ"  },   /* м */
	{ 0xa7df,  "ｎ"  },   /* н */
	{ 0xa7e0,  "ｏ"  },   /* о */
	{ 0xa7e1,  "ｐ"  },   /* п */
	{ 0xa7e2,  "ｒ"  },   /* р */
	{ 0xa7e3,  "ｓ"  },   /* с */
	{ 0xa7e4,  "ｔ"  },   /* т */
	{ 0xa7e5,  "ｕ"  },   /* у */
	{ 0xa7e6,  "ｆ"  },   /* ф */
	{ 0xa7e7,  "ｋｈ" },   /* х */
	{ 0xa7e8,  "ｔｓ" },   /* ц */
	{ 0xa7e9,  "ｃｈ" },   /* ч */
	{ 0xa7ea,  "ｓｈ" },   /* ш */
	{ 0xa7eb,  "ｓｈｃｈ" }, /* щ */
	{ 0xa7ec,  "″" },   /* ъ */
	{ 0xa7ed,  "ｙ"  },   /* ы */
	{ 0xa7ee,  "′" },   /* ь */
	{ 0xa7ef,  "ｅ"  },   /* э */
	{ 0xa7f0,  "ｉｕ" },   /* ю */
	{ 0xa7f1,  "ｉａ" },   /* я */
};
