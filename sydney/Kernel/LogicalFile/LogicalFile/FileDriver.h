// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDriver.h -- 論理ファイルドライバの基底クラス
// 
// Copyright (c) 1999, 2000, 2002, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOGICALFILE_FILEDRIVER_H
#define __SYDNEY_LOGICALFILE_FILEDRIVER_H

#include "LogicalFile/Module.h"
#include "Common/ExecutableObject.h"

#include "ModString.h"

_SYDNEY_BEGIN
_SYDNEY_LOGICALFILE_BEGIN

class File;
class FileID;

//
//	CLASS
//	LogicalFile::FileDriver -- 論理ファイルドライバの基底クラス
//
//	NOTES
//	各論理ファイルドライバの共通基底クラス
//
class SYD_LOGICALFILE_FUNCTION FileDriver : public Common::ExecutableObject
{
public:
	//コンストラクタ
	FileDriver();
	//デストラクタ
	virtual ~FileDriver();

	//論理ファイルドライバの初期化を行う(ここでは何も行わない)
	virtual void initialize();
	//論理ファイルドライバの後処理を行う(ここでは何も行わない)
	virtual void terminate();

	// 論理ファイルドライバの処理を止める(ここでは何も行わない)
	virtual void stop();

	// 論理ファイルドライバの処理を再開する(ここでは何も行わない)
	virtual void start();
	
	//論理ファイルドライバの後処理の前準備を行う(ここでは何も行わない)
	virtual void prepareTerminate();
	
	//論理ファイルをアタッチする
	virtual LogicalFile::File* attachFile(const LogicalFile::FileID& cFileID_) const = 0;
	virtual LogicalFile::File* attachFile(const LogicalFile::File* File_) const = 0;
	//論理ファイルをデタッチする
	virtual void detachFile(LogicalFile::File* pFile_) const = 0;
	//論理ファイルドライバのドライバIDを得る
	virtual int getDriverID() const = 0;
	//論理ファイルドライバのドライバ名を得る
	virtual ModString getDriverName() const = 0;

;
};

_SYDNEY_LOGICALFILE_END
_SYDNEY_END

//
//	FUNCTION global
//	DBGetFileDriver -- 論理ファイルドライバを得る
//
//	NOTES
//	論理ファイルドライバを得るエントリ関数。
//	論理ファイルドライバライブラリから論理ファイルドライバを得るのに
//	使用する関数。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	LogicalFile::FileDriver*
//		論理ファイルドライバへのポインタ
//
//	EXCEPTIONS
//	???
//
extern "C" SYD_FUNCTION
_SYDNEY::LogicalFile::FileDriver* DBGetFileDriver();

#endif //__SYDNEY_LOGICALFILE_FILEDRIVER_H

//
//	Copyright (c) 1999, 2000, 2002, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
