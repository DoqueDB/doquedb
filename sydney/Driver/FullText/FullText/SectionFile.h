// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SectionFile.h --
// 
// Copyright (c) 2003, 2006, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT_SECTIONFILE_H
#define __SYDNEY_FULLTEXT_SECTIONFILE_H

#include "FullText/Module.h"
#include "FullText/File.h"
#include "FullText/FileID.h"

#include "Btree/File.h"

#include "Common/DataArrayData.h"

#include "Os/Path.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT_BEGIN

//	CLASS
//	FullText::SectionFile --
//
//	NOTES
//
//
class SectionFile : public File
{
public:
	// コンストラクタ
	SectionFile(FileID& cFileID_);
	// デストラクタ
	virtual ~SectionFile();

	// ファイルサイズを得る
	ModUInt64 getSize(const Trans::Transaction& cTrans_)
		{ return m_pBtreeFile->getSize(cTrans_); }
	// ファイルに挿入されているタプル数を得る
	ModInt64 getCount() const { return m_pBtreeFile->getCount(); }

	// ファイルを作成する
	void create(const Trans::Transaction& cTransaction_);
	// ファイルを破棄する
	void destroy(const Trans::Transaction& cTransaction_);

	// ファイルをマウントする
	void mount(const Trans::Transaction& cTransaction_);
	// ファイルをアンマウントする
	void unmount(const Trans::Transaction& cTransaction_);
	
	// ファイルをフラッシュする
	void flush(const Trans::Transaction& cTransaction_)
	{
		m_pBtreeFile->flush(cTransaction_);
	}

	// ファイルのバックアップを開始する
	void startBackup(const Trans::Transaction& cTransaction_,
					 const bool bRestorable_)
	{
		m_pBtreeFile->startBackup(cTransaction_, bRestorable_);
	}
	// ファイルのバックアップを終了する
	void endBackup(const Trans::Transaction& cTransaction_)
	{
		m_pBtreeFile->endBackup(cTransaction_);
	}

	// ファイルを障害から回復する
	void recover(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);

	// 整合性検査を行う
	void verify(const Trans::Transaction& cTransaction_,
				Admin::Verification::Treatment::Value uiTreatment_,
				Admin::Verification::Progress& cProgress_)
	{
		m_pBtreeFile->verify(cTransaction_, uiTreatment_, cProgress_);
	}
	
	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	void restore(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_)
	{
		m_pBtreeFile->restore(cTransaction_, cPoint_);
	}

	//論理ファイルをオープンする
	void open(const Trans::Transaction& cTransaction_,
			  const LogicalFile::OpenOption& cOption_);
	void open(const Trans::Transaction& cTransaction_,
			  int iOpenMode_,
			  bool bEstimate_);
	//論理ファイルをクローズする
	void close();

	// 同期をとる
	void sync(const Trans::Transaction& cTransaction_,
			  bool& incomplete, bool& modified)
	{
		m_pBtreeFile->sync(cTransaction_, incomplete, modified);
	}

	//ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Common::StringArrayData& cArea_);

	// 実体である OS ファイルが存在するか調べる
	bool isAccessible(bool bForce_ = false) const
	{
		return m_pBtreeFile->isAccessible(bForce_);
	}
	// マウントされているか調べる
	bool isMounted(const Trans::Transaction& trans) const
	{
		return m_pBtreeFile->isMounted(trans);
	}

	// すべてのページの更新を破棄する
	void recoverAllPages() {}
	// すべてのページの更新を確定する
	void flushAllPages() {}

	// 挿入する
	void insert(ModUInt32 uiTupleID_,
				const Common::Data& cSectionOffset_);
	// 更新する
	void update(ModUInt32 uiTupleID_,
				const Common::Data& cSectionOffset_);
	// 削除する
	void expunge(ModUInt32 uiTupleID_);

	// セクションを情報を得る
	bool get(ModUInt32 uiTupleID_,
			 Common::Data& cSectionOffset_);

	// 新しいパスを得る
	Os::Path getNewPath(const Os::Path& cParent_);
	// パスを設定する
	void setNewPath(const Os::Path& cParent_);

private:
	// キーを作成する
	void makeKey(ModUInt32 uiTupleID_);

	// セクション情報をB木に挿入できる形にコンバートする
	void convertSectionData(const Common::Data& cSectionOffset_,
							Common::Data::Pointer pNewSectionOffset_);
	
	// パスを設定する
	void setPath(const Os::Path& cPath_);
	
	// B木
	Btree::File* m_pBtreeFile;

	// B木とやりとりするデータ
	Common::DataArrayData m_cKey;
	Common::DataArrayData m_cData;
	Common::DataArrayData m_cTuple;

	// FileID
	FileID& m_cFileID;
	// ディレクトリ
	Os::Path m_cPath;
};

_SYDNEY_FULLTEXT_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT_SECTIONFILE_H

//
//	Copyright (c) 2003, 2006, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
