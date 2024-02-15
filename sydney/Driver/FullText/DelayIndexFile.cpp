// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DelayIndexFile.cpp -- 転置ファイルのラッパークラス(遅延更新用)
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FullText";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "FullText/DelayIndexFile.h"
#include "FullText/OpenOption.h"
#include "FullText/MergeReserve.h"
#include "FullText/Parameter.h"
#include "FullText/MessageAll_Class.h"

#include "Common/StringArrayData.h"
#include "Common/DataArrayData.h"
#include "Common/Message.h"
#include "Common/Assert.h"

#include "Checkpoint/Daemon.h"

#include "FileCommon/OpenOption.h"
#include "FileCommon/FileOption.h"
#include "FileCommon/IDNumber.h"

#include "LogicalFile/OpenOption.h"
#include "LogicalFile/FileID.h"

#include "Trans/AutoLatch.h"
#include "Trans/Transaction.h"

#include "Os/Path.h"

#include "Exception/VerifyAborted.h"

#include "ModAutoPointer.h"
#include "ModLanguageSet.h"
#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT_USING

namespace
{
	//
	//	VARIABLE local
	//	_$$::_InsertMergeFileSize -- マージを開始する小転置のファイルサイズ
	//	_$$::_ExpungeMergeFileSize -- マージを開始する小転置のファイルサイズ
	//	_$$::_InsertMergeTupleSize -- マージを開始する小転置のタプル数
	//	_$$::_ExpungeMergeTupleSize -- マージを開始する小転置のタプル数
	//
	//	NOTES
	//	マージの閾値はファイルサイズとタプル数であるが、どちらか一方の条件
	//	を満たせばマージを開始する。0を指定するとその条件は無視される。
	//
	ParameterInteger _InsertMergeFileSize(
		"FullText_InsertMergeFileSize",	128 << 20);
	ParameterInteger _ExpungeMergeFileSize(
		"FullText_ExpungeMergeFileSize", 128 << 20);

	ParameterInteger _InsertMergeTupleSize(
		"FullText_InsertMergeTupleSize", 0);
	ParameterInteger _ExpungeMergeTupleSize(
		"FullText_ExpungeMergeTupleSize", 0);

	//
	//	CLASS
	//	_$$::_AutoDetachPage
	//
	class _AutoDetachPage
	{
	public:
		_AutoDetachPage(DelayIndexFile& cFile_) : m_cFile(cFile_)
		{
		}
		~_AutoDetachPage()
		{
			m_cFile.recoverAllPages();
		}
		void flush()
		{
			m_cFile.flushAllPages();
		}

	private:
		DelayIndexFile& m_cFile;
	};

	//
	//	VARIABLE local
	//	_$$::_IsAsyncMerge -- 非同期マージを行うかどうか
	//
	ParameterBoolean _IsAsyncMerge("FullText_IsAsyncMerge", true);
}

//
//	FUNCTION public
//	FullText::DelayIndexFile::DelayIndexFile -- コンストラクタ(1)
//
//	NOTES
//
//	ARGUMENTS
//	FullText::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
DelayIndexFile::DelayIndexFile(FullText::FileID& cFileID_)
	: m_pTransaction(0),
	m_bEstimate(false), m_bInsertDone(false), m_bExpungeDone(false),
	m_cInfoFile(cFileID_),IndexFile(cFileID_)
{
}

//
//	FUNCTION public
//	FullText::DelayIndexFile::getSize -- ファイルサイズを得る
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
DelayIndexFile::getSize(const Trans::Transaction& cTrans_)
{
	ModUInt64 size = m_cInfoFile.getSize(cTrans_);
	size += IndexFile::getSize(cTrans_);
	return size;
}

//
//	FUNCTION public
//	FullText::DelayIndexFile::destroy -- ファイルを破棄する
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
DelayIndexFile::destroy(const Trans::Transaction& cTransaction_)
{
	IndexFile::destroy(cTransaction_);
	m_cInfoFile.destroy(cTransaction_);
}

//
//	FUNCTION public
//	FullText::DelayIndexFile::mount -- マウントする
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
DelayIndexFile::mount(const Trans::Transaction& cTransaction_)
{
	IndexFile::mount(cTransaction_);
	try
	{
		m_cInfoFile.mount(cTransaction_);
	}
	catch (...)
	{
		IndexFile::unmount(cTransaction_);
	}
}

//
//	FUNCTION public
//	FullText::DelayIndexFile::unmount -- アンマウントする
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
DelayIndexFile::unmount(const Trans::Transaction& cTransaction_)
{
	IndexFile::unmount(cTransaction_);
	try
	{
		m_cInfoFile.unmount(cTransaction_);
	}
	catch (...)
	{
		IndexFile::mount(cTransaction_);
	}

}

//
//	FUNCTION public
//	FullText::DelayIndexFile::flush -- フラッシュする
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
DelayIndexFile::flush(const Trans::Transaction& cTransaction_)
{
	IndexFile::flush(cTransaction_);
	m_cInfoFile.flush(cTransaction_);
}

//
//	FUNCTION public
//	FullText::DelayIndexFile::startBackup -- バックアップを開始する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const bool bRestorable_
//		リストアフラグ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DelayIndexFile::startBackup(const Trans::Transaction& cTransaction_,
							const bool bRestorable_)
{
	IndexFile::startBackup(cTransaction_, bRestorable_);
	try
	{
		m_cInfoFile.startBackup(cTransaction_, bRestorable_);
	}
	catch (...)
	{
		IndexFile::endBackup(cTransaction_);
	}
}

//
//	FUNCTION public
//	FullText::DelayIndexFile::endBackup -- バックアップを終了する
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
DelayIndexFile::endBackup(const Trans::Transaction& cTransaction_)
{
	IndexFile::endBackup(cTransaction_);
	m_cInfoFile.endBackup(cTransaction_);
}

//
//	FUNCTION public
//	FullText::DelayIndexFile::recover -- 障害から回復する
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
DelayIndexFile::recover(const Trans::Transaction& cTransaction_,
						const Trans::TimeStamp& cPoint_)
{
	IndexFile::recover(cTransaction_, cPoint_);
	m_cInfoFile.recover(cTransaction_, cPoint_);
}

//
//	FUNCTION public
//	FullText::DelayIndexFile::verify -- 整合性検査を行う
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
DelayIndexFile::verify(const Trans::Transaction& cTransaction_,
						 Admin::Verification::Treatment::Value uiTreatment_,
						 Admin::Verification::Progress& cProgress_)
{
	_AutoDetachPage cAuto(*this);

	// ファイルチェック
	for (Iterator iter = begin(); iter != end(); ++iter)
	{
		Admin::Verification::Progress cProgress(cProgress_.getConnection());
		iter->verify(cTransaction_, uiTreatment_, cProgress);
		cProgress_ += cProgress;
		if (cProgress_.isGood() == false
			&& !(uiTreatment_ & Admin::Verification::Treatment::Continue))
			_SYDNEY_THROW0(Exception::VerifyAborted);
	}
	{
		Admin::Verification::Progress cProgress(cProgress_.getConnection());
		m_cInfoFile.verify(cTransaction_, uiTreatment_, cProgress);
		cProgress_ += cProgress;
		if (cProgress_.isGood() == false
			&& !(uiTreatment_ & Admin::Verification::Treatment::Continue))
			_SYDNEY_THROW0(Exception::VerifyAborted);
	}

	// ROWID整合性チェック
	LogicalFile::OpenOption cOpenOption;

	cOpenOption.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
		FileCommon::OpenOption::OpenMode::Key),
							 LogicalFile::OpenOption::OpenMode::Read);
	cOpenOption.setBoolean(_SYDNEY_OPEN_PARAMETER_KEY(
		FileCommon::OpenOption::Estimate::Key), false);

	// すべてのファイルをオープン
	open(cTransaction_, cOpenOption);

	Inverted::SearchCapsule *pSearchCapsule = new Inverted::SearchCapsule(
								*m_pOpenOption,
								0,
								this,
								m_pTokenizer
								);
	try
	{
	 // それぞれの転置索引を全件検索する
		Common::BitSet cBig
			= getAllEntry(pSearchCapsule,Inverted::Sign::_FullInvert);
		Common::BitSet cIns0
			= getAllEntry(pSearchCapsule,getMergeInsert());
		Common::BitSet cDel0
			= getAllEntry(pSearchCapsule,getMergeExpunge());
		Common::BitSet cIns1
			= getAllEntry(pSearchCapsule,getInsert());
		Common::BitSet cDel1
			= getAllEntry(pSearchCapsule,getExpunge());

		// 全エントリ
		Common::BitSet cAll = cBig;
		cAll |= cIns0;
		cAll |= cIns1;

		Common::BitSet::ConstIterator i = cAll.begin();
		for (; i != cAll.end(); ++i)
		{
			ModSize id = (*i);

			// 大転置と小転置の関係を調べる
			switch (((cBig[id]) ? 16 : 0)	 //大転置
					 + ((cDel0[id]) ? 8 : 0)	//Daemon 用削除
					 + ((cIns0[id]) ? 4 : 0)	//Daemon 用挿入
					 + ((cDel1[id]) ? 2 : 0)	//Executor 用削除
					 + ((cIns1[id]) ? 1 : 0)) //Executor 用挿入
			{
			case	1:// 0 - 0 + 0 - 0 + 1 の場合
			case	4:// 0 - 0 + 1 - 0 + 0 の場合
			case	6:// 0 - 0 + 1 - 1 + 0 の場合
			case	7:// 0 - 0 + 1 - 1 + 1 の場合
			case	16:// 1 - 0 + 0 - 0 + 0 の場合
			case	18:// 1 - 0 + 0 - 1 + 0 の場合
			case	19:// 1 - 0 + 0 - 1 + 1 の場合
			case	24:// 1 - 1 + 0 - 0 + 0 の場合
			case	25:// 1 - 1 + 0 - 0 + 1 の場合
			case	28:// 1 - 1 + 1 - 0 + 0 の場合
			case	30:// 1 - 1 + 1 - 1 + 0 の場合
			case	31:// 1 - 1 + 1 - 1 + 1 の場合
				//正常
				break;
			case	8:// 0 - 1 + 0 - 0 + 0 の場合
			case	9:// 0 - 1 + 0 - 0 + 1 の場合
			case	10:// 0 - 1 + 0 - 1 + 0 の場合
			case	11:// 0 - 1 + 0 - 1 + 1 の場合
			case	12:// 0 - 1 + 1 - 0 + 0 の場合
			case	13:// 0 - 1 + 1 - 0 + 1 の場合
			case	14:// 0 - 1 + 1 - 1 + 0 の場合
			case	15:// 0 - 1 + 1 - 1 + 1 の場合
			case	20:// 1 - 0 + 1 - 0 + 0 の場合
			case	21:// 1 - 0 + 1 - 0 + 1 の場合
			case	22:// 1 - 0 + 1 - 1 + 0 の場合
			case	23:// 1 - 0 + 1 - 1 + 1 の場合
			case	2:// 0 - 0 + 0 - 1 + 0 の場合
			case	3:// 0 - 0 + 0 - 1 + 1 の場合
			case	26:// 1 - 1 + 0 - 1 + 0 の場合
			case	27:// 1 - 1 + 0 - 1 + 1 の場合
			case	5:// 0 - 0 + 1 - 0 + 1 の場合
			case	17:// 1 - 0 + 0 - 0 + 1 の場合
			case	29:// 1 - 1 + 1 - 0 + 1 の場合
				{
					Admin::Verification::Progress cProgress(
						cProgress_.getConnection());
					_SYDNEY_VERIFY_INCONSISTENT(cProgress,
												m_cFileID.getPath(),
												Message::InaccurateRowid());
					cProgress_ += cProgress;
					if (cProgress_.isGood() == false
						&& !(uiTreatment_
							 & Admin::Verification::Treatment::Continue))
						_SYDNEY_THROW0(Exception::VerifyAborted);
				}
				break;
			case	0:// 0 - 0 + 0 - 0 + 0 の場合
			default:
				_SYDNEY_ASSERT(0);
				break;//ありえない
			}

			// 検査した削除用小転置は 0 にする
			cDel0.reset(id);
			cDel1.reset(id);
		}

		if (cDel0.any() || cDel1.any())
		{
			// 削除用小転置にROWIDが残っていたらエラー

			Admin::Verification::Progress cProgress(cProgress_.getConnection());
			_SYDNEY_VERIFY_INCONSISTENT(cProgress,
										m_cFileID.getPath(),
										Message::InaccurateRowid());
			cProgress_ += cProgress;
			if (cProgress_.isGood() == false
				&& !(uiTreatment_ & Admin::Verification::Treatment::Continue))
				_SYDNEY_THROW0(Exception::VerifyAborted);
		}
	}
	catch (...)
	{
		delete pSearchCapsule;
		close();
		_SYDNEY_RETHROW;
	}
	delete pSearchCapsule;
	close();
}

//
//	FUNCTION public
//	FullText::DelayIndexFile::restore
//		-- ある時点に開始された読取専用トランザクションが
//			参照する版を最新版とする
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
DelayIndexFile::restore(const Trans::Transaction& cTransaction_,
						const Trans::TimeStamp& cPoint_)
{
	IndexFile::restore(cTransaction_, cPoint_);
	m_cInfoFile.restore(cTransaction_, cPoint_);
}

//
//	FUNCTION public
//	FullText::DelayIndexFile::open -- ファイルをオープンする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const LogicalFile::OpenOption& cOption_
//		オープンオプション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DelayIndexFile::open(const Trans::Transaction& cTransaction_,
					 const LogicalFile::OpenOption& cOption_)
{
	m_pTransaction = &cTransaction_;
	IndexFile::open(cTransaction_,cOption_);
	m_cInfoFile.open(cTransaction_, cOption_);
}

//
//	FUNCTION public
//	FullText::DelayIndexFile::close -- ファイルをクローズする
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
DelayIndexFile::close()
{
	IndexFile::close();
	m_cInfoFile.close();
}

//
//	FUNCTION public
//	FullText::DelayIndexFile::sync -- 同期を取る
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	bool& incomplete
//			処理し残したかどうか
//	bool& modified
//		更新されたかどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DelayIndexFile::sync(const Trans::Transaction& cTransaction_,
					 bool& incomplete, bool& modified)
{
	IndexFile::sync(cTransaction_, incomplete, modified);
	m_cInfoFile.sync(cTransaction_, incomplete, modified);
}
void
DelayIndexFile::move(const Trans::Transaction& cTransaction_,
					 const Common::StringArrayData& cArea_)
{
	// 現在のパスを得る
	Common::StringArrayData cOrgArea;
	cOrgArea.setElement(0, m_cFileID.getPath());

	IndexFileSet::move(cTransaction_,cArea_);
	try
	{
		m_cInfoFile.move(cTransaction_, cArea_);
	}
	catch (...)
	{
		IndexFileSet::move(cTransaction_, cOrgArea);
		_SYDNEY_RETHROW;
	}
}

//
//
//	FUNCTION public
//	FullText::DelayIndexFile::recoverAllPages -- すべてのページの更新を破棄する
//
//	NOTES
//	といっても、本当に破棄できるのはInfoFileのみ。
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
DelayIndexFile::recoverAllPages()
{
	IndexFile::recoverAllPages();
	m_cInfoFile.recoverAllPages();
}

//
//	FUNCTION public
//	FullText::DelayIndexFile::flushAllPages -- すべてのページの更新を確定する
//
//	NOTES
//	といっても、本当に確定できるのはInfoFileのみ。
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
DelayIndexFile::flushAllPages()
{
	IndexFile::flushAllPages();
	m_cInfoFile.flushAllPages();
}


//
//	FUNCTION public
//	FullText::DelayIndexFile::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiTupleID_
//		タプルID
//	cosnt ModUnicodeString& cstrDocument_
//		各セクションを連結した文字列
//	const ModVector<ModLanguageSet>& vecLanguage_
//		各セクションの言語情報(言語指定がない場合は空を指定する)
//	ModVector<ModSize>& vecSectionOffset_
//		セクションオフセット(挿入後は正規化後のオフセットになる)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DelayIndexFile::insert(const ModUnicodeString& cstrDocument_,
					   const ModVector<ModLanguageSet>& vecLanguage_,
					   ModUInt32 uiTupleID_,
					   ModVector<ModSize>& vecSectionOffset_,
					   ModInvertedFeatureList& vecFeature_)
{
	if (m_cInfoFile.isMounted(*m_pTransaction) == false)
	{
		m_cInfoFile.create(*m_pTransaction);
	}

	_AutoDetachPage cAuto(*this);

	if (m_bBatch == true)
	{
		IndexFile::insert(cstrDocument_,
						  vecLanguage_,
						  uiTupleID_,
						  vecSectionOffset_,
						  vecFeature_);
	}
	else
	{
		getInsertFile().insert(m_pTokenizer,
							   cstrDocument_,
							   vecLanguage_,
							   uiTupleID_,
							   vecSectionOffset_,
							   vecFeature_);

		if (isInsertMerge() == true)
		{
			if (_IsAsyncMerge.get() == true)
			{
				// マージ条件を満たしたのでマージスタックに登録する
				MergeReserve::pushBack(m_cFileID.getLockName());
			}
			else
			{
				// 同期マージを行う
				syncMerge();
			}
		}

	}
}

//
//	FUNCTION public
//	FullText::DelayIndexFile::expunge -- 削除する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiTupleID_
//		タプルID
//	cosnt ModUnicodeString& cstrDocument_
//		各セクションを連結した文字列
//	const ModVector<ModLanguageSet>& vecLanguage_
//		各セクションの言語情報(言語指定がない場合は空を指定する)
//	const ModVector<ModSize>& vecSectionOffset_
//		セクションオフセット
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DelayIndexFile::expunge(const ModUnicodeString& cstrDocument_,
						const ModVector<ModLanguageSet>& vecLanguage_,
						ModUInt32 uiTupleID_,
						const ModVector<ModSize>& vecSectionOffset_)

{
	_AutoDetachPage cAuto(*this);
	Inverted::IndexFile& cInsertFile = getInsertFile();
	cInsertFile.setTokenizer(m_pTokenizer);
	if (cInsertFile.contains(uiTupleID_) == true)
	{
		// 小転置にあるので削除
		cInsertFile.expunge(m_pTokenizer,
							cstrDocument_,
							vecLanguage_,
							uiTupleID_,
							vecSectionOffset_);
	}
	else
	{
		// 小転置にないので、削除用小転置に挿入
		ModVector<ModSize> vecSectionOffset = vecSectionOffset_;
		ModInvertedFeatureList vecDummy;
		getExpungeFile().insert(m_pTokenizer,
								cstrDocument_,
								vecLanguage_,
								uiTupleID_,
								vecSectionOffset,
								vecDummy);

		if (isExpungeMerge() == true)
		{
			if (_IsAsyncMerge.get() == true)
			{
				// マージ条件を満たしたのでマージスタックに登録する
				MergeReserve::pushBack(m_cFileID.getLockName());
			}
			else
			{
				// 同期マージを行う
				syncMerge();
			}
		}
	}
}

//
//	FUNCTION public
//	FullText::DelayIndexFile::openForMerge -- マージ用のオープン
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
DelayIndexFile::openForMerge(const Trans::Transaction& cTransaction_)
{
	// ラッチする
	Trans::AutoLatch latch(const_cast<Trans::Transaction&>(cTransaction_),
							 m_cFileID.getLockName());

	m_pTransaction = &cTransaction_;


	_AutoDetachPage cAuto(*this);

	// 大転置用オープンオプションを作る
	LogicalFile::OpenOption cUpdateOption;
	cUpdateOption.setInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key),
		FileCommon::OpenOption::OpenMode::Update);

	// 小転置用オープンオプションを作る
	LogicalFile::OpenOption cReadOption;
	cReadOption.setInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key),
		FileCommon::OpenOption::OpenMode::Read);


	// 情報ファイルをオープンし、小転置切り替えを行う
	m_cInfoFile.open(cTransaction_, cUpdateOption);
	m_cInfoFile.flip();
	// swapする適当な場所を探す
	// ここでswapするとm_cInfoFile.getIndex()で落ちる

	if(m_cInfoFile.getIndex())
	{
		IndexFile::Iterator tmp0 = this->find(Inverted::Sign::_Insert0);
		IndexFile::Iterator tmp1 = this->find(Inverted::Sign::_Insert1);
		tmp0->signature(Inverted::Sign::_Insert1);
		tmp1->signature(Inverted::Sign::_Insert0);
		tmp0 = this->find(Inverted::Sign::_Delete0);
		tmp1 = this->find(Inverted::Sign::_Delete1);
		tmp0->signature(Inverted::Sign::_Delete1);
		tmp1->signature(Inverted::Sign::_Delete0);
	}

	// 大転置をオープン
	getFullInvert()->open(cTransaction_, cUpdateOption);
	// 小転置をオープン
	getMergeInsertFile().open(cTransaction_, cReadOption);
	getMergeExpungeFile().open(cTransaction_, cReadOption);
	getFullInvert()->setTokenizer(0);
	getMergeInsertFile().setTokenizer(0);
	getMergeExpungeFile().setTokenizer(0);
	// 変数の初期化
	m_cInsertKey.clear();
	m_cExpungeKey.clear();
	m_bInsertDone = false;
	m_bExpungeDone = false;

	cAuto.flush();

}

//
//	FUNCTION public
//	FullText::DelayIndexFile::closeForMerge -- マージ用のクローズ
//
//	NOTES
//
//	AGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DelayIndexFile::closeForMerge()
{
	// ラッチする
	Trans::AutoLatch latch(const_cast<Trans::Transaction&>(*m_pTransaction),
						   m_cFileID.getLockName());

	if (getMergeInsertFile().isOpen()) getMergeInsertFile().close();
	if (getMergeExpungeFile().isOpen()) getMergeExpungeFile().close();

	getFullInvert()->close();

	m_cInfoFile.close();
	m_pTransaction = 0;

}

//
//	FUNCTION public
//	FullText::DelayIndexFile::mergeList -- 1つの転置リストをマージする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		続きがある場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
DelayIndexFile::mergeList()
{
	// ラッチする
	Trans::AutoLatch latch(const_cast<Trans::Transaction&>(*m_pTransaction),
						   m_cFileID.getLockName());

	_AutoDetachPage cAuto(*this);

	bool result = false;

	if (m_cInfoFile.getProceeding() == 1)
	{
		// 実際にマージする
		result = mergeListInternal(getMergeInsertFile(), getMergeExpungeFile());

		cAuto.flush();
	}

	return result;
}

//
//	FUNCTION public
//	FullText::DelayIndexFile::mergeVector -- ベクター部分をマージする
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
FullText::DelayIndexFile::mergeVector()
{
	// ラッチする
	Trans::AutoLatch latch(const_cast<Trans::Transaction&>(*m_pTransaction),
							 m_cFileID.getLockName());

	{
		_AutoDetachPage cAuto(*this);

		if (m_cInfoFile.getProceeding() == 1)
		{
			
			// ベクタ部分をマージする
			getFullInvert()->mergeVectorFile(&getMergeInsertFile(),
										 &getMergeExpungeFile());

			// ベクタ部分マージ完了
			m_cInfoFile.nextProceeding();

			cAuto.flush();
		}
	}

	{
		_AutoDetachPage cAuto(*this);

		// それぞれをクローズする
		getMergeInsertFile().close();
		getMergeExpungeFile().close();

		// 内容をクリアする
		getMergeInsertFile().clear(*m_pTransaction, false);
		getMergeExpungeFile().clear(*m_pTransaction, false);

		// マージ完了
		m_cInfoFile.unsetProceeding();

		cAuto.flush();
	}
}

//
//	FUNCTION public static
//	FullText::DelayIndexFile::clearParameter -- パラメータをクリアする
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
DelayIndexFile::clearParameter()
{
	_InsertMergeFileSize.clear();
	_ExpungeMergeFileSize.clear();
	_InsertMergeTupleSize.clear();
	_ExpungeMergeTupleSize.clear();
}

IndexFile::Iterator
DelayIndexFile::getFullInvert()
{
	return IndexFile::find(Inverted::Sign::_FullInvert);
}
//
//	FUNCTION private
//	FullText::DelayIndexFile::getInsertFile -- 挿入用小転置を得る
//
//	NOTES
//	全文情報ファイルがオープンされていなければならない
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText::IndexFile&
//		挿入用小転置
//
//	EXCEPTIONS
//
Inverted::IndexFile&
DelayIndexFile::getInsertFile()
{
	return *IndexFile::findEntity(getInsert());
}

//
//	FUNCTION private
//	FullText::DelayIndexFile::getMergeInsertFile
//		-- マージする挿入用小転置を得る
//
//	NOTES
//	全文情報ファイルがオープンされていなければならない
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText::IndexFile&
//		マージする挿入用小転置
//
//	EXCEPTIONS
//
Inverted::IndexFile&
DelayIndexFile::getMergeInsertFile()
{
	return *IndexFile::findEntity(getMergeInsert());
}

//
//	FUNCTION private
//	FullText::DelayIndexFile::getExpungeFile -- 削除用小転置を得る
//
//	NOTES
//	全文情報ファイルがオープンされていなければならない
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText::IndexFile&
//		削除用小転置
//
//	EXCEPTIONS
//
Inverted::IndexFile&
DelayIndexFile::getExpungeFile()
{
	return *IndexFile::findEntity(getExpunge());
}

//
//	FUNCTION private
//	FullText::DelayIndexFile::getMergeExpungeFile
//		-- マージする削除用小転置を得る
//
//	NOTES
//	全文情報ファイルがオープンされていなければならない
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText::IndexFile&
//		マージする削除用小転置
//
//	EXCEPTIONS
//
Inverted::IndexFile&
DelayIndexFile::getMergeExpungeFile()
{
	return *IndexFile::findEntity(getMergeExpunge());
}

//
//	FUNCTION private
//	FullText::DelayIndexFile::isInsertMerge -- マージ条件を満たしているかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		マージ条件を満たしている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
DelayIndexFile::isInsertMerge()
{
	if (m_pTransaction->isNoLock())
		// 自動リカバリのトランザクションなので、マージしない
		return false;
	
	if ((_InsertMergeFileSize.get() != 0
		&& getInsertFile().getUsedSize(*m_pTransaction)
			> _InsertMergeFileSize.get())
		|| (_InsertMergeTupleSize.get() != 0
			&& getInsertFile().getCount() > _InsertMergeTupleSize.get())
		|| m_cInfoFile.isProceeding() == true)
		return true;
	return false;
}

//
//	FUNCTION private
//	FullText::DelayIndexFile::isExpungeMerge -- マージ条件を満たしているかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		マージ条件を満たしている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
DelayIndexFile::isExpungeMerge()
{
	if (m_pTransaction->isNoLock())
		// 自動リカバリのトランザクションなので、マージしない
		return false;
	
	if ((_ExpungeMergeFileSize.get() != 0
		&& getExpungeFile().getUsedSize(*m_pTransaction)
			>= _ExpungeMergeFileSize.get())
		|| (_ExpungeMergeTupleSize.get() != 0
			&& getExpungeFile().getCount() >= _ExpungeMergeTupleSize.get())
		|| m_cInfoFile.isProceeding() == true)
		return true;
	return false;
}

//
//	FUNCTION private
//	FullText::DelayIndexFile::getAllEntry -- 全件をビットマップで取得する
//
//	NOTES
//
//	ARGUMETNS
//		Inverted::SearchCapsule* pSearchCapsule
//		検索器
//		ModUnicodeString& sign
//		検索対象転置ファイルのsignature
//
//	RETURN
//	Common::BitSet
//		全件取得結果
//
//	EXCEPTIONS
//
Common::BitSet
DelayIndexFile::getAllEntry(Inverted::SearchCapsule* pSearchCapsule,
							ModUInt32 sign)
{
	Common::BitSet cBitSet;
	ModUInt32 resultType = 1 << Inverted::FieldType::Rowid;

	ModInvertedBooleanResult* resultSet	=
		_SYDNEY_DYNAMIC_CAST(ModInvertedBooleanResult*,
							 ModInvertedSearchResult::factory(resultType));
	pSearchCapsule->execute(sign,resultSet);
	ModInvertedBooleanResult ::Iterator first = resultSet->begin();
	ModInvertedBooleanResult ::Iterator last  = resultSet->end();
   	while (first != last)
	{
		cBitSet.set(*first++);
	}
	delete resultSet;
	return cBitSet;
}

//
//	FUNCTION private
//	FullText::DelayIndexFile::syncMerge -- 同期マージを行う
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
DelayIndexFile::syncMerge()
{
	Inverted::IndexFile& cInsertFile = getInsertFile();
	Inverted::IndexFile& cExpungeFile = getExpungeFile();
	IndexFile::Iterator cFullInvert = getFullInvert();
	cFullInvert->setTokenizer(0);
	cInsertFile.setTokenizer(0);
	cExpungeFile.setTokenizer(0);

	SydMessage << "Start FullText Index Merge ("
			   << m_cFileID.getPath() << ")" << ModEndl;
	
	bool doMerge = true;
	while (doMerge)
	{
		_AutoDetachPage cAuto(*this);

		// 転置リストをマージする
		doMerge = mergeListInternal(cInsertFile, cExpungeFile);

		cAuto.flush();
	}

	{
		// ベクターのマージ
		_AutoDetachPage cAuto(*this);
		cFullInvert->mergeVectorFile(&cInsertFile, &cExpungeFile);
		cAuto.flush();
	}

	{
		// 小転置のクリア
		_AutoDetachPage cAuto(*this);
		cInsertFile.clear(*m_pTransaction, false);
		cExpungeFile.clear(*m_pTransaction, false);
		cAuto.flush();
	}
	
	SydMessage << "End FullText Index Merge" << ModEndl;
}

//
//	FUNCTION private
//	FullText::DelayIndexFile::mergeListInternal -- 1つの転置リストをマージする
//
//	NOTES
//
//	ARGUMENTS
//	FullText::IndexFile& cInsertFile_
//		挿入用小転置
//	FullText::IndexFile& cExpungeFile_
//		削除用小転置
//
//	RETURN
//	bool
//		続きがある場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
DelayIndexFile::mergeListInternal(Inverted::IndexFile& cInsertFile_,
									Inverted::IndexFile& cExpungeFile_)
{
	// もう転置リストがないので終了
	if (m_bInsertDone == true && m_bExpungeDone == true)
	{
		return false;
	}

	Inverted::InvertedList* pInsertList = 0;
	Inverted::InvertedList* pExpungeList = 0;

	//	現状、lowerBoundで見つかるものは、必ず引数に与えられたキーの
	//	索引リストである。それを前提にコーディングしてある。

	if (m_bInsertDone == true)
	{
		// 挿入用は終了
		pExpungeList = cExpungeFile_.lowerBound(m_cExpungeKey);
	}
	else if (m_bExpungeDone == true)
	{
		// 削除用は終了
		pInsertList = cInsertFile_.lowerBound(m_cInsertKey);
	}
	else if (m_cInsertKey == m_cExpungeKey)
	{
		// 索引単位が同じ
		pInsertList = cInsertFile_.lowerBound(m_cInsertKey);
		pExpungeList = cExpungeFile_.lowerBound(m_cExpungeKey);

		if (m_cInsertKey.getLength() == 0)
		{
			// 一番最初->最初のキーで0件の場合のみnullが返ることがある
			if (pInsertList == 0)
				m_bInsertDone = true;
			if (pExpungeList == 0)
				m_bExpungeDone = true;
			if (m_bExpungeDone && m_bInsertDone)
				return false;
		}
	}
	else if (m_cInsertKey < m_cExpungeKey)
	{
		// 挿入用の方が小さい
		pInsertList = cInsertFile_.lowerBound(m_cInsertKey);
	}
	else
	{
		// 削除用の方が小さい
		pExpungeList = cExpungeFile_.lowerBound(m_cExpungeKey);
	}

	// マージする
	getFullInvert()->merge(pInsertList, pExpungeList);

	if (pInsertList)
	{
		pInsertList = cInsertFile_.next();
		if (pInsertList)
			m_cInsertKey = pInsertList->getKey();
		else
			m_bInsertDone = true;
	}
	if (pExpungeList)
	{
		pExpungeList = cExpungeFile_.next();
		if (pExpungeList)
			m_cExpungeKey = pExpungeList->getKey();
		else
			m_bExpungeDone = true;
	}

	return true;
}

// 検索器を得る
Inverted::SearchCapsule&
DelayIndexFile::getSearchCapsule(Inverted::OptionDataFile* file_)
{
	// 検索器を得る
	
	if (m_pSearchCapsule == 0)
	{
		// [NOTE] getSearchCapsule()が複数回呼ばれることがあるので、
		//  swapは初回の時だけにする。
		if(m_cInfoFile.getIndex() == 0)
		{
			// 小転置ファイルのswap
			// signatureだけのswapをすれば良い
			// _Insert0 <-> _Insert1
			// _Delete0 <-> _Delete1
			IndexFile::Iterator tmp0 = this->find(Inverted::Sign::_Insert0);
			IndexFile::Iterator tmp1 = this->find(Inverted::Sign::_Insert1);
			tmp0->signature(Inverted::Sign::_Insert1);
			tmp1->signature(Inverted::Sign::_Insert0);
			tmp0 = this->find(Inverted::Sign::_Delete0);
			tmp1 = this->find(Inverted::Sign::_Delete1);
			tmp0->signature(Inverted::Sign::_Delete1);
			tmp1->signature(Inverted::Sign::_Delete0);
		}
		
		_AutoDetachPage cAuto(*this);
		return IndexFile::getSearchCapsule(file_);
	} else
		return *m_pSearchCapsule;
}

Inverted::GetLocationCapsule&
DelayIndexFile::getGetLocationCapsule(Inverted::SearchCapsule* pCapsule_)
{
	if (m_pGetLocationCapsule == 0)
	{
		_AutoDetachPage cAuto(*this);
		m_pGetLocationCapsule = new Inverted::GetLocationCapsule(*pCapsule_);
	}
	return *m_pGetLocationCapsule;
}

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
