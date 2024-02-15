// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogicalInterface.cpp -- 
// 
// Copyright (c) 2005, 2006, 2007, 2008, 2009, 2011, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Vector2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "Vector2/LogicalInterface.h"
#include "Vector2/SimpleFile.h"
#include "Vector2/Types.h"
#include "Vector2/Condition.h"

#include "Checkpoint/Database.h"
#include "Common/BitSet.h"
#include "Common/IntegerData.h"
#include "Common/Integer64Data.h"
#include "Common/IntegerArrayData.h"
#include "Common/Message.h"
#include "Common/StringArrayData.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/UnsignedIntegerArrayData.h"
#include "Exception/BadArgument.h"
#include "Exception/FileNotOpen.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"
#include "Exception/VerifyAborted.h"
#include "FileCommon/OpenOption.h"
#include "FileCommon/DataManager.h"
#include "FileCommon/NodeWrapper.h"
#include "LogicalFile/Estimate.h"
#include "LogicalFile/TreeNodeInterface.h"

#include "ModUnicodeCharTrait.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_VECTOR2_BEGIN

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
					m_pFile->detachAllPages();
				}
				catch (...)
				{
					m_pFile->setNotAvailable();
					_SYDNEY_RETHROW;
				}
			}
		}

	private:
		LogicalInterface* m_pFile;
	};

	//
	//	LOCAL variable
	//
	// Types.hでIllegalKeyを定義する。SimpleFile, Condition等でも使うため。
	//const unsigned int _INVALID_TUPLEID = 0xffffffff;
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
//	Vector2::LogicalInterface::LogicalInterface -- コンストラクタ
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
	: m_cFileID(cFileID_), m_pVectorFile(0),
	  m_pTransaction(0), m_uiFieldCount(0),
	  m_bGetKey(false), m_bReverse(false), m_bGetByBitSet(false),
	  m_bGetCount(false), m_bFetch(false), m_bFirstGet(true),
	  m_uiKeyID(IllegalKey), m_uiMarkID(IllegalKey),
	  m_uiMinKeyID(IllegalKey), m_uiMaxKeyID(IllegalKey),
	  m_uiConditionID(0)
{
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::~LogicalInterface -- デストラクタ
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
//	Schema Information
//

//
//	FUNCTION public
//	Vector2::LogicalInterface::getFileID -- ファイルIDを返す
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
//	Vector2::LogicalInterface::getSize -- ファイルサイズを返す
//
//	NOTES
//	[?] Vectorファイルは一つのVectorが一つのファイル？
//	(ページはサイズが決まっているが、ファイルは決まっていない。)
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		ベクタファイルサイズ [byte]
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
		size = m_pVectorFile->getSize();
	}
	return size;
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::getCount
//		-- 挿入されているオブジェクト数を返す
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
		count = const_cast<VectorFile*>(m_pVectorFile)->getCount();
	}
	return count;
}

	//
	//	Query Optimization
	//

//
//	FUNCTION public
//	Vector2::LogicalInterface::getOverhead
//		-- オブジェクト検索時のオーバヘッドを返す
//
//	NOTES
//	概算を秒数で返すが、常に0.0。
//	keyからオブジェクトが格納されているアドレスを直接得られるため。
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
	return 0.0; // 常に 0.0 を返す
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::getProcessCost
//		-- オブジェクトへアクセスする際のプロセスコストを返す
//
//	NOTES
//	自身に挿入されているオブジェクトへアクセスする際のプロセスコスト
//	を秒数で返す。
//	[?] ヘッダーページの読み込みは無視して、
//		オブジェクトへのアクセスだけのコストを返す？
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

	if (!isMounted(*m_pTransaction))
		return 0.0;

	if  (m_eOpenMode == LogicalFile::OpenOption::OpenMode::Read)
	{
		// 読み込むバイト数（フィールドサイズの合計）を求める。
		double cost = static_cast<double>(m_cFileID.getFieldSize());
		// 1秒間にファイルからメモリに転送できる(バイト数)で割る。
		cost /= LogicalFile::Estimate::getTransferSpeed(
			LogicalFile::Estimate::File);
		return cost;
	}

	return 0.0;
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::getSearchParameter
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
	using namespace LogicalFile;

	if (pCondition_ == 0)
	{
		//
		// SCAN モード
		//

		// 条件の個数
		cOpenOption_.setInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(OpenOptionKey::ConditionSize), 1);

		// 最小値と最大値
		// ModUInt32はサポートされていないのでModInt64をつかう
		cOpenOption_.setLongLong(
			_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
				OpenOptionKey::MinKeyIndex, 0),
			static_cast<ModInt64>(0));
		cOpenOption_.setLongLong(
			_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
				OpenOptionKey::MaxKeyIndex, 0),
			static_cast<ModInt64>(IllegalKey));

		// オープンオプション
		setOpenMode(cOpenOption_, FileCommon::OpenOption::OpenMode::Read);

		return true;
	}
	else if (pCondition_->getType() == TreeNodeInterface::Fetch)
	{
		//
		// FETCH モード
		// （ fetch の引数でオブジェクト ID を指定するモード）
		//

		// オープンオプション
		
		// 第一要素は、Fetchされるカラムリスト
		// 第二要素は、未使用
		if (pCondition_->getOptionSize() != 2)
			_SYDNEY_THROW0(Exception::BadArgument);
		const TreeNodeInterface* pFetchedFields
			= pCondition_->getOptionAt(0);
		// カラムリストにはキーのみ
		// また、カラムリストは自身をオペランドとして扱う
		if (pFetchedFields->getOperandSize() != 1)
			return false;
		const TreeNodeInterface* pField
			= pFetchedFields->getOperandAt(0);
		if (pField->getType() != TreeNodeInterface::Field ||
			ModUnicodeCharTrait::toInt(pField->getValue()) != 0)
			return false;

		setOpenMode(cOpenOption_, FileCommon::OpenOption::OpenMode::Read);
		
		return true;
	}
	else
	{
		//
		// SEARCHモード
		//
		
		Condition condition(pCondition_);

		if (condition.parseRoot())
		{
			// 解析できた

			const int size = condition.getConditionSize();
			cOpenOption_.setInteger(
				_SYDNEY_OPEN_PARAMETER_KEY(OpenOptionKey::ConditionSize),
				size);

			for (int i = 0; i < size; ++i)
			{
				cOpenOption_.setLongLong(
					_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
						OpenOptionKey::MinKeyIndex, i),
					static_cast<ModInt64>(condition.getMin(i)));
				cOpenOption_.setLongLong(
					_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
						OpenOptionKey::MaxKeyIndex, i),
					static_cast<ModInt64>(condition.getMax(i)));
			}

			// オープンオプション
			setOpenMode(cOpenOption_, FileCommon::OpenOption::OpenMode::Read);

			return true;
		}
	}
	
	return false;
}


//
//	FUNCTION public
//	Vector2::LogicalInterface::getProjectionParameter
//		-- プロジェクションオープンパラメータを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pNode_
//		フィールドを指定するノード
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
	using namespace LogicalFile;
	
	// オープンオプション
	setOpenMode(cOpenOption_, FileCommon::OpenOption::OpenMode::Read);

	// BitSetで取得するかどうか
	bool isBitSet = cOpenOption_.getBoolean(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::GetByBitSet::Key));

	// フィールド指定がされている、という事を設定
	cOpenOption_.setBoolean(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::FieldSelect::Key),
		true);

	FileCommon::ListNodeWrapper node(pNode_);

	// 指定フィールド個数を設定
	int iFieldNum = node.getSize();
	cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
								FileCommon::OpenOption::TargetFieldNumber::Key),
							iFieldNum);

	// フィールド番号を設定
	for (int i = 0; i < iFieldNum; ++i)
	{
		const TreeNodeInterface* pOperand = node.get(i);

		if (pOperand->getType() == TreeNodeInterface::Field)
		{
			int num = FileCommon::DataManager::toInt(pOperand);

			if (isBitSet == true && num != 0)
				// ビットセットで取得できるのはキーのみ
				return false;

			// フィールドの番号を設定する
			cOpenOption_.setInteger(
				_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
					FileCommon::OpenOption::TargetFieldIndex::Key, i), num);
		}
		else if (pOperand->getType() == TreeNodeInterface::Count)
		{
			if (isBitSet == true)
				// 仮想フィールドはビットセットで取得できない
				return false;
			if (iFieldNum != 1)
				// 仮想フィールドは他のフィールドと同時取得できない
				return false;

			cOpenOption_.setBoolean(
				_SYDNEY_OPEN_PARAMETER_KEY(OpenOptionKey::Count), true);
		}
		else
		{
			return false;
		}
	}

	return true;
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::getUpdateParameter
//		-- 更新オープンパラメータを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::IntegerArrayData&	cUpdateFields_
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
	const Common::IntegerArrayData&	cUpdateFields_,
	LogicalFile::OpenOption& cOpenOption_) const
{
	// この関数はupdateモード専用
	//	今のところは使われない。

	// オープンオプション
	setOpenMode(cOpenOption_, FileCommon::OpenOption::OpenMode::Update);
	
	// フィールドが指定されていることを設定
	cOpenOption_.setBoolean(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::FieldSelect::Key),
		true);
	
	// 指定フィールド個数を設定
	const int iFieldNum = cUpdateFields_.getCount();
	if (iFieldNum <= 0)
		return false;
	cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
								FileCommon::OpenOption::TargetFieldNumber::Key),
							iFieldNum);
	
	// フィールド番号を設定
	for (int i = 0; i < iFieldNum; ++i)
	{
		cOpenOption_.setInteger(
			_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
				FileCommon::OpenOption::TargetFieldIndex::Key, i),
			cUpdateFields_.getElement(i));
	}

	return true;
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::getSortParameter -- ソート順パラメータを設定する
//
//	NOTES
//	[?]ソートされているから関係ない？
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
LogicalInterface::getSortParameter(
	const Common::IntegerArrayData& cKeys_,
	const Common::IntegerArrayData& cOrders_,
	LogicalFile::OpenOption&	cOpenOption_) const
{
	bool result = false;
	
	if (cKeys_.getCount() == 1 && cKeys_.getCount() == cOrders_.getCount())
	{
		if (cKeys_.getElement(0) == 0)
		{
			// キーによるソートは可能
			if (cOrders_.getElement(0) == 0)
			{
				cOpenOption_.setBoolean(
					_SYDNEY_OPEN_PARAMETER_KEY(OpenOptionKey::Reverse), false);
			}
			else
			{
				cOpenOption_.setBoolean(
					_SYDNEY_OPEN_PARAMETER_KEY(OpenOptionKey::Reverse), true);
			}

			result = true;
		}
	}

	return result;
}


	//
	//	Data Manipulation
	//

//
//	FUNCTION public
//	Vector2::LogicalInterface::create -- 
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
//	Vector2::LogicalInterface::destroy -- ファイルを破棄する
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
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかく削除する
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない
	
	_AutoAttachFile cAuto(*this);
	m_pVectorFile->destroy(cTransaction_);
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::isAccessible --
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
	return m_pVectorFile->isAccessible(bForce_);
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::isMounted -- マウントされているか調べる
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
	return m_pVectorFile->isMounted(cTransaction_);
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::open -- オープンする
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
	// オープンモード
	int iValue = cOpenOption_.getInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key));
	if (iValue == LogicalFile::OpenOption::OpenMode::Read ||
		iValue == LogicalFile::OpenOption::OpenMode::Search)
		m_eOpenMode = LogicalFile::OpenOption::OpenMode::Read;
	else if (iValue == LogicalFile::OpenOption::OpenMode::Update)
		m_eOpenMode = LogicalFile::OpenOption::OpenMode::Update;
	else if (iValue == LogicalFile::OpenOption::OpenMode::Initialize)
		m_eOpenMode = LogicalFile::OpenOption::OpenMode::Initialize;
	else if (iValue == LogicalFile::OpenOption::OpenMode::Batch)
		m_eOpenMode = LogicalFile::OpenOption::OpenMode::Batch;
	else
		_SYDNEY_THROW0(Exception::BadArgument);

	if (m_eOpenMode == LogicalFile::OpenOption::OpenMode::Read)
	{
		//
		// 参照系
		//

		if (cOpenOption_.getBoolean(
				_SYDNEY_OPEN_PARAMETER_KEY(
					FileCommon::OpenOption::Estimate::Key)) == false)
		{
			// Estimateではないので条件を設定する
			// getCount, getProcessCost, getOverhead は、
			// Estimate=trueで呼ばれる(ことがある)が、
			// ここで設定される条件は使われない。
			
			m_bFirstGet = true;

			int size = 0;
			if (cOpenOption_.getInteger(
					_SYDNEY_OPEN_PARAMETER_KEY(
						OpenOptionKey::ConditionSize), size))
			{
				// 条件が設定されている

				if (size == 1)
				{
					// 条件が一つだけ
				
					ModInt64 k = 0;
					if (cOpenOption_.getLongLong(
							_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
								OpenOptionKey::MinKeyIndex, 0), k))
						m_uiMinKeyID = static_cast<ModUInt32>(k);
					if (cOpenOption_.getLongLong(
							_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
								OpenOptionKey::MaxKeyIndex, 0), k))
						m_uiMaxKeyID = static_cast<ModUInt32>(k);
				}
				else if (size > 1)
				{
					// 条件が複数
					m_vecMinKeyID.reserve(static_cast<ModSize>(size));
					m_vecMaxKeyID.reserve(static_cast<ModSize>(size));
					for (int i = 0; i != size; ++i)
					{
						ModInt64 k = 0;
						if (cOpenOption_.getLongLong(
								_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
									OpenOptionKey::MinKeyIndex, i), k))
							m_vecMinKeyID.pushBack(static_cast<ModUInt32>(k));
						if (cOpenOption_.getLongLong(
								_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
									OpenOptionKey::MaxKeyIndex, i), k))
							m_vecMaxKeyID.pushBack(static_cast<ModUInt32>(k));
					}
				}
				else
					_SYDNEY_THROW0(Exception::Unexpected);
			}

			// ビットセットを取得するかどうか
			m_bGetByBitSet = cOpenOption_.getBoolean(
				_SYDNEY_OPEN_PARAMETER_KEY(
					FileCommon::OpenOption::GetByBitSet::Key));

			// 取得するフィールドの個数
			int count = cOpenOption_.getInteger(
				_SYDNEY_OPEN_PARAMETER_KEY(
					FileCommon::OpenOption::TargetFieldNumber::Key));

			if (count >= Data::MaxFieldCount)
				_SYDNEY_THROW0(Exception::BadArgument);
			else if (count <= 0)
			{
				// 個数が設定されていない場合は、全フィールドを取得する
				count = static_cast<int>(m_cFileID.getFieldCount()) + 1;
				m_bGetKey = true;
				for (int i = 0; i < count; ++i)
					m_pFieldID[i] = i;
			}
			else
			{
				m_bGetKey = false;
				for (int i = 0; i < count; ++i)
				{
					// 取得するフィールド番号を得る
					m_pFieldID[i] = cOpenOption_.getInteger(
						_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
							FileCommon::OpenOption::TargetFieldIndex::Key, i));
				
					if (m_pFieldID[i] == 0)
						// キーを取得する
						m_bGetKey = true;
				}
			}
			m_uiFieldCount = static_cast<ModSize>(count);

			// 取得順序
			m_bReverse = cOpenOption_.getBoolean(
				_SYDNEY_OPEN_PARAMETER_KEY(OpenOptionKey::Reverse));

			// 件数取得
			m_bGetCount = cOpenOption_.getBoolean(
				_SYDNEY_OPEN_PARAMETER_KEY(OpenOptionKey::Count));
		}
	}
	else
	{
		//
		// 更新系
		//
		
		// 更新するフィールドの個数
		int count = cOpenOption_.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::TargetFieldNumber::Key));
		if (count > Data::MaxFieldCount)
			_SYDNEY_THROW0(Exception::BadArgument);
		m_uiFieldCount = static_cast<ModSize>(count);

		for (int i = 0; i < count; ++i)
		{
			// 更新するフィールド番号を得る
			m_pFieldID[i] = cOpenOption_.getInteger(
				_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
					FileCommon::OpenOption::TargetFieldIndex::Key, i));
		}
	}

	open(cTransaction_, m_eOpenMode);
}


//
//	FUNCTION public
//	Vector2::LogicalInterface::close -- クローズする
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
	m_pVectorFile->close();
	detachFile();
	m_pTransaction = 0;

	// 検索条件の初期化
	// デストラクタが呼ばれずに次の検索条件が呼ばれることもある。
	// ModVector以外はopen時に上書きされるのでほっとく。
	m_vecMinKeyID.erase(m_vecMinKeyID.begin(), m_vecMinKeyID.end());
	m_vecMaxKeyID.erase(m_vecMaxKeyID.begin(), m_vecMaxKeyID.end());
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::isOpened -- オープンされているか
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
	return (m_pVectorFile && m_pTransaction) ? true : false;
}
	
//
//	FUNCTION public
//	Vector2::LogicalInterface::fetch -- 検索条件を設定する
//
//	NOTES
//  検索条件 (オブジェクトID) を設定する
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
	m_bFirstGet = true;
	m_bFetch = true;
	if (isMounted(*m_pTransaction))
	{
		Common::Data::Pointer p = pOption_->getElement(0);
		m_uiKeyID = _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData&, *p)
			.getValue();
	}
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::get -- オブジェクトを返す
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
//		カウントは0件でもtrue。
//		ビットセットは0件だとfalse。
//		フェッチやスキャンは取得できなければfalse。
//
//	EXCEPTIONS
//
bool
LogicalInterface::get(Common::DataArrayData* pTuple_)
{
	bool result = false;

	try
	{
		if (m_bFirstGet == false && m_uiKeyID == IllegalKey)
			// もう取得しきったのでこれ以上は取得できない
			return false;
	
		if (m_bGetCount)
		{
			// カウントのみを取得する

			if (pTuple_->getCount() != 1 ||
				pTuple_->getElement(0)->getType()
				!= Common::DataType::UnsignedInteger)
			{
				// エラー
				_SYDNEY_THROW0(Exception::BadArgument);
			}

			m_bFirstGet = false;

			ModSize count = 0;
			if (isMounted(*m_pTransaction))
			{
				// [OPTIMIZE!] Similar to LogicalInterface::getCount.
				LogicalInterface* p = 0;
				if (m_pTransaction->isNoVersion())
					p = this;
				_AutoDetachPage cAuto(p);
				// カウントを取得する
				count = m_pVectorFile->getCount();
			}

			// カウントを設定する
			_SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData&,
								 *pTuple_->getElement(0)).setValue(count);

			// 一度しか取得できない
			m_uiKeyID = IllegalKey;
			result = true;
		}
		else if (isMounted(*m_pTransaction))
		{
			LogicalInterface* p = 0;
			if (m_pTransaction->isNoVersion())
				p = this;
			_AutoDetachPage cAuto(p);

			if (m_bGetByBitSet)
			{
				// ビットセットで、キーを得る
				result = getByBitSet(pTuple_);
			}
			else
			{
				// キーも取得する時のための一時保存場所
				ModUInt32 key;			

				if (m_bFetch)
				{
					// fetch

					m_bFirstGet = false;

					if (m_pVectorFile->
						fetch(m_uiKeyID, *pTuple_, m_pFieldID, m_uiFieldCount)
						== false)
					{
						m_uiKeyID = IllegalKey;
						return false;
					}

					// キーも取得するかもしれない
					key = m_uiKeyID;

					// 一度しか取得できない
					m_uiKeyID = IllegalKey;
					result = true;
				}
				else if (m_bReverse)
				{
					// 降順のscan
					if ((result = getInDescending(pTuple_, key)) == false)
						return false;
				}
				else
					// 昇順のscan
					if ((result = getInAscending(pTuple_, key)) == false)
						return false;

				if (m_bGetKey)
				{
					// キーも取得する
					Common::Data::Pointer p = pTuple_->getElement(0);
					_SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData&, *p)
						.setValue(key);
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
//	Vector2::LogicalInterface::insert -- オブジェクトを挿入する
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData*	pTuple_
//		挿入するオブジェクト
//		pTuple_に想定されているデータ
//		[0]		key
//		[1]		value
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

		Common::Data::Pointer p = pTuple_->getElement(0);
		if (p->getType() != Common::DataType::UnsignedInteger)
			_SYDNEY_THROW0(Exception::BadArgument);
		const Common::UnsignedIntegerData& cKey
			= _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData&, *p);

		// batch mode does not detach page here
		_AutoDetachPage cAuto(
			(m_eOpenMode == LogicalFile::OpenOption::OpenMode::Batch)
			? 0 : this);
		// 挿入する
		m_pVectorFile->insert(cKey.getValue(), *pTuple_);
	}
	catch (...)
	{
		SydMessage << m_cFileID.getPath() << ModEndl;
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::update -- オブジェクトを更新する
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
		Common::Data::Pointer p = pKey_->getElement(0);
		if (p->getType() != Common::DataType::UnsignedInteger)
			_SYDNEY_THROW0(Exception::BadArgument);
		const Common::UnsignedIntegerData& cKey
			= _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData&, *p);

		_AutoDetachPage cAuto(this);
		// 更新する
		m_pVectorFile->update(cKey.getValue(), *pTuple_,
							  m_pFieldID, m_uiFieldCount);
	}
	catch (...)
	{
		SydMessage << m_cFileID.getPath() << ModEndl;
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::expunge -- オブジェクトを削除する
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
		Common::Data::Pointer p = pKey_->getElement(0);
		if (p->getType() != Common::DataType::UnsignedInteger)
			_SYDNEY_THROW0(Exception::BadArgument);
		const Common::UnsignedIntegerData& cKey
			= _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData&, *p);
	
		_AutoDetachPage cAuto(this);
		// 削除する
		m_pVectorFile->expunge(cKey.getValue());
	}
	catch (...)
	{
		SydMessage << m_cFileID.getPath() << ModEndl;
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::mark -- 巻き戻しの位置を記録する
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
		m_uiMarkID = m_uiKeyID;
	}
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::rewind -- 記録した位置に戻る
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
	m_uiKeyID = m_uiMarkID;
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::reset -- カーソルをリセットする
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
//	Vector2::LogicalInterface::equals -- 比較
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
//	Vector2::LogicalInterface::sync -- レコードファイルの同期をとる
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
			m_pVectorFile->sync(cTransaction_, bIncomplete_, bModified_);
		}
	}
	catch (...)
	{
		SydMessage << m_cFileID.getPath() << ModEndl;
		_SYDNEY_RETHROW;
	}
}

	//
	// Utiliry
	//

//	FUNCTION public
//	Vector2::LogicalInterface::move -- ファイルを移動する
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
	m_pVectorFile->move(cTransaction_, cPath);
	m_cFileID.setPath(cPath);
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::getNoLatchOperation
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
//
LogicalFile::File::Operation::Value
LogicalInterface::getNoLatchOperation()
{
	// 不要にできるほとんどの操作でラッチが不要
	return Operation::Open
		| Operation::Close
		| Operation::Reset
		| Operation::GetProcessCost
		| Operation::GetOverhead
		| Operation::Fetch;
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::toString --
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
	// 運用管理のためのメソッド
	//

//
//	FUNCTION public
//	Vector2::LogicalInterface::mount --	マウントする
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
	if (m_pVectorFile->isMounted(cTransaction_) == false)
	{
		m_pVectorFile->mount(cTransaction_);
		m_cFileID.setMounted(true);
	}

	return m_cFileID;
}

//	FUNCTION public
//	Vector2::LogicalInterface::unmount -- アンマウントする
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
LogicalInterface::unmount(const Trans::Transaction& cTransaction_)
{
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかくアンマウントする
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	_AutoAttachFile cAuto(*this);

	m_pVectorFile->unmount(cTransaction_);
	m_cFileID.setMounted(false);

	return m_cFileID;
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::flush -- フラッシュする
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
		m_pVectorFile->flush(cTransaction_);
	}
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::startBackup -- バックアップ開始を通知する
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
		m_pVectorFile->startBackup(cTransaction_, bRestorable_);
	}
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::endBackup -- バックアップ終了を通知する
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
		m_pVectorFile->endBackup(cTransaction_);
	}
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::recover -- 障害回復する
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
		m_pVectorFile->recover(cTransaction_, cPoint_);
	}
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::restore
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
		m_pVectorFile->restore(cTransaction_, cPoint_);
	}
}

	//
	// 整合性検査のためのメソッド
	//

//
//	FUNCTION public
//	Vector2::LogicalInterface::verify -- 整合性検査を行う
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
	// openされずに呼ばれる

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
		
		m_pVectorFile->startVerification(cTransaction_,
										 uiTreatment_, cProgress_);
		try
		{
			_AutoDetachPage cPage(this);
			m_pVectorFile->verify();
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
			m_pVectorFile->endVerification();
			_SYDNEY_RETHROW;
		}
		m_pVectorFile->endVerification();
	}
}

	// 以下publicであるが、外部には公開していないメソッド

//
//	FUNCTION public
//	Vector2::LogicalInterface::attachFile -- ファイルをattachする
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
	m_pVectorFile = new SimpleFile(m_cFileID);
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::detachFile -- ファイルをdetachする
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
	delete m_pVectorFile, m_pVectorFile = 0;
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::isAttached
//		-- ファイルがattachされているかどうか
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
	return m_pVectorFile ? true : false;
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::detacheAllPages
//		-- すべてのページをdetachする
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
LogicalInterface::detachAllPages()
{
	m_pVectorFile->detachAllPages();
}

//
//	FUNCTION public
//	Vector2::LogicalInterface::setNotAvailable -- 利用不可にする
//
//	NOTES
//	_AutoDetachPageのエラー処理に利用。
//	[?] ここでセットした値はどこで使われる？
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
//	Vector2::LogicalInterface::open -- オープンする
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
LogicalInterface::open(
	const Trans::Transaction& cTransaction_,
	LogicalFile::OpenOption::OpenMode::Value eOpenMode_)
{
	m_pTransaction = &cTransaction_;
	m_eOpenMode = eOpenMode_;

	bool owner = false;
	if (isAttached() == false)
	{
		attachFile();
		owner = true;
	}

	try
	{
		m_pVectorFile->open(cTransaction_, eOpenMode_);
	}
	catch (...)
	{
		if (owner == true)
		{
			detachFile();
		}
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION private
//	Vector2::LogicalInterface::substantiate -- ファイルを本当に作成する
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
	m_pVectorFile->create();
}

//
//	FUNCTION private
//	Vector2::LogicalInterface::getConditionSize -- 条件の個数を取得する
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	int
//		条件の個数
//
//	EXCEPTIONS
//
int
LogicalInterface::getConditionSize() const
{
	const int i = static_cast<int>(m_vecMinKeyID.getSize());

	if (i == 0)
		return 1;
	else
		return i;
}

//
//	FUNCTION private
//	Vector2::LogicalInterface::getMin -- 条件の最小値を取得する
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	ModUInt32
//		条件の最小値
//
//	EXCEPTIONS
//
ModUInt32
LogicalInterface::getMin() const
{
	if (getConditionSize() == 1)
		return m_uiMinKeyID;
	else
		return m_vecMinKeyID.at(m_uiConditionID);
}

//
//	FUNCTION private
//	Vector2::LogicalInterface::getMax -- 条件の最大値を取得する
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	ModUInt32
//		条件の最大値
//
//	EXCEPTIONS
//
ModUInt32
LogicalInterface::getMax() const
{
	if (getConditionSize() == 1)
		return m_uiMaxKeyID;
	else
		return m_vecMaxKeyID.at(m_uiConditionID);
}

//
//	FUNCTION private
//	Vector2::LogicalInterface::setOpenMode -- オープンオプションを設定する
//
//	NOTES
//
//	ARGUMENTS
//	LogicalFile::OpenOption& cOpenOption_
//		設定されるオープンオプションの参照
//	int mode_
//		設定するオープンモード
//
//	RETURN
//
//	EXCEPTIONS
//
void
LogicalInterface::setOpenMode(LogicalFile::OpenOption& cOpenOption_,
							  int mode_) const
{
	int iValue;
	if (cOpenOption_.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key),
			iValue))
	{
		// オープンオプションに既にオープンモードが設定されていた場合
		if (iValue != mode_)
			// 指定されたオープンモード以外は例外。
			_SYDNEY_THROW0(Exception::BadArgument);
	}
	else
	{
		// 設定されていなかった場合
		cOpenOption_.setInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key),
			mode_);
	}
}

//
//	FUNCTION private
//	Vector2::LogicalInterface::getInAscending -- 昇順でオブジェクトを取得する
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData* pTuple_
//		値を設定するタプル
//	ModUInt32& key_
//		取得したキー
//
//	RETURN
//
//	EXCEPTIONS
//
bool
LogicalInterface::getInAscending(Common::DataArrayData* pTuple_,
								 ModUInt32& key_)
{
	bool result = false;
	ModUInt32 min;
	ModUInt32 max;

	if (m_bFirstGet == true)
	{
		// 初めて
		m_bFirstGet = false;

		// 条件に、解析された条件の先頭条件を設定する
		m_uiConditionID = 0;

		min = getMin();
		max = getMax();

		if (min == IllegalKey)
			// 0件取得条件なので1件も取得できない
			return false;
		else if (min == 0)
			m_uiKeyID = IllegalKey;
		else
		{
			if (m_pVectorFile->isValid(min) == false)
			{
				return false;
			}
			m_uiKeyID = min - 1;
		}
	}
	else
	{
		min = getMin();
		max = getMax();
	}

	while ((m_uiKeyID = m_pVectorFile->
			next(m_uiKeyID, *pTuple_, m_pFieldID, m_uiFieldCount))
		   != IllegalKey)
	{
		bool update = false;
		while (m_uiKeyID > max)
		{
			if (++m_uiConditionID !=
				static_cast<ModUInt32>(getConditionSize()))
			{
				// 条件を更新する
				update = true;
				max = getMax();
			}
			else
			{
				// 次の条件がないので抜ける。もう次は取得できない。
				m_uiKeyID = IllegalKey;
				return false;
			}
		}

		if (update == false || (min = getMin()) <= m_uiKeyID)
		{
			result = true;
			break;
		}
		else
			// nextの探索開始地点を更新する
			m_uiKeyID = min - 1;
	}
	if (m_uiKeyID == IllegalKey)
		return false;
				
	// キーも取得するかもしれない
	key_ = m_uiKeyID;

	if (key_ == max && m_uiConditionID ==
		static_cast<ModUInt32>(getConditionSize() - 1))
	{
		// 次のデータは存在しないので、次は取得できない
		m_uiKeyID = IllegalKey;
	}

	return result;
}

//
//	FUNCTION private
//	Vector2::LogicalInterface::getInDescending -- 降順でオブジェクトを取得する
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData* pTuple_
//		値を設定するタプル
//	ModUInt32& key_
//		取得したキー
//
//	RETURN
//
//	EXCEPTIONS
//
bool
LogicalInterface::getInDescending(Common::DataArrayData* pTuple_,
								  ModUInt32& key_)
{
	bool result = false;
	ModUInt32 min;
	ModUInt32 max;

	if (m_bFirstGet == true)
	{
		// 初めて
		m_bFirstGet = false;

		// 条件に、解析された条件の末尾条件を設定する
		m_uiConditionID = getConditionSize() - 1;

		min = getMin();
		max = getMax();

		if (min == IllegalKey)
			// 0件取得条件なので1件も取得できない
			return false;
		if (max != IllegalKey)
		{
			if (m_pVectorFile->isValid(max) == false)
			{
				return false;
			}
			m_uiKeyID = max + 1;
		}
	}
	else
	{
		min = getMin();
		max = getMax();
	}

	
	while ((m_uiKeyID = m_pVectorFile->
			prev(m_uiKeyID, *pTuple_, m_pFieldID, m_uiFieldCount))
		   != IllegalKey)
	{
		bool update = false;
		while (m_uiKeyID < min)
		{
			if (m_uiConditionID != 0)
			{
				// 条件を更新する
				update = true;
				--m_uiConditionID;
				min = getMin();
			}
			else
			{
				// 次の条件がないので抜ける。もう次は取得できない。
				m_uiKeyID = IllegalKey;
				return false;
			}
		}

		if (update == false || (max = getMax()) >= m_uiKeyID)
		{
			result = true;
			break;
		}
		else
			// nextの探索開始地点を更新する
			m_uiKeyID = max + 1;
	}
	if (m_uiKeyID == IllegalKey)
		return false;
	
	// キーも取得するかもしれない
	key_ = m_uiKeyID;

	if (key_ == min && m_uiConditionID == 0)
		// 次のデータは存在しないので、次は取得できない
		m_uiKeyID = IllegalKey;

	return  result;
}

//
//	FUNCTION private
//	Vector2::LogicalInterface::getByBitSet -- ビットセットでオブジェクトを取得する
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData* pTuple_
//		値を設定するタプル
//
//	RETURN
//
//	EXCEPTIONS
//
bool
LogicalInterface::getByBitSet(Common::DataArrayData* pTuple_)
{
	if (pTuple_->getCount() != 1 ||
		pTuple_->getElement(0)->getType()
		!= Common::DataType::BitSet)
	{
		// エラー
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	bool result = false;
	ModUInt32 min;
	ModUInt32 max;

	if (m_bFirstGet == true)
	{
		// 初めて
		m_bFirstGet = false;

		// 条件に、解析された条件の先頭条件を設定する
		m_uiConditionID = 0;

		min = getMin();
		max = getMax();

		if (min == IllegalKey)
			// 0件取得条件なので1件も取得できない
			return false;
		else if (min == 0)
			m_uiKeyID = IllegalKey;
		else
		{
			if (m_pVectorFile->isValid(min) == false)
			{
				return false;
			}
			m_uiKeyID = min - 1;
		}
	}
	else
	{
		min = getMin();
		max = getMax();
	}

	// Common::ObjectPointer<Common::Data>をBitSetにキャストする
	Common::BitSet& bitset =
		_SYDNEY_DYNAMIC_CAST(Common::BitSet&,
							 *pTuple_->getElement(0));

	while((m_uiKeyID = m_pVectorFile->
		   next(m_uiKeyID, *pTuple_, m_pFieldID, m_uiFieldCount, true))
		  != IllegalKey)
	{
		bool update = false;
		while (m_uiKeyID > max)
		{
			if (++m_uiConditionID !=
				static_cast<ModUInt32>(getConditionSize()))
			{
				// 条件を更新する
				update = true;
				max = getMax();
			}
			else
			{
				// 次の条件がないので抜ける
				// 一度しか取得できない
				m_uiKeyID = IllegalKey;
				return result;
			}
		}

		if (update == false || (min = getMin()) <= m_uiKeyID)
		{
			// 条件の範囲内
			bitset.set(m_uiKeyID);
			result = true;
		}
		else
			// nextの探索開始地点を更新する
			m_uiKeyID = min - 1;
	}

	// m_uiKeyID==IllegalKeyで抜ける、つまり一度しか取得できない

	return result;
}

_SYDNEY_VECTOR2_END
_SYDNEY_END

//
//	Copyright (c) 2005, 2006, 2007, 2008, 2009, 2011, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
