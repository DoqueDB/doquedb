// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// JapaneseBlocker1.cpp
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
#include "FullText2/JapaneseBlocker1.h"

#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::JapaneseBlocker1::JapaneseBlocker1 -- コンストラクタ
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
JapaneseBlocker1::JapaneseBlocker1()
	: JapaneseBlocker()
{
}

//
//	FUNCTION public
//	FullText2::JapaneseBlocker1::~JapaneseBlocker1 -- デストラクタ
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
JapaneseBlocker1::~JapaneseBlocker1()
{
}

//
//	FUNCTION public
//	FullText2::JapaneseBlocker1::set
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
JapaneseBlocker1::set(const ModUnicodeString& cTarget_,
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

	// 先頭の文字種を確認する
	m_uiTailBlock = getBlock(m_pTarget[m_uiTailOffset]);
	m_uiHeadBlock = m_uiTailBlock;
	getBlockSize(m_uiTailBlock, m_uiMinLength, m_uiMaxLength);
	m_bPairValid = false;
	m_bNewBlock = true;
	m_uiCurrentLength = (m_bSearch) ? m_uiMaxLength : m_uiMinLength;
	m_uiCurrentMaxLength = 0;

	// すべてが同一文字種かどうか
	bool change = false;

	// 先頭位置を進めておく
	
	if (m_uiCurrentLength > m_uiTargetLength)
	{
		// 長さが足りない場合
		while (++m_uiHeadOffset < m_uiTargetLength)
		{
			m_uiHeadBlock = getBlock(m_pTarget[m_uiHeadOffset]);
			if (m_uiHeadBlock != m_uiTailBlock)
			{
				// 末尾に達する前に文字種が変わった
				m_uiCurrentLength = m_uiHeadOffset;
				m_uiCurrentMaxLength = m_uiHeadOffset;
				change = true;
				break;
			}
		}
		if (change == false)
		{
			// 末尾までが同一文字種だった場合
			m_uiCurrentMaxLength = m_uiTargetLength;
			
			if (m_bSearch == true)
			{
				if (m_uiMinLength > m_uiTargetLength)
				{
					// MIN よりも検索文字列が短ければショートワード処理
					// ショートワードになるのは、ターゲットの文字列が
					// MIN よりも短い場合のみ
					// 文字種の切れ目で MIN より短くても
					// ショートワードにはならない
					// ターゲットの文字列のすべてが同一文字種で、かつ、
					// MIN より短い場合のみショートとなる
					//
					// 例) DUAL:JAP:ALL:2 で '技M室' をトークナイズすると、
					// '技','M','室'となるが、'室'はショートではない
					// それは、文字種の変わり目は文字種跨ぎ処理で、
					// 1文字から切り出しているため
					// '技M室付' は '技','M','室','室付','付'と切り出され、
					// 文字種が変わった先頭の '室' は必ず切り出される
					
					m_bShortWord = true;
				}
				m_uiCurrentLength = m_uiCurrentMaxLength;
			}
		}
	}
	else
	{
		while (++m_uiHeadOffset < m_uiCurrentLength)
		{
			m_uiHeadBlock = getBlock(m_pTarget[m_uiHeadOffset]);
			if (m_uiHeadBlock != m_uiTailBlock)
			{
				// 現在長に達する前に文字種が変わった
				m_uiCurrentLength = m_uiHeadOffset;
				m_uiCurrentMaxLength = m_uiHeadOffset;
				change = true;
				break;
			}
		}
		if (change == false)
		{
			// 現在長までが同一文字種だった場合
			// 現在最大長をとりあえず最大長にしておく
			
			m_uiCurrentMaxLength = m_uiMaxLength;
			ModSize tmpOffset = m_uiHeadOffset;
			while (tmpOffset < m_uiMaxLength)
			{
				if (tmpOffset == m_uiTargetLength ||
					getBlock(m_pTarget[tmpOffset]) != m_uiTailBlock)
				{
					// 最大長に達する前に文字種が変わる
					m_uiCurrentMaxLength = tmpOffset;
					break;
				}
				++tmpOffset;
			}
		}
	}

	if (m_uiCurrentMaxLength == 1 &&
		checkPair(m_uiTailBlock, m_uiHeadBlock) == true &&
		m_uiTargetLength > 1)
	{
		m_uiCurrentMaxLength = 2;
		m_bPairValid = true;
	}
}

//
//	FUNCTION public
//	FullText2::JapaneseBlocker1::yield -- トークンの取得
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
JapaneseBlocker1::yield(ModUnicodeString& cToken_,
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
		m_bNewBlock = false;
		return true;
	}
	
	if (m_bSearch && m_bNewBlock == false &&
		m_uiCurrentLength < m_uiMaxLength &&
		m_uiHeadOffset == m_uiTargetLength)
	{
		// 検索時モードで、
		// 新しい文字種ではなく、
		// 現在のトークンの長さが最大長未満であり、
		// 末尾のオフセットがターゲット長に達していたら、終了
		
		return false;
	}

	cToken_.allocateCopy(m_pTarget + m_uiTailOffset, m_uiCurrentLength);
	uiOffset_ = m_uiTailOffset;
	
	if (m_uiCurrentLength == m_uiCurrentMaxLength &&
		m_uiTailBlock != m_uiHeadBlock)
	{
		// 最大長になっているので、文字種が変わる
		
		if (m_bSearch == false)
		{
			++m_uiTailOffset;
		}
		else
		{
			// ※ query モードで異文字種組を利用するにはここを修正する
			//    必要がある
			if (m_bPairValid == true)
			{
				++m_uiTailOffset;
			}
			else
			{
				m_uiTailOffset += m_uiCurrentLength;
			}
		}
	}
	else
	{
		++m_uiTailOffset;
	}
	
	m_bNewBlock = false;
	ModSize tmpBlock = getBlock(m_pTarget[m_uiTailOffset]);
	if (m_uiTailBlock != tmpBlock)
	{
		m_uiTailBlock = tmpBlock;
		m_bNewBlock = true;
	}
	getBlockSize(m_uiTailBlock, m_uiMinLength, m_uiMaxLength);
		
	if (m_uiTailOffset >= m_uiTargetLength)
	{
		// 末尾に達した -- 現在のトークンは出力する
		m_uiCurrentLength = 0;
		return true;
	}
	
	m_bPairValid = false;

 retry:
	// 文字種の変り目は 1 にしないといけない
	m_uiCurrentLength = (m_bSearch == false) ?
		((m_bNewBlock == true) ? 1 : m_uiMinLength) : m_uiMaxLength;

	m_uiHeadOffset = m_uiTailOffset;
	bool change = false;

	// 先頭位置を進めておく
	if (m_uiTailOffset + m_uiCurrentLength > m_uiTargetLength)
	{
		// 長さが足りない場合
		while (++m_uiHeadOffset < m_uiTargetLength)
		{
			m_uiHeadBlock = getBlock(m_pTarget[m_uiHeadOffset]);
			if (m_uiHeadBlock != m_uiTailBlock)
			{
				// 末尾に達する前に文字種が変わった
				m_uiCurrentLength = m_uiHeadOffset - m_uiTailOffset;
				m_uiCurrentMaxLength = m_uiHeadOffset - m_uiTailOffset;
				change = true;
				break;
			}
		}
		if (change == false)
		{
			// 末尾までが同一文字種だった場合
			m_uiCurrentMaxLength = m_uiTargetLength - m_uiTailOffset;
			m_uiCurrentLength = m_uiCurrentMaxLength;
		}
	}
	else
	{
		while (++m_uiHeadOffset < m_uiTailOffset + m_uiCurrentLength)
		{
			m_uiHeadBlock = getBlock(m_pTarget[m_uiHeadOffset]);
			if (m_uiHeadBlock != m_uiTailBlock)
			{
				// 現在長に達する前に文字種が変わった
				m_uiCurrentLength = m_uiHeadOffset - m_uiTailOffset;
				m_uiCurrentMaxLength = m_uiHeadOffset - m_uiTailOffset;
				
				if (m_bSearch && m_bNewBlock == false)
				{
					// 同一文字種で短いものなので、検索時は不要
					// ただし、異文字種ペアをとる場合にはさらに修正が必要
					
					m_uiTailOffset = m_uiHeadOffset;
					ModSize tmpBlock = getBlock(m_pTarget[m_uiTailOffset]);
					if (m_uiTailBlock != tmpBlock)
					{
						m_uiTailBlock = tmpBlock;
						m_bNewBlock = true;
					}
					getBlockSize(m_uiTailBlock, m_uiMinLength, m_uiMaxLength);
					m_bNewBlock = true;
					
					goto retry;
				}
				change = true;
				break;
			}
		}
		if (change == false)
		{
			// 現在長までが同一文字種だった場合
			// 現在最大長をとりあえず最大長にしておく
			
			m_uiCurrentMaxLength = m_uiMaxLength;
			ModSize tmpOffset = m_uiHeadOffset;
			// m_uiHeadOffset は文字種のチェックをしていない
			while (tmpOffset < m_uiTailOffset + m_uiMaxLength)
			{
				if (tmpOffset == m_uiTargetLength ||
					getBlock(m_pTarget[tmpOffset]) != m_uiTailBlock)
				{
					// 最大長に達する前に文字種が変わる
					m_uiCurrentMaxLength = tmpOffset - m_uiTailOffset;
					break;
				}
				++tmpOffset;
			} // ここでチェックした文字種はあとでチェックし直すので無駄
		}
	}

	if (m_uiCurrentMaxLength == 1 &&
		checkPair(getBlock(m_pTarget[m_uiTailOffset]),
				  getBlock(m_pTarget[m_uiHeadOffset])) == true &&
		m_uiHeadOffset < m_uiTargetLength)
	{
		m_uiCurrentMaxLength = 2;
		m_bPairValid = true;
	}
	
	return true;
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
