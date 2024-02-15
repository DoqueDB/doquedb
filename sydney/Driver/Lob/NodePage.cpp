// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NodePage.cpp --
// 
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
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
#include "Lob/NodePage.h"
#include "Lob/LobFile.h"

_SYDNEY_USING
_SYDNEY_LOB_USING

//
//	FUNCTION public
//	Lob::NodePage::NodePage -- コンストラクタ
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
NodePage::NodePage(LobFile& cFile_, PhysicalFile::Page* pPhysicalPage_)
	: BlockPage(cFile_, pPhysicalPage_), m_cLobFile(cFile_)
{
	m_pHeader = syd_reinterpret_cast<Header*>(getBuffer());
}

//
//	FUNCTION public
//	Lob::NodePage::~NodePage -- デストラクタ
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
NodePage::~NodePage()
{
}

//
//	FUNCTION public
//	Lob::NodePage::initialize -- 初期化する
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
NodePage::initialize()
{
	dirty();
	
	// ページタイプを設定
	setType(Type::Node);

	// ヘッダーを初期化
	m_pHeader->m_uiEntryCount = 0;
	m_pHeader->m_uiNextBlockPage = PhysicalFile::ConstValue::UndefinedPageID;
}

//
//	FUNCTION public
//	Lob::NodePage::allocateBlock -- 新しいブロックを得る
//
//	NOTES
//
//	ARGUMENTS
//	ModSize& uiPosition_
//		得られたブロックのオフセット
//
//	RETURN
//	Lob::BlockPage::Block*
//		得られたブロック。確保できなかった場合は0を返す
//
//	EXCEPTIONS
//
BlockPage::Block*
NodePage::allocateBlock(ModSize& uiPosition_)
{
	if (getFreeSize() < sizeof(Block))
		return 0;
	
	dirty();

	// 次のブロック位置を求める
	uiPosition_ = sizeof(Header) + sizeof(Block) * (m_pHeader->m_uiEntryCount);
	// エントリカウントを1つ増やす
	m_pHeader->m_uiEntryCount++;
	// ブロックを得る
	Block* pBlock = getBlock(uiPosition_);
								

	return pBlock;
}

//
//	FUNCTION public
//	Lob::NodePage::getUsedSize -- 使用サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		 使用サイズ
//
//	EXCEPTIONS
//
ModSize
NodePage::getUsedSize()
{
	return sizeof(Header) + sizeof(Block) * getHeader()->m_uiEntryCount;
}

//
//	Copyright (c) 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
