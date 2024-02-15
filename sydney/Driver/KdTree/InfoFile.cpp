// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InfoFile.cpp -- マージ情報を格納するファイル
// 
// Copyright (c) 2014, 2023 Ricoh Company, Ltd.
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
#include "KdTree/InfoFile.h"

#include "PhysicalFile/File.h"

#include "Os/Memory.h"

#include "Exception/BadArgument.h"
#include "Exception/VerifyAborted.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
	// パス
	Os::Path _cSubPath("Info");
}

//
//	FUNCTION public
//	KdTree::InfoFile::InfoFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::FileID& cFileID_
//		ファイルID
//	const Os::Path& cPath_
//		パス
//	bool bBatch_
//		バッチモードか否か
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
InfoFile::InfoFile(KdTree::FileID& cFileID_, const Os::Path& cPath_)
	: SubFile(cFileID_, PhysicalFile::NonManageType,
			  cFileID_.getPageSize(), Os::Path(cPath_).addPart(_cSubPath)),
	  m_pPhysicalPage(0), m_bDirty(false)
{
}

//
//	FUNCTION public
//	KdTree::InfoFile::~InfoFile -- デストラクタ
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
InfoFile::~InfoFile()
{
}

//
//	FUNCTION public
//	KdTree::InfoFile::create -- ファイルを作成する
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
InfoFile::create()
{
	SubFile::create();

	try
	{
		// 管理機能なし物理ファイルはファイル生成時にページが
		// 自動的に確保されるので、allocatePage は呼んではいけない
		
		// 初期化する
		initialize();
	}
	catch (...)
	{
		recoverAllPages();
		destroy(*m_pTransaction);
		_SYDNEY_RETHROW;
	}

	flushAllPages();
}

//
//	FUNCTION public
//	KdTree::InfoFile::verify -- 整合性検査を行う
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Admin::Verification::Treatment::Value eTreatment_
//		対応指示
//	Admin::Verification::Progress& cProgress_
//		経過報告
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InfoFile::verify(const Trans::Transaction& cTransaction_,
				 const Admin::Verification::Treatment::Value eTreatment_,
				 Admin::Verification::Progress& cProgress_)
{
	// 特に検査する項目はない
}

//
//	FUNCTION public
//	KdTree::InfoFile::recoverAllPages -- ページの更新を破棄する
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
InfoFile::recoverAllPages()
{
	if (m_pPhysicalPage)
	{
		if (m_bDirty)
		{
			// 変更を破棄して、読み直す
			
			m_pPhysicalPage->read(*m_pTransaction,
								  &m_cHeader, 0, sizeof(m_cHeader));
			m_bDirty = false;
		}
		
		m_pPhysicalFile->recoverPage(m_pPhysicalPage);
		m_pPhysicalPage = 0;
	}
}

//
//	FUNCTION public
//	KdTree::InfoFile::flushAllPages -- ページの更新を反映する
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
InfoFile::flushAllPages()
{
	if (m_pPhysicalPage)
	{
		if (m_bDirty)
		{
			// 変更を反映させる
			
			m_pPhysicalPage->write(*m_pTransaction,
								   &m_cHeader, 0, sizeof(m_cHeader));
		}
		
		PhysicalFile::Page::UnfixMode::Value mode
			= m_bDirty ? PhysicalFile::Page::UnfixMode::Dirty
			: PhysicalFile::Page::UnfixMode::NotDirty;
		m_pPhysicalFile->detachPage(m_pPhysicalPage, mode);
		m_pPhysicalPage = 0;
		m_bDirty = false;
	}
}

//
//	FUNCTION public
//	KdTree::InfoFile::move -- ファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Os::Path& cNewPath_
//		移動先のパス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InfoFile::move(const Trans::Transaction& cTransaction_,
			   const Os::Path& cNewPath_)
{
	// サブパスをappendして、親クラスを呼び出す

	SubFile::move(cTransaction_, Os::Path(cNewPath_).addPart(_cSubPath));
}

//
//	FUNCTION public
//	KdTree::VectorFile::flip -- 差分索引等を入れ替える
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCETPIONS
//
void
InfoFile::flip()
{
	// 差分ファイルを入れ替える
	// 同時にマージ中に設定する
	
	// まだ読み込んでなければヘッダーを読み込む
	readHeader();

	if (m_cHeader.m_iProceeding)
		// マージ中なら何もしない
		return;

	// マージ中にして、差分ファイルを入れ替える
	m_cHeader.m_iProceeding = 1;
	m_cHeader.m_iIndex = (m_cHeader.m_iIndex == 0) ? 1 : 0;

	// ヘッダーを dirty にする
	m_bDirty = true;
}

//
//	FUNCTION public
//	KdTree::InfoFile::isProceeding -- マージ中か否か
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		マージ中だった場合は true 、それ以外の場合は false
//
//	EXCEPTIONS
//
bool
InfoFile::isProceeding()
{
	// まだ読み込んでなければヘッダーを読み込む
	readHeader();

	return (m_cHeader.m_iProceeding != 0) ? true : false;
}

//
//	FUNCTION public
//	KdTree::InfoFile::mergeCancel -- マージ中断に設定する
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
InfoFile::mergeCancel()
{
	// まだ読み込んでなければヘッダーを読み込む
	readHeader();

	// ヘッダーを更新する
	m_cHeader.m_iProceeding = 2;
	// ヘッダーを dirty にする
	m_bDirty = true;
}

//
//	FUNCTION public
//	KdTree::InfoFile::isCanceled -- マージを中断したかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		キャンセルした場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InfoFile::isCanceled()
{
	// マージを中断したか否かを返す
	// 中断していた場合、本メソッド呼出し後はマージ中になる
	
	// ヘッダーを読む
	readHeader();

	bool result = false;
	if (m_cHeader.m_iProceeding == 2)
	{
		// マージが中断された
		result = true;
		m_cHeader.m_iProceeding = 1;	// マージ中にする
		
		// ヘッダーを更新したので、dirty にする
		m_bDirty = true;
	}
	
	return result;
}

//
//	FUNCTION public
//	KdTree::InfoFile::mergeDone -- マージの終了を通知する
//
//	NOTES
//
//	ARGUMENTS
//
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InfoFile::mergeDone()
{
	// ヘッダーを読む
	readHeader();

	if (m_cHeader.m_iProceeding == 0)
		// マージ中じゃないなら何もしない
		return;

	// マージは終わったので、フラグを戻す
	m_cHeader.m_iProceeding = 0;

	// ヘッダーを更新したので、dirty にする
	m_bDirty = true;
}

//
//	FUNCTION
//	KdTree::InfoFile::getIndex
//		-- エグゼキュータ側の小転置の番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		エグゼキュータ側の小転置の番号(0 または 1)
//
//	EXCEPTIONS
//
int
InfoFile::getIndex()
{
	// ヘッダーを読む
	readHeader();

	return m_cHeader.m_iIndex;
}

//
//	FUNCTION private
//	KdTree::InfoFile::readHeader -- ヘッダーページの内容を読み込む
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
InfoFile::readHeader()
{
	if (m_pPhysicalPage == 0)
	{
		m_pPhysicalPage
			= m_pPhysicalFile->attachPage(*m_pTransaction,
										  m_eFixMode,
										  Buffer::ReplacementPriority::Middle);

		m_pPhysicalPage->read(*m_pTransaction,
							  &m_cHeader, 0, sizeof(m_cHeader));
	}
}

//
//	FUNCTION private
//	KdTree::InfoFile::initialize -- 初期化する
//
// 	NOTES
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
InfoFile::initialize()
{
	readHeader();
	m_cHeader.m_uiVersion = 0;
	m_cHeader.m_iIndex = 0;
	m_cHeader.m_iProceeding = 0;
	m_bDirty = true;
}

//
//	Copyright (c) 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
