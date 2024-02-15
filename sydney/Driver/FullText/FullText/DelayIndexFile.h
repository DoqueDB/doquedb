// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DelayIndexFile.h --
// 
// Copyright (c) 2003, 2004, 2005, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT_DELATINDEXFILE_H
#define __SYDNEY_FULLTEXT_DELAYINDEXFILE_H

#include "FullText/Module.h"
#include "SyDefault.h"

#include "FullText/InfoFile.h"
#include "FullText/IndexFile.h"
#include "FullText/SimpleIndexFile.h"

#include "LogicalFile/OpenOption.h"

#include "Common/BitSet.h"
#include "Inverted/Sign.h"
#include "Inverted/SearchCapsule.h"
#include "Inverted/GetLocationCapsule.h"
#include "Inverted/OptionDataFile.h"
#include "Inverted/ModInvertedTypes.h"

#include "FullText/FileID.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT_BEGIN

//
//	CLASS
//	FullText::DelayIndexFile --
//
//	NOTES
//
class DelayIndexFile : public IndexFile
{
private:
	// 全文情報ファイル
	InfoFile m_cInfoFile;

	// マージが終了したかどうか
	bool m_bInsertDone;
	bool m_bExpungeDone;
	// トランザクション
	const Trans::Transaction* m_pTransaction;
	// オープンモード
	LogicalFile::OpenOption::OpenMode::Value m_eOpenMode;
	// Estimateか
	bool m_bEstimate;
	// マージ中の転置リストのキー
	ModUnicodeString m_cInsertKey;
	ModUnicodeString m_cExpungeKey;

	// 全件をビットマップで取得する
	Common::BitSet getAllEntry(Inverted::SearchCapsule *pSearchCapsule,ModUInt32 sign);


	ModUInt32
	getInsert()
	{
		return m_cInfoFile.getIndex() == 0 ? Inverted::Sign::_Insert0 :
			Inverted::Sign::_Insert1;
	}
	ModUInt32
	getMergeInsert()
	{
		return m_cInfoFile.getIndex() == 0 ? Inverted::Sign::_Insert1 :
			Inverted::Sign::_Insert0;
	}
	ModUInt32
	getExpunge()
	{
		return m_cInfoFile.getIndex() == 0 ? Inverted::Sign::_Delete0 :
			Inverted::Sign::_Delete1;
	}
	ModUInt32
	getMergeExpunge()
	{
		return m_cInfoFile.getIndex() == 0 ? Inverted::Sign::_Delete1 :
			Inverted::Sign::_Delete0;
	}

	IndexFile::Iterator  getFullInvert();
	Inverted::IndexFile& getInsertFile();
	Inverted::IndexFile& getExpungeFile();
	Inverted::IndexFile& getMergeInsertFile();
	Inverted::IndexFile& getMergeExpungeFile();
	
public:
	// コンストラクタ
	DelayIndexFile(FileID& cFileID_);
	// デストラクタ
	virtual ~DelayIndexFile(){}

	// 検索器を得る
	Inverted::SearchCapsule&
	getSearchCapsule(Inverted::OptionDataFile* file_);

	Inverted::GetLocationCapsule&
	getGetLocationCapsule(Inverted::SearchCapsule* pCapsule_);

	// ファイルサイズを得る
	ModUInt64 getSize(const Trans::Transaction& cTrans_);
	// ファイルに挿入されているタプル数を得る
	ModInt64 getCount() const;
	// 総文書長を得る
	ModUInt64 getTotalDocumentLength();
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

	// 整合性検査を行う
	void verify(const Trans::Transaction& cTransaction_,
				Admin::Verification::Treatment::Value uiTreatment_,
				Admin::Verification::Progress& cProgress_);

	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	void restore(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);

	//論理ファイルをオープンする
	void open(const Trans::Transaction& cTransaction_,
				const LogicalFile::OpenOption& cOption_);
	//論理ファイルをクローズする
	void close();

	// 同期をとる
	void sync(const Trans::Transaction& cTransaction_,
				bool& incomplete, bool& modified);

	//ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
				const Common::StringArrayData& cArea_);

	// 実体である OS ファイルが存在するか調べる
	bool isAccessible(bool bForce_ = false)
	{
		return m_cInfoFile.isAccessible(bForce_);
	}
	// マウントされているか調べる
	bool isMounted(const Trans::Transaction& trans)
	{
		return m_cInfoFile.isMounted(trans);
	}

	// すべてのページの更新を破棄する
	void recoverAllPages();
	// すべてのページの更新を確定する
	void flushAllPages();

	// 挿入する
	void insert(const ModUnicodeString& cstrDocument_,
				const ModVector<ModLanguageSet>& vecLanguage_,
				ModUInt32 uiTupleID_,
				ModVector<ModSize>& vecSectionOffset_,
				ModInvertedFeatureList& vecFeature_);

	// 削除する
	void expunge( const ModUnicodeString& cstrDocument_,
				const ModVector<ModLanguageSet>& vecLanguage_,
				ModUInt32 uiTupleID_,
				const ModVector<ModSize>& vecSectionOffset_);

	//
	// 以下マージ用
	//

	// オープンする
	void openForMerge(const Trans::Transaction& cTransaction_);
	// クローズする
	void closeForMerge();

	// 1つの転置リストをマージする
	bool mergeList();
	// ベクターをマージする
	void mergeVector();
	// パラメータをクリアする
	static void clearParameter();

	bool isInsertMerge();
	bool isExpungeMerge();
	void syncMerge();
	bool mergeListInternal(Inverted::IndexFile& cInsertFile_,
						   Inverted::IndexFile& cExpungeFile_);
};

_SYDNEY_FULLTEXT_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT_DELAYINDEXFILE_H

//
//	Copyright (c) 2003, 2004, 2005, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
