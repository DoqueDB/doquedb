// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogicalInterface.cpp --
// 
// Copyright (c) 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/LogicalInterface.h"

#include "FullText2/FullTextFile.h"
#include "FullText2/OpenOption.h"

#include "Common/DataArrayData.h"
#include "Common/IntegerArrayData.h"
#include "Common/StringArrayData.h"

#include "Common/Message.h"

#include "Trans/Transaction.h"

#include "FileCommon/OpenOption.h"

#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	//
	//  CLASS
	//  _$$::_AutoAttachFile
	//
	class _AutoAttachFile
	{
	public:
		_AutoAttachFile(LogicalInterface& cFile_)
			: m_cFile(cFile_), m_bOwner(false)
			{
				if (m_cFile.isAttached() == false)
				{
					m_cFile.attach();
					m_bOwner = true;
				}
			}
		~_AutoAttachFile()
			{
				if (m_bOwner) m_cFile.detach();
			}

	private:
		LogicalInterface& m_cFile;
		bool m_bOwner;
	};

}

//
//	FUNCTION public
//	FullText2::LogicalInterface::LogicalInterface -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
LogicalInterface::LogicalInterface(const LogicalFile::FileID& cFileID_)
	: m_cFileID(cFileID_), m_pFullTextFile(0)
{
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::~LogicalInterface -- デストラクタ
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
LogicalInterface::~LogicalInterface()
{
	detach();
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::isAccessible
//		-- 実体である OS ファイルが存在するか調べる
//
//	NOTES
//
//	ARGUMENTS
//	bool force (default false)
//		強制モードかどうか
//
//	RETURN
//	bool
//		存在する場合は true 、それ以外の場合は false
//
//	EXCEPTIONS
//
bool
LogicalInterface::isAccessible(bool force_) const
{
	_AutoAttachFile cAuto(*const_cast<LogicalInterface*>(this));
	return m_pFullTextFile->isAccessible(force_);
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::isMounted
//		-- マウントされているか調べる
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
LogicalInterface::isMounted(const Trans::Transaction& cTransaction_) const
{
	_AutoAttachFile cAuto(*const_cast<LogicalInterface*>(this));
	return m_pFullTextFile->isMounted(cTransaction_);
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::getSize -- ファイルサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTrans_
//		トランザクション
//
//	RETURN
//	ModUInt64
//		ファイルサイズ
//
//	EXCEPTIONS
//
ModUInt64
LogicalInterface::getSize(const Trans::Transaction& cTrans_)
{
	_AutoAttachFile cAuto(*this);
	return m_pFullTextFile->getSize();
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::getCount
//		-- 登録件数を得る。検索条件が与えられている場合には、検索結果件数
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModInt64
//		登録件数 or 検索結果件数
//
//	EXCEPTIONS
//
ModInt64
LogicalInterface::getCount() const
{
	// オープンされているはず
	return const_cast<FullTextFile*>(m_pFullTextFile)->getCount();
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::getOverhead -- オープン時のオーバヘッドを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	double
//		オーバヘッド(秒)
//
//	EXCEPTIONS
//
double
LogicalInterface::getOverhead() const
{
	// オープンされているはず
	return const_cast<FullTextFile*>(m_pFullTextFile)->getOverhead();
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::getProcessCost
//		-- ひとつのタプルを挿入or取得する時のプロセスコストを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	double
//		プロセスコスト(秒)
//
//	EXCEPTIONS
//
double
LogicalInterface::getProcessCost() const
{
	// オープンされているはず
	return const_cast<FullTextFile*>(m_pFullTextFile)->getProcessCost();
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::getSearchParameter
//		-- 検索用のパラメータをOpenOptionに設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	LogicalFile::OpenOption& cOpenOption_
//		検索用のパラメータを設定するOpenOption
//
//	RETURN
//	bool
//		全文索引で実行できる検索であればtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::
getSearchParameter(const LogicalFile::TreeNodeInterface* pCondition_,
				   LogicalFile::OpenOption& cOpenOption_) const
{
	if (pCondition_)
	{
		OpenOption cOpenOption(cOpenOption_);
		if (cOpenOption.parse(m_cFileID, pCondition_) == false)
			return false;

		// オープンモードを設定する
		cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
									FileCommon::OpenOption::OpenMode::Key),
								FileCommon::OpenOption::OpenMode::Search);
	}
	else
	{
		// 検索条件がないものは平均文書長と文書数の取得時である

		// オープンモードを設定する
		cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
									FileCommon::OpenOption::OpenMode::Key),
								FileCommon::OpenOption::OpenMode::Read);
	}

	// 全文はすべての検索結果をキャッシュしてしまう
	cOpenOption_.setBoolean(_SYDNEY_OPEN_PARAMETER_KEY(
								FileCommon::OpenOption::CacheAllObject::Key),
							true);

	return true;
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::getProjectionParameter
//		-- プロジェクションパラメータを得る
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pNode_
//		プロジェクションするフィールド
//	LogicalFile::OpenOption& cOpenOption_
//		プロジェクションパラメータを設定するオープンオプション
//
//	RETURN
//	bool
//		取得できないフィールドが含まれていた場合はfalse、それ以外の場合はtrue
//
//	EXCEPTIONS
//
bool
LogicalInterface::
getProjectionParameter(const LogicalFile::TreeNodeInterface* pNode_,
					   LogicalFile::OpenOption& cOpenOption_) const
{
	if (m_cFileID.getProjectionParameter(pNode_, cOpenOption_) == false)
		return false;

	// オープンモード
	int tmp;
	if (cOpenOption_.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::OpenMode::Key), tmp) == false)
	{
		// まだオープンモードが設定されていないので設定する
		cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
									FileCommon::OpenOption::OpenMode::Key),
								FileCommon::OpenOption::OpenMode::Read);
	}

	// 全文はすべての検索結果をキャッシュしてしまう
	cOpenOption_.setBoolean(_SYDNEY_OPEN_PARAMETER_KEY(
								FileCommon::OpenOption::CacheAllObject::Key),
							true);

	return true;
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::getUpdateParameter -- 更新パラメータを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::IntegerArrayData& cUpdateField_
//		更新するフィールド
//	LogicalFile::OpenOption& cOpenOption_
//		更新パラメータを設定するオープンオプション
//
//	RETURN
//	bool
//		更新できないフィールドが含まれていた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::
getUpdateParameter(const Common::IntegerArrayData& cUpdateField_,
				   LogicalFile::OpenOption& cOpenOption_) const
{
	if (m_cFileID.getUpdateParameter(cUpdateField_, cOpenOption_) == false)
		return false;

	// オープンモード
	cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
								FileCommon::OpenOption::OpenMode::Key),
							FileCommon::OpenOption::OpenMode::Update);

	return true;
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::getSortParameter
//		-- ソート順パラメータを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pNode_
//		ソート条件
//	LogicalFile::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETURN
//	bool
//		ソートできる場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::getSortParameter(const LogicalFile::TreeNodeInterface* pNode_,
								   LogicalFile::OpenOption& cOpenOption_) const
{
	return m_cFileID.getSortParameter(pNode_, cOpenOption_);
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::getLimitParameter -- 取得数と取得位置を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::IntegerArrayData& cSpec_
//		0番目はLIMIT、1番目はOFFSET(1ベース)
//	LogicalFile::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETURN
//	bool
//		実行できる場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::getLimitParameter(const Common::IntegerArrayData& cSpec_,
									LogicalFile::OpenOption& cOpenOption_) const
{
	int limit;
	int offset = 1;

	if (cSpec_.getCount() == 0 || cSpec_.getCount() > 2)
		return false;

	limit = cSpec_.getElement(0);
	if (cSpec_.getCount() == 2)
		offset = cSpec_.getElement(1);

	if (limit < 0 || offset < 1)
		return false;

	cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
								OpenOption::KeyID::Limit), limit);
	cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
								OpenOption::KeyID::Offset), offset);
	
	return true;
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::create
//		-- ファイルを作成する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	const LogicalFile::FileID&
//		ファイルID
//
//	EXCEPTIONS
//
const LogicalFile::FileID&
LogicalInterface::create(const Trans::Transaction& cTransaction_)
{
	// ファイルを作成すると言っても、物理的なファイルは作成しない
	// ファイルIDを整えるだけ
	
	m_cFileID.create();
	return m_cFileID;
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::destroy
//		-- ファイルを破棄する
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
LogicalInterface::destroy(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);
	m_pFullTextFile->destroy(cTransaction_);
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::mount
//		-- ファイルをマウントする
//
//	NOTES
//
//	ARGUMETNS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
// 	const LogicalFile::FileID&
//		ファイルID
//
//	EXCEPTIONS
//
const LogicalFile::FileID&
LogicalInterface::mount(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);

	m_pFullTextFile->mount(cTransaction_);
	m_cFileID.setMounted(true);

	return m_cFileID;
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::unmount
//		-- ファイルをアンマウントする
//
//	NOTES
//
//	ARGUMETNS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
// 	const LogicalFile::FileID&
//		ファイルID
//
//	EXCEPTIONS
//
const LogicalFile::FileID&
LogicalInterface::unmount(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);

	m_pFullTextFile->unmount(cTransaction_);
	m_cFileID.setMounted(false);

	return m_cFileID;
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::flush
//		-- ファイルをフラッシュする
//
//	NOTES
//
//	ARGUMETNS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::flush(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);
	m_pFullTextFile->flush(cTransaction_);
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::startBackup
//		-- バックアップを開始する
//
//	NOTES
//
//	ARGUMETNS
//	const Trans::Transaction& cTransaction_
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
LogicalInterface::startBackup(const Trans::Transaction& cTransaction_,
							  const bool bRestorable_)
{
	_AutoAttachFile cAuto(*this);
	m_pFullTextFile->startBackup(cTransaction_, bRestorable_);
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::flush
//		-- バックアップを終了する
//
//	NOTES
//
//	ARGUMETNS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::endBackup(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);
	m_pFullTextFile->endBackup(cTransaction_);
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::recover
//		-- 障害から回復する
//
//	NOTES
//
//	ARGUMETNS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Trans::TimeStamp& cPoint_
//		回復ポイント
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::recover(const Trans::Transaction& cTransaction_,
						  const Trans::TimeStamp& cPoint_)
{
	_AutoAttachFile cAuto(*this);
	m_pFullTextFile->recover(cTransaction_, cPoint_);
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::restore
//		-- ある時点に開始された読取専用トランザクションが参照する版を最新とする
//
//	NOTES
//
//	ARGUMETNS
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
LogicalInterface::restore(const Trans::Transaction& cTransaction_,
						  const Trans::TimeStamp& cPoint_)
{
	_AutoAttachFile cAuto(*this);
	m_pFullTextFile->restore(cTransaction_, cPoint_);
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::verify
//		-- 整合性検査を行う
//
//	NOTES
//
//	ARGUMETNS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Admin::Verification::Treatment::Value eTreatment_
//		処理方法
//	Admin::Verification::Progress& cProgress_
//		経過報告クラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::
verify(const Trans::Transaction& cTransaction_,
	   const Admin::Verification::Treatment::Value eTreatment_,
	   Admin::Verification::Progress& cProgress_)
{
	_AutoAttachFile cAuto(*this);
	m_pFullTextFile->verify(cTransaction_, eTreatment_, cProgress_);
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::open -- オープンする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const LogicalFile::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::open(const Trans::Transaction& cTransaction_,
					   const LogicalFile::OpenOption& cOpenOption_)
{
	attach();
	m_pFullTextFile->open(cTransaction_, cOpenOption_);
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::close -- クローズする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
// 	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::close()
{
	m_pFullTextFile->close();
	detach();
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::get -- データの取得を行う
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData* pTuple_
//		所得結果を格納する配列
//
//	RETURN
//	bool
//		データが取得できた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::get(Common::DataArrayData* pTuple_)
{
	if (pTuple_ == 0)
		_SYDNEY_THROW0(Exception::BadArgument);

	bool result = false;
	
	try
	{
		result = m_pFullTextFile->get(*pTuple_);
	}
	catch (...)
	{
		SydMessage << m_cFileID.getPath() << ModEndl;
		_SYDNEY_RETHROW;
	}

	return result;
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::insert -- データの挿入を行う
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData* pTuple_
//		挿入するデータが格納されている配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::insert(Common::DataArrayData* pTuple_)
{
	if (pTuple_ == 0)
		_SYDNEY_THROW0(Exception::BadArgument);

	try
	{
		m_pFullTextFile->insert(*pTuple_);
	}
	catch (...)
	{
		SydMessage << m_cFileID.getPath() << ModEndl;
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::update -- データの更新を行う
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData* pKey_
//		更新する行を特定するためのキーのデータが格納されている配列
//	Common::DataArrayData* pValue_
//		更新するカラムのデータが格納されている配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::update(const Common::DataArrayData* pKey_,
						 Common::DataArrayData* pValue_)
{
	if (pKey_ == 0 || pValue_ == 0)
		_SYDNEY_THROW0(Exception::BadArgument);

	try
	{
		m_pFullTextFile->update(*pKey_, *pValue_);
	}
	catch (...)
	{
		SydMessage << m_cFileID.getPath() << ModEndl;
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::expugne -- データの削除を行う
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData* pKey_
//		削除する行を特定するためのキーのデータが格納されている配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::expunge(const Common::DataArrayData* pKey_)
{
	if (pKey_ == 0)
		_SYDNEY_THROW0(Exception::BadArgument);

	try
	{
		m_pFullTextFile->expunge(*pKey_);
	}
	catch (...)
	{
		SydMessage << m_cFileID.getPath() << ModEndl;
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::fetch -- 検索条件を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData* pOption_
//		検索条件
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::fetch(const Common::DataArrayData* pOption_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::mark -- 巻き戻し位置を記録する
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
LogicalInterface::mark()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::rewind -- 巻き戻す
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
LogicalInterface::rewind()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::reset -- カーソルをリセットする
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
LogicalInterface::reset()
{
	m_pFullTextFile->reset();
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::equals -- 同じかどうか比較する
//
//	NOTES
//	同じファイル(同じパス)かどうか比較する
//
//	ARGUMENTS
//	const Common::Object* pOther_
//		比較対象へのポインタ
//
//	RETURN
//	bool
//		同じファイルの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::equals(const Common::Object* pOther_) const
{
	const LogicalInterface* pOther
		= dynamic_cast<const LogicalInterface*>(pOther_);
	if (pOther)
	{
		return toString() == pOther->toString();
	}
	return false;
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::sync
//		-- 同期処理を実施する
//
//	NOTES
//
//	ARGUMETNS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	bool& bIncomplete_
//		処理し残しがあるかどうか
//	bool& bModified_
//		ファイルを変更したかどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::sync(const Trans::Transaction& cTransaction_,
					   bool& bIncomplete_, bool& bModified_)
{
	try
	{
		_AutoAttachFile cAuto(*this);
		m_pFullTextFile->sync(cTransaction_, bIncomplete_, bModified_);
	}
	catch (...)
	{
		SydMessage << m_cFileID.getPath() << ModEndl;
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::move -- ファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Common::StringArrayData& cArea_
//		移動エリア
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::move(const Trans::Transaction& cTransaction_,
					   const Common::StringArrayData& cArea_)
{
	_AutoAttachFile cAuto(*this);
	
	Os::Path path = cArea_.getElement(0);	// 先頭要素のみ利用
	m_pFullTextFile->move(cTransaction_, path);
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::getNoLatchOperation
//		-- ラッチが不要なオペレーションを返す
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	LogicalFile::File::Operation::Value
//		ラッチが不要なオペレーションを示すビットをONにした値
//
// EXCEPTIONS
//
LogicalFile::File::Operation::Value
LogicalInterface::getNoLatchOperation()
{
	return Operation::Reset
		| Operation::GetProcessCost
		| Operation::GetOverhead
		| Operation::GetData;  // for keep latch during open-close
}

// FUNCTION public
//	FullText2::LogicalInterface::getCapability -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	LogicalFile::File::Capability::Value
//
// EXCEPTIONS

LogicalFile::File::Capability::Value
LogicalInterface::
getCapability()
{
	// 条件から件数を見積もることができる
	return Capability::EstimateCount;
}

//
//	FUNCTION public
//	FullText::LogicalInterface::toString -- ファイルを識別するための文字列を返す
//
//	NOTES
//	ファイルを識別するための文字列(パス)を返す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		ファイルを識別するための文字列
//
//	EXCEPTIONS
//
ModUnicodeString
LogicalInterface::toString() const
{
	return m_cFileID.getPath();
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::getProperty
//		-- プロパティを得る
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData* pKey_
//		プロパティの項目を表すキー
//	Common::DataArrayData* pValue_
//		キーに対応するプロパティのバリュー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::getProperty(Common::DataArrayData* pKey_,
							  Common::DataArrayData* pValue_)
{
	if (pKey_ == 0 || pValue_ == 0)
		_SYDNEY_THROW0(Exception::BadArgument);
	
	m_pFullTextFile->getProperty(*pKey_, *pValue_);
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::attach -- ファイルを attach する
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
LogicalInterface::attach()
{
	m_pFullTextFile = new FullTextFile(m_cFileID);
}

//
//	FUNCTION public
//	FullText2::LogicalInterface::detach -- すべてのファイルを detach する
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
LogicalInterface::detach()
{
	if (m_pFullTextFile) delete m_pFullTextFile, m_pFullTextFile = 0;
}

//
//	Copyright (c) 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
