// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleBaseList.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
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

#include "FullText2/FakeError.h"
#include "FullText2/InvertedIterator.h"
#include "FullText2/InvertedUnit.h"
#include "FullText2/MessageAll_Class.h"
#include "FullText2/MiddleBaseList.h"
#include "FullText2/MiddleBaseListIterator.h"
#include "FullText2/OverflowFile.h"

#include "Common/Assert.h"
#include "Exception/Unexpected.h"
#include "PhysicalFile/Page.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//  FUNCTION public
//  FullText2::MiddleBaseList::MiddleBaseList -- コンストラクタ
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::InvertedUnit& cInvertedUnit_
//		転置ファイルユニット
//  FullText2::LeafPage::PagePointer pLeafPage_
//		リーフページ
//  FullText2::LeafPage::Iterator ite_
//		該当する索引単位のエリアへのイテレータ
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
MiddleBaseList::MiddleBaseList(InvertedUnit& cInvertedUnit_,
							   LeafPage::PagePointer pLeafPage_,
							   LeafPage::Iterator ite_)
	: InvertedList(cInvertedUnit_,
				   (*ite_)->getKey(),
				   (*ite_)->getKeyLength(),
				   ListType::Middle,
				   pLeafPage_,
				   ite_),
	  m_cInvertedUnit(cInvertedUnit_), m_pTmpArea(0)
{
	m_pOverflowFile = cInvertedUnit_.getOverflowFile();
}

//
//  FUNCTION public
//  FullText2::MiddleBaseList::MiddleBaseList -- コンストラクタ
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::InvertedUnit& cInvertedUnit_
//		転置ファイルユニット
//  FullText2::LeafPage::PagePointer pLeafPage_
//		リーフページ
//  FullText2::LeafPage::Iterator ite_
//		該当する索引単位のエリアへのイテレータ
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
MiddleBaseList::MiddleBaseList(InvertedUnit& cInvertedUnit_,
							   LeafPage::Area* pTmpArea_)
	: InvertedList(cInvertedUnit_,
				   pTmpArea_->getKey(),
				   pTmpArea_->getKeyLength(),
				   ListType::Middle),
	  m_cInvertedUnit(cInvertedUnit_), m_pTmpArea(pTmpArea_)
{
	m_pOverflowFile = cInvertedUnit_.getOverflowFile();
}

//
//  FUNCTION public
//  FullText2::MiddleBaseList::~MiddleBaseList -- デストラクタ
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
MiddleBaseList::~MiddleBaseList()
{
	if (m_pTmpArea) delete m_pTmpArea;
}

//
//  FUNCTION public
//  FullText2::MiddleBaseList::insert -- 転置リストの挿入(1文書単位)
//
//  NOTES
//
//  ARGUMENTS
//  ModUInt32 uiDocumentID_
//	  挿入する文書の文書ID
//  const FullText2::SmartLocationList& cLocationList_
//	  位置情報配列
//
//  RETURN
//  bool
//	  挿入できた場合はtrue(MiddleListの範囲内であった)、それ以外の場合はfalse
//
//  EXCEPTIONS
//
bool
MiddleBaseList::insert(ModUInt32 uiDocumentID_,
					   const FullText2::SmartLocationList& cLocationList_)
{
	// 対応するLocページ
	OverflowPage::PagePointer pLocPage;
	// 対応するLocBlock
	OverflowPage::LocBlock cLocBlock;

	//
	// 最終IDブロック
	//
	OverflowPage::IDBlock cIdBlock = (*m_ite)->getIDBlock();
	
	//
	// LOCブロック
	//
	if ((*m_ite)->getDocumentCount() == 0 &&
		(*m_ite)->getLastLocationPageID()
		!= PhysicalFile::ConstValue::UndefinedPageID)
	{
		// IDブロックの削除に失敗している -> クリアする
		freeLastLocPage();
	}

	if ((*m_ite)->getDocumentCount() != 0)
	{
		// attachする
		pLocPage = m_pOverflowFile->attachPage(
			(*m_ite)->getLastLocationPageID());
		// LOCブロックを得る
		cLocBlock = pLocPage->getLocBlock(
			static_cast<unsigned short>((*m_ite)->getLocationOffset()));
	}
	else
	{
		// 一件も格納されていない。

		// 初回挿入時の最終LOCページは、IDLOCページ

		// ID-LOCページを確保する
		pLocPage = allocateIdLocPage(0);
		// LOCブロックを得る
		cLocBlock = allocateLocBlock(pLocPage);
	}

	//
	//  文書IDを書き込む
	//
	
	if (insertDocumentID(uiDocumentID_, cIdBlock, pLocPage, cLocBlock) == false)
		// ミドルリストの範囲外
		return false;

	//
	//  位置情報を書き込む
	//
	insertLocation(cLocationList_, pLocPage, cLocBlock);

	return true;
}

//
//  FUNCTION public
//  FullText2::MiddleBaseList::insert -- 転置リストの挿入(転置リスト単位)
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
MiddleBaseList::insert(InvertedList& cInvertedList_)
{
	const LeafPage::Area* pArea = cInvertedList_.getArea();
	
	if (pArea->getDocumentCount() == 0)
		// 挿入する転置リストが0件なので、終わり
		return true;
	
	// 対応するLocページ
	OverflowPage::PagePointer pLocPage;
	// 対応するLocBlock
	OverflowPage::LocBlock cLocBlock;
	
	//
	// 最終IDブロック
	//
	OverflowPage::IDBlock cIdBlock = (*m_ite)->getIDBlock();

	//
	// LOCブロック
	//
	if ((*m_ite)->getDocumentCount() == 0 &&
		(*m_ite)->getLastLocationPageID()
		!= PhysicalFile::ConstValue::UndefinedPageID)
	{
		// IDブロックの削除に失敗している -> クリアする
		freeLastLocPage();
	}

	// イテレータを得る
	ModAutoPointer<InvertedIterator> i = cInvertedList_.getIterator();
	// 先頭を得る
	DocumentID id = i->next();

	if ((*m_ite)->getDocumentCount() != 0)
	{
		// すでに挿入済みかどうかをチェックする
		if ((*m_ite)->getLastDocumentID() >= id)
		{
			// 挿入するものは必ず大きな文書IDになっている
			// そうなっていないっていうことは、すでにマージ済みってこと
			
			return true;
		}

		// attachする
		pLocPage = m_pOverflowFile->attachPage(
			(*m_ite)->getLastLocationPageID());
		// LOCブロックを得る
		cLocBlock = pLocPage->getLocBlock(
			static_cast<unsigned short>((*m_ite)->getLocationOffset()));
	}
	else
	{
		//
		// 一件も格納されていない。
		// LOCページではなく、IDLOCページを確保する。
		//
		
		// ID-LOCページを確保する
		pLocPage = allocateIdLocPage(0);
		// LOCブロックを得る
		cLocBlock = allocateLocBlock(pLocPage);
	}

	//
	// 挿入する
	//

	do
	{
		//
		//  文書IDを挿入する
		//
		if (insertDocumentID(id, cIdBlock, pLocPage, cLocBlock)
			== false)
			
			// ミドルリストの範囲外
			// このアルゴリズムだとロングリストができた場合に
			// ただしく動作しないが、現状ロングリストはないので、どうでもいい。
			
			return false;

		//
		//  位置情報を挿入する
		//
		insertLocation(*i, pLocPage, cLocBlock);
	}
	while ((id = i->next()) != UndefinedDocumentID);

	return true;
}

//
//  FUNCTION public
//  FullText2::MiddleBaseList::verify -- 整合性検査を行う
//
//  NOTES
//
//  ARGUMENTS
//  Admin::Verification::Treatment::Value uiTreatment_
//	  不整合発見時の動作
//  Admin::Verification::Progress& cProgress_
//	  不整合を通知するストリーム
//  const Os::Path& cRootPath_
//	  転置ファイルのルートパス
//
//  RETURN
//  なし
//
//  EXCEPTINS
//
void
MiddleBaseList::verify(Admin::Verification::Treatment::Value uiTreatment_,
					   Admin::Verification::Progress& cProgress_,
					   const Os::Path& cRootPath_)
{
	//
	// 全てのLOCブロックを取得できることと、
	// 余計なLOCブロックが存在しないことを確認する。
	//
	
	//
	// DIRブロックごとに、IDブロックとLOCブロックを確認する
	//

	LeafPage::DirBlock* pDirBlock = (*m_ite)->getDirBlock();
	int i = 0;
	for (; i < (*m_ite)->getDirBlockCount(); ++i)
	{
		// DIRブロックを得る
		LeafPage::DirBlock* pDir = pDirBlock + i;

		// IDページをアタッチする
		OverflowPage::PagePointer pIdPage;
		try
		{
			pIdPage = m_pOverflowFile->attachPage(pDir->getPageID());
		}
		catch (...)
		{
			_SYDNEY_VERIFY_INCONSISTENT(cProgress_,
										cRootPath_,
										Message::IllegalIdPage(
											pDir->getPageID()));
			_SYDNEY_RETHROW;
		}

		//
		// IDブロックとLOCブロックを確認する
		//
		int j = 0;
		for (; j < static_cast<int>(pIdPage->getIDBlockCount()); ++j)
		{
			// IDブロックを得る
			OverflowPage::IDBlock cIdBlock = pIdPage->getIDBlock(j);

			if (cIdBlock.isExpunge() == true)
			{
				//
				// 削除フラグが立っているIDブロックは存在しないはず
				//
				// InvertedFile::expungeで、ListManager::expungeを実行し、
				// 最終的にはMiddleBaseListIterator::expungeFirstDocumentIDで、
				// 削除フラグが立つ。
				// その後、InvertedFile::expunge内で、
				// ListManager::expungeIdBlockを実行し、
				// 最終的にはMiddleBaseListIterator::expungeIdBlockで、
				// 実際に削除されるため。
				//
				
				_SYDNEY_VERIFY_CORRECTABLE(
					cProgress_, cRootPath_,
					Message::DisusedIdBlock(cIdBlock.getFirstDocumentID()));
				// 削除されている
				m_cInvertedUnit.enterDeleteIdBlock(
					getKey(),
					cIdBlock.getFirstDocumentID());
			}

			// LOCブロックの整合性を検査する。
			OverflowPage::PagePointer pLocPage;
			verifyLocBlock(uiTreatment_, cProgress_, cRootPath_,
						   cIdBlock, pLocPage);
		}
	}

	//
	// 最終IDブロックとそのLOCブロックを確認する
	//

	{
		// 最終IDBlock
		OverflowPage::IDBlock cIdBlock = (*m_ite)->getIDBlock();

		if (cIdBlock.isExpunge() == false)
		{
			// 最終IDBlockが存在しているので、
			// IDBlockから整合性検査を実施する

			// LOCブロックの整合性を検査する
			OverflowPage::PagePointer pLocPage;
			verifyLocBlock(uiTreatment_, cProgress_, cRootPath_,
						   cIdBlock, pLocPage);

			//
			// 後処理 (LOCページが残っているかもしれない)
			//
			verifyNextLocPage(uiTreatment_, cProgress_, cRootPath_, pLocPage);
		}
		else
		{
			// 最終IDBlockは削除されているが、
			// 最終LOCブロックは存在しているので、
			// LOCブロックのみ整合性検査を行う

			if ((*m_ite)->getLastLocationPageID()
				!= PhysicalFile::ConstValue::UndefinedPageID)
			{
			
				OverflowPage::PagePointer pLocPage
					= m_pOverflowFile->attachPage(
						(*m_ite)->getLastLocationPageID());

				// 最終LOCブロックの整合性を検査する
				verifyNextLocPage(uiTreatment_, cProgress_, cRootPath_,
								  pLocPage);
			}
		}
	}
}

//
//	FUNCTION public
//	FullText2::MiddleBaseList::getArea -- エリアを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::LeafPage::Area*
//	const FullText2::Leafpage::Area*
//		エリア
//
//	EXCEPTIONS
//
LeafPage::Area*
MiddleBaseList::getArea()
{
	return (m_pTmpArea != 0) ? m_pTmpArea : *m_ite;
}
const LeafPage::Area*
MiddleBaseList::getArea() const
{
	return (m_pTmpArea != 0) ? m_pTmpArea : *m_ite;
}

//
//  FUNCTION public
//  FullText2::MiddleBaseList::convert -- ロングリストに変換する
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  FullText2::InvertedList*
//	  ロングリスト
//
//  EXCEPTIONS
//
InvertedList*
MiddleBaseList::convert()
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION public
//	FullText2::MiddleBaseList::vacuum
//		-- バキュームする
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
MiddleBaseList::vacuum()
{
	// バキュームするのはMiddleListだけ
	//
	// バキューム以前にすべて削除された場合は、
	// ShortListにコンバートされている
	
	if ((*m_ite)->getListType() != ListType::Middle)
		return;
	
	m_pLeafPage->dirty();
	
	//	バキュームは以下の手順で実施する
	//
	//	1. LeafPageのエリアをメモリ上にコピーし、ダミーのMiddleListを作成する
	//	2. 今のエリアを初期化する(サイズも小さくする)
	//	3. ダミーのMiddleListをすべてインサートする(マージと同じ処理)
	//	4. ダミーが保持しているOverflowFileの全ページを解放する

	// エリアをコピーする
	LeafPage::Area* pArea
		= LeafPage::Area::allocateArea(getKey(),
									   (*m_ite)->getDataUnitSize());
	pArea->copy(*m_ite);

	// ダミーのMiddleListを作成する
	ModAutoPointer<MiddleBaseList> pDummyList
		= makeTmpMiddleList(m_cInvertedUnit, pArea);

	// エリアサイズを変更する
	ModSize uiAreaSize = LeafPage::calcAreaUnitSize(
		(*m_ite)->getKeyLength(),
		InvertedList::getIDBlockUnitSize(getKey()));
	int change = static_cast<int>(uiAreaSize) - (*m_ite)->getUnitSize();
	m_pLeafPage->changeAreaSize(m_ite, change);
	
	// エリアを初期化する
	(*m_ite)->clear();
	(*m_ite)->setListType(ListType::Middle);
	(*m_ite)->setLastLocationPageID(PhysicalFile::ConstValue::UndefinedPageID);

	// ダミーのMiddleListを挿入する
	insert(*pDummyList);

	// ダミーのMiddleListが保持するOverflowFileのページを解放する
	pDummyList->freeAllPages();
}

//
//  FUNCTION public
//  FullText2::MiddleBaseList::expungeIdBlock -- IDブロックを削除する
//
//  NOTES
//
//  ARGUMENTS
//  ModUInt32 vecFirstDocumentID_
//	  削除するIDブロックの先頭文書ID
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
MiddleBaseList::expungeIdBlock(const ModVector<ModUInt32>& vecFirstDocumentID_)
{
	//
	// IDブロックを一つずつ削除
	// IDブロックの識別には、IDブロックの先頭文書IDが使われる。
	//
	
	ModVector<ModUInt32>::ConstIterator j = vecFirstDocumentID_.begin();
	for (; j != vecFirstDocumentID_.end(); ++j)
	{
		ModAutoPointer<MiddleBaseListIterator> i =
			_SYDNEY_DYNAMIC_CAST(MiddleBaseListIterator*, getIterator());
		bool result = (*i).find(*j, true);
		if (result == false)
			// 最終ブロックがマージ中に再利用された場合
			continue;
		
		if ((*i).expungeIdBlock() == true)
		{
			// DIRブロックを1つ削除する
			m_pLeafPage->changeAreaSize(m_ite,
										-static_cast<int>(
											sizeof(LeafPage::DirBlock)
											/ sizeof(ModUInt32)));

			// m_pLeafPage->dirty()は不要。changeAreaSize内で実行される。
		}
	}

	//
	// 後処理
	//
	
	if ((*m_ite)->getDocumentCount() == 0)
	{
		//  件数が0

		// 初期化する
		initialize();
		// Shortリストにする
		(*m_ite)->setListType(ListType::Short);
	}
}

//
//  FUNCTION protected
//  FullText2::MiddleBaseList::insertDocumentID -- 文書IDを書き出す
//
//  NOTES
//
//  ARGUMENTS
//  ModUInt32 uiDocumentID_
//	  文書ID
//  FullText2::OverflowPage::IDBlock& cIdBlock_
//	  最終IDブロック
//  FullText2::OverflowPage::PagePointer& pLocPage_
//	  対応するロックページ
//  FullText2::OverflowPage::LocBlock& cLocBlock_
//	  対応するロックブロック
//
//  RETURN
//  bool
//	  ミドルリストに範囲内の場合はtrue、それ以外の場合はfalse
//
//  EXCEPTIONS
//
bool
MiddleBaseList::insertDocumentID(ModUInt32 uiDocumentID_,
								 OverflowPage::IDBlock& cIdBlock_,
								 OverflowPage::PagePointer& pLocPage_,
								 OverflowPage::LocBlock& cLocBlock_)
{
	//
	//  最大文書IDを最終IDブロックに書き込む
	//

	// 文書IDの挿入方法
	// ・最終IDブロックに入るなら、そのまま挿入
	// ・最終IDブロックに空きがない場合、
	// 		最終IDブロックを、最終DIRブロックが指すIDページに移動
	//		最終IDブロックが空いたところで、挿入
	// ・最終DIRブロックが指すページに空きがない場合、
	//		新規にIDLOCページを作成
	//		元ページのIDLOCからIDを新ページに移動
	//		元ページをLOCページに変更
	//		最終DIRブロックが指すページを新ページに更新
	//		最終IDブロックを、新ページに移動
	//		最終IDブロックが空いたところで、挿入
	// ・元ページがID/LOCページだった場合、
	//		新規にID/IDLOCページを作成
	//		DIRブロックを追加
	//		新DIRブロックが指すページに新ページを設定
	//		最終IDブロックを、新ページに移動
	//		最終IDブロックが空いたところで、挿入
	// ・そもそもDIRブロックが存在しない場合、
	//		DIRブロックを追加
	//		新規にIDLOCページを作成
	//		元ページのIDLOCからIDを新ページに移動
	//		元ページをLOCページに変更
	//		最終DIRブロックが指すページを新ページに更新
	//		最終IDブロックを、新ページに移動
	//		最終IDブロックが空いたところで、挿入
	// ・DIRブロックが存在せず、元ページがLOCページだった場合
	//		新規にIDLOCページを作成
	//		DIRブロックを追加
	//		新DIRブロックが指すページに新ページを設定
	//		最終IDブロックを、新ページに移動
	//		最終IDブロックが空いたところで、挿入

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
			if (allocateIDBlock(cIdBlock_, pLocPage_, cLocBlock_) == false)
				// 移動先の新たなIDブロックの確保に失敗(ミドルリストの範囲外)
				return false;

			// 最終IDブロックをuiDocumentID_で初期化する
			initializeLastBlock(cIdBlock_, uiDocumentID_,
								pLocPage_, cLocBlock_);
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
		initializeLastBlock(cIdBlock_, uiDocumentID_, pLocPage_, cLocBlock_);
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
//  FUNCTION protected
//  FullText2::MiddleBaseList::allocateIdPage -- IDページを得る
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::OverflowPage::PagePointer pPrevPage_
//	  前方のページ
//
//  RETURN
//  FullText2::OverflowPage::PagePointer
//	  確保したIDページ
//
//  EXCEPTIONS
//
OverflowPage::PagePointer
MiddleBaseList::allocateIdPage(OverflowPage::PagePointer pPrevPage_)
{
	ModSize uiBlockSize = InvertedList::getIDBlockUnitSize(getKey());
	
	return m_pOverflowFile->allocatePage(uiBlockSize);
}

//
//  FUNCTION protected
//  FullText2::MiddleBaseList::allocateLocBlock -- LOCブロックを確保する
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::OverflowPage::PagePointer& pLocPage_
//	  LOCページ
//
//  RETURN
//  FullText2::OverflowPage::LocBlock
//	  新しく確保されたLOCブロック
//
//  EXCEPTIONS
//
OverflowPage::LocBlock
MiddleBaseList::allocateLocBlock(OverflowPage::PagePointer& pLocPage_)
{
	//
	// 新たなLOCブロックを確保する
	//
	
	// あたらしいLOCブロックを得る
	OverflowPage::LocBlock cLocBlock = pLocPage_->allocateLocBlock();
	
	while (cLocBlock.isInvalid() == true)
	{
		// もうこれ以上LOCブロックを置けない

		//
		// LOCブロックを確保できなかったので、新たなLOCページを確保
		//
		if (pLocPage_->getType() == OverflowPage::Type::IDLOC)
		{
			// 今まではIDLOCページだった
			
			// LOCページに変換する(IDブロックを移動して空きを作る)
			convertToLocPage(pLocPage_);
		}
		else if (pLocPage_->getNextPageID()
				 != PhysicalFile::ConstValue::UndefinedPageID)
		{
			// 次のLOCページが存在する
			
			// 次のLOCページをアタッチする(移動する)
			pLocPage_ = m_pOverflowFile->attachPage(
				pLocPage_->getNextPageID());
		}
		else
		{
			// LOC専用ページを新たに確保する
			pLocPage_ = allocateLocPage(pLocPage_);
		}

		//
		// あたらしいLOCブロックを再確保
		//
		cLocBlock = pLocPage_->allocateLocBlock();
	}

	//
	// Areaを更新
	//
	(*m_ite)->setLastLocationPageID(pLocPage_->getID());
	(*m_ite)->setLocationOffset(cLocBlock.getOffset());

	//
	// cLocBlockが確保できた場合、
	// pLocPage_はallocateLocBlock内でdirtyにされている。
	//

	return cLocBlock;
}

//
//	FUNCTION protected
//	FullText2::MiddleBaseList::expandLocBlock --
//		LOCブロックのデータ領域を拡張する
//
//  NOTES
//		拡張に必要な領域がLOCページに存在することが前提
//
//  ARGUMENTS
//	OverflowPage::LocBlock& cLocBlock_,
//		ロックブロック
//	ModSize uiInsertedLocBitLength_
//		挿入される位置情報のビット長
//	ModSize uiDataBitLength_
//		挿入前の位置情報のビット長
//	ModSize uiDataUnitSize_
//		挿入前の位置情報のユニットサイズ
//
//  RETURN
//
//  EXCEPTIONS
//
void
MiddleBaseList::expandLocBlock(OverflowPage::LocBlock& cLocBlock_,
							   ModSize uiInsertedLocBitLength_,
							   ModSize uiDataBitLength_,
							   ModSize uiDataUnitSize_)
{
	int expand =
		calcUnitSize(uiInsertedLocBitLength_ + uiDataBitLength_)
		- uiDataUnitSize_;
	if (expand > 0)
	{
		// 拡張が必要
		cLocBlock_.expandUnitSize(expand);
	}
}

//
//	FUNCTION protected
//	FullText2::MiddleBaseList::expandLocBlock --
//		LOCブロックのデータ領域を拡張する
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::OverflowPage::PagePoiner& pLocPage_
//		ロックページ
//	OverflowPage::LocBlock& cLocBlock_,
//		ロックブロック
//	ModSize uiInsertedLocBitLength_
//		挿入される位置情報のビット長
//	ModSize& uiDataBitLength_
//		挿入前の位置情報のビット長
//	ModSize& uiDataUnitSize_
//		挿入前の位置情報のユニットサイズ
//
//  RETURN
//
//  EXCEPTIONS
//
void
MiddleBaseList::expandLocBlock(OverflowPage::PagePointer& pLocPage_,
							   OverflowPage::LocBlock& cLocBlock_,
							   ModSize uiInsertedLocBitLength_,
							   ModSize& uiDataBitLength_,
							   ModSize& uiDataUnitSize_)
{
	// 拡張するユニット数を求める
	int expand =
		calcUnitSize(uiInsertedLocBitLength_ + uiDataBitLength_)
		- uiDataUnitSize_;
	
	if (expand > 0)
	{
		// 拡張する
		if (expandUnitSize(pLocPage_, cLocBlock_, expand) == false)
		{
			// 広げられなかった -> LOCブロックが新しくなっている
			uiDataUnitSize_ = cLocBlock_.getDataUnitSize();
			uiDataBitLength_ = cLocBlock_.getDataBitLength();
			expand = calcUnitSize(uiInsertedLocBitLength_ + uiDataBitLength_)
				- uiDataUnitSize_;
			
			// もう一度広げる -> 絶対広げられる
			expandUnitSize(pLocPage_, cLocBlock_, expand);
		}
	}

	//
	// expandUnitSize内で、pLocPage_->dirty() は実行されているはず。
	//
}

//
//  FUNCTION protected
//  FullText2::MiddleBaseList::getLastDirBlock -- 最終DIRブロックを取得する
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
LeafPage::DirBlock*
MiddleBaseList::getLastDirBlock()
{
	LeafPage::DirBlock* pDirBlock = (*m_ite)->getDirBlock();
	return pDirBlock + ((*m_ite)->getDirBlockCount() - 1);
}

//
//  FUNCTION protected
//  FullText2::MiddleBaseList::addDirBlock -- DIRブロックを拡張する
//
//  NOTES
//
//  ARGUMENTS
//  OverflowPage::IDBlock& cLastBlock_
//	  最終IDブロック
//
//  RETURN
//  bool
//	  拡張できた場合はtrue、それ以外の場合はfalse
//
//  EXCEPTIONS
//
bool
MiddleBaseList::addDirBlock(OverflowPage::IDBlock& cLastBlock_)
{
	//
	// DIRブロックのためにAreaを拡大するサイズを取得
	//
	ModSize expand = sizeof(LeafPage::DirBlock)/sizeof(ModUInt32);

	//
	// 拡大できるか確認
	//
	if ((*m_ite)->getUnitSize() + expand > m_pLeafPage->getMaxAreaUnitSize())
	{
		// ミドルリストでは格納できない
		return false;
	}

	//
	// 拡大する
	//
	if (m_pLeafPage->changeAreaSize(m_ite, expand) == false)
	{
		// 拡大できないのでsplit
		m_pLeafPage = m_pLeafPage->split(m_cInvertedUnit, expand, m_ite);
		m_pLeafPage->changeAreaSize(m_ite, expand);
		cLastBlock_ = (*m_ite)->getIDBlock();
	}
	
	return true;
}

//
//  FUNCTION protected
//  FullText2::MiddleBaseList::setToLastDirBlock
//		-- 最終DIRブロックのデータを設定する
//
//  NOTES
//
//  ARGUMENTS
//	PhysicalFile::PageID uiIDPageID_
//		IDページのページID
//	ModUInt32 uiDocumentID_
//		IDページで最小の文書ID
//
//  RETURN
//
//  EXCEPTIONS
//
void
MiddleBaseList::setToLastDirBlock(PhysicalFile::PageID uiIDPageID_,
								  ModUInt32 uiDocumentID_)
{
	LeafPage::DirBlock* pDirBlock = getLastDirBlock();
		
	pDirBlock->m_uiIDPageID = uiIDPageID_;
	pDirBlock->m_uiDocumentID = uiDocumentID_;
}
	
//
//  FUNCTION protected
//  FullText2::MiddleBaseList::initialize -- 自身を初期化する
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
MiddleBaseList::initialize()
{
	// 最終LOCページを開放する
	freeLastLocPage();
	// 初期化する
	(*m_ite)->clear();
	
	m_pLeafPage->dirty();
}

//
//  FUNCTION protected
//  FullText2::MiddleBaseList::prepareForLocation
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
MiddleBaseList::prepareForLocation(OverflowPage::PagePointer& pLocPage_,
								   ModSize uiUnitSize_,
								   ModSize uiNewUnitSize_)
{
	//
	// LOCページのスペースが不足していて、
	// それがID-LOCページなら、IDブロックを移動させる。
	//
	// 新たなLOCページの確保は、現在のLOCページを使い切ってから。
	// insertLocationを参照すること。
	//
	
	if (uiUnitSize_ + pLocPage_->getFreeUnitSize() < uiNewUnitSize_)
	{
		// 十分な空き領域がない
		
		if (pLocPage_->getType() == OverflowPage::Type::IDLOC)
		{
			// 今まではIDLOCページだった
			
			// LOCページに変換する(IDブロックを移動して空きを作る)
			convertToLocPage(pLocPage_);
		}
	}
}

//
//  FUNCTION protected
//  FullText2::MiddleBaseList::allocateLocPage -- LOCページを得る
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::OverflowPage::PagePointer pPrevPage_
//	  前方のページ
//
//  RETURN
//  FullText2::OverflowPage::PagePointer
//	  確保したLOCページ
//
//  EXCEPTIONS
//
OverflowPage::PagePointer
MiddleBaseList::allocateLocPage(OverflowPage::PagePointer pPrevPage_)
{
	PhysicalFile::PageID uiPrevPageID
		= PhysicalFile::ConstValue::UndefinedPageID;

	if (pPrevPage_)
	{
		uiPrevPageID = pPrevPage_->getID();
	}

	OverflowPage::PagePointer pNewPage
		= m_pOverflowFile->allocatePage(
			uiPrevPageID,
			PhysicalFile::ConstValue::UndefinedPageID);

	if (pPrevPage_)
	{
		pPrevPage_->setNextPageID(pNewPage->getID());
	}

	return pNewPage;
}

//
//  FUNCTION protected
//  FullText2::MiddleBaseList::allocateIdLocPage -- ID-LOCページを得る
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::OverflowPage::PagePointer pPrevPage_
//	  前方のページ
//
//  RETURN
//  FullText2::OverflowPage::PagePointer
//	  確保したID-LOCページ
//
//  EXCEPTIONS
//
OverflowPage::PagePointer
MiddleBaseList::allocateIdLocPage(OverflowPage::PagePointer pPrevPage_)
{
	ModSize uiBlockSize = 0;
	PhysicalFile::PageID uiPrevPageID
		= PhysicalFile::ConstValue::UndefinedPageID;
	uiBlockSize = InvertedList::getIDBlockUnitSize(getKey());

	if (pPrevPage_)
	{
		uiPrevPageID = pPrevPage_->getID();
	}

	OverflowPage::PagePointer pNewPage
		= m_pOverflowFile->allocatePage(
			uiBlockSize,
			uiPrevPageID,
			PhysicalFile::ConstValue::UndefinedPageID);

	if (pPrevPage_)
	{
		pPrevPage_->setNextPageID(pNewPage->getID());
	}

	return pNewPage;
}

//
//  FUNCTION protected
//  FullText2::MiddleBaseList::freeLastLocPage -- 最終LOCページを開放する
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
MiddleBaseList::freeLastLocPage()
{
	OverflowPage::PagePointer pLocPage =
		m_pOverflowFile->attachPage((*m_ite)->getLastLocationPageID());
	m_pOverflowFile->freePage(pLocPage);

	//
	// pLocPageのfreeに伴って、pLocPageの前後のページが修正される場合は、
	// OverflowPage::setPrevPageID等が呼ばれるが、
	// その中で変更されたページはdirtyにされている。
	//
}

//
//  FUNCTION protected
//  FullText2::MiddleBaseList::convertToLocPage
//		-- ID-LOCページをLOCページに変換する
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::OverflowPage::PagePointer pIdLocPage_
//	  変換するID-LOCページ
//
//  RETURN
//  FullText2::OverflowPage::PagePointer
//	  IDブロックをコピーしたあたらしいID-LOCページ
//
//  EXCEPTIONS
//
OverflowPage::PagePointer
MiddleBaseList::convertToLocPage(OverflowPage::PagePointer pIdLocPage_)
{
	//
	// 新たなID(or ID-LOC)ページを確保し、
	// 今までのID-LOCページのIDブロックをコピーし、
	// 今までのID-LOCページをLOCページにコンバートする。
	//

	// 今までのID-LOCページ内のIDブロックのコピー先
	OverflowPage::PagePointer pNewPage;

	if ((*m_ite)->getDirBlockCount() != 0)
	{
		//
		// DIRブロック数が0の場合、IDブロックは最終IDブロックしか存在しない。
		// したがって、今までのID-LOCページ内に移動すべきIDブロックは存在しない
		// setType(LOC)で十分。
		//
		
		//
		// 新たなID(or ID-LOC)ページを確保
		//
		if (pIdLocPage_->getIDBlockCount() * pIdLocPage_->getIDBlockSize() >
			pIdLocPage_->getPageUnitSize() / 2)
		{
			// 今までのページは、IDブロックの使用率が50%を超える
			
			// IDページを得る
			pNewPage = allocateIdPage(pIdLocPage_);
		}
		else
		{
			// ID-LOCページを得る
			pNewPage = allocateIdLocPage(pIdLocPage_);
		}

		// 新しいページにIDブロックをコピーする
		// 今までのID-LOCページのIDブロック部分の初期化はしていない
		// 初期化(IDブロックのヘッダのポインタを0)にするのは、setTypeで実行
		pNewPage->moveIDBlock(pIdLocPage_);

		//
		// DIRブロックを新たなID(or ID-LOC)ページで更新
		//
		setToLastDirBlock(pNewPage->getID());
	}

	//
	// 自身をLOC専用ページにする
	//
	pIdLocPage_->setType(OverflowPage::Type::LOC);

	//
	// pIdLocPage_はsetType内でdirtyにされる。
	//

	return pNewPage;
}

//
//  FUNCTION protected
//  FullText2::MiddleBaseList::allocateIDBlock
//		-- 最終IDブロックをIDページに移動する
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::OverflowPage::IDBlock& cIdBlock_
//	  最終IDブロック
//  FullText2::OverflowPage::PagePointer& pLocPage_
//	  対応するLOCページ
//  FullText2::OverflowPage::LocBlock& cLocBlock_
//	  対応するLOCブロック
//
//  RETURN
//
//  EXCEPTIONS
//
bool
MiddleBaseList::allocateIDBlock(OverflowPage::IDBlock& cIdBlock_,
								OverflowPage::PagePointer& pLocPage_,
								OverflowPage::LocBlock& cLocBlock_)
{
	//
	// 新たなIDブロックを最終IDブロックの位置に確保する。
	//

	// IDページを取得し、今までの最終IDブロックをそのIDページに移動する。

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

		// つまりIDブロックは最終IDブロックしか存在しないので、
		// IDページは存在しない。

		// 代わりに最終LOCページを得る。
		// copyIDBlock()で新しいIDLOCページを作成する際などに必要
		pIDPage = m_pOverflowFile->attachPage(
			(*m_ite)->getLastLocationPageID());

		if (pIDPage->getType() == OverflowPage::Type::IDLOC)
		{
			// 最終LOCページはIDLOC

			// DIRブロックを追加する
			// copyIDBlock()で、DIRブロックに指されていることが前提なため
			addDirBlock(cIdBlock_);
			// addDirBlockでcIdBlock_が変わってもFirstDocumentIDは変わらない
			// OverflowPage::IDBlockはポインタを持っているので、
			// それがsplitによって変わってしまう可能性はあるが、
			// 最終IDブロック等のデータ自体は変わらない
			setToLastDirBlock(pIDPage->getID(),
							  cIdBlock_.getFirstDocumentID());
		}
	}

	// 最終IDブロックを新しいIDブロックにコピーする
	if (copyIDBlock(pIDPage, cIdBlock_) == false)
		// ミドルリストの範囲外
		return false;

	// DIRブロックの削除フラグを落とす
	LeafPage::DirBlock* pDirBlock = getLastDirBlock();
	pDirBlock->unsetExpunge();

	// 変更されたので、あたらしいLOCブロックを得る
	cLocBlock_ = allocateLocBlock(pLocPage_);

	return true;
}

//
//  FUNCTION protected
//  FullText2::MiddleBaseList::copyIDBlock
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
MiddleBaseList::copyIDBlock(OverflowPage::PagePointer& pIdPage_,
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
		// IDブロックを確保できなかったので、新たなID(or ID-LOC)ページを確保
		//
		OverflowPage::PagePointer pNewPage;
		if (pIdPage_->getType() & OverflowPage::Type::ID &&
			pIdPage_->getIDBlockCount() * pIdPage_->getIDBlockSize() >
			pIdPage_->getPageUnitSize() / 2)
		{
			// 今までのページは、ID専用ページかつページ使用率が50%を超えている
			
			// ID専用ページを得る
			pNewPage = allocateIdPage(pIdPage_);
		}
		else
		{
			// ID-LOCページを得る
			pNewPage = allocateIdLocPage(pIdPage_);
		}

		//
		// DIRブロックを確保し、DIRブロックを新たなID(or ID-LOC)ページで初期設定
		//
		if (pIdPage_->getType() == OverflowPage::Type::IDLOC)
		{
			// 今までのページはIDLOCページ
			
			// IDブロックとLOCブロックを分けることによって、
			// IDブロックだけのページは、LOCブロックが使っていたスペースが
			// 使えるようになる
			
			// 新しいページにIDブロックを移動
			pNewPage->moveIDBlock(pIdPage_);
			// 自身をLOC専用ページにする
			pIdPage_->setType(OverflowPage::Type::LOC);

			// DIRブロックの追加は不要
			// DIRブロックが一件もない場合、allocateIDBlockで追加済み
			// DIRブロックがある場合は、今までIDLOCページを指していたので、
			// それを新ページを指すように更新すればよい。
			
			// 最終DIRブロックを更新
			setToLastDirBlock(pNewPage->getID());
		}
		else
		{
			// IDページだけでなく、LOCページの場合もある
			// allocateIDBlockを参照のこと

			// いずれにせよ、今までのページがIDLOCページだった場合とは
			// 処理が異なる。
			// 今までのページがIDページの場合
			// IDブロックを移動する意味はない。
			// 今までのページがLOCページの場合
			// そもそもIDブロックが存在しないので移動しようがない。

			// DIRブロックを追加する
			if (addDirBlock(cLastBlock_) == false)
			{
				// ミドルリストでは格納できない
				m_pOverflowFile->freePage(pNewPage);
				return false;
			}
			
			// 最終DIRブロックを初期設定
			setToLastDirBlock(pNewPage->getID(),
							  cLastBlock_.getFirstDocumentID());
		}

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
//  FUNCTION protected
//  FullText2::MiddleBaseList::initializeLastBlock
//		-- 最終IDブロック関係を初期化する
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::OverflowPage::IDBlock& cIdBlock_
//	  最終IDブロック
//  ModUInt32 uiDocumentID_
//	  先頭文書ID
//  FullText2::OverflowPage::PagePointer& pLocPage_
//	  対応するLOCページ
//  FullText2::OverflowPage::LocBlock& cLocBlock_
//	  対応するLOCブロック
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
MiddleBaseList::initializeLastBlock(OverflowPage::IDBlock& cIdBlock_,
									ModUInt32 uiDocumentID_,
									OverflowPage::PagePointer& pLocPage_,
									OverflowPage::LocBlock& cLocBlock_)
{
	// 最終IDブロック関係
	cIdBlock_.clear();
	cIdBlock_.setFirstDocumentID(uiDocumentID_);
	cIdBlock_.setLocBlockPageID(pLocPage_->getID());
	cIdBlock_.setLocBlockOffset(cLocBlock_.getOffset());

	// エリア関係
	(*m_ite)->setDocumentOffset(0);
}

//
//  FUNCTION protected
//  FullText2::MiddleBaseList::setToLastDirBlock
//		-- 最終DIRブロックのデータを設定する
//
//  NOTES
//
//  ARGUMENTS
//	PhysicalFile::PageID uiIDPageID_
//		IDページのページID
//
//  RETURN
//
//  EXCEPTIONS
//
void
MiddleBaseList::setToLastDirBlock(PhysicalFile::PageID uiIDPageID_)
{
	LeafPage::DirBlock* pDirBlock = getLastDirBlock();
		
	pDirBlock->m_uiIDPageID = uiIDPageID_;
}

//
//  FUNCTION protected
//  FullText2::MiddleBaseList::insertLocation -- 位置情報を挿入する
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
MiddleBaseList::insertLocation(const SmartLocationList& cLocationList_,
							   OverflowPage::PagePointer& pLocPage_,
							   OverflowPage::LocBlock& cLocBlock_)
{
	// 位置情報の挿入方法
	// ・最終LOCページに入るなら、そのまま挿入
	// ・最終LOCページ(IDLOC)に一部しか入らない場合
	//		新たなID/IDLOCページを確保
	//		最終LOCページのIDブロックを新ページに移動
	//		DIRブロックを更新
	//		今までのIDLOCをLOCに変更
	//		新ページに挿入
	// ・IDブロックを移動しても一部しか入らない場合
	//		新たなID/IDLOCページを確保
	//		最終LOCページのIDブロックを新ページに移動
	//		DIRブロックを更新
	//		今までのIDLOCをLOCに変更
	//		LOCブロックを拡張
	//		挿入
	// ・LOCブロックを十分拡張できない場合
	//		新たなID/IDLOCページを確保
	//		最終LOCページのIDブロックを新ページに移動
	//		DIRブロックを更新
	//		今までのIDLOCをLOCに変更
	//		隣のLOCページに移動する
	//		新しいLOCブロックを得る
	//		挿入
	// ・隣のLOCページで十分なサイズのLOCブロックが得られない場合
	//		新たなID/IDLOCページを確保
	//		最終LOCページのIDブロックを新ページに移動
	//		DIRブロックを更新
	//		今までのIDLOCをLOCに変更
	//		新しいLOCページを得る
	//		新しいLOCブロックを得る
	//		挿入
	// ・最終LOCページ(IDLOC)に一部しか入らず、DIRブロックもない場合
	//		IDLOCページをLOCに変更
	//		LOCブロックを拡張 or 隣のLOCページに移動 or 新LOCページを得る
	//		...
	// ・最終LOCページ(LOC)に一部しか入らない場合
	//		LOCブロックを拡張 or 隣のLOCページに移動 or 新LOCページを得る
	//		...
		
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//  FUNCTION protected
//  FullText2::MiddleBaseList::insertLocation -- 位置情報を挿入する
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
MiddleBaseList::insertLocation(FullText2::InvertedIterator& ite_,
							   OverflowPage::PagePointer& pLocPage_,
							   OverflowPage::LocBlock& cLocBlock_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//  FUNCTION protected
//  FullText2::MiddleBaseList::expandUnitSize -- LOCブロックのユニットを広げる
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::OverflowPage::PagePoiner& pLocPage_
//	  ロックページ
//  FullText2::OverflowPage::LocBlock& cLocBlock_
//	  ロックブロック
//  ModSize uiExpandUnitSize_
//	  広げるユニット数
//
//  RETURN
//  bool
//	  広げられたらtrue、広げられずに新しいロックブロックが確保されたらfalse
//
bool
MiddleBaseList::expandUnitSize(OverflowPage::PagePointer& pLocPage_,
							   OverflowPage::LocBlock& cLocBlock_,
							   ModSize uiExpandUnitSize_)
{
	bool result = true;

	//
	// 準備
	//

	prepareForLocation(pLocPage_, uiExpandUnitSize_);

	//
	// pLocPage_->dirty()は不要。insertLocationでdirtyにされている。
	//

	if (pLocPage_->isLargeEnough(uiExpandUnitSize_) == false)
	{
		// 現在のページには入りきらない -> 新しいロックブロックを得る
		cLocBlock_.setContinueFlag();
		cLocBlock_ = allocateLocBlock(pLocPage_);
		result = false;
	}
	else
	{
		// ブロックを広げる
		cLocBlock_.expandUnitSize(uiExpandUnitSize_);
	}

	return result;
}

//
//  FUNCTION protected
//  FullText2::MiddleBaseList::verifyLocBlock --
//		LOCブロックの整合性検査を行う
//
//  NOTES
//
//  ARGUMENTS
//  Admin::Verification::Treatment::Value uiTreatment_
//	 	不整合発見時の動作
//  Admin::Verification::Progress& cProgress_
//	  	不整合を通知するストリーム
//  const Os::Path& cRootPath_
//	 	転置ファイルのルートパス
//	const OverflowPage::IDBlock& cIdBlock_
//		検査対象のLOCブロックに対応するIDブロック
//
//  RETURN
//  なし
//
//  EXCEPTINS
//
void
MiddleBaseList::verifyLocBlock(
	Admin::Verification::Treatment::Value uiTreatment_,
	Admin::Verification::Progress& cProgress_,
	const Os::Path& cRootPath_,
	const OverflowPage::IDBlock& cIdBlock_,
	OverflowPage::PagePointer& pLocPage_)
{
	// LOCページを得る
	try
	{
		pLocPage_ = m_pOverflowFile->attachPage(
			cIdBlock_.getLocBlockPageID());
	}
	catch (...)
	{
		_SYDNEY_VERIFY_INCONSISTENT(
			cProgress_, cRootPath_,
			Message::IllegalLocPage(cIdBlock_.getLocBlockPageID()));
		_SYDNEY_RETHROW;
	}

	// LOCブロックを得る
	OverflowPage::LocBlock cLocBlock =
		pLocPage_->getLocBlock(cIdBlock_.getLocBlockOffset());

	while (cLocBlock.isContinue())
	{
		//
		// LOCブロックが複数ページにまたがっていた場合、
		// 各ページのLOCブロックを取得できることを確認する
		//

		PhysicalFile::PageID uiPageID = pLocPage_->getNextPageID();
		try
		{
			pLocPage_ = m_pOverflowFile->attachPage(uiPageID);
		}
		catch (...)
		{
			_SYDNEY_VERIFY_INCONSISTENT(cProgress_, cRootPath_,
										Message::IllegalNextLocPage(uiPageID));
			_SYDNEY_RETHROW;
		}
		cLocBlock = pLocPage_->getLocBlock();
	}
}

//
//  FUNCTION protected
//  FullText2::MiddleBaseList::verifyNextLocPage --
//		最終LOCブロックの次のLOCページの整合性検査を行う
//
//  NOTES
//
//  ARGUMENTS
//  Admin::Verification::Treatment::Value uiTreatment_
//	 	不整合発見時の動作
//  Admin::Verification::Progress& cProgress_
//	  	不整合を通知するストリーム
//  const Os::Path& cRootPath_
//	 	転置ファイルのルートパス
//	const OverflowPage::PagePointer& pLocPage_
//		最終LOCブロックが格納されていたLOCページ
//
//  RETURN
//  なし
//
//  EXCEPTINS
//
void
MiddleBaseList::verifyNextLocPage(
	Admin::Verification::Treatment::Value uiTreatment_,
	Admin::Verification::Progress& cProgress_,
	const Os::Path& cRootPath_,
	OverflowPage::PagePointer& pLocPage_)
{
	; _SYDNEY_ASSERT(pLocPage_ != 0);
	
	while (pLocPage_->getNextPageID() !=
		   PhysicalFile::ConstValue::UndefinedPageID)
	{
		// まだLOCページが残っている
		
		// これが呼ばれる時は、すべてのLOCブロックのチェックは完了しているので、
		// 次ページのLOCブロックは0になるはず
		
		// LOCページを得る
		PhysicalFile::PageID uiPageID = pLocPage_->getNextPageID();
		try
		{
			pLocPage_ = m_pOverflowFile->attachPage(uiPageID);
		}
		catch (...)
		{
			_SYDNEY_VERIFY_INCONSISTENT(cProgress_, cRootPath_,
										Message::IllegalNextLocPage(uiPageID));
			_SYDNEY_RETHROW;
		}

		// LOCページの状態を確認する
		if (pLocPage_->getType() == OverflowPage::Type::ID)
		{
			// IDページが次ページであることはない
			_SYDNEY_VERIFY_INCONSISTENT(
				cProgress_, cRootPath_,
				Message::NextLinkOfLastPage(pLocPage_->getID()));
		}
		else if (pLocPage_->getLocBlockCount() != 0)
		{
			// 次ページのLOCブロックは0のはず
			_SYDNEY_VERIFY_INCONSISTENT(
				cProgress_, cRootPath_,
				Message::NextLinkOfLastPage(pLocPage_->getID()));
		}
	}
}

//
//	FUNCTION protected
//	FullText2::MiddleBaseList::freeAllPages
//		-- すべてのOverflowFileのページを解放する
//
//	NOTES
//
//	ARGUMNETNS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MiddleBaseList::freeAllPages()
{
	if (m_pTmpArea == 0)
		_SYDNEY_THROW0(Exception::BadArgument);

	// DIRブロックごとに処理する
	
	LeafPage::DirBlock* pDirBlock = m_pTmpArea->getDirBlock();
	for (int i = 0; i < m_pTmpArea->getDirBlockCount(); ++i)
	{
		// DIRブロックを得る
		LeafPage::DirBlock* pDir = pDirBlock + i;

		// IDページを得る
		OverflowPage::PagePointer pIdPage
			= m_pOverflowFile->attachPage(pDir->getPageID());
		m_mapFreePage.insert(pDir->getPageID(), pIdPage);

		for (int j = 0; j < static_cast<int>(pIdPage->getIDBlockCount()); ++j)
		{
			// IDブロックを得る
			OverflowPage::IDBlock cIdBlock = pIdPage->getIDBlock(j);

			// IDブロック以下のページを追加する
			addIdBlockPages(cIdBlock);
		}
	}

	// 最終IDブロックのLOCブロックを処理する
	//
	// 最終IDブロックのLOCブロックはisContinue()がfalseでも次ページが存在する
	// 場合があり、LOCブロックがisContinue()か否かに関わらず、次ページを
	// 削除対象にする必要がある

	OverflowPage::IDBlock cIdBlock = m_pTmpArea->getIDBlock();
	addLocPages(cIdBlock.getLocBlockPageID());

	// すべてのページがリストアップされたので、
	// 順番に解放する

	FreePageMap::Iterator ite = m_mapFreePage.begin();
	for (; ite != m_mapFreePage.end(); ++ite)
	{
		m_pOverflowFile->freePageCore((*ite).second);
	}
	m_mapFreePage.erase(m_mapFreePage.begin(), m_mapFreePage.end());
}

//
//	FUNCTION protected
//	FullTex2::MiddleBaseList::addIdBlockPages
//		-- IDブロック以下の解放対象のページをリストアップする
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::OverflowPage::IDBlock& cIdBlock_
//		対象のIDブロック
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MiddleBaseList::addIdBlockPages(const OverflowPage::IDBlock& cIdBlock_)
{
	PhysicalFile::PageID uiPageID = cIdBlock_.getLocBlockPageID();
	if (uiPageID == PhysicalFile::ConstValue::UndefinedPageID)
		// LOCページが無いので終了
		return;

	// LOCページを得る
	OverflowPage::PagePointer pLocPage = m_pOverflowFile->attachPage(uiPageID);
	m_mapFreePage.insert(uiPageID, pLocPage);

	// LOCブロックを得る
	OverflowPage::LocBlock cLocBlock =
		pLocPage->getLocBlock(cIdBlock_.getLocBlockOffset());

	while (cLocBlock.isContinue())
	{
		// このLOCブロックは次ページに続いている
		
		uiPageID = pLocPage->getNextPageID();
		pLocPage = m_pOverflowFile->attachPage(uiPageID);
		m_mapFreePage.insert(uiPageID, pLocPage);
		
		cLocBlock = pLocPage->getLocBlock();
	}
}

//
//	FUNCTION protected
//	FullText2::MiddleBaseList::addLocPages
//		-- 解放対象のLOCページを追加する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		解放対象のLOCページのページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MiddleBaseList::addLocPages(PhysicalFile::PageID uiPageID_)
{
	while (uiPageID_ != PhysicalFile::ConstValue::UndefinedPageID)
	{
		OverflowFile::PagePointer pLocPage =
			m_pOverflowFile->attachPage(uiPageID_);
		m_mapFreePage.insert(uiPageID_, pLocPage);

		// 次へ
		uiPageID_ = pLocPage->getNextPageID();
	}
}

//
//  Copyright (c) 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
