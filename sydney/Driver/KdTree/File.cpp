// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "KdTree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "KdTree/File.h"

#include "LogicalFile/Estimate.h"

#include "Os/File.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
}

//
//	FUNCTION public
//	KdTree::File::File -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
File::File(FileID& cFileID_, const Os::Path& cPath_)
	: m_pTransaction(0), m_eFixMode(Buffer::Page::FixMode::Unknown),
	  m_bVerification(false), m_pProgress(0), m_uiTreatment(0),
	  m_cFileID(cFileID_), m_cPath(cPath_)
{
}

//
//	FUNCTION public
//	KdTree::File::~File -- デストラクタ
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
File::~File()
{
}

//
//  FUNCTION public static
//  Inverted::File::getOverhead -- 1ページを得るコストを得る
//
//  NOTES
//
//  ARGUMENTS
//  ModSize uiPageSize_
//	  1ページのサイズ (byte)
//
//  RETURN
//  double
//	  1ページを得る秒数
//
//  EXCEPTIONS
//
double
File::getOverhead(ModSize uiPageSize_)
{
	double cost = static_cast<double>(uiPageSize_);
	return cost
		/ LogicalFile::Estimate::getTransferSpeed(LogicalFile::Estimate::File);
}

//
//	FUNCTION public
//	KdTree::File::open -- ファイルをオープンする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	Buffer::Page::FixMode::Value eFixMode_
//		FIXモード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::open(const Trans::Transaction& cTransaction_,
		   Buffer::Page::FixMode::Value eFixMode_)
{
	m_pTransaction = &cTransaction_;
	m_eFixMode = eFixMode_;
}

//
//	FUNCTION public
//	KdTree::File::close -- ファイルをクローズする
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
File::close()
{
	m_pTransaction = 0;
	m_eFixMode = Buffer::Page::FixMode::Unknown;
}

//
//	FUNCTION public
//	KdTree::File::startVerification -- 整合性検査を開始する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	Admin::Verification::Treatment::Value uiTreatment_
//		整合性検査で矛盾を見つけた時の対処方法
//	Admin::Verification::Progress& cProgress_
//		整合性検査の経過を格納するクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::startVerification(const Trans::Transaction& cTransaction_,
						Admin::Verification::Treatment::Value uiTreatment_,
						Admin::Verification::Progress& cProgress_)
{
	m_pTransaction = &cTransaction_;
	m_bVerification = true;
	m_uiTreatment = uiTreatment_;
	m_pProgress = &cProgress_;

	if (uiTreatment_ & Admin::Verification::Treatment::Correct)
		m_eFixMode = Buffer::Page::FixMode::Write
						| Buffer::Page::FixMode::Discardable;
	else
		m_eFixMode = Buffer::Page::FixMode::ReadOnly;
}

//
//	FUNCTION public
//	KdTree::File::endVerification -- 整合性検査を終了する
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
File::endVerification()
{
	m_pTransaction = 0;
	m_eFixMode = Buffer::Page::FixMode::Unknown;
	m_bVerification = false;
	m_uiTreatment = 0;
	m_pProgress = 0;
}

//
//	FUNCTION protected
//	KdTree::File::rmdir -- ディレクトリを削除する
//
//	NOTES
//
//	ARGUMENTS
//	const Os::Path& cPath_
//		削除するディレクトリ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::rmdir(const Os::Path& cPath_)
{
	if (cPath_.getLength() == 0)
		// 空なので、何もしない
		return;
	
	// 存在を確認し、あれば削除する

	if (Os::Directory::access(cPath_, Os::Directory::AccessMode::File) == true)
	{
		// 存在するので、削除する
		
		Os::Directory::remove(cPath_);
	}
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
