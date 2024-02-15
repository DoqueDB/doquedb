// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.cpp -- ベクタファイルクラスの実現ファイル
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2011, 2023 Ricoh Company, Ltd.
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
const char	moduleName[] = "Vector";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

//なるべく軽減したい
#include "Checkpoint/Database.h"
#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/ExceptionMessage.h"
#include "Common/DateTimeData.h"
#include "Common/IntegerArrayData.h"
#include "Common/Message.h"
#include "Common/StringArrayData.h"
#include "Common/UnsignedIntegerData.h"

#include "Exception/BadArgument.h"
#include "Exception/FileNotOpen.h"
#include "Exception/NotSupported.h"
#include "Exception/FakeError.h"
#include "Exception/MemoryExhaust.h"
#include "Exception/Unexpected.h"

#include "FileCommon/AutoAttach.h"
#include "FileCommon/FileOption.h"
#include "FileCommon/NodeWrapper.h"
#include "FileCommon/OpenMode.h"
#include "FileCommon/VectorKey.h"
#include "LogicalFile/Estimate.h"
#include "LogicalFile/TreeNodeInterface.h"
#include "LogicalFile/VectorKey.h"
#include "LogicalFile/FileID.h"
#include "LogicalFile/OpenOption.h"

#include "PhysicalFile/File.h"
#include "PhysicalFile/Manager.h"
#include "PhysicalFile/Page.h"
#include "Os/Path.h"

#include "Vector/FileInformation.h"
#include "Vector/FileParameter.h"
#include "Vector/FieldIterator.h"
#include "Vector/ObjectIterator.h"
#include "Vector/ScanIterator.h"
#include "Vector/FetchIterator.h"
#include "Vector/SearchIterator.h"
#include "Vector/UpdateIterator.h"
#include "Vector/MessageAll_Class.h"
#include "Vector/OpenOption.h"
#include "Vector/OpenParameter.h"
#include "Vector/PageManager.h"

#include "Vector/File.h"

#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_VECTOR_USING

namespace {
// dynamic_cast 除去のための型チェックユーティリティクラス
class TypeID {
public:
	// Common::DataArrayData
	inline static bool isDataArrayData(const Common::Data* pObject_)
	{
		return ( (pObject_->getType() == Common::DataType::Array)
		      && (static_cast<const Common::ArrayData*>(pObject_)->getElementType() == Common::DataType::Data) )
		      ? true : false;
	}
	// LogicalFile::VectorKey
	inline static bool isVectorKey(const Common::Data* pObject_)
	{
		return (pObject_->getType() == Common::DataType::UnsignedInteger)? true : false;
	}
};

}

//
//	CONST
//	Vector::File::m_pszPhysicalFileName -- 物理ファイル名
//
//	NOTES
//	物理ファイル名。
//
const char*
File::m_pszPhysicalFileName = "VCT";

//
//	FUNCTION public
//	Vector::File::File -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	const LogicalFile::FileID&	cFileOption_
//		可変長ベクタファイルオプションオブジェクトへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
File::File(const LogicalFile::FileID& rFileOption_)
	: LogicalFile::File(),
      m_cFileParameter(rFileOption_),
	  m_pOpenParameter(0),
	  m_pPageManager(0),
	  m_pObjectIterator(0),
	  m_eTransactionCategory(Trans::Transaction::Category::Unknown),
	  m_iCountCache(-1)
{
	_SYDNEY_FAKE_ERROR("Vector::File::File",
					   Exception::BadArgument(moduleName, srcFile, __LINE__));

	const PhysicalFile::File::StorageStrategy&
		strategy = m_cFileParameter.getStorageStrategy();
	m_cFileParameter.initializeBlockParameters(
		PhysicalFile::File::getPageDataSize(
			strategy.m_PhysicalFileType, strategy.m_VersionFileInfo._pageSize));
}

//
//	FUNCTION public
//	Vector::File::~File -- デストラクタ
//
//	NOTES
//	デストラクタ
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
File::~File()
{
	if (isOpen()) { close(); }

	if (m_pOpenParameter != 0) {
		delete m_pOpenParameter, m_pOpenParameter = 0;
	}
}

//
//	FUNCTION public
//	Vector::File::initialize -- 初期化
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//		なし
//
void
File::initialize()
{
	; // do noting
}

//	FUNCTION public
//	Vector::File::terminate -- 終了処理
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//		なし
//
void
File::terminate()
{
	; // do nothing
}

// 物理ファイルが生成されたかどうかを調べる
bool
File::isAccessible( bool Force_ ) const
{
	bool bCreated = false;

	if (isOpen())
	{
		bCreated = m_pPageManager->getFile()->isAccessible( Force_ );
	}
	else // オープンされていない
	{
		_SYDNEY_FAKE_ERROR("Vector::File::isAccessible",Exception::BadArgument(moduleName, srcFile, __LINE__));
		FileCommon::AutoPhysicalFile file(m_cFileParameter.getStorageStrategy() ,m_cFileParameter.getBufferingStrategy() ,m_cFileParameter.getLockName());
		bCreated = file->isAccessible( Force_ );
	}
	return bCreated;
}

//	FUNCTION public
//	Vector::File::isMounted -- マウントされているか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			マウントされているか調べる
//			トランザクションのトランザクション記述子
//
//	RETURN
//		true
//			マウントされている
//		false
//			マウントされていない
//
//	EXCEPTIONS

bool
File::isMounted(const Trans::Transaction& trans) const
{
	return (isOpen()) ? m_pPageManager->getFile()->isMounted(trans) :
		FileCommon::AutoPhysicalFile(
			m_cFileParameter.getStorageStrategy(),
			m_cFileParameter.getBufferingStrategy(),
			m_cFileParameter.getLockName())->isMounted(trans);
}

//
//	FUNCTION
//	Vector::File::isOpen -- ファイルがオープンかどうか調べる
//
//	NOTE
//		ファイルがオープンかどうか調べる
//
//	ARGUMENTS
//		なし
//			
//	RETURN
//		bool
//
//	EXCEPTIONS
//		なし
//			
bool
File::isOpen() const
{
	return (m_pPageManager != 0);
}

//
//	FUNCTION public
//	Vector::File::getFileID -- ファイルIDを返す
//
//	NOTES
//	ファイルIDを返す
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
File::getFileID() const
{
	return m_cFileParameter.getFileOption();
}

//
//	FUNCTION public
//	Vector::File::getSize -- ファイルサイズを返す
//
//	NOTES
//	ベクタファイルは、自身がもつ物理ファイルのファイルサイズを返す。
//	この関数を呼び出す場合、目的のベクタファイルが
//	オープンされていなければならない。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		ベクタファイルサイズ [byte]
//
//	EXCEPTIONS
//	FileNotOpen
//		ベクタファイルがオープンされていない
//
ModUInt64
File::getSize() const
{
	// ベクタファイルがオープンされているかチェック
	if (!isOpen()) {
		throw Exception::FileNotOpen(moduleName, srcFile, __LINE__);
	}

	// 物理ファイルの実体であるOSファイルのサイズ(ModFileSize)を返す

	; _SYDNEY_ASSERT(m_pTransaction);
	return (isMounted(*m_pTransaction)) ?
		m_pPageManager->getFile()->getSize() : 0;
}

//
//	FUNCTION public
//	Vector::File::getCount -- 挿入されているオブジェクト数を返す
//
//	NOTES
//	ベクタファイル自身に挿入されているオブジェクトの総数を返す。
//	この関数を呼び出す場合には、目的のベクタファイルが
//	オープンされていなければならない。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModInt64
//		オブジェクトの総数
//
//	EXCEPTIONS
//	FileNotOpen
//		ベクタファイルがオープンされていない
//
ModInt64
File::getCount() const
{
	if ( ( m_eTransactionCategory == Trans::Transaction::Category::ReadOnly )
	  && ( 0 <= m_iCountCache ) )
	{
		//読み取り専用トランザクションかつ読み込み済み
		return m_iCountCache;
	}

	// ベクタファイルがオープンされているかチェック
	if (!isOpen()) {
		throw Exception::FileNotOpen(moduleName, srcFile, __LINE__);
	}

	; _SYDNEY_ASSERT(m_pTransaction);
	if (!isMounted(*m_pTransaction))
		return 0;

	AutoFileInformation fileinfo( *m_pPageManager );
	fileinfo.attach();//自動的に、detachする。
	m_iCountCache = fileinfo->getObjectCount();//m_iCountCache は mutable
	return m_iCountCache;
}

//	FUNCTION public
//	Vector::File::getOverhead -- オブジェクト検索時のオーバヘッドを返す
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
//		常に 0.0 を返す
//
//	EXCEPTIONS
//		なし
//
double
File::getOverhead() const
{
	//- オープンチェックはしなくていいのだろうか?
	return 0.0; // 常に 0.0 を返す
}

//
//	FUNCTION public
//	Vector::File::getProcessCost -- 
//		オブジェクトへアクセスする際のプロセスコストを返す
//
//	NOTES
//	常に 0.0 を返す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	double
//		プロセスコスト [秒]
//
//	EXCEPTIONS
//	FileNotOpen
//		ベクタファイルがオープンされていない
//
double
File::getProcessCost() const
{
	// ベクタファイルがオープンされているかチェック
	if (!isOpen()) {
		throw Exception::FileNotOpen(moduleName, srcFile, __LINE__);
	}

	; _SYDNEY_ASSERT(m_pTransaction);
	if (!isMounted(*m_pTransaction))
		return 0.0;

 	//- そのうちGetByBitSetモード対応の演算が必要になる

	FileCommon::OpenMode::Mode	eOpenMode = m_pOpenParameter->getOpenMode();
	if  (eOpenMode == FileCommon::OpenMode::Read
	  || eOpenMode == FileCommon::OpenMode::Search)
	{
		// 1秒間にファイルからメモリに転送できる(バイト数)を得る
		const double dXferRateToMemory = 
			(double)LogicalFile::Estimate::getTransferSpeed
				(LogicalFile::Estimate::File);
		SydAssert(dXferRateToMemory > 0.0);
		// 読み込むバイト数を求める
		const ModSize ulReadSize = 
				m_cFileParameter.getPhysicalPageSize();
		// コストを計算して返す
		double dCost = (double)ulReadSize / dXferRateToMemory;
		// 常にページあたりのオブジェクト数で割ることにした
/*		if (eOpenMode == FileCommon::OpenMode::Read
			&& (m_pOpenParameter->getReadSubMode()
				== FileCommon::ReadSubMode::ScanRead))
				dCost /= (double)m_cFileParameter.getBlocksPerPage();
*/
		dCost /= (double)m_cFileParameter.getBlocksPerPage();
		return dCost;
	} else {
		return 0.0;
	}
}

//
//	FUNCTION public
//	Vector::File::getSearchParameter -- 検索オープンパラメータを設定する
//
//	NOTES
//	ベクタファイルからget()によりオブジェクトを取得する場合、
//	  ・オブジェクトを先頭から順次取得する(scanモード)
//	  ・fetch の引数でベクタキーを指定し、
//	  	特定のオブジェクトを取得する(fetchモード)
//	  ・(searchモード)
//	の3つの方法でオブジェクトを取得することが可能である。
//	また、pCondition_ に0が指定されている場合、ベクタファイルは
//	scanモードが指定されているものと見なす。
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface*	pCondition_
//		木構造の検索条件オブジェクトへのポインタ
//	LogicalFile::OpenOption&				rOpenOption_
//		ベクタファイルオープンオプションオブジェクトへの参照
//
//	RETURN
//	bool
//		引数 pCondition_ で示される検索が可能ならば true を、
//		そうでない場合には false を返す。
//
//	EXCEPTIONS
//	BadArgument
//		既にRead以外のオープンモードが設定されている
//
bool
File::getSearchParameter(
	const LogicalFile::TreeNodeInterface*	pCondition_,
	LogicalFile::OpenOption&				rOpenOption_) const
{
	//検索条件と一致するすべてのオブジェクトをドライバ側で保持しない
	rOpenOption_.setBoolean( _SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::CacheAllObject::Key), false);
	if (pCondition_ == 0) {
		// Scanモード(get()でオブジェクトを順次取得するモード)
		rOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), _SYDNEY_OPEN_PARAMETER_VALUE(FileCommon::OpenOption::OpenMode::Read));
		// FileCommon::OpenOption::ReadSubModeの設定
		rOpenOption_.setInteger( _SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::ReadSubMode::Key), _SYDNEY_OPEN_PARAMETER_VALUE(FileCommon::OpenOption::ReadSubMode::Scan));
		return true;
	}

	// Fetchモード (fetch()の引数でVectorKeyを指定するモード)
	else if (pCondition_->getType() == LogicalFile::TreeNodeInterface::Fetch) {
		if (pCondition_->getOptionSize() != 2) {
			// 渡される要素は必ず２つ、ドライバは先頭要素だけ見ればよい
			//SydAssert(false); 
			return false;
		}

		// 第一要素がFetchされるカラムリスト
		const LogicalFile::TreeNodeInterface* 
		  pFetchedFields = pCondition_->getOptionAt(0);
		// リストの長さは1でなければならず、
		if (pFetchedFields->getOperandSize() != 1) {
			return false;
		}
		// かつその値は"0"でなければならない
		const LogicalFile::TreeNodeInterface* 
		  pField = pFetchedFields->getOperandAt(0);
		if (pField->getType() != LogicalFile::TreeNodeInterface::Field
		 || pField->getValue() != ModUnicodeString("0")) {
			return false;
		}

		// オープンオプションに既にオープンモードが設定されていた場合、
		// それは "Read" でなければいけない。そうでなければ例外。
		int iValue;
		const bool bFind = rOpenOption_.getInteger( _SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), iValue);
		if (bFind && iValue != FileCommon::OpenOption::OpenMode::Read) {
			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}
		// ReadSubModeも調べる?

		// オープンオプション(参照引数)にオープンモードを設定
		if (!bFind) {
			rOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), _SYDNEY_OPEN_PARAMETER_VALUE(FileCommon::OpenOption::OpenMode::Read));
			rOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::ReadSubMode::Key), _SYDNEY_OPEN_PARAMETER_VALUE(FileCommon::OpenOption::ReadSubMode::Fetch));
		}
		return true;
	}

	// Search モード
	if( pCondition_->getType() == LogicalFile::TreeNodeInterface::Equals ){
		if( pCondition_->getOperandSize() != 2 ){
			// 条件のオペランドは 2 でなくてはならない
			return false;
		}

		const LogicalFile::TreeNodeInterface* pField =
			pCondition_->getOperandAt(0);
		const LogicalFile::TreeNodeInterface* pValue =
			pCondition_->getOperandAt(1);

		if (pValue->getType() == LogicalFile::TreeNodeInterface::Field &&
			(pField->getType() == LogicalFile::TreeNodeInterface::ConstantValue
			 || pField->getType() == LogicalFile::TreeNodeInterface::Variable))
		{
			// Alternate pField and pValue.
			const LogicalFile::TreeNodeInterface* p = pField;
			pField = pValue;
			pValue = p;
		}

		int iFieldIndex = -1;
		ModInt64 iSearchValue = -1;
		if (pField->getType() == LogicalFile::TreeNodeInterface::Field &&
			(pValue->getType() == LogicalFile::TreeNodeInterface::ConstantValue
			 || pValue->getType() == LogicalFile::TreeNodeInterface::Variable))
		{
			iFieldIndex = ModUnicodeCharTrait::toInt(pField->getValue());
			iSearchValue = ModUnicodeCharTrait::toModInt64(pValue->getValue());
		}

		if (iFieldIndex != 0 || iSearchValue < 0)
		{
			// 条件の FieldIndex は 0 のみ許されている
			// フィールドに対応する値は 0 以上であること
			return false;
		}
		
		// [NOTE] Over ModUInt32Max is not supported.
		//  See OpenParameter::OpenParameter() for details.
		; _TRMEISTER_ASSERT(
			iSearchValue <= static_cast<ModInt64>(ModUInt32Max));

		// パラメータの設定
		// OpenMode パラメータ
		rOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), _SYDNEY_OPEN_PARAMETER_VALUE(FileCommon::OpenOption::OpenMode::Search));
		// SearchField パラメータ
		rOpenOption_.setInteger(_SYDNEY_VECTOR_OPEN_PARAMETER_KEY(Vector::OpenOption::SearchFieldNumber::Key), 1);
		// SearchFieldIndex パラメータ
		rOpenOption_.setInteger(_SYDNEY_VECTOR_OPEN_PARAMETER_KEY(Vector::OpenOption::SearchFieldIndex::Key), iFieldIndex);
		// SearchValue パラメータ
		rOpenOption_.setLongLong(_SYDNEY_VECTOR_OPEN_PARAMETER_KEY(Vector::OpenOption::SearchValue::Key), iSearchValue );

		// SearchOpe パラメータ
		rOpenOption_.setInteger(_SYDNEY_VECTOR_OPEN_PARAMETER_KEY(Vector::OpenOption::SearchOpe::Key), _SYDNEY_OPEN_PARAMETER_VALUE(Vector::OpenOption::SearchOpe::Equals));
		return true;
	}

	// rOpenOption_が不適切だった場合
	else return false;
}

//	FUNCTION public
//	Vector::File::getProjectionParameter --
//		プロジェクションオープンパラメータを設定する
//
//	NOTES
//	ベクタファイルは、引数 rProjection_ で指定されている
//	1つ以上のフィールド番号を読みとり、
//	オブジェクト取得時には、該当するフィールドのみで
//	オブジェクトを構成するようにオープンオプションを設定する。
//	例えば、4つのフィールド（ベクタキーフィールドを含む）
//	で構成されるオブジェクトを挿入するためのベクタファイルから
//	オブジェクトを取得する際に、第2・第3フィールドのみを
//	取得するのであれば、引数 rProjection_ は、下図のように設定する。
//
//	      rProjection_
//	   ┌───────┐
//	   │     ┌──┐ │
//	   │ [0] │  2 │ │
//	   │     ├──┤ │
//	   │ [1] │  3 │ │
//	   │     └──┘ │
//	   └───────┘
//
//	ARGUMENTS
//	const Common::IntegerArrayData&	rProjection_
//		フィールド番号の配列オブジェクトへの参照
//	LogicalFile::OpenOption&		rOpenOption_
//		オープンオプションオブジェクトへの参照
//
//	RETURN
//	bool
//		引数 rProjection_ で示されるフィールドでオブジェクトを構成可能ならば
//		true、そうでない場合は false。
//
//	EXCEPTIONS
//	BadArgument
//		オープンモードが正しくない
//
bool
File::getProjectionParameter(
	const LogicalFile::TreeNodeInterface* pNode_,
	LogicalFile::OpenOption&		rOpenOption_) const
{
	int iValue;
	const bool bFind = rOpenOption_.getInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key),
		iValue);
	//- この関数はRead/Search専用
	if (bFind
	 && iValue != FileCommon::OpenOption::OpenMode::Read
	 && iValue != FileCommon::OpenOption::OpenMode::Search)
	{
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	//- オープンモードが設定されていなければscanモードを設定
	if (!bFind)
	{
		rOpenOption_.setInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::OpenMode::Key),
			_SYDNEY_OPEN_PARAMETER_VALUE(
				FileCommon::OpenOption::OpenMode::Read));
		rOpenOption_.setInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::ReadSubMode::Key),
			_SYDNEY_OPEN_PARAMETER_VALUE(
				FileCommon::OpenOption::ReadSubMode::Scan));
	}

	bool isBitSet = rOpenOption_.getBoolean(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::GetByBitSet::Key));
	FileCommon::ListNodeWrapper node(pNode_);
	int num = node.getSize();
	if (isBitSet && num != 1)
		return false;

	Common::IntegerArrayData cProjection;
	
	for (int i = 0; i < num; ++i)
	{
		const LogicalFile::TreeNodeInterface* p = node.get(i);
		int n = -1;
		
		if (p->getType() == LogicalFile::TreeNodeInterface::Field)
		{
			n = FileCommon::DataManager::toInt(p);
			
			if (isBitSet)
			{
				Common::DataType::Type eBitSetFieldType
					= m_cFileParameter.getDataTypeForOuterFieldID(n);
				if (eBitSetFieldType != Common::DataType::UnsignedInteger)
					return false;
			}
		}
		else if (p->getType() == LogicalFile::TreeNodeInterface::Count)
		{
			n = m_cFileParameter.getOuterFieldNumber();
		}
		else
		{
			return false;
		}

		cProjection.setElement(i, n);
	}

	// rOpenOption_に値を実際に設定する
	return setProjectionOptions(cProjection, rOpenOption_, false);
}

//
//	FUNCTION public
//	Vector::File::getUpdateParameter -- 更新オープンパラメータを設定する
//
//	NOTES
//	更新モードでのベクタファイルオープンオプションを設定する。
//	ベクタファイルは、引数 cUpdateFields_で指定されている
//	1つ以上のフィールド番号を読みとり、
//	オブジェクト更新時には、該当するフィールドのみを
//	更新するようにオープンオプションを設定する。
//
//	ARGUMENTS
//	const Common::IntegerArrayData&	cUpdateFields_
//		フィールド番号の配列オブジェクトへの参照
//	LogicalFile::OpenOption&		rOpenOption_
//		オープンオプションオブジェクトへの参照
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//	BadArgument
//		オープンモードが正しくない
//
bool
File::getUpdateParameter(
	const Common::IntegerArrayData&	rProjection_,
	LogicalFile::OpenOption&		rOpenOption_) const
{
	// この関数はupdateモード専用
	int iValue;
	const bool bFind = rOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), iValue);
	if (bFind && iValue != FileCommon::OpenOption::OpenMode::Update) {
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	// オープンモードをUpdateに設定
	if (!bFind) {
		rOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), _SYDNEY_OPEN_PARAMETER_VALUE(FileCommon::OpenOption::OpenMode::Update));
	}

	// rOpenOption_に値を実際に設定する
	return setProjectionOptions(rProjection_, rOpenOption_, true);
}

//
//	FUNCTION public
//	Vector::File::getSortParameter -- ソート順パラメータを設定する
//
//	NOTES
//	現状では常にfalseを返す
//
//	ARGUMENTS
//	const Common::IntegerArrayData&	cKeys_
//		ソート順を指定するフィールドインデックスの列への参照
//	const Common::IntegerArrayData&	cOrders_
//		引数cKeys_で指定されたフィールドのソート順の列への参照
//		昇順ならば0を、降順ならば1を設定する。
//	LogicalFile::OpenOption&	rOpenOption_
//		ベクタファイルオープンオプションオブジェクトへの参照
//
//	RETURN
//	bool
//		ベクタキーフィールドのみのソート順が
//		指定されている場合には true を、
//		ベクタキーフィールド以外のフィールドのソート順が
//		指定されている場合には false を返す。
//-		しかし現状では常にfalseを返す。
//
//	EXCEPTIONS
//		なし
//
bool
File::getSortParameter(
	const Common::IntegerArrayData& /*cKeys_*/,
	const Common::IntegerArrayData& /*cOrders_*/,
	LogicalFile::OpenOption&	/*rOpenOption_*/) const
{
#if 0
	// 不正な引数
	if (cKeys_.getCount() != cOrders_.getCount()) {
		return false;
		// throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}
#endif

	return false;
}

//
//	FUNCTION public
//	Vector::File::create -- ベクタファイルを生成する
//
//	NOTES
//	ベクタファイルは、自身がもつ物理ファイルを生成し、
//	生成した物理ファイルの初期化を行なう。
//
//	ARGUMENTS
//	const Trans::Transaction& rTransaction_
//		トランザクション記述子
//
//	RETURN
//	const LogicalFile::FileID&
//		自身がもつ論理ファイルIDオブジェクトへの参照
//
//	EXCEPTIONS
//	YET!
//		ファイルオプションに設定されるべきパラメータが設定されていない
//
const LogicalFile::FileID&
File::create(const Trans::Transaction& rTransaction_)
{
	// FileIDをセットするのみ
	m_cFileParameter.setMounted();
	m_cFileParameter.setVersion(CurrentVersion);

	return m_cFileParameter.getFileOption();
}

//	FUNCTION public
//	Vector::File::destroy -- ベクタファイルを破棄する
//
//	NOTES
//	自身がもつ物理ファイルを破棄する。
//
//	ARGUMENTS
//	const Trans::Transaction& rTransaction_
//		トランザクション記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
File::destroy(const Trans::Transaction& rTransaction_)
{
	_SYDNEY_FAKE_ERROR("Vector::File::destroy",Exception::BadArgument(moduleName, srcFile, __LINE__));

	// マウントの有無や実体の存在の有無を確認せずに
	// とにかく削除する
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	FileCommon::AutoPhysicalFile file(
		m_cFileParameter.getStorageStrategy(),
		m_cFileParameter.getBufferingStrategy(),
		m_cFileParameter.getLockName());

	file->destroy(rTransaction_);
}

//	FUNCTION
//	Vector::File::mount -- 物理ファイルをマウントする
//
//	NOTE
//		物理ファイルの UNMOUNT を実行した後にエラーが起きた場合、
//		UNMOUNT は取り消す必要は無く、適切な例外を送出するだけでよい。
//
//	ARGUMENTS
//		const Trans::Transaction& rTransaction_
//			
//	RETURN
//		なし
//
//	EXCEPTIONS

const LogicalFile::FileID&
File::mount(const Trans::Transaction& rTransaction_)
{
	if (!isMounted(rTransaction_)) {

		// マウントされていなければ、マウントしてみる

		_SYDNEY_FAKE_ERROR("Vector::File::mount",Exception::BadArgument(moduleName, srcFile, __LINE__));
		{
		FileCommon::AutoPhysicalFile file(
			m_cFileParameter.getStorageStrategy(),
			m_cFileParameter.getBufferingStrategy(),
			m_cFileParameter.getLockName());

		file->mount(rTransaction_);
		}
		// マウントされたことをファイル識別子に記録する

		m_cFileParameter.setMounted();
	}
	return m_cFileParameter.getFileOption();
}

//	FUNCTION
//	Vector::File::unmount -- 物理ファイルをアンマウントする
//
//	NOTE
//		物理ファイルの UNMOUNT を実行した後にエラーが起きた場合、
//		UNMOUNT は取り消す必要は無く、適切な例外を送出するだけでよい。
//
//	ARGUMENTS
//		const Trans::Transaction& rTransaction_
//			
//	RETURN
//		なし
//
//	EXCEPTIONS

const LogicalFile::FileID&
File::unmount(const Trans::Transaction& rTransaction_)
{
	_SYDNEY_FAKE_ERROR("Vector::File::unmount",Exception::BadArgument(moduleName, srcFile, __LINE__));

	// マウントの有無や実体の存在の有無を確認せずに
	// とにかくアンマウントする
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない
	{
	FileCommon::AutoPhysicalFile file(
		m_cFileParameter.getStorageStrategy(),
		m_cFileParameter.getBufferingStrategy(),
		m_cFileParameter.getLockName());

	file->unmount(rTransaction_);
	}
	// アンマウントされたことをファイル識別子に記録する

	m_cFileParameter.unsetMounted();

	return m_cFileParameter.getFileOption();
}

//
//	FUNCTION
//	Vector::File::flush -- 物理ファイルをフラッシュする。
//
//	NOTE
//		物理ファイルをフラッシュする。
//		物理ファイルの flush を実行した後にエラーが起きた場合
//		flush は取り消す必要は無く、適切な例外を送出するだけでよい。
//
//	ARGUMENTS
//		const Trans::Transaction& rTransaction_
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//
void
File::flush(const Trans::Transaction& rTransaction_)
{
	if (!isMounted(rTransaction_))
		return;

	_SYDNEY_FAKE_ERROR("Vector::File::flush",Exception::BadArgument(moduleName, srcFile, __LINE__));
	FileCommon::AutoPhysicalFile file(m_cFileParameter.getStorageStrategy() ,m_cFileParameter.getBufferingStrategy() ,m_cFileParameter.getLockName());
	// 物理ファイルを破棄する
	file->flush(rTransaction_);
}

//
//	FUNCTION
//	Vector::File::startBackup -- 物理ファイルのバックアップの開始を通知する。
//
//	NOTE
//		物理ファイルのバックアップの開始を通知する。
//		物理ファイルの START BACKUP を実行した後にエラーが起きた場合
//		START BACKUP は END BACKUP によって取り消すことができる。
//		取り消しが完了したら適切な例外を送出する。
//
//	ARGUMENTS
//		const Trans::Transaction& rTransaction_
//		const bool bRestorable_
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//
void
File::startBackup(const Trans::Transaction& rTransaction_,
				  const bool bRestorable_)
{
	if (!isMounted(rTransaction_))
		return;

	_SYDNEY_FAKE_ERROR("Vector::File::startBackup",Exception::BadArgument(moduleName, srcFile, __LINE__));
	FileCommon::AutoPhysicalFile file(m_cFileParameter.getStorageStrategy() ,m_cFileParameter.getBufferingStrategy() ,m_cFileParameter.getLockName());

	file->startBackup(rTransaction_, bRestorable_);
}

//
//	FUNCTION
//	Vector::File::endBackup -- 物理ファイルのバックアップの終了を通知する。
//
//	NOTE
//		物理ファイルのバックアップの終了を通知する。
//		物理ファイルの END BACKUP を実行した後にエラーが起きた場合
//		END BACKUP は取り消せないので、FATAL なエラーを返さなければいけない。
//
//	ARGUMENTS
//		const Trans::Transaction& rTransaction_
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//
void
File::endBackup(const Trans::Transaction& rTransaction_)
{
	if (!isMounted(rTransaction_))
		return;

	try {
		_SYDNEY_FAKE_ERROR("Vector::File::endBackup",Exception::BadArgument(moduleName, srcFile, __LINE__));
		FileCommon::AutoPhysicalFile file(m_cFileParameter.getStorageStrategy() ,m_cFileParameter.getBufferingStrategy() ,m_cFileParameter.getLockName());
		file->endBackup(rTransaction_);
	} catch (...) {

		// 元に戻せなかったので、利用不可にする

		Checkpoint::Database::setAvailability(
			m_cFileParameter.getLockName(), false);
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION
//	Vector::File::recover -- ベクタファイルを障害から回復する
//
//	NOTE
//		ベクタファイルを障害から回復する。
//
//	ARGUMENTS
//		const Trans::Transaction& rTransaction_
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//
void
File::recover(const Trans::Transaction& rTransaction_,
			  const Trans::TimeStamp&	rPoint_)
{
	if (!isMounted(rTransaction_))
		return;

	// 物理ファイル格納戦略、バッファリング戦略を得て物理ファイルをアタッチする
	_SYDNEY_FAKE_ERROR("Vector::File::recover",Exception::BadArgument(moduleName, srcFile, __LINE__));
	FileCommon::AutoPhysicalFile file(m_cFileParameter.getStorageStrategy() ,m_cFileParameter.getBufferingStrategy() ,m_cFileParameter.getLockName());
	// 物理ファイルをrecoverする
	file->recover(rTransaction_, rPoint_);
}

//
//	FUNCTION
//	Vector::File::restore -- 
//
//	NOTE
//		ある時点に開始された読取専用トランザクションが
//		参照する版を最新版とする
//
//	ARGUMENTS
//		const Trans::Transaction& rTransaction_
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//
void
File::restore(const Trans::Transaction& rTransaction_,
			  const Trans::TimeStamp&	rPoint_)
{
	if (!isMounted(rTransaction_))
		return;

	// 物理ファイル格納戦略、バッファリング戦略を得て物理ファイルをアタッチする
	_SYDNEY_FAKE_ERROR("Vector::File::restore",Exception::BadArgument(moduleName, srcFile, __LINE__));
	FileCommon::AutoPhysicalFile file(m_cFileParameter.getStorageStrategy() ,m_cFileParameter.getBufferingStrategy() ,m_cFileParameter.getLockName());
	// 物理ファイルをrestoreする
	file->restore(rTransaction_, rPoint_);
}

//
//	FUNCTION public
//	Vector::File::verify -- 整合性検査を行う
//
//	NOTES
//	整合性検査を行う。
//-	課題:
//-	せっかく各クラスで宣言している定数を使えず、
//-	マジックナンバーをハードコーディングしてしまっている部分が多い。
//-	ObjectやFileInformationアタッチと物理ページとのアタッチとを
//- 分離した方が良さそう。
//
//	ARGUMENTS
//	const Trans::Transaction&						rTransaction_
//		トランザクション記述子への参照
//	const Version::Verification::Treatment::Value	eTreatment_
//		整合性検査の検査方法
//	Admin::Verification::Progress&					rProgress_
//		整合性検査の進行状況
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::verify(const Trans::Transaction&						rTransaction_,
			 const Admin::Verification::Treatment::Value	eTreatment_,
			 Admin::Verification::Progress&					rProgress_)
{
	if (!isMounted(rTransaction_) || !rProgress_.isGood())
		return;

	_SYDNEY_FAKE_ERROR("Vector::File::verify_1",Exception::BadArgument(moduleName, srcFile, __LINE__));
	FileCommon::AutoPhysicalFile file(m_cFileParameter.getStorageStrategy() ,m_cFileParameter.getBufferingStrategy() ,m_cFileParameter.getLockName());

	try
	{
		// 開始通知
		file->startVerification
			(rTransaction_, eTreatment_, rProgress_);

		ModUInt32 ulPageID = 0;
		ModUInt32 ulTotalObjectCount = 0;
		ModUInt32 ulFirstPageID;

		// 各々の物理ページのオブジェクト数を検査
		while((ulPageID = file->getNextPageID(rTransaction_, ulPageID))
				!= PhysicalFile::ConstValue::UndefinedPageID)
		{
			if (ulPageID == 0) { ulFirstPageID = ulPageID; }

			// エラーが発生したら中断
			if (! rProgress_.isGood()) break;

			// 使用を通知
			file->notifyUsePage(rTransaction_, rProgress_, ulPageID);

			_SYDNEY_FAKE_ERROR("Vector::File::verify_2",Exception::MemoryExhaust(moduleName, srcFile, __LINE__));
			FileCommon::AutoPhysicalPageVerify pPage( file ,rTransaction_, ulPageID, Buffer::Page::FixMode::ReadOnly, rProgress_);
			// ページごとのオブジェクト数を検査
			ulTotalObjectCount += verifyPageCount(pPage, ulPageID, rProgress_);
		}

		//オブジェクト数が正常なら、次工程へ。
		if (rProgress_.isGood())
		{
			// ヘッダページの使用を通知
			file->notifyUsePage(rTransaction_, rProgress_, 0);

			_SYDNEY_FAKE_ERROR("Vector::File::verify_3",Exception::MemoryExhaust(moduleName, srcFile, __LINE__));
			FileCommon::AutoPhysicalPageVerify pPage( file
			                                         ,rTransaction_
			                                         ,0// ヘッダページのIDは0
			                                         ,Buffer::Page::FixMode::ReadOnly
			                                         ,rProgress_
			                                        );
			FileInformation cFI
				(pPage, Buffer::Page::FixMode::ReadOnly);
		
			//- Obj総数の検査
			if (cFI.getObjectCount() != ulTotalObjectCount)
			{
#ifdef DEBUG
				SydDebugMessage << "INCONSISTENT!" << ModEndl;
#endif
				_SYDNEY_VERIFY_INCONSISTENT(rProgress_,
					_TRMEISTER_U_STRING((m_cFileParameter.
						getStorageStrategy()).m_VersionFileInfo._path._masterData),
					Message::CorruptTotalObjectCount
						(cFI.getObjectCount(), ulTotalObjectCount));
			}

			if (rProgress_.isGood())
			{						
				//- 先頭VKの検査
				verifyFirstVectorKey(rTransaction_, file.get(), 
					cFI.getFirstVectorKey(), rProgress_);
			}

			if (rProgress_.isGood())
			{
				//- 最終VKの検査
				verifyLastVectorKey(rTransaction_, file.get(), 
					cFI.getLastVectorKey(), rProgress_);
			}
		}

		// 終了通知
		file->endVerification(rTransaction_, rProgress_);
	}
	catch (...)
	{
		// 検査内のどこであってもここに到達するはず

		_SYDNEY_VERIFY_ABORTED(rProgress_,
			_TRMEISTER_U_STRING((m_cFileParameter.
				getStorageStrategy()).m_VersionFileInfo._path._masterData),
			Message::CaughtException());

		file->endVerification(rTransaction_, rProgress_);
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Vector::File::open -- ベクタファイルをオープンする
//
//	NOTES
//	ベクタファイルをオープンする。
//
//	ARGUMENTS
//	const Trans::Transaction& rTransaction_
//		トランザクション記述子
//	const LogicalFile::OpenOption& rOpenOption_
//		オープンオプションオブジェクトへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	NotSupported
//		既にオープンされている
//		オープンモードが間違っている
//
void
File::open(
	const Trans::Transaction&	rTransaction_,
	const LogicalFile::OpenOption&	rOpenOption_)
{
	// substantiateで使うためにポインターを保持
	m_pTransaction = &rTransaction_;

	// 既にオープンされているか？
	if (isOpen()) {
		// オープンの回数は1回まで。それ以上はサポートしていない。
		throw Exception::NotSupported(moduleName, srcFile, __LINE__);
	}

	SydAssert(m_pPageManager == 0);
	SydAssert(m_pObjectIterator == 0);

	// オープンパラメータをセット
	m_pOpenParameter = new OpenParameter
		(rOpenOption_, m_cFileParameter.getOuterFieldNumber());

	// オープンモードに従ってObjectIteratorを選択し、代入する
	const FileCommon::OpenMode::Mode eOpenMode = m_pOpenParameter->getOpenMode();
	int mode;
	if (eOpenMode == FileCommon::OpenMode::Initialize) {
		// nop 何もしない
		delete m_pOpenParameter; m_pOpenParameter = 0;
		return;
	} else if (eOpenMode == FileCommon::OpenMode::Read) {
		mode = 0;
	} else if (eOpenMode == FileCommon::OpenMode::Search) {
		mode = 1;
	} else if (eOpenMode == FileCommon::OpenMode::Update) {
		mode = 2;
	} else if (eOpenMode == FileCommon::OpenMode::Batch) {
		mode = 2; // use same iterator as Update
	} else {
		// OpenModeが間違っている
		delete m_pOpenParameter; m_pOpenParameter = 0;
		SydErrorMessage << "wrong openmode."  <<ModEndl;
		throw Exception::NotSupported(moduleName, srcFile, __LINE__);
	}

	// 物理ファイルをオープン
	PhysicalFile::File* pPhysicalFile = PhysicalFile::Manager::attachFile(m_cFileParameter.getStorageStrategy(), m_cFileParameter.getBufferingStrategy(), m_cFileParameter.getLockName());

	m_cFileParameter.initializeBlockParameters(pPhysicalFile->getPageDataSize());

	// ページマネージャをセット
	m_pPageManager = new PageManager
		(rTransaction_, m_cFileParameter, m_pOpenParameter, pPhysicalFile);

	switch (mode) {
	default:
		SydAssert(false);//ありえない
	case 0:
		{
		const FileCommon::ReadSubMode::Mode eReadSubMode = 
			m_pOpenParameter->getReadSubMode();
		if (eReadSubMode == FileCommon::ReadSubMode::ScanRead) {
				m_pObjectIterator = new ScanIterator(
					m_cFileParameter,
					*m_pOpenParameter,
					*m_pPageManager);
		} else if (eReadSubMode == FileCommon::ReadSubMode::FetchRead) {
				m_pObjectIterator = new FetchIterator(
					m_cFileParameter,
					*m_pOpenParameter,
					*m_pPageManager);
		} else {
			// ReadSubModeが間違っている
			SydErrorMessage << "wrong readsubmode." << ModEndl;
			throw Exception::NotSupported(moduleName, srcFile, __LINE__);
		}		
		}
		break;
	case 1:
		{
			m_pObjectIterator = new SearchIterator(
					m_cFileParameter,
					*m_pOpenParameter,
					*m_pPageManager,
					 m_pOpenParameter->getSearchValue());
		}
		break;
	case 2:
		{
			m_pObjectIterator = new UpdateIterator(
					m_cFileParameter,
					*m_pOpenParameter,
					*m_pPageManager,
					rTransaction_);
		}
		break;
	}

	// 読み取り専用トランザクションで、getCount() の返り値をキャッシュする準備
	m_eTransactionCategory = rTransaction_.getCategory();
	m_iCountCache = -1;//unknown
}

//
//	FUNCTION public
//	Vector::File::close -- ベクタファイルをクローズする
//
//	NOTES
//	ベクタファイルをクローズする。
//  注意:
//   - クローズしてもファイルオプション(メタデータ)は変化しない。
//   - クローズするとオープンオプションは変化する。
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
File::close()
{
	// 読み取り専用トランザクションで、getCount() の返り値をキャッシュする準備
	m_eTransactionCategory = Trans::Transaction::Category::Unknown;

	if (!isOpen()) { // オープンされていなかった
		return;
	}

	// キャッシュしているページをdetachする
	m_pPageManager->close();	

	// 物理ファイルの解放 (2000-01-22追加)
	PhysicalFile::Manager::detachFile(m_pPageManager->getFile());
	// リソースの解放
	delete m_pObjectIterator, m_pObjectIterator = 0;
	delete m_pOpenParameter, m_pOpenParameter = 0;
	delete m_pPageManager, m_pPageManager = 0;

	// substantiateで使うためのポインターをクリア
	m_pTransaction = 0;
}

//
//	FUNCTION public
//	Vector::File::fetch -- 検索条件であるベクタキーを設定する
//
//	NOTES
//	検索条件であるベクタキーを設定する。
//	データはget()で求める。
//
//	ARGUMENTS
//	const Common::DataArrayData* pOption_
//		オプションオブジェクトへのポインタ(省略可)。
//		ベクタファイルの fetch へのオプションは、ベクタキーである。
//		引数は、ベクタキーを値としてもつLogicalFile::VectorKey
//		( == Common::UnsignedInteger32Data)クラスのインスタンスか、
//		それを唯一の要素として持つCommon::DataArrayDataクラスの
//		インスタンスでなければならない。
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	FileNotOpen
//		ベクタファイルがオープンされていない

void
File::fetch(const Common::DataArrayData* pOption_)
{
	if (!isOpen()) {
		// ベクタファイルがオープンされていない
		throw Exception::FileNotOpen(moduleName, srcFile, __LINE__);
	}

	; _SYDNEY_ASSERT(m_pTransaction);

	ModUInt32 ulVectorKey = getVectorKeyFromObject(pOption_);
	m_pObjectIterator->fetch(ulVectorKey);
}

//
//	FUNCTION public
//	Vector::File::get -- ベクタファイルに挿入されているオブジェクトを返す
//
//	NOTES
//	ベクタファイルに挿入されているオブジェクトを返す。
//	カーソルがさしているオブジェクトを返す。
//	fetch で指定されたベクタキーが存在しないものであった場合 0 を返す。
//
//  注意! 返り値は関数内でnewして生成しているので、
//  これを利用する者は責任を持って結果をdeleteすること!
//
//	ARGUMENTS
//	Common::DataArrayData* pTuple_
//		値を格納する配列
//
//	RETURN
//	bool
//		値が得られた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	FileNotOpen
//		ファイルがオープンされていない

bool
File::get(Common::DataArrayData* pTuple_)
{
	// ベクタファイルがオープンされているかチェック
	if (!isOpen()) {
		throw Exception::FileNotOpen(moduleName, srcFile, __LINE__);
	}

	; _SYDNEY_ASSERT(m_pTransaction);
	; _SYDNEY_ASSERT(pTuple_);

	if (m_pOpenParameter->isGetCount()) {
		// COUNTを得る特殊列が指定されている

		if (m_pOpenParameter->isGottenCount())
			// 一度得たらそれ以降はfalse
			return false;

		ModInt64 iCount = getCount();
		switch (pTuple_->getElement(0)->getType()) {
		case Common::DataType::UnsignedInteger:
			{
				// (2005/01/25) UnsignedInteger以外は来ない
				_SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData&, *pTuple_->getElement(0)).setValue(static_cast<unsigned int>(iCount));
				break;
			}
		default:
			{
				; _SYDNEY_ASSERT(false);
				_SYDNEY_THROW0(Exception::Unexpected);
			}
		}
		m_pOpenParameter->setGottenCount();
		return true;
	}

	// getCount() == 0が返るような場合でも
	// ScanIterator::getはヘッダーページを読んで0を返すことができるし、
	// FetchIterator::getは(通常はgetCount() == 0のときに呼ばれるはずはないが)
	// 最大ページIDを調べて0を返すことができる。
	// したがって毎回ヘッダーページをアタッチしてまでgetCountを調べる理由は毛頭ない。
//	if (!isMounted(*m_pTransaction) || getCount() == 0)
	if (!isMounted(*m_pTransaction))

		// ファイルが空ならばnullを返す

		return false;

	try {
		return m_pObjectIterator->get(pTuple_);
	} catch (...) {
		// 例外の経路をトレース
		SydDebugMessage << "Rethrow EXCEPTION" << ModEndl;
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Vector::File::insert -- ベクタファイルにオブジェクトを挿入する
//
//	NOTES
//	ベクタファイルにオブジェクトを挿入する。
//
//	ARGUMENTS
//	Common::DataArrayDataData* pTuple_
//		ベクタファイルへ挿入するオブジェクトなどへのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	FileNotOpen
//		ベクタファイルがオープンされていない

void
File::insert(Common::DataArrayData* pTuple_)
{
	if (!isOpen()) {
		// ベクタファイルがオープンされていない
		throw Exception::FileNotOpen(moduleName, srcFile, __LINE__);
	}

	// 実体を作る
	substantiate();

	; _SYDNEY_ASSERT(m_pTransaction);
	; _SYDNEY_ASSERT(isMounted(*m_pTransaction));

	// 引数の検査
	Common::DataArrayData* pArrayData;
	ModUInt32 ulVectorKey(FileCommon::VectorKey::Undefined);
	checkInsertArgument(pTuple_, ulVectorKey, pArrayData);
	Common::DataArrayData cArrayData = *pArrayData;
	cArrayData.popFront(); // popFrontの返り値はvoidであることに注意

	m_pObjectIterator->insert(ulVectorKey, cArrayData);
}

//
//	FUNCTION public
//	Vector::File::update -- ベクタファイルのオブジェクトを更新する
//
//	NOTES
//	ベクタファイルのオブジェクトを更新する。
//
//	ARGUMENTS
//	const Common::DataArrayData*	pKey_
//		更新するオブジェクトを指定するベクタキー
//		pObject_ の先頭の要素にも設定されている
//	Common::DataArrayData*			pObject_
//		更新するオブジェクトなどへのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	FileNotOpen
//		ベクタファイルがオープンされていない

void
File::update(const Common::DataArrayData* pKey_, Common::DataArrayData* pTuple_)
{
	if (!isOpen()) {
		SydErrorMessage << "file is not open." << ModEndl;
		throw Exception::FileNotOpen(moduleName, srcFile, __LINE__);
	}

	; _SYDNEY_ASSERT(m_pTransaction);
	; _SYDNEY_ASSERT(isMounted(*m_pTransaction));

	// 引数 pTuple_ を検査。
	// 検査に合格するとベクタキーとフィールド値の配列が得られる
	ModUInt32 ulVectorKey = FileCommon::VectorKey::Undefined;
	Common::DataArrayData* pArrayData = 0;
	checkUpdateArgument(pKey_, pTuple_, ulVectorKey, pArrayData);

	m_pObjectIterator->update(ulVectorKey, *pArrayData);
}

//
//	FUNCTION public
//	Vector::File::expunge -- ベクタファイルのオブジェクトを削除する
//
//	NOTES
//	ベクタファイルのオブジェクトを削除する。
//
//	ARGUMENTS
//	const Common::Data*			pKey_
//		削除するオブジェクトを指定するベクタキー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	FileNotOpen
//		ベクタファイルがオープンされていない

void
File::expunge(const Common::DataArrayData* pKey_)
{
	if (!isOpen()) {
		// ベクタファイルがオープンされていない
		throw Exception::FileNotOpen(moduleName, srcFile, __LINE__);
	}

	; _SYDNEY_ASSERT(m_pTransaction);
	; _SYDNEY_ASSERT(isMounted(*m_pTransaction));

	// 引数 pObject_ を検査。
	// 検査に合格すると 削除するオブジェクトのベクタキーが得られる。
	ModUInt32 ulVectorKey = checkDeleteArgument(pKey_);

	m_pObjectIterator->expunge(ulVectorKey);
}


//
//	FUNCTION public
//	Vector::File::mark -- 巻き戻しの位置を記録する
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
File::mark()
{
	; _SYDNEY_ASSERT(m_pTransaction);
	; _SYDNEY_ASSERT(isMounted(*m_pTransaction));

	// ベクタファイルがオープンされていない
	// (markは数多く呼ばれるのでチェックはassertで済ます)
	SydAssert(m_pObjectIterator != 0);

	// 記録している位置にカーソルを戻す
	m_pObjectIterator->mark();
}

//
//	FUNCTION public
//	Vector::File::rewind -- 記録した位置に戻る
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
//	FileNotOpen
//		ベクタファイルがオープンされていない
//
void
File::rewind()
{
	if (!isOpen()) {
		// ベクタファイルがオープンされていない
		throw Exception::FileNotOpen(moduleName, srcFile, __LINE__);
	}

	; _SYDNEY_ASSERT(m_pTransaction);
	; _SYDNEY_ASSERT(isMounted(*m_pTransaction));

	// get されないのに rewind が呼ばれることはない
	SydAssert(m_pObjectIterator != 0);

	// 記録している位置にカーソルを戻す
	m_pObjectIterator->rewind();
}

//
//	FUNCTION public
//	Vector::File::reset -- カーソルをリセットする
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
//	FileNotOpen
//		ベクタファイルがオープンされていない
//
void
File::reset()
{
	if (!isOpen()) {
		// ベクタファイルがオープンされていない
		throw Exception::FileNotOpen(moduleName, srcFile, __LINE__);
	}

	; _SYDNEY_ASSERT(m_pTransaction);
	// 空でもresetされる場合があるので以下のアサートはやめる
	//; _SYDNEY_ASSERT(isMounted(*m_pTransaction));

	if (m_pOpenParameter->isGetCount()) {
		// COUNTを得る特殊列が指定されている -> 取得したフラグをクリアする
		m_pOpenParameter->clearGottenCount();
		return;
	}

	if (m_pObjectIterator == 0) {
		return;
	}

	m_pObjectIterator->reset();
}

//	FUNCTION public
//	Vector::File::equals -- 比較
//
//	NOTES
//	自身と引数 pOther_ の比較を行ない、比較結果を返す。
//	※ 同一オブジェクトかをチェックするのではなく、
//	   それぞれがもつメンバが等しいか（同値か）をチェックする。
//  ※ すべての値を比較する訳ではないことに注意。
//  
//	ARGUMENTS
//	const Common::Object*	pOther_
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
File::equals(const Common::Object* pOther_) const
{
	// 引数 pOther_ がヌルポインタならば等しくない
	if (pOther_ == 0) {
		return false;
	}

	// 引数 pOther_ が Vector::File クラスの
	// インスタンスオブジェクトでなければ等しくない
	const Vector::File*	pOther = dynamic_cast<const Vector::File*>(pOther_);//多重継承のため dynamic_cast は残す
	if (pOther == 0) {
		return false;
	}

	// ファイルパラメータが異なっていれば、両者は異なっているとみなす
	// (「ファイルパラメータが異なっている」という判定は
	// FileParameterクラスの実装に依存している)
	if (!m_cFileParameter.equals(pOther->m_cFileParameter)) {
		return false;
	}

	return true;
}

//	FUNCTION public
//	Vector::File::sync -- ベクタファイルの同期をとる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			ベクタファイルの同期を取る
//			トランザクションのトランザクション記述子
//		bool&				incomplete
//			true
//				今回の同期処理でベクタファイルを持つ
//				オブジェクトの一部に処理し残しがある
//			false
//				今回の同期処理でベクタファイルを持つ
//				オブジェクトを完全に処理してきている
//
//				同期処理の結果、ベクタファイルを処理し残したかを設定する
//		bool&				modified
//			true
//				今回の同期処理でベクタファイルを持つ
//				オブジェクトの一部が既に更新されている
//			false
//				今回の同期処理でベクタファイルを持つ
//				オブジェクトはまだ更新されていない
//
//				同期処理の結果、ベクタファイルが更新されたかを設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::sync(const Trans::Transaction& trans, bool& incomplete, bool& modified)
{
	_SYDNEY_FAKE_ERROR("Vector::File::sync",Exception::BadArgument(moduleName, srcFile, __LINE__));

	if (isMounted(trans))
		FileCommon::AutoPhysicalFile(
			m_cFileParameter.getStorageStrategy(),
			m_cFileParameter.getBufferingStrategy(),
			m_cFileParameter.getLockName())->sync(trans, incomplete, modified);
}

//	Utilities

//
//	FUNCTION
//	Vector::File::move -- ベクタファイルを物理的に移動する
//
//	NOTE
//		ベクタファイルを物理的に移動する
//
//	ARGUMENTS
//		const Trans::Transaction&		rTransaction_
//			
//		const Common::StringArrayData&	cArea_
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//			
void
File::move(
	const Trans::Transaction&		rTransaction_,
	const Common::StringArrayData&	cArea_)
{
	const ModUnicodeString& cstrOrigPath = m_cFileParameter.getDirectoryPath();
	const ModUnicodeString& cstrNewPath = cArea_.getElement(0);

	// パスが異なるときのみ実行する
	if (Os::Path::compare(
			cstrOrigPath, cstrNewPath) == Os::Path::CompareResult::Unrelated) {

		_SYDNEY_FAKE_ERROR("Vector::File::move",Exception::BadArgument(moduleName, srcFile, __LINE__));
		FileCommon::AutoPhysicalFile file(m_cFileParameter.getStorageStrategy() ,m_cFileParameter.getBufferingStrategy() ,m_cFileParameter.getLockName());

		try {
			m_cFileParameter.setDirectoryPath(cstrNewPath);
			file->move(rTransaction_, m_cFileParameter.getStorageStrategy().m_VersionFileInfo._path);

		} catch (...) {
			//元に戻す
			m_cFileParameter.setDirectoryPath(cstrOrigPath);
			try {
				file->move(rTransaction_, m_cFileParameter.getStorageStrategy().m_VersionFileInfo._path);
			} catch (...) {

				// 元に戻せなかったので、利用不可にする

				Checkpoint::Database::setAvailability(
					m_cFileParameter.getLockName(), false);
			}
			_SYDNEY_RETHROW;
		}
	}
	return;
}

// FUNCTION public
//	Vector::File::getNoLatchOperation -- ラッチが不要なオペレーションを返す
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
File::
getNoLatchOperation()
{
	return Operation::Open
		| Operation::Close
		| Operation::Reset
		| Operation::GetProcessCost
		| Operation::GetOverhead
		| Operation::Fetch;
}

//
//	FUNCTION
//	Vector::File::toString -- Fileクラスのオブジェクトを文字列に変換する
//
//	NOTE
//		Fileクラスのオブジェクトを文字列に変換する
//
//	ARGUMENTS
//		なし
//			
//	RETURN
//		ModUnicodeString
//
//	EXCEPTIONS
//		なし
//			
ModUnicodeString
File::toString() const
{
	return ModUnicodeString
		(m_cFileParameter.getStorageStrategy().m_VersionFileInfo._path._masterData);
}

// PRIVATE FUNCTIONS

//
//	FUNCTION
//	Vector::File::setProjectionOptions -- rOpenOption_に値を実際に設定する
//
//	NOTE
//		rOpenOption_に値を実際に設定する
//
//	ARGUMENTS
//		const Common::IntegerArrayData&	rProjection_
//			
//		LogicalFile::OpenOption&		rOpenOption_
//			
//		const bool bUpdateMode_
//			
//	RETURN
//		bool
//
//	EXCEPTIONS
//		Exception::BadArgument
//			
bool
File::setProjectionOptions(
	const Common::IntegerArrayData&	rProjection_,
	LogicalFile::OpenOption&		rOpenOption_,
	const bool bUpdateMode_) const
{
	// フィールド選択が指定されていることを設定する
	rOpenOption_.setBoolean(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::FieldSelect::Key), true);

	// フィールド選択指定を設定する
	const int iFieldNum = rProjection_.getCount();
	if (iFieldNum <= 0) {
		return false;
		//- throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	rOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::TargetFieldNumber::Key), iFieldNum);
	for (int i = 0; i < iFieldNum; ++i) {
		// Updateの場合、第0フィールド(VectorKey)は変更不能
		if (bUpdateMode_ && rProjection_.getElement(i) == 0) {
			return false;
		}

		// オープンオプションに選択されているフィールドの番号を設定
		rOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(FileCommon::OpenOption::TargetFieldIndex::Key, i), rProjection_.getElement(i));
	}

	return true;
}

//
//	FUNCTION private
//	Vector::File::getVectorKeyFromObject --
//		fetch()へのオプションからベクタキーを取得
//
//	NOTES
//	引数を解析し、設定されているベクタキーを返す。
//
//	ARGUMENTS
//	const Common::DataArrayData*	pOption_
//		Vector::File::fetch へのオプション
//
//	RETURN
//	ModUInt32
//		ベクタキー
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//
ModUInt32
File::getVectorKeyFromObject(const Common::DataArrayData* pOption_) const
{
	// 要素の型はLogicalFile::VectorKeyでなければならない。
	const Common::Data* pData = pOption_->getElement(0).get();
	if (!TypeID::isVectorKey(pData)) {
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	//	設定されているベクタキーを返す。
	const LogicalFile::VectorKey* pInt32Data = 
		_SYDNEY_DYNAMIC_CAST(const LogicalFile::VectorKey* ,pData);
	_SYDNEY_ASSERT(pInt32Data);
	return pInt32Data->getValue();
}

//
//	FUNCTION private
//	Vector::File::checkInsertArgument -- 引数検査
//
//	NOTES
//	挿入のための引数に異常がみつかれば例外 BadArgument を送出する。
//	検査に合格すると pObject をキャストした値を返す。
//
//	ARGUMENTS
//	Common::DataArrayData*	pObject_
//		挿入に必要な情報を格納したオブジェクトへのポインタ
//	ModUInt32				ulVectorKey_
//		得られたベクタキーのホルダ(事実上の返り値)
//	Common::DataArrayData*&	pArrayData_
//		フィールド値の配列(事実上の返り値)
//
//	RETURN
//		形式的にはなし
//
//	EXCEPTIONS
//	BadArgument
//
void
File::checkInsertArgument(
	Common::DataArrayData*	pObject_,
	ModUInt32&				ulVectorKey_,
	Common::DataArrayData*&	pArrayData_)
{
	_SYDNEY_ASSERT(pObject_);
	pArrayData_ = pObject_;

	const ModInt32 lFields = m_cFileParameter.getOuterFieldNumber();
	// 引数に渡したインサート用データの数が
	// ファイルオプションに指定したフィールド数より少ないと例外
	// (多い分は無視する)
	if (pArrayData_->getCount() < lFields) {
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}
	
	// 引数のデータタイプを検査
	for (int i=0; i<lFields; ++i) {
		const Common::Data::Pointer& pElement = pArrayData_->getElement(i);

		// デバッグしやすいようにデータ種を変数に格納
		Common::DataType::Type eTypeInFileParameter = 
			m_cFileParameter.getDataTypeForOuterFieldID(i);
		Common::DataType::Type eTypeInArgument = pElement->getType();
		if (eTypeInArgument != eTypeInFileParameter) {
		//- && !pElement->isNull()

			// 更新時に上書きするデータの型がファイルパラメータと異なる
			SydDebugMessage << "Field[" << i 
			  << "]'s data-type in the argument (" << eTypeInArgument 
				<< ") is not equal to that in the "<< "file parameter ("
				  << eTypeInFileParameter << ")."<< ModEndl;
			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}
	}

	// VectorKeyをpArrayDataから得る
	ulVectorKey_ = getVectorKeyFromObject(pArrayData_);
}

//
//	FUNCTION private
//	Vector::File::checkUpdateArgument -- update()の引数検査
//
//	NOTES
//	更新のための引数に異常がみつかれば例外 BadArgument を送出する。
//	検査に合格すると pObject をキャストした値を返す。
//
//	フィールドの選択方法が適切か判断するために m_pTargetFields も調べている。
//
//	ARGUMENTS
//	Common::Data*					pKey_
//		ベクタキーを唯一の要素として含むオブジェクト
//	Common::Data*					pObject_
//		更新に必要な情報を格納したオブジェクトへのポインタ
//		(常にベクタキーを含まない)
//	ModUInt32	ulVectorKey_
//		更新するオブジェクトのベクタキー(返り値)
//	Common::DataArrayData*&			pArrayData_
//		フィールド値の配列(返り値)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	BadArgument
//
void
File::checkUpdateArgument(
    const Common::DataArrayData*	pKey_,
	Common::DataArrayData*			pObject_,
	ModUInt32&				ulVectorKey_,
	Common::DataArrayData*&	pArrayData_)
{
	// pKey_を検査してulVectorKey_をセットする
	_SYDNEY_ASSERT(pKey_);

	// pKey_の型がDataArrayData*かどうかを調べる
	const Common::Data* pKey = pKey_->getElement(0).get();
	_SYDNEY_ASSERT(pKey);

	// pKey_の第一要素の型がVectorKey*かどうかを調べる
	if (!TypeID::isVectorKey(pKey)) {
		SydErrorMessage << "not vectorkey." << ModEndl;
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}
	const LogicalFile::VectorKey* pVectorKey
		= _SYDNEY_DYNAMIC_CAST(const LogicalFile::VectorKey* ,pKey);
	_SYDNEY_ASSERT(pVectorKey);

	ulVectorKey_ = pVectorKey->getValue();

	// pObject_を検査してpArrayData_をセットする

	// pArrayData_の型を検査
	_SYDNEY_ASSERT(pObject_);
	pArrayData_ = pObject_;

	// pArrayData_のフィールド数を確認
	int iFields = m_pOpenParameter->getSelectedFieldCount();
	if (pArrayData_->getCount() < iFields) {
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	if (iFields == 0)
		return; // もうすることがないので戻る

	// 書き換えるフィールドを示すためにFieldIteratorを用意
	// (第一引数はダミー)
	FieldIterator cFieldIterator(0, 0, m_cFileParameter, *m_pOpenParameter);
	ModUInt32 ulCurrentFieldID;
	// Updateモードにおいては、pArrayDataはベクタキーを含まない
	int iArrayDataIndex = 0; 
	while (cFieldIterator.next())
	{
		ulCurrentFieldID = cFieldIterator.getInnerFieldID();
		Common::DataType::Type eTypeInOpenParameter	= 
			m_cFileParameter.getDataTypeForInnerFieldID(ulCurrentFieldID);
		
		// pDataの要素数は先に調べてある
		const Common::Data* pData = 
			pArrayData_->getElement(iArrayDataIndex).get();
		SydAssert(pData != 0);
		Common::DataType::Type eTypeInArgument = pData->getType();
		// フィールドのタイプを検査
		if (eTypeInOpenParameter != eTypeInArgument) {
			// 更新時に上書きするデータの型がメタデータと異なる
			SydErrorMessage << "Wrong Data Type!" << ModEndl;
			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}
		++iArrayDataIndex;
	}	
	// 更新用データの数を検査
	if (pArrayData_->getCount() != iArrayDataIndex) {
		// 引数に渡された更新用のデータ(第0フィールドを含む)と
		// 選択されているフィールドの数は一致しないとおかしい
		SydErrorMessage << "wrong field numbers." << ModEndl;
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}
}

//
//	FUNCTION private
//	Vector::File::checkDeleteArgument -- expunge()の引数検査
//
//	NOTES
//	削除のための引数に異常がみつかれば例外 BadArgument を送出する。
//
//	ARGUMENTS
//	const Common::DataArrayData*	pObject_
//		削除に必要な情報を格納したオブジェクトへのポインタ
//
//	RETURN
//		ModUInt32 (VectorKey)
//
//	EXCEPTIONS
//	BadArgument
//
ModUInt32
File::checkDeleteArgument(const Common::DataArrayData* pObject_)
{
	_SYDNEY_ASSERT(pObject_);
	// 渡されたデータは配列型
	// 配列の先頭にはベクタキーが格納されているはず
	const Common::Data* pKey = pObject_->getElement(0).get();
	_SYDNEY_ASSERT(pKey);

	if (!TypeID::isVectorKey(pKey)) {
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}
	const LogicalFile::VectorKey* pVectorKey
		= _SYDNEY_DYNAMIC_CAST(const LogicalFile::VectorKey* ,pKey);
	_SYDNEY_ASSERT(pVectorKey);

	return pVectorKey->getValue();
}

//// 整合性検査の下請け関数

// 物理ページごとのオブジェクトカウントの整合性を検査する
// (いまのところ)第二引数はエラー表示にしか使わない
int
File::verifyPageCount(PhysicalFile::Page* pPage_,
				ModUInt32 ulPageID_,
				Admin::Verification::Progress& rProgress_)
{
	ModSize ulBPP = m_cFileParameter.getBlocksPerPage();
	ModSize ulBMOffset = m_cFileParameter.getBitMapAreaOffset();
	ModSize ulObjectCount = 0;
	const char* pPageHead = *pPage_;

	for(ModUInt32 ulBID=0; ulBID < ulBPP; ulBID++)
	{
		const char* pBitMap = pPageHead + ulBMOffset + (ulBID/8);
		unsigned char ucsBitMask = 0x80 >> (ulBID % 8);
		if (*pBitMap & ucsBitMask)
		{
			ulObjectCount++;
		}			
	}

	// ulCountInPageのセット
	Common::UnsignedIntegerData	cCount;
	FileCommon::DataManager::accessToCommonData
		(pPage_, 0, cCount, FileCommon::DataManager::AccessRead);
	ModSize ulCountInPage = cCount.getValue();

	// Vector::File::expunge() -> Vector::PageManager::detachObject() ->
	//   Vector::PageManager::unsetCurrentPage() 内において、
	// PhysicalFile::File::freePage() で FakeError を発生させた場合、
	// ブロック利用ビットマップは Vector::Object::unsetBit() 
	// でフラグは消去されているが、物理ファイルは使用中な為、
	// 0 == ulObjectCount の場合が想定されます。 
	if (ulObjectCount != ulCountInPage)
	{
#ifdef DEBUG
		SydDebugMessage << "Wrong object count in page #"<< ulPageID_<< "." << ModEndl;
#endif
		_SYDNEY_VERIFY_INCONSISTENT(rProgress_,
			_TRMEISTER_U_STRING((m_cFileParameter.getStorageStrategy()).m_VersionFileInfo._path._masterData),
				Message::CorruptPageObjectCount
					(ulPageID_, ulCountInPage, ulObjectCount));
	}

	return ulObjectCount;
}

// 先頭ベクタキーの整合性を検査する
void
File::verifyFirstVectorKey(const Trans::Transaction&	rTransaction_,
						   PhysicalFile::File* pFile_,
						   ModUInt32 ulFirstVectorKey_,
						   Admin::Verification::Progress& rProgress_)
{
	if (!isMounted(rTransaction_) ||
		ulFirstVectorKey_ == FileCommon::VectorKey::Undefined)
		return;

	// 結果
	bool bResult = true;
	ModUInt32 ulBPP = m_cFileParameter.getBlocksPerPage();

	// FirstPage, ということになっているもの
	ModUInt32 ulFirstPageID = 1 + (ulFirstVectorKey_ / ulBPP);

	// ↓0はヘッダページのID
	if (pFile_->getPrevPageID(rTransaction_, ulFirstPageID) != 0)
	{
		 bResult = false;
	}
	else // ページ内の検査
	{
		ModSize ulBMOffset = m_cFileParameter.getBitMapAreaOffset();

		_SYDNEY_FAKE_ERROR("Vector::File::verifyFirstVectorKey",Exception::MemoryExhaust(moduleName, srcFile, __LINE__));
		FileCommon::AutoPhysicalPageVerify pDataPage( pFile_
		                                             ,rTransaction_
		                                             ,ulFirstPageID
		                                             ,Buffer::Page::FixMode::ReadOnly
		                                             ,rProgress_
		                                            );
		const char* pPageHead = *pDataPage;//PhysicalFile::Page::operator char*()

		for(ModOffset ulBID = (ulFirstVectorKey_ % ulBPP)-1; ulBID >= 0; ulBID--)
		{
			const char* pBitMap = pPageHead + ulBMOffset + (ulBID/8);
			unsigned char ucsBitMask = 0x80 >> (ulBID % 8);
			if (*pBitMap & ucsBitMask)
			{
				bResult = false;
				break;
			}
		}
	}

	if (!bResult)
	{
#ifdef DEBUG
		SydDebugMessage << "INCONSISTENT!" << ModEndl;
#endif

		_SYDNEY_VERIFY_INCONSISTENT(rProgress_,
			_TRMEISTER_U_STRING((m_cFileParameter.getStorageStrategy()).m_VersionFileInfo._path._masterData),
				Message::CorruptFirstVectorKey());
	}
}


// 最終ベクタキーの整合性を検査する
void
File::verifyLastVectorKey(const Trans::Transaction&	rTransaction_,
						  PhysicalFile::File* pFile_,
						  ModUInt32 ulLastVectorKey_,
						  Admin::Verification::Progress& rProgress_)
{
	if (ulLastVectorKey_ == FileCommon::VectorKey::Undefined)
	{
		return;
	}

	// 結果
	bool bResult = true;
	ModUInt32 ulBPP = m_cFileParameter.getBlocksPerPage();

	// LastPage, ということになっているもの
	ModUInt32 ulLastPageID = 1 + (ulLastVectorKey_ / ulBPP);

	if (pFile_->getNextPageID(rTransaction_, ulLastPageID) != 
		PhysicalFile::ConstValue::UndefinedPageID)
	{
		bResult = false;
	}
	else // ページ内の検査
	{
		ModSize ulBMOffset = m_cFileParameter.getBitMapAreaOffset();

		_SYDNEY_FAKE_ERROR("Vector::File::verifyLastVectorKey",Exception::MemoryExhaust(moduleName, srcFile, __LINE__));
		FileCommon::AutoPhysicalPageVerify pDataPage( pFile_
		                                             ,rTransaction_
		                                             ,ulLastPageID
		                                             ,Buffer::Page::FixMode::ReadOnly
		                                             ,rProgress_
		                                            );
		const char* pPageHead = *pDataPage;//PhysicalFile::Page::operator char*()

		for(ModUInt32 ulBID = (ulLastVectorKey_ % ulBPP)+1; ulBID < ulBPP; ulBID++)
		{
			const char* pBitMap = pPageHead + ulBMOffset + (ulBID/8);
			unsigned char ucsBitMask = 0x80 >> (ulBID % 8);
			if (*pBitMap & ucsBitMask)
			{
				bResult = false;
				break;
			}
		}
	}

	if (!bResult)
	{
#ifdef DEBUG
		SydDebugMessage << "INCONSISTENT!" << ModEndl;
#endif

		_SYDNEY_VERIFY_INCONSISTENT(rProgress_,
			_TRMEISTER_U_STRING((m_cFileParameter.getStorageStrategy()).m_VersionFileInfo._path._masterData),
			Message::CorruptLastVectorKey());
	}
}

// 実体を作る
void
File::substantiate()
{
	; _SYDNEY_ASSERT(m_pTransaction);
	if (isMounted(*m_pTransaction))
		return;

	{// 物理ファイル格納戦略、バッファリング戦略を得て物理ファイルをアタッチする
	_SYDNEY_FAKE_ERROR("Vector::File::create",Exception::BadArgument(moduleName, srcFile, __LINE__));
	FileCommon::AutoPhysicalFile file(m_cFileParameter.getStorageStrategy() ,m_cFileParameter.getBufferingStrategy() ,m_cFileParameter.getLockName());

	// 物理ファイルを作成する
	file->create(*m_pTransaction);

	try {
		// ブロックに関する情報をFileParameterにセット
		m_cFileParameter.initializeBlockParameters(file->getPageDataSize());

		ModAutoPointer<PageManager> manager( new PageManager(*m_pTransaction, m_cFileParameter, 0, file.get(), true) );

		// 管理情報の初期化(ヘッダページの初期化)
		manager->createFileInformation();
	} catch(...) {
		// 物理ファイルを作成を元に戻す
		try {
			file->destroy(*m_pTransaction);
		} catch(...) {

			// 元に戻せなかったので、利用不可にする

			Checkpoint::Database::setAvailability(
				m_cFileParameter.getLockName(), false);
		}
		_SYDNEY_RETHROW;
	}
	}
}

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
