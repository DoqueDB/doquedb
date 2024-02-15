// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleBaseListIterator.cpp --
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
#include "SyDynamicCast.h"
#include "FullText2/MiddleBaseListIterator.h"
#include "FullText2/MiddleBaseList.h"
#include "FullText2/OverflowFile.h"
#include "FullText2/InvertedUnit.h"
#include "FullText2/MiddleListLocationListIterator.h"
#include "FullText2/FakeError.h"

#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::MiddleBaseListIterator::MiddleBaseListIterator -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::MiddleBaseList& cMiddleBaseList_
//		ミドルリスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
MiddleBaseListIterator::MiddleBaseListIterator(MiddleBaseList& cMiddleBaseList_)
	: m_cInvertedList(cMiddleBaseList_),
	  m_cInvertedUnit(cMiddleBaseList_.getInvertedUnit()),
	  m_pDirBlock(0), m_iDirBlockPosition(0), m_uiIDBlockPosition(0),
	  m_iCurrentPosition(-1), m_uiPrevID(0), m_uiCurrentID(0),
	  m_uiCurrentOffset(0), m_uiNextOffset(0), 
	  m_bSynchronized(false)
{
	// オーバーフローファイル
	m_pOverflowFile = cMiddleBaseList_.getInvertedUnit().getOverflowFile();
	// ページを得る(参照カウンターを保持するため)
	m_pLeafPage = cMiddleBaseList_.getLeafPage();
	// LeafPageのエリア
	m_pArea = cMiddleBaseList_.getArea();

	// 初期設定 (resetImpl の一部)
	m_pDirBlock = m_pArea->getDirBlock();
	if (m_pArea->getDocumentCount() == 0)
	{
		m_uiCurrentID = UndefinedDocumentID;
	}
}

//
//	FUNCTION public
//	FullText2::MiddleBaseListIterator::~MiddleBaseListIterator -- デストラクタ
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
MiddleBaseListIterator::~MiddleBaseListIterator()
{
}

//
//	FUNCTION public
//	FullText2::MiddleBaseListIterator::getLength -- 索引単位の長さを得る
//
//	NOTES
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	int
//		索引単位の長さ
//
//	EXCEPTIONS
//
int
MiddleBaseListIterator::getLength()
{
	return (m_cInvertedList.isWordIndex() ?
			1 : static_cast<int>(m_pArea->getKeyLength()));
}

//
//	FUNCTION public
//	FullText2::MiddleBaseListIterator::find -- 文書IDを検索する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::DocumentID uiDocumentID_
//		検索する文書ID
//	bool bUndo_
//		削除のUNDO処理中かどうか
//
//	RETURN
//	bool
//		検索でヒットした場合はtruee、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
MiddleBaseListIterator::find(DocumentID uiDocumentID_, bool bUndo_)
{
	if (lowerBound(uiDocumentID_, bUndo_) == uiDocumentID_)
	{
		return true;
	}
	return false;
}

//
//	FUNCTION public
//	FullText2::MiddleBaseListIterator::lowerBound -- 文書IDをlower_bound検索する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::DocumentID uiDocumentID_
//		検索する文書ID
//	bool bUndo_
//		削除のUNDO処理中かどうか
//
//	RETURN
//	ModBoolean
//		検索でヒットした場合はModTrue、それ以外の場合はModFalse
//
//	EXCEPTIONS
//
DocumentID
MiddleBaseListIterator::lowerBound(DocumentID uiDocumentID_, bool bUndo_)
{
	if (bUndo_ == false &&
		(m_pArea->getDocumentCount() == 0 ||
		 (m_pArea->getLastDocumentID() != 0 &&
		  m_pArea->getLastDocumentID() < uiDocumentID_)))
	{
		m_uiCurrentID = UndefinedDocumentID;
		return m_uiCurrentID;
	}

	if (bUndo_ == true
		|| m_cIdBlock.isInvalid() == true
		|| m_cIdBlock.getFirstDocumentID() > uiDocumentID_
		|| m_uiLastDocumentID == 1)
	{
		// UNDO処理中か、まだIDブロックが割り当てられていないか、
		// 現在の文書IDよりも小さい場合はDIRブロックから2分探索

		m_cLocBlock = OverflowPage::LocBlock();
		m_bSynchronized = false;

		LeafPage::DirBlock* pDirBlock = 0;

		if (bUndo_ == true)
		{
			// IDブロックの先頭文書IDをUndoするのかもしれない
			ModUInt32 ID = getExpungeFirstDocumentID(uiDocumentID_);
			if (ID != UndefinedDocumentID)
			{
				// 置き換えちゃう
				//	-> マージの時はUndoは来ないので、
				//	   ここに来るのは1件削除のときだけ
				uiDocumentID_ = ID;
			}
		}

		// とりあえず最終IDブロックを設定する
		m_cIdBlock = m_pArea->getIDBlock();

		// 最終IDブロック外か？
		if (m_cIdBlock.getFirstDocumentID() > uiDocumentID_)
		{
			//
			//	DIRブロックを2分探索
			//
			LeafPage::DirBlock cDirBlock;
			cDirBlock.m_uiDocumentID = uiDocumentID_;
			int count = m_pArea->getDirBlockCount();
			pDirBlock = ModUpperBound(m_pDirBlock,
									  m_pDirBlock + count,
									  cDirBlock,
									  LeafPage::DirBlock::Less());
			if (pDirBlock != m_pDirBlock)
				pDirBlock--;

			while (bUndo_ == false && pDirBlock->isExpunge())
			{
				if (pDirBlock == m_pDirBlock + count)
				{
					// 見つからなかった -> 最終IDブロック
					break;
				}
				pDirBlock++;
			}

			if (pDirBlock != m_pDirBlock + count)
			{
				m_iDirBlockPosition = static_cast<int>(pDirBlock - m_pDirBlock);

				//
				//	IDページをアタッチし、ページ内のIDブロックを検索
				//
				if (m_pIdPage == 0 ||
					m_pIdPage->getID() != pDirBlock->getPageID())
				{
					m_pIdPage = m_pOverflowFile->attachPage(
						pDirBlock->getPageID());
				}
				m_cIdBlock = m_pIdPage->lowerBoundIDBlock(
					uiDocumentID_, m_uiIDBlockPosition, bUndo_);
			}
		}

		if (bUndo_ == false && m_cIdBlock.isExpunge())
		{
			// IDブロックが削除されている
			m_uiCurrentID = UndefinedDocumentID;
			return m_uiCurrentID;
		}

		// 先頭文書IDを設定する
		setFirstDocumentID(m_cIdBlock);
	}
	else if (m_uiLastDocumentID <= uiDocumentID_)
	{
		// UNDO処理でもなく、すでにIDブロックが割り当てられていて、
		// 現在のIDブロックの最大文書IDよりも大きい文書IDの場合は、
		// 2分探索せず、存在するIDブロックをシーケンシャルに探す

		m_cLocBlock = OverflowPage::LocBlock();
		m_bSynchronized = false;

		while (m_uiLastDocumentID <= uiDocumentID_)
		{
			if (nextIdBlock() == false)
			{
				m_uiCurrentID = UndefinedDocumentID;
				return m_uiCurrentID;
			}
		}
	}
	else if (m_uiCurrentID > uiDocumentID_)
	{
		// 現在のIDブロック中にあるが、現在位置より前
		
		m_cLocBlock = OverflowPage::LocBlock();
		m_bSynchronized = false;

		setFirstDocumentID(m_cIdBlock);
	}

	//
	//	IDBlockの中を検索する
	//

	while (m_uiCurrentID < uiDocumentID_)
	{
		// 次へ
		if (nextImpl(bUndo_) == UndefinedDocumentID)
			break;
	}

	return m_uiCurrentID;
}

//
//	FUNCTION public
//	FullText2::MiddleBaseListIterator::enterDeleteIdBlock
//		-- 不用なIDブロックを登録する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiFirstDocumentID_
//		不用なIDブロックの先頭文書ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MiddleBaseListIterator::enterDeleteIdBlock(ModUInt32 uiFirstDocumentID_)
{
	m_cInvertedUnit.enterDeleteIdBlock(m_cInvertedList.getKey(),
									   uiFirstDocumentID_);
}

//
//	FUNCTION public
//	FullText2::MiddleBaseListIterator::enterExpungeFirstDocumentID
//		-- 先頭文書ID削除のログを登録する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiOldDocumentID_
//		削除前の文書ID
//	ModUInt32 uiNewDocumentID_
//		削除後の文書ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MiddleBaseListIterator::enterExpungeFirstDocumentID(ModUInt32 uiOldDocumentID_,
													ModUInt32 uiNewDocumentID_)
{
	m_cInvertedUnit.enterExpungeFirstDocumentID(m_cInvertedList.getKey(),
												uiOldDocumentID_,
												uiNewDocumentID_);
}

//
//	FUNCTION public
//	FullText2::MiddleBaseListIterator::getExpungeFirstDocumentID
//		-- 先頭文書IDが削除されたものか
//
//	NOTES
//
//	ARUGMENTS
//	ModUInt32 uiOldDocumentID_
//		削除前の文書ID
//
//	RETURN
//	ModUInt32
//		削除後の文書ID、存在しない場合はUndefinedDocumentID
//
//	EXCEPTIONS
//
ModUInt32
MiddleBaseListIterator::getExpungeFirstDocumentID(ModUInt32 uiOldDocumentID_)
{
	return m_cInvertedUnit.getExpungeFirstDocumentID(m_cInvertedList.getKey(),
													 uiOldDocumentID_);
}

//
//	FUNCTION protected
//	FullText2::MiddleBaseListIterator::resetImpl -- 初期化する
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
MiddleBaseListIterator::resetImpl()
{
	m_pDirBlock = m_pArea->getDirBlock();

	m_iDirBlockPosition = 0;	// 現在のDIRブロック位置
	m_uiIDBlockPosition = 0;	// IDページ中の現在のIDブロック位置
	m_pIdPage = 0;				// 現在のIDページ
	m_uiCurrentID = 0;			// 現在のID

	m_cLocBlock = OverflowPage::LocBlock();
	m_cIdBlock = OverflowPage::IDBlock();
	m_bSynchronized = false;

	if (m_pArea->getDocumentCount() == 0)
	{
		m_uiCurrentID = UndefinedDocumentID;
	}
}

//
//	FUNCTION protected
//	FullText2::MiddleBaseListIterator::nextImpl -- 次へ
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
DocumentID
MiddleBaseListIterator::nextImpl(bool bUndo_)
{
	if (isEndImpl() == ModTrue) return m_uiCurrentID;

	m_bSynchronized = false;

	if (m_cIdBlock.isInvalid() == true)
	{
		// 初めてのnextImpl
		
		m_uiCurrentID = UndefinedDocumentID;
		nextIdBlock();
		return m_uiCurrentID;
	}

	m_uiPrevID = m_uiCurrentID;
	m_uiCurrentOffset = m_uiNextOffset;
	m_iCurrentPosition++;

	// 文書IDを1つ読む
	m_uiCurrentID
		= m_cInvertedList.readDocumentID(m_uiPrevID,
										 m_cIdBlock.getBuffer(),
										 m_cIdBlock.getDataSize(),
										 m_uiNextOffset);
	if (m_uiCurrentID == UndefinedDocumentID && bUndo_ == false)
	{
		// 終端に達したので、次のブロックを読む
		nextIdBlock();
		m_cLocBlock = OverflowPage::LocBlock();
	}

	return m_uiCurrentID;
}

//
//	FUNCTION protected
//	FullText2::MiddleBaseListIterator::setFirstDocumentID
//		-- 先頭文書IDを設定する
//
//	NOTES
//
//	ARGUMENTS
//	OverflowPage::IDBlock& cIdBlock_
//		IDブロック
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MiddleBaseListIterator::setFirstDocumentID(OverflowPage::IDBlock& cIdBlock_)
{
	m_uiPrevID = 0;				// 直前の文書ID
	m_uiNextOffset = 0;			// IDブロック中の次のビット位置
	m_uiCurrentOffset = 0;		// IDブロック中の現在のビット位置
	m_iCurrentPosition = 0;		// IDブロック中の位置

	m_uiCurrentID = cIdBlock_.getFirstDocumentID();

	if (cIdBlock_ == m_pArea->getHeadAddress())
	{
		// 最終IDブロック
		m_uiLastDocumentID = m_pArea->getLastDocumentID() + 1;
	}
	else
	{
		m_uiLastDocumentID = m_pIdPage->getNextDocumentID(m_uiIDBlockPosition);
		if (m_uiLastDocumentID == UndefinedDocumentID)
		{
			if (m_iDirBlockPosition + 1 >= m_pArea->getDirBlockCount())
			{
				// 最終IDブロックの先頭文書ID
				m_uiLastDocumentID = m_pArea->getIDBlock().getFirstDocumentID();
			}
			else
			{
				// 次のDIRブロックの先頭文書ID
				m_uiLastDocumentID
					= m_pDirBlock[m_iDirBlockPosition+1].getDocumentID();
			}
		}
	}
}

//
//	FUNCTION protected
//	FullText2::MiddleBaseListIterator::nextIdBlock -- 次のIDブロックへ移動する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		次のIDブロックが得られた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
MiddleBaseListIterator::nextIdBlock()
{
	m_bSynchronized = false;

	if (m_cIdBlock == m_pArea->getHeadAddress())
	{
		// 最終IDブロックなので終了
		m_cIdBlock = OverflowPage::IDBlock();
		return false;
	}

	while (true)
	{
		if (m_pIdPage == 0)
		{
			// 初めて
			
			if (m_pArea->getDirBlockCount() == 0)
			{
				// DIRブロックがないので、最終IDブロック
				m_cIdBlock = m_pArea->getIDBlock();
				break;
			}
			
			m_pIdPage = m_pOverflowFile->attachPage(
				m_pDirBlock[m_iDirBlockPosition].getPageID());
			m_cIdBlock = m_pIdPage->getIDBlock(m_uiIDBlockPosition);
		}
		else
		{
			m_uiIDBlockPosition++;
			if (m_uiIDBlockPosition >= m_pIdPage->getIDBlockCount())
			{
				// もうこのページにはIDブロックはない
				m_iDirBlockPosition++;
				if (m_iDirBlockPosition >= m_pArea->getDirBlockCount())
				{
					// もうDIRブロックもない -> 最終IDブロック
					m_cIdBlock = m_pArea->getIDBlock();
					break;
				}
				m_pIdPage = m_pOverflowFile->attachPage(
					m_pDirBlock[m_iDirBlockPosition].getPageID());
				m_uiIDBlockPosition = 0;
			}

			m_cIdBlock = m_pIdPage->getIDBlock(m_uiIDBlockPosition);
		}

		if (m_cIdBlock.isExpunge() == false)
			break;
	}

	if (m_cIdBlock.isExpunge())
		return false;

	setFirstDocumentID(m_cIdBlock);

	return true;
}

//
//	FUNCTION protected
//	FullText2::MiddleBaseListIterator::expungeFirstDocumentID
//		-- 先頭文書IDを削除する
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
MiddleBaseListIterator::expungeFirstDocumentID()
{
	if (m_cIdBlock != m_pArea->getHeadAddress())
		m_pIdPage->dirty();

	// 次の文書IDを読む
	ModSize uiOffset = m_uiNextOffset;
	ModUInt32 uiNextID
		= m_cInvertedList.readDocumentID(m_uiCurrentID,
										 m_cIdBlock.getBuffer(),
										 m_cIdBlock.getDataSize(),
										 uiOffset);
	if (uiNextID == UndefinedDocumentID)
	{
		// １つも読めない -> もうこのIDブロックには1つもデータは入っていない
		m_cIdBlock.setExpunge();
		if (m_cIdBlock == m_pArea->getHeadAddress())
		{
			// 最終IDブロック
			enterDeleteIdBlock(m_cIdBlock.getFirstDocumentID());
		}
		else
		{
			// IDページ中のIDブロック
			if (m_pIdPage->enterDeleteIdBlock(*this) == true)
			{
				// このページにはもうIDブロックは存在しない
				m_pDirBlock[m_iDirBlockPosition].setExpunge();
			}
		}
		// 次のIDブロックへ
		nextIdBlock();
	}
	else
	{
		// 後ろを移動
		ModSize uiDstOffset = 0;
		ModSize uiSrcOffset = uiOffset;
		ModSize uiLength = m_cIdBlock.getDataSize() - uiOffset;

		InvertedList::moveBack(m_cIdBlock.getBuffer(), uiDstOffset,
								m_cIdBlock.getBuffer(), uiSrcOffset, uiLength);
		InvertedList::setOffBack(m_cIdBlock.getBuffer(), uiLength, uiOffset);

		if (m_cIdBlock == m_pArea->getHeadAddress())
			// 最終IDブロックなので文書IDオフセットを更新する
			m_pArea->setDocumentOffset(m_pArea->getDocumentOffset() - uiOffset);

		// 先頭文書ID
		m_cIdBlock.setFirstDocumentID(uiNextID);

		// ログを記録する
		enterExpungeFirstDocumentID(m_uiCurrentID, uiNextID);

		m_uiCurrentID = uiNextID;
	}
}

//
//	FUNCTION protected
//	FullText2::MiddleBaseListIterator::expungeDocumentID -- 文書IDを削除する
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
MiddleBaseListIterator::expungeDocumentID()
{
	// 次の文書IDを読む
	ModSize uiOffset = m_uiNextOffset;
	ModUInt32 uiNextID
		= m_cInvertedList.readDocumentID(m_uiCurrentID,
										 m_cIdBlock.getBuffer(),
										 m_cIdBlock.getDataSize(),
										 uiOffset);

	if (m_cIdBlock != m_pArea->getHeadAddress())
		m_pIdPage->dirty();

	if (uiNextID != UndefinedDocumentID)
	{
		// 差分を書き直す
		m_uiNextOffset = m_uiCurrentOffset;
		InvertedList::setOffBack(m_cIdBlock.getBuffer(),
								 m_uiCurrentOffset,
								 uiOffset - m_uiCurrentOffset);
		m_cInvertedList.writeDocumentID(m_uiPrevID,
										uiNextID,
										m_cIdBlock.getBuffer(),
										m_uiNextOffset);

		// 後ろを移動
		ModSize uiDstOffset = m_uiNextOffset;
		ModSize uiSrcOffset = uiOffset;
		ModSize uiLength = m_cIdBlock.getDataSize() - uiOffset;

		if (uiDstOffset != uiSrcOffset)
		{
			if (uiLength != 0)
			{
				InvertedList::moveBack(m_cIdBlock.getBuffer(),
									   uiDstOffset,
									   m_cIdBlock.getBuffer(),
									   uiSrcOffset,
									   uiLength);
			}
			InvertedList::setOffBack(m_cIdBlock.getBuffer(),
									 uiDstOffset + uiLength,
									 uiSrcOffset - uiDstOffset);
		}

		if (m_cIdBlock == m_pArea->getHeadAddress())
			// 最終IDブロックなので文書IDオフセットを更新する
			m_pArea->setDocumentOffset(
				m_pArea->getDocumentOffset() - (uiOffset - m_uiNextOffset));

		// 今の文書ID
		m_uiCurrentID = uiNextID;
	}
	else
	{
		// このIDブロックの最後の文書IDなので、
		// 書かれていたビットをクリアするのみ
		InvertedList::setOffBack(m_cIdBlock.getBuffer(),
								 m_uiCurrentOffset,
								 m_uiNextOffset - m_uiCurrentOffset);

		if (m_cIdBlock == m_pArea->getHeadAddress())
		{
			// 最終IDブロックなので文書IDオフセットを更新する
			m_pArea->setDocumentOffset(m_pArea->getDocumentOffset()
									   - (m_uiNextOffset - m_uiCurrentOffset));
			// 最終文書IDを更新する
			m_pArea->setLastDocumentID(m_uiPrevID);
		}

		// 次のIDブロックを設定する
		nextIdBlock();
	}
}

//
//	FUNCTION protected
//	FullText2::MiddleBaseListIterator::undoExpungeDocumentID
//		-- 文書IDの削除を取り消す
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		文書ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MiddleBaseListIterator::undoExpungeDocumentID(ModUInt32 uiDocumentID_)
{
	if (m_cIdBlock != m_pArea->getHeadAddress())
		m_pIdPage->dirty();

	if (m_cIdBlock.getFirstDocumentID() == uiDocumentID_)
	{
		// 先頭文書IDが削除されている
		m_cIdBlock.setFirstDocumentID(uiDocumentID_);
		if (m_cIdBlock != m_pArea->getHeadAddress())
		{
			// 削除フラグが立っているかもしれないので削除する
			m_pDirBlock[m_iDirBlockPosition].unsetExpunge();
		}
	}
	else if (m_cIdBlock.getFirstDocumentID() == m_uiCurrentID)
	{
		// 先頭文書IDを挿入する

		// ビット長
		ModSize length
			= m_cInvertedList.getCompressedBitLengthDocumentID(uiDocumentID_,
															   m_uiCurrentID);

		// 後ろを移動
		ModSize uiDstOffset = length;
		ModSize uiSrcOffset = m_uiCurrentOffset;
		ModSize uiLength = m_cIdBlock.getDataSize() - uiDstOffset;
		InvertedList::moveBack(m_cIdBlock.getBuffer(), uiDstOffset,
								m_cIdBlock.getBuffer(), uiSrcOffset, uiLength);
		InvertedList::setOffBack(m_cIdBlock.getBuffer(), uiSrcOffset,
								 uiDstOffset - uiSrcOffset);

		// 書き込む
		m_cInvertedList.writeDocumentID(uiDocumentID_,
										m_uiCurrentID,
										m_cIdBlock.getBuffer(),
										uiSrcOffset);

		// 先頭文書ID
		m_cIdBlock.setFirstDocumentID(uiDocumentID_);
	
		// ビットオフセット
		if (m_cIdBlock == m_pArea->getHeadAddress())
		{
			// 最終IDブロックなので、ビットオフセットを書き込む
			m_pArea->setDocumentOffset(m_pArea->getDocumentOffset() + uiLength);
		}
	}
	else if (m_uiCurrentID == UndefinedDocumentID)
	{
		// そのIDブロックの最後の文書ID

		// 書き込む
		ModSize uiOffset = m_uiCurrentOffset;
		m_cInvertedList.writeDocumentID(m_uiPrevID,
										uiDocumentID_,
										m_cIdBlock.getBuffer(),
										uiOffset);

		if (m_cIdBlock == m_pArea->getHeadAddress())
		{
			// 文書IDオフセット
			m_pArea->setDocumentOffset(uiOffset);
		}
	}
	else
	{
		// その他の文書IDを挿入する

		ModSize length
			= m_cInvertedList.getCompressedBitLengthDocumentID(m_uiPrevID,
															   uiDocumentID_);
		length +=
			m_cInvertedList.getCompressedBitLengthDocumentID(uiDocumentID_,
															 m_uiCurrentID);
		length -= m_uiNextOffset - m_uiCurrentOffset;

		// 後ろを移動する
		ModSize uiDstOffset = m_uiNextOffset + length;
		ModSize uiSrcOffset = m_uiNextOffset;
		ModSize uiLength = m_cIdBlock.getDataSize() - uiDstOffset;
		if (uiLength)
		{
			InvertedList::moveBack(m_cIdBlock.getBuffer(), uiDstOffset,
									m_cIdBlock.getBuffer(), uiSrcOffset,
								   uiLength);
		}
		InvertedList::setOffBack(m_cIdBlock.getBuffer(),
								 m_uiCurrentOffset,
								 uiDstOffset - m_uiCurrentOffset);

		// 書き込む
		ModSize uiOffset = m_uiCurrentOffset;
		m_cInvertedList.writeDocumentID(m_uiPrevID,
										uiDocumentID_,
										m_cIdBlock.getBuffer(),
										uiOffset);
		m_cInvertedList.writeDocumentID(uiDocumentID_,
										m_uiCurrentID,
										m_cIdBlock.getBuffer(),
										uiOffset);

		if (m_cIdBlock == m_pArea->getHeadAddress())
		{
			// 文書IDオフセット
			m_pArea->setDocumentOffset(m_pArea->getDocumentOffset() + length);
		}
	}

	if (m_pArea->getLastDocumentID() < uiDocumentID_)
	{
		// 最終文書IDを更新する
		m_pArea->setLastDocumentID(uiDocumentID_);
	}
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
