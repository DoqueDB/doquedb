// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogicalInterface.cpp -- 
// 
// Copyright (c) 2006, 2007, 2008, 2009, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
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
const char	srcFile[] = __FILE__;
const char	moduleName[] = "Array";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "SyInclude.h"

#include "Array/LogicalInterface.h"
#include "Array/AutoPointer.h"
#include "Array/Condition.h"
#include "Array/Parameter.h"
#include "Array/ArrayFile.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/IntegerArrayData.h"
#include "Common/StringArrayData.h"
#include "Common/Message.h"

#include "Checkpoint/Database.h"

#include "FileCommon/OpenOption.h"

#include "Exception/FileNotOpen.h"
#include "Exception/BadArgument.h"
#include "Exception/VerifyAborted.h"

_SYDNEY_USING
_SYDNEY_ARRAY_USING

namespace
{
	//
	//	CLASS
	//	_$$::_AutoAttachFile
	//
	class _AutoAttachFile
	{
	public:
		_AutoAttachFile(LogicalInterface& cFile_)
			: m_cFile(cFile_), m_bOwner(false)
		{
			if (m_cFile.isAttached() == false)
			{
				m_cFile.attachFile();
				m_bOwner = true;
			}
		}
		~_AutoAttachFile()
		{
			if (m_bOwner) m_cFile.detachFile();
		}

	private:
		LogicalInterface& m_cFile;
		bool m_bOwner;
	};

	//
	//	CLASS
	//	_$$::_AutoDetachPage
	//
	class _AutoDetachPage
	{
	public:
		_AutoDetachPage(LogicalInterface* pFile_ = 0) : m_pFile(pFile_)
		{
		}
		~_AutoDetachPage()
		{
			if (m_pFile)
			{
				try
				{
					m_pFile->recoverAllPages();
				}
#ifdef NO_CATCH_ALL
				catch (Exception::Object&)
#else
				catch (...)
#endif
				{
					m_pFile->setNotAvailable();
					_SYDNEY_RETHROW;
				}
			}
		}

		void flush()
		{
			if (m_pFile == 0) return;
			
			try
			{
				m_pFile->flushAllPages();
			}
#ifdef NO_CATCH_ALL
			catch (Exception::Object&)
#else
			catch (...)
#endif
			{
				try
				{
					m_pFile->recoverAllPages();
				}
#ifdef NO_CATCH_ALL
				catch (Exception::Object&)
#else
				catch (...)
#endif
				{
					m_pFile->setNotAvailable();
				}
				_SYDNEY_RETHROW;
			}
		}

	private:
		LogicalInterface* m_pFile;
	};

	//
	//	LOCAL variable
	//
	const unsigned int _INVALID_TUPLEID = 0xffffffff;

	//
	//	バッチインサートの時、キャッシュするページ数の上限
	//
	ParameterInteger _cMaxPageCache("Array_BatchMaxPageCache", 20);
}

//
//	DEFINE
//	_CHECK_OPEN -- ファイルがオープンされているかどうか
//
#define _CHECK_OPEN()	\
{ \
	if (isOpened() == false) \
	{ \
		_SYDNEY_THROW0(Exception::FileNotOpen); \
	} \
}

//
//	FUNCTION public
//	Array::LogicalInterface::LogicalInterface -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cFileID_
//		可変長レコードファイルオプションオブジェクトへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
LogicalInterface::LogicalInterface(const LogicalFile::FileID& cFileID_)
	: m_cFileID(cFileID_), m_pArrayFile(0), m_pTransaction(0),
	  m_bGetByBitSet(false), m_uiTupleID(_INVALID_TUPLEID),
	  m_uiVerifyTupleID(_INVALID_TUPLEID), m_pSearchByBitSet(0)
{
}

//
//	FUNCTION public
//	Array::LogicalInterface::~LogicalInterface -- デストラクタ
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
}

//
//	FUNCTION public
//	Array::LogicalInterface::getFileID -- ファイルIDを返す
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Logical::FileID&
//		自身がもつ論理ファイル ID オブジェクトへの参照
//
//	EXCEPTIONS
//
const LogicalFile::FileID&
LogicalInterface::getFileID() const
{
	return m_cFileID;
}

//
//	FUNCTION public
//	Array::LogicalInterface::getSize -- ファイルサイズを返す
//
//	NOTES
//	ファイルサイズを返す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		レコードファイルサイズ [byte]
//
//	EXCEPTIONS
//
ModUInt64
LogicalInterface::getSize(const Trans::Transaction& cTrans_)
{
	_AutoAttachFile cAuto(*this);
	
	ModUInt64 size = 0;
	if (isMounted(cTrans_))
	{
		size = m_pArrayFile->getSize();
	}
	return size;
}

//
//	FUNCTION public
//	Array::LogicalInterface::getCount -- 挿入されているオブジェクト数を返す
//
//	NOTES
//	自身に挿入されているオブジェクトの総数を返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModInt64
//		オブジェクトの総数
//
//	EXCEPTIONS
//
ModInt64
LogicalInterface::getCount() const
{
	// なぜかconstなので、キャストする
	return const_cast<LogicalInterface*>(this)->getCount();
}

//
//	FUNCTION public
//	Array::LogicalInterface::getOverhead
//		-- オブジェクト検索時のオーバヘッドを返す
//
//	NOTES
//	オブジェクト検索時のオーバヘッドの概算を秒数で返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	double
//		オブジェクト検索時のオーバヘッド [秒]
//
//	EXCEPTIONS
//
double
LogicalInterface::getOverhead() const
{
	; _CHECK_OPEN();
	// 旧B木が1ページあたりのノード数で割ったものを返しているので、
	// それと同じように変更した。
	return getProcessCost() * 3;
}

//
//	FUNCTION public
//	Array::LogicalInterface::getProcessCost --
//		オブジェクトへアクセスする際のプロセスコストを返す
//
//	NOTES
//	自身に挿入されているオブジェクトへアクセスする際のプロセスコスト
//	を秒数で返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	double
//		プロセスコスト [秒]
//
//	EXCEPTIONS
//
double
LogicalInterface::getProcessCost() const
{
	; _CHECK_OPEN();
	
	// 1ページに格納できる最大数で割ることにした。
	// varchar等の可変長の場合、もっと格納できるけど...
	
	return m_pArrayFile->getCost()
		/ (m_pArrayFile->getPageDataSize() / m_cFileID.getTupleSize());
}

//
//	FUNCTION public
//	Array::LogicalInterface::getSearchParameter
//		-- 検索オープンパラメータを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		木構造の検索条件オブジェクトへのポインタ
//	LogicalFile::OpenOption& cOpenOption_
//		レコードファイルオープンオプションオブジェクトへの参照
//
//	RETURN
//	bool
//		引数 pCondition_ で示される検索が可能ならば true を、
//		そうでない場合には false を返す。
//
//	EXCEPTIONS
//
bool
LogicalInterface::getSearchParameter(
						const LogicalFile::TreeNodeInterface* pCondition_,
						LogicalFile::OpenOption& cOpenOption_) const
{
	Condition cCondition(m_cFileID);
	return cCondition.getSearchParameter(pCondition_, cOpenOption_);
}

//
//	FUNCTION public
//	Array::LogicalInterface::getProjectionParameter
//		-- プロジェクションオープンパラメータを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::IntegerArrayData&	cProjection_
//		フィールド番号の配列オブジェクトへの参照
//	LogicalFile::OpenOption& cOpenOption_
//		オープンオプションオブジェクトへの参照
//
//	RETURN
//	bool
//		引数 cProjection_ で示されるフィールドでオブジェクトを構成可能ならば
//		true を、そうでない場合には false を返す。
//
//	EXCEPTIONS
//
bool
LogicalInterface::getProjectionParameter(
						const Common::IntegerArrayData&	cProjection_,
						LogicalFile::OpenOption& cOpenOption_) const
{
	// Set open mode.
	// getSearchParameter()が呼ばれている場合は、そちらで設定済み

	int dummy;
	if (cOpenOption_.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::OpenMode::Key), dummy) == false)
	{
		cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
									FileCommon::OpenOption::OpenMode::Key),
								FileCommon::OpenOption::OpenMode::Read);
	}

	// Check parameters.
	if (cProjection_.getCount() != 1
		|| cProjection_.getElement(0) != FileID::FieldPosition::RowID)
	{
		return false;
	}

	// This Array Index returns only RowID.
	// Because the other fields might be NOT used in the upper modules.
	// So, NOT need to set the parameters.
	
	return true;
}

//
//	FUNCTION public
//	Array::LogicalInterface::getUpdateParameter
//		-- 更新オープンパラメータを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::IntegerArrayData&	cProjection_
//		フィールド番号の配列オブジェクトへの参照
//	LogicalFile::OpenOption& cOpenOption_
//		オープンオプションオブジェクトへの参照
//
//	RETURN
//	bool
//		更新できる場合にはtrue、それ以外の場合はfalseを返す
//
//	EXCEPTIONS
//
bool
LogicalInterface::getUpdateParameter(
						 const Common::IntegerArrayData& cProjection_,
						 LogicalFile::OpenOption& cOpenOption_) const
{
	// Set open mode.
	cOpenOption_.setInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key),
		FileCommon::OpenOption::OpenMode::Update);

	// Check the number of fields and the field.
	if (cProjection_.getCount() != 1
		|| cProjection_.getElement(0) != FileID::FieldPosition::Array)
	{
		return false;
	}

	// The updated field is only Value one.
	// When the others are updated, they are expunged and inserted.
	// So, NOT need to set the parameter.

	return true;
}

//
//	FUNCTION public
//	Array::LogicalInterface::getSortParameter -- ソート順パラメータを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::IntegerArrayData&	cKeys_
//		ソート順を指定するフィールドインデックスの列への参照
//	const Common::IntegerArrayData&	cOrders_
//		引数cKeys_で指定されたフィールドのソート順の列への参照
//		昇順ならば0を、降順ならば1を設定する。
//	LogicalFile::OpenOption& cOpenOption_
//		レコードファイルオープンオプションオブジェクトへの参照
//
//	RETURN
//	bool
//		ソートできる場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::getSortParameter(const Common::IntegerArrayData& cKeys_,
								   const Common::IntegerArrayData& cOrders_,
								   LogicalFile::OpenOption&	cOpenOption_) const
{
	// This Array Index returns only RowID.
	// In many case, the data is NOT sorted.
	return false;
}

//
//	FUNCTION public
//	Array::LogicalInterface::create -- ファイルを生成する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	const LogicalFile::FileID&
//		自身がもつ論理ファイル ID オブジェクトへの参照
//
//	EXCEPTIONS
//
const LogicalFile::FileID&
LogicalInterface::create(const Trans::Transaction& cTransaction_)
{
	m_cFileID.create();
	return m_cFileID;
}

//	FUNCTION public
//	Array::LogicalInterface::destroy -- ファイルを破棄する
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

void
LogicalInterface::destroy(const Trans::Transaction& cTransaction_)
{
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかく削除する
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	_AutoAttachFile cAuto(*this);
	m_pArrayFile->destroy(cTransaction_);
}

//
//	FUNCTION public
//	Array::LogicalInterface::isAccessible --
//		実体である OS ファイルが存在するか調べる
//
//	NOTES
//
//	ARGUMENTS
//	bool bForce_
//		強制モードかどうか
//
//	RETURN
//	bool
//		生成されているかどうか
//			true  : 生成されている
//			false : 生成されていない
//
//	EXCEPTIONS
//
bool
LogicalInterface::isAccessible(bool bForce_) const
{
	_AutoAttachFile cAuto(*const_cast<LogicalInterface*>(this));
	return m_pArrayFile->isAccessible(bForce_);
}

//
//	FUNCTION public
//	Array::LogicalInterface::isMounted -- マウントされているか調べる
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
	return m_pArrayFile->isMounted(cTransaction_);
}

//
//	FUNCTION public
//	Array::LogicalInterface::open -- オープンする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション記述子
//	const LogicalFile::OpenOption& cOpenOption_
//		オープンオプションオブジェクトへの参照
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
	// This instance is reused without destructor in prepared statements.
	// So, only m_bFirstGet=true is NOT enough to initialize the search status.
	reset();
	
	// Set open mode.
	int iValue = cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(
								FileCommon::OpenOption::OpenMode::Key));
	if (iValue == LogicalFile::OpenOption::OpenMode::Read
		|| iValue == LogicalFile::OpenOption::OpenMode::Search)
		m_eOpenMode = LogicalFile::OpenOption::OpenMode::Read;
	else if (iValue == LogicalFile::OpenOption::OpenMode::Update)
		m_eOpenMode = LogicalFile::OpenOption::OpenMode::Update;
	else if (iValue == LogicalFile::OpenOption::OpenMode::Initialize)
		m_eOpenMode = LogicalFile::OpenOption::OpenMode::Initialize;
	else if (iValue == LogicalFile::OpenOption::OpenMode::Batch)
		m_eOpenMode = LogicalFile::OpenOption::OpenMode::Batch;
	else {
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	if (m_eOpenMode == LogicalFile::OpenOption::OpenMode::Read)
	{
		// For Read Only

		// Set conditions.
		int count = cOpenOption_.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(Condition::Key::ConditionCount));
		// fetchに利用するフィールド数を得る
		int fetch = cOpenOption_.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(Condition::Key::FetchFieldNumber));
		if (count != 0)
		{
			m_vecpCondition.reserve(count);
			for (int i = 0; i < count; ++i)
			{
				Condition* pCondition = new Condition(m_cFileID);
				pCondition->setOpenOption(cOpenOption_, i);
				pCondition->setFetchField(fetch);
				m_vecpCondition.pushBack(pCondition);
			}
		}
		else
		{
			// 条件がないかfetch
			Condition* pCondition = new Condition(m_cFileID);
			pCondition->setFetchField(fetch);
			m_vecpCondition.pushBack(pCondition);
		}

		m_bGetByBitSet = cOpenOption_.getBoolean(
			_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::GetByBitSet::Key));

		// For verify
		if (cOpenOption_.getBoolean(
				_SYDNEY_OPEN_PARAMETER_KEY(Condition::Key::Verify)) == true)
		{
			m_uiVerifyTupleID =	ModUnicodeCharTrait::toUInt(
				cOpenOption_.getString(_SYDNEY_OPEN_PARAMETER_KEY(
										   Condition::Key::RowID)));
		}

		// ビットセットによる絞り込みがあるか
		const Common::Object* p = 0;
		if (cOpenOption_.getObjectPointer(
				_SYDNEY_OPEN_PARAMETER_KEY(
					FileCommon::OpenOption::SearchByBitSet::Key), p) == true)
		{
			// ビットセットによる絞り込みがある
			const Common::BitSet* pBitSet
				= _SYDNEY_DYNAMIC_CAST(const Common::BitSet*, p);
			m_pSearchByBitSet = new Common::BitSet(*pBitSet);
		}
	}
	// Not need to set anything for Update.
	
	open(cTransaction_, m_eOpenMode);
}

//
//	FUNCTION public
//	Array::LogicalInterface::close -- クローズする
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
LogicalInterface::close()
{
	// For Condition
	ModVector<Condition*>::Iterator i = m_vecpCondition.begin();
	for (; i != m_vecpCondition.end(); ++i)
	{
		delete *i;
	}
	m_vecpCondition.clear();
	if (m_pSearchByBitSet) delete m_pSearchByBitSet, m_pSearchByBitSet = 0;
		
	// For Page
	flushAllPages();

	// For File
	m_pArrayFile->close();
	detachFile();

	// For Transaction
	m_pTransaction = 0;
}

//
//	FUNCTION public
//	Array::LogicalInterface::isOpened -- オープンされているか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//	   オープンされている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::isOpened() const
{
	return (m_pArrayFile && m_pTransaction) ? true : false;
}

//
//	FUNCTION public
//	Array::LogicalInterface::fetch -- 検索条件を設定する
//
//	NOTES
//	検索条件を設定する。
//	データは get で求める。
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
	// fetch does NOT care the previous result.
	reset();

	if (isMounted(*m_pTransaction))
	{
		// Set a condition.

		ModVector<Condition*>::Iterator iterator = m_vecpCondition.begin();
		const ModVector<Condition*>::Iterator last = m_vecpCondition.end();
		for (; iterator != last; ++iterator) {
			(*iterator)->setFetchKey(*pOption_);
		}
	}
}

//
//	FUNCTION public
//	Array::LogicalInterface::get -- オブジェクトを返す
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData* pTuple_
//		値を設定するタプル
//
//	RETURN
//	bool
//		結果が得られた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::get(Common::DataArrayData* pTuple_)
{
	bool result = false;

	try
	{
		if (isMounted(*m_pTransaction))
		{
			LogicalInterface* p = 0;
			if (m_pTransaction->isNoVersion())
				p = this;
		
			_AutoDetachPage cAuto(p);

			if (m_uiVerifyTupleID != _INVALID_TUPLEID)
			{
				result = getForVerify(pTuple_);
			}
			else if (m_bGetByBitSet == true)
			{
				result = getByBitSet(pTuple_);
			}
			else
			{
				result = getSingly(pTuple_);
			}

			cAuto.flush();
		}
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
//	Array::LogicalInterface::insert -- オブジェクトを挿入する
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData*	pTuple_
//		挿入するオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::insert(Common::DataArrayData* pTuple_)
{
	try
	{
		if (isMounted(*m_pTransaction) == false)
		{
			// File has not created substantially yet.
			substantiate();
		}

		LogicalInterface* p = 0;
		if (m_eOpenMode != LogicalFile::OpenOption::OpenMode::Batch)
			p = this;
		_AutoDetachPage cAuto(p);

		// Insert the tuple
		m_pArrayFile->insert(*pTuple_);

		// [NOTE] HeaderPageはflushの直前にdirtyにする。
		//  dirtyにしてからflushするまでの間にエラーが発生すると不整合を起こす。
		//   参照: ArrayFile::recoverAllPages(), detachPage()
		//    PagePointer::operator = ()
		m_pArrayFile->dirtyHeaderPage();
		cAuto.flush();

		// Detach pages for batch.
		detachPageForBatch();
	}
	catch (...)
	{
		SydMessage << m_cFileID.getPath() << ModEndl;
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Array::LogicalInterface::update -- オブジェクトを更新する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData* pKey_
//	Common::DataArrayData*	pNewKey_
//		更新するオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::update(const Common::DataArrayData* pKey_,
						 Common::DataArrayData* pNewKey_)
{
	try
	{
		LogicalInterface* p = 0;
		if (m_eOpenMode != LogicalFile::OpenOption::OpenMode::Batch)
			p = this;
		_AutoDetachPage cAuto(p);

		// Update the Key with the Tuple.
		m_pArrayFile->update(*pKey_, *pNewKey_);

		// [NOTE] HeaderPageはflushの直前にdirtyにする。
		//  参照: LogicalInterface::insert()
		m_pArrayFile->dirtyHeaderPage();
		cAuto.flush();
	
		// バッチインサートの時、必要に応じてページをdetachする
		detachPageForBatch();
	}
	catch (...)
	{
		SydMessage << m_cFileID.getPath() << ModEndl;
		_SYDNEY_RETHROW;
	}	
}

//
//	FUNCTION public
//	Array::LogicalInterface::expunge -- オブジェクトを削除する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData*	pKey_
//		削除するオブジェクトを指定するキー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::expunge(const Common::DataArrayData* pKey_)
{
	try
	{
		LogicalInterface* p = 0;
		if (m_eOpenMode != LogicalFile::OpenOption::OpenMode::Batch)
			p = this;
		_AutoDetachPage cAuto(p);

		// Expunge the key.
		m_pArrayFile->expunge(*pKey_);

		// [NOTE] HeaderPageはflushの直前にdirtyにする。
		//  参照: LogicalInterface::insert()
		m_pArrayFile->dirtyHeaderPage();
		cAuto.flush();
	
		// Detach pages for Batch.
		detachPageForBatch();
	}
	catch (...)
	{
		SydMessage << m_cFileID.getPath() << ModEndl;
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Array::LogicalInterface::mark -- 巻き戻しの位置を記録する
//
//	NOTES
//	巻き戻しの位置を記録する。
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
void
LogicalInterface::mark()
{
	if (m_bFirstGet == false)
	{
		m_pArrayFile->mark();
	}
}

//
//	FUNCTION public
//	Array::LogicalInterface::rewind -- 記録した位置に戻る
//
//	NOTES
//	巻き戻しで記録した位置に戻る。
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
	; _SYDNEY_ASSERT(m_uiTupleID != _INVALID_TUPLEID);

	if (m_uiTupleID != _INVALID_TUPLEID)
	{
		// Initialize the previous result.
		// Array set the result just after ArrayFile::get().
		// It is different from Btree2.
		m_cTupleBit.reset(m_uiTupleID);
		m_uiTupleID = _INVALID_TUPLEID;
	}

	if (m_pArrayFile->rewind() == false)
		// rewind successes in the following case.
		// 1. Set a new condition to m_iterator and Reset a mark.
		// 2. Get the new tuple which has been not returned yet.
		// 3. Set a mark.
		// 4. Get the another new tuple.
		// 5. Rewinding happens.
		// -> The mark has been set.
		//
		// rewind FAILS in the following case.
		// 1. Set a new condition to m_iterator and Reset a mark.
		// 2. Get the new tuple which has been not returned yet.
		// 3. Rewinding happens.
		// -> The mark has NOT been set yet.
		//
		// When rewind fails, a mark is not set.
		// So search again with the current condition.

		// NOT execute reset(). Because m_cTupleBit is also reset.
		m_bFirstGet = true;
}

//
//	FUNCTION public
//	Array::LogicalInterface::reset -- カーソルをリセットする
//
//	NOTES
//	カーソルをリセットする。
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
	m_uiTupleID = _INVALID_TUPLEID;
	m_cTupleBit.reset();
	m_bFirstGet = true;
}

//
//	FUNCTION public
//	Array::LogicalInterface::equals -- 比較
//
//	NOTES
//	自身と引数 pOther_ の比較を行ない、比較結果を返す。
//	※ 同一オブジェクトかをチェックするのではなく、
//	   それぞれがもつメンバが等しいか（同値か）をチェックする。
//  ※ すべての値を比較する訳ではないことに注意。
//  
//	ARGUMENTS
//	const Common::Object* pOther_
//		比較対象オブジェクトへのポインタ
//
//	RETURN
//	bool
//		自身と引数 pOther_ が同値ならば true を、
//		そうでなければ false を返す。
//
//	EXCEPTIONS
//	なし
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
//	Array::LogicalInterface::sync -- レコードファイルの同期をとる
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction& cTransaction_
//		レコードファイルの同期を取る
//		トランザクションのトランザクション記述子
//	bool& bIncomplete_
//		true
//			今回の同期処理でレコードファイルを持つ
//			オブジェクトの一部に処理し残しがある
//		false
//			今回の同期処理でレコードファイルを持つ
//			オブジェクトを完全に処理してきている
//
//			同期処理の結果、レコードファイルを処理し残したかを設定する
//	bool& bModified_
//		true
//			今回の同期処理でレコードファイルを持つ
//			オブジェクトの一部が既に更新されている
//		false
//			今回の同期処理でレコードファイルを持つ
//			オブジェクトはまだ更新されていない
//
//			同期処理の結果、レコードファイルが更新されたかを設定する
//
//	RETURN
//		なし
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
		if (isMounted(cTransaction_) == true)
		{
			m_pArrayFile->sync(cTransaction_, bIncomplete_, bModified_);
		}
	}
	catch (...)
	{
		SydMessage << m_cFileID.getPath() << ModEndl;
		_SYDNEY_RETHROW;
	}
}

//
//	Utility
//

//	FUNCTION public
//	Array::LogicalInterface::move -- ファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション記述子への参照
//	const Common::StringArrayData& cArea_
//		移動後のレコードファイル格納ディレクトリパス構造体への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
LogicalInterface::move(const Trans::Transaction& cTransaction_,
					   const Common::StringArrayData& cArea_)
{
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかく移動する
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	_AutoAttachFile cAuto(*this);
	Os::Path cPath = cArea_.getElement(0);
	m_pArrayFile->move(cTransaction_, cPath);
	m_cFileID.setPath(cPath);
}

// FUNCTION public
//	Array::LogicalInterface::getNoLatchOperation -- ラッチが不要なオペレーションを返す
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	LogicalFile::File::Operation::Value
//
// EXCEPTIONS

LogicalFile::File::Operation::Value
LogicalInterface::
getNoLatchOperation()
{
	// 不要にできるほとんどの操作でラッチが不要
	return Operation::Open
		| Operation::Close
		| Operation::Reset
		| Operation::GetProcessCost
		| Operation::GetOverhead
		| Operation::Fetch;
}

// FUNCTION public
//	Array::LogicalInterface::getCapability -- Capabilities of file driver
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
//	Array::LogicalInterface::toString --
//		ファイルを識別するための文字列を返す
//
//	NOTES
//	ファイルを識別するための文字列を返す。
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
//	Array::LogicalInterface::mount --	マウントする
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
const LogicalFile::FileID&
LogicalInterface::mount(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);
	if (m_pArrayFile->isMounted(cTransaction_) == false)
	{
		m_pArrayFile->mount(cTransaction_);
		m_cFileID.setMounted(true);
	}

	return m_cFileID;
}

//	FUNCTION public
//	Array::LogicalInterface::unmount -- アンマウントする
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

const LogicalFile::FileID&
LogicalInterface::unmount(const Trans::Transaction& cTransaction_)
{
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかくアンマウントする
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	_AutoAttachFile cAuto(*this);

	m_pArrayFile->unmount(cTransaction_);
	m_cFileID.setMounted(false);

	return m_cFileID;
}

//
//	FUNCTION public
//	Array::LogicalInterface::flush -- フラッシュする
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
LogicalInterface::flush(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);
	if (isMounted(cTransaction_) == true)
	{
		m_pArrayFile->flush(cTransaction_);
	}
}

//
//	FUNCTION public
//	Array::LogicalInterface::startBackup -- バックアップ開始を通知する
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
//			        読取専用トランザクションの参照する版が
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
LogicalInterface::startBackup(const Trans::Transaction& cTransaction_,
							  const bool bRestorable_)
{
	_AutoAttachFile cAuto(*this);
	if (isMounted(cTransaction_) == true)
	{
		m_pArrayFile->startBackup(cTransaction_, bRestorable_);
	}
}

//
//	FUNCTION public
//	Array::LogicalInterface::endBackup -- バックアップ終了を通知する
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
LogicalInterface::endBackup(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);
	if (isMounted(cTransaction_) == true)
	{
		m_pArrayFile->endBackup(cTransaction_);
	}
}

//
//	FUNCTION public
//	Array::LogicalInterface::recover -- 障害回復する
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
LogicalInterface::recover(const Trans::Transaction& cTransaction_,
						  const Trans::TimeStamp& cPoint_)
{
	_AutoAttachFile cAuto(*this);
	if (isMounted(cTransaction_) == true)
	{
		m_pArrayFile->recover(cTransaction_, cPoint_);
	}
}

//
//	FUNCTION public
//	Array::LogicalInterface::restore
//		-- あるタイムスタンプの表す時点に開始された
//		   読取専用トランザクションの参照する版が
//		   最新版になるようにバージョンファイルを変更する
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
LogicalInterface::restore(const Trans::Transaction& cTransaction_,
						  const Trans::TimeStamp& cPoint_)
{
	_AutoAttachFile cAuto(*this);
	if (isMounted(cTransaction_) == true)
	{
		m_pArrayFile->restore(cTransaction_, cPoint_);
	}
}

//
//	FUNCTION public
//	Array::LogicalInterface::verify -- 整合性検査を行う
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション記述子への参照
//	const unsigned int uiTreatment_
//		整合性検査の検査方法
//		const Admin::Verification::Treatment::Valueを
//		const unsigned intにキャストした値
//	Admin::Verification::Progress& cProgress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::verify(const Trans::Transaction& cTransaction_,
						 const unsigned int uiTreatment_,
						 Admin::Verification::Progress& cProgress_)
{
	_AutoAttachFile cAuto(*this);
	
	if (isMounted(cTransaction_))
	{
		// オープンモードを設定
		// [NOTE] verifyは、openされずに呼ばれる。
		if (uiTreatment_ & Admin::Verification::Treatment::Correct)
		{
			m_eOpenMode = LogicalFile::OpenOption::OpenMode::Update;
		}
		else
		{
			m_eOpenMode = LogicalFile::OpenOption::OpenMode::Read;
		}
		
		m_pArrayFile->startVerification(cTransaction_,
										uiTreatment_, cProgress_);
		try
		{
			_AutoDetachPage cPage(this);
			m_pArrayFile->verify();
			if (uiTreatment_ & Admin::Verification::Treatment::Correct)
				m_pArrayFile->dirtyHeaderPage();
			cPage.flush();
		}
		catch (Exception::VerifyAborted&)
		{
			// なにもしない
			;
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			m_pArrayFile->endVerification();
			_SYDNEY_RETHROW;
		}
		m_pArrayFile->endVerification();
	}
}

//
//	FUNCTION public
//	Array::LogicalInterface::attachFile -- ファイルをattachする
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
LogicalInterface::attachFile()
{
	m_pArrayFile = new ArrayFile(m_cFileID);
}

//
//	FUNCTION public
//	Array::LogicalInterface::detachFile -- ファイルをdetachする
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
LogicalInterface::detachFile()
{
	delete m_pArrayFile, m_pArrayFile = 0;
}

//
//	FUNCTION public
//	Array::LogicalInterface::isAttached -- ファイルがattachされているかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		attachされている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::isAttached() const
{
	return m_pArrayFile ? true : false;
}

//
//	FUNCTION public
//	Array::LogicalInterface::recoverAllPages -- 全ページの変更を破棄する
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
	if (m_eOpenMode == LogicalFile::OpenOption::OpenMode::Batch)
		// バッチモードの時はページのrecoverはできない
		m_pArrayFile->flushAllPages();
	else
		m_pArrayFile->recoverAllPages();
}

//
//	FUNCTION public
//	Array::LogicalInterface::flushAllPages -- 全ページの変更を確定する
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
	m_pArrayFile->flushAllPages();
}

//
//	FUNCTION public
//	Array::LogicalInerface::setNotAvailable -- データベースを利用不可にする
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
LogicalInterface::setNotAvailable()
{
	Checkpoint::Database::setAvailability(m_cFileID.getLockName(), false);
}

//
//	FUNCTION private
//	Array::LogicalInterface::getCount -- 挿入されているオブジェクト数を返す
//
//	NOTES
//	自身に挿入されているオブジェクトの総数を返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModInt64
//		オブジェクトの総数
//
//	EXCEPTIONS
//
ModInt64
LogicalInterface::getCount()
{
	; _CHECK_OPEN();

	ModInt64 count = 0;
	if (isMounted(*m_pTransaction))
	{
		LogicalInterface* p = 0;
		if (m_pTransaction->isNoVersion())
			p = this;
		_AutoDetachPage cAuto(p);

		// 全件
		count = m_pArrayFile->getTupleCount();
		
		if (count != 0 && m_vecpCondition.getSize())
		{
			// 検索条件が与えられているので、全件を返さないかもしれない
			
			ModVector<Condition*>::Iterator i = m_vecpCondition.begin();
			if (m_vecpCondition.getSize() > 1 ||
				(*i)->isValid() == false ||
				(*i)->getTreeType() != Tree::Type::Undefined)
			{
				// [NOTE] 条件1個でvalidでTree::Type::Undefinedな条件は
				//  スキップする。open時に条件なしは全件取得を意味し、
				//  この場合のみ、このif文がfalseになる。
				//   参照: LogicalInterface::open(), Condition::setFetchField()
				
				// [NOTE] 条件1個でvalidでTree::Type::Undefined以外の条件は
				//  スキップしない。fetchやUnknownではない条件などが該当する。
				//   参照:  Condition::setFetchField(), setOpenOption(),
				//    LogicalInterface::open()
				
				// [NOTE] 条件1個でinvalidでTree::Type::Undefinedな条件は
				//  スキップしない。Unknown条件のような0件を返すものが該当する。
				//   参照: Condition::setUnknownStream(), setOpenOption()
				
				// [NOTE] 条件1個でinvalidでTree::Type::Undefined以外の条件は
				//  スキップしない。Condition::setFetchKey()でnullを設定後に
				//  この関数を呼べば該当する。(しかしおそらく呼ばれないはず)
				
				// [NOTE] Arrayの複数条件はORを想定しているので、
				//  各条件に対する見積もり結果を足し合わせて求める。
				//  したがって一部の条件がinvalidだったりしても、
				//  他の条件について調べる必要がある
				//   参照: Condition::setTreeCondition()
				
				ModUInt32 temp = 0;
				for (; i != m_vecpCondition.end(); ++i)
				{
					temp += m_pArrayFile->getEstimateCount(*i);
				}
				// 上限を超えていたら修正
				count = ModMin(count, static_cast<ModInt64>(temp));
			}
		}
	}
	return count;
}

//
//	FUNCTION private
//	Array::LogicalInterface::open -- オープンする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	LogicalFile::OpenOption::OpenMode::Value eOpenMode_
//		モード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::open(const Trans::Transaction& cTransaction_,
					   LogicalFile::OpenOption::OpenMode::Value eOpenMode_)
{
	// 引数を記憶する
	m_pTransaction = &cTransaction_;
	m_eOpenMode = eOpenMode_;

	bool owner = false;
	if (isAttached() == false)
	{
		// ファイルをattachする
		attachFile();
		owner = true;
	}

	try
	{
		// ファイルをオープンする
		m_pArrayFile->open(cTransaction_, eOpenMode_);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		if (owner == true)
		{
			// detachする
			detachFile();
		}
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION private
//	Array::LogicalInterface::substantiate -- ファイルを本当に作成する
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
LogicalInterface::substantiate()
{
	// Create the file substantially.
	m_pArrayFile->create();

	// Flush the header page.
	m_pArrayFile->dirtyHeaderPage();
	flushAllPages();
}

//
//	FUNCTION private
//	Array::LogicalInterface::detachPageForBatch
//		-- バッチモードの時、必要に応じてページをdetachする
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
LogicalInterface::detachPageForBatch()
{
	if (m_eOpenMode == LogicalFile::OpenOption::OpenMode::Batch)
	{
		// バッチモードの時だけ
		
		if (m_pArrayFile->getDirtyPageCount()
			> static_cast<ModSize>(_cMaxPageCache.get()))
		{
			// dirtyなページ数が上限を超えている
			//	-> すべてdetachする
			
			flushAllPages();
		}
	}
}

//
//	FUNCTION private
//	Array::LogicalInterface::getForVerify --
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData* pTuple_
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool
LogicalInterface::getForVerify(Common::DataArrayData* pTuple_)
{
	; _SYDNEY_ASSERT(m_uiVerifyTupleID != _INVALID_TUPLEID);
	
	if (m_bFirstGet == false)
	{
		return false;
	}
	m_bFirstGet = false;

	bool result = true;

	unsigned int index = 0;
	; _SYDNEY_ASSERT(m_vecpCondition.getSize() != 0);
	m_iterator = m_vecpCondition.begin();
	for (; m_iterator != m_vecpCondition.end(); ++m_iterator, ++index)
	{
		if (m_pArrayFile->checkEntry(*m_iterator, m_uiVerifyTupleID, index)
			== false)
		{
			result = false;
			break;
		}
	}

	if (result == true)
	{
		// Set RowID.
		Common::Data::Pointer p = pTuple_->getElement(0);
		; _SYDNEY_ASSERT(p->getType() == Common::DataType::UnsignedInteger);
		Common::UnsignedIntegerData* pUIData
			= _SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData*, p.get());
		pUIData->setValue(m_uiVerifyTupleID);
	}

	return result;
}

//
//	FUNCTION private
//	Array::LogicalInterface::getByBitSet --
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData* pTuple_
//		値を設定するタプル
//
//	RETURN
//	bool
//		結果が得られた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::getByBitSet(Common::DataArrayData* pTuple_)
{
	if (m_bFirstGet == false)
	{
		// All TupleIDs has been returned at first get.
		return false;
	}
	m_bFirstGet = false;

	// Initialize
	Common::Data::Pointer p = pTuple_->getElement(0);
	; _SYDNEY_ASSERT(p->getType() == Common::DataType::BitSet);
	Common::BitSet* pBitSet
		= _SYDNEY_DYNAMIC_CAST(Common::BitSet*, p.get());
	pBitSet->reset();

	// Get TupleIDs by BitSet.
	; _SYDNEY_ASSERT(m_vecpCondition.getSize() != 0);
	m_iterator = m_vecpCondition.begin();
	for (; m_iterator != m_vecpCondition.end(); ++m_iterator)
	{
		m_pArrayFile->search(*m_iterator);
		m_pArrayFile->getByBitSet(*pBitSet);
	}
	if (m_pSearchByBitSet)
	{
		// 絞り込みのビットセットがあるので、論理積を取る
		pBitSet->operator&=(*m_pSearchByBitSet);
	}

	// Return true when one or more TupleIDs is gotten.
	return pBitSet->any();
}

//
//	FUNCTION private
//	Array::LogicalInterface::getSingly --
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData* pData_
//		値を設定するタプル
//
//	RETURN
//	bool
//		結果が得られた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::getSingly(Common::DataArrayData* pData_)
{
	if (m_bFirstGet == true)
	{
		m_bFirstGet = false;

		// There is a possibility that the searched page includes
		// the tuple which satisfies the condition.
				
		if (m_cTupleBit.any() == false)
		{
			// Search the page with the begin of the conditions.
			; _SYDNEY_ASSERT(m_vecpCondition.getSize() != 0);
			m_iterator = m_vecpCondition.begin();
					
			// If the bitset is not empty, NOT initialize m_iterator.
			// Because the conditions ahead of the current condition
			// has been used for searching.
			// See the comment of rewind() for the details.
		}
		
		m_pArrayFile->search(*m_iterator);
	}

	bool result = false;
	while (result == false)
	{
		// Get a (new) TupleID.
		while (1)
		{
			unsigned int tupleID;
			result = m_pArrayFile->get(*pData_, tupleID);

			// [OPTIMIZE!] 条件が一つで等号条件の場合、
			// 取得済みかどうかのテストをしなくてもよい実装がある。
			// 例えば、ArrayFileで前回取得したRowIDを覚えておき、
			// 等号条件が一つの場合は、必ず異なるRowIDを返すようにする。
			
			if (result == true)
			{
				// Successed to get a TupleID.
				; _SYDNEY_ASSERT(tupleID != _INVALID_TUPLEID);
				
				if (m_pSearchByBitSet)
				{
					if (m_pSearchByBitSet->test(tupleID) == false)
						// ヒットしなかったので次
						continue;
				}
				
				if (m_cTupleBit.test(tupleID) == true)
				{
					// NOT new TupleID.
					continue;
				}

				// New TupleID!
				m_uiTupleID = tupleID;
				m_cTupleBit.set(m_uiTupleID);
			}
			else
			{
				// The TupleID is NOT set in m_cTupleBit, so initilize it.
				m_uiTupleID = _INVALID_TUPLEID;
			}
			break;
		}
		
		if (result == false)
		{
			// Failed to get a (new) TupleID with the condition.
			
			// Set next condition.
			; _SYDNEY_ASSERT(m_iterator != m_vecpCondition.end());
			if (++m_iterator == m_vecpCondition.end())
			{
				break;
			}
			m_pArrayFile->search(*m_iterator);
		}
	}
	return result;
}

//
//	Copyright (c) 2006, 2007, 2008, 2009, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
