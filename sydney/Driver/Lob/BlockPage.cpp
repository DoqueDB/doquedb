// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BlockPage.cpp --
// 
// Copyright (c) 2003, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Lob";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Lob/BlockPage.h"
#include "Lob/LobFile.h"

_SYDNEY_USING
_SYDNEY_LOB_USING

namespace
{
	//
	//	VARIABLE local
	//
	ModUInt32 _EXPUNGE_MASK	= 0x80000000;
}

//
//	FUNCTION public
//	Lob::BlockPage::Block::setUsedPageNumber -- 使用バリューページ数を設定する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiUsedPageNumber_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BlockPage::Block::setUsedPageNumber(ModUInt32 uiUsedPageNumber_)
{
	m_uiUsedPageNumber &= _EXPUNGE_MASK;
	m_uiUsedPageNumber |= uiUsedPageNumber_;
}

//
//	FUNCTION pblic
//	Lob::BlockPage::Block::getUsedPageNumber -- 使用バリューページ数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		使用バリューページ数
//
//	EXCEPTIONS
//
ModUInt32
BlockPage::Block::getUsedPageNumber()
{
	return m_uiUsedPageNumber & ~_EXPUNGE_MASK;
}

//
//	FUNCTION public
//	Lob::BlockPage::Block::incrementUsedPageNumber
//		-- 使用バリューページ数を1つ増やす
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
BlockPage::Block::incrementUsedPageNumber()
{
	++m_uiUsedPageNumber;
}

//
//	FUNCTION public
//	Lob::BlockPage::Block::decrementUsedPageNumber
//		-- 使用バリューページ数を1つ減らす
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
BlockPage::Block::decrementUsedPageNumber()
{
	--m_uiUsedPageNumber;
}

//
//	FUNCTION public
//	Lob::BlockPage::Block::isExpunge -- 削除されているかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		削除されている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
BlockPage::Block::isExpunge()
{
	return m_uiUsedPageNumber & _EXPUNGE_MASK;
}

//
//	FUNCTION public
//	Lob::BlockPage::Block::setExpungeFlag -- 削除フラグを設定する
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
BlockPage::Block::setExpungeFlag()
{
	m_uiUsedPageNumber |= _EXPUNGE_MASK;
}

//
//	FUNCTION public
//	Lob::BlockPage::Block::unsetExpungeFlag -- 削除フラグを削除する
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
BlockPage::Block::unsetExpungeFlag()
{
	m_uiUsedPageNumber &= ~_EXPUNGE_MASK;
}

//
//	FUNCTION public
//	Lob::BlockPage::Block::clear -- データ参照情報をクリアする
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
BlockPage::Block::clear()
{
	m_uiDirPage = PhysicalFile::ConstValue::UndefinedPageID;
	m_uiLastPage = PhysicalFile::ConstValue::UndefinedPageID;
}

//
//	FUNCTION public
//	Lob::BlockPage::BlockPage -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Lob::LobFile& cFile_
//		ファイル
//	PhysicalFile::Page* pPhysicalPage_
//		物理ページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
BlockPage::BlockPage(LobFile& cFile_, PhysicalFile::Page* pPhysicalPage_)
	: Page(cFile_, pPhysicalPage_)
{
}

//
//	FUNCTION public
//	Lob::BlockPage::~BlockPage -- デストラクタ
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
BlockPage::~BlockPage()
{
}

//
//	FUNCTION public
//	Lob::BlockPage::getBlock -- 指定のブロックを得る
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiPosition_
//		先頭からのオフセット
//
//	RETURN
//	Block*
//		得られたBlockへのポインタ。存在しない場合は0
//
//	EXCEPTIONS
//
BlockPage::Block*
BlockPage::getBlock(ModSize uiPosition_)
{
	Type::Value eType = getType();
	if ((eType != Type::Top && eType != Type::Node)
		|| (((uiPosition_ - getHeaderSize()) % sizeof(Block)) != 0)
		|| (uiPosition_ >= (getHeaderSize() + sizeof(Block) * getEntryCount())))
		return 0;
	return syd_reinterpret_cast<Block*>(getBuffer() + uiPosition_);
}

//
//	FUNCTION public
//	Lob::TopPage::getNextBlock -- 次のブロックを得る
//
//	NOTES
//
//	ARGUMENTS
//	ModSize& uiPosition_
//		今のブロック。実行後は次のブロックへのポジション
//
//	RETURN
//	Lob::BlockPage::Block*
//		次のブロック。存在しない場合は0
//
//	EXCEPTIONS
//
BlockPage::Block*
BlockPage::getNextBlock(ModSize& uiPosition_)
{
	Block* pBlock = 0;

	ModSize uiHeaderSize = getHeaderSize();
	
	if (uiPosition_ < uiHeaderSize)
		uiPosition_ = uiHeaderSize;
	
	while (pBlock == 0)
	{
		if (uiPosition_	>= (uiHeaderSize + sizeof(Block) * getEntryCount()))
			// もうないので終わり
			break;
		
		Block* p = getBlock(uiPosition_);
		if (p->m_uiLastPage != PhysicalFile::ConstValue::UndefinedPageID)
			pBlock = p;

		// 次の位置を設定
		uiPosition_ += sizeof(Block);
	}

	return pBlock;
}

//
//	Copyright (c) 2003, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
