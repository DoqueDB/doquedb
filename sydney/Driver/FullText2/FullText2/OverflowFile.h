// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OverflowFile.h --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_OVERFLOWFILE_H
#define __SYDNEY_FULLTEXT2_OVERFLOWFILE_H

#include "FullText2/Module.h"
#include "FullText2/IndexFile.h"
#include "FullText2/FileID.h"
#include "FullText2/OverflowPage.h"
#include "PhysicalFile/File.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::OverflowFile --
//
//	NOTES
//
//
class OverflowFile : public IndexFile
{
public:
	//
	//	TYPEDEF
	//	FullText2::OverflowFile::PagePointer
	//
	typedef OverflowPage::PagePointer PagePointer;

	//コンストラクタ
	OverflowFile(const FileID& cFileID_,
				 const Os::Path& cFilePath_, bool batch_);
	//デストラクタ
	virtual ~OverflowFile();

	// IDページを確保する
	PagePointer allocatePage(ModSize uiBlockSize_);
	// LOCページを確保する
	PagePointer allocatePage(PhysicalFile::PageID uiPrevID_,
							 PhysicalFile::PageID uiNextID_);
	// ID-LOCページを確保する
	PagePointer allocatePage(ModSize uiBlockSize_,
							 PhysicalFile::PageID uiPrevID_,
							 PhysicalFile::PageID uiNextID_);

	// ページをattachする
	PagePointer attachPage(PhysicalFile::PageID uiPageID_);

	// ページを開放する
	void freePage(OverflowPage* pPage_);
	// ページを解放する
	void freePageCore(OverflowPage* pPage_) { IndexFile::freePage(pPage_); }

	// 移動する
	void move(const Trans::Transaction& cTransaction_, const Os::Path& cPath_);

#ifndef SYD_COVERAGE
	// ファイル状態を出力する
	void reportFile(const Trans::Transaction& cTransaction_,
					ModOstream& stream_);
#endif

private:
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_OVERFLOWFILE_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
