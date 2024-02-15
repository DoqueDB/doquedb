// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDriver.cpp -- Ｂ＋木ファイルドライバクラスの実現ファイル
// 
// Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Btree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "LogicalFile/FileDriverTable.h"
#include "LogicalFile/FileID.h"

#include "Btree/FileDriver.h"
#include "Btree/File.h"

_SYDNEY_USING

using namespace Btree;

//
//	FUNCTION public
//	Btree::FileDriver::FileDriver -- コンストラクタ
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
//	YET!
//
FileDriver::FileDriver()
	: LogicalFile::FileDriver()
{
}

//
//	FUNCTION public
//	Btree::FileDriver::~FileDriver -- デストラクタ
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
//	YET!
//
FileDriver::~FileDriver()
{
}

//
//	FUNCTION public
//	Btree::FileDriver::initialize -- Ｂ＋木ファイルドライバを初期化する
//
//	NOTES
//	Ｂ＋木ファイルドライバを初期化する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Common::UnexpectedException
//		予想外のエラー
//		( Btree::File::initialize )
//
void
FileDriver::initialize()
{
	Btree::File::initialize();
}

//
//	FUNCTION public
//	Btree::FileDriver::terminate -- Ｂ＋木ファイルドライバの後処理をする
//
//	NOTES
//	Ｂ＋木ファイルドライバの後処理をする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Common::UnexpectedException
//		予想外のエラー
//		( Btree::File::terminate )
//
void
FileDriver::terminate()
{
	Btree::File::terminate();
}

//
//	FUNCTION public
//	Btree::FileDriver::attachFile -- Ｂ＋木ファイルをアタッチする
//
//	NOTES
//	Ｂ＋木ファイルをアタッチする。
//
//	ARGUMENTS
//	const LogicalFile::FileID&	cFileID_
//		Ｂ＋木ファイルの論理ファイル ID オブジェクトへの参照
//
//	RETURN
//	LogicalFile::File*
//		Ｂ＋木ファイル記述子
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//		※ cFileID_ に設定されているパラメータに不正があった場合に発生
//		( Btree::File::File )
//
LogicalFile::File*
FileDriver::attachFile(const LogicalFile::FileID&	cFileID_) const
{
	return new Btree::File(cFileID_);
}

//
//	FUNCTION public
//	Btree::FileDriver::attachFile -- Ｂ＋木ファイルをアタッチする
//
//	NOTES
//	Ｂ＋木ファイルをアタッチする。
//
//	ARGUMENTS
//	const LogicalFile::File*	SrcFile_
//		Ｂ＋木ファイル記述子
//
//	RETURN
//	LogicalFile::File*
//		アタッチしたＢ＋木ファイル
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//		( Btree::File::File )
//
LogicalFile::File*
FileDriver::attachFile(const LogicalFile::File*	SrcFile_) const
{
	return new Btree::File(SrcFile_->getFileID());
}

//
//	FUNCTION public
//	Btree::FileDriver::detachFile -- Ｂ＋木ファイルをデタッチする
//
//	NOTES
//	Ｂ＋木ファイルをデタッチする。
//
//	ARGUMENTS
//	LogicalFile::File*	pFile_
//		デタッチするＢ＋木ファイル記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	YET!
//
void
FileDriver::detachFile(LogicalFile::File*	pFile_) const
{
	delete pFile_;
}

//
//	FUNCTION public
//	Btree::FileDriver::getDriverID --
//		Ｂ＋木ファイルドライバのドライバ ID を返す
//
//	NOTES
//	Ｂ＋木ファイルドライバのドライバ ID を返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		Ｂ＋木ファイルドライバのドライバ ID
//
//	EXCEPTIONS
//	なし
//
int
FileDriver::getDriverID() const
{
	return LogicalFile::FileDriverID::Btree;
}

//
//	FUNCTION public
//	Btree::FileDriver::getDriverName --
//		Ｂ＋木ファイルドライバのドライバ名を返す
//
//	NOTES
//	Ｂ＋木ファイルドライバのドライバ名を返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSting
//		Ｂ＋木ファイルドライバのドライバ名
//
//	EXCEPTIONS
//	なし
//
ModString
FileDriver::getDriverName() const
{
	return LogicalFile::FileDriverName::Btree;
}

//
//	Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
