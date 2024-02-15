// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDriverManager.h -- 論理ファイルドライバマネージャクラス
// 
// Copyright (c) 1999, 2000, 2002, 2005, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOGICALFILE_FILEDRIVERMANAGER_H
#define __SYDNEY_LOGICALFILE_FILEDRIVERNANAGER_H

#include "LogicalFile/Module.h"
#include "Os/CriticalSection.h"
#include "ModString.h"
#include "ModMap.h"

_SYDNEY_BEGIN

namespace Schema
{
	class Database;
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_LOGICALFILE_BEGIN

class FileDriver;
class LogData;

//
//	CLASS
//	LogicalFile::FileDriverManager -- 論理ファイルドライバマネージャクラス
//
//	NOTES
//	論理ファイルドライバマネージャクラス
//	このクラスのインスタンスが確保されることはないので、
//	Common::Objectのサブクラスにはしない。
//
class FileDriverManager
{
public:
	//論理ファイルドライバマネージャの初期化を行う
	SYD_LOGICALFILE_FUNCTION
	static void initialize();
	//論理ファイルドライバマネージャの後処理を行う
	SYD_LOGICALFILE_FUNCTION
	static void terminate();
	//論理ファイルドライバマネージャの後処理の準備を行う
	SYD_LOGICALFILE_FUNCTION
	static void prepareTerminate();

	//論理ファイルドライバを得る
	SYD_LOGICALFILE_FUNCTION
	static FileDriver* getDriver(int iDriverID_);

#ifndef SYD_COVERAGE
	//論理ファイルドライバを得る
	SYD_LOGICALFILE_FUNCTION
	static FileDriver* getDriver(const ModString& cstrDriverName_);
#endif

	// Undoする
	static void undo(Trans::Transaction& cTransaction_,
					 const LogData& cLogData_,
					 Schema::Database& cDatabase_);
	// Redoする
	static void redo(Trans::Transaction& cTransaction_,
					 const LogData& cLogData_,
					 Schema::Database& cDatabase_);

private:
	//排他制御用のクリティカルセクション
	static Os::CriticalSection m_cCriticalSection;
	//論理ファイルドライバのマップ(ID)
	static ModMap<int, FileDriver*, ModLess<int> >
		m_mapFileDriverByID;
	//論理ファイルドライバのマップ(Name)
	static ModMap<ModString, FileDriver*, ModLess<ModString> >
		m_mapFileDriverByName;
	//初期化カウンター
	static int m_iInitialized;
};

_SYDNEY_LOGICALFILE_END
_SYDNEY_END

#endif //__SYDNEY_LOGICALFILE_FILEDRIVERMANAGER_H

//
//	Copyright (c) 1999, 2000, 2002, 2005, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
