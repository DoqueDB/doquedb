// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Blocker.cpp
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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
#include "FullText2/Blocker.h"

#include "Exception/SQLSyntaxError.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::Blocker::Blocker -- コンストラクタ
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
Blocker::Blocker()
{
}

//
//	FUNCTION public
//	FullText2::Blocker::~Blocker -- デストラクタ
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
Blocker::~Blocker()
{
}

//
//	FUNCTION protected static
//	FullText2::Blocker::nextToken -- 次のトークンを得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeChar*& target_
//		対象の文字列。次のトークンの先頭まで進められる
//	ModVector<const ModUnicodeChar*> element_
//		要素の配列
//
//	RETURN
//	FullText2::Blocker::Token::Type
//		トークンのタイプ
//
//	EXCEPTIONS
//
Blocker::Token::Type
Blocker::nextToken(const ModUnicodeChar*& target_,
				   ModVector<const ModUnicodeChar*>& element_)
{
	ModSize type = 0;
	ModSize now = 0;
	
	element_.assign(3, 0);	// 今は最大３要素
	ModVector<const ModUnicodeChar*>::Iterator i = element_.begin();
	
	while (*target_ != 0)
	{
		if (*target_ >= '0' && *target_ <= '9')
		{
			if ((*i) == 0) {
				*i = target_;
				now = Token::Number;
			}
		}
		else if (*target_ >= 'A' && *target_ <= 'Z')
		{
			if ((*i) == 0) {
				*i = target_;
				now = Token::Keyword;
			}
			if (now == Token::Number)
				// 数字の後にアルファベットが来たらエラー
				_TRMEISTER_THROW1(Exception::SQLSyntaxError, target_);
		}
		else if (*target_ == ':')
		{
			// この要素は終わり
			++i;
			type <<= 4;
			type |= now;
			now = 0;
		}
		else if (*target_ == ' ')
		{
			// スペースなので、スペースの間読み飛ばす
			while (*target_ != 0 && *target_ == ' ') ++target_;
			// 終了
			break;
		}
		else
		{
			// その他の文字が来たらエラー
			_TRMEISTER_THROW1(Exception::SQLSyntaxError, target_);
		}

		++target_;
	}

	type <<= 4;
	type |= now;
	now = 0;

	return static_cast<Token::Type>(type);
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
