// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VectorFile.h -- 全文のその他情報を格納するベクターファイル
// 
// Copyright (c) 2005, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT_VECTORFILE_H
#define __SYDNEY_FULLTEXT_VECTORFILE_H

#include "FullText/Module.h"
#include "FullText/File.h"
#include "FullText/FileID.h"

#include "Version/File.h"
#include "Version/Page.h"

#include "LogicalFile/OpenOption.h"

#include "Common/DataArrayData.h"

#include "Os/Path.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT_BEGIN

//	CLASS
//	FullText::VectorFile
//		-- 全文ファイルの転置以外の情報を管理する
//
//	NOTES
//
class VectorFile : public File
{
public:
	//
	//	STRUCT
	//	FullText::VectorFile::Header
	//
	struct Header
	{
		ModUInt32	m_uiVersion;	// バージョン
		ModUInt32	m_uiCount;		// 登録行数
		ModUInt32	m_uiMaxPageID;	// 最大ページID
	};
	
	//コンストラクタ
	VectorFile(FileID& cFileID_);
	//デストラクタ
	virtual ~VectorFile();

	// ファイルIDを得る
	FileID& getFileID() const { return m_cFileID; }

	// ファイルサイズを得る
	ModUInt64 getSize(const Trans::Transaction& cTrans_)
	{
		ModUInt64 size = 0;
		if (isMounted(cTrans_))
			size = m_pVersionFile->getSize();
		return size;
	}

	// ファイルに挿入されているタプル数を得る
	ModInt64 getCount() const;

	// ファイルを作成する
	void create(const Trans::Transaction& cTransaction_);
	// ファイルを破棄する
	void destroy(const Trans::Transaction& cTransaction_);

	// ファイルをマウントする
	void mount(const Trans::Transaction& cTransaction_)
	{
		m_pVersionFile->mount(cTransaction_);
	}
	// ファイルをアンマウントする
	void unmount(const Trans::Transaction& cTransaction_)
	{
		// マウントの有無や実体の存在の有無を確認せずに
		// とにかくアンマウントする
		//
		//【注意】	そうしないと下位層で管理している
		//			情報がメンテナンスされない

		m_pVersionFile->unmount(cTransaction_);
		m_bMounted = false;
	}
	
	// ファイルをフラッシュする
	void flush(const Trans::Transaction& cTransaction_)
	{
		if (isMounted(cTransaction_))
			m_pVersionFile->flush(cTransaction_);
	}

	// ファイルのバックアップを開始する
	void startBackup(const Trans::Transaction& cTransaction_,
					 const bool bRestorable_)
	{
		if (isMounted(cTransaction_))
			m_pVersionFile->startBackup(cTransaction_, bRestorable_);
	}
	// ファイルのバックアップを終了する
	void endBackup(const Trans::Transaction& cTransaction_)
	{
		if (isMounted(cTransaction_))
			m_pVersionFile->endBackup(cTransaction_);
	}

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
				 const Trans::TimeStamp& cPoint_)
	{
		if (isMounted(cTransaction_))
			m_pVersionFile->restore(cTransaction_, cPoint_);
	}

	//論理ファイルをオープンする
	void open(const Trans::Transaction& cTransaction_,
			  const LogicalFile::OpenOption& cOption_);
	//論理ファイルをクローズする
	void close();

	// 同期をとる
	void sync(const Trans::Transaction& cTransaction_,
			  bool& incomplete, bool& modified)
	{
		if (isMounted(cTransaction_))
			
			//
			//	【注意】
			//	トランケートはしない
			
			m_pVersionFile->sync(cTransaction_, incomplete, modified);
	}

	//ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Common::StringArrayData& cArea_);

	// 実体である OS ファイルが存在するか調べる
	bool isAccessible(bool bForce_ = false) const
	{
		return m_pVersionFile->isAccessible(bForce_);
	}
	// マウントされているか調べる
	bool isMounted(const Trans::Transaction& trans) const
	{
		if (m_bMounted == false)
			
			// falseだったらバージョンに聞く
			// trueだったらdetachするまでfalseになることはない
			
			m_bMounted = m_pVersionFile->isMounted(trans);
		
		return m_bMounted;
	}

	// すべてのページの更新を破棄する
	void recoverAllPages();
	// すべてのページの更新を確定する
	void flushAllPages();

	// 最大ページIDを得る
	Version::Page::ID getMaxPageID();

	// 挿入する
	void insert(ModUInt32 uiRowID_, const Common::DataArrayData& cValue_);
	// 更新する
	void update(ModUInt32 uiRowID_, const Common::DataArrayData& cValue_,
				const ModVector<int>& vecUpdateFields_);
	// 削除する
	void expunge(ModUInt32 uiRowID_);
	
	// 取得する
	void get(ModUInt32 uiRowID_, int iField_, Common::Data& cValue_);
	void get(ModUInt32 uiRowID_, Common::DataArrayData& cValue_,
			 const ModVector<int>& vecGetFields_);

	// トランザクションを取得する
	const Trans::Transaction& getTransaction() const
		{ return *m_pTransaction; }
	
private:
	// 物理ファイルをアタッチする
	void attach();
	// 物理ファイルをデタッチする
	void detach();

	// ヘッダーページの内容を読み込む
	void readHeader();
	// ヘッダーページ内容を初期化する
	void initializeHeader();
	// ヘッダーページをdirtyにする
	void dirtyHeaderPage() { m_bDirtyHeaderPage = true; }

	// ページをfixする
	Version::Page::Memory& fixPage(Version::Page::ID uiPageID_);
	// ページをallocateする
	void allocatePage(Version::Page::ID uiPageID_);

	// ROWIDから格納領域を得る
	const char* getConstBuffer(ModUInt32 uiRowID_, const char*& bitmap_);
	char* getBuffer(ModUInt32 uiRowID_, char*& bitmap_);

	// ROWIDからページIDとオフセットを得る
	Version::Page::ID convertToPageID(ModUInt32 uiRowID_, int& offset_) const
	{
		Version::Page::ID id = uiRowID_ / getCountPerPage() + 1;
		offset_ = static_cast<int>(
			(uiRowID_ % getCountPerPage()) * getElementSize());
		return id;
	}

	// 1ページに格納できる個数を得る
	ModSize getCountPerPage() const
	{
		ModSize totalBit
			= Version::Page::getContentSize(m_pVersionFile->getPageSize()) * 8;
		ModSize elementBit = getElementSize() * 8 + getElementFieldCount();
		return totalBit / elementBit;
	}
	// 1要素のサイズを得る
	ModSize getElementSize() const
	{
		return m_cFileID.getVectorElementTotalSize();
	}
	// 1要素のフィールド数を得る
	ModSize getElementFieldCount() const
	{
		return m_cFileID.getVectorElementFieldCount();
	}

	// ビットマップをチェックする
	bool isNull(const char* bitmap_, ModUInt32 uiRowID_, int n_) const;

	// ビットマップをONにする
	void bitOn(char* bitmap_, ModUInt32 uiRowID_, int n_);
	// ビットマップをOFFにする
	void bitOff(char* bitmap_, ModUInt32 uiRowID_, int n_);

	// バージョンファイル
	Version::File* m_pVersionFile;

	// FileID
	FileID& m_cFileID;

	// トランザクション
	const Trans::Transaction* m_pTransaction;
	// FixMode
	Buffer::Page::FixMode::Value m_eFixMode;
	// 整合性検査か
	bool m_bVerify;

	Admin::Verification::Progress* m_pProgress;

	// ヘッダーページ
	Version::Page::Memory m_cHeaderPage;
	// ヘッダーページがdirtyかどうか
	bool m_bDirtyHeaderPage;
	// Header情報
	Header m_cHeader;

	// 現在見ているページ(ヘッダーページ以外で)
	Version::Page::Memory m_cCurrentPage;
	// 現在見ているページがdirtyかどうか
	bool m_bDirtyCurrentPage;

	// パス
	Os::Path m_cPath;

	// マウントされているかどうか
	mutable bool m_bMounted;
};

_SYDNEY_FULLTEXT_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT_VECTORFILE_H

//
//	Copyright (c) 2005, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
