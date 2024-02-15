// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InfoFile.h -- マージ情報を格納するファイル
// 
// Copyright (c) 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_KDTREE_INFOFILE_H
#define __SYDNEY_KDTREE_INFOFILE_H

#include "KdTree/Module.h"
#include "KdTree/SubFile.h"

#include "KdTree/FileID.h"

#include "PhysicalFile/File.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

//	CLASS
//	KdTree::InfoFile -- マージ情報を格納するファイル
//
//	NOTES
//
class InfoFile : public SubFile
{
public:
	//コンストラクタ
	InfoFile(FileID& cFileID_, const Os::Path& cPath_);
	//デストラクタ
	virtual ~InfoFile();

	// ファイルを作成する
	void create();

	// 整合性検査を行う
	void verify(const Trans::Transaction& cTransaction_,
				const Admin::Verification::Treatment::Value eTreatment_,
				Admin::Verification::Progress& cProgress_);
	
	// すべてのページの更新を確定する
	void flushAllPages();
	// すべてのページの更新を破棄する
	void recoverAllPages();

	// ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cNewPath_);

	// 入れ替える(マージ中になる)
	void flip();
	// マージ中かどうか
	bool isProceeding();
	// マージ中断に設定する
	void mergeCancel();
	// マージが中断したかどうか(確認後は中断ではなくマージ中になる)
	bool isCanceled();
	// マージ終了に設定する
	void mergeDone();
	// エグゼキュータ側の番号を得る
	int getIndex();

private:
	//
	//	STRUCT
	//	KdTree::InfoFile::Header
	//
	struct Header
	{
		Header()
			: m_uiVersion(0), m_iIndex(0), m_iProceeding(0) {}

		ModUInt32	m_uiVersion;	// バージョン
		
		ModInt32	m_iIndex;		// エグゼキュータ側の索引
		ModInt32	m_iProceeding;	// マージ中かどうか
									// 0: マージ中ではない
									// 1: マージ中、2: マージ中断
	};
	
	// 初期化する
	void initialize();

	// ヘッダーを読み込む
	void readHeader();

	// ヘッダー
	Header m_cHeader;
	// ページ
	PhysicalFile::Page* m_pPhysicalPage;
	// ヘッダーがdirtyか否か
	bool m_bDirty;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_INFOFILE_H

//
//	Copyright (c) 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
