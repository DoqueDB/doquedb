// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UnicodeChar.h -- Unicode 文字の定義ファイル
// 
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
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

#ifndef __UNICODECHAR__HEADER__
#define __UNICODECHAR__HEADER__

#include "ModUnicodeChar.h"
#include "ModKanjiCode.h"

namespace UNA {

// character encoding
const ModKanjiCode::KanjiCodeType LiteralCode =
#if defined(SOURCE_ENCODING_SHIFTJIS)
		ModKanjiCode::shiftJis;
#elif defined(SOURCE_ENCODING_EUC)
		ModKanjiCode::euc;
#elif defined(SOURCE_ENCODING_UTF8)
		ModKanjiCode::utf8;
#else
		ModKanjiCode::unknown;
#endif

	namespace UnicodeChar
	{
	// UNICODE の文字を表す定数を定義する
	
	//	const	ModUnicodeChar	us				=	0x00;		// ''
		const	ModUnicodeChar	usNull			=	0x0000;		// '\0'
	
		const	ModUnicodeChar	usCtrlTab		=	0x0009;		// '\t'
		const	ModUnicodeChar	usCtrlRet		=	0x000a;		// '\n'
		const	ModUnicodeChar	usCtrlCr		=	0x000d;		// '\r'
			
		const	ModUnicodeChar	usSpace			=	0x0020;		// ' '
		const	ModUnicodeChar	usExclamation		=	0x0021;		// '!'
		const	ModUnicodeChar	usWquate		=	0x0022;		// '"'
		const	ModUnicodeChar	usSharp			=	0x0023;		// '#'
		const	ModUnicodeChar	usDollar		=	0x0024;		// '$'
		const	ModUnicodeChar	usPercent		=	0x0025;		// '%'
		const	ModUnicodeChar	usAmpersand		=	0x0026;		// '&'
		const	ModUnicodeChar	usQuate			=	0x0027;		// '''
		const	ModUnicodeChar	usLparent		=	0x0028;		// '('
		const	ModUnicodeChar	usRparent		=	0x0029;		// ')'
		const	ModUnicodeChar	usAsterisc		=	0x002a;		// '*'
		const	ModUnicodeChar	usPlus			=	0x002b;		// '+'
		const	ModUnicodeChar	usComma			=	0x002c;		// ','
		const	ModUnicodeChar	usHyphen		=	0x002d;		// '-'
		const	ModUnicodeChar	usPeriod		=	0x002e;		// '.'
		const	ModUnicodeChar	usSlash			=	0x002f;		// '/'
		const	ModUnicodeChar	usColon			=	0x003a;		// ':'
		const	ModUnicodeChar	usSemiColon		=	0x003b;		// ';'
		const	ModUnicodeChar	usLcompare		=	0x003c;		// '<'
		const	ModUnicodeChar	usEqual			=	0x003d;		// '='
		const	ModUnicodeChar	usRcompare		=	0x003e;		// '>'
		const	ModUnicodeChar	usQuestion		=	0x003f;		// '?'
		const	ModUnicodeChar	usAtMark		=	0x0040;		// '@'
		const 	ModUnicodeChar	usLbracket		=	0x005b;		// '['
		const	ModUnicodeChar	usBackSlash		=	0x005c;		// '\'
		const 	ModUnicodeChar	usRbracket		=	0x005d;		// ']'
		const	ModUnicodeChar	usLowLine		=	0x005f;		// '_'
		const	ModUnicodeChar	usRSquote		=	0x0060;		// '`'
		const	ModUnicodeChar	usLbrace		=	0x007b;		// '{'
		const	ModUnicodeChar	usVerticalLine		=	0x007c;		// '|'
		const	ModUnicodeChar	usRbrace		=	0x007d;		// '}'
		const	ModUnicodeChar	usAboveLine		=	0x007e;		// '~'
	
		const	ModUnicodeChar	usZero			=	0x0030;		// '0'
		const	ModUnicodeChar	usOne			=	0x0031;		// '1'
		const	ModUnicodeChar	usTwo			=	0x0032;		// '2'
		const	ModUnicodeChar	usThree			=	0x0033;		// '3'
		const	ModUnicodeChar	usFour			=	0x0034;		// '4'
		const	ModUnicodeChar	usFive			=	0x0035;		// '5'
		const	ModUnicodeChar	usSix			=	0x0036;		// '6'
		const	ModUnicodeChar	usSeven			=	0x0037;		// '7'
		const	ModUnicodeChar	usEight			=	0x0038;		// '8'
		const	ModUnicodeChar	usNine			=	0x0039;		// '9'
	
		const	ModUnicodeChar	usLargeA		=	0x0041;		// 'A'
		const	ModUnicodeChar	usLargeB		=	0x0042;		// 'B'
		const	ModUnicodeChar	usLargeC		=	0x0043;		// 'C'
		const	ModUnicodeChar	usLargeD		=	0x0044;		// 'D'
		const	ModUnicodeChar	usLargeE		=	0x0045;		// 'E'
		const	ModUnicodeChar	usLargeF		=	0x0046;		// 'F'
		const	ModUnicodeChar	usLargeG		=	0x0047;		// 'G'
		const	ModUnicodeChar	usLargeH		=	0x0048;		// 'H'
		const	ModUnicodeChar	usLargeI		=  	0x0049;		// 'I'
		const	ModUnicodeChar	usLargeJ		=	0x004a;		// 'J'
		const	ModUnicodeChar	usLargeK		=	0x004b;		// 'K'
		const	ModUnicodeChar	usLargeL		=	0x004c;		// 'L'
		const	ModUnicodeChar	usLargeM		=	0x004d;		// 'M'
		const	ModUnicodeChar	usLargeN		=	0x004e;		// 'N'
		const	ModUnicodeChar	usLargeO		=	0x004f;		// 'O'
		const	ModUnicodeChar	usLargeP		=	0x0050;		// 'P'
		const	ModUnicodeChar	usLargeQ		=	0x0051;		// 'Q'
		const	ModUnicodeChar	usLargeR		=	0x0052;		// 'R'
		const	ModUnicodeChar	usLargeS		=	0x0053;		// 'S'
		const	ModUnicodeChar	usLargeT		=	0x0054;		// 'T'
		const	ModUnicodeChar	usLargeU		=	0x0055;		// 'U'
		const	ModUnicodeChar	usLargeV		=	0x0056;		// 'V'
		const	ModUnicodeChar	usLargeW		=	0x0057;		// 'W'
		const	ModUnicodeChar	usLargeX		=	0x0058;		// 'X'
		const	ModUnicodeChar	usLargeY		=	0x0059;		// 'Y'
		const	ModUnicodeChar	usLargeZ		=	0x005a;		// 'Z'
	
		const	ModUnicodeChar	usSmallA		=	0x0061;		// 'a'
		const	ModUnicodeChar	usSmallB		=	0x0062;		// 'b'
		const	ModUnicodeChar	usSmallC		=	0x0063;		// 'c'
		const	ModUnicodeChar	usSmallD		=	0x0064;		// 'd'
		const	ModUnicodeChar	usSmallE		=	0x0065;		// 'e'
		const	ModUnicodeChar	usSmallF		=	0x0066;		// 'f'
		const	ModUnicodeChar	usSmallG		=	0x0067;		// 'g'
		const	ModUnicodeChar	usSmallH		=	0x0068;		// 'h'
		const	ModUnicodeChar	usSmallI		=	0x0069;		// 'i'
		const	ModUnicodeChar	usSmallJ		=	0x006a;		// 'j'
		const	ModUnicodeChar	usSmallK		=	0x006b;		// 'k'
		const	ModUnicodeChar	usSmallL	   	=	0x006c;		// 'l'
		const	ModUnicodeChar	usSmallM		=	0x006d;		// 'm'
		const	ModUnicodeChar	usSmallN		=	0x006e;		// 'n'
		const	ModUnicodeChar	usSmallO		=	0x006f;		// 'o'
		const	ModUnicodeChar	usSmallP		=	0x0070;		// 'p'
		const	ModUnicodeChar	usSmallQ		=	0x0071;		// 'q'
		const	ModUnicodeChar	usSmallR		=	0x0072;		// 'r'
		const	ModUnicodeChar	usSmallS		=	0x0073;		// 's'
		const	ModUnicodeChar	usSmallT		=	0x0074;		// 't'
		const	ModUnicodeChar	usSmallU		=	0x0075;		// 'u'
		const	ModUnicodeChar	usSmallV		=	0x0076;		// 'v'
		const	ModUnicodeChar	usSmallW		=	0x0077;		// 'w'
		const	ModUnicodeChar	usSmallX		=	0x0078;		// 'x'
		const	ModUnicodeChar	usSmallY		=	0x0079;		// 'y'
		const	ModUnicodeChar	usSmallZ		=	0x007a;		// 'z'
	
		const	ModUnicodeChar	usBOM			=	0xfeff;		// Byte Order Mark
	
	}
}

#endif // __UNICODECHAR__HEADER__

//
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
