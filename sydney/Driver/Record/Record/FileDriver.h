// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDriver.h -- 可変長レコードファイルの論理ファイルドライバクラス
// 
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD_FILEDRIVER_H
#define __SYDNEY_RECORD_FILEDRIVER_H

#include "Record/Module.h"

#include "LogicalFile/FileDriver.h"

_SYDNEY_BEGIN

namespace Record
{

//
//	CLASS
//	Record::FileDriver -- 可変長レコードファイルの論理ファイルドライバクラス
//
//	NOTES
//	可変長レコードファイルの論理ファイルドライバクラス
//
class SYD_RECORD_FUNCTION_TESTEXPORT FileDriver : public LogicalFile::FileDriver
{
public:

	// コンストラクタ
	FileDriver();

	// デストラクタ
	~FileDriver();

	// レコードファイルドライバの初期化を行なう
	void initialize();

	// レコードファイルドライバの後処理を行なう
	void terminate();

	// 可変長レコードファイルをアタッチする
	LogicalFile::File* attachFile(const LogicalFile::FileID&	cFileOption_) const;
	LogicalFile::File* attachFile(const LogicalFile::File*	pFile_) const;

	// 可変長レコードファイルをデタッチする
	void detachFile(LogicalFile::File*	pFile_) const;

	// 可変長レコードファイルの論理ファイルドライバのドライバ ID を返す
	int getDriverID() const;

	// 可変長レコードファイルの論理ファイルドライバのドライバ名を返す
	ModString getDriverName() const;

private:
	// 親クラスとクラス名が同じ場合の operator= 問題を回避する
	FileDriver&	operator= (const FileDriver&	cObject_);
};

} // end of namespace Record

_SYDNEY_END

#endif // __SYDNEY_RECORD_FILEDRIVER_H

//
//	Copyright (c) 2000, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
