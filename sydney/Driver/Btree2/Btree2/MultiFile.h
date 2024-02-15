// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MultiFile.h --
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE2_MULTIFILE_H
#define __SYDNEY_BTREE2_MULTIFILE_H

#include "Btree2/Module.h"
#include "Btree2/BtreeFile.h"
#include "Btree2/FileID.h"
#include "Btree2/Data.h"
#include "Btree2/Compare.h"
#include "Btree2/AutoPointer.h"
#include "Btree2/HeaderPage.h"
#include "Btree2/MultiPage.h"
#include "Btree2/Condition.h"

#include "LogicalFile/OpenOption.h"

#include "Common/Data.h"
#include "Common/BitSet.h"
#include "Common/DataArrayData.h"

#include "Utility/OpenMP.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_BTREE2_BEGIN

//
//	CLASS
//	Btree2::MultiFile --
//
//	NOTES
//
//
class MultiFile : public BtreeFile
{
public:
	//
	//	CLASS
	//	Btree2::MultiFile::GetByBitSet
	//
	//	NOTES
	//
	class GetByBitSet : public Utility::OpenMP
	{
	public:
		// コンストラクタ
		GetByBitSet(MultiFile& cFile_, int iFieldID_,
					Common::BitSet& cBitSet_)
			: m_cFile(cFile_), m_iFieldID(iFieldID_), m_cBitSet(cBitSet_) {}
		// デストラクタ
		~GetByBitSet() {}

		// マルチスレッドで実行するメソッド
		void parallel();

	private:
		// ファイル
		MultiFile& m_cFile;
		// 対象フィールド
		int m_iFieldID;
		// ビットセット
		Common::BitSet& m_cBitSet;
	};
	
	//コンストラクタ
	MultiFile(const FileID& cFileID_, const Os::Path& cSubPath_ = Os::Path());
	//デストラクタ
	virtual ~MultiFile();

	// ファイルサイズを得る
	ModUInt64 getSize() const;

	// ファイルを破棄する
	void destroy(const Trans::Transaction& cTransaction_);

	// ファイルをマウントする
	void mount(const Trans::Transaction& cTransaction_);
	// ファイルをアンマウントする
	void unmount(const Trans::Transaction& cTransaction_);
	// ファイルをフラッシュする
	void flush(const Trans::Transaction& cTransaction_);

	// ファイルのバックアップを開始する
	void startBackup(const Trans::Transaction& cTransaction_,
					 const bool bRestorable_);
	// ファイルのバックアップを終了する
	void endBackup(const Trans::Transaction& cTransaction_);

	// ファイルを障害から回復する
	void recover(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);
	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	void restore(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);

	//論理ファイルをオープンする
	void open(const Trans::Transaction& cTransaction_,
			  LogicalFile::OpenOption::OpenMode::Value eOpenMode_);
	// クローズする
	void close();

	// 検索結果件数を見積もる
	ModUInt32 getEstimateCount(Condition* pCondition_);

	// 同期をとる
	void sync(const Trans::Transaction& cTransaction_,
			  bool& incomplete, bool& modified);
	// 不要なデータを削除する
	void compact(const Trans::Transaction& cTransaction_,
				 bool& incomplete, bool& modified);

	//ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cFilePath_);

	// 検索
	void search(Condition* pCondition_, bool isReverse_);

	// 取得 -- 配列で
	bool get(unsigned char ucBitSet_,
			 Common::DataArrayData& cTuple_,
			 unsigned int& uiTupleID_);
	// 取得 -- ビットセットで
	void getByBitSet(ModVector<Condition*>& vecpCondition_,
					 int iFieldID_, Common::BitSet& cBitSet_);

	// 挿入
	void insert(const Common::DataArrayData& cValue_);
	// 削除
	void expunge(const Common::DataArrayData& cKey_);

	// 物理ファイルの整合性検査を開始する
	void startVerification(
		const Trans::Transaction& cTransaction_,
		Admin::Verification::Treatment::Value uiTreatment_,
		Admin::Verification::Progress& cProgress_);
	// 物理ファイルの整合性検査を終了する
	void endVerification();

	// 整合性チェック
	void verify();

	// マーク
	bool mark();
	// リワインド
	bool rewind();

	// 全ページをフラッシュする
	void flushAllPages();
	// 全ページを元に戻す
	void recoverAllPages();

	// 検索ページをdetachする
	void detachSearchPage() { m_pSearchPage = 0; }

	// ページを得る
	MultiPage::PagePointer attachPage(PhysicalFile::PageID uiPageID_,
									  const MultiPage::PagePointer& pParent_,
									   Buffer::Page::FixMode::Value eFixMode_
									   = Buffer::Page::FixMode::Unknown);

	// 新しいページを得る
	MultiPage::PagePointer allocatePage(PhysicalFile::PageID uiPrevPageID_,
										PhysicalFile::PageID uiNextPageID_,
										const MultiPage::PagePointer& pParent_);

	// 親ページを検索する
	MultiPage::PagePointer findPage(const ModUInt32* pValue_,
									unsigned char nullBitmap_,
									PhysicalFile::PageID uiChildPageID_);

	// 次処理するリーフページと検索条件を得る
	MultiPage::PagePointer nextLeafPage(Condition*& pCondition_,
										bool& isNewCondition_);

protected:
	// 検索結果数を見積もる(for 検索)
	ModUInt32 getEstimateCountForSearch(Condition* pCondition_,
										ModUInt32 count_);
	// 検索結果数を見積もる(for フェッチ)
	ModUInt32 getEstimateCountForFetch(Condition* pCondition_,
									   ModUInt32 count_);

private:
	// データを作成する
	AutoPointer<ModUInt32> makeData(const Common::DataArrayData& cValue_,
									const Data& cData_,
									ModSize& uiSize_,
									unsigned char& nullBitmap_);
	
	// リーフページを得る
	MultiPage::PagePointer getLeafPage(const ModUInt32* pBuffer_,
									   unsigned char nullBitmap_,
									   const Compare& cCompare_,
									   bool isLower_ = true);
	// ルートページを得る
	MultiPage::PagePointer getRootPage();

	// 検索前準備を行う
	void preSearch();

	// 先頭のリーフページを得る
	MultiPage::PagePointer searchPage(Condition* pCondition_,
									  bool bReverse_);

	// 次の検索結果を設定する
	void next();
	void nextReverse();

	// リーフエントリをコピーする
	void copyEntry(ModUInt32* dst, MultiPage::Iterator i_);

	// ページ内のヒット件数を確認する
	ModSize getHitCount(MultiPage::PagePointer pPage_,
						Condition* pCondition_,
						MultiPage::Iterator& l_,
						MultiPage::Iterator& u_,
						bool isLower_ = false);

	// 検索中のページ
	MultiPage::PagePointer m_pSearchPage;

	// 直前にgetした情報
	PhysicalFile::PageID m_uiSearchPageID;		// ページID
	int m_iSearchEntryPosition;					// エントリ位置
	ModUInt32 m_pSearchEntryBuffer[FileID::MAX_SIZE];	// エントリ内容
	unsigned char m_ucSearchEntryNullBitmap;	// エントリのnullビットマップ

	// マークした情報
	PhysicalFile::PageID m_uiMarkPageID;		// ページID
	int m_iMarkEntryPosition;					// エントリ位置
	ModUInt32 m_pMarkEntryBuffer[FileID::MAX_SIZE];	// エントリ内容
	unsigned char m_ucMarkEntryNullBitmap;		// エントリのnullビットマップ
	
	// 操作方向
	bool m_bReverse;
	// 制約ロックのための検索か
	bool m_bConstraintLock;

	// 検索条件
	Condition* m_pCondition;

	// 削除エントリ
	MultiFile* m_pExpungeFile;

	// 以下はマルチスレッド版 GetByBitset のための変数
	
	ModVector<Condition*>* m_vecpCondition;		// 検索条件
	ModVector<Condition*>::Iterator m_iteCondition;	// 現在の検索条件
	PhysicalFile::PageID m_uiNextPageID;		// 次のリーフページのID
};

_SYDNEY_BTREE2_END
_SYDNEY_END

#endif //__SYDNEY_BTREE2_MULTIFILE_H

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2007, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
