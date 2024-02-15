// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SimpleFile.cpp -- キーがnot nullなもの
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2011, 2012, 2014, 2015, 2018, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Btree2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "Btree2/SimpleFile.h"
#include "Btree2/MessageAll_Class.h"

#include "FileCommon/FileOption.h"

#include "Os/Memory.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "Schema/File.h"

#include "Exception/BadArgument.h"
#include "Exception/IntegrityViolation.h"
#include "Exception/NullabilityViolation.h"
#include "Exception/Unexpected.h"
#include "Exception/VerifyAborted.h"
#include "Exception/Unexpected.h"

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_BTREE2_USING

namespace
{
	// 削除したエントリを格納するB木用のサブディレクトリ
	
	Os::Path	_cSubPath("Deleted");
}

//
//	FUNCTION public
//	Btree2::SimpleFile::GetByBitSet::parallel
//		-- マルチスレッドで実行するメソッド
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
SimpleFile::GetByBitSet::parallel()
{
	Common::BitSet cBitSet;
	Condition* pCondition = 0;
	bool isNewCondition = false;

	while (true)
	{
		SimplePage::PagePointer pLeafPage
			= m_cFile.nextLeafPage(pCondition, isNewCondition);
		if (pLeafPage == 0)
			// 終了
			break;

		Condition::LimitCond& cLower = pCondition->getLowerCondition();
		SimplePage::Iterator i;

		if (isNewCondition && cLower.m_pBuffer)
		{
			// ページ内を検索する
			if (cLower.m_eType == LogicalFile::TreeNodeInterface::GreaterThan)
			{
				i = pLeafPage->upperBound(cLower.m_pBuffer,
										  cLower.m_cCompare);
			}
			else
			{
				i = pLeafPage->lowerBound(cLower.m_pBuffer,
										  cLower.m_cCompare);
			}
		}
		else
		{
			// ページの先頭
			i = pLeafPage->begin();
		}

		Condition::LimitCond& cUpper = pCondition->getUpperCondition();
		
		while (i != pLeafPage->end())
		{
			if (cUpper.m_pBuffer && cUpper.m_cCompare(*i, cUpper.m_pBuffer) > 0)
			{
				// 上限に達した
				break;
			}

			if (pCondition->isOtherConditionMatch(*i) == true)
			{
				// すべての条件を満たした
				m_cFile.getLeafData().getBitSet(cBitSet, *i, m_iFieldID);
			}

			if (cLower.m_cCompare.isUnique())
				// 1件しかヒットしないユニーク条件なので、終了
				break;

			// 次へ
			++i;
		}
	}

	{
		// 結果をマージする
		Os::AutoCriticalSection cAuto(m_cLatch);
		m_cBitSet |= cBitSet;
	}
}

//
//	FUNCTION public
//	Btree2::SimpleFile::SimpleFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const Btree2::FileID& cFileID_
//		ファイルID
//	const Os::Path& cSubPath_
//		サブディレクトリ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SimpleFile::SimpleFile(const FileID& cFileID_, const Os::Path& cSubPath_)
	: BtreeFile(cFileID_, cSubPath_), m_pCondition(0),
	  m_uiSearchPageID(PhysicalFile::ConstValue::UndefinedPageID),
	  m_iSearchEntryPosition(-1),
	  m_uiMarkPageID(PhysicalFile::ConstValue::UndefinedPageID),
	  m_iMarkEntryPosition(-1),
	  m_bReverse(false), m_bConstraintLock(false),
	  m_pExpungeFile(0), m_vecpCondition(0),
	  m_uiNextPageID(PhysicalFile::ConstValue::UndefinedPageID)
{
	if (cFileID_.isUnique() && cSubPath_.getLength() == 0)
	{
		m_pExpungeFile = new SimpleFile(cFileID_, _cSubPath);
	}
}

//
//	FUNCTION public
//	Btree2::SimpleFile::~SimpleFile -- デストラクタ
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
SimpleFile::~SimpleFile()
{
	if (m_pExpungeFile)
		delete m_pExpungeFile, m_pExpungeFile = 0;
}

//
//	FUNCTION public
//	Btree2::SimpleFile::getSize -- ファイルサイズを得る
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
SimpleFile::getSize() const
{
	ModUInt64 size = BtreeFile::getSize();
	if (m_pExpungeFile)
		size += m_pExpungeFile->getSize();
	return size;
}

//
//	FUNCTION public
//	Btree2::SimpleFile::destroy -- ファイルを破棄する
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
SimpleFile::destroy(const Trans::Transaction& cTransaction_)
{
	BtreeFile::destroy(cTransaction_);
	if (m_bSubFile)
	{
		// ディレクトリも削除する
		rmdir();
	}
	
	if (m_pExpungeFile)
		m_pExpungeFile->destroy(cTransaction_);
}

//
//	FUNCTION public
//	Btree2::SimpleFile::mount -- ファイルをマウントする
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
SimpleFile::mount(const Trans::Transaction& cTransaction_)
{
	BtreeFile::mount(cTransaction_);
	try
	{
		if (m_pExpungeFile)
			m_pExpungeFile->mount(cTransaction_);
	}
	catch (...)
	{
		// エラーになったのでunmountする
		BtreeFile::unmount(cTransaction_);
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Btree2::SimpleFile::unmount -- ファイルをアンマウントする
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
SimpleFile::unmount(const Trans::Transaction& cTransaction_)
{
	BtreeFile::unmount(cTransaction_);
	try
	{
		if (m_pExpungeFile)
			m_pExpungeFile->unmount(cTransaction_);
	}
	catch (...)
	{
		// エラーになったのでmountする
		BtreeFile::mount(cTransaction_);
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Btree2::SimpleFile::flush -- ファイルをフラッシュする
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
SimpleFile::flush(const Trans::Transaction& cTransaction_)
{
	BtreeFile::flush(cTransaction_);
	if (m_pExpungeFile && m_pExpungeFile->isMounted(cTransaction_))
		m_pExpungeFile->flush(cTransaction_);
}

//
//	FUNCTION public
//	Btree2::SimpleFile::startBackup -- ファイルのバックアップを開始する
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
SimpleFile::startBackup(const Trans::Transaction& cTransaction_,
						const bool bRestorable_)
{
	BtreeFile::startBackup(cTransaction_, bRestorable_);
	try
	{
		if (m_pExpungeFile)
			m_pExpungeFile->startBackup(cTransaction_, bRestorable_);
	}
	catch (...)
	{
		// エラーになったのでendBackupする
		BtreeFile::endBackup(cTransaction_);
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Btree2::SimpleFile::endBackup -- ファイルのバックアップを終了する
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
SimpleFile::endBackup(const Trans::Transaction& cTransaction_)
{
	BtreeFile::endBackup(cTransaction_);
	if (m_pExpungeFile)
		m_pExpungeFile->endBackup(cTransaction_);
}

//
//	FUNCTION public
//	Btree2::SimpleFile::recover -- ファイルを障害から回復する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Trans::TimeStamp& cPoint_
//		バージョンファイルを戻す時点のタイムスタンプ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SimpleFile::recover(const Trans::Transaction& cTransaction_,
					const Trans::TimeStamp& cPoint_)
{
	BtreeFile::recover(cTransaction_, cPoint_);
	if (m_bSubFile)
	{
		if (!isAccessible())
		{
			// リカバリの結果、実体である OS ファイルが存在しなく
			// なったので、ディレクトリを削除する

			rmdir();
		}
	}
	
	if (m_pExpungeFile && m_pExpungeFile->isMounted(cTransaction_))
		m_pExpungeFile->recover(cTransaction_, cPoint_);
}

//
//	FUNCTION public
//	Btree2::SimpleFile::restore
//		-- ある時点に開始された読み取り専用トランザクションが
//		   参照する版を最新版とする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Trans::TimeStamp& cPoint_
//		このタイムスタンプの表す時点に開始された
//		読取専用トランザクションの参照する版が
//		最新版になるようにバージョンファイルを変更する
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SimpleFile::restore(const Trans::Transaction& cTransaction_,
					const Trans::TimeStamp& cPoint_)
{
	BtreeFile::restore(cTransaction_, cPoint_);
	if (m_pExpungeFile && m_pExpungeFile->isMounted(cTransaction_))
		m_pExpungeFile->restore(cTransaction_, cPoint_);
}

//
//	FUNCTION public
//	Btree2::SimpleFile::open -- オープンする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	LogicalFile::OpenOption::OpenMode::Value eOpenMode_
///		オープンモード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SimpleFile::open(const Trans::Transaction& cTransaction_,
				 LogicalFile::OpenOption::OpenMode::Value eOpenMode_)
{
	BtreeFile::open(cTransaction_, eOpenMode_);
	if (m_pExpungeFile)
		m_pExpungeFile->open(cTransaction_, eOpenMode_);
}

//
//	FUNCTION public
//	Btree2::SimpleFile::close -- クローズする
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
SimpleFile::close()
{
	if (m_pExpungeFile) m_pExpungeFile->close();
	
	// 検索中ページをdetachする
	m_pSearchPage = 0;
	
	if (m_uiSearchPageID != PhysicalFile::ConstValue::UndefinedPageID)
		eraseLock(m_uiSearchPageID);
	m_uiSearchPageID = PhysicalFile::ConstValue::UndefinedPageID;
	BtreeFile::close();
}

//
//	FUNCTION public
//	Btree2::SimpleFile::sync -- 同期をとる
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	bool& incomplete
//		完了したか
//	bool& modified
//		更新したか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SimpleFile::sync(const Trans::Transaction& cTransaction_,
				 bool& incomplete, bool& modified)
{
	BtreeFile::sync(cTransaction_, incomplete, modified);
	if (m_pExpungeFile && m_pExpungeFile->isMounted(cTransaction_))
		m_pExpungeFile->sync(cTransaction_, incomplete, modified);
}

//
//	FUNCTION public
//	Btree2::SimpleFile::compact -- 不要なデータを削除する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	bool& incomplete
//	   	不完全かどうか
//	bool& modified
//		更新したか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SimpleFile::compact(const Trans::Transaction& cTransaction_,
					bool& incomplete, bool& modified)
{
	if (m_bSubFile)
	{
		// 削除ファイルの場合、可能ならファイルをクリアする

		Trans::Transaction& trans
			= const_cast<Trans::Transaction&>(cTransaction_);
		expungeConstraintLockEntry(trans,
								   m_cFileID.getLockName(),
								   modified);
	}
	
	if (m_pExpungeFile && m_pExpungeFile->isMounted(cTransaction_))
		m_pExpungeFile->compact(cTransaction_, incomplete, modified);
}

//
//	FUNCTION public
//	Btree2::SimpleFile::move -- ファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Os::Path& cFilePath_
//		移動先のパス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SimpleFile::move(const Trans::Transaction& cTransaction_,
				 const Os::Path& cFilePath_)
{
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかく移動する
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	int step = 0;
	Os::Path cOrgPath = getPath();
	
	try
	{
		BtreeFile::move(cTransaction_, cFilePath_);
		step++;

		if (m_pExpungeFile)
		{
			Os::Path path = cFilePath_;
			path.addPart(_cSubPath);
			m_pExpungeFile->move(cTransaction_, path);
		}
	}
	catch (...)
	{
		try
		{
			if (step == 1)
			{
				BtreeFile::move(cTransaction_, cOrgPath);
			}
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(m_cFileID.getLockName(), false);
		}

		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Btree2::SimpleFile::search -- 検索する
//
//	NOTES
//
//	ARGUMENTS
//	Btree2::Condition* pCondition_
//		検索条件
//	bool bReverse_
//		逆順にとりだすかどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SimpleFile::search(Condition* pCondition_, bool bReverse_)
{
	m_pCondition = pCondition_;
	m_bReverse = bReverse_;
	m_bConstraintLock = false;

	// 検索前準備を行う
	preSearch();

	if (pCondition_->isConstraintLock())
	{
		// 制約ロックのための検索なので、削除済みファイルも検索する
		
		if (m_pExpungeFile && m_pExpungeFile->isMounted(getTransaction()))
		{
			m_pExpungeFile->search(pCondition_, bReverse_);
			m_bConstraintLock = true;
		}
	}
}

//
//	FUNCTION public
//	Btree2::SimpleFile::get -- 取得する
//
//	NOTES
//
//	ARGUMENTS
//	unsigned char ucBitSet_
//		取得するフィールド番号
//	Common::DataArrayData& cTuple_
//		取得した結果
//	unsigned int& uiTupleID_
//		タプルID
//
//	RETURN
//	bool
//		結果が得られた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
SimpleFile::get(unsigned char ucBitSet_,
				Common::DataArrayData& cTuple_,
				unsigned int& uiTupleID_)
{
	if (m_bReverse == false)
	{
		next();
	}
	else
	{
		nextReverse();
	}
	if (m_uiSearchPageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		getLeafData().getData(m_pSearchEntryBuffer,
							  ucBitSet_,
							  cTuple_,
							  uiTupleID_);
		return true;
	}

	bool result = false;

	if (m_bConstraintLock)
	{
		// 制約ロックのための検索なので、
		// ここでヒットしなくても、削除エントリから取得する

		result = m_pExpungeFile->get(ucBitSet_, cTuple_, uiTupleID_);
	}
	
	return result;
}

//
//	FUNCTION public
//	Btree2::SimpleFile::getByBitSet -- ビットセットで得る
//
//	NOTES
//
//	ARGUMENTS
//	ModVector<Condition*>& vecpCondition_
//		検索条件
//	int iFieldID_
//		フィールド番号
//	Common::BitSet& cBitSet_
//		設定するビットセット
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SimpleFile::getByBitSet(ModVector<Condition*>& vecpCondition_,
						int iFieldID_, Common::BitSet& cBitSet_)
{
	// メンバー変数を初期化する
	m_vecpCondition = &vecpCondition_;
	m_iteCondition = m_vecpCondition->begin();
	m_uiNextPageID = PhysicalFile::ConstValue::UndefinedPageID;

	// OpenMPで並列処理する
	GetByBitSet cGetByBitSet(*this, iFieldID_, cBitSet_);
	cGetByBitSet.run();

	if (m_bConstraintLock)
	{
		// 制約ロックのための検索なので、削除エントリからも取得する

		m_pExpungeFile->getByBitSet(vecpCondition_, iFieldID_, cBitSet_);
	}
}

//
//	FUNCTION public
//	Btree2::SimpleFile::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cValue_
//		挿入するデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SimpleFile::insert(const Common::DataArrayData& cValue_)
{
	//
	//	入力チェック
	//

	const ModVector<ModSize>& vecMaxSize = m_cFileID.getFieldSize();

	// 要素数チェック
	if (static_cast<ModSize>(cValue_.getCount()) != vecMaxSize.getSize())
	{
		// フィールド数があっていない
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// サイズチェック
	const Data& cData = getLeafData();
	ModSize size = 0;
	for (int i = 0; i != cValue_.getCount(); ++i)
	{
		ModSize s = cData.getSize(*(cValue_.getElement(i)), i);
		if (vecMaxSize[i] < s * sizeof(ModUInt32))
			// 通常ありえない
			_SYDNEY_THROW0(Exception::BadArgument);
		if (s == 0)	{ // nullはだめ
			ModUnicodeOstrStream stream;
			stream << "field(" << i << ")";
			_SYDNEY_THROW1(Exception::NullabilityViolation, stream.getString());
		}
		size += s;
	}

	// 挿入データ作成
	AutoPointer<ModUInt32> p = makeData(cValue_, cData, size);

	// リーフページを得る
	SimplePage::PagePointer pLeafPage = getLeafPage(p, getCompare());
	if (pLeafPage == 0)
	{
		// ルートページがないので新たに確保する
		pLeafPage = allocatePage(PhysicalFile::ConstValue::UndefinedPageID,
								 PhysicalFile::ConstValue::UndefinedPageID,
								 SimplePage::PagePointer());
		pLeafPage->setLeaf();
		
		// ヘッダーページに設定する
		HeaderPage::PagePointer pHeaderPage = getHeaderPage();
		pHeaderPage->setRootPageID(pLeafPage->getID());
		pHeaderPage->setLeftLeafPageID(pLeafPage->getID());
		pHeaderPage->setRightLeafPageID(pLeafPage->getID());
		pHeaderPage->incrementStepCount();
	}
	
	// 挿入する
	pLeafPage->insertEntry(p, size);
	// エントリ数を増やす
	getHeaderPage()->incrementCount();

	if (m_pExpungeFile && m_pExpungeFile->isMounted(getTransaction()))
	{
		// 削除ファイルがあるので、そこから同じキーを削除する
		//
		//【注意】
		// 引数の cValue_ には、挿入するデータのROWIDが入っていて、
		// 削除するもののROWIDとは別であるが、
		// ユニーク制約になっているB木では、比較クラスがキー値でのみ
		// 比較しているので、そのままで問題ない

		m_pExpungeFile->expunge(cValue_);
		m_pExpungeFile->dirtyHeaderPage();
	}
}

//
//	FUNCTION public
//	Btree2::SimpleFile::expunge -- 削除する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cKey_
//		削除するデータを特定するためのキー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SimpleFile::expunge(const Common::DataArrayData& cKey_)
{
	// フィールド数のチェック
	if (static_cast<ModSize>(cKey_.getCount())
		< m_cFileID.getKeyType().getSize())
	{
		// フィールド数が足らない
		_SYDNEY_THROW0(Exception::BadArgument);
	}
		
	// 削除のためのキーデータ作成
	ModSize size = 0;
	AutoPointer<ModUInt32> p = makeData(cKey_, getLeafData(), size);

	// リーフページを得る
	SimplePage::PagePointer pLeafPage = getLeafPage(p, getCompare());
	if (pLeafPage == 0)
	{
		if (m_bSubFile)
			// サブファイルの場合、見つからなくてもエラーにしない
			return;
		
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	if (m_bSubFile)
	{
		// 削除する前に存在しているか確認する
		if (pLeafPage->exist(p) == false)
			return;
	}

	// 削除する
	pLeafPage->expungeEntry(p, size);
	// エントリ数を減らす
	getHeaderPage()->decrementCount();

	if (m_pExpungeFile)
	{
		// 削除ファイルがあるので、削除したエントリを挿入する

		if (m_pExpungeFile->isMounted(getTransaction()) == false)
		{
			// ファイルが存在しないので、作成する

			m_pExpungeFile->create();
			m_pExpungeFile->dirtyHeaderPage();
			m_pExpungeFile->flushAllPages();
		}

		m_pExpungeFile->insert(cKey_);
		m_pExpungeFile->dirtyHeaderPage();
	}
}

//
//	FUNCTION public
//	Btree2::SimpleFile::startVerification -- 整合性チェックを開始する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	Admin::Verification::Treatment::Value uiTreatment_
//		修正方法
//	Admin::Verification::Progress& cProgress_
//		経過
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SimpleFile::
startVerification(const Trans::Transaction& cTransaction_,
				  Admin::Verification::Treatment::Value uiTreatment_,
				  Admin::Verification::Progress& cProgress_)
{
	BtreeFile::startVerification(cTransaction_,
								 uiTreatment_,
								 cProgress_);
	if (m_pExpungeFile && m_pExpungeFile->isMounted(cTransaction_))
	{
		try
		{
			m_pExpungeFile->startVerification(cTransaction_,
											  uiTreatment_,
											  cProgress_);
		}
		catch (...)
		{
			BtreeFile::endVerification();
			_SYDNEY_RETHROW;
		}
	}
}

//
//	FUNCTION public
//	Btree2::SimpleFile::endVerification -- 整合性検査を終了する
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
SimpleFile::endVerification()
{
	if (m_pExpungeFile && m_pExpungeFile->isMounted(getTransaction()))
		m_pExpungeFile->endVerification();
	BtreeFile::endVerification();
}

//
//	FUNCTION public
//	Btree2::SimpleFile::verify -- 整合性チェック
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
SimpleFile::verify()
{
	// 削除ファイルから
	if (m_pExpungeFile && m_pExpungeFile->isMounted(getTransaction()))
	{
		m_pExpungeFile->verify();
	}

	SimplePage::PagePointer pRootPage = getRootPage();
	if (pRootPage != 0)
	{
		if (pRootPage->getNextPageID()
			!= PhysicalFile::ConstValue::UndefinedPageID
			|| pRootPage->getPrevPageID()
			!= PhysicalFile::ConstValue::UndefinedPageID)
		{
			_SYDNEY_VERIFY_INCONSISTENT(getProgress(),
										getPath(),
										Message::IllegalRootPageID());
			_SYDNEY_THROW0(Exception::VerifyAborted);
		}
		
		// ページをverify
		pRootPage->verify();
	}

	// 左端リーフページのチェック
	if (getHeaderPage()->getLeftLeafPageID()
		!= PhysicalFile::ConstValue::UndefinedPageID)
	{
		SimplePage::PagePointer pPage
			= attachPage(getHeaderPage()->getLeftLeafPageID(),
						 SimplePage::PagePointer());
		if (pPage->isLeaf() != true
			|| pPage->getPrevPageID()
			!= PhysicalFile::ConstValue::UndefinedPageID)
		{
			_SYDNEY_VERIFY_INCONSISTENT(getProgress(),
										getPath(),
										Message::IllegalLeftLeafPageID());
			_SYDNEY_THROW0(Exception::VerifyAborted);
		}
	}

	// 右端リーフページのチェック
	if (getHeaderPage()->getRightLeafPageID()
		!= PhysicalFile::ConstValue::UndefinedPageID)
	{
		SimplePage::PagePointer pPage
			= attachPage(getHeaderPage()->getRightLeafPageID(),
						 SimplePage::PagePointer());
		if (pPage->isLeaf() != true
			|| pPage->getNextPageID()
			!= PhysicalFile::ConstValue::UndefinedPageID)
		{
			_SYDNEY_VERIFY_INCONSISTENT(getProgress(),
										getPath(),
										Message::IllegalLeftLeafPageID());
			_SYDNEY_THROW0(Exception::VerifyAborted);
		}
	}

	// エントリ数のチェック
	if (m_uiVerifyCount != getHeaderPage()->getCount())
	{
		_SYDNEY_VERIFY_INCONSISTENT(
			getProgress(),
			getPath(),
			Message::DiscordKeyNum(getHeaderPage()->getCount(),
								   m_uiVerifyCount));
		_SYDNEY_THROW0(Exception::VerifyAborted);
	}
}

//
//	FUNCTION public
//	Btree2::SimpleFile::mark -- マークする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		マークできた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
SimpleFile::mark()
{
	if (m_iSearchEntryPosition == -1)
		return false;

	m_uiMarkPageID = m_uiSearchPageID;
	m_iMarkEntryPosition = m_iSearchEntryPosition;
	Os::Memory::copy(m_pMarkEntryBuffer, m_pSearchEntryBuffer,
					 FileID::MAX_SIZE * sizeof(ModUInt32));

	return true;
}

//
//	FUNCTION public
//	Btree2::SimpleFile::rewind -- リワインドする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		リワインドできた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
SimpleFile::rewind()
{
	if (m_iMarkEntryPosition == -1)
		return false;

	// ロック情報を削除する -> 次のget()で検索が実行される
	eraseLock(m_uiSearchPageID);

	m_pSearchPage = 0;
	m_uiSearchPageID = m_uiMarkPageID;
	m_iSearchEntryPosition = m_iMarkEntryPosition;
	Os::Memory::copy(m_pSearchEntryBuffer, m_pMarkEntryBuffer,
					 FileID::MAX_SIZE * sizeof(ModUInt32));

	return true;
}

//
//	FUNCTION public
//	Btree2::SimpleFile::flushAllPages -- 全ページをフラッシュする
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
SimpleFile::flushAllPages()
{
	// 検索中ページをdetachする
	m_pSearchPage = 0;
	// 全ページをフラッシュする
	BtreeFile::flushAllPages();

	// 削除ファイルもフラッシュする
	if (m_pExpungeFile)	m_pExpungeFile->flushAllPages();
}

//
//	FUNCTION public
//	Btree2::SimpleFile::recoverAllPages -- 全ページを元に戻す
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
SimpleFile::recoverAllPages()
{
	if (m_pSearchPage && (getTransaction().isNoVersion() || doVerify()))
	{
		// 検索中ページをdetachする
		m_pSearchPage = 0;
	}
	// 全ページを元に戻す
	BtreeFile::recoverAllPages();
	
	// 削除ファイルも元に戻す
	if (m_pExpungeFile)	m_pExpungeFile->recoverAllPages();
}

//
//	FUNCTION public
//	Btree2::SimpleFile::attachPage -- ページを得る
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		ページID
//	const Btree2::SimplePage::PagePointer& pParent_
//		親ページ
//	Buffer::Page::FixMode::Value eFixMode_
//		FIXモード(default Buffer::Page::FixMode::Unknown)
//
//	RETURN
//	Btree2::SimplePage::PagePointer
//		ページ
//
//	EXCEPTIONS
//
SimplePage::PagePointer
SimpleFile::attachPage(PhysicalFile::PageID uiPageID_,
					   const SimplePage::PagePointer& pParent_,
					   Buffer::Page::FixMode::Value eFixMode_)
{
	if (uiPageID_ == PhysicalFile::ConstValue::UndefinedPageID)
		return SimplePage::PagePointer();
	
	SimplePage::PagePointer pPage
		= _SYDNEY_DYNAMIC_CAST(SimplePage*, File::findMap(uiPageID_));
	if (pPage == 0)
	{
		pPage = _SYDNEY_DYNAMIC_CAST(SimplePage*, popPage());
		if (pPage == 0)
		{
			pPage = new SimplePage(*this);
		}
		// 無かったので、新たに確保する
		PhysicalFile::Page* pPhysicalPage = attachPhysicalPage(uiPageID_,
															   eFixMode_);
		pPage->setPhysicalPage(pPhysicalPage);
		File::attachPage(pPage);
	}

	// 親を設定する
	pPage->setParentPage(pParent_);

	return pPage;
}

//
//	FUNCTION public
//	Btree2::SimpleFile::allocatePage -- 新しいページを確保する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPrevPageID_
//		前方のページID
//	PhysicalFile::PageID uiNextPageID_
//		後方のページID
//	const Btree2::SimplePage::PagePointer& pParent_
//		親ページ
//
//	RETURN
//	Btree2::SimplePage::PagePointer
//		新しく確保したページ
//
//	EXCEPTIONS
//
SimplePage::PagePointer
SimpleFile::allocatePage(PhysicalFile::PageID uiPrevPageID_,
						 PhysicalFile::PageID uiNextPageID_,
						 const SimplePage::PagePointer& pParent_)
{
	PhysicalFile::Page* p = File::getFreePage();
	if (p == 0)
	{
		// 新たに確保する
		p = File::allocatePage();
	}

	SimplePage::PagePointer pPage
		= _SYDNEY_DYNAMIC_CAST(SimplePage*, popPage());
	if (pPage == 0)
		pPage = new SimplePage(*this);
	pPage->setPhysicalPage(p, uiPrevPageID_, uiNextPageID_);
	File::attachPage(pPage);

	// 親を設定する
	pPage->setParentPage(pParent_);

	return pPage;
}

//
//	FUNCTION public
//	Btree2::SimpleFile::findPage -- 親ページを検索する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pValue_
//		キーデータ
//	PhysicalFile::PageID uiChildPageID_
//		親を探している子ページ
//
//	RETURN
//	Btree2::SimplePage::PagePointer
//		親ページ
//
//	EXCEPTIONS
//
SimplePage::PagePointer
SimpleFile::findPage(const ModUInt32* pValue_,
					 PhysicalFile::PageID uiChildPageID_)
{
	SimplePage::PagePointer pPage;
	PhysicalFile::PageID uiPageID = getHeaderPage()->getRootPageID();
	
	do
	{
		if (uiPageID == PhysicalFile::ConstValue::UndefinedPageID)
		{
			_SYDNEY_THROW0(Exception::BadArgument);
		}

		pPage = attachPage(uiPageID, pPage);
		
		if (pPage->isLeaf() == true)
		{
			_SYDNEY_THROW0(Exception::BadArgument);
		}
		
		SimplePage::Iterator i = pPage->upperBound(pValue_, getCompare());
		if (i != pPage->begin())
			--i;
		uiPageID = getNodeData().getPageID(*i);
	}
	while (uiPageID != uiChildPageID_);

	return pPage;
}

//
//	FUNCTION public
//	Btree2::SimpleFile::nextLeafPage
//		-- getByBitSetで次に処理するリーフページと検索条件を得る
//
//	NOTES
//
//	ARGUMENTS
//	Btree2::Condition*& pCondition_
//		検索条件
//	bool& isNewCondition_
//		新しい検索条件か否か
//
//	RETURN
//	Btree2::SimplePage::PagePointer
//		リーフページ
//
//	EXCEPTIONS
//
SimplePage::PagePointer
SimpleFile::nextLeafPage(Condition*& pCondition_,
						 bool& isNewCondition_)
{
	Os::AutoCriticalSection cAuto(getLatch());

	SimplePage::PagePointer pLeafPage;
	if (m_iteCondition == m_vecpCondition->end())
		// 終了
		return pLeafPage;
	
	if (m_uiNextPageID == PhysicalFile::ConstValue::UndefinedPageID)
	{
		while (true)
		{
			isNewCondition_ = true;
			pCondition_ = *m_iteCondition;

			pLeafPage = searchPage(pCondition_, false);
			if (pLeafPage != 0)
				// ヒットした
				break;

			// ヒットしなかったので、次の条件
			++m_iteCondition;
			
			if (m_iteCondition == m_vecpCondition->end())
				// もう条件がないので、終了
				return pLeafPage;
		}
	}
	else
	{
		isNewCondition_ = false;
		pCondition_ = *m_iteCondition;
		
		pLeafPage = attachPage(m_uiNextPageID,
							   SimplePage::PagePointer());
	}

	if (pCondition_->getLowerCondition().m_cCompare.isUnique())
	{
		// ユニーク条件なので、次の条件に進める
		++m_iteCondition;
		m_uiNextPageID = PhysicalFile::ConstValue::UndefinedPageID;
	}
	else
	{
		Condition::LimitCond& cUpper = pCondition_->getUpperCondition();
		SimplePage::Iterator i = pLeafPage->end();
		--i;
		if (cUpper.m_pBuffer && cUpper.m_cCompare(*i, cUpper.m_pBuffer) > 0)
		{
			// 超えているので、次の条件に進める
			++m_iteCondition;
			m_uiNextPageID = PhysicalFile::ConstValue::UndefinedPageID;
		}
		else
		{
			// まだ超えていないので、つぎのページへ
			m_uiNextPageID = pLeafPage->getNextPageID();
			if (m_uiNextPageID == PhysicalFile::ConstValue::UndefinedPageID)
			{
				// 次がないので、次の条件に進める
				++m_iteCondition;
			}
		}
	}

	return pLeafPage;
}

//
//	FUNCTION protected
//	Btree2::SimpleFile::getEstimateCountForSearch
//		-- 検索での結果件数の見積もり
//
//	NOTES
//
//	ARUGMENTS
//	Btree2::Condition* pCondition_
//		検索条件
//	ModUInt32 count_
//		登録件数
//
//	RETURN
//	ModUInt32
//		見積もった結果件数
//
//	EXCEPTIONS
//
ModUInt32
SimpleFile::getEstimateCountForSearch(Condition* pCondition_, ModUInt32 count_)
{
	using namespace LogicalFile;
	
	//
	//	【注意】
	//	登録件数が各ページに均一に分布していると仮定している。
	//
	
	// 検索での見積もりは、下限と上限のエントリをチェックし、
	// エントリが異なっているところで、見積もる。
		
	if (pCondition_->isValid() == false)
		// 検索結果が0件の条件
		return 0;

	// 上限と下限の条件を得る
	Condition::LimitCond& cLower = pCondition_->getLowerCondition();
	Condition::LimitCond& cUpper = pCondition_->getUpperCondition();

	// 条件がequalでユニークなら1件
	if (cLower.m_eType == TreeNodeInterface::Equals &&
		cLower.m_cCompare.isUnique())
		return 1;

	// Simpleファイルの場合、nullのデータは挿入されていないので、
	// is null が条件に入っていたら0件
	if (cLower.m_nullBitmap != 0)
		return 0;

	// 上限も下限も検索条件がなければ全件ヒット
	if (cLower.m_pBuffer == 0 && cUpper.m_pBuffer == 0)
		return count_;

	// ルートページを得る
	PhysicalFile::PageID uiPageID = getHeaderPage()->getRootPageID();
	
	; _SYDNEY_ASSERT(uiPageID != PhysicalFile::ConstValue::UndefinedPageID);
	SimplePage::PagePointer pPage = attachPage(uiPageID,
											   SimplePage::PagePointer(),
											   Buffer::Page::FixMode::ReadOnly);
	SimplePage::Iterator l;
	SimplePage::Iterator u;
		
	// ページ内のヒット件数を得る
	ModSize n = getHitCount(pPage, pCondition_, l, u);

	while (true)
	{
		if (pPage->isLeaf())
		{
			// リーフなので、件数そのまま
			count_ = n;
			break;
		}
		else if (l == u)
		{
			if (pPage->getCount() == 0)
				// ありえない
				_SYDNEY_THROW0(Exception::BadArgument);
			
			// この段では違いがないので、1つ下の段を調べる
			count_ /= pPage->getCount();
			uiPageID = getNodeData().getPageID(*u);

			; _SYDNEY_ASSERT(uiPageID !=
							 PhysicalFile::ConstValue::UndefinedPageID);
			pPage = attachPage(uiPageID,
							   SimplePage::PagePointer(),
							   Buffer::Page::FixMode::ReadOnly);
			
			// ページ内のヒット件数を得る
			n = getHitCount(pPage, pCondition_, l, u);
		}
		else if ((u - l) == 1)
		{
			if (pPage->getCount() == 0)
				// ありえない
				_SYDNEY_THROW0(Exception::BadArgument);
			
			count_ /= pPage->getCount();
			
			// 隣り合うページがヒットしたので、
			// 下位ページをそれぞれ検索する

			PhysicalFile::PageID pid;
			
			// 下限を検索
			pid = getNodeData().getPageID(*l);
			SimplePage::PagePointer p0
				= attachPage(pid,
							 SimplePage::PagePointer(),
							 Buffer::Page::FixMode::ReadOnly);
			SimplePage::Iterator l0;
			SimplePage::Iterator u0;
			ModSize n0 = getHitCount(p0, pCondition_, l0, u0, true);

			// 上限を検索
			pid = getNodeData().getPageID(*u);
			SimplePage::PagePointer p1
				= attachPage(pid,
							 SimplePage::PagePointer(),
							 Buffer::Page::FixMode::ReadOnly);
			SimplePage::Iterator l1;
			SimplePage::Iterator u1;
			ModSize n1 = getHitCount(p1, pCondition_, l1, u1);

			n = n0 + n1;
			
			if (n0 != 0 && n1 == 0)
			{
				pPage = p0;
				l = l0;
				u = u0;
			}
			else if (n0 == 0)
			{
				// 下限側の戻り値が 0 の場合は、iterator は不正値なので、
				// 上限側を利用する
				
				pPage = p1;
				l = l1;
				u = u1;
			}
			else
			{
				// 両方ヒットしたのでここで見積もる
				
				if (p0->isLeaf())
				{
					count_ = n;
				}
				else
				{
					if (p0->getCount() == 0)
						// ありえない
						_SYDNEY_THROW0(Exception::BadArgument);
			
					count_ /= p0->getCount();
					
					if (n == 1)
						count_ /= 2;
					else
						count_ = ((count_ == 0) ? 1 : count_) * (n - 1);
				}
				break;
			}
		}
		else
		{
			if (pPage->getCount() == 0)
				// ありえない
				_SYDNEY_THROW0(Exception::BadArgument);
			
			// 上限と下限に違いがあるので、ヒット件数を見積もる

			count_ /= pPage->getCount();
			
			if (n == 1)
				count_ /= 2;
			else
				count_ = ((count_ == 0) ? 1 : count_) * (n - 1);
			break;
		}
	}

	return (count_ == 0) ? 1 : count_;
}

//
//	FUNCTION protected
//	Btree2::SimpleFile::getEstimateCountForFetch
//		-- フェッチでの結果件数の見積もり
//
//	NOTES
//
//	ARUGMENTS
//	Btree2::Condition* pCondition_
//		検索条件
//	ModUInt32 count_
//		登録件数
//
//	RETURN
//	ModUInt32
//		見積もった結果件数
//
//	EXCEPTIONS
//
ModUInt32
SimpleFile::getEstimateCountForFetch(Condition* pCondition_, ModUInt32 count_)
{
	//
	//	【注意】
	//	登録件数が各ページに均一に分布していると仮定している。
	//
	
	// フェッチでの見積もり
	// フェッチでの見積もりはルートノードから同じ値のエントリを数を数え、
	// 同じ値のエントリが複数あったところで、見積もる

	const ModVector<Data::Type::Value>& keyType = m_cFileID.getKeyType();
	if (static_cast<int>(keyType.getSize()) == pCondition_->getFetchField())
	{
		// キーの数と同じなので、ユニークになる
		return 1;
	}
	
	// 比較クラスを得る
	Compare cCompare;
	ModVector<Data::Type::Value> vecType;
	for (int i = 0; i < pCondition_->getFetchField(); ++i)
	{
		vecType.pushBack(keyType[i]);
	}
	cCompare.setType(vecType, false, false);

	// ルートページを得る
	PhysicalFile::PageID uiPageID = getHeaderPage()->getRootPageID();
	; _SYDNEY_ASSERT(uiPageID != PhysicalFile::ConstValue::UndefinedPageID);

	while (uiPageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// ページを得る
		SimplePage::PagePointer pPage
			= attachPage(uiPageID,
						 SimplePage::PagePointer(),
						 Buffer::Page::FixMode::ReadOnly);

		// 先頭要素から同じエントリ数をチェックしていく
		SimplePage::Iterator i = pPage->begin();
		SimplePage::Iterator j = pPage->begin();
		if (j != pPage->end())
			++j;
		
		bool same = false;
		ModUInt32 d = 1;
		for (; j != pPage->end(); ++i, ++j)
		{
			if (cCompare(*i, *j) == 0)
			{
				// 前後で同じエントリ
				same = true;
			}
			else
			{
				// 前後で異なるエントリ
				++d;
			}
		}

		if (same == true || pPage->isLeaf())
		{
			// 同じエントリがあるか、リーフなので、
			// このページの異なり数で割る
			count_ /= d;
			break;
		}
		else
		{
			if (pPage->getCount() == 0)
				// ありえない
				_SYDNEY_THROW0(Exception::BadArgument);
			
			// この段では違いがないので、1つ下の段を調べる
			count_ /= pPage->getCount();

			i = pPage->begin();
			i += pPage->getCount() / 2;	// 中間点を調べる
			uiPageID = getNodeData().getPageID(*i);
		}
	}

	return (count_ == 0) ? 1 : count_;
}

//
//	FUNCTION private
//	Btree2::SimpleFile::makeData -- データを作成する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cValue_
//		データ
//	const Btree2::Data& cData_
//		データクラス
//	ModSize& uiSize_
//		サイズ
//
//	RETURN
//	Btree2::AutoPointer<ModUInt32>
//		データ
//
//	EXCEPTIONS
//
AutoPointer<ModUInt32>
SimpleFile::makeData(const Common::DataArrayData& cValue_,
					 const Data& cData_,
					 ModSize& uiSize_)
{
	if (uiSize_ == 0)
	{
		uiSize_ = cData_.getSize(cValue_);
	}

	AutoPointer<ModUInt32> p = static_cast<ModUInt32*>(
		Os::Memory::allocate(uiSize_ * sizeof(ModUInt32)));
	cData_.dump(p, cValue_);

	return p;
}

//
//	FUNCTION private
//	Btree2::SimpleFile::getLeafPage -- リーフページを得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pBuffer_
//		検索するためのデータ
//	const Btree2::Compare& cCompare_
//		比較クラス
//	bool isLower_
//		下限を検索するかどうか。上限の場合はfalseを指定 (default true)
//
//	RETURN
//	Btree2::SimplePage::PagePointer
//		リーフページ
//
//	EXCEPTIONS
//
SimplePage::PagePointer
SimpleFile::getLeafPage(const ModUInt32* pBuffer_, const Compare& cCompare_,
						bool isLower_)
{
	SimplePage::PagePointer pLeafPage = getRootPage();
	if (pLeafPage == 0)
		return pLeafPage;

	ModSize stepCount = getHeaderPage()->getStepCount();
	ModSize c = 1;
	
	while (pLeafPage->isLeaf() == false)
	{
		PhysicalFile::PageID uiPageID;
		SimplePage::Iterator i;
		c++;
		
		if (cCompare_.isUnique() == true || isLower_ == false)
		{
			// upper_boundで検索して１つ前
			i = pLeafPage->upperBound(pBuffer_, cCompare_);
			if (i != pLeafPage->begin())
				--i;
		}
		else
		{
			// lower_boundで検索して1つ前
			i = pLeafPage->lowerBound(pBuffer_, cCompare_);
			if (i != pLeafPage->begin())
				--i;
		}

		// ページIDを得る
		uiPageID = getNodeData().getPageID(*i);
		// ページを得る
		if (getFixMode() != Buffer::Page::FixMode::ReadOnly)
		{
			if (c == stepCount)
			{
				// リーフページ
				pLeafPage = attachPage(uiPageID, pLeafPage);
			}
			else
			{
				// ノードページ
				//
				// 【注意】
				//		更新時もつねにノードページが更新されるとは限らないので、
				//		とりあえずReadOnlyでattachする
				
				pLeafPage = attachPage(uiPageID, pLeafPage,
									   Buffer::Page::FixMode::ReadOnly);
			}
		}
		else
		{
			pLeafPage = attachPage(uiPageID, SimplePage::PagePointer());
		}
	}
	
	return pLeafPage;
}

//
//	FUNCTION private
//	Btree2::SimpleFile::getRootPage -- ルートページを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Btree2::SimplePage::PagePointer
//		るーとページを得る
//
//	EXCEPTIONS
//
SimplePage::PagePointer
SimpleFile::getRootPage()
{
	SimplePage::PagePointer pPage;
	
	PhysicalFile::PageID uiPageID = getHeaderPage()->getRootPageID();
	ModSize stepCount = getHeaderPage()->getStepCount();
	if (uiPageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		Buffer::Page::FixMode::Value eFixMode
			= Buffer::Page::FixMode::Unknown;
		if (stepCount != 1)
			eFixMode = Buffer::Page::FixMode::ReadOnly;

		// ルートページを得る
		pPage = attachPage(uiPageID, SimplePage::PagePointer(), eFixMode);
	}

	return pPage;
}

//
//	FUNCTION private
//	Btree2::SimpleFile::preSearch -- 検索前準備を行う
//
//	NOTES
//	ここではデータがあるそうなリーフページの特定まで行う
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
SimpleFile::preSearch()
{
	m_pSearchPage = searchPage(m_pCondition, m_bReverse);

	if (m_uiSearchPageID != PhysicalFile::ConstValue::UndefinedPageID)
		eraseLock(m_uiSearchPageID);
	
	if (m_pSearchPage)
	{
		m_uiSearchPageID = m_pSearchPage->getID();
		insertLock(m_uiSearchPageID);
	}
	else
	{
		m_uiSearchPageID = PhysicalFile::ConstValue::UndefinedPageID;
	}
	m_iSearchEntryPosition = -1;
	m_uiMarkPageID = PhysicalFile::ConstValue::UndefinedPageID;
	m_iMarkEntryPosition = -1;
}

//
//	FUNCTION private
//	Btree2::SimpleFile::searchPage -- 検索条件を満たす先頭のリーフページを得る
//
//	NOTES
//
//	ARGUMENTS
//	Btree2::Condition* pCondition_
//		検索条件
//	bool bReverse_
//		向き
//
//	RETURN
//	Btree2::SimplePage::PagePointer
//		リーフページ
//
//	EXCEPTIONS
//
SimplePage::PagePointer
SimpleFile::searchPage(Condition* pCondition_,
					   bool bReverse_)
{
	using namespace LogicalFile;

	SimplePage::PagePointer pSearchPage;

	if (getHeaderPage()->getCount() && pCondition_->isValid())
	{
		if (bReverse_ == false)
		{
			// 順方向に取り出す
			Condition::LimitCond& cLower = pCondition_->getLowerCondition();
			if (cLower.m_pBuffer == 0)
			{
				// 左端のページを得る
				pSearchPage
					= attachPage(getHeaderPage()->getLeftLeafPageID(),
								 SimplePage::PagePointer());
			}
			else
			{
				if (cLower.m_nullBitmap == 0)
				{
					bool isLower = true;
					if (cLower.m_eType == TreeNodeInterface::GreaterThan)
						isLower = false;
					pSearchPage = getLeafPage(cLower.m_pBuffer,
											  cLower.m_cCompare,
											  isLower);
				}
			}
		}
		else
		{
			// 逆方向に取り出す
			Condition::LimitCond& cUpper = pCondition_->getUpperCondition();
			if (cUpper.m_pBuffer == 0)
			{
				// 右端のページを得る
				pSearchPage
					= attachPage(getHeaderPage()->getRightLeafPageID(),
								 SimplePage::PagePointer());
			}
			else
			{
				if (cUpper.m_nullBitmap == 0)
				{
					bool isLower = false;
					if (cUpper.m_eType == TreeNodeInterface::LessThan)
						isLower = true;
					pSearchPage = getLeafPage(cUpper.m_pBuffer,
											  cUpper.m_cCompare,
											  isLower);
				}
			}
		}
	}
	
	return pSearchPage;
}

//
//	FUNCTION private
//	Btree2::SimpleFile::next -- 次の検索結果を設定する
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
SimpleFile::next()
{
	using namespace LogicalFile;
	
	if (m_uiSearchPageID == PhysicalFile::ConstValue::UndefinedPageID)
		return;
	
	Condition::LimitCond& cLower = m_pCondition->getLowerCondition();
	if (m_iSearchEntryPosition != -1 && cLower.m_cCompare.isUnique())
	{
		// ユニーク検索の2回目
		eraseLock(m_uiSearchPageID);
		m_uiSearchPageID = PhysicalFile::ConstValue::UndefinedPageID;
		return;
	}
	
	if (m_pSearchPage == 0)
	{
		// デタッチページされた後
		if (checkLock(m_uiSearchPageID) == false)
		{
			const Compare& compare = getCompare();
			// このページは更新されてしまったので検索する
			m_pSearchPage = getLeafPage(m_pSearchEntryBuffer,
										compare,
										false);
			m_uiSearchPageID = m_pSearchPage->getID();
			// ロック情報を登録する
			insertLock(m_uiSearchPageID);

			// ページ内を検索する
			SimplePage::Iterator j = m_pSearchPage->find(m_pSearchEntryBuffer,
														 compare);
			if (j == m_pSearchPage->end())
			{
				// あり得ない
				_SYDNEY_THROW0(Exception::Unexpected);
			}

			m_iSearchEntryPosition
				= static_cast<int>(j - m_pSearchPage->begin());
		}
		else
		{
			// attachする
			m_pSearchPage = attachPage(m_uiSearchPageID,
									   SimplePage::PagePointer());
		}
	}
	
	SimplePage::Iterator i;
		
	if (m_iSearchEntryPosition == -1)
	{
		// はじめてのnext()
		if (cLower.m_pBuffer)
		{
			if (cLower.m_eType == TreeNodeInterface::GreaterThan)
			{
				i = m_pSearchPage->upperBound(cLower.m_pBuffer,
											  cLower.m_cCompare);
			}
			else
			{
				i = m_pSearchPage->lowerBound(cLower.m_pBuffer,
											  cLower.m_cCompare);
			}
		}
		else
		{
			i = m_pSearchPage->begin();
		}
	}
	else
	{
		// 直前の位置
		i = m_pSearchPage->begin() + m_iSearchEntryPosition;
		// の次
		++i;
	}
		
	while (true)
	{
		if (i == m_pSearchPage->end())
		{
			// このページは終わりなので次へ
				
			// ロック情報を削除する
			eraseLock(m_uiSearchPageID);
				
			m_uiSearchPageID = m_pSearchPage->getNextPageID();
			if (m_uiSearchPageID
				== PhysicalFile::ConstValue::UndefinedPageID)
				// もうページがない
				break;

			m_pSearchPage = attachPage(m_uiSearchPageID,
									   SimplePage::PagePointer());
			i = m_pSearchPage->begin();

			insertLock(m_uiSearchPageID);
		}

		Condition::LimitCond& cUpper = m_pCondition->getUpperCondition();
		if (cUpper.m_pBuffer && cUpper.m_cCompare(*i, cUpper.m_pBuffer) > 0)
		{
			// もう終わり
			eraseLock(m_uiSearchPageID);
			m_uiSearchPageID = PhysicalFile::ConstValue::UndefinedPageID;
			break;
		}

		if (m_pCondition->isOtherConditionMatch(*i) == true)
		{
			// エントリをコピーする
			copyEntry(m_pSearchEntryBuffer, i);
			// 位置
			m_iSearchEntryPosition
				= static_cast<int>(i - m_pSearchPage->begin());
			break;
		}

		// 条件にマッチしないので次へ
		++i;
	}
}

//
//	FUNCTION private
//	Btree2::SimpleFile::nextReverse -- 次の検索結果を設定する
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
SimpleFile::nextReverse()
{
	using namespace LogicalFile;
	
	if (m_uiSearchPageID == PhysicalFile::ConstValue::UndefinedPageID)
		return;
	
	Condition::LimitCond& cUpper = m_pCondition->getUpperCondition();
	if (m_iSearchEntryPosition != -1 && cUpper.m_cCompare.isUnique())
	{
		// ユニーク検索の2回目
		eraseLock(m_uiSearchPageID);
		m_uiSearchPageID = PhysicalFile::ConstValue::UndefinedPageID;
		return;
	}
	
	if (m_pSearchPage == 0)
	{
		if (checkLock(m_uiSearchPageID) == false)
		{
			const Compare& compare = getCompare();
			// このページは更新されてしまったので検索する
			m_pSearchPage = getLeafPage(m_pSearchEntryBuffer,
										compare,
										false);
			m_uiSearchPageID = m_pSearchPage->getID();
			// ロック情報を登録する
			insertLock(m_uiSearchPageID);

			// ページ内を検索する
			SimplePage::Iterator j
				= m_pSearchPage->find(m_pSearchEntryBuffer, compare);
			if (j == m_pSearchPage->end())
			{
				// あり得ない
				_SYDNEY_THROW0(Exception::Unexpected);
			}

			m_iSearchEntryPosition
				= static_cast<int>(j - m_pSearchPage->begin());
		}
		else
		{
			// attachする
			m_pSearchPage = attachPage(m_uiSearchPageID,
									   SimplePage::PagePointer());
		}
	}
	
	SimplePage::Iterator i;
		
	if (m_iSearchEntryPosition == -1)
	{
		// はじめてのnext()
		if (cUpper.m_pBuffer)
		{
			if (cUpper.m_eType == TreeNodeInterface::LessThan)
			{
				i = m_pSearchPage->lowerBound(cUpper.m_pBuffer,
											  cUpper.m_cCompare);
			}
			else
			{
				i = m_pSearchPage->upperBound(cUpper.m_pBuffer,
											  cUpper.m_cCompare);
			}
		}
		else
		{
			i = m_pSearchPage->end();
		}
	}
	else
	{
		// 直前の位置
		i = m_pSearchPage->begin() + m_iSearchEntryPosition;
	}

	bool cont = true;

	if (i == m_pSearchPage->begin())
	{
		// このページは終わりなので次へ
		
		// ロック情報を削除する
		eraseLock(m_uiSearchPageID);
				
		m_uiSearchPageID = m_pSearchPage->getPrevPageID();
		if (m_uiSearchPageID == PhysicalFile::ConstValue::UndefinedPageID)
		{
			// もうページがない
			cont = false;
		}
		else
		{
			m_pSearchPage = attachPage(m_uiSearchPageID,
									   SimplePage::PagePointer());
			i = m_pSearchPage->end();
			// ロック情報を登録する
			insertLock(m_uiSearchPageID);
		}
	}

	while (cont)
	{
		--i;
		
		Condition::LimitCond& cLower = m_pCondition->getLowerCondition();
		if (cLower.m_pBuffer && cLower.m_cCompare(*i, cLower.m_pBuffer) < 0)
		{
			// もう終わり
			eraseLock(m_uiSearchPageID);
			m_uiSearchPageID = PhysicalFile::ConstValue::UndefinedPageID;
			break;
		}

		if (m_pCondition->isOtherConditionMatch(*i) == true)
		{
			// エントリをコピーする
			copyEntry(m_pSearchEntryBuffer, i);
			// 位置
			m_iSearchEntryPosition
				= static_cast<int>(i - m_pSearchPage->begin());
			break;
		}

		if (i == m_pSearchPage->begin())
		{
			// このページは終わりなので次へ
				
			// ロック情報を削除する
			eraseLock(m_uiSearchPageID);
				
			m_uiSearchPageID = m_pSearchPage->getPrevPageID();
			if (m_uiSearchPageID
				== PhysicalFile::ConstValue::UndefinedPageID)
				// もうページがない
				break;

			m_pSearchPage = attachPage(m_uiSearchPageID,
									   SimplePage::PagePointer());
			i = m_pSearchPage->end();
			// ロック情報を登録する
			insertLock(m_uiSearchPageID);
		}
	}
}

//
//	FUNCTION private
//	Btree2::SimpleFile::copyEntry -- リーフエントリをコピーする
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32* dst
//		コピー先のバッファ
//	SimplePage::Iterator i_
//		コピーするエントリへのイテレータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SimpleFile::copyEntry(ModUInt32* dst, SimplePage::Iterator i_)
{
	ModSize size = static_cast<ModSize>((*(i_ + 1) - *i_) * sizeof(ModUInt32));
	Os::Memory::copy(dst, *i_, size);
}

//
//	FUNCTION private
//	Btree2::SimpleFile::getHitCount -- ページ内のヒット件数を確認する
//
//	NOTES
//
//	ARGUMENTS
//	Btree2::SimplePage::PagePointer pPage_
//		検索するページ
//	Btree2::Condition* pCondition_
//		検索条件
//	Btree2::SimplePage::Iterator& l_
//		下限
//	Btree2::SimplePage::Iterator& u_
//		上限
//		
//	RETURN
//	ModSize
//		ヒット件数
//
//	EXCEPTIONS
//
ModSize
SimpleFile::getHitCount(SimplePage::PagePointer pPage_,
						Condition* pCondition_,
						SimplePage::Iterator& l_,
						SimplePage::Iterator& u_,
						bool isLower_)
{
	using namespace LogicalFile;
	
	// 上限と下限の条件を得る
	Condition::LimitCond& cLower = pCondition_->getLowerCondition();
	Condition::LimitCond& cUpper = pCondition_->getUpperCondition();
	
	// 下限を検索
	if (cLower.m_pBuffer == 0)
	{
		// 下限がない
		l_ = pPage_->begin();
	}
	else
	{
		if (cLower.m_cCompare.isUnique() ||
			cLower.m_eType == TreeNodeInterface::GreaterThan)
		{
			// upper_boundで検索する
			l_ = pPage_->upperBound(cLower.m_pBuffer, cLower.m_cCompare);
		}
		else
		{
			// lower_boundで検索する
			l_ = pPage_->lowerBound(cLower.m_pBuffer, cLower.m_cCompare);
		}
		
		// 隣り合う2ページの下限部分の場合、ヒットしなかったら終わり
		if (isLower_ && l_ == pPage_->end())
			return 0;
		
		// その１つ前
		if (l_ != pPage_->begin() && pPage_->isLeaf() == false)
			--l_;
	}

	// 上限を検索
	if (cUpper.m_pBuffer == 0)
	{
		// 上限がない
		u_ = pPage_->end();
	}
	else
	{
		if (cUpper.m_eType != TreeNodeInterface::LessThan)
		{
			// upper_boundで検索する
			u_ = pPage_->upperBound(cUpper.m_pBuffer, cUpper.m_cCompare);
		}
		else
		{
			// lower_boundで検索する
			u_ = pPage_->lowerBound(cUpper.m_pBuffer, cUpper.m_cCompare);
		}
	}
	// その1つ前
	if (u_ != pPage_->begin())
		--u_;

	if (l_ > u_)
		// ヒットしない
		return 0;

	ModSize n = (u_ - l_) + 1;

	if (n >= 1)
	{
		// B木の比較はPadSpaceで行われるが、LIKEはNoPadでする必要がある
		// そのため、例えば、'abc%' は、'abc' <= x < 'abd' ではなく、
		// 'abb' < x < 'abd' となり、多く見積もられすぎてしまう
		// よって、上限と下限に違いがあったところで、OtherConditionも
		// 評価し、その誤差を軽減する

		if (pPage_->isLeaf())
		{
			// リーフなので、全部 OtherCondition を評価する
			
			n = 0;
			SimplePage::Iterator i = l_;
			while (i <= u_)
			{
				if (pCondition_->isOtherConditionMatch(*i) == true)
				{
					++n;
				}

				++i;
			}
		}
		else if (isLower_)
		{
			// ノードだけど、下限側のページなので、全部評価する

			while (l_ <= u_)
			{
				if (pCondition_->isOtherConditionMatch(*l_) == true)
					break;

				++l_;
				--n;
			}
		}
		else if (n > 2)
		{
			// ノードなので、u_-2 まで評価する

			while (n > 2)
			{
				if (pCondition_->isOtherConditionMatch(*l_) == true)
					break;

				++l_;
				--n;
			}
		}
	}

	return n;
}

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2011, 2012, 2014, 2015, 2018, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
