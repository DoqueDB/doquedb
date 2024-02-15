// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedUnit.h -- 1つの転置ファイルをあらわすクラス
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_INVERTEDUNIT_H
#define __SYDNEY_FULLTEXT2_INVERTEDUNIT_H

#include "FullText2/Module.h"
#include "FullText2/InvertedUpdateFile.h"
#include "FullText2/InvertedSection.h"
#include "FullText2/FileID.h"
#include "FullText2/LeafPage.h"

#include "Trans/Transaction.h"
#include "Trans/TimeStamp.h"
#include "LogicalFile/OpenOption.h"
#include "Os/Path.h"

#include "ModVector.h"
#include "ModMap.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class BtreeFile;
class LeafFile;
class OverflowFile;

//
//	CLASS
//	FullText2::InvertedUnit
//		-- B木、Leaf、Overflowのファイルを1つづつ保持しているクラス
//
//	NOTES
//	転置ファイルをなす必要最小限のファイルを保持しているクラス
//
class InvertedUnit : public InvertedUpdateFile
{
public:
	// コンストラクタ
	InvertedUnit(InvertedSection& cInvertedSection_,
				 const Os::Path& cPath_,
				 bool bBatch_,
				 int iUnitNumber_ = -1);
	// デストラクタ
	virtual ~InvertedUnit();

	// 利用サイズを得る
	ModUInt64 getUsedSize(const Trans::Transaction& cTransaction_);

	// ファイルを作成する
	void create();

	// ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cNewPath_)
		{ move(cTransaction_, cNewPath_, true); }

	// ファイルの内容をクリアする
	void clear();

	// 整合性検査を行う
	void verify(const Trans::Transaction& cTransaction_,
				const Admin::Verification::Treatment::Value eTreatment_,
				Admin::Verification::Progress& cProgress_);

	// ページをsaveする
	bool saveAllPages();

	// LeafPageを得る
	LeafPage::PagePointer attachLeafPage(PhysicalFile::PageID uiPageID_);
	// OverflowFileを得る
	OverflowFile* getOverflowFile() { return m_pOverflowFile; }
	
	// 削除するIDブロックを登録する
	// (すべての削除が成功した場合に、本当にIDブロックを消す)
	void enterDeleteIdBlock(const ModUnicodeString& cstrKey_,
							ModUInt32 uiFirstDocumentID_);
	// 先頭文書IDを削除したIDブロックのログを登録する
	void enterExpungeFirstDocumentID(const ModUnicodeString& cstrKey_,
									 ModUInt32 uiOldDocumentID_,
									 ModUInt32 uiNewDocumentID_);
	// 先頭文書IDを削除したIDブロックのログを検索する
	ModUInt32 getExpungeFirstDocumentID(const ModUnicodeString& cstrKey_,
										ModUInt32 uiOldDocumentID_);

	// B木関係
	void insertBtree(const ModUnicodeString& cstrKey_,
					 PhysicalFile::PageID uiPageID_);
	void expungeBtree(const ModUnicodeString& cstrKey_,
					  PhysicalFile::PageID uiPageID_);
	void updateBtree(const ModUnicodeString& cstrKey1_,
					 PhysicalFile::PageID uiPageID1_,
					 const ModUnicodeString& cstrKey2_,
					 PhysicalFile::PageID uiPageID2_);
	bool searchBtree(const ModUnicodeString& cstrKey_,
					 PhysicalFile::PageID& uiPageID_);

#ifndef SYD_COVERAGE
	void reportFile(const Trans::Transaction& cTransaction_,
					Buffer::Page::FixMode::Value eFixMode_,
					ModOstream& stream_);
#endif

	// マウントされているか
	bool isMounted(const Trans::Transaction& cTransaction_) const;
	// ファイルが存在するか
	bool isAccessible(bool force_ = false) const;

	// ユニット番号を得る
	int getUnitNumber() const { return m_iUnitNumber; }

	// 更新用のListManagerを得る
	UpdateListManager* getUpdateListManager();

	// 削除対象のIDブロックを削除する
	void expungeIdBlock();
	
	// 削除IDブロックのUndoログをクリアする
	void clearDeleteIdBlockUndoLog()
		{
			m_mapExpungeFirstDocumentID.erase(
				m_mapExpungeFirstDocumentID.begin(),
				m_mapExpungeFirstDocumentID.end());
		}

	// 削除対象のIDブロック情報をクリアする
	void clearDeleteIdBlockLog()
		{
			m_mapDeleteIdBlock.erase(
				m_mapDeleteIdBlock.begin(),
				m_mapDeleteIdBlock.end());
		}

	// バキュームが必要かどうか
	virtual bool isNeedVacuum(const ModUnicodeString& cstrKey_,
							  int iNewExpungeCount_);

	// バキュームが必要かどうかのカウントをクリアする
	virtual void clearExpungeCount(const ModUnicodeString& cstrKey_);

protected:
	// ファイルをattachする
	void attach();
	// ファイルをdetachする
	void detach();

	// 整合性検査を開始する
	void startVerification(const Trans::Transaction& cTransaction_,
						   Admin::Verification::Treatment::Value eTreatment_,
						   Admin::Verification::Progress& cProgress_);
	// 整合性検査を終了する
	void endVerification();
	
	// ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cNewPath_, bool bDirectory_);
	
	//
	//	TYPEDEF
	//	FullText2::InvertedUnit::Vector
	//
	typedef ModVector<ModUInt32> Vector;

	//
	//	TYPEDEF
	//	FullText2::InvertedUnit::Map
	//
	typedef ModMap<ModUnicodeString, Vector, ModLess<ModUnicodeString> > Map;

	//
	//	TYPEDEF
	//	FullText2::InvertedUnit::IDMap
	//
	typedef ModMap<ModPair<ModUnicodeString, ModUInt32>,
		ModUInt32, ModLess<ModPair<ModUnicodeString, ModUInt32> > > IDMap;

	// B木ファイル
	BtreeFile* m_pBtreeFile;
	// リーフファイル
	LeafFile* m_pLeafFile;
	// オーバーフローファイル
	OverflowFile* m_pOverflowFile;
	
	// 削除するIDブロック
	Map m_mapDeleteIdBlock;
	// 先頭文書IDを削除したIDブロック
	IDMap m_mapExpungeFirstDocumentID;

	// バッチインサートかどうか
	bool m_bBatch;

	// ユニット番号
	int m_iUnitNumber;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_INVERTEDUNIT_H

//
//	Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
