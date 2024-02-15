// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VectorFile.h -- ベクターファイル
// 
// Copyright (c) 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_KDTREE_VECTORFILE_H
#define __SYDNEY_KDTREE_VECTORFILE_H

#include "KdTree/Module.h"
#include "KdTree/SubFile2.h"

#include "KdTree/FileID.h"

#include "Version/Page.h"

#include "Os/Path.h"

#include "ModMap.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

//	CLASS
//	KdTree::VectorFile -- ベクターファイル
//
//	NOTES
//
class VectorFile : public SubFile2
{
public:
	//コンストラクタ
	VectorFile(FileID& cFileID_, const Os::Path& cPath_);
	//デストラクタ
	virtual ~VectorFile();

	// ファイルに挿入されているタプル数を得る
	ModUInt32 getCount();

	// ファイルを作成する
	void create();

	// 論理ファイルをクローズする
	void close();

	// すべてのページの更新を破棄する
	void recoverAllPages();
	// すべてのページの更新を確定する
	void flushAllPages();

	// 最大のキー値を得る
	ModUInt32 getMaxKey();
	// 最少のページIDを得る
	Version::Page::ID getMinPageID()
		{ return static_cast<Version::Page::ID>(1); }
	// 最大のページIDを得る
	Version::Page::ID getMaxPageID();

	// ファイルの中身をクリアする
	void clear();

protected:
	// 整合性検査を開始する
	void startVerification(const Trans::Transaction& cTransaction_,
						   Admin::Verification::Treatment::Value eTreatment_,
						   Admin::Verification::Progress& cProgress_);
	// 整合性検査を終了する
	void endVerification();

	// ヘッダーページをdirtyにする
	void dirtyHeaderPage() { m_bDirtyHeaderPage = true; }
	
	//
	//	STRUCT
	//	KdTree::VectorFile::Header
	//
	struct Header
	{
		Header()
			: m_uiCount(0), m_uiMaxKey(0), m_uiMaxPageID(0) {}
		
		ModUInt32	m_uiCount;		// 登録行数
		ModUInt32	m_uiMaxKey;		// 最大登録キー値
		ModUInt32	m_uiMaxPageID;	// 最大ページID
	};

	//
	//	CLASS
	//	KdTree::VectorFile::Page
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
	//	KdTree::VectorFile::PageMap -- dirtyなページを格納しておくマップ
	//
	typedef ModMap<Version::Page::ID, VectorFile::Page*,
				   ModLess<Version::Page::ID> >	PageMap;
	
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

	// verify中か否か
	bool m_bVerify;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_VECTORFILE_H

//
//	Copyright (c) 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
