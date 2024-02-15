// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDriver.cpp --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "KdTree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "KdTree/FileDriver.h"
#include "KdTree/LogicalInterface.h"
#include "KdTree/KdTreeIndexSet.h"
#include "KdTree/MergeDaemon.h"

#include "LogicalFile/FileDriverTable.h"
#include "LogicalFile/FileID.h"
#include "LogicalFile/File.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

//
//	FUNCTION public
//	KdTree::FileDriver::FileDriver -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
FileDriver::FileDriver()
	: m_pMergeDaemon(0)
{
}

//
//	FUNCTION public
//	KdTree::FileDriver::~FileDriver -- デストラクタ
//
//	NOTES
//	デストラクタ
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
FileDriver::~FileDriver()
{
}

//
//	FUNCTION public
//	KdTree::FileDriver::initialize -- ファイルドライバを初期化する
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
	KdTreeIndexSet::initialize();
	start();
}

//
//	FUNCTION public
//	KdTree::FileDriver::terminate -- ファイルドライバの後処理をする
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
	stop();
	KdTreeIndexSet::terminate();
}

//
//	FUNCTION public
//	KdTree::FileDriver::prepareTerminate
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
	stop();
}

//
//	FUNCTION public
//	KdTree::FileDriver::stop -- 論理ファイルドライバの処理を止める
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
	if (m_pMergeDaemon)
	{
		m_pMergeDaemon->abort();
		m_pMergeDaemon->join();
		delete m_pMergeDaemon;
		m_pMergeDaemon = 0;
	}
}

//
//	FUNCTION public
//	KdTree::FileDriver::start -- 論理ファイルドライバの処理を再開する
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
//	KdTree::FileDriver::attachFile -- ファイルをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cFileID_
//		ファイルId
//
//	RETURN
//	LogicalFile::File*
//		ファイル
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
//	KdTree::FileDriver::attachFile -- ファイルをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::File* pSrcFile_
//		ファイル
//
//	RETURN
//	LogicalFile::File*
//		ファイル
//
//	EXCEPTIONS
//
LogicalFile::File*
FileDriver::attachFile(const LogicalFile::File*	pSrcFile_) const
{
	return new LogicalInterface(pSrcFile_->getFileID());
}

//
//	FUNCTION public
//	KdTree::FileDriver::detachFile -- ファイルをデタッチする
//
//	NOTES
//
//	ARGUMENTS
//	LogicalFile::File* pFile_
//		ファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileDriver::detachFile(LogicalFile::File* pFile_) const
{
	delete pFile_;
}

//
//	FUNCTION public
//	KdTree::FileDriver::getDriverID --
//		KdTreeファイルドライバのドライバIDを返す
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		KdTreeファイルドライバのドライバ ID
//
//	EXCEPTIONS
//	なし
//
int
FileDriver::getDriverID() const
{
	return LogicalFile::FileDriverID::KdTree;
}

//
//	FUNCTION public
//	KdTree::FileDriver::getDriverName --
//		KdTreeファイルドライバのドライバ名を返す
//
//	NOTES
//  KdTreeファイルドライバのドライバ名を返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSting
//		KdTreeファイルドライバのドライバ名
//
//	EXCEPTIONS
//	なし
//
ModString
FileDriver::getDriverName() const
{
	return LogicalFile::FileDriverName::KdTree;
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
