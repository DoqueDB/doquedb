/*
 * EUC化NVTのEXC文字とその翻字形との変換表
 * 
 * Copyright (c) 1998, 2023 Ricoh Company, Ltd.
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
struct _ambExcTransTbl {
	unsigned short		exccode;
	unsigned char		cnvdata[3];
}ambExcTransTbl[] =
{
	{ 0xf6a1,  "L"  },   /* polish-L */
	{ 0xf6a2,  "O"  },   /* scandinavian-O */
	{ 0xf6a3,  "D"  },   /* D-with-crossbar */
	{ 0xf6a4,  "TH" },   /* icelandic-thorn */
	{ 0xf6a5,  "AE" },   /* digraph-AE */
	{ 0xf6a6,  "OE" },   /* digraph-OE */
	{ 0xf6a8,  "O"  },   /* O-hook */
	{ 0xf6a9,  "U"  },   /* U-hook */
	{ 0xf6ac,  "l"  },   /* polish-l */
	{ 0xf6ad,  "o"  },   /* scandinavian-o */
	{ 0xf6ae,  "d"  },   /* d-with-crossbar */
	{ 0xf6af,  "th" },   /* icelandic-thorn */
	{ 0xf6b0,  "ae" },   /* digraph-ae */
	{ 0xf6b1,  "oe" },   /* digraph-oe */
	{ 0xf6b3,  "i"  },   /* turkish-i */
	{ 0xf6b4,  "d"  },   /* eth */
	{ 0xf6b5,  "o"  },   /* o-hook */
	{ 0xf6b6,  "u"  },   /* u-hook */
	{ 0xf6b7,  "ss" },   /* sz */
	{ 0xf7a1,  "A"  },   /* pseudo-question A */
	{ 0xf7a2,  "E"  },   /* pseudo-question E */
	{ 0xf7a3,  "I"  },   /* pseudo-question I */
	{ 0xf7a4,  "O"  },   /* pseudo-question O */
	{ 0xf7a5,  "U"  },   /* pseudo-question U */
	{ 0xf7a6,  "Y"  },   /* pseudo-question Y */
	{ 0xf7a7,  "a"  },   /* pseudo-question a */
	{ 0xf7a8,  "e"  },   /* pseudo-question e */
	{ 0xf7a9,  "i"  },   /* pseudo-question i */
	{ 0xf7aa,  "o"  },   /* pseudo-question o */
	{ 0xf7ab,  "u"  },   /* pseudo-question u */
	{ 0xf7ac,  "y"  },   /* pseudo-question y */ 
        { 0xf7ad,  "O"  },   /* pseudo-question O */   /* new */
	{ 0xf7ae,  "U"  },   /* pseudo-question U */   /* new */
	{ 0xf7af,  "o"  },   /* pseudo-question o */   /* new */
	{ 0xf7b0,  "u"  },   /* pseudo-question u */   /* new */
	{ 0xf7b1,  "a"  },   /* Circumflex a  */   /* new */
	{ 0xf7b2,  "E"  },   /*  E */   /* new */
	{ 0xf7b3,  "O"  },   /*  O */   /* new */
	{ 0xf7b4,  "a"  },   /*  a */   /* new */
	{ 0xf7b5,  "e"  },   /*  e */   /* new */
	{ 0xf7b6,  "o"  },   /*  o */   /* new */
	{ 0xf7b7,  "A"  },   /*  A */   /* new */
	{ 0xf7b8,  "a"  },   /*  a */   /* new */
	{ 0xf7b9,  "A"  },   /* grave A */    
	{ 0xf7ba,  "E"  },   /* grave E */
	{ 0xf7bb,  "I"  },   /* grave I */
	{ 0xf7bc,  "N"  },   /* grave N */
	{ 0xf7bd,  "O"  },   /* grave O */
	{ 0xf7be,  "S"  },   /* grave S */
	{ 0xf7bf,  "U"  },   /* grave U */
	{ 0xf7c0,  "Y"  },   /* grave Y */
	{ 0xf7c1,  "a"  },   /* grave a */
	{ 0xf7c2,  "e"  },   /* grave e */
	{ 0xf7c3,  "i"  },   /* grave i */
	{ 0xf7c4,  "n"  },   /* grave n */
	{ 0xf7c5,  "o"  },   /* grave o */
	{ 0xf7c6,  "s"  },   /* grave s */
	{ 0xf7c7,  "u"  },   /* grave u */
	{ 0xf7c8,  "y"  },   /* grave y */
	{ 0xf7d5,  "A"  },   /* acute A */
	{ 0xf7d6,  "C"  },   /* acute C */
	{ 0xf7d7,  "D"  },   /* acute D */
	{ 0xf7d8,  "E"  },   /* acute E */
	{ 0xf7d9,  "G"  },   /* acute G */
	{ 0xf7da,  "H"  },   /* acute H */
	{ 0xf7db,  "I"  },   /* acute I */
	{ 0xf7dc,  "K"  },   /* acute K */
	{ 0xf7dd,  "L"  },   /* acute L */
	{ 0xf7de,  "M"  },   /* acute M */
	{ 0xf7df,  "N"  },   /* acute N */
	{ 0xf7e0,  "O"  },   /* acute O */
	{ 0xf7e1,  "P"  },   /* acute P */
	{ 0xf7e2,  "Q"  },   /* acute Q */
	{ 0xf7e3,  "R"  },   /* acute R */
	{ 0xf7e4,  "S"  },   /* acute S */
	{ 0xf7e5,  "T"  },   /* acute T */
	{ 0xf7e6,  "U"  },   /* acute U */
	{ 0xf7e7,  "V"  },   /* acute V */
	{ 0xf7e8,  "W"  },   /* acute W */
	{ 0xf7e9,  "Y"  },   /* acute Y */
	{ 0xf7ea,  "Z"  },   /* acute Z */
	{ 0xf7eb,  "a"  },   /* acute a */
	{ 0xf7ec,  "c"  },   /* acute c */
	{ 0xf7ed,  "d"  },   /* acute d */
	{ 0xf7ee,  "e"  },   /* acute e */
	{ 0xf7ef,  "g"  },   /* acute g */
	{ 0xf7f0,  "h"  },   /* acute h */
	{ 0xf7f1,  "i"  },   /* acute i */
	{ 0xf7f2,  "k"  },   /* acute k */
	{ 0xf7f3,  "l"  },   /* acute l */
	{ 0xf7f4,  "m"  },   /* acute m */
	{ 0xf7f5,  "n"  },   /* acute n */
	{ 0xf7f6,  "o"  },   /* acute o */
	{ 0xf7f7,  "p"  },   /* acute p */
	{ 0xf7f8,  "q"  },   /* acute q */
	{ 0xf7f9,  "r"  },   /* acute r */
	{ 0xf7fa,  "s"  },   /* acute s */
	{ 0xf7fb,  "t"  },   /* acute t */
	{ 0xf7fc,  "u"  },   /* acute u */
	{ 0xf7fd,  "v"  },   /* acute v */
	{ 0xf7fe,  "w"  },   /* acute w */
	{ 0xf8a1,  "y"  },   /* acute y */
	{ 0xf8a2,  "z"  },   /* acute z */
	{ 0xf8a3,  "O"  },   /* O */    /* new */
	{ 0xf8a4,  "U"  },   /* U */    /* new */      
	{ 0xf8a5,  "o"  },   /* o */    /* new */
	{ 0xf8a6,  "u"  },   /* u */    /* new */
	{ 0xf8a7,  "A"  },   /* A */    /* new */
	{ 0xf8a8,  "E"  },   /* E */    /* new */
	{ 0xf8a9,  "O"  },   /* O */    /* new */
	{ 0xf8aa,  "a"  },   /* a */    /* new */
	{ 0xf8ab,  "e"  },   /* e */    /* new */
	{ 0xf8ac,  "o"  },   /* o */    /* new */
	{ 0xf8ad,  "A"  },   /* A */    /* new */
	{ 0xf8ae,  "a"  },   /* a */    /* new */
	{ 0xf8af,  "A"  },   /* A */    /* new */
	{ 0xf8b0,  "E"  },   /* E */    /* new */
	{ 0xf8b1,  "I"  },   /* I */    /* new */
	{ 0xf8b2,  "O"  },   /* O */    /* new */
	{ 0xf8b3,  "a"  },   /* a */    /* new */
	{ 0xf8b4,  "e"  },   /* e */    /* new */
	{ 0xf8b5,  "i"  },   /* i */    /* new */
	{ 0xf8b6,  "o"  },   /* o */    /* new */
	{ 0xf8b7,  "A"  },   /* circumflex ^A */
	{ 0xf8b8,  "C"  },   /* circumflex ^C */
	{ 0xf8b9,  "E"  },   /* circumflex ^E */
	{ 0xf8ba,  "G"  },   /* circumflex ^G */
	{ 0xf8bb,  "H"  },   /* circumflex ^H */
	{ 0xf8bc,  "I"  },   /* circumflex ^I */
	{ 0xf8bd,  "J"  },   /* circumflex ^J */
	{ 0xf8be,  "N"  },   /* circumflex ^N */
	{ 0xf8bf,  "O"  },   /* circumflex ^O */
	{ 0xf8c0,  "S"  },   /* circumflex ^S */
	{ 0xf8c1,  "U"  },   /* circumflex ^U */
	{ 0xf8c2,  "W"  },   /* circumflex ^W */
	{ 0xf8c3,  "Y"  },   /* circumflex ^Y */
	{ 0xf8c4,  "a"  },   /* circumflex ^a */
	{ 0xf8c5,  "c"  },   /* circumflex ^c */
	{ 0xf8c6,  "e"  },   /* circumflex ^e */
	{ 0xf8c7,  "g"  },   /* circumflex ^g */
	{ 0xf8c8,  "h"  },   /* circumflex ^h */
	{ 0xf8c9,  "i"  },   /* circumflex ^i */
	{ 0xf8ca,  "j"  },   /* circumflex ^j */
	{ 0xf8cb,  "n"  },   /* circumflex ^n */
	{ 0xf8cc,  "o"  },   /* circumflex ^o */
	{ 0xf8cd,  "s"  },   /* circumflex ^s */
	{ 0xf8ce,  "u"  },   /* circumflex ^u */
	{ 0xf8cf,  "w"  },   /* circumflex ^w */
	{ 0xf8d0,  "y"  },   /* circumflex ^y */
	{ 0xf8d1,  "O"  },   /* O */    /* new */
	{ 0xf8d2,  "U"  },   /* U */    /* new */
	{ 0xf8d3,  "l"  },   /* l */    /* new */
	{ 0xf8d4,  "o"  },   /* o */    /* new */
	{ 0xf8d5,  "u"  },   /* u */    /* new */
	{ 0xf8d6,  "A"  },   /* A */    /* new */
	{ 0xf8d7,  "E"  },   /* E */    /* new */
	{ 0xf8d8,  "O"  },   /* O */    /* new */
	{ 0xf8d9,  "a"  },   /* a */    /* new */
	{ 0xf8da,  "c"  },   /* c */    /* new */
	{ 0xf8db,  "o"  },   /* o */    /* new */
	{ 0xf8dc,  "A"  },   /* A */    /* new */
	{ 0xf8dd,  "E"  },   /* E */    /* new */
	{ 0xf8de,  "O"  },   /* O */    /* new */
	{ 0xf8df,  "a"  },   /* a */    /* new */
	{ 0xf8e0,  "e"  },   /* tilde ~c---->e修正 */
	{ 0xf8e1,  "o"  },   /* o */    /* new */
	{ 0xf8e2,  "A"  },   /* tilde ~A */
	{ 0xf8e3,  "C"  },   /* tilde ~C */
	{ 0xf8e4,  "E"  },   /* tilde ~E */
	{ 0xf8e5,  "I"  },   /* tilde ~I */
	{ 0xf8e6,  "N"  },   /* tilde ~N */
	{ 0xf8e7,  "O"  },   /* tilde ~O */
	{ 0xf8e8,  "Q"  },   /* tilde ~Q */
	{ 0xf8e9,  "R"  },   /* tilde ~R */
	{ 0xf8ea,  "S"  },   /* tilde ~S */
	{ 0xf8eb,  "U"  },   /* tilde ~U */
	{ 0xf8ec,  "W"  },   /* tilde ~W */
	{ 0xf8ed,  "Y"  },   /* tilde ~Y */
	{ 0xf8ee,  "Z"  },   /* tilde ~Z */
	{ 0xf8ef,  "a"  },   /* tilde ~a */
	{ 0xf8f0,  "c"  },   /* c */    /* new */
	{ 0xf8f1,  "e"  },   /* tilde ~e */
	{ 0xf8f2,  "i"  },   /* tilde ~i */
	{ 0xf8f3,  "n"  },   /* tilde ~n */
	{ 0xf8f4,  "o"  },   /* tilde ~o */
	{ 0xf8f5,  "q"  },   /* tilde ~q */
	{ 0xf8f6,  "r"  },   /* tilde ~r */
	{ 0xf8f7,  "s"  },   /* tilde ~s */
	{ 0xf8f8,  "u"  },   /* tilde ~u */
	{ 0xf8f9,  "w"  },   /* tilde ~w */
	{ 0xf8fa,  "y"  },   /* tilde ~y */
	{ 0xf8fb,  "z"  },   /* tilde ~z */
	{ 0xf8fc,  "O"  },   /* O */    /* new */
	{ 0xf8fd,  "U"  },   /* U */    /* new */
	{ 0xf8fe,  "o"  },   /* o */    /* new */
        { 0xf9a1,  "u"  },   /* u */    /* new */
	{ 0xf9a2,  "A"  },   /* A */    /* new */
	{ 0xf9a3,  "I"  },   /* I */    /* new */
	{ 0xf9a4,  "U"  },   /* U */    /* new */
	{ 0xf9a5,  "a"  },   /* a */    /* new */
	{ 0xf9a6,  "i"  },   /* i */    /* new */
	{ 0xf9a7,  "u"  },   /* u */    /* new */
	{ 0xf9a8,  "A"  },   /* macron A */
	{ 0xf9a9,  "C"  },   /* macron C */
	{ 0xf9aa,  "D"  },   /* macron D */
	{ 0xf9ab,  "E"  },   /* macron E */
	{ 0xf9ac,  "G"  },   /* macron G */
	{ 0xf9ad,  "I"  },   /* macron I */
	{ 0xf9ae,  "J"  },   /* macron J */
	{ 0xf9af,  "K"  },   /* macron K */
	{ 0xf9b0,  "L"  },   /* macron L */
	{ 0xf9b1,  "M"  },   /* macron M */
	{ 0xf9b2,  "N"  },   /* macron N */
	{ 0xf9b3,  "O"  },   /* macron O */
	{ 0xf9b4,  "P"  },   /* macron P */
	{ 0xf9b5,  "Q"  },   /* macron Q */
	{ 0xf9b6,  "R"  },   /* macron R */
	{ 0xf9b7,  "S"  },   /* macron S */
	{ 0xf9b8,  "T"  },   /* macron T */
	{ 0xf9b9,  "U"  },   /* macron U */
	{ 0xf9ba,  "Y"  },   /* macron Y */
	{ 0xf9bb,  "Z"  },   /* macron Z */
	{ 0xf9bc,  "a"  },   /* macron a */
	{ 0xf9bd,  "c"  },   /* macron c */
	{ 0xf9be,  "d"  },   /* macron d */
	{ 0xf9bf,  "e"  },   /* macron e */
	{ 0xf9c0,  "g"  },   /* macron g */
	{ 0xf9c1,  "i"  },   /* macron i */
	{ 0xf9c2,  "j"  },   /* macron j */
	{ 0xf9c3,  "k"  },   /* macron k */
	{ 0xf9c4,  "l"  },   /* macron l */
	{ 0xf9c5,  "m"  },   /* macron m */
	{ 0xf9c6,  "n"  },   /* macron n */
	{ 0xf9c7,  "o"  },   /* macron o */
	{ 0xf9c8,  "p"  },   /* macron p */
	{ 0xf9c9,  "q"  },   /* macron q */
	{ 0xf9ca,  "r"  },   /* macron r */
	{ 0xf9cb,  "s"  },   /* macron s */
	{ 0xf9cc,  "t"  },   /* macron t */
	{ 0xf9cd,  "u"  },   /* macron u */
	{ 0xf9ce,  "y"  },   /* macron y */
	{ 0xf9cf,  "z"  },   /* macron z */
	{ 0xf9d0,  "AE"  },   /* */    /* new */
	{ 0xf9d1,  "OE"  },   /* */    /* new */
	{ 0xf9d2,  "O"  },   /* */    /* new */
	{ 0xf9d3,  "U"  },   /* */    /* new */
	{ 0xf9d4,  "ae"  },   /* */    /* new */
	{ 0xf9d5,  "oe"  },   /* */    /* new */
	{ 0xf9d6,  "o"  },   /* */    /* new */
	{ 0xf9d7,  "u"  },   /* */    /* new */
	{ 0xf9d8,  "L"  },   /* */    /* new */
	{ 0xf9d9,  "R"  },   /* */    /* new */
	{ 0xf9da,  "l"  },   /* */    /* new */
	{ 0xf9db,  "r"  },   /* */    /* new */
	{ 0xf9dc,  "O"  },   /* */    /* new */
	{ 0xf9dd,  "o"  },   /* */    /* new */
	{ 0xf9de,  "A"  },   /* breve A */
	{ 0xf9df,  "C"  },   /* breve C */
	{ 0xf9e0,  "E"  },   /* breve E */
	{ 0xf9e1,  "G"  },   /* breve G */
	{ 0xf9e2,  "I"  },   /* breve I */
	{ 0xf9e3,  "M"  },   /* breve M */
	{ 0xf9e4,  "O"  },   /* breve O */
	{ 0xf9e5,  "U"  },   /* breve U */
	{ 0xf9e6,  "Y"  },   /* breve Y */
	{ 0xf9e7,  "Z"  },   /* breve Z */
	{ 0xf9e8,  "a"  },   /* breve a */
	{ 0xf9e9,  "c"  },   /* breve c */
	{ 0xf9ea,  "e"  },   /* breve e */
	{ 0xf9eb,  "g"  },   /* breve g */
	{ 0xf9ec,  "i"  },   /* breve i */
	{ 0xf9ed,  "m"  },   /* breve m */
	{ 0xf9ee,  "o"  },   /* breve o */
	{ 0xf9ef,  "u"  },   /* breve u */
	{ 0xf9f0,  "y"  },   /* breve y */
	{ 0xf9f1,  "z"  },   /* breve z */
        { 0xf9f2,  "O"  },   /*  */    /* new */
	{ 0xf9f3,  "U"  },   /*  */    /* new */
	{ 0xf9f4,  "o"  },   /*  */    /* new */
	{ 0xf9f5,  "u"  },   /*  */    /* new */
	{ 0xf9f6,  "A"  },   /*  */    /* new */
	{ 0xf9f7,  "a"  },   /*  */    /* new */
	{ 0xf9f8,  "A"  },   /* superior-dot A */
	{ 0xf9f9,  "B"  },   /* superior-dot B */
	{ 0xf9fa,  "C"  },   /* superior-dot C */
	{ 0xf9fb,  "D"  },   /* superior-dot D */
	{ 0xf9fc,  "E"  },   /* superior-dot E */
	{ 0xf9fd,  "F"  },   /* superior-dot F */
	{ 0xf9fe,  "G"  },   /* superior-dot G */
	{ 0xfaa1,  "H"  },   /* superior-dot H */
	{ 0xfaa2,  "I"  },   /* superior-dot I */
	{ 0xfaa3,  "K"  },   /* superior-dot K */
	{ 0xfaa4,  "L"  },   /* superior-dot L */
	{ 0xfaa5,  "M"  },   /* superior-dot M */
	{ 0xfaa6,  "N"  },   /* superior-dot N */
	{ 0xfaa7,  "O"  },   /* superior-dot O */
	{ 0xfaa8,  "P"  },   /* superior-dot P */
	{ 0xfaa9,  "R"  },   /* superior-dot R */
	{ 0xfaaa,  "S"  },   /* superior-dot S */
	{ 0xfaab,  "T"  },   /* superior-dot T */
	{ 0xfaac,  "U"  },   /* superior-dot U */
	{ 0xfaad,  "W"  },   /* superior-dot W */
	{ 0xfaae,  "Y"  },   /* superior-dot Y */
	{ 0xfaaf,  "Z"  },   /* superior-dot Z */
	{ 0xfab0,  "a"  },   /* superior-dot a */
	{ 0xfab1,  "b"  },   /* superior-dot b */
	{ 0xfab2,  "c"  },   /* superior-dot c */
	{ 0xfab3,  "d"  },   /* superior-dot d */
	{ 0xfab4,  "e"  },   /* superior-dot e */
	{ 0xfab5,  "f"  },   /* superior-dot f */
	{ 0xfab6,  "g"  },   /* superior-dot g */
	{ 0xfab7,  "h"  },   /* superior-dot h */
	{ 0xfab8,  "k"  },   /* superior-dot k */
	{ 0xfab9,  "l"  },   /* superior-dot l */
	{ 0xfaba,  "m"  },   /* superior-dot m */
	{ 0xfabb,  "n"  },   /* superior-dot n */
	{ 0xfabc,  "o"  },   /* superior-dot o */
	{ 0xfabd,  "p"  },   /* superior-dot p */
	{ 0xfabe,  "r"  },   /* superior-dot r */
	{ 0xfabf,  "s"  },   /* superior-dot s */
	{ 0xfac0,  "t"  },   /* superior-dot t */
	{ 0xfac1,  "u"  },   /* superior-dot u */
	{ 0xfac2,  "w"  },   /* superior-dot w */
	{ 0xfac3,  "y"  },   /* superior-dot y */
	{ 0xfac4,  "z"  },   /* superior-dot z */
	{ 0xfac5,  "A"  },   /* umlaut 'A' */
	{ 0xfac6,  "C"  },   /* umlaut 'C' */
	{ 0xfac7,  "E"  },   /* umlaut 'E' */
	{ 0xfac8,  "G"  },   /* umlaut 'G' */
	{ 0xfac9,  "I"  },   /* umlaut 'I' */
	{ 0xfaca,  "J"  },   /* umlaut 'J' */
	{ 0xfacb,  "O"  },   /* umlaut 'O' */
	{ 0xfacc,  "R"  },   /* umlaut 'R' */
	{ 0xfacd,  "S"  },   /* umlaut 'S' */
	{ 0xface,  "U"  },   /* umlaut 'U' */
	{ 0xfacf,  "V"  },   /* umlaut 'V' */
	{ 0xfad0,  "Y"  },   /* umlaut 'Y' */
	{ 0xfad1,  "Z"  },   /* umlaut 'Z' */
	{ 0xfad2,  "a"  },   /* umlaut 'a' */
	{ 0xfad3,  "c"  },   /* umlaut 'c' */
	{ 0xfad4,  "e"  },   /* umlaut 'e' */
	{ 0xfad5,  "g"  },   /* umlaut 'g' */
	{ 0xfad6,  "i"  },   /* umlaut 'i' */
	{ 0xfad7,  "j"  },   /* umlaut 'j' */
	{ 0xfad8,  "o"  },   /* umlaut 'o' */
	{ 0xfad9,  "r"  },   /* umlaut 'r' */
	{ 0xfada,  "s"  },   /* umlaut 's' */
	{ 0xfadb,  "u"  },   /* umlaut 'u' */
	{ 0xfadc,  "v"  },   /* umlaut 'v' */
	{ 0xfadd,  "y"  },   /* umlaut 'y' */
	{ 0xfade,  "z"  },   /* umlaut 'z' */
	{ 0xfadf,  "A"  },   /* hacek A */
	{ 0xfae0,  "C"  },   /* hacek C */
	{ 0xfae1,  "D"  },   /* hacek D */
	{ 0xfae2,  "E"  },   /* hacek E */
	{ 0xfae3,  "G"  },   /* hacek G */
	{ 0xfae4,  "I"  },   /* hacek I */
	{ 0xfae5,  "J"  },   /* hacek J */
	{ 0xfae6,  "L"  },   /* hacek L */
	{ 0xfae7,  "N"  },   /* hacek N */
	{ 0xfae8,  "O"  },   /* hacek O */
	{ 0xfae9,  "R"  },   /* hacek R */
	{ 0xfaea,  "S"  },   /* hacek S */
	{ 0xfaeb,  "T"  },   /* hacek T */
	{ 0xfaec,  "U"  },   /* hacek U */
	{ 0xfaed,  "Z"  },   /* hacek Z */
	{ 0xfaee,  "a"  },   /* hacek a */
	{ 0xfaef,  "c"  },   /* hacek c */
	{ 0xfaf0,  "d"  },   /* hacek d */
	{ 0xfaf1,  "e"  },   /* hacek e */
	{ 0xfaf2,  "g"  },   /* hacek g */
	{ 0xfaf3,  "i"  },   /* hacek i */
	{ 0xfaf4,  "j"  },   /* hacek j */
	{ 0xfaf5,  "l"  },   /* hacek l */
	{ 0xfaf6,  "n"  },   /* hacek n */
	{ 0xfaf7,  "o"  },   /* hacek o */
	{ 0xfaf8,  "r"  },   /* hacek r */
	{ 0xfaf9,  "s"  },   /* hacek s */
	{ 0xfafa,  "t"  },   /* hacek t */
	{ 0xfafb,  "u"  },   /* hacek u */
	{ 0xfafc,  "z"  },   /* hacek z */
	{ 0xfafd,  "A"  },   /* angstrom A */
	{ 0xfafe,  "E"  },   /* angstrom E */
	{ 0xfba1,  "G"  },   /* angstrom G */
	{ 0xfba2,  "K"  },   /* angstrom K */
	{ 0xfba3,  "R"  },   /* angstrom R */
	{ 0xfba4,  "S"  },   /* angstrom S */
	{ 0xfba5,  "U"  },   /* angstrom U */
	{ 0xfba6,  "Y"  },   /* angstrom Y */
	{ 0xfba7,  "a"  },   /* angstrom a */
	{ 0xfba8,  "e"  },   /* angstrom e */
	{ 0xfba9,  "g"  },   /* angstrom g */
	{ 0xfbaa,  "k"  },   /* angstrom k */
	{ 0xfbab,  "r"  },   /* angstrom r */
	{ 0xfbac,  "s"  },   /* angstrom s */
	{ 0xfbad,  "u"  },   /* angstrom u */
	{ 0xfbae,  "y"  },   /* angstrom y */
	{ 0xfbaf,  "D"  },   /* ligature-first D */
	{ 0xfbb0,  "G"  },   /* ligature-first G */
	{ 0xfbb1,  "I"  },   /* ligature-first I */
	{ 0xfbb2,  "J"  },   /* ligature-first J */
	{ 0xfbb3,  "K"  },   /* ligature-first K */
	{ 0xfbb4,  "N"  },   /* ligature-first N */
	{ 0xfbb5,  "O"  },   /* ligature-first O */
	{ 0xfbb6,  "P"  },   /* ligature-first P */
	{ 0xfbb7,  "T"  },   /* ligature-first T */
	{ 0xfbb8,  "Z"  },   /* ligature-first Z */
	{ 0xfbb9,  "d"  },   /* ligature-first d */
	{ 0xfbba,  "g"  },   /* ligature-first g */
	{ 0xfbbb,  "i"  },   /* ligature-first i */
	{ 0xfbbc,  "j"  },   /* ligature-first j */
	{ 0xfbbd,  "k"  },   /* ligature-first k */
	{ 0xfbbe,  "n"  },   /* ligature-first n */
	{ 0xfbbf,  "o"  },   /* ligature-first o */
	{ 0xfbc0,  "p"  },   /* ligature-first p */
	{ 0xfbc1,  "t"  },   /* ligature-first t */
	{ 0xfbc2,  "z"  },   /* ligature-first z */
	{ 0xfbc3,  "A"  },   /* ligature-sond A */
	{ 0xfbc4,  "E"  },   /* ligature-sond E */
	{ 0xfbc5,  "G"  },   /* ligature-sond G */
	{ 0xfbc6,  "H"  },   /* ligature-sond H */
	{ 0xfbc7,  "N"  },   /* ligature-sond N */
	{ 0xfbc8,  "O"  },   /* ligature-sond O */
	{ 0xfbc9,  "S"  },   /* ligature-sond S */
	{ 0xfbca,  "T"  },   /* ligature-sond T */
	{ 0xfbcb,  "U"  },   /* ligature-sond U */
	{ 0xfbcc,  "Z"  },   /* ligature-sond Z */
	{ 0xfbcd,  "a"  },   /* ligature-sond a */
	{ 0xfbce,  "e"  },   /* ligature-sond e */
	{ 0xfbcf,  "g"  },   /* ligature-sond g */
	{ 0xfbd0,  "h"  },   /* ligature-sond h */
	{ 0xfbd1,  "n"  },   /* ligature-sond n */
	{ 0xfbd2,  "o"  },   /* ligature-sond o */
	{ 0xfbd3,  "s"  },   /* ligature-sond s */
	{ 0xfbd4,  "t"  },   /* ligature-sond t */
	{ 0xfbd5,  "u"  },   /* ligature-sond u */
	{ 0xfbd6,  "z"  },   /* ligature-sond z */
	{ 0xfbd7,  "D"  },   /* high-comma-off-center D */
	{ 0xfbd8,  "G"  },   /* high-comma-off-center G */
	{ 0xfbd9,  "K"  },   /* high-comma-off-center K */
	{ 0xfbda,  "L"  },   /* high-comma-off-center L */
	{ 0xfbdb,  "T"  },   /* high-comma-off-center T */
	{ 0xfbdc,  "d"  },   /* high-comma-off-center d */
	{ 0xfbdd,  "g"  },   /* high-comma-off-center g */
	{ 0xfbde,  "k"  },   /* high-comma-off-center k */
	{ 0xfbdf,  "l"  },   /* high-comma-off-center l */
	{ 0xfbe0,  "t"  },   /* high-comma-off-center t */
	{ 0xfbe1,  "A"  },   /* double-acute A */
	{ 0xfbe2,  "O"  },   /* double-acute O */
	{ 0xfbe3,  "U"  },   /* double-acute U */
	{ 0xfbe4,  "a"  },   /* double-acute a */
	{ 0xfbe5,  "o"  },   /* double-acute o */
	{ 0xfbe6,  "u"  },   /* double-acute u */
	{ 0xfbe7,  "A"  },   /* candrabindu A */
	{ 0xfbe8,  "H"  },   /* candrabindu H */
	{ 0xfbe9,  "I"  },   /* candrabindu I */
	{ 0xfbea,  "M"  },   /* candrabindu M */
	{ 0xfbeb,  "N"  },   /* candrabindu N */
	{ 0xfbec,  "O"  },   /* candrabindu O */
	{ 0xfbed,  "U"  },   /* candrabindu U */
	{ 0xfbee,  "a"  },   /* candrabindu a */
	{ 0xfbef,  "h"  },   /* candrabindu h */
	{ 0xfbf0,  "i"  },   /* candrabindu i */
	{ 0xfbf1,  "m"  },   /* candrabindu m */
	{ 0xfbf2,  "n"  },   /* candrabindu n */
	{ 0xfbf3,  "o"  },   /* candrabindu o */
	{ 0xfbf4,  "u"  },   /* candrabindu u */
	{ 0xfbf5,  "C"  },   /* cedilla C */
	{ 0xfbf6,  "E"  },   /* cedilla E */
	{ 0xfbf7,  "H"  },   /* cedilla H */
	{ 0xfbf8,  "L"  },   /* cedilla L */
	{ 0xfbf9,  "N"  },   /* cedilla N */
	{ 0xfbfa,  "S"  },   /* cedilla S */
	{ 0xfbfb,  "c"  },   /* cedilla c */
	{ 0xfbfc,  "e"  },   /* cedilla e */
	{ 0xfbfd,  "h"  },   /* cedilla h */
	{ 0xfbfe,  "l"  },   /* cedilla l */
	{ 0xfca1,  "n"  },   /* cedilla n */
	{ 0xfca2,  "s"  },   /* cedilla s */
	{ 0xfca3,  "A"  },   /* right-hook A */
	{ 0xfca4,  "E"  },   /* right-hook E */
	{ 0xfca5,  "H"  },   /* right-hook H */
	{ 0xfca6,  "I"  },   /* right-hook T */
	{ 0xfca7,  "O"  },   /* right-hook O */
	{ 0xfca8,  "S"  },   /* right-hook S */
	{ 0xfca9,  "T"  },   /* right-hook T */
	{ 0xfcaa,  "U"  },   /* right-hook U */
	{ 0xfcab,  "a"  },   /* right-hook a */
	{ 0xfcac,  "e"  },   /* right-hook e */
	{ 0xfcad,  "h"  },   /* right-hook h */
	{ 0xfcae,  "i"  },   /* right-hook i */
	{ 0xfcaf,  "o"  },   /* right-hook o */
	{ 0xfcb0,  "s"  },   /* right-hook s */
	{ 0xfcb1,  "t"  },   /* right-hook y */
	{ 0xfcb2,  "u"  },   /* right-hook u */
	{ 0xfcb3,  "O"  },   /*  */    /* new */
	{ 0xfcb4,  "o"  },   /*  */    /* new */
	{ 0xfcb5,  "A"  },   /* dot-below-character A */
	{ 0xfcb6,  "B"  },   /* dot-below-character B */
	{ 0xfcb7,  "D"  },   /* dot-below-character D */
	{ 0xfcb8,  "E"  },   /* dot-below-character E */
	{ 0xfcb9,  "H"  },   /* dot-below-character H */
	{ 0xfcba,  "I"  },   /* dot-below-character I */
	{ 0xfcbb,  "K"  },   /* dot-below-character K */
	{ 0xfcbc,  "L"  },   /* dot-below-character L */
	{ 0xfcbd,  "M"  },   /* dot-below-character M */
	{ 0xfcbe,  "N"  },   /* dot-below-character N */
	{ 0xfcbf,  "O"  },   /* dot-below-character O */
	{ 0xfcc0,  "P"  },   /* dot-below-character P */
	{ 0xfcc1,  "R"  },   /* dot-below-character R */
	{ 0xfcc2,  "S"  },   /* dot-below-character S */
	{ 0xfcc3,  "T"  },   /* dot-below-character T */
	{ 0xfcc4,  "U"  },   /* dot-below-character U */
	{ 0xfcc5,  "V"  },   /* dot-below-character V */
	{ 0xfcc6,  "X"  },   /* dot-below-character X */
	{ 0xfcc7,  "Y"  },   /* dot-below-character Y */
	{ 0xfcc8,  "Z"  },   /* dot-below-character Z */
	{ 0xfcc9,  "a"  },   /* dot-below-character a */
	{ 0xfcca,  "b"  },   /* dot-below-character b */
	{ 0xfccb,  "d"  },   /* dot-below-character d */
	{ 0xfccc,  "e"  },   /* dot-below-character e */
	{ 0xfccd,  "h"  },   /* dot-below-character h */
	{ 0xfcce,  "i"  },   /* dot-below-character i */
	{ 0xfccf,  "k"  },   /* dot-below-character k */
	{ 0xfcd0,  "l"  },   /* dot-below-character l */
	{ 0xfcd1,  "m"  },   /* dot-below-character m */
	{ 0xfcd2,  "n"  },   /* dot-below-character n */
	{ 0xfcd3,  "o"  },   /* dot-below-character o */
	{ 0xfcd4,  "p"  },   /* dot-below-character p */
	{ 0xfcd5,  "r"  },   /* dot-below-character r */
	{ 0xfcd6,  "s"  },   /* dot-below-character s */
	{ 0xfcd7,  "t"  },   /* dot-below-character t */
	{ 0xfcd8,  "u"  },   /* dot-below-character u */
	{ 0xfcd9,  "v"  },   /* dot-below-character v */
	{ 0xfcda,  "x"  },   /* dot-below-character x */
	{ 0xfcdb,  "y"  },   /* dot-below-character y */
	{ 0xfcdc,  "z"  },   /* dot-below-character z */
        { 0xfcdd,  "O"  },   /*  */    /* new */
	{ 0xfcde,  "U"  },   /*  */    /* new */
	{ 0xfcdf,  "o"  },   /*  */    /* new */
	{ 0xfce0,  "u"  },   /*  */    /* new */
	{ 0xfce1,  "B"  },   /* doble-dot-below-character B */
	{ 0xfce2,  "D"  },   /* doble-dot-below-character D */
	{ 0xfce3,  "H"  },   /* doble-dot-below-character H */
	{ 0xfce4,  "L"  },   /* doble-dot-below-character L */
	{ 0xfce5,  "S"  },   /* doble-dot-below-character S */
	{ 0xfce6,  "T"  },   /* doble-dot-below-character T */
	{ 0xfce7,  "Z"  },   /* doble-dot-below-character Z */
	{ 0xfce8,  "b"  },   /* doble-dot-below-character b */
	{ 0xfce9,  "d"  },   /* doble-dot-below-character d */
	{ 0xfcea,  "h"  },   /* doble-dot-below-character h */
	{ 0xfceb,  "l"  },   /* doble-dot-below-character l */
	{ 0xfcec,  "s"  },   /* doble-dot-below-character s */
	{ 0xfced,  "t"  },   /* doble-dot-below-character t */
	{ 0xfcee,  "z"  },   /* doble-dot-below-character z */
	{ 0xfcef,  "L"  },   /* circle-below-character L */
	{ 0xfcf0,  "M"  },   /* circle-below-character M */
	{ 0xfcf1,  "N"  },   /* circle-below-character N */
	{ 0xfcf2,  "R"  },   /* circle-below-character R */
	{ 0xfcf3,  "S"  },   /* circle-below-character S */
	{ 0xfcf4,  "T"  },   /* circle-below-character T */
	{ 0xfcf5,  "l"  },   /* circle-below-character l */
	{ 0xfcf6,  "m"  },   /* circle-below-character m */
	{ 0xfcf7,  "n"  },   /* circle-below-character n */
	{ 0xfcf8,  "r"  },   /* circle-below-character r */
	{ 0xfcf9,  "s"  },   /* circle-below-character s */
	{ 0xfcfa,  "t"  },   /* circle-below-character t */
	{ 0xfcfb,  "G"  },   /* doble-underscore G */
	{ 0xfcfc,  "H"  },   /* doble-underscore H */
	{ 0xfcfd,  "g"  },   /* doble-underscore g */
	{ 0xfcfe,  "h"  },   /* doble-underscore h */
	{ 0xfda1,  "A"  },   /* underscore A */
	{ 0xfda2,  "B"  },   /* underscore B */
	{ 0xfda3,  "C"  },   /* underscore C */
	{ 0xfda4,  "D"  },   /* underscore D */
	{ 0xfda5,  "G"  },   /* underscore G */
	{ 0xfda6,  "H"  },   /* underscore H */
	{ 0xfda7,  "I"  },   /* underscore I */
	{ 0xfda8,  "K"  },   /* underscore K */
	{ 0xfda9,  "L"  },   /* underscore L */
	{ 0xfdaa,  "N"  },   /* underscore N */
	{ 0xfdab,  "O"  },   /* underscore O */
	{ 0xfdac,  "R"  },   /* underscore R */
	{ 0xfdad,  "S"  },   /* underscore S */
	{ 0xfdae,  "T"  },   /* underscore T */
	{ 0xfdaf,  "U"  },   /* underscore U */
	{ 0xfdb0,  "Z"  },   /* underscore Z */
	{ 0xfdb1,  "a"  },   /* underscore a */
	{ 0xfdb2,  "b"  },   /* underscore b */
	{ 0xfdb3,  "c"  },   /* underscore c */
	{ 0xfdb4,  "d"  },   /* underscore d */
	{ 0xfdb5,  "g"  },   /* underscore g */
	{ 0xfdb6,  "h"  },   /* underscore h */
	{ 0xfdb7,  "i"  },   /* underscore i */
	{ 0xfdb8,  "k"  },   /* underscore k */
	{ 0xfdb9,  "l"  },   /* underscore l */
	{ 0xfdba,  "n"  },   /* underscore n */
	{ 0xfdbb,  "o"  },   /* underscore o */
	{ 0xfdbc,  "r"  },   /* underscore r */
	{ 0xfdbd,  "s"  },   /* underscore s */
	{ 0xfdbe,  "t"  },   /* underscore t */
	{ 0xfdbf,  "u"  },   /* underscore u */
	{ 0xfdc0,  "z"  },   /* underscore z */
	{ 0xfdc1,  "A"  },   /* left-hook A */
	{ 0xfdc2,  "C"  },   /* left-hook C */
	{ 0xfdc3,  "D"  },   /* left-hook D */
	{ 0xfdc4,  "E"  },   /* left-hook E */
	{ 0xfdc5,  "G"  },   /* left-hook G */
	{ 0xfdc6,  "I"  },   /* left-hook I */
	{ 0xfdc7,  "K"  },   /* left-hook K */
	{ 0xfdc8,  "L"  },   /* left-hook L */
	{ 0xfdc9,  "N"  },   /* left-hook N */
	{ 0xfdca,  "R"  },   /* left-hook R */
	{ 0xfdcb,  "S"  },   /* left-hook S */
	{ 0xfdcc,  "T"  },   /* left-hook T */
	{ 0xfdcd,  "a"  },   /* left-hook a */
	{ 0xfdce,  "c"  },   /* left-hook c */
	{ 0xfdcf,  "d"  },   /* left-hook d */
	{ 0xfdd0,  "e"  },   /* left-hook e */
	{ 0xfdd1,  "g"  },   /* left-hook g */
	{ 0xfdd2,  "i"  },   /* left-hook i */
	{ 0xfdd3,  "k"  },   /* left-hook k */
	{ 0xfdd4,  "l"  },   /* left-hook l */
	{ 0xfdd5,  "n"  },   /* left-hook n */
	{ 0xfdd6,  "r"  },   /* left-hook r */
	{ 0xfdd7,  "s"  },   /* left-hook s */
	{ 0xfdd8,  "t"  },   /* left-hook t */
	{ 0xfdd9,  "O"  },   /* right-cedilla O */
	{ 0xfdda,  "o"  },   /* right-cedilla o */
	{ 0xfddb,  "H"  },   /* upadhmaniya H */
	{ 0xfddc,  "h"  },   /* upadhmaniya h */
	{ 0xfddd,  "N"  },   /* double-tilde-first N */
	{ 0xfdde,  "n"  },   /* double-tilde-first n */
	{ 0xfddf,  "G"  },   /* double-tilde-second G */
	{ 0xfde0,  "g"  },   /* double-tilde-second g */
	{ 0xfde1,  "G"  },   /* high-comma-centered G */
	{ 0xfde2,  "W"  },   /* high-comma-centered W */
	{ 0xfde3,  "g"  },   /* high-comma-centered g */
	{ 0xfde4,  "w"  }    /* high-comma-centered w */
}; 
