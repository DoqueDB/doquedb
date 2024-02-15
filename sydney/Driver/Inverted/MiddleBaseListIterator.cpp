// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleBaseListIterator.cpp --
// 
// Copyright (c) 2008, 2009, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Inverted/InvertedList.h"
#include "Inverted/InvertedUnit.h"
#include "Inverted/MiddleBaseList.h"
#include "Inverted/MiddleBaseListIterator.h"
#include "Inverted/MiddleListLocationListIterator.h"
#include "Inverted/OverflowFile.h"

#include "Common/Assert.h"
#include "Exception/Unexpected.h"

#include "ModInvertedLocationListIterator.h"
#include "ModInvertedSmartLocationList.h"

#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public
//	Inverted::MiddleBaseListIterator::MiddleBaseListIterator -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::MiddleBaseList& cMiddleBaseList_
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
	  m_pDirBlock(0), m_iDirBlockPosition(0), m_uiIDBlockPosition(0),
	  m_iCurrentPosition(0), m_uiPrevID(0), m_uiCurrentID(0),
	  m_uiCurrentOffset(0), m_uiNextOffset(0), m_bSynchronized(false)
{
	// オーバーフローファイル
	m_pOverflowFile = cMiddleBaseList_.getInvertedUnit().getOverflowFile();
	// LeafPageのエリア
	m_pArea = cMiddleBaseList_.getArea();
	// 先頭文書IDを設定する
	// setFirstDocumentID();
	// -> 関数内で純粋仮想関数setLocBlockByEmpty()を使うので派生クラスに移動
}

//
//	FUNCTION public
//	Inverted::MiddleBaseListIterator::~MiddleBaseListIterator -- デストラクタ
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
//	Inverted::MiddleBaseListIterator::next -- 次へ
//
//	NOTES
//
//	ARGUMENTS
//	bool bUndo_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MiddleBaseListIterator::next()
{
	// 引数ありのnextと分割し、
	// next2()の後のif文の条件を減らした。
	
	// if (isEnd() == ModTrue) return;
	if (m_uiCurrentID != UndefinedDocumentID)
	{
		// 終端ではないので次を取得する

		next2();

		// 新しい情報の確認
		// if (isEnd() == ModTrue)
		if (m_uiCurrentID == UndefinedDocumentID)
		{
			// 終端に達したので、次のブロックを読む
			nextIdBlock();
			// Locブロックはまだ設定されない。
			setLocBlockByEmpty();
		}
	}
}

//
//	FUNCTION public
//	Inverted::MiddleBaseListIterator::reset -- 先頭に戻る
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
MiddleBaseListIterator::reset()
{
	setFirstDocumentID();
	setLocBlockByEmpty();
	m_bSynchronized = false;
}

//
//	FUNCTION public
//	Inverted::MiddleBaseListIterator::find -- 文書IDを検索する
//
//	NOTES
//
//	ARGUMENTS
//	ModInvertedIterator::DocumentID uiDocumentID_
//		検索する文書ID
//	bool bUndo_
//		削除のUNDO処理中かどうか
//
//	ModBoolean
//		検索でヒットした場合はModTrue、それ以外の場合はModFalse
//
//	EXCEPTIONS
//
ModBoolean
MiddleBaseListIterator::find(const DocumentID uiDocumentID_, bool bUndo_)
{
	if (lowerBound(uiDocumentID_, bUndo_) == ModTrue)
	{
		if (m_uiCurrentID == uiDocumentID_)
			return ModTrue;
	}
	return ModFalse;
}

//
//	FUNCTION public
//	Inverted::MiddleBaseListIterator::lowerBound -- 文書IDをlower_bound検索する
//
//	NOTES
//
//	ARGUMENTS
//	ModInvertedIterator::DocumentID uiDocumentID_
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
ModBoolean
MiddleBaseListIterator::lowerBound(const DocumentID uiDocumentID_, bool bUndo_)
{
	DocumentID uiDocumentID = uiDocumentID_;

	if (bUndo_ == false &&
		(m_pArea->getDocumentCount() == 0 ||
		 (m_pArea->getLastDocumentID() != 0 &&
		  m_pArea->getLastDocumentID() < uiDocumentID_)))
	{
		// UNDO処理中ではなくて、上記の場合、uiDocumentID_が存在しないことが明らか
		m_uiCurrentID = UndefinedDocumentID;
		return ModFalse;
	}

	//
	// uiDocumentID_を含むIDブロックを取得
	//

	if (bUndo_ == true
		|| m_cIdBlock.isInvalid() == true
		|| m_cIdBlock.getFirstDocumentID() > uiDocumentID
		|| m_uiLastDocumentID == 1)
	{
		// UNDO処理中か、まだIDブロックが割り当てられていないか、
		// 現在の文書IDよりも小さい場合はDIRブロックから2分探索

		setLocBlockByEmpty();
		m_bSynchronized = false;

		LeafPage::DirBlock* pDirBlock = 0;

		if (bUndo_ == true)
		{
			// IDブロックの先頭文書IDをUndoするのかもしれない
			ModUInt32 ID = m_cInvertedList.getExpungeFirstDocumentID(uiDocumentID);
			if (ID != UndefinedDocumentID)
			{
				// 置き換えちゃう
				//	-> マージの時はUndoは来ないので、
				//	   ここに来るのは1件削除のときだけ
				uiDocumentID = ID;
			}
		}

		//
		// uiDocumentID_を含むIDブロックの候補を取得
		//

		// とりあえず最終IDブロックを設定する
		m_cIdBlock = m_pArea->getIDBlock();

		// 最終IDブロック外か？
		if (m_cIdBlock.getFirstDocumentID() > uiDocumentID)
		{
			//
			//	DIRブロックを2分探索
			//
			LeafPage::DirBlock cDirBlock;
			cDirBlock.m_uiDocumentID = uiDocumentID;
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
				//
				//	Dirブロックがみつかった
				//
				
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
				ModSize uiPosition = m_uiIDBlockPosition;
				m_cIdBlock = m_pIdPage->lowerBoundIDBlock(
							   uiDocumentID, uiPosition, bUndo_);
				m_uiIDBlockPosition = uiPosition;
			}
		}

		if (bUndo_ == false && m_cIdBlock.isExpunge())
		{
			// IDブロックが削除されている
			m_uiCurrentID = UndefinedDocumentID;
			return ModFalse;
		}

		// 先頭文書IDを設定する
		setFirstDocumentID(m_cIdBlock);
	}
	else if (m_uiLastDocumentID <= uiDocumentID)
	{
		// UNDO処理でもなく、すでにIDブロックが割り当てられていて、
		// 現在のIDブロックの最大文書IDよりも大きい文書IDの場合は、
		// 2分探索せず、存在するIDブロックをシーケンシャルに探す

		setLocBlockByEmpty();
		m_bSynchronized = false;

		//
		//	シーケンシャルサーチ
		//

		while (m_uiLastDocumentID <= uiDocumentID)
		{
			if (nextIdBlock() == false)
			{
				m_uiCurrentID = UndefinedDocumentID;
				return ModFalse;
			}
		}

		// setFirstDocumentIDは不要。nextIdBlock内で実行される。
	}
	else if (m_uiCurrentID > uiDocumentID)
	{
		// 現在のIDブロック中にあるが、現在位置より前
		
		setLocBlockByEmpty();
		m_bSynchronized = false;

		setFirstDocumentID(m_cIdBlock);
	}

	//
	//	IDBlockの中を検索する
	//

	while (m_uiCurrentID < uiDocumentID)
	{
		if (isEnd() == ModTrue) return ModFalse;
		// 次へ
		next(bUndo_);
	}

	return ModTrue;
}

//
//	FUNCTION public
//	Inverted::MiddleBaseListIterator::getDocumentId -- 文書IDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	DocumentID
//		文書ID
//
//	EXCEPTIONS
//
MiddleBaseListIterator::DocumentID
MiddleBaseListIterator::getDocumentId()
{
// 	if (isEnd() == ModTrue) return UndefinedDocumentID;
//	
// 	if (m_cIdBlock.isInvalid() == true)
// 	{
// 		nextIdBlock();
// 	}
	
	if (m_uiCurrentID != UndefinedDocumentID)
	{
		if (m_cIdBlock.isInvalid() == true)
		{
			nextIdBlock();
		}
	}
	return m_uiCurrentID;
}

//
//	FUNCTION public
//	Inverted::MiddleBaseListIterator::getInDocumentFrequency --
//		位置情報内の頻度情報を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		位置情報内の頻度情報
//
//	EXCEPTIONS
//
ModSize
MiddleBaseListIterator::getInDocumentFrequency()
{
	// 位置情報との同期を取る
	synchronize();

	return m_uiCurrentFrequency;
}

//
//	FUNCTION public
//	Inverted::MiddleBaseListIterator::getLocationListIterator --
//		現在の位置情報のイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
ModInvertedLocationListIterator*
MiddleBaseListIterator::getLocationListIterator()
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION public
//	Inverted::MiddleBaseListIterator::expunge -- 現在位置の情報を削除する
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
MiddleBaseListIterator::expunge()
{
	synchronize();
	
	//
	//	文書IDを削除する
	//
	if (m_cIdBlock.getFirstDocumentID() == m_uiCurrentID)
	{
		// 先頭文書IDを削除する
		expungeFirstDocumentID();
	}
	else
	{
		// 先頭以外の文書IDを削除する
		expungeDocumentID();
	}

	// [YET] 意味ないif文を削除

	//
	// エリアの文書数をデクリメントする
	//
	m_pArea->decrementDocumentCount();

	//
	//	位置情報を削除する
	//
	expungeLocation();
}

//
//	FUNCTION public
//	Inverted::MiddleBaseListIterator::undoExpunge -- 削除の取り消し
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		文書ID
//	const ModInvertedSmartLocationList& cLocationList_
//		位置情報リスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MiddleBaseListIterator::undoExpunge(ModUInt32 uiDocumentID_,
									const ModInvertedSmartLocationList& cLocationList_)
{
	synchronize(true);

	// 文書IDを挿入する
	undoExpungeDocumentID(uiDocumentID_);

	// 位置情報を挿入する
	undoExpungeLocation(cLocationList_);

	// エリアの文書数をインクリメントする
	m_pArea->incrementDocumentCount();
}

//
//	FUNCTION public
//	Inverted::MiddleBaseListIterator::getHeadAddress --
//		現在の位置情報の先頭アドレスを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32*
//		現在の位置情報のアドレス
//
//	EXCEPTIONS
//
ModUInt32*
MiddleBaseListIterator::getHeadAddress()
{
	synchronize();
	; _SYDNEY_ASSERT(getLocBlockDataHeadAddress() != 0);
	return getLocBlockDataHeadAddress();
}

//
//	FUNCTION public
//	Inverted::MiddleBaseListIterator::getLocationOffset --
//		現在の位置情報のオフセットを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		現在の位置情報のオフセット
//
//	EXCEPTIONS
//
ModSize
MiddleBaseListIterator::getLocationOffset()
{
	synchronize();
	return getLocOffset();
}

//
//	FUNCTION public
//	Inverted::MiddleBaseListIterator::getLocationBitLength --
//		現在の位置情報のビット長を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		現在の位置情報のビット長
//
//	EXCEPTIONS
//
ModSize
MiddleBaseListIterator::getLocationBitLength()
{
	synchronize();
	return getLocLength();
}

//
//	FUNCTION public
//	Inverted::MiddleBaseListIterator::getLocationDataOffset --
//		現在の位置情報データのオフセットを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		現在の位置情報データのオフセット
//
//	EXCEPTIONS
//
ModSize
MiddleBaseListIterator::getLocationDataOffset()
{
	synchronize();
	return getLocDataOffset();
}

//
//	FUNCTION public
//	Inverted::MiddleBaseListIterator::getLocationDataBitLength --
//		現在の位置情報データのビット長を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		現在の位置情報データのビット長
//
//	EXCEPTIONS
//
ModSize
MiddleBaseListIterator::getLocationDataBitLength()
{
	synchronize();
	return getLocDataLength();
}

//
//	FUNCTION public
//	Inverted::MiddleBaseListIterator::expungeIdBlock --
//		現在位置のIDブロックを削除する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		IDページを削除したらtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
MiddleBaseListIterator::expungeIdBlock()
{
	if (m_cIdBlock.isExpunge() == true)
	{
		// まずLOCブロックを削除する
		PhysicalFile::PageID uiLocPageID = m_cIdBlock.getLocBlockPageID();
		unsigned short usOffset = m_cIdBlock.getLocBlockOffset();
		while (uiLocPageID != PhysicalFile::ConstValue::UndefinedPageID)
		{
			setLocPage(m_pOverflowFile->attachPage(uiLocPageID));
			setLocBlock(getLocPage()->getLocBlock(usOffset));
			usOffset = 0;

			if (getLocBlock().isContinue())
			{
				// 次があったら
				uiLocPageID = getLocPage()->getNextPageID();
			}
			else
			{
				// なかったら
				uiLocPageID = PhysicalFile::ConstValue::UndefinedPageID;
			}

			if (m_pArea->getLastLocationPageID() == getLocPage()->getID()
				&& getLocBlock().getOffset() == m_pArea->getLocationOffset())
			{
				// 最終IDブロックの先頭のLOCブロック -> 続きフラグを削除する
				getLocBlock().unsetContinueFlag();
			}
			else
			{
				// LOCブロックを開放する
				getLocPage()->freeLocBlock();
			}

			// 最終LOCページは削除できない
			if (getLocPage()->getType() == OverflowPage::Type::LOC
				&& getLocPage()->getLocBlockCount() == 0
				&& m_pArea->getLastLocationPageID() != getLocPage()->getID())
			{
				// このページは不要
				m_pOverflowFile->freePage(getLocPage());
			}
		}

		if (m_cIdBlock != m_pArea->getHeadAddress())
		{
			// IDブロックはオーバーフローページ内にある
			
			// IDブロックを削除する
			m_pIdPage->freeIDBlock(m_uiIDBlockPosition);

			ModSize idBlockCount = m_pIdPage->getIDBlockCount();

			// 最終LOCページは削除できない
			if ((m_pIdPage->getType() == OverflowPage::Type::ID &&
				  m_pIdPage->getIDBlockCount() == 0) ||
				 (m_pIdPage->getType() == OverflowPage::Type::IDLOC &&
				  m_pIdPage->getIDBlockCount() == 0 &&
				  m_pIdPage->getLocBlockCount() == 0 &&
				  m_pArea->getLastLocationPageID() != m_pIdPage->getID()))
			{
				// このページは不要
				m_pOverflowFile->freePage(m_pIdPage);
			}

			if (idBlockCount == 0)
			{
				// DIRブロックを削除する
				int count = m_pArea->getDirBlockCount();
				for (int i = m_iDirBlockPosition; i < count-1; ++i)
				{
					m_pDirBlock[i] = m_pDirBlock[i+1];
				}

				return true;
			}
		}
	}

	return false;
}

//
//	FUNCTION public
//	Inverted::MiddleBaseListIterator::pushBack -- 不要な位置情報クラスをpushする
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::MiddleBaseListLocationListIterator* pFree_
//		不要な位置情報クラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MiddleBaseListIterator::pushBack(MiddleListLocationListIterator* pFree_)
{
	pFree_->nextInstance = getFreeLocationListIterator();
	setFreeLocationListIterator(pFree_);
}

//
//	FUNCTION protected
//	Inverted::MiddleBaseListIterator::setFirstDocumentID -- 先頭文書IDを設定する
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
MiddleBaseListIterator::setFirstDocumentID()
{
	//
	// IDブロックが最終IDブロックだけの場合、
	// 最終IDブロックの先頭文書IDを、参照中の文書IDに設定
	//

	// 初期化
	m_pDirBlock = m_pArea->getDirBlock();

	m_iDirBlockPosition = 0;	// 現在のDIRブロック位置
	m_uiIDBlockPosition = 0;	// IDページ中の現在のIDブロック位置
	m_pIdPage = 0;			// 現在のIDページ
	m_uiCurrentID = 0;		// 現在のID

	setLocBlockByEmpty();

	// DocumentIDの設定
	if (m_pArea->getDocumentCount() == 0)
	{
		m_uiCurrentID = UndefinedDocumentID;
	}
	
	if (m_pArea->getDirBlockCount() == 0)
	{
		// IDブロックは最終IDブロックのみ

		// IDブロックを設定
		m_cIdBlock = m_pArea->getIDBlock();
		// 先頭文書IDを設定
		setFirstDocumentID(m_cIdBlock);
	}
	else
	{
		// IDブロックを初期化

		// [YET!] 最終IDブロックではない時、設定しないのはなぜ？
		m_cIdBlock = OverflowPage::IDBlock();
	}
}

//
//	FUNCTION protected
//	Inverted::MiddleBaseListIterator::setFirstDocumentID -- 先頭文書IDを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const OverflowPage::IDBlock& cIdBlock_
//		IDブロック
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MiddleBaseListIterator::setFirstDocumentID(const OverflowPage::IDBlock& cIdBlock_)
{
	//
	// 参照中のIDブロックの先頭文書IDを、参照中の文書IDに設定
	//
	
	// 初期化
	m_uiPrevID = 0;				// 直前の文書ID
	m_uiNextOffset = 0;			// IDブロック中の次のビット位置
	m_uiCurrentOffset = 0;		// IDブロック中の現在のビット位置
	m_iCurrentPosition = 0;		// IDブロック中の位置

	// 先頭文書IDを設定
	m_uiCurrentID = cIdBlock_.getFirstDocumentID();

	//
	// 最終文書IDをインクリメント
	//

	// [YET!] 最終文書IDをインクリメントするのはなぜ？

	DocumentID uiDocumentID = 0;
	if (cIdBlock_ == m_pArea->getHeadAddress())
	{
		// 最終IDブロック
		uiDocumentID = m_pArea->getLastDocumentID() + 1;
	}
	else
	{
		// 最終IDブロック以外のIDブロック
		uiDocumentID = m_pIdPage->getNextDocumentID(m_uiIDBlockPosition);
		
		if (uiDocumentID == UndefinedDocumentID)
		{
			// 次のDocumentIDが存在しなかった
			if (m_iDirBlockPosition + 1 >= m_pArea->getDirBlockCount())
			{
				// 最終IDブロックの先頭文書ID
				uiDocumentID = m_pArea->getIDBlock().getFirstDocumentID();
			}
			else
			{
				// 次のDIRブロックの先頭文書ID
				uiDocumentID = m_pDirBlock[m_iDirBlockPosition+1].getDocumentID();
			}
		}
	}
	m_uiLastDocumentID = uiDocumentID;
}

//
//	FUNCTION protected
//	Inverted::MiddleBaseListIterator::nextIdBlock -- 次のIDブロックへ移動する
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
		// 最終IDブロックなので次はない
		
		m_cIdBlock = OverflowPage::IDBlock();
		return false;
	}

	//
	//	次のIDブロックに移動する
	//

	while (true)
	{
		if (m_pIdPage == 0)
		{
			// 初めて

			// IDページを取得
			m_pIdPage = m_pOverflowFile->attachPage(
						  m_pDirBlock[m_iDirBlockPosition].getPageID());
			// [YET!] Positionを初期化しなくて大丈夫？
			; _SYDNEY_ASSERT(m_uiIDBlockPosition == 0);
		}
		else
		{
			// IDブロックの位置を進める
			++m_uiIDBlockPosition;
			
			if (m_uiIDBlockPosition >= m_pIdPage->getIDBlockCount())
			{
				// もうこのページにはIDブロックはない
				// 次のDirブロックに移動
				
				++m_iDirBlockPosition;
				if (m_iDirBlockPosition >= m_pArea->getDirBlockCount())
				{
					// もうDIRブロックもない -> 最終IDブロック
					m_cIdBlock = m_pArea->getIDBlock();
					break;
				}

				// 次のIDページを取得
				m_pIdPage = m_pOverflowFile->attachPage(
							  m_pDirBlock[m_iDirBlockPosition].getPageID());
				m_uiIDBlockPosition = 0;
			}
		}

		// [YET!] IDブロックの取得をif文の外に出した
		// IDブロックを取得
		m_cIdBlock = m_pIdPage->getIDBlock(m_uiIDBlockPosition);

		if (m_cIdBlock.isExpunge() == false)
			break;
	}

	if (m_cIdBlock.isExpunge())
		return false;

	//
	// 次のIDブロックが見つかったので、先頭文書IDを設定する
	//
	
	setFirstDocumentID(m_cIdBlock);

	return true;
}

//
//	FUNCTION protected
//	Inverted::MiddleBaseListIterator::nextLocBlock -- 次のLOCブロックへ移動する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiOddLength_
//		前のLOCブロックに位置情報が全て入らなかった場合、
//		次のLOCブロックのデータ部の先頭には、残りの位置情報が格納されている。
//		その分の参照中の位置情報のオフセットを進めておく。
//
//	RETURN
//	bool
//		常にtrue
//
//	EXCEPTIONS
//
bool
MiddleBaseListIterator::nextLocBlock(ModSize uiOddLength_)
{
	; _SYDNEY_ASSERT(getLocPage() != 0);
	; _SYDNEY_ASSERT(getLocPage()->getNextPageID()
					 != PhysicalFile::ConstValue::UndefinedPageID);

	setLocPage(m_pOverflowFile->attachPage(getLocPage()->getNextPageID()));

	; _SYDNEY_ASSERT(getLocPage() != 0);

	// [YET]
	// LocPosition, LocLengthの初期化は？
	
	setLocBlock(getLocPage()->getLocBlock());
	setLocOffset(uiOddLength_);
	setLocBlockDataHeadAddress(getLocBlock().getBuffer());
	setLocBlockDataLength(getLocBlock().getDataBitLength());

	return true;
}

//
//	FUNCTION protected
//	Inverted::MiddleBaseListIterator::expungeFirstDocumentID --
//		先頭文書IDを削除する
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
	// 次の文書IDを読む
	ModSize uiOffset = m_uiNextOffset;
	ModUInt32 uiNextID = m_cInvertedList.readDocumentID(
			m_uiCurrentID,
			m_cIdBlock.getBuffer(), m_cIdBlock.getDataSize(), uiOffset);

	if (m_cIdBlock != m_pArea->getHeadAddress())
		// IDブロックはオーバーフローページ内にある
		m_pIdPage->dirty();

	//
	//	先頭文書IDを削除する
	//
	
	if (uiNextID == UndefinedDocumentID)
	{
		// １つも読めない -> もうこのIDブロックには1つもデータは入っていない

		//
		//	IDブロックを削除
		//
		
		m_cIdBlock.setExpunge();
		if (m_cIdBlock == m_pArea->getHeadAddress())
		{
			// 最終IDブロック
			m_cInvertedList.enterDeleteIdBlock(
					m_cIdBlock.getFirstDocumentID());
		}
		else
		{
			// IDページ中のIDブロック
			if (m_pIdPage->enterDeleteIdBlock(m_cInvertedList) == true)
			{
				// このページにはもうIDブロックは存在しない
				m_pDirBlock[m_iDirBlockPosition].setExpunge();
			}
		}
		
		//
		//	現在の状況を更新
		//
		
		// 次のIDブロックへ
		nextIdBlock();

		// m_uiCurrentID = uiNextIDは不要。nextIdBlock内のsetFirstDocumentID内で実行される。
	}
	else
	{
		//
		//	文書IDを削除
		//
		
		// 後ろを移動
		ModSize uiDstOffset = 0;
		ModSize uiSrcOffset = uiOffset;
		ModSize uiLength = m_cIdBlock.getDataSize() - uiOffset;

		InvertedList::moveBack(m_cIdBlock.getBuffer(), uiDstOffset,
								m_cIdBlock.getBuffer(), uiSrcOffset, uiLength);
		InvertedList::setOffBack(m_cIdBlock.getBuffer(), uiLength, uiOffset);

		//
		//	Areaと現在の状況を更新
		//
								 
		if (m_cIdBlock == m_pArea->getHeadAddress())
			// 最終IDブロックなので文書IDオフセットを更新する
			m_pArea->setDocumentOffset(m_pArea->getDocumentOffset() - uiOffset);

		// 先頭文書ID
		m_cIdBlock.setFirstDocumentID(uiNextID);

		// ログを記録する
		m_cInvertedList.enterExpungeFirstDocumentID(
				m_uiCurrentID, uiNextID);

		m_uiCurrentID = uiNextID;
	}
}

//
//	FUNCTION protected
//	Inverted::MiddleBaseListIterator::expungeDocumentID -- 文書IDを削除する
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
	ModUInt32 uiNextID = m_cInvertedList.readDocumentID(
			m_uiCurrentID,
			m_cIdBlock.getBuffer(), m_cIdBlock.getDataSize(), uiOffset);

	if (m_cIdBlock != m_pArea->getHeadAddress())
		// IDブロックはオーバーフローページ内にある
		m_pIdPage->dirty();

	//
	//	文書IDを削除する
	//

	if (uiNextID != UndefinedDocumentID)
	{
		// 差分を書き直す
		m_uiNextOffset = m_uiCurrentOffset;
		InvertedList::setOffBack(m_cIdBlock.getBuffer(), m_uiCurrentOffset, uiOffset - m_uiCurrentOffset);
		m_cInvertedList.writeDocumentID(m_uiPrevID, uiNextID, m_cIdBlock.getBuffer(), m_uiNextOffset);
		
		// 後ろを移動
		ModSize uiDstOffset = m_uiNextOffset;
		ModSize uiSrcOffset = uiOffset;
		ModSize uiLength = m_cIdBlock.getDataSize() - uiOffset;

		if (uiDstOffset != uiSrcOffset)
		{
			// (NextIDのビット長 + CurrentIDのビット長)が(NextIDのビット長)と等しくない

			// もし、PrevID < CurrentID <<< NextID なら、
			// 圧縮の過程で同じ長さのままということとはありえる。

			if (uiLength != 0)
			{
				// NextIDは最後の文書IDではない

				// もし、最後の文書IDなら、その後ろは存在しないので移動する必要はない。

				InvertedList::moveBack(m_cIdBlock.getBuffer(), uiDstOffset,
									   m_cIdBlock.getBuffer(), uiSrcOffset, uiLength);
			}
			InvertedList::setOffBack(m_cIdBlock.getBuffer(),
									 uiDstOffset + uiLength,
									 uiSrcOffset - uiDstOffset);
		}

		//
		//	Areaと現在の状況を更新
		//
		
		if (m_cIdBlock == m_pArea->getHeadAddress())
			// 最終IDブロックなので文書IDオフセットを更新する
			m_pArea->setDocumentOffset(m_pArea->getDocumentOffset() - (uiOffset - m_uiNextOffset));

		// 今の文書ID
		m_uiCurrentID = uiNextID;
	}
	else
	{
		// このIDブロックの最後の文書IDなので、書かれていたビットをクリアするのみ

		//
		// ビットをクリア
		//
		
		InvertedList::setOffBack(m_cIdBlock.getBuffer(), m_uiCurrentOffset, m_uiNextOffset - m_uiCurrentOffset);

		//
		//	Areaと現在の状況を更新
		//

		if (m_cIdBlock == m_pArea->getHeadAddress())
		{
			// 最終IDブロックなので文書IDオフセットを更新する
			m_pArea->setDocumentOffset(m_pArea->getDocumentOffset() - (m_uiNextOffset - m_uiCurrentOffset));
			// 最終文書IDを更新する
			m_pArea->setLastDocumentID(m_uiPrevID);
		}

		// 次のIDブロックを設定する
		nextIdBlock();
	}
}

//
//	FUNCTION protected
//	Inverted::MiddleBaseListIterator::expungeLocation -- 位置情報を削除する
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
MiddleBaseListIterator::expungeLocation()
{
	//
	//	参照中の位置情報を削除
	//
	
	ModSize bitLength = getLocLength();
	while (bitLength)
	{
		ModSize uiLocBlockDataLength = getLocBlockDataLength();
		ModSize currentBitLength = bitLength;
		if (currentBitLength + getLocOffset() > uiLocBlockDataLength)
		{
			// 現在の位置情報はLOCブロックを跨いでいるので、
			// このLOCブロック内の分だけを設定
			currentBitLength = uiLocBlockDataLength - getLocOffset();
		}

		getLocPage()->dirty();

		//
		// 削除する
		//
		ModSize uiDstOffset = getLocOffset();
		ModSize uiSrcOffset = uiDstOffset + currentBitLength;
		ModSize uiLength = uiLocBlockDataLength - uiSrcOffset;
		if (uiLength)
		{
			InvertedList::move(getLocBlockDataHeadAddress(), uiDstOffset,
							   getLocBlockDataHeadAddress(), uiSrcOffset, uiLength);
		}
		InvertedList::setOff(
			getLocBlockDataHeadAddress(), uiDstOffset + uiLength, uiSrcOffset - uiDstOffset);

		//
		// LOCブロックと現在の状況を更新
		//
		uiLocBlockDataLength -= currentBitLength;
		setLocBlockDataLength(uiLocBlockDataLength);
		getLocBlock().setDataBitLength(uiLocBlockDataLength);

		//
		// 次のLOCブロック
		//
		
		// 位置情報の残りのビット長を設定
		bitLength -= currentBitLength;
		
		if (bitLength)
		{
			// 次のLOCブロックへ
			nextLocBlock();
		}
	}

	//
	//	同期もしくは初期化
	//

	if (m_bSynchronized == true)
	{
		while (getLocBlockDataLength() == getLocOffset())
		{
			// 次のLOCブロックへ
			nextLocBlock();
		}

		setLocationInformation();
	}
	else
	{
		// ロックブロックをクリアする
		setLocBlockByEmpty();
	}
}

//
//	FUNCTION protected
//	Inverted::MiddleBaseListIterator::undoExpungeDocumentID --
//		文書IDの削除を取り消す
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
	//
	// 参照中の文書IDの直前に、削除された文書IDを戻す
	//
	
	if (m_cIdBlock != m_pArea->getHeadAddress())
		// IDブロックはオーバーフローページ内にある
		m_pIdPage->dirty();

	//
	//	削除を取り消す
	//
	
	if (m_cIdBlock.getFirstDocumentID() == uiDocumentID_)
	{
		// 先頭文書IDが削除されている

		// IDブロックの削除フラグを落とす(uiDocumentID_で上書きする)
		m_cIdBlock.setFirstDocumentID(uiDocumentID_);
		
		if (m_cIdBlock != m_pArea->getHeadAddress())
		{
			// DIRブロックの削除フラグが立っているかもしれないので、
			// 削除フラグを落とす。
			m_pDirBlock[m_iDirBlockPosition].unsetExpunge();
		}
	}
	else if (m_cIdBlock.getFirstDocumentID() == m_uiCurrentID)
	{
		// 先頭文書IDを挿入する

		// ビット長
		ModSize length = m_cInvertedList.getCompressedBitLengthDocumentID(
				uiDocumentID_, m_uiCurrentID);

		// 後ろを移動
		ModSize uiDstOffset = length;
		ModSize uiSrcOffset = m_uiCurrentOffset;
		ModSize uiLength = m_cIdBlock.getDataSize() - uiDstOffset;
		InvertedList::moveBack(m_cIdBlock.getBuffer(), uiDstOffset,
							   m_cIdBlock.getBuffer(), uiSrcOffset, uiLength);
		InvertedList::setOffBack(
			m_cIdBlock.getBuffer(), uiSrcOffset, uiDstOffset - uiSrcOffset);

		// 書き込む
		m_cInvertedList.writeDocumentID(
			uiDocumentID_, m_uiCurrentID,
			m_cIdBlock.getBuffer(), uiSrcOffset);

		//
		//	Areaと現在の状況を更新
		//
		
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
		m_cInvertedList.writeDocumentID(
			m_uiPrevID, uiDocumentID_,
			m_cIdBlock.getBuffer(), uiOffset);

		//
		//	Areaを更新
		//
		
		if (m_cIdBlock == m_pArea->getHeadAddress())
		{
			// 文書IDオフセット
			m_pArea->setDocumentOffset(uiOffset);
		}
	}
	else
	{
		// その他の文書IDを挿入する

		ModSize length = m_cInvertedList.getCompressedBitLengthDocumentID(m_uiPrevID, uiDocumentID_);
		length += m_cInvertedList.getCompressedBitLengthDocumentID(uiDocumentID_, m_uiCurrentID);
		length -= m_uiNextOffset - m_uiCurrentOffset;

		// 後ろを移動する
		ModSize uiDstOffset = m_uiNextOffset + length;
		ModSize uiSrcOffset = m_uiNextOffset;
		ModSize uiLength = m_cIdBlock.getDataSize() - uiDstOffset;
		if (uiLength)
		{
			// NextIDは最後の文書IDではない
			// もし最後の文書IDならその後ろを移動する必要はない
			InvertedList::moveBack(m_cIdBlock.getBuffer(), uiDstOffset,
								   m_cIdBlock.getBuffer(), uiSrcOffset, uiLength);
		}
		InvertedList::setOffBack(m_cIdBlock.getBuffer(),
								 m_uiCurrentOffset,
								 uiDstOffset - m_uiCurrentOffset);

		// 書き込む
		ModSize uiOffset = m_uiCurrentOffset;
		m_cInvertedList.writeDocumentID(m_uiPrevID, uiDocumentID_, m_cIdBlock.getBuffer(), uiOffset);
		m_cInvertedList.writeDocumentID(uiDocumentID_, m_uiCurrentID, m_cIdBlock.getBuffer(), uiOffset);

		//
		//	Areaを更新
		//
		
		if (m_cIdBlock == m_pArea->getHeadAddress())
		{
			// 文書IDオフセット
			m_pArea->setDocumentOffset(m_pArea->getDocumentOffset() + length);
		}
	}

	//
	//	Areaを更新
	//

	if (m_pArea->getLastDocumentID() < uiDocumentID_)
	{
		// 最終文書IDを更新する
		m_pArea->setLastDocumentID(uiDocumentID_);
	}
}

//
//	FUNCTION protected
//	Inverted::MiddleBaseListIterator::undoExpungeLocation --
//		位置情報の削除を取り消す
//
//	NOTES
//
//	ARGUMENTS
//	const ModInvertedSmartLocationList& cLocationList_
//		位置情報リスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MiddleBaseListIterator::undoExpungeLocation(const ModInvertedSmartLocationList& cLocationList_)
{
	ModSize bitLength;
	ModSize length = m_cInvertedList.getCompressedBitLengthLocationList(
			cLocationList_, bitLength);
	ModAutoPointer<ModInvertedLocationListIterator> i;
	ModAutoPointer<ModInvertedLocationListIterator> j;
	ModUInt32 uiPrevLocation = 0;
	bool initial = true;

	while (length)
	{
		// 今のLOCブロックに書き込めるビット長を得る
		ModSize maxBitLength = getLocBlock().getDataUnitSize() * sizeof(ModUInt32) * 8;

		//
		//	今のLOCブロックに書き込むビット長を得る
		//
		
		ModSize currentLength = length;
		while (currentLength + getLocBlockDataLength() > maxBitLength)
		{
			// いっぺんに書き込めないので、書き込める最大値を求める
			currentLength = 0;

			if (i.get() == 0)
			{
				// はじめて
				
				// 頻度とビット長
				ModSize frequency = cLocationList_.getSize();
				currentLength += m_cInvertedList.getCompressedBitLengthFrequency(
						frequency);
				if (frequency > 1)
					currentLength += m_cInvertedList.getCompressedBitLengthBitLength(
							bitLength);
				
				i = cLocationList_.begin();	// 書き込み用
				j = cLocationList_.begin();	// サイズ用
			}

			ModSize l = 0;
			ModUInt32 prev = uiPrevLocation;
			while (currentLength <= maxBitLength - getLocBlockDataLength())
			{
				//
				//	文書内の位置を一つずつ追加
				//

				l = m_cInvertedList.getCompressedBitLengthLocation(
						prev, (*j).getLocation());
				currentLength += l;
				prev = (*j).getLocation();
				if (currentLength <= maxBitLength - getLocBlockDataLength())
					(*j).next();
			}
			currentLength -= l;	// 超えちゃったので引く

			if (currentLength + getLocBlockDataLength() > maxBitLength)
			{
				// このブロックには何も書けない -> 次のブロックへ
				nextLocBlock();
				
				maxBitLength = getLocBlock().getDataUnitSize() * sizeof(ModUInt32) * 8;
				currentLength = length;
				i = 0;
				j = 0;
			}
		}

		//
		//	位置情報を書き込む
		//

		getLocPage()->dirty();

		// 書き込む分移動する
		ModSize uiOffset = getLocOffset();
		ModSize uiDstOffset = uiOffset + currentLength;
		ModSize uiSrcOffset = uiOffset;
		ModSize uiLength = getLocBlockDataLength() - getLocOffset();
		InvertedList::move(getLocBlockDataHeadAddress(), uiDstOffset,
						   getLocBlockDataHeadAddress(), uiSrcOffset, uiLength);
		InvertedList::setOff(getLocBlockDataHeadAddress(), uiSrcOffset, uiDstOffset - uiSrcOffset);
		
		// 書き込む
		if (length == currentLength)
		{
			// そのまま書き込める
			if (i.get() == 0)
				m_cInvertedList.writeLocationList(
					cLocationList_, bitLength, getLocBlockDataHeadAddress(), uiOffset);
			else
				m_cInvertedList.writeLocationData(
					uiPrevLocation, i, getLocBlockDataHeadAddress(), uiOffset);
		}
		else
		{
			if (initial == true)
			{
				ModUInt32 saveCurrent = uiOffset;
				// 頻度と位置情報を書く
				ModSize frequency = cLocationList_.getSize();
				m_cInvertedList.writeLocationFrequency(
						frequency, getLocBlockDataHeadAddress(), uiOffset);
				if (frequency > 1)
					m_cInvertedList.writeLocationBitLength(
							bitLength, getLocBlockDataHeadAddress(), uiOffset);
				
				currentLength -= (uiOffset - saveCurrent);
				length -= (uiOffset - saveCurrent);
				setLocBlockDataLength(getLocBlockDataLength() + uiOffset - saveCurrent);
				getLocBlock().setDataBitLength(getLocBlockDataLength());
				
				initial = false;
			}
			// 位置情報を書けるところまで書く
			uiPrevLocation = m_cInvertedList.writeLocationData(
					uiPrevLocation, i, getLocBlockDataHeadAddress(), uiOffset,
					uiOffset + currentLength);
		}
		setLocOffset(uiOffset);

		//
		//	現在の状況を更新
		//

		getLocBlock().setDataBitLength(getLocBlockDataLength() + currentLength);

		//
		//	次のLOCブロック
		//
		
		length -= currentLength;

		if (length)
		{
			// 次のLOCブロックを得る
			nextLocBlock();
		}
	}
}

//
//	FUNCTION private
//	Inverted::MiddleBaseListIterator::next -- 次へ
//
//	NOTES
//
//	ARGUMENTS
//	bool bUndo_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MiddleBaseListIterator::next(bool bUndo_)
{
	// 引数なしのnextと分割した。
	// 引数ありのnextは、lowerBoundからのみ使われており、
	// isEnd() == ModFalseを確認してから使っているので、
	// if文を省いた。
	; _SYDNEY_ASSERT(isEnd() == ModFalse);

	next2();
	
	// 新しい情報の確認
	// if (isEnd() == ModTrue && bUndo_ == false)
	if (m_uiCurrentID == UndefinedDocumentID && bUndo_ == false)
	{
		// 終端に達したので、次のブロックを読む
		nextIdBlock();
		// Locブロックはまだ設定されない。
		setLocBlockByEmpty();
	}
}

//
//	FUNCTION private
//	Inverted::MiddleBaseListIterator::next2 -- nextの下請け
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
MiddleBaseListIterator::next2()
{
	; _SYDNEY_ASSERT(isEnd() == ModFalse);

	// 同期がずれる
	m_bSynchronized = false;

	// IDBlockの確認
	if (m_cIdBlock.isInvalid() == true)
	{
		nextIdBlock();
	}

	// Positionをインクリメントする
	++m_iCurrentPosition;
	
	// 今までの情報をコピー
	m_uiPrevID = m_uiCurrentID;
	m_uiCurrentOffset = m_uiNextOffset;

	// 新しい情報を設定する
	m_uiCurrentID = m_cInvertedList.readDocumentID(
		m_uiPrevID,
		m_cIdBlock.getBuffer(),
		m_cIdBlock.getDataSize(),
		m_uiNextOffset);	// 次の文書IDオフセットも更新される
}

//
//	FUNCTION private
//	Inverted::MiddleBaseListIterator::setLocationInformation --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
MiddleBaseListIterator::setLocationInformation()
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::MiddleBaseListIterator::getLocPage --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	OverflowPage*
//
//	EXCEPTIONS
//
OverflowPage::PagePointer
MiddleBaseListIterator::getLocPage() const
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::MiddleBaseListIterator::setLocPage --
//
//	NOTES
//
//	ARGUMENTS
//	OverflowPage* cLocPage_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MiddleBaseListIterator::setLocPage(OverflowPage* cLocPage_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::MiddleBaseListIterator::getLocBlock --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	OverflowPage::IDBlock&
//
//	EXCEPTIONS
//
OverflowPage::LocBlock&
MiddleBaseListIterator::getLocBlock()
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::MiddleBaseListIterator::setLocBlock --
//
//	NOTES
//
//	ARGUMENTS
//	OverflowPage::LocBlock cLocBlock_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MiddleBaseListIterator::setLocBlock(OverflowPage::LocBlock cLocBlock_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::MiddleBaseListIterator::getLocBlockDataHeadAddress --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	int
//
//	EXCEPTIONS
//
ModUInt32*
MiddleBaseListIterator::getLocBlockDataHeadAddress()
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::MiddleBaseListIterator::setLocBlockDataHeadAddress --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	int
//
//	EXCEPTIONS
//
void
MiddleBaseListIterator::setLocBlockDataHeadAddress(ModUInt32* p_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::MiddleBaseListIterator::getLocBlockDataLength --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	int
//
//	EXCEPTIONS
//
ModSize
MiddleBaseListIterator::getLocBlockDataLength() const
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::MiddleBaseListIterator::setLocBlockDataLength --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	int
//
//	EXCEPTIONS
//
void
MiddleBaseListIterator::setLocBlockDataLength(ModSize uiLength_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::MiddleBaseListIterator::getLocOffset --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	ModSize
//
//	EXCEPTIONS
//
ModSize
MiddleBaseListIterator::getLocOffset() const
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::MiddleBaseListIterator::setLocOffset --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MiddleBaseListIterator::setLocOffset(ModSize uiOffset_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::MiddleBaseListIterator::getLocLength --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	ModSize
//
//	EXCEPTIONS
//
ModSize
MiddleBaseListIterator::getLocLength() const
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::MiddleBaseListIterator::getLocDataOffset --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	ModSize
//
//	EXCEPTIONS
//
ModSize
MiddleBaseListIterator::getLocDataOffset() const
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::MiddleBaseListIterator::getLocDataLength --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ModSize
MiddleBaseListIterator::getLocDataLength() const
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::MiddleBaseListIterator::getFreeLocationListIterator --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	ModInvertedLocationListIterator*
//
//	EXCEPTIONS
//
ModInvertedLocationListIterator*
MiddleBaseListIterator::getFreeLocationListIterator() const
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::MiddleBaseListIterator::setFreeLocationListIterator --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
MiddleBaseListIterator::setFreeLocationListIterator(ModInvertedLocationListIterator* ite_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	Copyright (c) 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
