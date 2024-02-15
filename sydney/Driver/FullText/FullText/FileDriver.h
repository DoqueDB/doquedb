// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDriver.h --
// 
// Copyright (c) 2003, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT_FILEDRIVER_H
#define __SYDNEY_FULLTEXT_FILEDRIVER_H

#include "FullText/Module.h"
#include "LogicalFile/FileDriver.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT_BEGIN

class MergeDaemon;

//
//	CLASS
//	FullText::FileDriver --
//
//	NOTES
//
class SYD_FULLTEXT_FUNCTION FileDriver : public LogicalFile::FileDriver
{
public:
	//コンストラクタ
	FileDriver();
	//デストラクタ
	virtual ~FileDriver();

	// 論理ファイルドライバの初期化を行う
	void initialize();
	// 論理ファイルドライバの後処理を行う
	void terminate();
	// 論理ファイルドライバの後処理の前準備を行う
	void prepareTerminate();
	// 論理ファイルドライバの処理を止める
	void stop();
	// 論理ファイルドライバの処理を再開する
	void start();

	// 論理ファイルをアタッチする
	LogicalFile::File* attachFile(const LogicalFile::FileID& cFileID_) const;
	LogicalFile::File* attachFile(const LogicalFile::File* cFile_) const;
	
	// 論理ファイルをデタッチする
	void detachFile(LogicalFile::File* pFile_) const;
	
	// 論理ファイルドライバのドライバIDを得る
	int getDriverID() const;
	
	// 論理ファイルドライバのドライバ名を得る
	ModString getDriverName() const;

private:
	// マージデーモン
	MergeDaemon* m_pMergeDaemon;
};

_SYDNEY_FULLTEXT_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT_FILEDRIVER_H

//
//	Copyright (c) 2003, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
