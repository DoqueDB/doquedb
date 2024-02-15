// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MultiFile.cpp --
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
#include "KdTree/MultiFile.h"

#include "Common/Message.h"

#include "Schema/File.h"

#include "Os/Path.h"

#include "Exception/Object.h"
#include "Exception/VerifyAborted.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
}

//
//	FUNCTION public
//	KdTree::MultiFile::MultiFile -- コンストラクタ
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
MultiFile::MultiFile(FileID& cFileID_, const Os::Path& cPath_)
	: File(cFileID_, cPath_)
{
}

//
//	FUNCTION public
//	KdTree::MultiFile::~MultiFile -- デストラクタ
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
MultiFile::~MultiFile()
{
	// メモリの解放は行わない
	m_vecpSubFile.clear();
}

//
//	FUNCTION public
//	KdTree::MultiFile::getSize -- ファイルサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		ファイルサイズ
//
//	EXCEPTIONS
//
ModUInt64
MultiFile::getSize()
{
	ModUInt64 size = 0;
	ModVector<File*>::Iterator i = m_vecpSubFile.begin();
	for (; i != m_vecpSubFile.end(); ++i)
		size += (*i)->getSize();
	return size;
}

//
//	FUNCTION public
//	KdTree::MultiFile::create -- ファイルを作成する
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
MultiFile::create()
{
	//【注意】	サブファイルのディレクトリのケアは行うが、
	//			このクラスのディレクトリのケアは呼び出し側で行う必要あり
	
	ModVector<File*>::Iterator i = m_vecpSubFile.begin();
	try
	{
		for (; i != m_vecpSubFile.end(); ++i)
			// サブファイルを作成する
			(*i)->create();
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		try
		{
			if (i != m_vecpSubFile.begin())
			{
				do
				{
					--i;
					(*i)->destroy();
				} while (i != m_vecpSubFile.begin());
			}
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(getLockName(), false);
		}
		
		_TRMEISTER_RETHROW;
	}
}

//
//	FUNCTION public
//	KdTree::MultiFile::destroy -- ファイルを破棄する
//
//	NOTES
//
//	ARGUMENTS
// 	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MultiFile::destroy(const Trans::Transaction& cTransaction_)
{
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかく破棄する
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	try
	{
		ModVector<File*>::Iterator i = m_vecpSubFile.begin();
		for (; i != m_vecpSubFile.end(); ++i)
			// サブファイルを破棄する
			(*i)->destroy(cTransaction_);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		SydErrorMessage << "Recovery failed." << ModEndl;
		Schema::File::setAvailability(getLockName(), false);
		
		_TRMEISTER_RETHROW;
	}
}

//
//	FUNCTION public
//	KdTree::MultiFile::mount -- ファイルをマウントする
//
//	NOTES
//
//	ARGUMENTS
// 	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MultiFile::mount(const Trans::Transaction& cTransaction_)
{
	if (isAccessible(true) == false)
		return;
	
	ModVector<File*>::Iterator i = m_vecpSubFile.begin();
	try
	{
		for (; i != m_vecpSubFile.end(); ++i)
			// サブファイルをマウントする
			(*i)->mount(cTransaction_);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		try
		{
			if (i != m_vecpSubFile.begin())
			{
				do
				{
					--i;
					(*i)->unmount(cTransaction_);
				} while (i != m_vecpSubFile.begin());
			}
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(getLockName(), false);
		}
		
		_TRMEISTER_RETHROW;
	}
}

//
//	FUNCTION public
//	KdTree::MultiFile::unmount -- ファイルをアンマウントする
//
//	NOTES
//
//	ARGUMENTS
// 	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MultiFile::unmount(const Trans::Transaction& cTransaction_)
{
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかくアンマウントする
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	ModVector<File*>::Iterator i = m_vecpSubFile.begin();
	try
	{
		for (; i != m_vecpSubFile.end(); ++i)
			// サブファイルをアンマウントする
			(*i)->unmount(cTransaction_);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// ここにくるのは mount されているときのみ
		
		try
		{
			if (i != m_vecpSubFile.begin())
			{
				do
				{
					--i;
					(*i)->mount(cTransaction_);
				} while (i != m_vecpSubFile.begin());
			}
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(getLockName(), false);
		}
		
		_TRMEISTER_RETHROW;
	}
}

//
//	FUNCTION public
//	KdTree::MultiFile::flush -- ファイルをフラッシュする
//
//	NOTES
//
//	ARGUMENTS
// 	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MultiFile::flush(const Trans::Transaction& cTransaction_)
{
	if (isMounted(cTransaction_))
	{
		ModVector<File*>::Iterator i = m_vecpSubFile.begin();
		for (; i != m_vecpSubFile.end(); ++i)
			// サブファイルをフラッシュする
			(*i)->flush(cTransaction_);
	}
}

//
//	FUNCTION public
//	KdTree::MultiFile::startBackup -- ファイルのバックアップを開始する
//
//	NOTES
//
//	ARGUMENTS
// 	const Trans::Transaction& cTransaction_
//		トランザクション
//	const bool bRestorable_
//		リストラフラグ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MultiFile::startBackup(const Trans::Transaction& cTransaction_,
					   const bool bRestorable_)
{
	if (isMounted(cTransaction_))
	{
		ModVector<File*>::Iterator i = m_vecpSubFile.begin();
		try
		{
			for (; i != m_vecpSubFile.end(); ++i)
				// サブファイルのバックアップを開始する
				(*i)->startBackup(cTransaction_, bRestorable_);
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			try
			{
				if (i != m_vecpSubFile.begin())
				{
					do
					{
						--i;
						(*i)->endBackup(cTransaction_);
					} while (i != m_vecpSubFile.begin());
				}
			}
			catch (...)
			{
				SydErrorMessage << "Recovery failed." << ModEndl;
				Schema::File::setAvailability(getLockName(), false);
			}
			
			_TRMEISTER_RETHROW;
		}
	} 
}

//
//	FUNCTION public
//	KdTree::MultiFile::endBackup -- ファイルのバックアップを終了する
//
//	NOTES
//
//	ARGUMENTS
// 	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MultiFile::endBackup(const Trans::Transaction& cTransaction_)
{
	if (isMounted(cTransaction_))
	{
		try
		{
			ModVector<File*>::Iterator i = m_vecpSubFile.begin();
			for (; i != m_vecpSubFile.end(); ++i)
				// サブファイルのバックアップを終了する
				(*i)->endBackup(cTransaction_);
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(getLockName(), false);
			
			_TRMEISTER_RETHROW;
		}
	}
}

//
//	FUNCTION public
//	KdTree::MultiFile::recover -- 障害から回復する(物理ログ)
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
MultiFile::recover(const Trans::Transaction& cTransaction_,
				   const Trans::TimeStamp& cPoint_)
{
	if (isMounted(cTransaction_))
	{
		ModVector<File*>::Iterator i = m_vecpSubFile.begin();
		for (; i != m_vecpSubFile.end(); ++i)
			// サブファイルを障害から回復する
			(*i)->recover(cTransaction_, cPoint_);
	}
}

//
//	FUNCTION public
//	KdTree::MultiFile::restore
//		-- ある時点に開始された読取専用トランザクションが
//		   参照する版を最新版とする
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
MultiFile::restore(const Trans::Transaction& cTransaction_,
				   const Trans::TimeStamp& cPoint_)
{
	if (isMounted(cTransaction_))
	{
		ModVector<File*>::Iterator i = m_vecpSubFile.begin();
		for (; i != m_vecpSubFile.end(); ++i)
			// サブファイルをrestoreする
			(*i)->restore(cTransaction_, cPoint_);
	}
}

//
//	FUNCTION public
//	KdTree::MultiFile::open -- 論理ファイルをオープンする
//
//	NOTES
//
//	ARGUMETNS
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
MultiFile::open(const Trans::Transaction& cTransaction_,
				Buffer::Page::FixMode::Value eFixMode_)
{
	File::open(cTransaction_, eFixMode_);
		
	ModVector<File*>::Iterator i = m_vecpSubFile.begin();
	try
	{
		for (; i != m_vecpSubFile.end(); ++i)
			// サブファイルをオープンする
			(*i)->open(cTransaction_, eFixMode_);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		try
		{
			if (i != m_vecpSubFile.begin())
			{
				do
				{
					--i;
					(*i)->close();
				} while (i != m_vecpSubFile.begin());
			}
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(getLockName(), false);
		}
		
		_TRMEISTER_RETHROW;
	}
}

//
//	FUNCTION public
//	KdTree::MultiFile::close -- 論理ファイルをクローズする
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
MultiFile::close()
{
	File::close();
	
	try
	{
		ModVector<File*>::Iterator i = m_vecpSubFile.begin();
		for (; i != m_vecpSubFile.end(); ++i)
			// サブファイルをクローズする
			(*i)->close();
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		_TRMEISTER_RETHROW;
	}
}

//
//	FUNCTION public
//	KdTree::MultiFile::sync -- 同期をとる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	cTransaction_
//		   	トランザクションのトランザクション記述子
//		bool&				incomplete
//			true
//				一部に処理し残しがある
//			false
//				完全に処理できた
//
//				同期処理の結果、処理し残したかが返る
//		bool&				modified
//			true
//				一部が更新された
//			false
//				更新されていない
//
//				同期処理の結果、更新されたかを設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//
void
MultiFile::sync(const Trans::Transaction& cTransaction_,
				bool& incomplete, bool& modified)
{
	if (isMounted(cTransaction_))
	{
		ModVector<File*>::Iterator i = m_vecpSubFile.begin();
		for (; i != m_vecpSubFile.end(); ++i)
			// サブファイルの同期をとる
			(*i)->sync(cTransaction_, incomplete, modified);
	}
}

//
//	FUNCTION public
//	KdTree::MultiFile::flushAllPages -- すべてのページの更新を確定する
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
MultiFile::flushAllPages()
{
	try
	{
		ModVector<File*>::Iterator i = m_vecpSubFile.begin();
		for (; i != m_vecpSubFile.end(); ++i)
			// サブファイルの更新を確定する
			(*i)->flushAllPages();
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		SydErrorMessage << "Recovery failed." << ModEndl;
		Schema::File::setAvailability(getLockName(), false);

		_TRMEISTER_RETHROW;
	}
}

//
//	FUNCTION public
//	KdTree::MultiFile::recoverAllPages -- すべてのページの更新を破棄する
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
MultiFile::recoverAllPages()
{
	try
	{
		ModVector<File*>::Iterator i = m_vecpSubFile.begin();
		for (; i != m_vecpSubFile.end(); ++i)
			// サブファイルの更新を破棄する
			(*i)->recoverAllPages();
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		SydErrorMessage << "Recovery failed." << ModEndl;
		Schema::File::setAvailability(getLockName(), false);

		_TRMEISTER_RETHROW;
	}
}

//
//	FUNCTION public
//	KdTree::MultiFile::isMounted -- マウントされているか調べる
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	bool
//		マウントされている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
MultiFile::isMounted(const Trans::Transaction& trans) const
{
	// 先頭の要素だけの結果を返す
	ModVector<File*>::ConstIterator i = m_vecpSubFile.begin();
	return (*i)->isMounted(trans);
}

//
//	FUNCTION public
//	KdTree::MultiFile::isAccessible -- 実態である OS ファイルが存在するか調べる
//
//	NOTES
//
//	ARGUMENTS
//	bool bForce_ (default false)
//		強制モード否か
//
//	RETURN
//	bool
//		ファイルが存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
MultiFile::isAccessible(bool bForce_) const
{
	// 先頭の要素だけの結果を返す
	ModVector<File*>::ConstIterator i = m_vecpSubFile.begin();
	return (*i)->isAccessible(bForce_);
}

//
//	FUNCTION public
//	KdTree::MultiFile::move -- ファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
// 	const Trans::Transaction& cTransaction_
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
MultiFile::move(const Trans::Transaction& cTransaction_,
				const Os::Path& cPath_)
{
	//【注意】	サブファイルのディレクトリのケアは行うが、
	//			このクラスのディレクトリのケアは呼び出し側で行う必要あり
	
	ModVector<File*>::Iterator i = m_vecpSubFile.begin();
	try
	{
		for (; i != m_vecpSubFile.end(); ++i)
		{
			// サブファイルをmoveする

			Os::Path cNewPath(cPath_);
			cNewPath.addPart((*i)->getPath().getPart());
			(*i)->move(cTransaction_, cNewPath);
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		try
		{
			if (i != m_vecpSubFile.begin())
			{
				// move したファイルを元に戻す
				do
				{
					--i;
					Os::Path cOldPath(getPath());
					cOldPath.addPart((*i)->getPath().getPart());
					(*i)->move(cTransaction_, cOldPath);
				} while (i != m_vecpSubFile.begin());
			}
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(getLockName(), false);
		}
		
		_TRMEISTER_RETHROW;
	}

	// 新しいパスを設定する
	setPath(cPath_); 
}

//
//	FUNCTION protected
//	KdTree::MultiFile::startVerification -- 整合性検査を開始する
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
MultiFile::startVerification(
	const Trans::Transaction& cTransaction_,
	Admin::Verification::Treatment::Value uiTreatment_,
	Admin::Verification::Progress& cProgress_)
{
	File::startVerification(cTransaction_, uiTreatment_, cProgress_);

	ModVector<File*>::Iterator i = m_vecpSubFile.begin();
	try
	{
		for (; i != m_vecpSubFile.end(); ++i)
			// サブファイルの整合性検査を開始する
			(*i)->startVerification(cTransaction_, uiTreatment_, cProgress_);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		try
		{
			if (i != m_vecpSubFile.begin())
			{
				do
				{
					--i;
					(*i)->endVerification();
				} while (i != m_vecpSubFile.begin());
			}
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(getLockName(), false);
		}
		
		_TRMEISTER_RETHROW;
	} 
}

//
//	FUNCTION public
//	KdTree::MultiFile::endVerification -- 整合性検査を終了する
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
MultiFile::endVerification()
{
	File::endVerification();
	
	try
	{
		ModVector<File*>::Iterator i = m_vecpSubFile.begin();
		for (; i != m_vecpSubFile.end(); ++i)
			// サブファイルの整合性検査を終了する
			(*i)->endVerification();
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		SydErrorMessage << "Recovery failed." << ModEndl;
		Schema::File::setAvailability(getLockName(), false);
			
		_TRMEISTER_RETHROW;
	}
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
