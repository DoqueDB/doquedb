// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogicalInterface.cpp -- 
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2014, 2023 Ricoh Company, Ltd.
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
const char	moduleName[] = "Lob";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "SyInclude.h"
#include "Lob/LogicalInterface.h"
#include "Lob/LobFile.h"
#include "Lob/AutoPointer.h"
#include "Lob/DataOperation.h"
#include "Lob/Locator.h"

#include "Common/DataArrayData.h"
#include "Common/ObjectIDData.h"
#include "Common/BinaryData.h"
#include "Common/StringData.h"
#include "Common/Assert.h"
#include "Common/IntegerArrayData.h"
#include "Common/StringArrayData.h"
#include "Common/NullData.h"

#include "Common/Message.h"

#include "FileCommon/OpenOption.h"

#include "LogicalFile/TreeNodeInterface.h"

#include "Checkpoint/Database.h"

#include "Exception/FileNotOpen.h"
#include "Exception/BadArgument.h"
#include "Exception/VerifyAborted.h"

#include "ModUnicodeCharTrait.h"

_SYDNEY_USING
_SYDNEY_LOB_USING

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
		_AutoDetachPage(LogicalInterface& cFile_) : m_cFile(cFile_)
		{
		}
		~_AutoDetachPage()
		{
			try
			{
				m_cFile.recoverAllPages();
			}
			catch (...)
			{
				m_cFile.setNotAvailable();
				_SYDNEY_RETHROW;
			}
		}

		void flush()
		{
			try
			{
				m_cFile.flushAllPages();
			}
			catch (...)
			{
				try
				{
					m_cFile.recoverAllPages();
				}
				catch (...)
				{
					m_cFile.setNotAvailable();
				}
				_SYDNEY_RETHROW;
			}
		}

	private:
		LogicalInterface& m_cFile;
	};

	//
	//	CLASS
	//	_$$::_AutoOpen
	//
	class _AutoOpen
	{
	public:
		_AutoOpen(LogicalInterface& cFile_,
				  const Trans::Transaction& cTransaction_,
				  LogicalFile::OpenOption::OpenMode::Value eOpenMode_)
			: m_cFile(cFile_), m_bOpen(false)
		{
			if (m_cFile.isOpened() == false)
			{
				m_cFile.open(cTransaction_, eOpenMode_);
				m_bOpen = true;
			}
		}
		~_AutoOpen()
		{
			if (m_bOpen) m_cFile.close();
		}

	private:
		LogicalInterface& m_cFile;
		bool m_bOpen;
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
//	Lob::LogicalInterface::LogicalInterface -- コンストラクタ
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
	: m_cFileID(cFileID_), m_pLobFile(0), m_iRefCount(0),
	  m_bObjectID(false), m_bValue(false), m_pTransaction(0)
{
	m_cObjectID.initialize();
}

//
//	FUNCTION public
//	Lob::LogicalInterface::~LogicalInterface -- デストラクタ
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
//	Lob::LogicalInterface::getFileID -- ファイルIDを返す
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
//	Lob::LogicalInterface::getSize -- ファイルサイズを返す
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
LogicalInterface::getSize() const
{
	; _CHECK_OPEN();
	ModUInt64 size = 0;
	if (isMounted(*m_pTransaction))
	{
		size = m_pLobFile->getFileSize();
	}
	return size;
}

//
//	FUNCTION public
//	Lob::LogicalInterface::getCount -- 挿入されているオブジェクト数を返す
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
		_AutoDetachPage Auto(*const_cast<LogicalInterface*>(this));
		count = m_pLobFile->getCount();
	}
	return count;
}

//
//	FUNCTION public
//	Lob::LogicalInterface::getOverhead -- オブジェクト検索時のオーバヘッドを返す
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
	return m_pLobFile->getOverhead();
}

//
//	FUNCTION public
//	Lob::LogicalInterface::getProcessCost --
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
	return m_pLobFile->getOverhead() * 3;	// 平均3ページぐらいかな
}

//
//	FUNCTION public
//	Lob::LogicalInterface::getSearchParameter
//		-- 検索オープンパラメータを設定する
//
//	NOTES
//	LOBファイルから get によりオブジェクトを取得する場合、
//	  ・オブジェクトを先頭から順次取得する
//	  ・fetch の引数でオブジェクト ID を指定し、特定のオブジェクトを取得する
//	以上、2 つの方法でオブジェクトを取得可能である。
//	このことから、解析可能な検索条件（引数 pCondition_ ）は
//	Fetch ノードを唯一のノードとして持つ木構造のみである。
//	また、pCondition_ に 0 が指定されている場合、
//	オブジェクトを先頭から順次取得と見なす。
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
	//検索条件と一致するすべてのオブジェクトをドライバ側で保持しない
	cOpenOption_.setBoolean(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::CacheAllObject::Key),
		false);
	
	if (pCondition_ == 0)
	{
		//
		// SCAN モード
		//
		
		return false;	// LOBファイルはSCANできない
	}

	if (pCondition_->getType() == LogicalFile::TreeNodeInterface::Fetch)
	{
		//
		// FETCH モード
		// （ fetch の引数でオブジェクト ID を指定するモード）
		//

		if (pCondition_->getOptionSize() != 2)
		{
			_SYDNEY_THROW0(Exception::BadArgument);
		}

		// 第一要素がFetchされるカラムリスト
		const LogicalFile::TreeNodeInterface* pFetchedFields
			= pCondition_->getOptionAt(0);

		// リストは自身をオペランドとして扱う
		if (pFetchedFields->getOperandSize() != 1)
		{
			// ObjectIDしかFETCHは許さない
			return false;
		}

		const LogicalFile::TreeNodeInterface* pField
			= pFetchedFields->getOperandAt(0);
		if (pField->getType() != LogicalFile::TreeNodeInterface::Field
			|| ModUnicodeCharTrait::toInt(pField->getValue()) != 0)
		{
			// ObjectIDしかFETCHは許さない
			return false;
		}

		// オープンオプションに既にオープンモードが設定されていた場合、
		// それは "Read" でなければいけない。そうでなければ例外。
		int 	iValue;
		bool bFind = cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(
							FileCommon::OpenOption::OpenMode::Key), iValue);
		if (bFind && iValue != FileCommon::OpenOption::OpenMode::Read)
		{
			_SYDNEY_THROW0(Exception::BadArgument);
		}

		// オープンオプション(参照引数)にオープンモードを設定
		if (!bFind)
		{
			cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
									FileCommon::OpenOption::OpenMode::Key),
									FileCommon::OpenOption::OpenMode::Read);
		}

		return true;
	}

	return false;
}

//
//	FUNCTION public
//	Lob::LogicalInterface::getProjectionParameter
//		-- プロジェクションオープンパラメータを設定する
//
//	NOTES
//	LOBファイルで、Projectionに指定できるフィールド番号は0と1のみ
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
	// オープンオプションに既にオープンモードが設定されていた場合、
	// それは "Read" でなければいけない。そうでなければ例外。
	int iValue;
	bool bFind = cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(
										FileCommon::OpenOption::OpenMode::Key),
										iValue);

	if (bFind && iValue != FileCommon::OpenOption::OpenMode::Read)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	if (cOpenOption_.getBoolean(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::GetByBitSet::Key)))
	{
		// ビットセットで返せない。
		return false;
	}

	// オープンオプション(参照引数)にオープンモードを設定
	if (!bFind)
	{
		cOpenOption_.setInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key),
			FileCommon::OpenOption::OpenMode::Read);
	}

	// フィールド選択指定がされている、という事を設定する
	cOpenOption_.setBoolean(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::FieldSelect::Key),
		true);

	// フィールド選択指定を設定する
	int iFieldNum = cProjection_.getCount();
	cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
		FileCommon::OpenOption::TargetFieldNumber::Key),
							iFieldNum);

	// オープンオプションに選択されているフィールドの番号を設定
	for (int i = 0; i < iFieldNum; ++i)
	{
		int num = cProjection_.getElement(i);
		if (num != 0 && num != 1)
			return false;
		cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
						FileCommon::OpenOption::TargetFieldIndex::Key, i), num);
	}

	return true;
}

//
//	FUNCTION public
//	Lob::LogicalInterface::getUpdateParameter
//		-- 更新オープンパラメータを設定する
//
//	NOTES
//	LOBファイルでは更新できるフィールドはフィールド番号1のフィールドのみである
//
//	ARGUMENTS
//	const Common::IntegerArrayData&	cProjection_
//		フィールド番号の配列オブジェクトへの参照
//	LogicalFile::OpenOption&				cOpenOption_
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
	// オープンオプションに既にオープンモードが設定されていた場合、
	// それは "Update" でなければいけない。そうでなければ例外。
	int	iValue;
	bool bFind = cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(
								FileCommon::OpenOption::OpenMode::Key), iValue);
	if (bFind && iValue != FileCommon::OpenOption::OpenMode::Update)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// オープンオプション(参照引数)にオープンモードを設定
	if (!bFind)
	{
		cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
									FileCommon::OpenOption::OpenMode::Key),
								FileCommon::OpenOption::OpenMode::Update);
	}

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
		if (num != 1)
			return false;
		cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
						FileCommon::OpenOption::TargetFieldIndex::Key, i), num);
	}

	return true;
}

//
//	FUNCTION public
//	Lob::LogicalInterface::getSortParameter -- ソート順パラメータを設定する
//
//	NOTES
//	LOBファイルはソートできないので、常にfalseを返す
//
//	ARGUMENTS
//	const Common::IntegerArrayData&	cKeys_
//		ソート順を指定するフィールドインデックスの列への参照
//	const Common::IntegerArrayData&	cOrders_
//		引数cKeys_で指定されたフィールドのソート順の列への参照
//		昇順ならば0を、降順ならば1を設定する。
//	LogicalFile::OpenOption&				cOpenOption_
//		レコードファイルオープンオプションオブジェクトへの参照
//
//	RETURN
//	bool
//		常に false を返す。
//
//	EXCEPTIONS
//
bool
LogicalInterface::getSortParameter(const Common::IntegerArrayData& cKeys_,
								   const Common::IntegerArrayData& cOrders_,
								   LogicalFile::OpenOption&	cOpenOption_) const
{
	return false;
}

//
//	FUNCTION public
//	Lob::LogicalInterface::create -- ファイルを生成する
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

//
//	FUNCTION public
//	Lob::LogicalInterface::destroy -- ファイルを破棄する
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
	// とにかくアンマウントする
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	_AutoAttachFile cAuto(*this);
	m_pLobFile->destroy(cTransaction_);
}

//
//	FUNCTION public
//	Lob::LogicalInterface::isAccessible --
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
	return m_pLobFile->isAccessible(bForce_);
}

//
//	FUNCTION public
//	Lob::LogicalInterface::isMounted -- マウントされているか調べる
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
	return m_pLobFile->isMounted(cTransaction_);
}

//
//	FUNCTION public
//	Lob::LogicalInterface::open -- オープンする
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

	// 取得するフィールドを得る
	m_bObjectID = false;
	m_bValue = false;
	if (m_eOpenMode == LogicalFile::OpenOption::OpenMode::Read)
	{
		if (cOpenOption_.getBoolean(_SYDNEY_OPEN_PARAMETER_KEY(
			FileCommon::OpenOption::FieldSelect::Key)))
		{
			int iFieldNum = cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::TargetFieldNumber::Key));
			for (int i = 0; i < iFieldNum; ++i)
			{
				if (cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
					FileCommon::OpenOption::TargetFieldIndex::Key, i)) == 0)
				{
					m_bObjectID = true;
				}
				if (cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
					FileCommon::OpenOption::TargetFieldIndex::Key, i)) == 1)
				{
					m_bValue = true;
				}
			}
		}
		else
		{
			m_bObjectID = true;
			m_bValue = true;
		}
	}
	
	open(cTransaction_, m_eOpenMode);
}

//
//	FUNCTION public
//	Lob::LogicalInterface::close -- クローズする
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
	m_pLobFile->close();
	detachFile();
	m_pTransaction = 0;
}

//
//	FUNCTION public
//	Lob::LogicalInterface::isOpened -- オープンされているか
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
	return (m_pLobFile && m_pTransaction) ? true : false;
}

//
//	FUNCTION public
//	Lob::LogicalInterface::fetch -- 検索条件 (オブジェクトID) を設定する
//
//	NOTES
//	検索条件 (オブジェクトID) を設定する。
//	データは get で求める。
//
//	ARGUMENTS
//	const Common::DataArrayData* pOption_
//		Common::ObjectIDDataを唯一の要素として持つオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::fetch(const Common::DataArrayData* pOption_)
{
	if (isMounted(*m_pTransaction))
	{
		m_cObjectID = convertToObjectID(pOption_);
	}
}

//
//	FUNCTION public
//	Lob::LogicalInterface::get -- オブジェクトを返す
//
//	NOTES
//	fetch で指定されたオブジェクトIDが存在しないものであった場合 0 を返す。
//
//	ARGUMENTS
//	Common::DataArrayData* pTuple_
//		結果を格納する配列
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
		if (m_cObjectID.isInvalid() == false)
		{
			_AutoDetachPage cAuto(*this);
			int e = 0;

			if (m_pLobFile->check(m_cObjectID) == true)
			{
				if (m_bObjectID == true)
				{
					// ObjectIDを取得する
					Common::ObjectIDData cObjectIDData;
					convertToObjectIDData(m_cObjectID, &cObjectIDData);
					// instanceを変えないためassignを使う
					pTuple_->getElement(e++)->assign(&cObjectIDData);
				}
		
				if (m_bValue == true)
				{
					// Valueを取得する
					switch (m_cFileID.getFileType())
					{
					case FileID::FileType::BLOB:
					{
						DataOperation<char> cDataOperation(m_pLobFile);
						ModSize uiLength = cDataOperation.getMaxSize();	// 全部
						bool isNull = false;
						AutoPointer<char> p
							= cDataOperation.get(m_cObjectID, 0,
												 uiLength, isNull);

						// instanceを変えないためgetElementしたBinaryDataを使う
						Common::Data::Pointer pData = pTuple_->getElement(e++);
						if (pData->getType() != Common::DataType::Binary) {
							_SYDNEY_THROW0(Exception::BadArgument);
						}
						Common::BinaryData* pBinaryData =
							_SYDNEY_DYNAMIC_CAST(Common::BinaryData*,
												 pData.get());

						if (p.get() != 0)
						{
							pBinaryData->setValue(p.release(),
												  uiLength,
												  false,
												  uiLength);
						}
						else if (isNull == false)
						{
							pBinaryData->setValue(0, 0);
						}
						else
						{
							pBinaryData->setNull();
						}
					}
					break;
					case FileID::FileType::NCLOB:
					{
						DataOperation<ModUnicodeChar>
							cDataOperation(m_pLobFile);
						ModSize uiLength = cDataOperation.getMaxSize();	// 全部
						bool isNull = false;
						AutoPointer<ModUnicodeChar> p
							= cDataOperation.get(m_cObjectID, 0,
												 uiLength, isNull);
						// instanceを変えないためgetElementしたStringDataを使う
						Common::Data::Pointer pData = pTuple_->getElement(e++);
						if (pData->getType() != Common::DataType::String) {
							_SYDNEY_THROW0(Exception::BadArgument);
						}
						Common::StringData* pStringData =
							_SYDNEY_DYNAMIC_CAST(Common::StringData*,
												 pData.get());

						if (p.get() != 0)
						{
							pStringData->setValue(p.get(), uiLength);
						}
						else if (isNull == false)
						{
							pStringData->setValue(ModUnicodeString());
						}
						else
						{
							pStringData->setNull();
						}
					}
					break;
					default:
						;
					}
				}
			
				result = true;
			}

			m_cObjectID.clear();
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
//	Lob::LogicalInterface::insert -- オブジェクトを挿入する
//
//	NOTES
//	オブジェクトを挿入する。
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

		_AutoDetachPage cAuto(*this);

		ObjectID cObjectID;
		Common::Data::Pointer pData = pTuple_->getElement(1);

		if (pData->getType() == Common::DataType::ObjectID)
		{
			// undoExpungeである
			const LogicalFile::ObjectID* pIDData
				= _SYDNEY_DYNAMIC_CAST(const LogicalFile::ObjectID*,
									   pData.get());
		
			cObjectID.m_uiPageID = pIDData->getFormerValue();
			cObjectID.m_uiPosition = pIDData->getLatterValue();

			m_pLobFile->undoExpunge(cObjectID);
		}
		else
		{
			// 普通の挿入
			switch (m_cFileID.getFileType())
			{
			case FileID::FileType::BLOB:
			{
				if (pData->getType() == Common::DataType::String)
				{
					pData = pData->cast(Common::DataType::Binary);
				}
				else if (pData->getType() != Common::DataType::Binary)
				{
					_SYDNEY_THROW0(Exception::BadArgument);
				}
				Common::BinaryData* pBinaryData
					= _SYDNEY_DYNAMIC_CAST(Common::BinaryData*, pData.get());
				DataOperation<char> cDataOperation(m_pLobFile);
				const char* p
					= syd_reinterpret_cast<const char*>(
						pBinaryData->getValue());
				ModSize uiLength = pBinaryData->getSize();
				cObjectID = cDataOperation.insert(p, uiLength);
			}
			break;
			case FileID::FileType::NCLOB:
			{
				if (pData->getType() != Common::DataType::String)
				{
					_SYDNEY_THROW0(Exception::BadArgument);
				}
				Common::StringData* pStringData
					= _SYDNEY_DYNAMIC_CAST(Common::StringData*, pData.get());
				DataOperation<ModUnicodeChar> cDataOperation(m_pLobFile);
				const ModUnicodeString& p = pStringData->getValue();
				ModSize uiLength = p.getLength();
				cObjectID = cDataOperation.insert(p, uiLength);
			}
			break;
			default:
				;
			}
		}
		
		// ObjectIDを設定する

		// 第一フィールドにObjectIDを入れる
		Common::ObjectIDData* pObjectIDData
			= _SYDNEY_DYNAMIC_CAST(Common::ObjectIDData*,
								   pTuple_->getElement(0).get());
		; _SYDNEY_ASSERT(pObjectIDData);

		convertToObjectIDData(cObjectID, pObjectIDData);
		
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
//	Lob::LogicalInterface::update -- オブジェクトを更新する
//
//	NOTES
//	オブジェクトを更新する。
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
		_AutoDetachPage cAuto(*this);

		// ObjectIDを得る
		ObjectID cObjectID = convertToObjectID(pKey_);
		Common::Data::Pointer pData = pTuple_->getElement(0);

		if (pData->getType() == Common::DataType::ObjectID)
		{
			// undoUpdateである
			m_pLobFile->undoUpdate(cObjectID);
		}
		else
		{
			// 普通の更新
			switch (m_cFileID.getFileType())
			{
			case FileID::FileType::BLOB:
			{
				if (pData->getType() == Common::DataType::String)
				{
					pData = pData->cast(Common::DataType::Binary);
				}
				else if (pData->getType() != Common::DataType::Binary)
				{
					_SYDNEY_THROW0(Exception::BadArgument);
				}
				Common::BinaryData* pBinaryData
					= _SYDNEY_DYNAMIC_CAST(Common::BinaryData*, pData.get());
				DataOperation<char> cDataOperation(m_pLobFile);
				const char* p = syd_reinterpret_cast<const char*>(
					pBinaryData->getValue());
				ModSize uiLength = pBinaryData->getSize();
				cDataOperation.update(cObjectID, p, uiLength);
			}
			break;
			case FileID::FileType::NCLOB:
			{
				if (pData->getType() != Common::DataType::String)
				{
					_SYDNEY_THROW0(Exception::BadArgument);
				}
				Common::StringData* pStringData
					= _SYDNEY_DYNAMIC_CAST(Common::StringData*, pData.get());
				DataOperation<ModUnicodeChar> cDataOperation(m_pLobFile);
				const ModUnicodeString& p = pStringData->getValue();
				ModSize uiLength = p.getLength();
				cDataOperation.update(cObjectID, p, uiLength);
			}
			break;
			default:
				;
			}
		}

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
//	Lob::LogicalInterface::expunge -- オブジェクトを削除する
//
//	NOTES
//	オブジェクトを削除する。
//
//	ARGUMENTS
//	const Common::DataArrayData*	pKey_
//		削除するオブジェクトを指定するオブジェクトID
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
		_AutoDetachPage cAuto(*this);

		// ObjectIDを得る
		ObjectID cObjectID = convertToObjectID(pKey_);
		// 削除する
		m_pLobFile->expunge(cObjectID);

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
//	Lob::LogicalInterface::mark -- 巻き戻しの位置を記録する
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
	; _SYDNEY_ASSERT(0);
}

//
//	FUNCTION public
//	Lob::LogicalInterface::rewind -- 記録した位置に戻る
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
	; _SYDNEY_ASSERT(0);
}

//
//	FUNCTION public
//	Lob::LogicalInterface::reset -- カーソルをリセットする
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
}

//
//	FUNCTION public
//	Lob::LogicalInterface::getLocator -- ロケータを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData* pKey_
//		ロケータを得るオブジェクトを指定するオブジェクトID
//
//	RETURN
//	LogicalFile::Locator*
//		ロケータ
//
//	EXCEPTIONS
//
LogicalFile::Locator*
LogicalInterface::getLocator(const Common::DataArrayData* pKey_)
{
	ObjectID cObjectID = convertToObjectID(pKey_);
	return new Locator(*m_pTransaction, cObjectID, this);
}

//
//	FUNCTION public
//	Lob::LogicalInterface::undoExpunge -- 削除を取り消す
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData* pKey_
//		削除を取り消すデータのキー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::undoExpunge(const Common::DataArrayData* pKey_)
{
	_AutoDetachPage cAuto(*this);

	// ObjectIDを得る
	ObjectID cObjectID = convertToObjectID(pKey_);
	// 削除を取り消す
	m_pLobFile->undoExpunge(cObjectID);

	cAuto.flush();
}

//
//	FUNCTION public
//	Lob::LogicalInterface::undoUpdate -- 更新を取り消す
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData* pKey_
//		更新を取り消すデータのキー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::undoUpdate(const Common::DataArrayData* pKey_)
{
	_AutoDetachPage cAuto(*this);

	// ObjectIDを得る
	ObjectID cObjectID = convertToObjectID(pKey_);
	// 更新を取り消す
	m_pLobFile->undoUpdate(cObjectID);

	cAuto.flush();
}

//
//	FUNCTION public
//	Lob::LogicalInterface::compact -- ファイルから不要なデータを削除する
//
//	NOTES
//	LOBファイルの削除は、削除フラグを立てるだけで実際には削除しない。
//	それはLOBファイルに格納されるデータは長大なので、削除のUNDOのために
//	ログに記録することができないからである。削除したトランザクションが
//	確定すれば、そのデータは削除することができる。
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	bool& bIncomplete_
//		true
//			今回の処理で処理し残しがある
//		false
//			今回の処理で完全に処理してきている
//
//	bool& bModified_
//		true
//			今回の処理で更新されている
//		false
//			今回の処理で更新されていない
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::compact(const Trans::Transaction& cTransaction_,
						  bool& bIncomplete_, bool& bModified_)
{
	try
	{
		if (isMounted(cTransaction_))
		{
			bool isExist = false;
			{
				// まずはReadOnlyでオープンして
				// 削除すべきデータがあるかチェックする
				_AutoOpen cOpen(*this, cTransaction_,
								LogicalFile::OpenOption::OpenMode::Read);
				{
					_AutoDetachPage cAuto(*this);
					isExist = m_pLobFile->isExistExpungeData();
					cAuto.flush();
				}
			}

			if (isExist == true)
			{
				// 削除するべきデータがあるので削除する
				_AutoOpen cOpen(*this, cTransaction_,
								LogicalFile::OpenOption::OpenMode::Update);
			
				while (isExist == true)
				{
					_AutoDetachPage cAuto(*this);
					// 削除を取り消す
					if (m_pLobFile->compact() == false)
					{
						// 削除するものはもうない
						isExist = false;
					}
					bModified_ = true;
					cAuto.flush();

					// 中断をチェックする
					if (cTransaction_.isCanceledStatement() == true)
					{
						if (isExist == true && bIncomplete_ == false)
							bIncomplete_ = true;	// まだ処理が途中
						break;
					}
				}
			}
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
//	Lob::LogicalInterface::equals -- 比較
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
//	Lob::LogicalInterface::sync -- レコードファイルの同期をとる
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction& cTransaction_
//		トランザクション
//	bool& bIncomplete_
//		true
//			今回の処理で処理し残しがある
//		false
//			今回の処理で完全に処理してきている
//
//	bool& bModified_
//		true
//			今回の処理で更新されている
//		false
//			今回の処理で更新されていない
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
			m_pLobFile->sync(cTransaction_, bIncomplete_, bModified_);
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

//
//	FUNCTION public
//	Lob::LogicalInterface::move -- ファイルを移動する
//
//	NOTES
//  ファイルを移動する。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	const Common::StringArrayData&	Area_
//		移動後のレコードファイル格納ディレクトリパス構造体への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
LogicalInterface::move(const Trans::Transaction& cTransaction_,
					   const Common::StringArrayData& cArea_)
{
	_AutoAttachFile cAuto(*this);
	Os::Path cPath = cArea_.getElement(0);
	m_pLobFile->move(cTransaction_, cPath);
	m_cFileID.setPath(cPath);
}

// FUNCTION public
//	Lob::LogicalInterface::getNoLatchOperation -- ラッチが不要なオペレーションを返す
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
	return Operation::Open
		| Operation::Close
		| Operation::Reset
		| Operation::GetProcessCost
		| Operation::GetOverhead;
}

// FUNCTION public
//	Lob::LogicalInterface::getCapability -- Capabilities of file driver
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
	return Capability::Undo;
}

//
//	FUNCTION public
//	Lob::LogicalInterface::toString --
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
//	Lob::LogicalInterface::mount --	マウントする
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
	if (m_pLobFile->isMounted(cTransaction_) == false)
	{
		m_pLobFile->mount(cTransaction_);
		m_cFileID.setMounted(true);
	}

	return m_cFileID;
}

//
//	FUNCTION public
//	Lob::LogicalInterface::unmount -- アンマウントする
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
	_AutoAttachFile cAuto(*this);

	// とにかくアンマウントする
	m_pLobFile->unmount(cTransaction_);
	m_cFileID.setMounted(false);

	return m_cFileID;
}

//
//	FUNCTION public
//	Lob::LogicalInterface::flush -- フラッシュする
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
		m_pLobFile->flush(cTransaction_);
	}
}

//
//	FUNCTION public
//	Lob::LogicalInterface::startBackup -- バックアップ開始を通知する
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
		m_pLobFile->startBackup(cTransaction_, bRestorable_);
	}
}

//
//	FUNCTION public
//	Lob::LogicalInterface::endBackup -- バックアップ終了を通知する
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
		m_pLobFile->endBackup(cTransaction_);
	}
}

//
//	FUNCTION public
//	Lob::LogicalInterface::recover -- 障害回復する
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
		m_pLobFile->recover(cTransaction_, cPoint_);
	}
}

//
//	FUNCTION public
//	Lob::LogicalInterface::restore
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
		m_pLobFile->restore(cTransaction_, cPoint_);
	}
}

//
//	FUNCTION public
//	Lob::LogicalInterface::verify -- 整合性検査を行う
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
		
		m_pLobFile->startVerification(cTransaction_, uiTreatment_, cProgress_);
		try
		{
			_AutoDetachPage cPage(*this);
			m_pLobFile->verify();
			cPage.flush();
		}
		catch (Exception::VerifyAborted&)
		{
			// 何もしない
			;
		}
		catch (...)
		{
			m_pLobFile->endVerification();
			_SYDNEY_RETHROW;
		}
		m_pLobFile->endVerification();
	}
}

//
//	FUNCTION public
//	Lob::LogicalInterface::attachFile -- ファイルをattachする
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
	m_pLobFile = new LobFile(m_cFileID);
}

//
//	FUNCTION public
//	Lob::LogicalInterface::detachFile -- ファイルをdetachする
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
	delete m_pLobFile, m_pLobFile = 0;
}

//
//	FUNCTION public
//	Lob::LogicalInterface::isAttached -- ファイルがattachされているかどうか
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
	return m_pLobFile ? true : false;
}

//
//	FUNCTION public static
//	Lob::LogicalInterface::attach -- 参照カウンタを増やす
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalInterface* pFile_
//		ファイル
//
//	RETURN
//	LogicalInterface*
//		ファイルへのポインタ
//
//	EXCEPTIONS
//
LogicalInterface*
LogicalInterface::attach(const LogicalInterface* pFile_)
{
	pFile_->m_iRefCount++;
	return const_cast<LogicalInterface*>(pFile_);
}

//
//	FUNCTION public static
//	Lob::LogicalInterface::detach -- 参照カウンタを減らし、開放する
//
//	NOTES
//
//	ARGUMENTS
//	LogicalInterface* pFile_
//		ファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::detach(LogicalInterface* pFile_)
{
	if (--pFile_->m_iRefCount == 0)
		delete pFile_;
}

//
//	FUNCTION public
//	Lob::LogicalInterface::recoverAllPages -- 全ページの変更を破棄する
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
	m_pLobFile->recoverAllPages();
}

//
//	FUNCTION public
//	Lob::LogicalInterface::flushAllPages -- 全ページの変更を確定する
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
	m_pLobFile->flushAllPages();
}

//
//	FUNCTION public
//	Lob::LogicalInterface::setNotAvailable -- 利用不可にする
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
//	FUNCTION public
//	Lob::LogicalInterface::get -- 指定範囲のデータを取り出す
//
//	NOTES
//	Locatorのためのインターフェース
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const ObjectID& cObjectID_
//		オブジェクトID
//	ModSize uiPosition_
//		位置(SQLの規格により1ベース)
//	ModSize uiLength_
//		長さ
//	Common::Data* pResult_
//		[OUT]データ
//
//	RETURN
//		true ... 正しく取り出せた
//		false... NULLだった(pResult_はNULLになる)
//
//	EXCEPTIONS
//
bool
LogicalInterface::get(const Trans::Transaction& cTransaction_,
					  const ObjectID& cObjectID_,
					  ModSize uiPosition_, ModSize uiLength_,
					  Common::Data* pResult_)
{
	; _SYDNEY_ASSERT(pResult_);

	bool bResult = false;

	if (uiPosition_ < 1)
		_SYDNEY_THROW0(Exception::BadArgument);
		
	_AutoOpen cOpen(*this, cTransaction_,
					LogicalFile::OpenOption::OpenMode::Read);
	{
		uiPosition_ -= 1;	// 1ベースなので0ベースへ
		
		_AutoDetachPage cAuto(*this);

		switch (m_cFileID.getFileType())
		{
		case FileID::FileType::BLOB:
			{
				bool isNull = false;
				DataOperation<char> cDataOperation(m_pLobFile);
				AutoPointer<char> p
					= cDataOperation.get(cObjectID_, uiPosition_,
										 uiLength_, isNull);

				if (pResult_->getType() != Common::DataType::Binary) {
					_SYDNEY_THROW0(Exception::BadArgument);
				}
				Common::BinaryData* pBinaryData =
					_SYDNEY_DYNAMIC_CAST(Common::BinaryData*, pResult_);
					
				if (p.get() != 0)
				{
					pBinaryData->setValue(p.release(),
										  uiLength_,
										  false,
										  uiLength_);
					bResult = true;
				}
				else if (isNull == false)
				{
					pBinaryData->setValue(0, 0);
					bResult = true;
				}
				else
				{
					pBinaryData->setNull();
				}
			}
			break;
		case FileID::FileType::NCLOB:
			{
				bool isNull = false;
				DataOperation<ModUnicodeChar> cDataOperation(m_pLobFile);
				AutoPointer<ModUnicodeChar> p
					= cDataOperation.get(cObjectID_, uiPosition_,
										 uiLength_, isNull);

				if (pResult_->getType() != Common::DataType::String) {
					_SYDNEY_THROW0(Exception::BadArgument);
				}
				Common::StringData* pStringData =
					_SYDNEY_DYNAMIC_CAST(Common::StringData*, pResult_);

				if (p.get() != 0)
				{
					pStringData->setValue(p.get(), uiLength_);
					bResult = true;
				}
				else if (isNull == false)
				{
					pStringData->setValue(ModUnicodeString());
					bResult = true;
				}
				else
				{
					pStringData->setNull();
				}
			}
			break;
		default:
			;
		}

		cAuto.flush();
	}

	return bResult;
}

//
//	FUNCTION public
//	Lob::LogicalInterface::replcae -- 指定範囲のデータを変更する
//
//	NOTES
//	Locatorのためのインターフェース
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const ObjectID& cObjectID_
//		オブジェクトID
//	ModSize uiPosition_
//		位置(SQLの規格により1ベース)
//	const Common::Data* pData_
//		データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::replace(const Trans::Transaction& cTransaction_,
						  const ObjectID& cObjectID_,
						  ModSize uiPosition_, const Common::Data* pData_)
{
	if (uiPosition_ < 1)
		_SYDNEY_THROW0(Exception::BadArgument);
	
	_AutoOpen cOpen(*this, cTransaction_,
					LogicalFile::OpenOption::OpenMode::Update);
	{
		uiPosition_ -= 1;	// 1ベースなので0ベースへ
		
		_AutoDetachPage cAuto(*this);

		switch (m_cFileID.getFileType())
		{
		case FileID::FileType::BLOB:
			{
				Common::Data::Pointer data;
				if (pData_->getType() == Common::DataType::String)
				{
					data = pData_->cast(Common::DataType::Binary);
					pData_ = data.get();
				}
				else if (pData_->getType() != Common::DataType::Binary)
				{
					_SYDNEY_THROW0(Exception::BadArgument);
				}
				const Common::BinaryData* p
					= _SYDNEY_DYNAMIC_CAST(const Common::BinaryData*, pData_);
				DataOperation<char> cDataOperation(m_pLobFile);
				const char* ptr = syd_reinterpret_cast<const char*>(p->getValue());
				cDataOperation.replace(cObjectID_, uiPosition_,
									   ptr, p->getSize());
			}
			break;
		case FileID::FileType::NCLOB:
			{
				if (pData_->getType() != Common::DataType::String)
				{
					_SYDNEY_THROW0(Exception::BadArgument);
				}
				const Common::StringData* p
					= _SYDNEY_DYNAMIC_CAST(const Common::StringData*, pData_);
				const ModUnicodeString& data = p->getValue();
				DataOperation<ModUnicodeChar> cDataOperation(m_pLobFile);
				cDataOperation.replace(cObjectID_, uiPosition_,
									   data, data.getLength());
			}
			break;
		default:
			;
		}

		cAuto.flush();
	}
}

//
//	FUNCTION public
//	Lob::LogicalInterface::append -- データを末尾に追加する
//
//	NOTES
//	Locatorのためのインターフェース
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const ObjectID& cObjectID_
//		オブジェクトID
//	const Common::Data* pData_
//		データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::append(const Trans::Transaction& cTransaction_,
						 const ObjectID& cObjectID_,
						 const Common::Data* pData_)
{
	_AutoOpen cOpen(*this, cTransaction_,
					LogicalFile::OpenOption::OpenMode::Update);
	{					
		_AutoDetachPage cAuto(*this);

		switch (m_cFileID.getFileType())
		{
		case FileID::FileType::BLOB:
			{
				Common::Data::Pointer data;
				if (pData_->getType() == Common::DataType::String)
				{
					data = pData_->cast(Common::DataType::Binary);
					pData_ = data.get();
				}
				else if (pData_->getType() != Common::DataType::Binary)
				{
					_SYDNEY_THROW0(Exception::BadArgument);
				}
				const Common::BinaryData* p
					= _SYDNEY_DYNAMIC_CAST(const Common::BinaryData*, pData_);
				DataOperation<char> cDataOperation(m_pLobFile);
				const char* ptr = syd_reinterpret_cast<const char*>(p->getValue());
				cDataOperation.append(cObjectID_,
									  ptr, p->getSize());
			}
			break;
		case FileID::FileType::NCLOB:
			{
				if (pData_->getType() != Common::DataType::String)
				{
					_SYDNEY_THROW0(Exception::BadArgument);
				}
				const Common::StringData* p
					= _SYDNEY_DYNAMIC_CAST(const Common::StringData*, pData_);
				const ModUnicodeString& data = p->getValue();
				DataOperation<ModUnicodeChar> cDataOperation(m_pLobFile);
				cDataOperation.append(cObjectID_,
									  data, data.getLength());
			}
			break;
		default:
			;
		}

		cAuto.flush();
	}
}

//
//	FUNCTION public
//	Lob::LogicalInterface::truncate -- データを末尾から指定サイズ分切り詰める
//
//	NOTES
//	Locatorのためのインターフェース
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const ObjectID& cObjectID_
//		オブジェクトID
//	ModSize uiLength_
//		長さ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::truncate(const Trans::Transaction& cTransaction_,
						   const ObjectID& cObjectID_,
						   ModSize uiLength_)
{
	_AutoOpen cOpen(*this, cTransaction_,
					LogicalFile::OpenOption::OpenMode::Update);
	{					
		_AutoDetachPage cAuto(*this);

		switch (m_cFileID.getFileType())
		{
		case FileID::FileType::BLOB:
			{
				DataOperation<char> cDataOperation(m_pLobFile);
				cDataOperation.truncate(cObjectID_, uiLength_);
			}
			break;
		case FileID::FileType::NCLOB:
			{
				DataOperation<ModUnicodeChar> cDataOperation(m_pLobFile);
				cDataOperation.truncate(cObjectID_, uiLength_);
			}
			break;
		default:
			;
		}

		cAuto.flush();
	}
}

//
//	FUNCTION public
//	Lob::LogicalInterface::getDataSize -- データ長を得る
//
//	NOTES
//	Locatorのためのインターフェース
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const ObjectID& cObjectID_
//		オブジェクトID
//
//	RETURN
//	ModSize
//		データ長
//
//	EXCEPTIONS
//
ModSize
LogicalInterface::getDataSize(const Trans::Transaction& cTransaction_,
							  const ObjectID& cObjectID_)
{
	ModSize uiLength = 0;
	
	_AutoOpen cOpen(*this, cTransaction_,
					LogicalFile::OpenOption::OpenMode::Read);
	
	{					
		_AutoDetachPage cAuto(*this);

		switch (m_cFileID.getFileType())
		{
		case FileID::FileType::BLOB:
			{
				DataOperation<char> cDataOperation(m_pLobFile);
				uiLength = cDataOperation.getSize(cObjectID_);
			}
			break;
		case FileID::FileType::NCLOB:
			{
				DataOperation<ModUnicodeChar> cDataOperation(m_pLobFile);
				uiLength = cDataOperation.getSize(cObjectID_);
			}
			break;
		default:
			;
		}

		cAuto.flush();
	}

	return uiLength;
}

//
//	FUNCTION public
//	Lob::LogicalInterface::open -- オープンする
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
		m_pLobFile->open(cTransaction_, eOpenMode_);
	}
	catch (...)
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
//	Lob::LogicalInterface::substantiate -- ファイルを本当に作成する
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
	m_pLobFile->create();
	flushAllPages();
}

//
//	FUNCTION private
//	Lob::LogicalInterface::convertToObjectID
//		-- Common::DataArrayDataからオブジェクトIDへの変換
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData* pObjectID_
//		ObjectIDDataを唯一の要素に持つDataArrayData
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ObjectID
LogicalInterface::convertToObjectID(const Common::DataArrayData* pObjectID_)
{
	//
	// 必ず 1 要素がなければならない。
	//
	if (!pObjectID_ || pObjectID_->getCount() != 1)
		_SYDNEY_THROW0(Exception::BadArgument);

	//
	// 配列の要素が Common::ObjectIDData クラスの
	// インスタンスオブジェクトでなければならない。
	//
	const Common::DataArrayData::Pointer& pElement = pObjectID_->getElement(0);
	if (pElement->getType() != Common::DataType::ObjectID)
		_SYDNEY_THROW0(Exception::BadArgument);

	const LogicalFile::ObjectID* pIDData =
		_SYDNEY_DYNAMIC_CAST(const LogicalFile::ObjectID*, pElement.get());
	; _SYDNEY_ASSERT(pIDData);

	ObjectID cObjectID;
	cObjectID.m_uiPageID = pIDData->getFormerValue();
	cObjectID.m_uiPosition = pIDData->getLatterValue();
	
	return cObjectID;
}

//
//	FUNCTION private
//	Lob::LogicalInterface::convertToObjectIDData
//		-- オブジェクトIDからObjectIDデータへの変換
//
//	NOTES
//
//	ARGUMENTS
//	const Lob::ObjectID& cObjectID_
//		オブジェクトID
//	Common::ObjectIDData* pObjectIDData_
//		設定するオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::convertToObjectIDData(const ObjectID& cObjectID_,
										Common::ObjectIDData* pObjectIDData_)
{
	pObjectIDData_->setValue(
		static_cast<Common::ObjectIDData::FormerType>(
		cObjectID_.m_uiPageID),
		static_cast<Common::ObjectIDData::LatterType>(
		cObjectID_.m_uiPosition));
}

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
