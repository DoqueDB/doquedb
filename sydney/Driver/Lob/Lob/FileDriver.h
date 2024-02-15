// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDriver.h -- 
// 
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOB_FILEDRIVER_H
#define __SYDNEY_LOB_FILEDRIVER_H

#include "Lob/Module.h"

#include "LogicalFile/FileDriver.h"
#include "LogicalFile/FileID.h"

_SYDNEY_BEGIN
_SYDNEY_LOB_BEGIN

//
//	CLASS
//	Lob::FileDriver -- ドライバクラス
//
//	NOTES
//
class SYD_LOB_FUNCTION FileDriver : public LogicalFile::FileDriver
{
public:
	// コンストラクタ
	FileDriver();
	// デストラクタ
	virtual ~FileDriver();

	// ファイルドライバを初期化する
	void initialize();
	// ファイルドライバの後処理をする
	void terminate();

	// ファイルをアタッチする
	LogicalFile::File* attachFile(const LogicalFile::FileID& cFileID_) const;
	// ファイルをアタッチする
	LogicalFile::File* attachFile(const LogicalFile::File* pSrcFile_) const;
	// ファイルをデタッチする
	void detachFile(LogicalFile::File* pFile_) const;

	// ファイルドライバのドライバ ID を返す
	int getDriverID() const;

	// ファイルドライバのドライバ名を返す
	ModString getDriverName() const;

private:
	// 親クラスとクラス名が同じ場合の operator= 問題を回避する
	FileDriver&	operator= (const FileDriver&	cObject_);
};

_SYDNEY_LOB_END
_SYDNEY_END

#endif // __SYDNEY_LOB_FILEDRIVER_H

//
//	Copyright (c) 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
