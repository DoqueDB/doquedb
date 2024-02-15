// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IndexFile.h --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_KDTREE_INDEXFILE_H
#define __SYDNEY_KDTREE_INDEXFILE_H

#include "KdTree/Module.h"
#include "KdTree/SubFile2.h"

#include "KdTree/FileID.h"

#include "Version/File.h"
#include "Version/Page.h"

#include "Os/Path.h"

#include "Trans/TimeStamp.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

class Archiver;

//	CLASS
//	KdTree::IndexFile -- オンメモリのKD-Tree索引を格納するファイル
//
//	NOTES
//
class IndexFile : public SubFile2
{
public:
	//コンストラクタ
	IndexFile(FileID& cFileID_);
	//デストラクタ
	virtual ~IndexFile();

	// アーカイバーを得る
	Archiver* getArchiver(bool bWrite_);

	// 整合性検査を行う
	void verify(const Trans::Transaction& cTransaction_,
				const Admin::Verification::Treatment::Value eTreatment_,
				Admin::Verification::Progress& cProgress_);

	// ページを得る
	Version::Page::Memory allocatePage(Version::Page::ID id_);
	// ページを得る
	Version::Page::Memory fixPage(Version::Page::ID id_);
	
	// すべてのページの更新を確定する
	void flushAllPages();
	// すべてのページの更新を破棄する
	void recoverAllPages();

private:
	// 整合性検査を開始する
	void startVerification(const Trans::Transaction& cTransaction_,
						   Admin::Verification::Treatment::Value eTreatment_,
						   Admin::Verification::Progress& cProgress_);
	// 整合性検査を終了する
	void endVerification();
	
	// ダンプ開始時のタイムスタンプ
	Trans::TimeStamp m_cTimeStamp;

	// 整合性検査か否か
	bool m_bVerify;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_INDEXFILE_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
