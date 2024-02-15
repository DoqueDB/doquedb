// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDriver.h -- Ｂ＋木ファイルドライバクラスのヘッダーファイル
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

#ifndef __SYDNEY_BTREE_FILEDRIVER_H
#define __SYDNEY_BTREE_FILEDRIVER_H

#include "Btree/Module.h"

#include "LogicalFile/FileDriver.h"
#include "LogicalFile/FileID.h"

_SYDNEY_BEGIN

namespace Btree
{

//
//	CLASS
//	Btree::FileDriver -- Ｂ＋木ファイルドライバクラス
//
//	NOTES
//	Ｂ＋木ファイルドライバクラス。
//
class SYD_BTREE_FUNCTION_TESTEXPORT FileDriver : public LogicalFile::FileDriver
{
public:

	// コンストラクタ
	FileDriver();

	// デストラクタ
	~FileDriver();

	// Ｂ＋木ファイルドライバを初期化する
	void initialize();

	// Ｂ＋木ファイルドライバの後処理をする
	void terminate();

	// Ｂ＋木ファイルをアタッチする
	LogicalFile::File* attachFile(const LogicalFile::FileID&	cFileID_) const;

	// Ｂ＋木ファイルをアタッチする
	LogicalFile::File* attachFile(const LogicalFile::File*	SrcFile_) const;

	// Ｂ＋木ファイルをデタッチする
	void detachFile(LogicalFile::File*	pFile_) const;

	// Ｂ＋木ファイルドライバのドライバ ID を返す
	int getDriverID() const;

	// Ｂ＋木ファイルドライバのドライバ名を返す
	ModString getDriverName() const;

private:
	// 親クラスとクラス名が同じ場合の operator= 問題を回避する
	FileDriver&	operator= (const FileDriver&	cObject_);

}; // end of class FileDriver

} // end of namespace Btree

_SYDNEY_END

#endif // __SYDNEY_BTREE_FILEDRIVER_H

//
//	Copyright (c) 2000, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
