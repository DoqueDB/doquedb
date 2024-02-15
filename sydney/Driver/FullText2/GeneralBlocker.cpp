// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// GeneralBlocker.cpp
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
#include "FullText2/GeneralBlocker.h"

#include "Exception/SQLSyntaxError.h"

#include "ModUnicodeCharTrait.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::GeneralBlocker::GeneralBlocker -- コンストラクタ
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
GeneralBlocker::GeneralBlocker()
	: m_pTarget(0), m_uiTargetLength(0),
	  m_uiTailOffset(0), m_uiHeadOffset(0),
	  m_uiMaxLength(0), m_uiMinLength(0),
	  m_uiCurrentLength(0), m_uiCurrentMaxLength(0),
	  m_bShortWord(false)
{
}

//
//	FUNCTION public
//	FullText2::GeneralBlocker::~GeneralBlocker -- デストラクタ
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
GeneralBlocker::~GeneralBlocker()
{
}

//
//	FUNCTION public
//	FullText2::GeneralBlocker::parse
//		-- トークナイズパラメータをパースする
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeChar* pParameter_
//		トークナイズパラメータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
GeneralBlocker::parse(const ModUnicodeChar* pParameter_)
{
	// デフォルトはバイグラム
	m_uiMinLength = m_uiMaxLength = 2;

	if (*pParameter_ != 0 && *pParameter_ != '@')
	{
		// ここには、例えば NGR:1:3 の 1:3 の部分のみが来る

		ModVector<const ModUnicodeChar*> element;
		Blocker::Token::Type eType = Blocker::nextToken(pParameter_, element);

		if (eType == Blocker::Token::Number)
		{
			m_uiMinLength = ModUnicodeCharTrait::toInt(element[0]);
			m_uiMaxLength = m_uiMinLength;
		}
		else if (eType == Blocker::Token::Number_Number)
		{
			m_uiMinLength = ModUnicodeCharTrait::toInt(element[0]);
			m_uiMaxLength = ModUnicodeCharTrait::toInt(element[1]);
		}
		else
		{
			_TRMEISTER_THROW1(Exception::SQLSyntaxError, pParameter_);
		}
	}
}

//
//	FUNCTION public
//	FullText2::GeneralBlocker::set
//		-- トークナイズ対象の文字列を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cToken_
//		トークナイズ対象の文字列
//		この文字列のメモリは呼び出し側が本クラス消滅まで保持する必要あり
//	bool bSearch_
//		検索時かどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
GeneralBlocker::set(const ModUnicodeString& cTarget_,
					bool bSearch_)
{
	// メンバー変数に引数を設定する
	m_bSearch = bSearch_;
	m_pTarget = cTarget_;
	m_uiTargetLength = cTarget_.getLength();

	// トークナイズに必要な変数を初期化する
	m_uiTailOffset = 0;
	m_uiHeadOffset = 0;
	m_bShortWord = false;

	if (m_uiTargetLength == 0)
	{
		// 空文字列なので、次の yield で false が返るようにする
		m_uiCurrentLength = 0;
		return;
	}

	m_uiCurrentLength = (m_bSearch) ? m_uiMaxLength : m_uiMinLength;
	m_uiCurrentMaxLength = 0;
	
	// 先頭位置を進めておく

	if (m_uiCurrentLength > m_uiTargetLength)
	{
		// 長さが足りない場合
		m_uiHeadOffset = m_uiTargetLength;
		m_uiCurrentMaxLength = m_uiTargetLength;
		
		if (m_bSearch == true)
		{
			if (m_uiMinLength > m_uiTargetLength)
			{
				// MIN よりも検索文字列が短ければショートワード処理
				
				m_bShortWord = true;
			}
			m_uiCurrentLength = m_uiCurrentMaxLength;
		}
	}
	else
	{
		m_uiHeadOffset = m_uiCurrentLength;
		if (m_uiMaxLength > m_uiTargetLength)
		{
			m_uiCurrentMaxLength = m_uiTargetLength;
		}
		else
		{
			m_uiCurrentMaxLength = m_uiMaxLength;
		}
	}
}

//
//	FUNCTION public
//	FullText2::GeneralBlocker::yield -- トークンの取得
//
//	NOTES
//	処理対象のテキストからトークンを取得し、位置情報とともに出力する
//	新たなトークンを生成できない場合は false を返す
//
//	ARGUMENTS
//	ModUnicodeString& cToken_
//		トークン
//	ModSize& uiOffset_
//		位置情報
//	bool& isShort_
//		ショートワードかどうか
//	ModSize& uiMinLength_
//		ショートワードの場合、その文字種の最小長
//
//	RETURN
//	bool
//		新たなトークンを生成できない場合は false を返す
//
//	EXCEPTIONS
//
bool
GeneralBlocker::yield(ModUnicodeString& cToken_,
					  ModSize& uiOffset_,
					  bool& isShort_,
					  ModSize& uiMinLength_)
{
	if (m_uiCurrentLength == 0)
		return false;

	isShort_ = m_bShortWord;

	if (m_bShortWord)
	{
		// ショートワードなら現在のトークンを出力して終わり
		cToken_.allocateCopy(m_pTarget + m_uiTailOffset, m_uiCurrentLength);
		uiOffset_ = m_uiTailOffset;
		m_uiCurrentLength = 0;
		uiMinLength_ = m_uiMinLength;
		return true;
	}
	
	if (m_uiCurrentLength < m_uiCurrentMaxLength)
	{
		cToken_.allocateCopy(m_pTarget + m_uiTailOffset, m_uiCurrentLength);
		++m_uiCurrentLength;
		++m_uiHeadOffset;
		uiOffset_ = m_uiTailOffset;
		return true;
	}
	
	if (m_bSearch &&
		m_uiCurrentLength < m_uiMaxLength &&
		m_uiTailOffset != 0 &&
		m_uiHeadOffset == m_uiTargetLength)
	{
		// 検索時モードで、
		// 現在のトークンの長さが最大長未満であり、
		// 末尾のオフセットがターゲット長に達していたら、終了
		
		return false;
	}

	cToken_.allocateCopy(m_pTarget + m_uiTailOffset, m_uiCurrentLength);
	uiOffset_ = m_uiTailOffset;

	++m_uiTailOffset;
	if (m_uiTailOffset >= m_uiTargetLength)
	{
		// 末尾に達した -- 現在のトークンは出力する
		m_uiCurrentLength = 0;
		return true;
	}

	m_uiCurrentLength = (m_bSearch == false) ? m_uiMinLength : m_uiMaxLength;

	m_uiHeadOffset = m_uiTailOffset + m_uiCurrentLength;
	if (m_uiHeadOffset >= m_uiTargetLength)
	{
		m_uiHeadOffset = m_uiTargetLength;
	}
	
	if (m_uiTailOffset + m_uiMaxLength >= m_uiTargetLength)
	{
		m_uiCurrentMaxLength = m_uiTargetLength - m_uiTailOffset;
		if (m_uiCurrentLength > m_uiCurrentMaxLength)
		{
			m_uiCurrentLength = m_uiCurrentMaxLength;
		}
	}
	else
	{
		m_uiCurrentMaxLength = m_uiMaxLength;
	}

	return true;
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
