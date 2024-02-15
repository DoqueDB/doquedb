// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// JapaneseBlocker3.cpp
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/JapaneseBlocker3.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	// 同の字点「々」のコード値
	const ModUnicodeChar _DOU = 0x3005;
}

//
//	FUNCTION public
//	FullText2::JapaneseBlocker3::JapaneseBlocker3 -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
JapaneseBlocker3::JapaneseBlocker3()
	: JapaneseBlocker2()
{
}

//
//	FUNCTION public
//	FullText2::JapaneseBlocker3::~JapaneseBlocker3 -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
JapaneseBlocker3::~JapaneseBlocker3()
{
}

//
//	FUNCTION protected
//	FullText2::JapaneseBlocker3::getBlock -- ブロック種別を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeChar c_
//		文字
//
//	RETURN
//	ModSize
//		ブロック種別
//
//	EXCEPTIONS
//
ModSize
JapaneseBlocker3::getBlock(ModUnicodeChar c_) const
{
	ModSize r = JapaneseBlocker::getBlock(c_);
	if (r == Symbol && c_ == _DOU)
	{
		// 同の字点「々」なので、記号ではなく漢字にする
		r = Kanji;
	}
	return r;
}

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
