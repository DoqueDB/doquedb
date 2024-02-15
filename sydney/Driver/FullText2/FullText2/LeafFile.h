// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LeafFile.h -- リーフファイルをあらわすクラス
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

#ifndef __SYDNEY_FULLTEXT2_LEAFFILE_H
#define __SYDNEY_FULLTEXT2_LEAFFILE_H

#include "FullText2/Module.h"
#include "FullText2/IndexFile.h"
#include "FullText2/FileID.h"
#include "FullText2/LeafPage.h"
#include "FullText2/PagePointer.h"
#include "PhysicalFile/File.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class InvertedUnit;

//
//	CLASS
//	FullText2::LeafFile -- リーフファイルをあらわすクラス
//
//	NOTES
//	リーフファイルにはショートリストと、ミドルリストのヘッダーが記録されている
//
class LeafFile : public IndexFile
{
public:
	//
	//	TYPEDEF
	//	FullText2::LeafFile::PagePointer
	//
	typedef LeafPage::PagePointer PagePointer;

	// コンストラクタ
	LeafFile(const FileID& cFileID_, const Os::Path& cFilePath_, bool batch_);
	// デストラクタ
	virtual ~LeafFile();

	// ファイルを作成する
	void create(InvertedUnit& cInvertedUnit_);

	// 移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cPath_);

	// クリアする
	void clear(InvertedUnit& cInvertedUnit_,
			   bool bForce_);

	// ページをallocateする
	PagePointer allocatePage(PhysicalFile::PageID uiPrevPageID_,
							 PhysicalFile::PageID uiNextPageID_);
	// ページをアタッチする
	PagePointer attachPage(PhysicalFile::PageID uiPageID_);

#ifndef SYD_COVERAGE
	// ファイル状態を出力する
	void reportFile(const Trans::Transaction& cTransaction_,
					ModOstream& stream_);
#endif

};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_LEAFFILE_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
