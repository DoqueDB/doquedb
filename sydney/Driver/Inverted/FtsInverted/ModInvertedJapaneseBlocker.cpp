// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedJapaneseBlocker.cpp -- 
// 
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
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

#ifdef SYD_INVERTED // SYDNEY 対応
#include "SyDefault.h"
#include "SyReinterpretCast.h"
#endif

#include "ModAssert.h"
#include "ModInvertedException.h"
#include "ModInvertedJapaneseBlocker.h"

//
// CONST
// ModInvertedJapaneseBlocker::blockBegin -- 各ブロックの開始文字
//
// NOTES
// 各ブロックの開始文字を表す。
//
/*static*/ ModUnicodeChar
ModInvertedJapaneseBlocker::blockBegin[] = {
	0x0000,		// undefined
	0x0000,		// ascii
	0x0000,		// symbol		XXXX
	0x0660,		// digit (ascii は除く)
	0x00C0,		// alphabet	(ascii は除く)
	0x3040,		// hiragana
	0x30A0,		// katakana
	0x0370,		// greek
	0x0400,		// russian
	0x2500,		// line
	0x4E00,		// kanji
	0xFF61,		// hankakukana
	0xE000		// gaiji
};
#if 0
; ModAssert(
	sizeof(ModInvertedJapaneseBlocker::blockBegin)/sizeof(ModUnicodeChar)
	== ModInvertedJapaneseBlocker::blockNum);
#endif

//
// CONST
// ModInvertedJapaneseBlocker::blockEnd -- 各ブロックの末尾文字
//
// NOTES
// 各ブロックの末尾文字（そのブロックの最後の文字の次の文字）を表す。
//
/*static*/ ModUnicodeChar
ModInvertedJapaneseBlocker::blockEnd[] = {
	0xFFFF,		// undefined
	0x0080,		// ascii		007F
	0xFFFF,		// symbol		XXXX
	0xFF1A,		// digit		FF19
	0xFF5B,		// alphabet		FF5A
	0x30A0,		// hiragana		309F
	0x3100,		// katakana		30FF
	0x2000,		// greek		03FF 1FFF
	0x0500,		// russian		04FF
	0x2580,		// line			257F
	0xA000,		// kanji		9FFF
	0xFFA0,		// hankakukana	FF9F
	0xF900		// gaiji		F8FF
};
#if 0
; ModAssert(
	sizeof(ModInvertedJapaneseBlocker::blockEnd)/sizeof(ModUnicodeChar)
	== ModInvertedJapaneseBlocker::blockNum);
#endif

void
ModInvertedJapaneseBlocker::getBlockRegion(const ModSize blockId_,
										   ModUnicodeChar& begin_,
										   ModUnicodeChar& end_) const
{
	if (blockId_ >= blockNum) {
		ModErrorMessage << "getBlockRegion failed: invalid id: "
						<< blockId_ << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorInternal);
	}

	begin_ = blockBegin[blockId_];
	end_ = blockEnd[blockId_];
}


//
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
