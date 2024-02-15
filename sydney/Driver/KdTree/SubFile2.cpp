// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SubFile2.cpp -- ベクターファイル
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
#include "KdTree/SubFile2.h"

#include "Version/File.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
}

//
//	FUNCTION public
//	KdTree::SubFile2::SubFile2 -- コンストラクタ
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
SubFile2::SubFile2(FileID& cFileID_, const Os::Path& cPath_)
	: File(cFileID_, cPath_), m_pVersionFile(0), m_bMounted(false)
{
	// ファイルをアタッチする
	attach();
}

//
//	FUNCTION public
//	KdTree::SubFile2::~SubFile2 -- デストラクタ
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
SubFile2::~SubFile2()
{
	// ファイルをデタッチする
	detach();
}

//
//	FUNCTION public
//	KdTree::SubFile2::create -- ファイルを作成する
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
SubFile2::create()
{
	try
	{
		// バージョンファイルに作成を依頼する
		m_pVersionFile->create(*m_pTransaction);
	}
	catch (...)
	{
		// ディレクトリを破棄する
		//
		//【注意】	ディレクトリは
		//			実体である物理ファイルの生成時に
		//			必要に応じて生成されるが、
		//			エラー時には削除されないので、
		//			この関数で削除する必要がある

		rmdir(getPath());
		
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	KdTree::SubFile2::destroy -- ファイルを破棄する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SubFile2::destroy(const Trans::Transaction& cTransaction_)
{
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかく削除する
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	m_bMounted = false;
	m_pVersionFile->destroy(cTransaction_);

	// フォルダーを削除する
	rmdir(getPath());
}

//
//	FUNCTION public
//	KdTree::SubFile2::recover -- リカバリする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Trans::TimeStamp& cPoint_
//		チェックポイント
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SubFile2::recover(const Trans::Transaction& cTransaction_,
					const Trans::TimeStamp& cPoint_)
{
	if (isMounted(cTransaction_))
	{
		m_pVersionFile->recover(cTransaction_, cPoint_);

		if (!isAccessible())
		{
			// リカバリの結果
			// 実体である OS ファイルが存在しなくなったので、
			// サブディレクトを削除する

			rmdir(getPath());
		}
	}
}

//
//	FUNCTION public
//	KdTree::SubFile2::move -- ファイルを移動する
//
//	NOTES
//	移動元と移動先のパスが異なっていることが前提。
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Os::Path& cPath_
//		移動先のパス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SubFile2::move(const Trans::Transaction& cTransaction_,
			   const Os::Path& cPath_)
{
	// ファイルがあるか
	bool accessible = (isAccessible() &&
					   Os::Path::compare(getPath(), cPath_)
					   == Os::Path::CompareResult::Unrelated);

	// 一時ファイルか
	bool temporary =
		(m_pVersionFile->getBufferingStrategy()._category
		 == Buffer::Pool::Category::Temporary);
	
	int step = 0;
	try
	{
		Version::File::StorageStrategy::Path cVersionPath;
		cVersionPath._masterData = cPath_;
		if (!temporary)
		{
			cVersionPath._versionLog = cPath_;
			cVersionPath._syncLog = cPath_;
		}
		
		m_pVersionFile->move(cTransaction_, cVersionPath);
		step++;
		if (accessible)
			// 古いディレクトリを削除する
			rmdir(getPath());
		step++;
	}
	catch (...)
	{
		switch (step)
		{
		case 1:
			{
				Version::File::StorageStrategy::Path cVersionPath;
				cVersionPath._masterData = getPath();
				if (!temporary)
				{
					cVersionPath._versionLog = getPath();
					cVersionPath._syncLog = getPath();
				}
				m_pVersionFile->move(cTransaction_, cVersionPath);
			}
		case 0:
			if (accessible)
				rmdir(cPath_);
		}
		_SYDNEY_RETHROW;
	}

	// 新しいパスを設定する
	setPath(cPath_);
}

//
//	FUNCTION protected
//	KdTree::SubFile2::attach -- 物理ファイルをアタッチする
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
SubFile2::attach()
{
	Version::File::StorageStrategy cStorageStrategy;
	Version::File::BufferingStrategy cBufferingStrategy;

	//
	//	格納戦略を設定する
	//

	// マウントされているか
	cStorageStrategy._mounted = m_cFileID.isMounted();
	// 読み取り専用か
	cStorageStrategy._readOnly = m_cFileID.isReadOnly();
	// ページサイズ
	cStorageStrategy._pageSize = m_cFileID.getPageSize();

	// マスタデータファイルの親ディレクトリの絶対パス名
	cStorageStrategy._path._masterData = getPath();
	if (m_cFileID.isTemporary() == false)
	{
		// バージョンログファイルの親ディレクトリの絶対パス名
		cStorageStrategy._path._versionLog = getPath();
		// 同期ログファイルの親ディレクトリの絶対パス名
		cStorageStrategy._path._syncLog = getPath();
	}

	// マスタデータファイルの最大ファイルサイズ(B 単位)
	cStorageStrategy._sizeMax._masterData = 0;
	// バージョンログファイルの最大ファイルサイズ(B 単位)
	cStorageStrategy._sizeMax._versionLog = 0;
	// 同期ログファイルの最大ファイルサイズ(B 単位)
	cStorageStrategy._sizeMax._syncLog = 0;

	// マスタデータファイルのエクステンションサイズ(B 単位)
	cStorageStrategy._extensionSize._masterData = 0;
	// バージョンログファイルのエクステンションサイズ(B 単位)
	cStorageStrategy._extensionSize._versionLog = 0;
	// 同期ログファイルのエクステンションサイズ(B 単位)
	cStorageStrategy._extensionSize._syncLog = 0;

	
	//
	//	バッファリング戦略を設定する
	//
	if (m_cFileID.isTemporary())
	{
		// 一時なら
		cBufferingStrategy._category = Buffer::Pool::Category::Temporary;
	}
	else if (m_cFileID.isReadOnly())
	{
		// 読み取り専用なら
		cBufferingStrategy._category = Buffer::Pool::Category::ReadOnly;
	}
	else
	{
		// その他
		cBufferingStrategy._category = Buffer::Pool::Category::Normal;
	}


	// バージョンファイルをアタッチする
	m_pVersionFile = Version::File::attach(cStorageStrategy,
										   cBufferingStrategy,
										   m_cFileID.getLockName());
}

//
//	FUNCTION protected
//	KdTree::SubFile2::detach -- 物理ファイルをデタッチする
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
SubFile2::detach()
{
	if (m_pVersionFile)
	{
		Version::File::detach(m_pVersionFile, true);
		m_pVersionFile = 0;
	}
}

//
//	FUNCTION protected
//	KdTree::SubFile2::startVerification -- 整合性検査を開始する
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
SubFile2::startVerification(
	const Trans::Transaction& cTransaction_,
	Admin::Verification::Treatment::Value uiTreatment_,
	Admin::Verification::Progress& cProgress_)
{
	// スーパークラス
	File::startVerification(cTransaction_, uiTreatment_, cProgress_);

	try
	{
		// バージョンファイルへの開始通知
		m_pVersionFile->startVerification(cTransaction_,
										  uiTreatment_,
										  cProgress_,
										  false);

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		File::endVerification();
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION protected
//	KdTree::SubFile2::endVerification -- 整合性検査を終了する
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
SubFile2::endVerification()
{
	// 物理ファイルへの終了通知
	m_pVersionFile->endVerification(*m_pTransaction, *m_pProgress);
	
	File::endVerification();
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
