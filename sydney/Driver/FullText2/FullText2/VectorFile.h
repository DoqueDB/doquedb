// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VectorFile.h -- ベクターファイル
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

#ifndef __SYDNEY_FULLTEXT2_VECTORFILE_H
#define __SYDNEY_FULLTEXT2_VECTORFILE_H

#include "FullText2/Module.h"
#include "FullText2/File.h"
#include "FullText2/FileID.h"

#include "Version/File.h"
#include "Version/Page.h"

#include "Common/DataArrayData.h"

#include "Os/Path.h"

#include "ModVector.h"
#include "ModMap.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class OtherInformationFile;

//	CLASS
//	FullText2::VectorFile -- ベクターファイル
//
//	NOTES
//
class VectorFile : public File
{
	friend class OtherInformationFile;

public:
	//コンストラクタ
	VectorFile(const FileID& cFileID_,
			   const Os::Path& cPath_,
			   bool bBatch_);
	//デストラクタ
	virtual ~VectorFile();

	// ファイルサイズを得る
	ModUInt64 getSize()
	{
		return m_pVersionFile->getSize();
	}

	// ファイルに挿入されているタプル数を得る
	ModUInt32 getCount();

	// ファイルを作成する
	void create();
	// ファイルを破棄する
	void destroy() { destroy(*m_pTransaction); }
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

	// 整合性検査を開始する
	void startVerification(const Trans::Transaction& cTransaction_,
						   Admin::Verification::Treatment::Value eTreatment_,
						   Admin::Verification::Progress& cProgress_);
	// 整合性検査を終了する
	void endVerification();
	
	// 整合性検査を行う
	void verify(const Trans::Transaction& cTransaction_,
				const Admin::Verification::Treatment::Value eTreatment_,
				Admin::Verification::Progress& cProgress_)
	{
		// 自分では特に何も管理している項目はないので、何もしない
	}
	
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
			  Buffer::Page::FixMode::Value eFixMode_);
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
			  const Os::Path& cPath_);

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

	// 最大のキー値を得る
	ModUInt32 getMaxKey();
	// 最大ページIDを得る
	Version::Page::ID getMaxPageID();

	// ファイルの中身をクリアする
	void clear();

protected:
	// ロック名を得る
	const Lock::FileName& getLockName() const
		{ return m_cFileID.getLockName(); }
	
	// サブクラス用のヘッダー領域の先頭を得る
	char* getSubClassHeader();
	// サブクラス用のヘッダー領域のサイズを得る
	ModSize getSubClassHeaderSize();
	
	// ヘッダーページをdirtyにする
	void dirtyHeaderPage() { m_bDirtyHeaderPage = true; }
	
	//	ENUM
	//	FullText2::VectorFile::_SUBCLASS_HEADER_OFFSET
	//
	enum { _SUBCLASS_HEADER_OFFSET = 64 };
	
	//
	//	STRUCT
	//	FullText2::VectorFile::Header
	//
	struct Header
	{
		Header()
			: m_uiVersion(0), m_uiCount(0), m_uiMaxKey(0), m_uiMaxPageID(0) {}
		
		ModUInt32	m_uiVersion;	// バージョン
		ModUInt32	m_uiCount;		// 登録行数
		ModUInt32	m_uiMaxKey;		// 最大登録キー値
		ModUInt32	m_uiMaxPageID;	// 最大ページID
	};

	//
	//	CLASS
	//	FullText2::VectorFile::Page
	//
	class Page
	{
	public:
		// コンストラクタ
		Page(bool bReadOnly_)
			: m_bDirty(false), m_pConstBuffer(0), m_bReadOnly(bReadOnly_) {}
		// デストラクタ
		~Page();

		// メモリを取得する
		const char* getConstBuffer() const;
		char* getBuffer();

		// 更新した
		void dirty() { m_bDirty = true; }
		// 更新したかどうか
		bool isDirty() const { return m_bDirty; }

		// バッファリング内容を確定 or 破棄する
		void unfix(bool commit_);

		// バージョン
		Version::Page::Memory m_cMemory;
		// 変更したかどうか
		bool m_bDirty;

		// バッファのキャッシュ
		mutable const char* m_pConstBuffer;
		// ReadOnlyか否か
		bool m_bReadOnly;
	};

	//
	//	TYPEDEF
	//	FullText2::VectorFile::PageMap -- dirtyなページを格納しておくマップ
	//
	typedef ModMap<Version::Page::ID, VectorFile::Page*,
				   ModLess<Version::Page::ID> >	PageMap;
	
	// 物理ファイルをアタッチする
	void attach(bool bBatch_);
	// 物理ファイルをデタッチする
	void detach();

	// ヘッダーページの内容を読み込む
	void readHeader();
	// ヘッダーページ内容を初期化する
	void initializeHeader();

	// ヘッダーページをfixする
	void fixHeaderPage();
	
	// ページをfixする
	void fixPage(Version::Page::ID uiPageID_);
	// ページをallocateする
	void allocatePage(Version::Page::ID uiPageID_);

	// ファイルID
	const FileID& m_cFileID;
	// バージョンファイル
	Version::File* m_pVersionFile;

	// トランザクション
	const Trans::Transaction* m_pTransaction;
	// FixMode
	Buffer::Page::FixMode::Value m_eFixMode;
	// 整合性検査か
	bool m_bVerify;

	Admin::Verification::Progress* m_pProgress;

	// Header情報
	Header m_cHeader;

	// ヘッダーページ
	Version::Page::Memory m_cHeaderPage;
	// ヘッダーページがダーティーかどうか
	bool m_bDirtyHeaderPage;
	
	// 現在見ているページ
	Page* m_pCurrentPage;

	// dirty なページが格納されているマップ
	PageMap m_mapDirtyPage;

	// パス
	Os::Path m_cPath;

	// マウントされているかどうか
	mutable bool m_bMounted;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_VECTORFILE_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
