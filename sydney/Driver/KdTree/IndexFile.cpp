// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IndexFile.cpp --
// 
// Copyright (c) 2012, 2013, 2017, 2023 Ricoh Company, Ltd.
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
#include "KdTree/IndexFile.h"

#include "KdTree/Archiver.h"
#include "KdTree/KdTreeIndex.h"

#include "Os/Path.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
	// サブパス
	Os::Path _cSubPath("Index");
}

//
//	FUNCTION public
//	KdTree::IndexFile::IndexFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
IndexFile::IndexFile(KdTree::FileID& cFileID_)
	: SubFile2(cFileID_, Os::Path(cFileID_.getPath()).addPart(_cSubPath)),
	  m_bVerify(false)
{
}

//
//	FUNCTION public
//	KdTree::IndexFile::~IndexFile -- デストラクタ
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
IndexFile::~IndexFile()
{
}

//
//	FUNCTION public
//	KdTree::IndexFile::getArchiver -- アーカイバーを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Archiver*
//		アーカイバー。呼び出し側で解放すること
//
//	EXCEPTIONS
//
Archiver*
IndexFile::getArchiver(bool bWrite_)
{
	if (bWrite_)
		// リカバリするポイントを保存しておく
		m_cTimeStamp = Trans::TimeStamp::assign();
	
	return new Archiver(*this, bWrite_);
}

//
//	FUNCTION public
//	KdTree::IndexFile::verify -- 整合性検査を行う
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
IndexFile::verify(const Trans::Transaction& cTransaction_,
				  const Admin::Verification::Treatment::Value eTreatment_,
				  Admin::Verification::Progress& cProgress_)
{
	Admin::Verification::Progress cProgress(cProgress_.getConnection());
	
	// 開始
	startVerification(cTransaction_, eTreatment_, cProgress);

	try
	{
		// 索引を読み込んで確認する

		// インスタンスを確保する
		ModAutoPointer<KdTreeIndex> pIndex
			= new KdTreeIndex(m_cFileID.getDimension(),
							  Trans::TimeStamp::assign());

		// ロードする
		pIndex->load(*this);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// 整合性検査を終了する
		endVerification();
		_SYDNEY_RETHROW;
	}

	// 整合性検査を終了する
	endVerification();

	// 経過を追加する
	cProgress_ += cProgress;
}

//
//	FUNCTION public
//	KdTree::IndexFile::allocatePage -- ページを得る
//
//	NOTES
//
//	ARGUMENTS
//	Version::Page::ID id_
//		ページID
//
//	RETURN
//	Version::Page::Memory
//		バージョンページ
//
//	EXCEPTIONS
//
Version::Page::Memory
IndexFile::allocatePage(Version::Page::ID id_)
{
	return Version::Page::fix(*m_pTransaction,
							  *m_pVersionFile,
							  id_,
							  Buffer::Page::FixMode::Allocate,
							  Buffer::ReplacementPriority::Low);
}

//
//	FUNCTION public
//	KdTree::IndexFile::fixPage -- ページを得る
//
//	NOTES
//
//	ARGUMENTS
//	Version::Page::ID id_
//		ページID
//
//	RETURN
//	Version::Page::Memory
//		バージョンページ
//
//	EXCEPTIONS
//
Version::Page::Memory
IndexFile::fixPage(Version::Page::ID id_)
{
	if (m_bVerify)
	{
		Admin::Verification::Progress cProgress(
			m_pProgress->getConnection());

		Version::Page::Memory page
			= Version::Page::verify(*m_pTransaction,
									*m_pVersionFile,
									id_,
									m_eFixMode,
									cProgress);
		*m_pProgress += cProgress;

		return page;
	}
	
	return Version::Page::fix(*m_pTransaction,
							  *m_pVersionFile,
							  id_,
							  Buffer::Page::FixMode::ReadOnly,
							  Buffer::ReplacementPriority::Low);
}

//
//	FUNCTION public
//	KdTree::IndexFile::recoverAllPages -- ページの更新を破棄する
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
IndexFile::recoverAllPages()
{
	if (m_cTimeStamp.isIllegal())
		return;

	// 更新を開始する前の状態にリカバリする
	//
	// オンメモリのKD-Treeが更新された後に、そのメモリ内容のダンプが
	// このファイルに格納される
	// エラー発生時には、保存していたダンプ開始前のタイムスタンプを利用し、
	// その時点にバージョンファイルをリカバリする
	//
	//【注意】	このような方法がとれるのは、他のトランザクションは
	//			このファイルを参照・更新しないことが前提となる
	//			KD-Treeの場合、1回のラッチ内で一気にダンプするので、
	//			他の更新トランザクションが入り込む余地はない
	//			版を利用するトランザクションの場合、すでにロード済みの
	//			オンメモリのKD-Treeを参照するので、このファイルを
	//			参照することはない
	
	m_pVersionFile->recover(*m_pTransaction, m_cTimeStamp);
	m_cTimeStamp = Trans::IllegalTimeStamp;
}

//
//	FUNCTION public
//	KdTree::IndexFile::flushAllPages -- ページの更新を反映する
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
IndexFile::flushAllPages()
{
	// リカバリする必要がないので、タイムスタンプを無効にする
	
	m_cTimeStamp = Trans::IllegalTimeStamp;
}

//
//	FUNCTION protected
//	KdTree::IndexFile::startVerification -- 整合性検査を開始する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	Admin::Verification::Treatment::Value uiTreatment_
//		動作
//	Admin::Verification::Progress& cProgress_
//		経過
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IndexFile::startVerification(
	const Trans::Transaction& cTransaction_,
	Admin::Verification::Treatment::Value uiTreatment_,
	Admin::Verification::Progress& cProgress_)
{
	// スーパークラス
	SubFile2::startVerification(cTransaction_, uiTreatment_, cProgress_);
	m_bVerify = true; 
}

//
//	FUNCTION protected
//	KdTree::IndexFile::endVerification -- 整合性検査を終了する
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
IndexFile::endVerification()
{
	SubFile2::endVerification();
	m_bVerify = false;
}

//
//	Copyright (c) 2012, 2013, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
