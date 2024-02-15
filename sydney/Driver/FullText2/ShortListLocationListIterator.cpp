// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortListLocationListIterator.cpp --
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
#include "FullText2/ShortListLocationListIterator.h"
#include "FullText2/ShortListIterator.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::ShortListLocationListIterator::ShortListLocationListIterator
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::LocationListManager* pListManager_
//		位置情報管理クラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ShortListLocationListIterator::
ShortListLocationListIterator(LocationListManager* pListManager_)
	: LocationListIterator(pListManager_)
{
}

//
//	FUNCTION public
//	FullText2::ShortListLocationListIterator::~ShortListLocationListIterator
//		-- デストラクタ
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
//	なし
//
ShortListLocationListIterator::~ShortListLocationListIterator()
{
}

//
//	FUNCTION public
//	FullText2::ShortListLocationListIterator::initialize -- 初期化する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32* pHeadAddress_
//		先頭アドレス
//	ModSize uiLength_
//		検索語長
//	ModSize uiFrequency_
//		文書内頻度
//	ModSize uiStartOffset_
//		開始ビットオフセット
//	ModSize uiEndOffset_
//		終端ビットオフセット
//	ModInvertedCoder* pCoder_
//		圧縮器
//	ModUint32 uiCurrentLocation_
//		頻度が1の時の位置情報
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
ShortListLocationListIterator::initialize(
	ModUInt32* pHeadAddress_,
	ModSize uiLength_,
	ModSize uiFrequency_,
	ModSize uiStartOffset_,
	ModSize uiEndOffset_,
	ModInvertedCoder* pCoder_,
	ModUInt32 uiCurrentLocation_)
{
	m_pHeadAddress = pHeadAddress_;
	m_uiLength = uiLength_;
	m_uiFrequency = uiFrequency_;
	m_uiStartOffset = uiStartOffset_;
	m_uiEndOffset = uiEndOffset_;
	m_pCoder = pCoder_;
	m_uiCurrentLocation = uiCurrentLocation_;

	resetImpl();
}

//
//	FUNCTION public
//	FullText2::ShortListLocationListIterator::find -- 位置情報を検索する
//
//	NOTES
//	Unary Coderの場合のみこの関数はModTrueを返す可能性がある
//
//	ARGUMENTS
//	ModSize uiLocation_
//		検索する位置情報
//
//	RETURN
//	bool
//		存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
ShortListLocationListIterator::find(ModSize uiLocation_, int& length_)
{
	length_ = static_cast<int>(m_uiLength);
	if (m_uiFrequency == 1)
	{
		return (m_uiCurrentLocation == uiLocation_) ? true : false;
	}
	if (m_pCoder->find(uiLocation_, m_pHeadAddress,
					   m_uiEndOffset, m_uiStartOffset) == ModTrue)
	{
		m_uiNextOffset = m_uiStartOffset + uiLocation_;
		m_uiCurrentLocation = uiLocation_;
		return true;
	}
	return false;
}

//
//	FUNCTION public
//	FullText2::ShortListLocationListIterator::lowerBound -- 下限検索
//
//	NOTES
//	下限検索であるが、カーソルは単純に大きな方に移動していくのみである
//	そのため、現在値より小さい location_ を与えても正しい結果は得られない
//	しかし、転置の検索ではそのほうが都合がいいのでそうしている
//
//	ARGUMENTS
//	ModSize location_
//		検索対象の位置
//	ModSize& maxLocation_
//		有効な最大位置情報
//
//	RETURN
//	ModSize
//		ヒットした場合は位置データ、存在しない場合はUndefinedLocation
//
//	EXCEPTIONS
//
ModSize
ShortListLocationListIterator::lowerBoundImpl(ModSize location_,
											  ModSize& maxLocation_)
{
	maxLocation_ = m_uiCurrentLocation;
	if (location_ <= m_uiCurrentLocation)
	{
		if (m_uiFrequency == 1)
		{
			// 頻度が１の場合、m_uiCurrentLocation に値が設定されているので、
			// nextImpl() に行かないように、ここで終了条件を設定する

			m_uiNextOffset = m_uiEndOffset;
		}
			
		return m_uiCurrentLocation;
	}

	ModSize loc;
	while (m_uiNextOffset < m_uiEndOffset) {
		loc = nextImpl();
		if (loc >= location_) {
			return loc;
		}
		maxLocation_ = loc;
	}
	return UndefinedLocation;
}

//
//	FUNCTION public
//	FullText2::ShortListLocationListIterator::nextImpl -- 次へ
//
//	NOTES
//
//	ARGUMENTS
//	ModSize
//		次の位置情報。終端に達した場合は UndefinedLocation
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ModSize
ShortListLocationListIterator::nextImpl()
{
	if (isEndImpl())
		return UndefinedLocation;

	if (m_uiFrequency == 1)
	{
		m_uiNextOffset = m_uiEndOffset;
	}

	if (m_uiNextOffset < m_uiEndOffset)
	{
		ModSize locationGap;
		if (m_pCoder->get(locationGap, m_pHeadAddress,
						  m_uiEndOffset, m_uiNextOffset) == ModFalse)
		{
			m_uiNextOffset = m_uiEndOffset;
			return UndefinedLocation;
		}
		m_uiCurrentLocation += locationGap;
	}

	return m_uiCurrentLocation;
}

//
//	FUNCTION private
//	FullText2::ShortListLocationListIterator::resetImpl -- 先頭へ
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
void
ShortListLocationListIterator::resetImpl()
{
	m_uiNextOffset = m_uiStartOffset;
	
	if (m_uiFrequency != 1)
		
		// 差分しか格納されていないので、0に初期化する
		
		m_uiCurrentLocation = 0;
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
