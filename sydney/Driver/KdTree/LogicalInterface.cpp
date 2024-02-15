// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogicalInterface.cpp --
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
#include "SyDynamicCast.h"
#include "KdTree/LogicalInterface.h"

#include "KdTree/KdTreeFile.h"
#include "KdTree/MergeReserve.h"
#include "KdTree/OpenOption.h"

#include "Common/DataArrayData.h"
#include "Common/DoubleData.h"
#include "Common/FloatData.h"
#include "Common/IntegerArrayData.h"
#include "Common/IntegerData.h"
#include "Common/Message.h"
#include "Common/StringArrayData.h"
#include "Common/UnsignedIntegerData.h"

#include "LogicalFile/FileID.h"

#include "Trans/Transaction.h"

#include "Os/File.h"
#include "Os/Limits.h"

#include "Schema/File.h"

#include "FileCommon/OpenOption.h"

#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

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

	//
	//	CLASS
	//	_$$::_AutoPage
	//
	class _AutoPage
	{
	public:
		_AutoPage(LogicalInterface* pFile_)
			: m_pFile(pFile_)
			{}
		~_AutoPage()
			{
				if (m_pFile && m_pFile->isBatch() == false)
					m_pFile->recoverAllPages();
			}
		void flush()
			{
				if (m_pFile && m_pFile->isBatch() == false)
					m_pFile->flushAllPages();
			}

	private:
		LogicalInterface* m_pFile;
	};
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::LogicalInterface -- コンストラクタ
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
	: m_cFileID(cFileID_), m_pKdTreeFile(0),
	  m_eTraceType(Node::TraceType::Unknown),
	  m_iMaxCalculateCount(0),
	  m_uiLimit(0),
	  m_uiInnerPosition(0), m_pTransaction(0),
	  m_eFixMode(Buffer::Page::FixMode::Unknown),
	  m_bFirst(true), m_bBatch(false)
{
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::~LogicalInterface -- デストラクタ
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
//	KdTree::LogicalInterface::isAccessible
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
	return m_pKdTreeFile->isAccessible(force_);
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::isMounted
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
	return m_pKdTreeFile->isMounted(cTransaction_);
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::getSize -- ファイルサイズを得る
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
	ModUInt64 size = 0;
	if (isMounted(cTrans_))
		size = m_pKdTreeFile->getSize();
	return size;
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::getCount
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
	ModInt64 c = 0;
	if (isMounted(*m_pTransaction))
		c = const_cast<KdTreeFile*>(m_pKdTreeFile)->getCount();
	return c;
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::getOverhead -- オープン時のオーバヘッドを得る
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
	return const_cast<KdTreeFile*>(m_pKdTreeFile)->getOverhead();
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::getProcessCost
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
	return const_cast<KdTreeFile*>(m_pKdTreeFile)->getProcessCost();
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::getSearchParameter
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
	OpenOption cOpenOption(cOpenOption_);
	return cOpenOption.getSearchParameter(m_cFileID, pCondition_);
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::getProjectionParameter
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
	OpenOption cOpenOption(cOpenOption_);
	return cOpenOption.getProjectionParameter(m_cFileID, pNode_);
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::getUpdateParameter -- 更新パラメータを得る
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
	return m_cFileID.getUpdateParameter(cUpdateField_, cOpenOption_);
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::getSortParameter
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
	OpenOption cOpenOption(cOpenOption_);
	return cOpenOption.getSortParameter(m_cFileID, pNode_);
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::getLimitParameter -- 取得数と取得位置を設定する
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
	OpenOption cOpenOption(cOpenOption_);
	return cOpenOption.getLimitParameter(m_cFileID, cSpec_);
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::create
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
//	KdTree::LogicalInterface::destroy
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
	m_pKdTreeFile->destroy(cTransaction_);
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::mount
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

	m_pKdTreeFile->mount(cTransaction_);
	m_cFileID.setMounted(true);

	return m_cFileID;
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::unmount
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

	m_pKdTreeFile->unmount(cTransaction_);
	m_cFileID.setMounted(false);

	return m_cFileID;
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::flush
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
	m_pKdTreeFile->flush(cTransaction_);
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::startBackup
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
	m_pKdTreeFile->startBackup(cTransaction_, bRestorable_);
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::flush
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
	m_pKdTreeFile->endBackup(cTransaction_);
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::recover
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
	m_pKdTreeFile->recover(cTransaction_, cPoint_);
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::restore
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
	m_pKdTreeFile->restore(cTransaction_, cPoint_);
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::verify
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
	if (isMounted(cTransaction_))
		m_pKdTreeFile->verify(cTransaction_, eTreatment_, cProgress_);
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::open -- オープンする
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

	// オープンモードを得る
	int openMode;
	if (cOpenOption_.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key),
			openMode) == false)
	{
		// 通常ありえない
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	m_bBatch = false;
	
	switch (openMode)
	{
	case LogicalFile::OpenOption::OpenMode::Search:
		// 検索
		{
			OpenOption op(const_cast<LogicalFile::OpenOption&>(cOpenOption_));

			// 検索条件
			if (op.getCondition(m_vecCondition) == false)
				_TRMEISTER_THROW0(Exception::BadArgument);
			
			// 探索タイプ
			m_eTraceType = op.getTraceType();
			// 計算回数上限
			m_iMaxCalculateCount = op.getMaxCalculateCount();
			// 結果取得件数
			m_uiLimit = op.getSelectLimit();
			// プロジェクション
			m_vecProjection = op.getProjection();
		}
		// thru
	case LogicalFile::OpenOption::OpenMode::Read:
		// 全件検索
		m_eFixMode = Buffer::Page::FixMode::ReadOnly;
		break;

	case LogicalFile::OpenOption::OpenMode::Batch:
		m_bBatch = true;
	case LogicalFile::OpenOption::OpenMode::Update:
	default:
		// 更新
		m_eFixMode = Buffer::Page::FixMode::Write |
			Buffer::Page::FixMode::Discardable;
		break;
	}

	// openする
	m_pKdTreeFile->open(cTransaction_, m_eFixMode);
	if (m_bBatch) m_pKdTreeFile->setBatchMode();
	m_pTransaction = &cTransaction_;
	m_bFirst = true;
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::close -- クローズする
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
	if (isMounted(*m_pTransaction) && isBatch() == false)
		m_pKdTreeFile->flushAllPages();
	
	m_pKdTreeFile->close();	// バッチモードの場合はここでページがflushされる
	
	m_vecCondition.clear();
	m_vecResult.clear();
	m_uiInnerPosition = 0;
	
	m_pTransaction = 0;
	m_eFixMode = Buffer::Page::FixMode::Unknown;

	detach();
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::get -- データの取得を行う
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
	_AutoPage cAuto(m_pTransaction->isNoVersion() ? this : 0);
	
	if (pTuple_ == 0)
		_SYDNEY_THROW0(Exception::BadArgument);

	bool r = false;

	if (m_pKdTreeFile->isMounted(*m_pTransaction))
	{
		if (m_vecCondition.getSize())
		{
			if (m_bFirst)
			{
				// 最初の呼出しなので、
				// 最近傍検索を実施し、結果を設定する

				m_pKdTreeFile->nnsearch(m_vecCondition,
										m_eTraceType,
										m_iMaxCalculateCount,
										m_uiLimit,
										m_vecResult);
				
				m_bFirst = false;
				m_ite = m_vecResult.begin();
				m_uiInnerPosition = 0;
			}

			if (m_ite != m_vecResult.end())
			{
				if (m_uiInnerPosition == (*m_ite).getSize())
				{
					// 次の要素へ
					++m_ite;
					m_uiInnerPosition = 0;
				}
			}

			if (m_ite != m_vecResult.end())
			{
				int n = 0;
				ModVector<OpenOption::Projection::Value>::Iterator i
					= m_vecProjection.begin();
				for (; i < m_vecProjection.end(); ++i, ++n)
				{
					switch (*i)
					{
					case OpenOption::Projection::ROWID:
						{
							// ROWID
							
							Common::Data::Pointer p = pTuple_->getElement(n);
							if (p->getType()
								!= Common::DataType::UnsignedInteger)
								_TRMEISTER_THROW0(Exception::BadArgument);
							Common::UnsignedIntegerData& cRowID
								= _SYDNEY_DYNAMIC_CAST(
									Common::UnsignedIntegerData&, *p);
							cRowID.setValue((*m_ite)[m_uiInnerPosition].first);
						}
						break;

					case OpenOption::Projection::NeighborID:
						{
							// 検索条件の位置
							
							Common::Data::Pointer p = pTuple_->getElement(n);
							if (p->getType()
								!= Common::DataType::Integer)
								_TRMEISTER_THROW0(Exception::BadArgument);
							Common::IntegerData& cID
								= _SYDNEY_DYNAMIC_CAST(
									Common::IntegerData&, *p);
							int n = static_cast<int>(
								(m_ite - m_vecResult.begin()) + 1);
							cID.setValue(n);
						}
						break;

					case OpenOption::Projection::NeighborDistance:
						{
							// 距離の二乗
							
							Common::Data::Pointer p = pTuple_->getElement(n);
							if (p->getType()
								!= Common::DataType::Double)
								_TRMEISTER_THROW0(Exception::BadArgument);
							Common::DoubleData& cData
								= _SYDNEY_DYNAMIC_CAST(
									Common::DoubleData&, *p);
							cData.setValue((*m_ite)[m_uiInnerPosition].second);
						}
						break;
					}
				}

				++m_uiInnerPosition;
				r = true;
			}
		}
		else
		{
			// !!!!!!!!!!!! 全件検索は未実装 !!!!!!!!!!!!!!!!
			_TRMEISTER_THROW0(Exception::NotSupported);
		}
	}

	cAuto.flush();
		
	return r;
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::insert -- データの挿入を行う
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
	_AutoPage cAuto(this);
	
	if (pTuple_ == 0)
		_SYDNEY_THROW0(Exception::BadArgument);

	ModVector<float> vecKey;
	ModUInt32 uiValue;

	// [キー:floatの配列][バリュー:unsigned int]
	getKey(pTuple_, 0, vecKey);
	getValue(pTuple_, 1, uiValue);

	if (isMounted(*m_pTransaction) == false)
	{
		// 初めての挿入で、ファイルが作成されていないので、作成する
		m_pKdTreeFile->create();
	}
		
	// 挿入する
	m_pKdTreeFile->insert(uiValue, vecKey);

	cAuto.flush();

	if (!isBatch())
	{
		// 更新で版が新たに作成されたかもしれないので、
		// 版を消すジョブを投入する

		MergeReserve::pushBack(m_cFileID.getLockName(),
							   MergeReserve::Type::Discard);
	}
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::update -- データの更新を行う
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
	_AutoPage cAuto(this);
	
	// 削除してから挿入する
	
	if (pKey_ == 0 || pValue_ == 0)
		_SYDNEY_THROW0(Exception::BadArgument);

	ModVector<float> vecKey;
	ModUInt32 uiValue;

	// [キー:floatの配列][バリュー:unsigned int]
	getKey(pValue_, 0, vecKey);
	getValue(pKey_, 1, uiValue);

	if (isMounted(*m_pTransaction))
	{
		// 削除
		m_pKdTreeFile->expunge(uiValue);
	}
	else
	{
		// 更新なのにマウントされてないのはおかしいが、
		// エラーにしない
		
		SydInfoMessage << m_cFileID.getPath()
					   << " is not mounted."  << ModEndl;
		
		m_pKdTreeFile->create();
	}

	try
	{
		// [キー:floatの配列]
		getKey(pValue_, 0, vecKey);

		// 挿入
		m_pKdTreeFile->insert(uiValue, vecKey);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		try
		{
			// [キー:floatの配列][バリュー:unsigned int]
			getKey(pKey_, 0, vecKey);
			
			
			// 挿入に失敗したので、削除したものを挿入する
			m_pKdTreeFile->insert(uiValue, vecKey);
		}
		catch (...)
		{
			// エラー処理に失敗したので、利用可能性をOFFにする
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(m_cFileID.getLockName(), false);
		}
			
		_SYDNEY_RETHROW;
	}

	cAuto.flush(); 

	if (!isBatch())
	{
		// 更新で版が新たに作成されたかもしれないので、
		// 版を消すジョブを投入する

		MergeReserve::pushBack(m_cFileID.getLockName(),
							   MergeReserve::Type::Discard);
	}
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::expugne -- データの削除を行う
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
	_AutoPage cAuto(this);
	
	if (pKey_ == 0)
		_SYDNEY_THROW0(Exception::BadArgument);

	if (isMounted(*m_pTransaction) == false)
	{
		// 削除なのにマウントされていないのはおかしいが、エラーにはしない
		
		SydInfoMessage << m_cFileID.getPath()
					   << " is not mounted."  << ModEndl;
		return;
	}

	ModUInt32 uiValue;

	// [キー:floatの配列][バリュー:unsigned int]
	getValue(pKey_, 1, uiValue);

	// 削除する
	m_pKdTreeFile->expunge(uiValue);

	cAuto.flush();

	if (!isBatch())
	{
		// 更新で版が新たに作成されたかもしれないので、
		// 版を消すジョブを投入する

		MergeReserve::pushBack(m_cFileID.getLockName(),
							   MergeReserve::Type::Discard);
	}
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::fetch -- 検索条件を設定する
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
//	KdTree::LogicalInterface::mark -- 巻き戻し位置を記録する
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
//	KdTree::LogicalInterface::rewind -- 巻き戻す
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
//	KdTree::LogicalInterface::reset -- カーソルをリセットする
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
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::equals -- 同じかどうか比較する
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
//	KdTree::LogicalInterface::sync
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
	_AutoAttachFile cAuto(*this);
	if (isMounted(cTransaction_))
		m_pKdTreeFile->sync(cTransaction_, bIncomplete_, bModified_);
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::move -- ファイルを移動する
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
	Os::Path cNewPath = cArea_.getElement(0);	// 先頭要素のみ利用
	Os::Path cOrgPath = m_cFileID.getPath();
	int r = Os::Path::compare(cNewPath, cOrgPath);
	if (r == Os::Path::CompareResult::Identical)
	{
		// 同じパスなので、何もしない
		return;
	}
	
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかく移動する
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	bool accessible = (isAccessible() &&
					   r == Os::Path::CompareResult::Unrelated);
	
	_AutoAttachFile cAuto(*this);
	
	try
	{
		m_pKdTreeFile->move(cTransaction_, cNewPath);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		if (accessible)
		{
			// 存在を確認し、あれば削除する
			if (Os::Directory::access(cNewPath, Os::Directory::AccessMode::File)
				== true)
			{
				// 存在するので、削除する
				Os::Directory::remove(cNewPath);
			}
		}

		_TRMEISTER_RETHROW;
	}
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::getNoLatchOperation
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
//	KdTree::LogicalInterface::getCapability -- 
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
//	KdTree::LogicalInterface::attach -- ファイルを attach する
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
	m_pKdTreeFile = new KdTreeFile(m_cFileID);
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::detach -- すべてのファイルを detach する
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
	if (m_pKdTreeFile)
	{
		delete m_pKdTreeFile, m_pKdTreeFile = 0;
	}
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::flushAllPages
//		-- すべてのファイルのページを確定する
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
LogicalInterface::flushAllPages()
{
	m_pKdTreeFile->flushAllPages();
}

//
//	FUNCTION public
//	KdTree::LogicalInterface::recoverAllPages
//		-- すべてのファイルのページを破棄する
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
LogicalInterface::recoverAllPages()
{
	m_pKdTreeFile->recoverAllPages();
}

//
//	FUNCTION private
//	KdTree::LogicalInterface::getKey -- キーを取り出す
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData* pTuple_
//		タプル
//	int iElement_
//		要素番号
//	ModVector<float>& vecKey_
//		取り出したキー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::getKey(const Common::DataArrayData* pTuple_,
						 int iElement_,
						 ModVector<float>& vecKey_)
{
	Common::Data::Pointer p = pTuple_->getElement(iElement_);
	if (p->getType() != Common::DataType::Array ||
		p->getElementType() != Common::DataType::Data)
		_SYDNEY_THROW0(Exception::BadArgument);

	const Common::DataArrayData& d
		= _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&, *(p.get()));

	int n = d.getCount();
	
	vecKey_.clear();
	vecKey_.reserve(n);

	for (int i = 0; i < n; ++i)
	{
		float f = 0;
		Common::Data::Pointer pp = d.getElement(i);
		switch (pp->getType())
		{
		case Common::DataType::UnsignedInteger:
			{
				const Common::UnsignedIntegerData& dd
					= _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData&,
										   *(pp.get()));
				f = static_cast<float>(dd.getValue());
			}
			break;
		case Common::DataType::Integer:
			{
				const Common::IntegerData& dd
					= _SYDNEY_DYNAMIC_CAST(const Common::IntegerData&,
										   *(pp.get()));
				f = static_cast<float>(dd.getValue());
			}
			break;
		case Common::DataType::Float:
			{
				const Common::FloatData& dd
					= _SYDNEY_DYNAMIC_CAST(const Common::FloatData&,
										   *(pp.get()));
				f = dd.getValue();
			}
			break;
		case Common::DataType::Double:
			{
				const Common::DoubleData& dd
					= _SYDNEY_DYNAMIC_CAST(const Common::DoubleData&,
										   *(pp.get()));
				f = static_cast<float>(dd.getValue());
			}
			break;
		default:
			_SYDNEY_THROW0(Exception::BadArgument);
		}

		vecKey_.pushBack(f);
	}
}

//
//	FUNCTION private
//	KdTree::LogicalInterface::getValue -- バリューを取り出す
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData* pTuple_
//		タプル
//	int iElement_
//		要素番号
//	ModUInt32& uiValue_
//		取り出したバリュー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::getValue(const Common::DataArrayData* pTuple_,
						   int iElement_,
						   ModUInt32& uiValue_)
{
	Common::Data::Pointer p = pTuple_->getElement(iElement_);
	if (p->getType() != Common::DataType::UnsignedInteger)
		_SYDNEY_THROW0(Exception::BadArgument);

	const Common::UnsignedIntegerData& v
		= _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData&, *(p.get()));

	uiValue_ = v.getValue();
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
