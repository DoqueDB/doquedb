// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileInformation.cpp -- レコードファイル管理情報クラス
// 
// Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Vector";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Vector/File.h"
#include "Vector/FileInformation.h"

#include "ModTime.h"

#include "Common/Assert.h"
#include "Common/Message.h"
#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"
#include "FileCommon/VectorKey.h"
#include "Vector/PageManager.h"

_SYDNEY_USING

namespace { //無名名前空間

//
//	CONST
//	$$$::*Offset, *Size -- 各種のオフセット及びサイズ
//
//	NOTES
//

using namespace FileCommon;

const ModOffset m_ulFileVersionOffset = 0;

const ModOffset m_ulObjectCountOffset
	= m_ulFileVersionOffset
// ↓ModUInt32ArchiveSizeでないのは互換性を保つための措置
	+ DataManager::getFixedCommonDataArchiveSize(Common::DataType::DateTime);

const ModOffset m_ulFirstVectorKeyOffset
	= m_ulObjectCountOffset 
	+ DataManager::getModUInt32ArchiveSize();

const ModOffset m_ulLastVectorKeyOffset
	= m_ulFirstVectorKeyOffset
	+ DataManager::getVectorKeyArchiveSize();

const ModOffset m_ulLastModifiedTimeOffset
	= m_ulLastVectorKeyOffset
	+ DataManager::getVectorKeyArchiveSize();

const ModSize m_ulFileInformationSize
	= m_ulLastModifiedTimeOffset
	+ DataManager::getFixedCommonDataArchiveSize(Common::DataType::DateTime);
}

using namespace Vector;

//	FUNCTION public
//	Vector::FileInformation::FileInformation -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
FileInformation::FileInformation(
    const PhysicalFile::Page* pPage_,
	const Buffer::Page::FixMode::Value	eFixMode_,
	bool /*bFirst_*/)
	: m_pPage(pPage_),
	  m_eFixMode(eFixMode_)
{
	SydAssert(m_pPage != 0);
}

//	FUNCTION public
//	Vector::FileInformation::FileInformation -- デストラクタ
//
//	NOTES
//	デストラクタ。
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
FileInformation::~FileInformation()
{
#if 0 //現在タイムスタンプを入れる予定はないのでコメントアウト
	if (m_eFixMode == Buffer::Page::FixMode::Write)
	{
		setLastUpdateTime();
	}
#endif
}

//
PhysicalFile::Page*
FileInformation::getPage()
{
	return const_cast<PhysicalFile::Page*>(m_pPage);
}

#ifdef OBSOLETE
//
const ModUInt32
FileInformation::getVersion() const
{
	Common::UnsignedIntegerData cVer;
	FileCommon::DataManager::accessToCommonData(
		 m_pPage, m_ulFileVersionOffset,
		 cVer, FileCommon::DataManager::AccessRead);
	return cVer.getValue();
}
#endif

#ifdef OBSOLETE
//	FUNCTION public
//	Vector::FileInformation::getLastUpdateTime -- 最終更新時刻を取得
//
//	NOTES
//	最終更新時刻を取得
//  !leaks!
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Common::DateTimeData
//		最終更新時刻オブジェクト
//
//	EXCEPTIONS
//	なし
//
const Common::DateTimeData
FileInformation::getLastUpdateTime() const
{
	Common::DateTimeData cDateTimeData;
	FileCommon::DataManager::accessToCommonData(
		 m_pPage, m_ulLastModifiedTimeOffset,
		 cDateTimeData, FileCommon::DataManager::AccessRead);
	return cDateTimeData;
}
#endif

//	FUNCTION public
//	Vector::FileInformation::getObjectCount -- 挿入されているオブジェクト数を取得
//
//	NOTES
//	挿入されているオブジェクト数を取得
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModUInt32
//
//	EXCEPTIONS
//	なし
//
const ModUInt32
FileInformation::getObjectCount() const
{
	Common::UnsignedIntegerData cResult;
	FileCommon::DataManager::accessToCommonData(
		 m_pPage, m_ulObjectCountOffset,
		 cResult, FileCommon::DataManager::AccessRead);
	return cResult.getValue();
}

//	FUNCTION public
//	Vector::FileInformation::getFirstVectorKey
//	  -- 先頭オブジェクトのベクタキーを取得
//
//	NOTES
//	先頭オブジェクトのベクタキーを取得。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModUInt32
//
//	EXCEPTIONS
//	なし
//
const ModUInt32
FileInformation::getFirstVectorKey() const
{
	Common::UnsignedIntegerData cResult;
	FileCommon::DataManager::accessToCommonData(
		 m_pPage, m_ulFirstVectorKeyOffset,
		 cResult, FileCommon::DataManager::AccessRead);
	return cResult.getValue();
}

//	FUNCTION public
//	Vector::FileInformation::getLastVectorKey
//	  -- 最終オブジェクトのベクタキーを取得
//
//	NOTES
//	最終オブジェクトのベクタキーを取得。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModUInt32
//
//	EXCEPTIONS
//	なし
//
const ModUInt32
FileInformation::getLastVectorKey() const
{
	Common::UnsignedIntegerData cResult;
	FileCommon::DataManager::accessToCommonData(
		 m_pPage, m_ulLastVectorKeyOffset,
		 cResult, FileCommon::DataManager::AccessRead);
	return cResult.getValue();
}

//	FUNCTION
//	FileInformation::getFixMode
//	  -- ファイル情報オブジェクトの読み込みモードを取得する
//
//	NOTES
//	ファイル情報オブジェクトの読み込みモードを取得する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Buffer::Page::FixMode::Value
//
//	EXCEPTIONS
//	なし
//
const Buffer::Page::FixMode::Value
FileInformation::getFixMode() const
{
	return m_eFixMode;
}

// マニピュレータ

//
//	FUNCTION public
//	Vector::FileInformation::setObjectCount -- 挿入されているオブジェクト数を設定
//
//	NOTES
//	挿入されているオブジェクト数を設定
//
//	ARGUMENTS
//	const ModUInt32 ulObjectCount_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileInformation::setObjectCount(ModUInt32 ulObjectCount_)
{
	Common::UnsignedIntegerData cObjectCount(ulObjectCount_);
	FileCommon::DataManager::accessToCommonData(
		 m_pPage, m_ulObjectCountOffset,
		 cObjectCount, FileCommon::DataManager::AccessWrite);
}

//
//	FUNCTION public
//	Vector::FileInformation::setFirstVectorKey 
//	  -- 先頭オブジェクトのベクタキーを設定
//
//	NOTES
//	先頭オブジェクトのベクタキーを設定
//
//	ARGUMENTS
//	const ModUInt32 ulVectorKey_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileInformation::setFirstVectorKey(ModUInt32 ulVectorKey_)
{
	Common::UnsignedIntegerData cVectorKey(ulVectorKey_);
	FileCommon::DataManager::accessToCommonData(
		 m_pPage, m_ulFirstVectorKeyOffset,
		 cVectorKey, FileCommon::DataManager::AccessWrite);
}

//
//	FUNCTION public
//	Vector::FileInformation::setLastVectorKey 
//	  -- 最終オブジェクトのベクタキーを設定
//
//	NOTES
//	最終オブジェクトのベクタキーを設定
//
//	ARGUMENTS
//	const ModUInt32 ulVectorKey_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileInformation::setLastVectorKey(ModUInt32 ulVectorKey_ )
{
	Common::UnsignedIntegerData cVectorKey(ulVectorKey_);
	FileCommon::DataManager::accessToCommonData(
		 m_pPage, m_ulLastVectorKeyOffset,
		 cVectorKey, FileCommon::DataManager::AccessWrite);
}

// private functions

// バージョン番号をページに埋め込む
void
FileInformation::setVersion()
{
	Common::UnsignedIntegerData version(File::CurrentVersion);
	FileCommon::DataManager::accessToCommonData(
		m_pPage, m_ulFileVersionOffset,
		version,
		FileCommon::DataManager::AccessWrite);
}

// メンバ変数「最終更新時刻」に現在の時刻をセットする
void
FileInformation::setLastUpdateTime()
{
	ModTime	cModTime(ModTime::getCurrentTime());
	Common::DateTimeData cLastModifiedTime;
	cLastModifiedTime.setValue(cModTime.getYear(), cModTime.getMonth(),
								 cModTime.getDay(), cModTime.getHour(),
								 cModTime.getMinute(), cModTime.getSecond());
	SydAssert(m_pPage!=0);
	FileCommon::DataManager::accessToCommonData(
		m_pPage, m_ulLastModifiedTimeOffset,
		cLastModifiedTime, FileCommon::DataManager::AccessWrite);
}

//
//	FUNCTION public
//	Vector::FileInformation::initializePhysicalPage -- 管理情報用ページの初期化
//
//	NOTES
//	管理情報用の初期化を行う。
//	これを用いるのはFile::createのみ。
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileInformation::initialize()
{
	setVersion();
	setObjectCount(0); 
	setFirstVectorKey(FileCommon::VectorKey::Undefined);
	setLastVectorKey(FileCommon::VectorKey::Undefined);
	setLastUpdateTime();
}

//
//	Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
