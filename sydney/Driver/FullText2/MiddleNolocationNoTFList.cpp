// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleNolocationNoTFList.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#include "FullText2/FakeError.h"
#include "FullText2/InvertedUnit.h"
#include "FullText2/InvertedIterator.h"
#include "FullText2/MiddleNolocationNoTFList.h"
#include "FullText2/MiddleNolocationNoTFListIterator.h"
#include "FullText2/OverflowFile.h"

#include "Exception/Unexpected.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//  FUNCTION public
//  FullText2::MiddleNolocationNoTFList::MiddleNolocationNoTFList
//		-- コンストラクタ(1)
//
//  NOTES
//  ShortListと違って、LeafPageに該当するエリアがない場合は
//	MiddleNolocationNoTFListを構築することはできない。
//
//  ARGUMENTS
//  FullText2::InvertedUnit& cInvertedUnit_
//	  転置ファイル
//  FullText2::LeafPage::PagePointer pLeafPage_
//	  リーフページ
//  FullText2::LeafPage::Iterator ite_
//	  該当する索引単位のエリアへのイテレータ
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
MiddleNolocationNoTFList::MiddleNolocationNoTFList(
	InvertedUnit& cInvertedUnit_,
	LeafPage::PagePointer pLeafPage_,
	LeafPage::Iterator ite_)
	: MiddleBaseList(cInvertedUnit_, pLeafPage_, ite_)
{
}

//
//  FUNCTION public
//  FullText2::MiddleNolocationNoTFList::MiddleNolocationNoTFList
//		-- コンストラクタ(2)
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::InvertedUnit& cInvertedUnit_
//	  転置ファイル
//  FullText2::LeafPage::Area* pTmpArea_
//	  一時的なエリア
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
MiddleNolocationNoTFList::MiddleNolocationNoTFList(InvertedUnit& cInvertedUnit_,
												   LeafPage::Area* pTmpArea_)
	: MiddleBaseList(cInvertedUnit_, pTmpArea_)
{
}

//
//  FUNCTION public
//  FullText2::MiddleNolocationNoTFList::~MiddleNolocationNoTFList
//		-- デストラクタ
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
MiddleNolocationNoTFList::~MiddleNolocationNoTFList()
{
}

//
//  FUNCTION public
//  FullText2::MiddleNolocationNoTFList::insert -- 転置リストの挿入(1文書単位)
//
//  NOTES
//
//  ARGUMENTS
//  ModUInt32 uiDocumentID_
//		挿入する文書の文書ID
//  const FullText2::SmartLocationList& cLocationList_
//		位置情報配列
//
//  RETURN
//  bool
//		挿入できた場合はtrue(MiddleNolocationNoTFListの範囲内であった)、
//		それ以外の場合はfalse
//
//  EXCEPTIONS
//
bool
MiddleNolocationNoTFList::insert(ModUInt32 uiDocumentID_,
								 const SmartLocationList& cLocationList_)
{
	// 最終IDBlock
	OverflowPage::IDBlock cIdBlock = (*m_ite)->getIDBlock();

	//
	// MiddleList等とは異なり、LOCページは存在しないので、チェック不要。
	//
	
	// 文書IDを書き込む
	if (insertDocumentID(uiDocumentID_, cIdBlock) == false)
		return false;

	; _FULLTEXT2_FAKE_ERROR(MiddleNolocationNoTFList::insert);

	return true;
}

//
//  FUNCTION public
//  FullText2::MiddleNolocationNoTFList::insert
//		-- 転置リストの挿入(転置リスト単位)
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::InvertedList& cInvertedList_
//	  転置リスト
//
//  RETURN
//  bool
//	  ミドルリストの範囲内の場合はtrue、それ以外の場合はfalse
//
//  EXCEPTIONS
//
bool
MiddleNolocationNoTFList::insert(InvertedList& cInvertedList_)
{
	const LeafPage::Area* pArea = cInvertedList_.getArea();
	
	if (pArea->getDocumentCount() == 0)
		// 挿入する転置リストが0件なので、終わり
		return true;

	//
	// 最終IDブロック
	//
	OverflowPage::IDBlock cIdBlock = (*m_ite)->getIDBlock();

	// 挿入する転置リストが、すでに挿入済みかどうかをチェックする
	if ((*m_ite)->getDocumentCount() != 0)
	{
		// すでに挿入済みかどうかをチェックする
		if ((*m_ite)->getLastDocumentID() >= pArea->getFirstDocumentID())
		{
			// 挿入するものは必ず大きな文書IDになっている
			// そうなっていないっていうことは、すでにマージ済みってこと
			
			return true;
		}
	}

	//
	// 挿入する
	//
	
	// イテレータを得る
	ModAutoPointer<InvertedIterator> i = cInvertedList_.getIterator();
	DocumentID id;
	
	while ((id = i->next()) != UndefinedDocumentID)
	{
		//
		// 文書IDを挿入する
		//
		if (insertDocumentID(id, cIdBlock) == false)
			
			// ミドルリストの範囲外
			// このアルゴリズムだとロングリストができた場合に
			// ただしく動作しないが、現状ロングリストはないので、どうでもいい。
			
			return false;
	}

	return true;
}

//
//  FUNCTION public
//  FullText2::MiddleNolocationNoTFList::getIterator
//		-- 転置リストイテレータを得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  FullText2::InvertedIterator*
//	  転置リストイテレータ
//
//  EXCEPTIONS
//
InvertedIterator*
MiddleNolocationNoTFList::getIterator()
{
	return new MiddleNolocationNoTFListIterator(*this);
}

//
//  FUNCTION private
//  FullText2::MiddleNolocationNoTFList::insertDocumentID -- 文書IDを書き出す
//
//  NOTES
//
//  ARGUMENTS
//  ModUInt32 uiDocumentID_
//	  文書ID
//  FullText2::OverflowPage::IDBlock& cIdBlock_
//	  最終IDブロック
//
//  RETURN
//  bool
//	  ミドルリストに範囲内の場合はtrue、それ以外の場合はfalse
//
//  EXCEPTIONS
//
bool
MiddleNolocationNoTFList::insertDocumentID(ModUInt32 uiDocumentID_,
										   OverflowPage::IDBlock& cIdBlock_)
{
	//
	//  最大文書IDを最終IDブロックに書き込む
	// [YET!] ほとんどMiddleBaseListと同じ
	//

	m_pLeafPage->dirty();

	//
	// 文書IDを書き込む
	//
	
	if ((*m_ite)->getDocumentCount() != 0 && cIdBlock_.isExpunge() == false)
	{
		// 最終IDブロックにIDが格納されている

		// 文書IDのビット長
		ModSize idLength = getCompressedBitLengthDocumentID(
			(*m_ite)->getLastDocumentID(), uiDocumentID_);

		if (cIdBlock_.getDataSize() < (*m_ite)->getDocumentOffset() + idLength)
		{
			// 最終IDBlockには入りきらない

			// 最終IDブロックをIDページに移動
			if (allocateIDBlock(cIdBlock_) == false)
				// 移動先の新たなIDブロックの確保に失敗(ミドルリストの範囲外)
				return false;

			// 最終IDブロックをuiDocumentID_で初期化する
			initializeLastBlock(cIdBlock_, uiDocumentID_);
		}
		else
		{
			// 入りきる

			// 文書IDの書き込み
			ModSize uiBitOffset = (*m_ite)->getDocumentOffset();
			writeDocumentID((*m_ite)->getLastDocumentID(), uiDocumentID_,
							cIdBlock_.getBuffer(), uiBitOffset);

			// ヘッダの更新
			(*m_ite)->setDocumentOffset(uiBitOffset);
		}
	}
	else
	{
		// 最終IDブロックにIDが格納されていない

		// 最終IDブロックを、uiDocumentID_で初期化する
		initializeLastBlock(cIdBlock_, uiDocumentID_);
	}

	//
	// Areaの更新
	//

	// 出現頻度
	(*m_ite)->incrementDocumentCount();
	// 最終文書ID
	(*m_ite)->setLastDocumentID(uiDocumentID_);

	return true;
}

//
//	FUNCTION protected
//	FullText2::MiddleNolocationNoTFList::expandLocBlock --
//		LOCブロックのデータ領域を拡張する
//
//  NOTES
//		拡張に必要な領域がLOCページに存在することが前提
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
void
MiddleNolocationNoTFList::expandLocBlock(OverflowPage::LocBlock& cLocBlock_,
										 ModSize uiInsertedLocBitLength_,
										 ModSize uiDataBitLength_,
										 ModSize uiDataUnitSize_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION protected
//	FullText2::MiddleNolocationNoTFList::expandLocBlock --
//		LOCブロックのデータ領域を拡張する
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
void
MiddleNolocationNoTFList::expandLocBlock(OverflowPage::PagePointer& pLocPage_,
										 OverflowPage::LocBlock& cLocBlock_,
										 ModSize uiInsertedLocBitLength_,
										 ModSize& uiDataBitLength_,
										 ModSize& uiDataUnitSize_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//  FUNCTION private
//  FullText2::MiddleNolocationNoTFList::allocateIDBlock
//		-- 最終IDブロックをIDページに移動する
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::OverflowPage::IDBlock& cIdBlock_
//	  最終IDブロック
//
//  RETURN
//
//  EXCEPTIONS
//
bool
MiddleNolocationNoTFList::allocateIDBlock(OverflowPage::IDBlock& cIdBlock_)
{
	// 移動先のIDページ
	OverflowPage::PagePointer pIDPage;
	if ((*m_ite)->getDirBlockCount() != 0)
	{
		// DIRブロックを得る
		LeafPage::DirBlock* pDirBlock = getLastDirBlock();
		// IDページを得る
		pIDPage = m_pOverflowFile->attachPage(pDirBlock->m_uiIDPageID);
	}
	else
	{
		// DIRブロック数が0

		//
		// DIRブロックが0ということは、IDブロックは最終IDブロックしか存在しない
		// つまりIDページは存在しない。
		//
		
		// IDページを確保する
		pIDPage = allocateIdPage(0);
		
		// copyIDBlock()で、DIRブロックに指されていることが前提
		addDirBlock(cIdBlock_);
		// addDirBlockでcIdBlock_が変わってもFirstDocumentIDは変わらない
		// OverflowPage::IDBlockはポインタを持っているので、
		// それがsplitによって変わってしまう可能性はあるが、
		// 最終IDブロック等のデータ自体は変わらない
		setToLastDirBlock(pIDPage->getID(),
						  cIdBlock_.getFirstDocumentID());
	}
		
	// 最終IDブロックを新しいIDブロックにコピーする
	if (copyIDBlock(pIDPage, cIdBlock_) == false)
		// ミドルリストの範囲外
		return false;

	// DIRブロックの削除フラグを落とす
	LeafPage::DirBlock* pDirBlock = getLastDirBlock();
	pDirBlock->unsetExpunge();

	return true;
}

//
//  FUNCTION private
//  FullText2::MiddleNolocationNoTFList::copyIDBlock
//		-- IDブロックを確保し、内容をコピーする
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::OveflowPage::PagePointer& pIdPage_
//	  IDページ
//  FullText2::OverflowPage::IDBlock& cLastBlock_
//	  最終IDブロック(コピー元)
//
//  RETURN
//  bool
//	  ミドルリストの範囲内の場合はtrue、それ以外の場合はfalse
//
//  EXCEPTIONS
//
bool
MiddleNolocationNoTFList::copyIDBlock(OverflowPage::PagePointer& pIdPage_,
									  OverflowPage::IDBlock& cLastBlock_)
{
	//
	// 新たなIDブロックを確保し、そこに今までの最終IDブロックをコピーする
	//

	//
	// コピー先のIDブロックを確保
	//
	OverflowPage::IDBlock cNewIdBlock;
	cNewIdBlock = pIdPage_->allocateIDBlock();
	
	while (cNewIdBlock.isInvalid() == true)
	{
		// もうこれ以上IDブロックを置けない

		//
		// IDブロックを確保できなかったので、新たなIDページを確保
		//
		OverflowPage::PagePointer pNewPage = allocateIdPage(pIdPage_);

		//
		// DIRブロックを確保し、DIRブロックを新たなIDページで初期設定
		//

		// DIRブロックを追加する
		if (addDirBlock(cLastBlock_) == false)
		{
			// ミドルリストでは格納できない
			m_pOverflowFile->freePage(pNewPage);
			return false;
		}
		
		// 最終DIRブロックのデータを初期設定
		setToLastDirBlock(pNewPage->getID(),
						  cLastBlock_.getFirstDocumentID());

		//
		// コピー先のIDブロックを再確保
		//
		pIdPage_ = pNewPage;
		cNewIdBlock = pIdPage_->allocateIDBlock();
	}

	//
	// コピーできるIDブロックが見つかった
	//

	// 最終IDブロックの内容をコピーする
	cNewIdBlock.copy(cLastBlock_);

	return true;
}

//
//  FUNCTION private
//  FullText2::MiddleNolocationNoTFList::initalizeLastBlock
//		-- 最終IDブロック関係を初期化する
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::OverflowPage::IDBlock& cIdBlock_
//	  最終IDブロック
//  ModUInt32 uiDocumentID_
//	  先頭文書ID
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
MiddleNolocationNoTFList::initializeLastBlock(OverflowPage::IDBlock& cIdBlock_,
											  ModUInt32 uiDocumentID_)
{
	// 最終IDブロック関係
	cIdBlock_.clear();
	cIdBlock_.setFirstDocumentID(uiDocumentID_);

	// エリア関係
	(*m_ite)->setDocumentOffset(0);
}

//
//  FUNCTION private
//  FullText2::MiddleNolocationNoTFList::initialize -- 自身を初期化する
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
void
MiddleNolocationNoTFList::initialize()
{
	// 初期化する
	(*m_ite)->clear();
	
	m_pLeafPage->dirty();
}

//
//  FUNCTION private
//  FullText2::MiddleNolocationNoTFList::prepareForLocation
//		-- 位置情報や位置リストを挿入する準備をする
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
void
MiddleNolocationNoTFList::prepareForLocation(
	OverflowPage::PagePointer& pLocPage_,
	ModSize uiUnitSize_,
	ModSize uiNewUnitSize_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION protected
//	FullText2::MiddleNolocationNoTFList::makeTmpMiddleList
//		-- 一時的なミドルリストを得る
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	FullText2::LeafPage::Area* pTmpArea_
//		一時エリア
//
//	RETURN
//	FullText2::MiddleBaseList*
//		ミドルリスト
//
//	EXCEPTIONS
//
MiddleBaseList*
MiddleNolocationNoTFList::makeTmpMiddleList(InvertedUnit& cInvertedUnit_,
											LeafPage::Area* pTmpArea_)
{
	return new MiddleNolocationNoTFList(cInvertedUnit_, pTmpArea_);
}

//
//  Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
