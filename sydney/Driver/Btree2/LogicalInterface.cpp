// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogicalInterface.cpp -- 
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
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
const char	moduleName[] = "Btree2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "SyInclude.h"
#include "Btree2/LogicalInterface.h"
#include "Btree2/SimpleFile.h"
#include "Btree2/MultiFile.h"
#include "Btree2/AutoPointer.h"
#include "Btree2/Condition.h"
#include "Btree2/Parameter.h"
#include "Btree2/UniqueFile.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/IntegerArrayData.h"
#include "Common/StringArrayData.h"
#include "Common/Message.h"

#include "Checkpoint/Database.h"

#include "FileCommon/DataManager.h"
#include "FileCommon/FileOption.h"
#include "FileCommon/NodeWrapper.h"
#include "FileCommon/OpenOption.h"

#include "Exception/FileNotOpen.h"
#include "Exception/BadArgument.h"
#include "Exception/VerifyAborted.h"

_SYDNEY_USING
_SYDNEY_BTREE2_USING

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
	ParameterInteger _cMaxPageCache("Btree2_BatchMaxPageCache", 20);
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
//	Btree2::LogicalInterface::LogicalInterface -- コンストラクタ
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
	: m_cFileID(cFileID_), m_pBtreeFile(0), m_pTransaction(0),
	  m_ucFieldBitSet(0), m_bReverse(false), m_bGetByBitSet(false),
	  m_bGetMin(false), m_bGetMax(false), m_uiTupleID(_INVALID_TUPLEID),
	  m_pSearchByBitSet(0), m_iConditionCount(0), m_bValidCondition(true)

{
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::~LogicalInterface -- デストラクタ
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
//	Btree2::LogicalInterface::getFileID -- ファイルIDを返す
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
//	Btree2::LogicalInterface::getSize -- ファイルサイズを返す
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
		size = m_pBtreeFile->getSize();
	}
	return size;
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::getCount -- 挿入されているオブジェクト数を返す
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
//	Btree2::LogicalInterface::getOverhead
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
//	Btree2::LogicalInterface::getProcessCost --
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
	
	return m_pBtreeFile->getCost()
		/ (m_pBtreeFile->getPageDataSize() / m_cFileID.getTupleSize());
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::getSearchParameter
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
//	Btree2::LogicalInterface::getProjectionParameter
//		-- プロジェクションオープンパラメータを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pNode_
//		プロジェクションを表すノード
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
	bool isBitSet = false;
	if (cOpenOption_.getBoolean(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::GetByBitSet::Key)))
	{
		isBitSet = true;
	}

	// オープンオプション(参照引数)にオープンモードを設定
	// getSearchParameter()が呼ばれている場合は、そちらで設定済み
	int iValue;
	if (cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(
		FileCommon::OpenOption::OpenMode::Key), iValue) == false)
	{
		cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
			FileCommon::OpenOption::OpenMode::Key),
								FileCommon::OpenOption::OpenMode::Read);
	}

	// フィールド選択指定がされている、という事を設定する
	cOpenOption_.setBoolean(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::FieldSelect::Key),
		true);

	FileCommon::ListNodeWrapper node(pNode_);

	// フィールド選択指定を設定する
	int iFieldNum = node.getSize();
	cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
		FileCommon::OpenOption::TargetFieldNumber::Key),
							iFieldNum);

	// ビットセットの場合は取得フィールド数は1じゃないとだめ
	if (isBitSet == true && iFieldNum != 1)
		return false;

	// オープンオプションに選択されているフィールドの番号を設定
	bool isNormal = false;
	bool isSet = false;
	for (int i = 0; i < iFieldNum; ++i)
	{
		const LogicalFile::TreeNodeInterface* p = node.get(i);
		int num = -1;

		switch (p->getType())
		{
		case LogicalFile::TreeNodeInterface::Field:
			{
				if (isSet) return false;
				isNormal = true;

				num = FileCommon::DataManager::toInt(p);
				if (num <= 0 ||
					num > static_cast<int>(m_cFileID.getFieldCount()))
					return false;
			}
			break;
		case LogicalFile::TreeNodeInterface::Max:
		case LogicalFile::TreeNodeInterface::Min:
			{
				// Max/Min -- Operand -- Field
				
				if (isBitSet == true || isNormal == true)
					return false;
				if (p->getOperandSize() != 1)
					return false;
				
				const LogicalFile::TreeNodeInterface* f = p->getOperandAt(0);
				if (f->getType() != LogicalFile::TreeNodeInterface::Field)
					return false;
				int n = FileCommon::DataManager::toInt(f);
				if (n != 1)
					// 常にフィールド番号1が先頭のキー(0はOID)
					return false;

				isSet = true;
				if (p->getType() == LogicalFile::TreeNodeInterface::Min)
				{
					num = static_cast<int>(m_cFileID.getMinFieldNumber()) + 1;
				}
				else
				{
					num = static_cast<int>(m_cFileID.getMaxFieldNumber()) + 1;
				}
			}
			break;
	  default:
			return false;
		}
		
		cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
						FileCommon::OpenOption::TargetFieldIndex::Key, i), num);
	}

	return true;
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::getUpdateParameter
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
	cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
								FileCommon::OpenOption::TargetFieldNumber::Key),
							iFieldNum);

	// オープンオプションに選択されているフィールドの番号を設定
	for (int i = 0; i < iFieldNum; ++i)
	{
		int num = cProjection_.getElement(i);
		if (num <= 0 || num > static_cast<int>(m_cFileID.getFieldCount()))
			return false;
		cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
						FileCommon::OpenOption::TargetFieldIndex::Key, i), num);
	}

	return true;
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::getSortParameter -- ソート順パラメータを設定する
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
	if (cKeys_.getCount() != cOrders_.getCount())
		return false;

	// ORがあるとソートできない
	int count = cOpenOption_.getInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(Condition::Key::ConditionCount));
	if (count > 1)
		return false;

	// フィールド0はObjectIDである
	
	// B木でのソートはequals条件以降のキーフィールドの
	// ソート順が同じ場合に行える

	int maxFieldNumber = cOpenOption_.getInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(Condition::Key::EqualFieldNumber));

	int Reverse = -1;
	int size = cKeys_.getCount();
	int startPosition = 0;

	for (int i = 0; i < size; ++i)
	{
		int Field = cKeys_.getElement(i);
		int Order = cOrders_.getElement(i);

		if (Field <= 0 ||
			Field > static_cast<int>(m_cFileID.getKeyType().getSize()))
			// ObjectIDやキー以外ではソートできない
			return false;

		if (startPosition == 0)
		{
			// ソートを開始するフィール番号
			startPosition = Field;
			if (startPosition > (maxFieldNumber + 1))
				return false;
		}

		if (startPosition != Field)
			// 複数のフィールドが指定された場合はキーの順番で
			// なければならない
			return false;
		
		if (Field > maxFieldNumber)
		{
			// equals条件以降のみチェックする
			
			if (Reverse == -1)
			{
				Reverse = Order;
			}
			else if (Reverse != Order)
			{
				return false;
			}
		}

		startPosition++;
	}

	if (Reverse == 1)
	{
		cOpenOption_.setBoolean(
			_SYDNEY_OPEN_PARAMETER_KEY(Condition::Key::Reverse), true);
	}
	else
	{
		cOpenOption_.setBoolean(
			_SYDNEY_OPEN_PARAMETER_KEY(Condition::Key::Reverse), false);
	}
	
	return true;
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::create -- ファイルを生成する
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
//	Btree2::LogicalInterface::destroy -- ファイルを破棄する
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
	m_pBtreeFile->destroy(cTransaction_);
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::isAccessible --
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
	return m_pBtreeFile->isAccessible(bForce_);
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::isMounted -- マウントされているか調べる
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
	return m_pBtreeFile->isMounted(cTransaction_);
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::open -- オープンする
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
		// 参照系
		int i;
			
		// 条件数を得る
		m_iConditionCount = cOpenOption_.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(Condition::Key::ConditionCount));
		// fetchに利用するフィールド数を得る
		int fetch = cOpenOption_.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(Condition::Key::FetchFieldNumber));
		// 制約ロックのための検索か
		bool bConstraintLock = cOpenOption_.getBoolean(
			_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::GetForConstraintLock::Key));

		if (m_iConditionCount != 0)
		{
			m_vecpCondition.reserve(m_iConditionCount);
			for (i = 0; i < m_iConditionCount; ++i)
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
			pCondition->setConstraintLock(bConstraintLock);
			m_vecpCondition.pushBack(pCondition);
			m_iConditionCount = 1;
		}

		// ビットセットで得るか
		m_bGetByBitSet = cOpenOption_.getBoolean(
			_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::GetByBitSet::Key));

		if (m_bGetByBitSet == true)
		{
			// 取得フィールド
			m_iFieldID = cOpenOption_.getInteger(
				_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
					FileCommon::OpenOption::TargetFieldIndex::Key, 0)) - 1;
		}
		else
		{
			m_bGetMin = false;
			m_bGetMax = false;
			m_ucFieldBitSet = 0;
				
			// 取得するフィールド
			int count = cOpenOption_.getInteger(
				_SYDNEY_OPEN_PARAMETER_KEY(
					FileCommon::OpenOption::TargetFieldNumber::Key));

			for (i = 0; i < count; ++i)
			{
				int f = cOpenOption_.getInteger(
					_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
						FileCommon::OpenOption::TargetFieldIndex::Key, i));
				if (f ==
					(static_cast<int>(m_cFileID.getMinFieldNumber())+1))
				{
					m_bGetMin = true;
					m_ucFieldBitSet = 1;
				}
				else if (f ==
						 (static_cast<int>(m_cFileID.getMaxFieldNumber())+1))
				{
					m_bGetMax = true;
					m_ucFieldBitSet = 1;
				}
				else
				{
					m_ucFieldBitSet |= (1 << (f-1));
				}
			}
		}

		// 取得順序
		m_bReverse = cOpenOption_.getBoolean(
			_SYDNEY_OPEN_PARAMETER_KEY(Condition::Key::Reverse));

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
	else
	{
		// 更新系
		
		// 更新するフィールド
		int count = cOpenOption_.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::TargetFieldNumber::Key));

		m_vecFieldID.erase(m_vecFieldID.begin(), m_vecFieldID.end());
		for (int i = 0; i < count; ++i)
		{
			int f = cOpenOption_.getInteger(
				_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
					FileCommon::OpenOption::TargetFieldIndex::Key, i));
			m_vecFieldID.pushBack(f - 1);
		}

		m_ucFieldBitSet = 0;
		// 更新(update)の時に取得するフィールド(全部)
		int size = static_cast<int>(m_cFileID.getFieldCount());
		for (int j = 0; j < size; ++j)
		{
			m_ucFieldBitSet |= (1 << j);
		}
	}
	
	open(cTransaction_, m_eOpenMode);
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::close -- クローズする
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
	ModVector<Condition*>::Iterator i = m_vecpCondition.begin();
	for (; i != m_vecpCondition.end(); ++i)
	{
		delete *i;
	}
	if (m_pSearchByBitSet) delete m_pSearchByBitSet, m_pSearchByBitSet = 0;
	m_vecpCondition.clear();
	flushAllPages();
	m_pBtreeFile->close();
	detachFile();
	m_pTransaction = 0;
	m_iConditionCount = 0;
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::isOpened -- オープンされているか
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
	return (m_pBtreeFile && m_pTransaction) ? true : false;
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::fetch -- 検索条件を設定する
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
		// 正規化する
		const Common::DataArrayData* pUsedOption = pOption_;
		Common::DataArrayData cOption;
		if (m_cFileID.isNormalized()) {
			m_cFileID.normalize(cOption, *pOption_);
			pUsedOption = &cOption;
		}

		// fetch条件に配列があった場合は、その要素数分、
		// 検索条件を複製する

		bool isSet = false;
		bool isArray = false;
		int element = 0;
		int condition = 0;
		
		while (true)
		{
			Common::DataArrayData fetchData;
			
			int c = pOption_->getCount();
			for (int i = 0; i < c; ++i)
			{
				Common::Data::Pointer p = pOption_->getElement(i);

				if (p->isNull() == false &&
					p->getType() == Common::DataType::Array)
				{
					// 配列の場合
					
					isArray = true;
					
					Common::DataArrayData* a
						= _SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, p.get());
					
					while (element < a->getCount())
					{
						Common::Data::Pointer pp = a->getElement(element);
						if (pp->isNull() == true)
						{
							// 要素が null の場合は無視
							//
							//【注意】
							//	キー自体がnullの場合は、検索する
							
							element++;
							continue;
						}

						fetchData.setElement(i, pp);
						element++;
						break;
					}
				}
				else
				{
					// nullまたは配列ではない場合
					//
					//【注意】
					//	キー自体がnullの場合は、検索する
					
					fetchData.setElement(i, p);
				}
			}

			if (fetchData.getCount() != pOption_->getCount())
				break;

			if (isSet == true)
			{
				// 一回設定されているので、条件をコピーする
				for (int i = 0; i < m_iConditionCount; ++i)
				{
					m_vecpCondition.pushBack(m_vecpCondition[i]->copy());
				}
			}

			// 条件を設定する
			for (int i = 0; i < m_iConditionCount; ++i)
			{
				m_vecpCondition[condition++]->setFetchKey(fetchData);
			}

			// 条件は設定された
			isSet = true;
			
			if (isArray == false)
				// 配列がない場合はここで終了
				break;
		}
		
		if (isSet == false)
		{
			// 条件が設定されなかったので、無効な検索条件
			m_bValidCondition = false;
		}
	}
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::get -- オブジェクトを返す
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
		if (m_bValidCondition == false)
			// 無効な検索条件
			return result;

		if (m_bGetMin == true || m_bGetMax == true)
		{
			if (m_bFirstGet == true)
			{
				result = true;

				int n = 0;
				if (m_bGetMin == true)
					pTuple_->getElement(n++)->setNull();
				if (m_bGetMax == true)
					pTuple_->getElement(n++)->setNull();
			}
		}

		if (isMounted(*m_pTransaction))
		{
			LogicalInterface* p = 0;
			if (m_pTransaction->isNoVersion())
				p = this;
		
			_AutoDetachPage cAuto(p);
		
			if (m_bGetByBitSet == true)
			{
				if (m_bFirstGet == true)
				{
					// BitSetで得るときは専用のメソッドを実行する
					Common::Data::Pointer p = pTuple_->getElement(0);
					; _SYDNEY_ASSERT(p->getType() == Common::DataType::BitSet);
					Common::BitSet* pBitSet
						= _SYDNEY_DYNAMIC_CAST(Common::BitSet*, p.get());
					pBitSet->reset();
					m_pBtreeFile->getByBitSet(m_vecpCondition,
											  m_iFieldID, *pBitSet);
					if (m_pSearchByBitSet)
					{
						// 絞り込みのビットセットがあるので、論理積を取る
						pBitSet->operator&=(*m_pSearchByBitSet);
					}
					if (pBitSet->any())
					{
						// 結果があるときのみ
						result = true;
					}
				}
			}
			else if (m_bGetMin == true || m_bGetMax == true)
			{
				if (m_bFirstGet == true)
				{
					int n = 0;
					Common::DataArrayData cTuple;
					const ModVector<Data::Type::Value>& vecKeyType
						= m_cFileID.getKeyType();
					const ModVector<int>& vecKeyPosition
						= m_cFileID.getKeyPosition();
					int precision = m_cFileID.getInteger(
						_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
							FileCommon::FileOption::FieldLength::Key,
							vecKeyPosition[0]));
					int scale = m_cFileID.getInteger(
						_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
							FileCommon::FileOption::FieldFraction::Key,
							vecKeyPosition[0]));
				
					if (m_bGetMin == true)
					{
						// 最小値を得る
						cTuple.setElement(0, Data::makeData(
											  vecKeyType[0], precision, scale));
						m_iterator = m_vecpCondition.begin();
						m_pBtreeFile->search(*m_iterator, false);
						for (;;)
						{
							if (m_pBtreeFile->get(
									m_ucFieldBitSet, cTuple, m_uiTupleID))
							{
								if (m_pSearchByBitSet)
								{
									if (m_pSearchByBitSet->test(m_uiTupleID)
										== false)
										// ヒットしなかったので次
										continue;
								}
							
								if (cTuple.getElement(0)->isNull() == false)
								{
									// instanceを変えないためassignを使う
									pTuple_->getElement(n++)->assign(
										cTuple.getElement(0).get());
									break;
								}
								else
								{
									// Continue until data except null is
									// gotten.
								
									// Null is NOT stored when TopNull is false.
									; _TRMEISTER_ASSERT(m_cFileID.isTopNull());
								}
							}
							else
							{
								// all the data is null or data is nothing.
							
								// The default value of pTuple_ is null.
								; _TRMEISTER_ASSERT(
									pTuple_->getElement(n)->isNull() == true);
								// So below duplicate setting is unnecessary.
								// But, the closed process is more important.
								pTuple_->getElement(n++)->setNull();
								break;
							}
						}
					}
					if (m_bGetMax == true)
					{
						// 最大値を得る
						cTuple.setElement(0, Data::makeData(
											  vecKeyType[0], precision, scale));
						m_iterator = m_vecpCondition.begin();
						m_pBtreeFile->search(*m_iterator, true);
						for (;;)
						{
							if (m_pBtreeFile->get(
									m_ucFieldBitSet, cTuple, m_uiTupleID))
							{
								if (m_pSearchByBitSet)
								{
									if (m_pSearchByBitSet->test(m_uiTupleID)
										== false)
										// ヒットしなかったので次
										continue;
								}
							
								// instanceを変えないためassignを使う
								pTuple_->getElement(n++)->assign(
									cTuple.getElement(0).get());

								// Set null when the gotton data is null
								// although the default value of pTuple_ is
								// null.
								// If if-condition is added,
								// this duplicate setting can be avoided.
								// But in many case the gotton data is not null,
								// so the if-confition is unnecessary.
							}
							else
							{
								// data is noting.
								// See getting minimum.
								pTuple_->getElement(n++)->setNull();
							}
						
							break;
						}
					}

					result = true;
				}
			}
			else
			{
				if (m_bFirstGet == true)
				{
					// Search the page.

					// There is a possibility that the searched page includes
					// the tuple which satisfies the condition.
				
					if (m_cTupleBit.any() == false)
					{
						// Search the page with the begin of the conditions.
						m_iterator = m_vecpCondition.begin();
					
						// If the bitset is not empty, NOT initialize
						// m_iterator.
						// Because the conditions ahead of the current condition
						// has been used for searching.
						// See the comment of rewind() for the details.
					}
				
					m_pBtreeFile->search(*m_iterator, m_bReverse);
				}
				if (m_vecpCondition.getSize() > 1
					&& m_uiTupleID != _INVALID_TUPLEID)
				{
					// 直前のタプルIDをビットマップに格納する
					m_cTupleBit.set(m_uiTupleID);
				}
				while (result == false)
				{
					while (1)
					{
						result = m_pBtreeFile->get(m_ucFieldBitSet,
												   *pTuple_, m_uiTupleID);
						if (result == true)
						{
							if (m_pSearchByBitSet)
							{
								if (m_pSearchByBitSet->test(m_uiTupleID)
									== false)
									// ヒットしなかったので次
									continue;
							}
							
							if (m_iterator != m_vecpCondition.begin())
							{
								// すでに取得済みかチェックする
								if (m_cTupleBit.test(m_uiTupleID) == true)
									continue;
							}
						}

						break;
					}

					if (result == false)
					{
						// 他の条件があるかもしれない
						if (m_iterator != m_vecpCondition.end())
							++m_iterator;
						if (m_iterator == m_vecpCondition.end())
						{
							// もうヒットしない
							m_uiTupleID = _INVALID_TUPLEID;
							break;
						}

						m_pBtreeFile->search(*m_iterator, m_bReverse);
					}
				}
			}

			cAuto.flush();
		}
	
		m_bFirstGet = false;
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
//	Btree2::LogicalInterface::insert -- オブジェクトを挿入する
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

		LogicalInterface* p = 0;
		if (m_eOpenMode != LogicalFile::OpenOption::OpenMode::Batch)
			p = this;
		_AutoDetachPage cAuto(p);
	
		if (pTuple_->getCount() <= 1)
		{
			_SYDNEY_THROW0(Exception::BadArgument);
		}

		// B木にはObjectIDは入れないので先頭要素は削除し、正規化する
		Common::DataArrayData cTuple;
		m_cFileID.normalize(cTuple, *pTuple_, 1);

		// 挿入する
		m_pBtreeFile->insert(cTuple);

		m_pBtreeFile->dirtyHeaderPage();

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
//	Btree2::LogicalInterface::update -- オブジェクトを更新する
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
		LogicalInterface* p = 0;
		if (m_eOpenMode != LogicalFile::OpenOption::OpenMode::Batch)
			p = this;
		_AutoDetachPage cAuto(p);

		// B木の更新は削除+挿入

		// 正規化する
		Common::DataArrayData cKey;
		m_cFileID.normalize(cKey, *pKey_);

		// 検索する
		Condition cCondition(m_cFileID);
		cCondition.setFetchKey(cKey);
		m_pBtreeFile->search(&cCondition, false);
		if (m_cResult.getCount() == 0)
			m_cFileID.makeData(m_cResult);
		unsigned int dummy;
		bool result = m_pBtreeFile->get(m_ucFieldBitSet, m_cResult, dummy);
		m_pBtreeFile->detachSearchPage();

		if (result == false)
		{
			// ありえない
			_SYDNEY_THROW0(Exception::BadArgument);
		}

		// まずは削除する
		m_pBtreeFile->expunge(cKey);

		// 正規化する
		Common::DataArrayData cTuple;
		m_cFileID.normalize(cTuple, *pTuple_);

		// 次に挿入する
		for (int i = 0; i < static_cast<int>(m_vecFieldID.getSize()); ++i)
		{
		
			m_cResult.getElement(m_vecFieldID[i]).get()
				->assign(cTuple.getElement(i).get());
		}
		m_pBtreeFile->insert(m_cResult);

		m_pBtreeFile->dirtyHeaderPage();

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
//	Btree2::LogicalInterface::expunge -- オブジェクトを削除する
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

		// 正規化する
		Common::DataArrayData cKey;
		m_cFileID.normalize(cKey, *pKey_);

		// 削除する
		m_pBtreeFile->expunge(cKey);

		m_pBtreeFile->dirtyHeaderPage();

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
//	Btree2::LogicalInterface::mark -- 巻き戻しの位置を記録する
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
		m_pBtreeFile->mark();
	}
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::rewind -- 記録した位置に戻る
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
	m_uiTupleID = _INVALID_TUPLEID;

	if (m_pBtreeFile->rewind() == false)
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
//	Btree2::LogicalInterface::reset -- カーソルをリセットする
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
	m_bValidCondition = true;
	
	// fetchで配列が渡された場合、検索条件が配列の要素数文複製される
	// ここで、複製された分を削除する
	ModVector<Condition*>::Iterator c = m_vecpCondition.begin();
	ModVector<Condition*>::Iterator e = m_vecpCondition.end();
	for (int i = 0; c != m_vecpCondition.end(); ++c, ++i)
	{
		if (i == m_iConditionCount)
			e = c;
		if (i >= m_iConditionCount)
			delete *c;
	}
	m_vecpCondition.erase(e, m_vecpCondition.end());
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::equals -- 比較
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
//	Btree2::LogicalInterface::sync -- 同期をとる
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction& cTransaction_
//		トランザクションのトランザクション記述子
//	bool& bIncomplete_
//		true
//			オブジェクトの一部に処理し残しがある
//		false
//			オブジェクトを完全に処理してきている
//
//	bool& bModified_
//		true
//			オブジェクトの一部が既に更新されている
//		false
//			オブジェクトはまだ更新されていない
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
			m_pBtreeFile->sync(cTransaction_, bIncomplete_, bModified_);
		}
	}
	catch (...)
	{
		SydMessage << m_cFileID.getPath() << ModEndl;
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::compact -- 不要なデータを削除する
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction& cTransaction_
//		トランザクションのトランザクション記述子
//	bool& bIncomplete_
//		true
//			オブジェクトの一部に処理し残しがある
//		false
//			オブジェクトを完全に処理してきている
//
//	bool& bModified_
//		true
//			オブジェクトの一部が既に更新されている
//		false
//			オブジェクトはまだ更新されていない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//
void
LogicalInterface::compact(const Trans::Transaction& cTransaction_,
						  bool& bIncomplete_, bool& bModified_)
{
	try
	{
		_AutoAttachFile cAuto(*this);
		if (isMounted(cTransaction_) == true)
		{
			m_pBtreeFile->compact(cTransaction_, bIncomplete_, bModified_);
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
//	Btree2::LogicalInterface::move -- ファイルを移動する
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
	m_pBtreeFile->move(cTransaction_, cPath);
	m_cFileID.setPath(cPath);
}

// FUNCTION public
//	Btree2::LogicalInterface::getNoLatchOperation -- ラッチが不要なオペレーションを返す
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
//	Btree2::LogicalInterface::getCapability -- Capabilities of file driver
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
//	Btree2::LogicalInterface::toString --
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
//	Btree2::LogicalInterface::mount --	マウントする
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
	if (m_pBtreeFile->isMounted(cTransaction_) == false)
	{
		m_pBtreeFile->mount(cTransaction_);
		m_cFileID.setMounted(true);
	}

	return m_cFileID;
}

//	FUNCTION public
//	Btree2::LogicalInterface::unmount -- アンマウントする
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

	m_pBtreeFile->unmount(cTransaction_);
	m_cFileID.setMounted(false);

	return m_cFileID;
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::flush -- フラッシュする
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
		m_pBtreeFile->flush(cTransaction_);
	}
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::startBackup -- バックアップ開始を通知する
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
		m_pBtreeFile->startBackup(cTransaction_, bRestorable_);
	}
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::endBackup -- バックアップ終了を通知する
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
		m_pBtreeFile->endBackup(cTransaction_);
	}
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::recover -- 障害回復する
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
		m_pBtreeFile->recover(cTransaction_, cPoint_);
	}
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::restore
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
		m_pBtreeFile->restore(cTransaction_, cPoint_);
	}
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::verify -- 整合性検査を行う
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
		
		m_pBtreeFile->startVerification(cTransaction_,
										uiTreatment_, cProgress_);
		try
		{
			_AutoDetachPage cPage(this);
			m_pBtreeFile->verify();
			if (uiTreatment_ & Admin::Verification::Treatment::Correct)
				m_pBtreeFile->dirtyHeaderPage();
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
			m_pBtreeFile->endVerification();
			_SYDNEY_RETHROW;
		}
		m_pBtreeFile->endVerification();
	}
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::attachFile -- ファイルをattachする
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
	if (m_cFileID.isUseHeader())
		m_pBtreeFile = new UniqueFile(m_cFileID);
	else if (m_cFileID.isNotNull() == true)
		m_pBtreeFile = new SimpleFile(m_cFileID);
	else
		m_pBtreeFile = new MultiFile(m_cFileID);
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::detachFile -- ファイルをdetachする
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
	delete m_pBtreeFile, m_pBtreeFile = 0;
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::isAttached -- ファイルがattachされているかどうか
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
	return m_pBtreeFile ? true : false;
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::recoverAllPages -- 全ページの変更を破棄する
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
		m_pBtreeFile->flushAllPages();
	else
		m_pBtreeFile->recoverAllPages();
}

//
//	FUNCTION public
//	Btree2::LogicalInterface::flushAllPages -- 全ページの変更を確定する
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
	m_pBtreeFile->flushAllPages();
}

//
//	FUNCTION public
//	Btree2::LogicalInerface::setNotAvailable -- データベースを利用不可にする
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
//	Btree2::LogicalInterface::getCount -- 挿入されているオブジェクト数を返す
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
		ModInt64 max = m_pBtreeFile->getHeaderPage()->getCount();
		
		if (max != 0 && m_vecpCondition.getSize())
		{
			// 検索条件が与えられているので見積もる
			ModVector<Condition*>::Iterator i = m_vecpCondition.begin();
			for (; i != m_vecpCondition.end(); ++i)
			{
				count += m_pBtreeFile->getEstimateCount(*i);
			}
			// 上限を超えていたら修正
			count = (count < max) ? count : max;
		}
		else
		{
			// 全件
			count = max;
		}
	}
	return count;
}

//
//	FUNCTION private
//	Btree2::LogicalInterface::open -- オープンする
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
		m_pBtreeFile->open(cTransaction_, eOpenMode_);
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
//	Btree2::LogicalInterface::substantiate -- ファイルを本当に作成する
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
	m_pBtreeFile->create();
	m_pBtreeFile->dirtyHeaderPage();
	flushAllPages();
}

//
//	FUNCTION private
//	Btree2::LogicalInterface::detachPageForBatch
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
		
		if (m_pBtreeFile->getDirtyPageCount()
			> static_cast<ModSize>(_cMaxPageCache.get()))
		{
			// dirtyなページ数が上限を超えている
			//	-> すべてdetachする
			
			flushAllPages();
		}
	}
}

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
