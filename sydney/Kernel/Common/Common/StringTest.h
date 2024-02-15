// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// StringTest.h --
// 
// Copyright (c) 1999, 2023 Ricoh Company, Ltd.
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

//#define MESSAGE_ASSERT

#ifdef MESSAGE_ASSERT
#include "Common/Message.h"
#undef _TRMEISTER_ASSERT
#define _TRMEISTER_ASSERT(__b__) \
	if (!(__b__)) { \
		SydMessage << "Like bug: data=\"" << cData.getValue() \
				   << "\" pattern=\"" << cPattern.getValue() \
				   << ModEndl; \
	}
#else
#include "Common/Assert.h"
#endif

class StringTest
{
public:
	StringTest() {dotest();}

	void dotest() {

		StringData cData(_TRMEISTER_U_STRING(""));
		StringData cPattern(_TRMEISTER_U_STRING(""));
		bool b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%%%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cData.setValue(_TRMEISTER_U_STRING("あうあうほげほげがうがう"));

		cPattern.setValue(_TRMEISTER_U_STRING("%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%%%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あう%ほげ%がう"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あう%ほげ%がう"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あう%ほげ%がう"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あう%ぐげ%がう"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("がう%ほげ%がう"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あう%ほげ%ぐげ"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あう%ほげ%げが%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あう%ほげ%ぐげが%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あう%ぐげ%げが%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("がう%ほげ%げが%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("%うほ%ほげ%がう"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%うほ%ほげ%ぐげ"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("%うほ%ぐげ%がう"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("%ぐほ%ほげ%がう"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("%うほ%ほげ%がう"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%うほ%ほげ%ぐが%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("%うほ%ぐげ%うが%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("%ぐほ%ほげ%うが%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(!b);

		cData.setValue(_TRMEISTER_U_STRING("auuuaauaua"));

		cPattern.setValue(_TRMEISTER_U_STRING("%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%%%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%uaa%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%%uaa%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%uaa%%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%%%uaa%%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cData.setValue(_TRMEISTER_U_STRING("hoge10%"));

		cPattern.setValue(_TRMEISTER_U_STRING("%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%%%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%ge__+%"));
		b = cData.like(&cPattern, UnicodeChar::usPlus);
		; _TRMEISTER_ASSERT(b);

		cData.setValue(_TRMEISTER_U_STRING("あいうえおかきくけこ"));

		cPattern.setValue(_TRMEISTER_U_STRING("%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%%%"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あいうえお%すせそ"));
		b = cData.like(&cPattern, UnicodeChar::usPlus);
		; _TRMEISTER_ASSERT(!b);

		ModUnicodeChar escape = Common::UnicodeChar::usPlus;
		cData.setValue(_TRMEISTER_U_STRING("あい+あう++あえ+++あお++++かき+++++かく++++かけ+++かこ++さし+"));

		cPattern.setValue(_TRMEISTER_U_STRING("%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%%%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい+%かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい++%かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい+++%かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい++++%かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき+%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき++%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき+++%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき++++%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき+++++%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき++++++%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき+++++++%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき++++++++%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき+++++++++%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき++++++++++%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき+++++++++++%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき++++++++++++%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき%さし+%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき%さし++%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき%さし+++%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき%さし++"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき%さし++++"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("%+あう%かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%++あう%かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%+++あう%かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%++++あう%かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%+かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%++かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%+++かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%++++かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%+++++かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%++++++かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%+++++++かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%++++++++かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%+++++++++かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%++++++++++かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき%+さし++"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき%++さし++"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき%+++さし++"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき%++++さし++"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき%+++++さし++"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき%++++++さし++"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(!b);

		cData.setValue(_TRMEISTER_U_STRING("あい%あう%%あえ%%%あお%%%%かき%%%%%かく%%%%かけ%%%かこ%%さし%"));

		cPattern.setValue(_TRMEISTER_U_STRING("%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("%%%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい+%%かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい++%%かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい+%かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい+%あう%かき%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき+%%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき++%%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき+%+%%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき+%+%+%%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき+%+%+%+%%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき+%+%+%+%+%%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき+%+%+%+%+%+%%さし%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき%さし+%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき%さし+%%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき%さし++%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい%かき%さし+%+%"));
		b = cData.like(&cPattern, escape);
		; _TRMEISTER_ASSERT(!b);

		cData.setValue(_TRMEISTER_U_STRING("%あいうえお"));
		cPattern.setValue(_TRMEISTER_U_STRING("/%%/_"));
		b = cData.like(&cPattern, UnicodeChar::usSlash);
		; _TRMEISTER_ASSERT(!b);

		cData.setValue(_TRMEISTER_U_STRING("あいうえお"));
		cPattern.setValue(_TRMEISTER_U_STRING("あい_"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい__"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい___"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あい____"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("_えお"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("__えお"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("___えお"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("____えお"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あ_お"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あ__お"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(!b);

		cPattern.setValue(_TRMEISTER_U_STRING("あ___お"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(b);

		cPattern.setValue(_TRMEISTER_U_STRING("あ____お"));
		b = cData.like(&cPattern);
		; _TRMEISTER_ASSERT(!b);

	}
};

StringTest t;

#ifdef MESSAGE_ASSERT
#undef _TRMEISTER_ASSERT
#undef SydAssert
#undef __TRMEISTER_COMMON_ASSERT_H
#include "Common/Assert.h"
#endif

//
//	Copyright (c) 1999, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
