// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// JapaneseBlocker2.cpp
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
#include "FullText2/JapaneseBlocker2.h"

#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::JapaneseBlocker2::JapaneseBlocker2 -- コンストラクタ
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
JapaneseBlocker2::JapaneseBlocker2()
	: JapaneseBlocker()
{
}

//
//	FUNCTION public
//	FullText2::JapaneseBlocker2::~JapaneseBlocker2 -- デストラクタ
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
JapaneseBlocker2::~JapaneseBlocker2()
{
}

//
//	FUNCTION public
//	FullText2::JapaneseBlocker2::set
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
JapaneseBlocker2::set(const ModUnicodeString& cTarget_,
					  bool bSearch_)
{
	// メンバー変数に引数を設定する
	m_bSearch = bSearch_;
	m_pTarget = cTarget_;
	m_uiTargetLength = cTarget_.getLength();

	// トークナイズに必要な変数を初期化する
	m_uiTailOffset = 0;
	m_uiHeadOffset = 0;

	if (m_uiTargetLength == 0)
	{
		// 空文字列なので終わり
		return;
	}

	// 次のための処理
	nextProcess();
}

//
//	FUNCTION public
//	FullText2::JapaneseBlocker2::yield -- トークンの取得
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
JapaneseBlocker2::yield(ModUnicodeString& cToken_,
						ModSize& uiOffset_,
						bool& isShort_,
						ModSize& uiMinLength_)
{
	isShort_ = false;
	
	if (m_uiTailOffset >= m_uiTargetLength)
		return false;

	if (m_bSearch &&
		(m_uiTailOffset + m_uiCurrentLength) == m_uiTargetLength &&
		m_uiCurrentLength < m_uiMinLength)
	{
		// 検索モードで、
		// 最後の文字種で、かつ、
		// 最小長より短い場合は、ショートワード

		cToken_.allocateCopy(m_pTarget + m_uiTailOffset, m_uiCurrentLength);
		uiOffset_ = m_uiTailOffset;
		isShort_ = true;
		uiMinLength_ = m_uiMinLength;
		m_uiTailOffset += m_uiCurrentLength;
		return true;
	}

	if (m_uiCurrentLength < m_uiCurrentMaxLength)
	{
		cToken_.allocateCopy(m_pTarget + m_uiTailOffset, m_uiCurrentLength);
		++m_uiCurrentLength;
		uiOffset_ = m_uiTailOffset;
		return true;
	}

	if (m_uiCurrentLength == m_uiCurrentMaxLength)
	{
		cToken_.allocateCopy(m_pTarget + m_uiTailOffset, m_uiCurrentLength);
		uiOffset_ = m_uiTailOffset;
	}

	// 次の準備をする

	if (m_bSearch &&
		m_uiHeadOffset == (m_uiTailOffset + m_uiCurrentLength))
	{
		// 検索モードで、かつ、
		// 現在のトークンで m_uiHeadOffset に達する場合には、
		// 次の文字種からスタートする
		
		m_uiTailOffset = m_uiHeadOffset;
	}
	else
	{
		// 次の文字へ
		
		++m_uiTailOffset;
	}

	// 次ためのの処理
	nextProcess();

	return true;
}

//
//	FUNCTION private
//	FullText2::JapaneseBlocker2::nextProcess
//		-- 次ための処理を行う
//
//	NOTES
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
JapaneseBlocker2::nextProcess()
{
	//
	//	m_bSearch == false で DUAL:JAP:ALL:1:2 KAN:HIR の場合
	//
	//          m_uiHeadOffset = 4
	//         ↓
	//  環境への取り組み
	// ↑
	//	m_uiTailOffset = 0
	//
	//	m_uiCurrentLength = 1
	//	m_uiCurrentMaxLength = 2
	//
	//	となる
	//
	
	if (m_uiTailOffset >= m_uiTargetLength)
		return;
	
	ModSize tailBlock = getBlock(m_pTarget[m_uiTailOffset]);
	getBlockSize(tailBlock, m_uiMinLength, m_uiMaxLength);
	m_uiCurrentLength = (m_bSearch) ? m_uiMaxLength : m_uiMinLength;
	m_uiCurrentMaxLength = 0;

	if (m_uiTailOffset >= m_uiHeadOffset)
	{
		// 次の文字種
			
		m_uiHeadOffset = m_uiTailOffset;
		
		// 与えられた文字列の末尾に達するか、
		// m_uiMaxLength まで同じ文字種か、
		// 文字種の変わり目まで確認し、m_uiHeadOffsetとする

		ModSize prev = tailBlock;	// 先頭の文字種
		while (++m_uiHeadOffset < m_uiTargetLength)
		{
			// 先頭位置の文字種を得る
			ModSize next = getBlock(m_pTarget[m_uiHeadOffset]);
		
			if (prev != next &&	checkPair(prev, next) == false)
			{
				// 文字種が変わっており、かつペアではないので、ここまで
				break;
			}

			prev = next;
		}
	}

	// 現時点の最大長
	m_uiCurrentMaxLength =
		((m_uiHeadOffset - m_uiTailOffset) < m_uiMaxLength) ?
		(m_uiHeadOffset - m_uiTailOffset) : m_uiMaxLength;
	if (m_uiCurrentLength > m_uiCurrentMaxLength)
		m_uiCurrentLength = m_uiCurrentMaxLength;
}	

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
