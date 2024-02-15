// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDriver.cpp --
// 
// Copyright (c) 2003, 2005, 2011, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "FullText";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText/FileDriver.h"
#include "FullText/LogicalInterface.h"
#include "FullText/MergeDaemon.h"

#include "Inverted/InvertedFile.h"
#include "Inverted/IntermediateFileID.h"

#include "LogicalFile/File.h"
#include "LogicalFile/FileDriverTable.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT_USING

//
//	FUNCTION public
//	FullText::FileDriver::FileDriver -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
FileDriver::FileDriver()
	: m_pMergeDaemon(0)
{
}

//
//	FUNCTION public
//	FullText::FileDriver::~FileDriver -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
FileDriver::~FileDriver()
{
}

//
//	FUNCTION public
//	FullText::FileDriver::initialize -- 論理ファイルドライバの初期化を行う
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileDriver::initialize()
{
	Inverted::InvertedFile::initialize();
	m_pMergeDaemon = new MergeDaemon;
	m_pMergeDaemon->create();
}

//
//	FUNCTION public
//	FullText::FileDriver::terminate -- 論理ファイルドライバの後処理を行う
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileDriver::terminate()
{
	delete m_pMergeDaemon, m_pMergeDaemon = 0;
	Inverted::InvertedFile::terminate();
}

//
//	FUNCTION public
//	FullText2::FileDriver::stop -- 論理ファイルドライバの処理を止める
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileDriver::stop()
{
	m_pMergeDaemon->abort();
	m_pMergeDaemon->join();
	delete m_pMergeDaemon;
	m_pMergeDaemon = 0;
}

//
//	FUNCTION public
//	FullText2::FileDriver::start -- 論理ファイルドライバの処理を再開する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileDriver::start()
{
	m_pMergeDaemon = new MergeDaemon;
	m_pMergeDaemon->create();
}


//
//	FUNCTION public
//	FullText::FileDriver::prepareTerminate
//		-- 論理ファイルドライバの後処理の前準備を行う
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileDriver::prepareTerminate()
{
	m_pMergeDaemon->abort();
	m_pMergeDaemon->join();
}

//
//	FUNCTION public
//	FullText::FileDriver::attachFile -- 論理ファイルをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cFileID_
//		ファイルID クラス
//
//	RETURN
//	LogicalFile::File*
//		ファイルドライバクラスポインタ
//
//	EXCEPTIONS
//
LogicalFile::File* 
FileDriver::attachFile(const LogicalFile::FileID& cFileID_) const
{
	return new LogicalInterface(cFileID_);
}

//
//	FUNCTION public
//	FullText::FileDriver::attachFile -- 論理ファイルをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::File* cFile_
//		ファイル記述子 クラス
//
//	RETURN
//	LogicalFile::File*
//		ファイルドライバクラスポインタ
//
//	EXCEPTIONS
//
LogicalFile::File* 
FileDriver::attachFile(const LogicalFile::File* cFile_) const
{
	return new LogicalInterface(cFile_->getFileID());
}

//
//	FUNCTION public
//	FullText::FileDriver::detachFile -- 論理ファイルをデタッチする
//
//	NOTES
//
//	ARGUMENTS
//	LogicalFile::File* pFile_
//		ファイルポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
FileDriver::detachFile(LogicalFile::File* pFile_) const
{
	delete pFile_;
}

//
//	FUNCTION public
//	FullText::FileDriver::getDriverID -- 論理ファイルドライバのドライバIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		論理ファイルドライバID
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
int
FileDriver::getDriverID() const
{
	return LogicalFile::FileDriverID::FullText;
}

//
//	FUNCTION public
//	FullText::FileDriver::getDriverName
//		-- 論理ファイルドライバのドライバ名を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModString
//		ドライバ名
//
//	EXCEPTIONS
//
ModString
FileDriver::getDriverName() const
{
	return LogicalFile::FileDriverName::FullText;
}

//
//	Copyright (c) 2003, 2005, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
