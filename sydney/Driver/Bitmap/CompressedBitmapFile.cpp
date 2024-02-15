// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CompressedBitmapFile.cpp
//		-- ビット列を圧縮したビットマップ索引は2つのファイルで構成されている
//			1. BitmapFile(継承しているもの)
//			2. DirectAreaFile
//		   圧縮したビット列を格納する領域は可変長になるが、
//		   既存のコードの変更量などを考慮し、可変長部分はDirectAreaFileに
//		   格納することとする。
// 
// Copyright (c) 2007, 2008, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Bitmap";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "Bitmap/CompressedBitmapFile.h"
#include "Bitmap/Condition.h"
#include "Bitmap/ShortBitmapIterator.h"
#include "Bitmap/MiddleBitmapIterator.h"
#include "Bitmap/LongBitmapIterator.h"
#include "Bitmap/MessageAll_Class.h"

#include "Common/DataArrayData.h"
#include "Common/ObjectIDData.h"
#include "Common/Assert.h"
#include "Common/Message.h"

#include "PhysicalFile/Manager.h"
#include "PhysicalFile/File.h"
#include "PhysicalFile/DirectArea.h"

#include "Version/File.h"

#include "Os/AutoCriticalSection.h"
#include "Os/File.h"

#include "Exception/BadArgument.h"
#include "Exception/FileNotFound.h"
#include "Exception/Unexpected.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

namespace
{
	//
	//	VARIABLE local
	//	_$$::_cSubDir -- 圧縮する場合に付加するサブディレクトリ名
	//
	ModUnicodeString _cSubDir("Area");

	//
	//	FUNCTION local
	//	_$$::_isValid -- エリアが有効かどうか
	//
	bool _isValid(const Common::ObjectIDData& cAreaID_)
	{
		if (cAreaID_.getValue() == 0 ||
			cAreaID_.getFormerValue()
			== PhysicalFile::ConstValue::UndefinedPageID)
			return false;
		return true;
	}
	
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapFile::CompressedBitmapFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const Bitmap::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
CompressedBitmapFile::CompressedBitmapFile(const FileID& cFileID_)
	: BitmapFile(cFileID_), m_pDirectFile(0)
{
	// DirectAreaFileをattachする
	attachDirectAreaFile(cFileID_);
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapFile::~CompressedBitmapFile -- デストラクタ
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
CompressedBitmapFile::~CompressedBitmapFile()
{
	// DirectAreaFileをdetachする
	detachDirectAreaFile();
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapFile::copy -- コピーする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Bitmap::BitmapFile*
//		コピーしたファイル
//
//	EXCEPTIONS
//
BitmapFile*
CompressedBitmapFile::copy()
{
	return new CompressedBitmapFile(m_cFileID);
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapFile::create -- ファイルを作成する
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
CompressedBitmapFile::create()
{
	// 下位から
	BitmapFile::create();

	try
	{
		// 物理ファイルを作成を依頼する
		m_pDirectFile->create(getTransaction());
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// 下位を消す
		BitmapFile::destroy();
		BitmapFile::rmdir();
		
		// ディレクトリを破棄する
		//
		//【注意】	ディレクトリは
		//			実体である物理ファイルの生成時に
		//			必要に応じて生成されるが、
		//			エラー時には削除されないので、
		//			この関数で削除する必要がある

		rmdir();
		_SYDNEY_RETHROW;
	}
}

//
//	FUCNTION public
//	Bitmap::CompressedBitmapFile::destroy -- ファイルを削除する
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
CompressedBitmapFile::destroy(const Trans::Transaction& cTransaction_)
{
	// まず下位から
	BitmapFile::destroy(cTransaction_);
	// ファイルを削除する
	m_pDirectFile->destroy(cTransaction_);
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapFile::mount -- マウントする
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
CompressedBitmapFile::mount(const Trans::Transaction& cTransaction_)
{
	BitmapFile::mount(cTransaction_);
	try
	{
		m_pDirectFile->mount(cTransaction_);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		BitmapFile::unmount(cTransaction_);
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapFile::unmount -- アンマウントする
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
CompressedBitmapFile::unmount(const Trans::Transaction& cTransaction_)
{
	BitmapFile::unmount(cTransaction_);
	try
	{
		m_pDirectFile->unmount(cTransaction_);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		BitmapFile::mount(cTransaction_);
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapFile::flush -- フラッシュする
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
// 	EXCEPTIONS
//
void
CompressedBitmapFile::flush(const Trans::Transaction& cTransaction_)
{
	BitmapFile::flush(cTransaction_);
	m_pDirectFile->flush(cTransaction_);
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapFile::startBackup -- バックアップを開始する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const bool bRestorable_
//		版が最新版になるように変更可能とするかどうか
//			true  : バックアップされた内容をリストアしたとき、
//			        あるタイムスタンプの表す時点に開始された
//			        版管理するトランザクションの参照する版が
//			        最新版になるように変更可能にする。
//			false : バックアップされた内容をリストアしたとき、
//			        バックアップ開始時点に障害回復可能にする。
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
CompressedBitmapFile::startBackup(const Trans::Transaction& cTransaction_,
								  const bool bRestorable_)
{
	BitmapFile::startBackup(cTransaction_, bRestorable_);
	try
	{
		m_pDirectFile->startBackup(cTransaction_, bRestorable_);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		BitmapFile::endBackup(cTransaction_);
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapFile::endBackup -- バックアップを終了する
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
CompressedBitmapFile::endBackup(const Trans::Transaction& cTransaction_)
{
	BitmapFile::endBackup(cTransaction_);
	m_pDirectFile->endBackup(cTransaction_);
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapFile::recover -- 障害から回復する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Trans::TimeStamp& cPoint_
//		回復する時点
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
CompressedBitmapFile::recover(const Trans::Transaction& cTransaction_,
							  const Trans::TimeStamp& cPoint_)
{
	BitmapFile::recover(cTransaction_, cPoint_);
	m_pDirectFile->recover(cTransaction_, cPoint_);
}

//
//	FUCNTION public
//	Bitmap::CompressedBitmapFile::restore
//		-- ある時点に開始された読み取り専用トランザクションが
//		   参照する版を最新版とする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Trans::TimeStamp& cPoint_
//		ある時点
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
CompressedBitmapFile::restore(const Trans::Transaction& cTransaction_,
							  const Trans::TimeStamp& cPoint_)
{
	BitmapFile::restore(cTransaction_, cPoint_);
	m_pDirectFile->restore(cTransaction_, cPoint_);
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapFile::startVerification -- 整合性検査を開始する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	Admin::Verification::Treatment::Value uiTreatment_
//		整合性検査で矛盾を見つけたときの処置を表す値
//	Admin::Verification::Progress& cProgress_
//		整合性検査の経過を表すクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
CompressedBitmapFile::startVerification(
	const Trans::Transaction& cTransaction_,
	Admin::Verification::Treatment::Value uiTreatment_,
	Admin::Verification::Progress& cProgress_)
{
	BitmapFile::startVerification(cTransaction_, uiTreatment_, cProgress_);
	try
	{
		m_pDirectFile->startVerification(cTransaction_, uiTreatment_,
										 cProgress_);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		try
		{
			BitmapFile::endVerification();
		}
		catch (...)
		{
			// Nothing to do. This code is for debug in Windows.
		}
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapFile::endVerification --
//		物理ファイルの整合性検査を終了する
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
CompressedBitmapFile::endVerification()
{
	m_pDirectFile->endVerification(getTransaction(), getProgress());
	BitmapFile::endVerification();
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapFile::open -- オープンする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	LogicalFile::OpenOption::OpenMode::Value eOpenMode_
//		オープンモード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
CompressedBitmapFile::open(const Trans::Transaction& cTransaction_,
						   LogicalFile::OpenOption::OpenMode::Value eOpenMode_)
{
	BitmapFile::open(cTransaction_, eOpenMode_);
	if (eOpenMode_ == LogicalFile::OpenOption::OpenMode::Batch)
		m_pDirectFile->setBatch(true);
}

//
//	FUCNTION public
//	Bitmap::CompressedBitmapFile::close -- クローズする
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
CompressedBitmapFile::close()
{
	BitmapFile::close();
	if (m_pDirectFile)
		m_pDirectFile->setBatch(false);
}

//
//	FUCNTION public
//	Bitmap::CompressedBitmapFile::sync -- 同期を取る
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	bool& incomplete
//		処理し残したかどうか
//	bool& modified
//		更新されたかどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
CompressedBitmapFile::sync(const Trans::Transaction& cTransaction_,
						   bool& incomplete, bool& modified)
{
	BitmapFile::sync(cTransaction_, incomplete, modified);
	m_pDirectFile->sync(cTransaction_, incomplete, modified);
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapFile::move -- ファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Os::Path& cPath_
//		移動先のディレクトリ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
CompressedBitmapFile::move(const Trans::Transaction& cTransaction_,
						   const Os::Path& cPath_)
{
	BitmapFile::move(cTransaction_, cPath_);

	try
	{
		if (Os::Path::compare(m_cFileID.getPath(), cPath_)
			== Os::Path::CompareResult::Unrelated)
		{
			; _TRMEISTER_ASSERT(m_pDirectFile);
		
			m_cDirectPath = cPath_;
			m_cDirectPath.addPart(_cSubDir);
		
			// ファイルが一時ファイルか調べる
			const bool temporary =
				(m_pDirectFile->getBufferingStrategy().
				 m_VersionFileInfo._category
				 == Buffer::Pool::Category::Temporary);

			// 新しいパス名を設定する
			Version::File::StorageStrategy::Path cPath;
			cPath._masterData = m_cDirectPath;
			if (!temporary) {
				cPath._versionLog = m_cDirectPath;
				cPath._syncLog = m_cDirectPath;
			}

			m_pDirectFile->move(cTransaction_, cPath);
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// 戻す
		BitmapFile::move(cTransaction_, m_cFileID.getPath());
		m_cDirectPath = m_cFileID.getPath();
		m_cDirectPath.addPart(_cSubDir);
		
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapFile::flushAllPages
//		-- デタッチされている全ページをフラッシュする
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
CompressedBitmapFile::flushAllPages()
{
	BitmapFile::flushAllPages();
	m_pDirectFile->detachAllAreas();
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapFile::recoverAllPages
//		-- デタッチされている全ページの内容を元に戻す
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
CompressedBitmapFile::recoverAllPages()
{
	BitmapFile::recoverAllPages();
	m_pDirectFile->recoverAllAreas();
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapFile::verify -- 整合性検査
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
CompressedBitmapFile::verify()
{
	Common::ObjectIDData cValue;
	
	// まずB木の整合性検査
	BtreeFile::verify();
	
	// nullのビットマップの整合性検査
	if (getHeaderPage().getNullID(cValue) == true)
	{
		ModAutoPointer<CompressedBitmapIterator> ite = getIteratorImpl(cValue);
		if (ite.get() == 0)
		{
			_SYDNEY_VERIFY_INCONSISTENT(getProgress(),
										getPath(),
										Message::IllegalBtreeValue());
		}
		else
		{
			ite->verify();
		}
	}
	if (getHeaderPage().getAllNullID(cValue) == true)
	{
		ModAutoPointer<CompressedBitmapIterator> ite = getIteratorImpl(cValue);
		if (ite.get() == 0)
		{
			_SYDNEY_VERIFY_INCONSISTENT(getProgress(),
										getPath(),
										Message::IllegalBtreeValue());
		}
		else
		{
			ite->verify();
		}
	}
	
	// 次にその他のビットマップの整合性検査
	
	Condition cond(m_cFileID);
	search(&cond);	// B木を全数検索
	
	while (get(cValue))
	{
		ModAutoPointer<CompressedBitmapIterator> ite = getIteratorImpl(cValue);
		if (ite.get() == 0)
		{
			_SYDNEY_VERIFY_INCONSISTENT(getProgress(),
										getPath(),
										Message::IllegalBtreeValue());
		}
		else
		{
			ite->verify();
		}
	}
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapFile::getIterator -- ビットマップイテレータを得る
//
//	NOTES
//	BtreeFile::search実行後に実行する必要がある
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Bitmap::BitmapIterator*
//		ビットマップイテレータ。条件にマッチする間はgetできる。
//		すべてのイテレータを返したら、0を返す
//
//	EXCEPTIONS
//
BitmapIterator*
CompressedBitmapFile::getIterator()
{
	// B木の検索結果を得る
	Common::ObjectIDData cValue;
	if (get(cValue) == false)
		return 0;

	; _TRMEISTER_ASSERT(_isValid(cValue));

	return getIteratorImpl(cValue);
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapFile::getIterator -- ビットマップイテレータを得る
//
//	NOTES
//	BtreeFile::search実行後に実行する必要がある
//
//	ARGUMENTS
//	Common::Data& cKey_
//
//	RETURN
//	Bitmap::BitmapIterator*
//		ビットマップイテレータ。条件にマッチする間はgetできる。
//		すべてのイテレータを返したら、0を返す
//
//	EXCEPTIONS
//
BitmapIterator*
CompressedBitmapFile::getIterator(Common::Data& cKey_)
{
	// B木の検索結果を得る
	Common::ObjectIDData cValue;
	if (get(cKey_, cValue) == false)
		return 0;

	; _TRMEISTER_ASSERT(_isValid(cValue));

	return getIteratorImpl(cValue);
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapFile::getIteratorForGroupBy
//		-- ビットマップイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Bitmap::BitmapIterator*
//		ビットマップイテレータ
//
//	EXCEPTIONS
//
BitmapIterator*
CompressedBitmapFile::getIteratorForGroupBy(const Common::Data& cValue_)
{
	const Common::ObjectIDData& v
		= _SYDNEY_DYNAMIC_CAST(const Common::ObjectIDData&,
							   cValue_);

	return getIteratorImpl(v);
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapFile::attachArea -- エリアを attach する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::DirectArea::ID& id_
//		エリアID
//
//	RETURN
//	PhysicalFile::DirectArea
//		エリア
//
//	EXCEPTIONS
//
PhysicalFile::DirectArea
CompressedBitmapFile::attachArea(const PhysicalFile::DirectArea::ID& id_)
{
	PhysicalFile::DirectArea area;
	
	// エリアを得る
	Admin::Verification::Progress& cProgress = getProgress();
	if (&cProgress == 0)
	{
		area = m_pDirectFile->attachArea(getTransaction(),
										 id_,
										 getFixMode());
	}
	else
	{
		// 整合性検査時
		area = m_pDirectFile->verifyArea(getTransaction(),
										 id_,
										 getFixMode(),
										 cProgress);
	}

	return area;
}

//
//	FUNCTION protected
//	Bitmap::CompressedBitmapFile::on -- ビットをONする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cKey_
//		ビットをONするキー値
//	ModUInt32 uiRowID_
//		ビットをONするROWID
//	bool isNull_
//		配列かつデータがnullの場合はtrue、それ以外の場合はfalse
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
CompressedBitmapFile::on(const Common::Data& cKey_,
						 ModUInt32 uiRowID_,
						 bool isNull_)
{
	// B木からObjectIDを得る
	Common::ObjectIDData cValue;
	get(cKey_, isNull_, cValue);

	// 更新用のイテレータを得る
	ModAutoPointer<CompressedBitmapIterator> iterator;
	try
	{
		iterator = getIteratorImpl(cValue,
								   true);
	}
	catch (...)
	{
		SydMessage << "key=" << cKey_ << " rowid=" << uiRowID_ << ModEndl;
		throw;
	}

	if (_isValid(cValue) == false)
	{
		// 初めてなので初期化
		iterator->initialize(uiRowID_);
		// AreaID -> ObjectID
		PhysicalFile::File::convertToObjectID(iterator->getArea().getID(),
											  cValue);
		// B木に挿入
		BtreeFile::insert(cKey_, cValue, isNull_);
	}
	else
	{
		// ビットをON
		CompressedBitmapIterator::Result::Value r = iterator->on(uiRowID_);
		switch (r)
		{
		case CompressedBitmapIterator::Result::Modify:
			{
				// Areaが変更された -> B木を更新する
				PhysicalFile::File::convertToObjectID(
					iterator->getArea().getID(), cValue);
				BtreeFile::update(cKey_, cValue, isNull_);
			}
			break;
		case CompressedBitmapIterator::Result::NeedConvert:
			{
				// より大きな構造への変換が必要
				ModAutoPointer<CompressedBitmapIterator> n
					= iterator->convert(uiRowID_);
				PhysicalFile::File::convertToObjectID(n->getArea().getID(),
													  cValue);
				BtreeFile::update(cKey_, cValue, isNull_);
			}
			break;
		default:
			;
		}
	}
}

//
//	FUNCTION protected
//	Bitmap::CompressedBitmapFile::off -- ビットをOFFする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cKey_
//		ビットをOFFするキー値
//	ModUInt32 uiRowID_
//		ビットをOFFするROWID
//	bool isNull_
//		配列かつデータがnullの場合はtrue、それ以外の場合はfalse
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
CompressedBitmapFile::off(const Common::Data& cKey_,
						  ModUInt32 uiRowID_,
						  bool isNull_)
{
	// B木からDIRページを得る
	Common::ObjectIDData cValue;
	if (get(cKey_, isNull_, cValue) == false)
	{
		// 以前はgetに失敗するとBadArgumentだったが、何もしないように変更した。
		// Compressed版では、非NULL要素が0件になるとB木は削除されてしまう。
		// そのため、配列型を削除する際に、配列に同じ値が複数入っていると、
		// 1個目の削除でB木が削除されてしまうので、
		// 2個目の削除の時点ではB木がないのでgetに失敗してしまう。
		return;
	}

	// 更新用のイテレータを得る
	ModAutoPointer<CompressedBitmapIterator> iterator = getIteratorImpl(cValue);
	if (iterator.get() == 0)
	{
		// 以前はgetに失敗するとBadArgumentだったが、何もしないように変更した。
		// Compressed版では、ある値が0件になるとエリアは削除されてしまう。
		// そのため、配列型を削除する際に、配列に同じ値が複数入っていると、
		// 1個目の削除でエリアが削除されてしまうので、
		// 2個目の削除の時点ではエリアがないのでgetに失敗してしまう。
		return;
	}
	
	// ビットをOFF
	CompressedBitmapIterator::Result::Value r = iterator->off(uiRowID_);
	switch (r)
	{
	case CompressedBitmapIterator::Result::Modify:
		{
			// Areaが変更された -> B木を更新する
			PhysicalFile::File::convertToObjectID(
				iterator->getArea().getID(), cValue);
			BtreeFile::update(cKey_, cValue, isNull_);
		}
		break;
	case CompressedBitmapIterator::Result::Deleted:
		{
			// 削除された -> B木からも削除する
			BtreeFile::expunge(cKey_, cValue, isNull_);
		}
		break;
	default:
		;
	}
}

//
//	FUNCTION private
//	Bitmap::CompressedBitmapFile::attachDirectAreaFile
//		-- ダイレクトエリアファイルをattachする
//
//	NOTES
//
//	ARGUMENTS
//	const Bitmap::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
CompressedBitmapFile::attachDirectAreaFile(const FileID& cFileID_)
{
	; _TRMEISTER_ASSERT(cFileID_.isCompressed());
	
	PhysicalFile::File::StorageStrategy cStorageStrategy;
	PhysicalFile::File::BufferingStrategy cBufferingStrategy;

	//
	//	物理ファイル格納戦略を設定する
	//

	// 物理ファイルの空き領域管理機能
	cStorageStrategy.m_PhysicalFileType = PhysicalFile::DirectAreaType;
	// マウントされているか
	cStorageStrategy.m_VersionFileInfo._mounted = cFileID_.isMounted();
	// 読み取り専用か
	cStorageStrategy.m_VersionFileInfo._readOnly = cFileID_.isReadOnly();
	// ページサイズ
	cStorageStrategy.m_VersionFileInfo._pageSize = cFileID_.getPageSize();

	// 圧縮する場合にはサブディレクトを追加する
	m_cDirectPath = cFileID_.getPath();
	m_cDirectPath.addPart(_cSubDir);

	// マスタデータファイルの親ディレクトリの絶対パス名
	cStorageStrategy.m_VersionFileInfo._path._masterData = m_cDirectPath;
	if (cFileID_.isTemporary() == false)
	{
		// バージョンログファイルの親ディレクトリの絶対パス名
		cStorageStrategy.m_VersionFileInfo._path._versionLog = m_cDirectPath;
		// 同期ログファイルの親ディレクトリの絶対パス名
		cStorageStrategy.m_VersionFileInfo._path._syncLog = m_cDirectPath;
	}

	// マスタデータファイルの最大ファイルサイズ(B 単位)
	cStorageStrategy.m_VersionFileInfo._sizeMax._masterData
		= PhysicalFile::ConstValue::DefaultFileMaxSize;
	// バージョンログファイルの最大ファイルサイズ(B 単位)
	cStorageStrategy.m_VersionFileInfo._sizeMax._versionLog
		= PhysicalFile::ConstValue::DefaultFileMaxSize;
	// 同期ログファイルの最大ファイルサイズ(B 単位)
	cStorageStrategy.m_VersionFileInfo._sizeMax._syncLog
		= PhysicalFile::ConstValue::DefaultFileMaxSize;

	// マスタデータファイルのエクステンションサイズ(B 単位)
	cStorageStrategy.m_VersionFileInfo._extensionSize._masterData
		= PhysicalFile::ConstValue::DefaultFileExtensionSize;
	// バージョンログファイルのエクステンションサイズ(B 単位)
	cStorageStrategy.m_VersionFileInfo._extensionSize._versionLog
		= PhysicalFile::ConstValue::DefaultFileExtensionSize;
	// 同期ログファイルのエクステンションサイズ(B 単位)
	cStorageStrategy.m_VersionFileInfo._extensionSize._syncLog
		= PhysicalFile::ConstValue::DefaultFileExtensionSize;

	
	//
	//	物理ファイルバッファリング戦略を設定する
	//
	if (cFileID_.isTemporary())
	{
		// 一時なら
		cBufferingStrategy.m_VersionFileInfo._category
			= Buffer::Pool::Category::Temporary;
	}
	else if (cFileID_.isReadOnly())
	{
		// 読み取り専用なら
		cBufferingStrategy.m_VersionFileInfo._category
			= Buffer::Pool::Category::ReadOnly;
	}
	else
	{
		// その他
		cBufferingStrategy.m_VersionFileInfo._category
			= Buffer::Pool::Category::Normal;
	}


	// 物理ファイルをアタッチする
	m_pDirectFile = PhysicalFile::Manager::attachFile(cStorageStrategy,
													  cBufferingStrategy,
													  cFileID_.getLockName());
}

//
//	FUNCTION private
//	Bitmap::CompressedBitmapFile::rmdir -- ディレクトリを削除する
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
CompressedBitmapFile::rmdir()
{
	try
	{
		Os::Directory::remove(m_cDirectPath);
	}
	catch (Exception::FileNotFound&)
	{
		// 無視
	}
}

//
//	FUNCTION private
//	Bitmap::CompressedBitmapFile::detachDirectAreaFile
//		-- DirectAreaFileをdetachする
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
CompressedBitmapFile::detachDirectAreaFile()
{
	if (m_pDirectFile)
	{
		PhysicalFile::Manager::detachFile(m_pDirectFile);
		m_pDirectFile = 0;
	}
}

//
//	FUNCTION private
//	Bitmap::CompressedBitmapFile::getIteratorImpl
//		-- イテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::ObjectIDData& cObjectID_
//		オブジェクトID
//	bool bInsert_
//		挿入かどうか
//
//	RETURN
//	CompressedBitmapIterator*
//		イテレータ
//
//	EXCEPTIONS
//
CompressedBitmapIterator*
CompressedBitmapFile::getIteratorImpl(const Common::ObjectIDData& cObjectID_,
									  bool bInsert_)
{
	if (_isValid(cObjectID_) == false)
	{
		if (bInsert_)
			// 初めての登録なのでショート
			return new ShortBitmapIterator(*this);
		else
			return 0;
	}

	PhysicalFile::DirectArea cArea;
	CompressedBitmapIterator* i = 0;
	
	switch (getIteratorType(cObjectID_, cArea))
	{
	case CompressedBitmapIterator::Type::Short:
		i = new ShortBitmapIterator(*this);
		break;
	case CompressedBitmapIterator::Type::Middle:
		i = new MiddleBitmapIterator(*this);
		break;
	case CompressedBitmapIterator::Type::Long:
		i = new LongBitmapIterator(*this);
		break;
	default:
		SydMessage << cObjectID_ << ModEndl;
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	i->setArea(cArea);
	return i;
}

//
//	FUNCTION private
//	Bitmap::CompressedBitmapFile::getIteratorType
//		-- イテレータ種別を求める
//
//	NOTES
//
//	ARGUMENTS
//	const Common::ObjectIDData& cObjectID_
//		オブジェクトID
//	PhysicalFile::DirectArea& cArea_
//		得られたエリア
//
//	RETURN
//	Bitmap::CompressedBitmapIterator::Type::Value
//		イテレータの種別
//
//	EXCEPTIONS
//
CompressedBitmapIterator::Type::Value
CompressedBitmapFile::getIteratorType(const Common::ObjectIDData& cObjectID_,
									  PhysicalFile::DirectArea& cArea_)
{
	if (_isValid(cObjectID_) == false)
	{
		// まだ登録されていないので、Short
		return CompressedBitmapIterator::Type::Short;
	}

	// ObjectIDData -> DirectArea::IDへの変換
	PhysicalFile::DirectArea::ID cId;
	PhysicalFile::File::convertToDirectAreaID(cObjectID_, cId);

	// エリアを得る
	Admin::Verification::Progress& cProgress = getProgress();
	if (&cProgress == 0)
	{
		cArea_ = m_pDirectFile->attachArea(getTransaction(),
										   cId,
										   getFixMode());
	}
	else
	{
		// 整合性検査時
		cArea_ = m_pDirectFile->verifyArea(getTransaction(),
										   cId,
										   getFixMode(),
										   cProgress);
	}

	// 種別を得る
	return CompressedBitmapIterator::getType(cArea_);
}

//
//	Copyright (c) 2007, 2008, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
