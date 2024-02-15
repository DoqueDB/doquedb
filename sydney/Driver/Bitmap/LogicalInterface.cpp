// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogicalInterface.cpp -- 
// 
// Copyright (c) 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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
const char	moduleName[] = "Bitmap";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "SyInclude.h"
#include "Bitmap/LogicalInterface.h"
#include "Bitmap/NormalBitmapFile.h"
#include "Bitmap/CompressedBitmapFile.h"
#include "Bitmap/Condition.h"
#include "Bitmap/OpenOption.h"
#include "Bitmap/SearchGroupBy.h"
#include "Bitmap/QueryNode.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/IntegerArrayData.h"
#include "Common/StringArrayData.h"
#include "Common/BitSet.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/Message.h"

#include "Checkpoint/Database.h"

#include "FileCommon/OpenOption.h"

#include "Exception/BadArgument.h"
#include "Exception/FileNotOpen.h"
#include "Exception/NotSupported.h"
#include "Exception/VerifyAborted.h"

#include "ModUnicodeCharTrait.h"
#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

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
//	Bitmap::LogicalInterface::LogicalInterface -- コンストラクタ
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
	: m_cFileID(cFileID_), m_pBitmapFile(0), m_pTransaction(0),
	  m_eOpenMode(LogicalFile::OpenOption::OpenMode::Unknown),
	  m_uiRowID(0), m_pNarrowingBitSet(0),
	  m_bGetByBitSet(false), m_bFirstGet(true), m_bVerify(false),
	  m_bGroupBy(false), m_bIsAsc(true), m_bGetKey(false)
{
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::~LogicalInterface -- デストラクタ
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
//	Bitmap::LogicalInterface::getFileID -- ファイルIDを返す
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
//	Bitmap::LogicalInterface::getSize -- ファイルサイズを返す
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
		size = m_pBitmapFile->getSize();
	}
	return size;
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::getCount -- 挿入されているオブジェクト数を返す
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
	; _CHECK_OPEN();
	ModInt64 count = 0;
	if (isMounted(*m_pTransaction))
	{
		LogicalInterface* p = 0;
		if (m_pTransaction->isNoVersion())
			p = const_cast<LogicalInterface*>(this);
		
		_AutoDetachPage cAuto(p);

		if (m_cstrCondition.getLength())
		{
			// 結果件数を見積もる

			count = const_cast<LogicalInterface*>(this)->getEstimateCount();
		}
		else
		{
			// 全件
		
			count = const_cast<BitmapFile*>(m_pBitmapFile)
				->getHeaderPage().getCount();
		}
	}
	return count;
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::getOverhead
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

	// B木と同じように、3ページ参照するが、
	// B木部分の1ページあたりのノード数で割ったものを返す
	
	return (m_pBitmapFile->getCost()
			/ (m_pBitmapFile->getPageDataSize() / m_cFileID.getTupleSize()))
		* 3;
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::getProcessCost --
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

	// 1ページには最大でページサイズ x 8 格納できるが、
	// キーが１つということはないので、その 1/8 とする
	
	return m_pBitmapFile->getCost()
		/ (m_pBitmapFile->getPageDataSize() * 8 / 8);
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::getSearchParameter
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
	OpenOption cOpenOption(m_cFileID, cOpenOption_);
	return cOpenOption.getSearchParameter(pCondition_);
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::getProjectionParameter
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
	const LogicalFile::TreeNodeInterface* pNode_,
	LogicalFile::OpenOption& cOpenOption_) const
{
	OpenOption cOpenOption(m_cFileID, cOpenOption_);
	return cOpenOption.getProjectionParameter(pNode_);
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::getUpdateParameter
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
	// オープンオプション(参照引数)にオープンモードを設定
	cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
								FileCommon::OpenOption::OpenMode::Key),
							FileCommon::OpenOption::OpenMode::Update);

	// フィールド選択指定がされている、という事を設定する
	cOpenOption_.setBoolean(_SYDNEY_OPEN_PARAMETER_KEY(
							FileCommon::OpenOption::FieldSelect::Key), true);

	// フィールド選択指定を設定する
	int iFieldNum = cProjection_.getCount();
	if (iFieldNum != 1)
		return false;
	
	cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
								FileCommon::OpenOption::TargetFieldNumber::Key),
							iFieldNum);

	// オープンオプションに選択されているフィールドの番号を設定
	for (int i = 0; i < iFieldNum; ++i)
	{
		int num = cProjection_.getElement(i);
		if (num != 0)
			// キーしか更新できない
			return false;

		cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
						FileCommon::OpenOption::TargetFieldIndex::Key, i), num);
	}

	return true;
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::getSortParameter -- ソート順パラメータを設定する
//
//	NOTES
//	GroupByのソートキーが、主キーと一致した時のみ、
//	パラメータを設定してtrueを返す。
//  ただし、B主キーがNoPadの場合は、falseを返す
// 
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pNode_
//		ソート条件を指定したTreeNodeInterface
//	LogicalFile::OpenOption& cOpenOption_
//		オープンオプションオブジェクトへの参照
//
//	RETURN
//	bool
//		ソートできる場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::getSortParameter(
	const LogicalFile::TreeNodeInterface*	pNode_,
	LogicalFile::OpenOption&				cOpenOption_) const
{
	OpenOption cOpenOption(m_cFileID, cOpenOption_);
	return cOpenOption.getSortParameter(pNode_);
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::create -- ファイルを生成する
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
//	Bitmap::LogicalInterface::destroy -- ファイルを破棄する
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
	m_pBitmapFile->destroy(cTransaction_);
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::isAccessible --
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
	return m_pBitmapFile->isAccessible(bForce_);
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::isMounted -- マウントされているか調べる
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
	return m_pBitmapFile->isMounted(cTransaction_);
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::open -- オープンする
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
	m_bFirstGet = true;
	// オープンモードを設定
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
		// 条件を得る
		m_cstrCondition = cOpenOption_.getString(
			_SYDNEY_OPEN_PARAMETER_KEY(OpenOption::Key::Condition));

		// verifyか?
		m_bVerify = cOpenOption_.getBoolean(
			_SYDNEY_OPEN_PARAMETER_KEY(OpenOption::Key::Verify));
			
		// ビットセットで得るか
		m_bGetByBitSet = cOpenOption_.getBoolean(
			_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::GetByBitSet::Key));

		// group by かどうか？
		m_bGroupBy = cOpenOption_.getBoolean(
			_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::GroupBy::Key));
			
		if (m_bGroupBy)
		{
			int order = 0;
			if (cOpenOption_.getInteger(
					_SYDNEY_OPEN_PARAMETER_KEY(OpenOption::Key::SortOrder),
					order) == true)
			{
				if (order == 0)
					m_bIsAsc = true;
				else
					m_bIsAsc = false;
			}
		}
	
		if (m_bVerify == true)
		{
			// verify である
			m_uiRowID = ModUnicodeCharTrait::toUInt(
				cOpenOption_.getString(
					_SYDNEY_OPEN_PARAMETER_KEY(OpenOption::Key::RowID)));
		}

		const Common::Object* p = 0;
		if (cOpenOption_.getObjectPointer(
				_SYDNEY_OPEN_PARAMETER_KEY(
					FileCommon::OpenOption::SearchByBitSet::Key), p)
			== true)
		{
			// ビットセットによる絞り込みがある

			m_pNarrowingBitSet
				= _SYDNEY_DYNAMIC_CAST(const Common::BitSet*, p);
		}

		// 取得フィールドをチェックする
		int count = cOpenOption_.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::TargetFieldNumber::Key));

		for (int i = 0; i < count; ++i)
		{
			int f = cOpenOption_.getInteger(
				_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
					FileCommon::OpenOption::TargetFieldIndex::Key, i));

			if (m_bGroupBy)
			{
				if (f == 0)
				{
					// キー値も取得する
					m_bGetKey = true;
				}
				else if (f != 1)
				{
					// ROWIDしか取得できない
					_SYDNEY_THROW0(Exception::BadArgument);
				}
			}
			else
			{
				if (f != 1)
				{
					// ROWIDしか取得できない
					_SYDNEY_THROW0(Exception::BadArgument);
				}
			}
		}
	}
	else
	{
		// 更新系
	}
	
	open(cTransaction_, m_eOpenMode);
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::close -- クローズする
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
	m_pBitmapFile->flushAllPages();
	m_pBitmapFile->close();
	detachFile();

	// 変数をopen前の状態と同じにする
	
	m_pTransaction = 0;
	m_eOpenMode = LogicalFile::OpenOption::OpenMode::Unknown;
	m_cstrCondition.clear();
	m_cBitSet.reset();
	m_uiRowID = 0;
	m_pNarrowingBitSet = 0;
	m_cBitSetMap.erase(m_cBitSetMap.begin(), m_cBitSetMap.end());
	
	m_bGetByBitSet = false;
	m_bFirstGet = true;
	m_bVerify = false;
	m_bGroupBy = false;
	m_bIsAsc = true;
	m_bGetKey = false;
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::isOpened -- オープンされているか
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
	return (m_pBitmapFile && m_pTransaction) ? true : false;
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::fetch -- 検索条件を設定する
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
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::get -- オブジェクトを返す
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
			if (m_bFirstGet == true)
			{
				_AutoDetachPage cAuto(this);
			
				if (m_bGroupBy)
				{
					// group by の検索
					searchGroupBy();
				}
				else if (m_bVerify)
				{
					// verify の検索
					searchVerify();
				}
				else
				{
					// 普通の検索
					Common::BitSet* pBitSet = 0;
					if (m_bGetByBitSet)
					{
						Common::Data::Pointer p = pTuple_->getElement(0);
						; _SYDNEY_ASSERT(p->getType()
										 == Common::DataType::BitSet);
						pBitSet	=
							_SYDNEY_DYNAMIC_CAST(Common::BitSet*, p.get());

						// GetByBitSetの場合は最初の一回目だけ結果を返す
						result = true;
					}

					searchNormal(pBitSet);
				}
				cAuto.flush();
			
				m_bFirstGet = false;
			}

			if (m_bGroupBy)
			{
				if (m_bIsAsc)
				{
					if (m_bitIte != m_cBitSetMap.end())
					{
						// タプルにデータを設定する
						setTupleGroupBy(pTuple_);
						result = true;
					
						// 次へ
						++m_bitIte;
					}
				}
				else
				{
					if (m_bitIte != m_cBitSetMap.begin())
					{
						// 次へ
						--m_bitIte;
					
						// タプルにデータを設定する
						setTupleGroupBy(pTuple_);
						result = true;
					}
				}
			}
			else if (m_bGetByBitSet == false)
			{
				if (m_ite != m_cBitSet.end())
				{
					// 値を設定する
					Common::Data::Pointer p = pTuple_->getElement(0);
					if (p->getType() != Common::DataType::UnsignedInteger)
					{
						_SYDNEY_THROW0(Exception::BadArgument);
					}
				
					Common::UnsignedIntegerData* pRowID
						= _SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData*,
											   p.get());
			
					// ヒットした
					pRowID->setValue(*m_ite);
					++m_ite;
					result = true;
				}
			}
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
//	Bitmap::LogicalInterface::insert -- オブジェクトを挿入する
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
			// 作成遅延でまだファイルが作成されていない
			substantiate();
		}

		_AutoDetachPage cAuto(this);
	
		if (pTuple_->getCount() <= 1)
		{
			_SYDNEY_THROW0(Exception::BadArgument);
		}

		// 挿入する
		m_pBitmapFile->insert(*pTuple_);

		cAuto.flush();
	}
	catch (...)
	{
		SydMessage << m_cFileID.getPath() << ModEndl;
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::update -- オブジェクトを更新する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData* pKey_
//		更新するオブジェクトを指定するオブジェクトID
//	Common::DataArrayData*	pObject_
//		更新するオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::update(const Common::DataArrayData* pKey_,
						 Common::DataArrayData* pTuple_)
{
	try
	{
		_AutoDetachPage cAuto(this);

		// Bitmapの更新は削除+挿入

		// 削除
		m_pBitmapFile->expunge(*pKey_);
	
		// 挿入
		Common::DataArrayData cTuple;
		cTuple.reserve(2);
		cTuple.setElement(0, pTuple_->getElement(0));
		cTuple.setElement(1, pKey_->getElement(1));
		m_pBitmapFile->insert(cTuple);

		cAuto.flush();
	}
	catch (...)
	{
		SydMessage << m_cFileID.getPath() << ModEndl;
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::expunge -- オブジェクトを削除する
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
		_AutoDetachPage cAuto(this);

		// 削除する
		m_pBitmapFile->expunge(*pKey_);

		cAuto.flush();
	}
	catch (...)
	{
		SydMessage << m_cFileID.getPath() << ModEndl;
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::mark -- 巻き戻しの位置を記録する
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
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::rewind -- 記録した位置に戻る
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
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::reset -- カーソルをリセットする
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
	m_bFirstGet = true;
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::equals -- 比較
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
//	Bitmap::LogicalInterface::sync -- レコードファイルの同期をとる
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
			m_pBitmapFile->sync(cTransaction_, bIncomplete_, bModified_);
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
//	Bitmap::LogicalInterface::move -- ファイルを移動する
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
	m_pBitmapFile->move(cTransaction_, cPath);
	m_cFileID.setPath(cPath);
}

//	FUNCTION public
//	Bitmap::LogicalInterface::getNoLatchOperation
//		-- ラッチが不要なオペレーションを返す
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	LogicalFile::File::Operation::Value
//
//	EXCEPTIONS

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

//	FUNCTION public
//	Bitmap::LogicalInterface::getCapability -- Capabilities of file driver
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	LogicalFile::File::Capability::Value
//
//	EXCEPTIONS

LogicalFile::File::Capability::Value
LogicalInterface::
getCapability()
{
	// 条件から件数を見積もることができる
	return Capability::EstimateCount;
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::toString --
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
//	Bitmap::LogicalInterface::mount --	マウントする
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
	if (m_pBitmapFile->isMounted(cTransaction_) == false)
	{
		m_pBitmapFile->mount(cTransaction_);
		m_cFileID.setMounted(true);
	}

	return m_cFileID;
}

//	FUNCTION public
//	Bitmap::LogicalInterface::unmount -- アンマウントする
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

	m_pBitmapFile->unmount(cTransaction_);
	m_cFileID.setMounted(false);

	return m_cFileID;
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::flush -- フラッシュする
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
		m_pBitmapFile->flush(cTransaction_);
	}
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::startBackup -- バックアップ開始を通知する
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
		m_pBitmapFile->startBackup(cTransaction_, bRestorable_);
	}
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::endBackup -- バックアップ終了を通知する
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
		m_pBitmapFile->endBackup(cTransaction_);
	}
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::recover -- 障害回復する
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
//	[YET!]
//
void
LogicalInterface::recover(const Trans::Transaction& cTransaction_,
						  const Trans::TimeStamp& cPoint_)
{
	_AutoAttachFile cAuto(*this);
	if (isMounted(cTransaction_) == true)
	{
		m_pBitmapFile->recover(cTransaction_, cPoint_);
	}
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::restore
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
		m_pBitmapFile->restore(cTransaction_, cPoint_);
	}
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::verify -- 整合性検査を行う
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
//	[YET!]
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
		
		m_pBitmapFile->startVerification(cTransaction_,
										uiTreatment_, cProgress_);
		try
		{
			_AutoDetachPage cPage(this);
			m_pBitmapFile->verify();
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
			m_pBitmapFile->endVerification();
			_SYDNEY_RETHROW;
		}
 
		m_pBitmapFile->endVerification();
	}
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::attachFile -- ファイルをattachする
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
	if (m_cFileID.isCompressed())
		m_pBitmapFile = new CompressedBitmapFile(m_cFileID);
	else
		m_pBitmapFile = new NormalBitmapFile(m_cFileID);
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::detachFile -- ファイルをdetachする
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
	delete m_pBitmapFile, m_pBitmapFile = 0;
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::isAttached -- ファイルがattachされているかどうか
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
	return m_pBitmapFile ? true : false;
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::recoverAllPages -- 全ページの変更を破棄する
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
	m_pBitmapFile->recoverAllPages();
}

//
//	FUNCTION public
//	Bitmap::LogicalInterface::flushAllPages -- 全ページの変更を確定する
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
	m_pBitmapFile->flushAllPages();
}

//
//	FUNCTION public
//	Bitmap::LogicalInerface::setNotAvailable -- データベースを利用不可にする
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
//	Bitmap::LogicalInterface::open -- オープンする
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
		m_pBitmapFile->open(cTransaction_, eOpenMode_);
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
//	Bitmap::LogicalInterface::substantiate -- ファイルを本当に作成する
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
	m_pBitmapFile->create();
	flushAllPages();
}

//
//	FUNCTION private
//	Bitmap::LogicalInterface::searchNormal -- 通常のgetのための検索
//
//	NOTES
//
//	ARGUMENTS
//	Common::BitSet* pBitSet_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::searchNormal(Common::BitSet* pBitSet_)
{
	Condition cCondition(m_cFileID);

	ModAutoPointer<QueryNode> pNode
		= cCondition.createQueryNode(m_cstrCondition,
									 *m_pBitmapFile,
									 m_bVerify);

	// 検索結果を格納するビットセット
	Common::BitSet* pBitSet = (pBitSet_ != 0) ? pBitSet_ : &m_cBitSet;

	// 中身をクリアする
	pBitSet->reset();

	// ビットセットを取得する
	
	ModUInt32 uiMaxRowID = m_pBitmapFile->getHeaderPage().getMaxRowID();
	bool result = pNode->get(pBitSet, uiMaxRowID, m_pNarrowingBitSet);

	if (pBitSet_ == 0)
	{
		// ビットセットで取得しない場合には、イテレータに設定する
		m_ite = pBitSet->begin();
	}
}

//
//	FUNCTION private
//	Bitmap::LogicalInterface::searchVerify -- verifyのgetのための検索
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
LogicalInterface::searchVerify()
{
	Condition cCondition(m_cFileID);

	ModAutoPointer<QueryNode> pNode
		= cCondition.createQueryNode(m_cstrCondition,
									 *m_pBitmapFile,
									 m_bVerify);

	m_cBitSet.reset();

	// verify の equal 検索なので、
	// これから取得する Common::BitSet 上のポジションに移動する

	ModUInt32 pos = m_uiRowID / (Common::BitSet::UNIT_SIZE * 8);
	pNode->seek(pos);	// ここはUnitType単位
	
	// Common::BitSet::UnitType ごとに
	// ビットセットを取り出す
	Common::BitSet::UnitType unit = pNode->next();

	// 一時的なBitSetに代入する
	if (unit.none() == false)
	{
		m_cBitSet.insertUnitType(pos, unit);
	}
				
	m_ite = m_cBitSet.begin();
}

//
//	FUNCTION private
//	Bitmap::LogicalInterface::searchGroupBy -- group byのgetのための検索
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
LogicalInterface::searchGroupBy()
{
	if (m_cstrCondition.getLength() != 0)
	{
		// 検索条件があるので、通常の検索を実施する
		searchNormal(0);

		// 絞り込み条件に検索結果を設定する
		m_pNarrowingBitSet = &m_cBitSet;
	}

	// 格納されている最大のROWID
	ModUInt32 uiMaxRowID = m_pBitmapFile->getHeaderPage().getMaxRowID();
	// そのROWIDのUnitポジション
	ModUInt32 last = uiMaxRowID / (Common::BitSet::UNIT_SIZE * 8);
	
	// 空の条件クラスを作り、B木を検索する(全件検索することになる)
	Condition cCondition(m_cFileID);
	m_pBitmapFile->search(&cCondition);

	// イテレータを、それが尽きるまで取出し、
	// 絞り込み条件と論理積を取って、集合がある場合に配列に格納する
	// 並列処理可能なので、OpenMPで並列処理を行う
	
	SearchGroupBy cSearchGroupBy(m_cFileID,
								 m_pBitmapFile,
								 last,
								 m_pNarrowingBitSet,
								 m_cBitSetMap);
	cSearchGroupBy.run();

	if (m_bIsAsc)
		m_bitIte = m_cBitSetMap.begin();
	else
		m_bitIte = m_cBitSetMap.end();
}

//
//	FUNCTION private
//	Bitmap::LogicalInterface::getEstimateCount -- 検索結果件数見積り
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//
//	EXCEPTIONS
//
ModSize
LogicalInterface::getEstimateCount()
{
	Condition cCondition(m_cFileID);

	ModAutoPointer<QueryNode> pNode
		= cCondition.createQueryNode(m_cstrCondition,
									 *m_pBitmapFile,
									 false);

	// 検索結果件数見積り
	ModUInt32 uiMaxRowID = m_pBitmapFile->getHeaderPage().getMaxRowID();
	return pNode->getEstimateCount(uiMaxRowID);
}

//
//	FUNCTION private
//	Bitmap::LogicalInterface::setTupleGroupBy
//		-- group byのデータをタプルに設定する
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData* pTuple_
//		データを設定するタプル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::setTupleGroupBy(Common::DataArrayData* pTuple_)
{
	int i = 0;
	
	if (m_bGetKey)
	{
		// キーを assign する
		Common::Data::Pointer p = pTuple_->getElement(i++);
		p->assign((*m_bitIte).first.get());
	}

	Common::Data::Pointer p = pTuple_->getElement(i);
	; _SYDNEY_ASSERT(p->getType() == Common::DataType::BitSet);
	Common::BitSet* pBitSet = _SYDNEY_DYNAMIC_CAST(Common::BitSet*, p.get());

	// ビットセットを assign する
	pBitSet->assign((*m_bitIte).second.get());
}

//
//	Copyright (c) 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
