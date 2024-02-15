// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BtreeFile.h --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_BTREEFILE_H
#define __SYDNEY_INVERTED_BTREEFILE_H

#include "Inverted/Module.h"
#include "Inverted/File.h"
#include "Inverted/FileID.h"
#include "Inverted/Page.h"
#include "Inverted/BtreePage.h"
#include "Inverted/PagePointer.h"
#include "PhysicalFile/Page.h"

#include "ModUnicodeChar.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

//
//	CLASS
//	Inverted::BtreeFile --
//
//	NOTES
//
class BtreeFile : public File
{
public:
	//
	//	STRUCT
	//	Inverted::BtreeFile::Header
	//
	struct Header
	{
		PhysicalFile::PageID	m_uiRootPageID;			// ルートページID
		PhysicalFile::PageID	m_uiLeftLeafPageID;		// 一番左のリーフページ
		PhysicalFile::PageID	m_uiRightLeafPageID;	// 一番右のリーフページ
		ModUInt32				m_uiCount;				// エントリ数
		ModUInt32				m_uiStepCount;			// B木の段数
	};

	//
	//	TYPEDEF
	//	Inverted::BtreeFile::PagePointer
	//
	typedef BtreePage::PagePointer PagePointer;

	// コンストラクタ
	BtreeFile(const FileID& cFileID_,
			  const Os::Path& cFilePath_,
			  bool batch_);
	// デストラクタ
	virtual ~BtreeFile();

	// ファイルを作成する
	void create();

	// 整合性検査を行う
	void verify();

	// 移動する
	void move(const Trans::Transaction& cTransaction_, const Os::Path& cPath_);

	// すべてのページをクリアする
	void clear(const Trans::Transaction& cTransaction_, bool bForce_);

	// 引数以下で最大のキーを持つバリューを検索する
	PhysicalFile::PageID search(const ModUnicodeChar* pszKey_);

	// 新しいエントリを挿入する
	void insert(const ModUnicodeChar* pszKey_, PhysicalFile::PageID uiPageID_);
	// エントリを削除する
	void expunge(const ModUnicodeChar* pszKey_);
	// エントリを更新する
	void update(const ModUnicodeChar* pszKey1_, PhysicalFile::PageID uiPageID1_,
				const ModUnicodeChar* pszKey2_, PhysicalFile::PageID uiPageID2_);

	// 既存のページをattachする
	PagePointer attachPage(PhysicalFile::PageID uiPageID_,
							ModSize uiStepCount_);
	// 新しいページをattachする
	PagePointer allocatePage(PhysicalFile::PageID uiPrevPageID_,
							PhysicalFile::PageID uiNextPageID_,
							ModSize uiStepCount_);

	// ルートページを設定する
	void setRootPage(PagePointer pPage_);
	// 一番右のリーフページを設定する
	void setRightLeafPage(PagePointer pPage_);
	// 一番左のリーフページを設定する
	void setLeftLeafPage(PagePointer pPage_);

	// 内容を確定する
	void flushAllPages();
	// 内容を破棄する
	void recoverAllPages();
	// 内容を保存する
	void saveAllPages();

	// ノードページを得る
	PagePointer getNodePage(const BtreePage::Entry& cEntry_,
							ModSize uiStepCount_);

	// エントリ数を得る
	ModSize getEntryCount() const;

#ifndef SYD_COVERAGE
	// ファイル状態を出力する
	void reportFile(const Trans::Transaction& cTransaction_,
					ModOstream& stream_);
#endif

private:
	// ヘッダーページを初期化する
	void initializeHeaderPage();
	// ヘッダーを設定する
	void setHeader(PhysicalFile::Page* pPhysicalPage_ = 0);
	// ルートノードを得る
	PagePointer getRootPage();

	// リーフページを得る
	PagePointer getLeafPage(const BtreePage::Entry& cEntry_);

	// エントリ数が同じかどうかチェックする
	void verifyEntryCount();

	// 平均ページ使用率
	float getAverageUsedPageRatio();

	// ヘッダー
	Header* m_pHeader;

	// ヘッダーページ
	Inverted::PageObjectPointer<Page> m_pHeaderPage;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_BTREEFILE_H

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
