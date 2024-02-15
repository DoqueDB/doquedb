// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedFile.h --
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

#ifndef __SYDNEY_FULLTEXT2_INVERTEDFILE_H
#define __SYDNEY_FULLTEXT2_INVERTEDFILE_H

#include "FullText2/Module.h"
#include "FullText2/MultiFile.h"

#include "Os/Path.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class ListManager;
class FileID;

//
//	CLASS
//	FullText2::InvertedFile
//		-- 転置ファイルの基本的なインターフェースを規定するためのクラス
//
//	NOTES
//
class InvertedFile : public MultiFile
{
public:
	// コンストラクタ
	InvertedFile(FileID& cFileID_, const Os::Path& cPath_);
	// デストラクタ
	virtual ~InvertedFile();

	// リストマネージャーを得る
	virtual ListManager* getListManager() = 0;

	// ファイルIDを得る
	FileID& getFileID() { return m_cFileID; }

	// 位置情報が格納されていないかどうか
	bool isNolocation() const { return m_bNolocation; }
	// TFも格納されていないかどうか
	bool isNoTF() const { return m_bNoTF; }
	// 単語単位索引かどうか
	bool isWordIndex() const { return m_bWordIndex; }


protected:
	// ロック名を得る
	const Lock::FileName& getLockName() const;
	
	// ファイルID
	FileID& m_cFileID;
	
	// 単語単位索引かどうか
	bool m_bWordIndex;
	// 位置情報がないかどうか
	bool m_bNolocation;
	// TF値もないかどうか
	bool m_bNoTF;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_INVERTEDFILE_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
