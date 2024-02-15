// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VariableFile.cpp -- 可変長フィールドを格納するファイル
// 
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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
const char srcFile[] = __FILE__;
const char moduleName[] = "Record";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyInclude.h"
#include "Record/VariableFile.h"
#include "Record/DirectFile.h"
#include "Record/FileInformation.h"
#include "Record/FreeAreaManager.h"
#include "Record/LinkedObject.h"
#include "Record/MetaData.h"
#include "Record/OpenParameter.h"
#include "Record/TargetFields.h"
#include "Record/UseInfo.h"
#include "Record/VariableIterator.h"
#include "Record/Debug.h"

#include "Checkpoint/Database.h"

#include "Common/Assert.h"
#include "Common/Message.h"
#include "Common/SystemParameter.h"

#include "FileCommon/AutoAttach.h"
#include "FileCommon/FileOption.h"
#include "FileCommon/OpenOption.h"


#include "Os/Limits.h"

#include "Exception/FakeError.h"
#include "Exception/IllegalFileAccess.h"
#include "Exception/BadArgument.h"

#include "LogicalFile/Estimate.h"

#include "PhysicalFile/Manager.h"

_SYDNEY_USING
_SYDNEY_RECORD_USING

namespace
{
	// VariableFileの種別
	const PhysicalFile::Type _ePhysicalFileType = PhysicalFile::AreaManageType;

	// VariableFileのファイル名
	const ModUnicodeString _cstrFileName(_TRMEISTER_U_STRING("Variable"));

	// データページの最小ID
	PhysicalFile::PageID _iDataPageID = 0;

#ifdef DEBUG
	int _iOutputUseRate = -1;

	bool _OutputUseRate()
	{
		if (_iOutputUseRate < 0) {
			ModUnicodeString v;
			if (Common::SystemParameter::getValue(ModCharString("Record_MessageOutputSize"), v)) {
				_iOutputUseRate = (v == _TRMEISTER_U_STRING("0")) ? 0 : 1;

			} else {
				_iOutputUseRate = 0;
			}
		}
		return (_iOutputUseRate == 1);
	}
#endif

	//
	class AutoMemory {
	public:
		AutoMemory(ModSize areaIDArraySize_)
			: m_areaIDArraySize(areaIDArraySize_)
			, m_areaIDArray(ModDefaultManager::allocate(areaIDArraySize_))
		{
		}

		~AutoMemory() throw()
		{
			free();
		}

		void* get() const
		{
			return m_areaIDArray;
		}

		void free()
		{
			if (m_areaIDArray) {
				ModDefaultManager::free(m_areaIDArray, m_areaIDArraySize);
				m_areaIDArray = 0;
			}
		}

	private:
		const ModSize m_areaIDArraySize;
		void* m_areaIDArray;

#ifndef SYD_C_ICC
	private://auto のみで作成させる為の処置
		static void* operator new(size_t);
		static void operator delete(void*);
		static void* operator new[](size_t);
		static void operator delete[](void*);
#endif
	};

} // namespace

//	FUNCTION public
//	Record::VariableFile::VariableFile --
//
//	NOTES
//
//	ARGUMENTS
//		const Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const MetaData& cMetaData_
//			作成するファイルの情報を表すメタデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

VariableFile::
VariableFile(const Trans::Transaction& cTrans_,
			 const MetaData& cMetaData_)
	: FileBase(cTrans_, cMetaData_),
	  m_pReadIterator(0), m_pWriteIterator(0),
	  m_pFreeAreaManager(0)
{
	setStorategy(cMetaData_);
}

//	FUNCTION public
//	Record::VariableFile::~VariableFile --
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

VariableFile::
~VariableFile()
{
	if (m_pReadIterator) delete m_pReadIterator, m_pReadIterator = 0;
	if (m_pWriteIterator) delete m_pWriteIterator, m_pWriteIterator = 0;
	if (m_pFreeAreaManager) delete m_pFreeAreaManager, m_pFreeAreaManager = 0;
}

#ifdef OBSOLETE
//	FUNCTION public
//	Record::VariableFile::getCount --
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		オブジェクト数
//
//	EXCEPTIONS

ModInt64
VariableFile::
getCount() const
{
	if (!isMounted(m_cTrans))
		return 0;

	// 可変長ファイルからはオブジェクト総数は得られない
	return 0;
}
#endif //OBSOLETE

//	FUNCTION public
//	Record::VariableFile::getOverhead --
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		オーバーヘッド
//
//	EXCEPTIONS

double
VariableFile::
getOverhead() const
{
	// オーバーヘッドはない
	return 0.0;
}

//	FUNCTION public
//	Record::VariableFile::getProcessCost --
//
//	NOTES
//
//	ARGUMENTS
//		ModInt64 iCount_
//			オブジェクト総数
//
//	RETURN
//		1件あたりの処理にかかる時間
//
//	EXCEPTIONS

double
VariableFile::
getProcessCost(ModInt64 iCount_) const
{
	if (!isMounted(m_cTrans))
		return 0.0;

	// オブジェクトの平均サイズを得る
	ModInt64 iSize = getSize();
	double dAverageSize = static_cast<double>(iSize) / (iCount_ ? iCount_ : 1);
	const double dCostFileToMemory =
		static_cast<const double>(
			LogicalFile::Estimate::getTransferSpeed(
				LogicalFile::Estimate::File));

	return dAverageSize / dCostFileToMemory;
}

//	FUNCTION public
//	Record::VariableFile::setUseInfo --
//		使用中の物理ページIDとエリアIDを集める
//
//	NOTES
//
//	ARGUMENTS
//		Record::Tools::ObjectID iObjectID_
//			対象のオブジェクトID
//		Record::UseInfo& cUseInfo_
//			登録情報
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VariableFile::
setUseInfo(Tools::ObjectID iObjectID_,
		   UseInfo& cUseInfo_,
		   Admin::Verification::Progress& cProgress_)
{
	if (!isMounted(m_cTrans) || !cProgress_.isGood())
		return;

	; _SYDNEY_ASSERT(isAttached());
	; _SYDNEY_ASSERT(m_pFreeAreaManager);

	LinkedObject cLinkedObject(LinkedObject::Operation::Read, *m_pFreeAreaManager);

	// リンクオブジェクトのIDをたどって登録していく
	// ★注意★
	// 分割されていなくてもLinkedObjectでアクセスできる
	cLinkedObject.setUseInfo(iObjectID_, cUseInfo_ ,cProgress_);
}

//	FUNCTION public
//	Record::VariableFile::setFreeAreaUseInfo --
//		使用中の物理ページIDとエリアIDを集める（FreeArea 用）
//
//	NOTES
//
//	ARGUMENTS
//		Record::Tools::ObjectID iObjectID_
//			対象のオブジェクトID
//		Record::UseInfo& cUseInfo_
//			登録情報
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VariableFile::
setFreeAreaUseInfo(Tools::ObjectID iObjectID_,
		   UseInfo& cUseInfo_,
		   Admin::Verification::Progress& cProgress_)
{
	if (!isMounted(m_cTrans) || !cProgress_.isGood())
		return;

	; _SYDNEY_ASSERT(isAttached());
	; _SYDNEY_ASSERT(m_pFreeAreaManager);

	m_pFreeAreaManager->setUseInfo(iObjectID_, cUseInfo_ ,cProgress_);
}

//	FUNCTION public
//	Record::VariableFile::startVerification -- 物理ファイルの整合性検査を開始する
//
//	NOTES
//
//	ARGUMENTS
//	AAdmin::Verification::Treatment::Value iTreatment_
//		整合性検査の検査方法
//	Admin::Verification::Progress&	cProgress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VariableFile::
startVerification(const Trans::Transaction&		Transaction_,
						const unsigned int				iTreatment_,
						Admin::Verification::Progress&	cProgress_)
{
	if (isMounted(m_cTrans))
		m_pFile->startVerification(m_cTrans, iTreatment_, cProgress_);
}

//	FUNCTION public
//	Record::VariableFile::endVerification -- 物理ファイルの整合性検査を終了する
//
//	NOTES
//
//	ARGUMENTS
//	Admin::Verification::Progress&	cProgress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VariableFile::
endVerification(const Trans::Transaction&		Transaction_,
						 Admin::Verification::Progress&	cProgress_)
{
	if (isMounted(m_cTrans))
		// cProgress_ の中身に拠らず endVerification() は実行。
		m_pFile->endVerification(m_cTrans, cProgress_);
}

//	FUNCTION public
//	Record::VariableFile::verifyPhysicalFile --
//		物理ファイルの整合性検査をする
//
//	NOTES
//
//	ARGUMENTS
//		Record::UseInfo& cUseInfo_
//			登録情報
//		Admin::Verification::Treatment::Value iTreatment_
//			整合性検査の検査方法
//		Admin::Verification::Progress& cProgress_
//			途中経過を記録するための変数
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VariableFile::
verifyPhysicalFile(UseInfo& cUseInfo_,
				   Admin::Verification::Treatment::Value iTreatment_,
				   Admin::Verification::Progress& cProgress_)
{
	if (!isMounted(m_cTrans))
		return;

	try {
		notifyUsePage(cUseInfo_, iTreatment_, cProgress_);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		this->detachPageAll(true);
		_SYDNEY_RETHROW;
	}
	this->detachPageAll(true);
}

//	FUNCTION public
//	Record::VariableFile::notifyUsePage --
//		使用中の物理ページIDとエリアIDを物理ファイルに知らせる
//
//	NOTES
//
//	ARGUMENTS
//		Record::UseInfo& cUseInfo_
//			登録情報
//		Admin::Verification::Treatment::Value iTreatment_
//			整合性検査の検査方法
//		Admin::Verification::Progress& cProgress_
//			途中経過を記録するための変数
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VariableFile::
notifyUsePage(UseInfo& cUseInfo_,
			  Admin::Verification::Treatment::Value iTreatment_,
			  Admin::Verification::Progress& cProgress_)
{
	if (!isMounted(m_cTrans))
		return;

	ModSize	areaIDArraySize =
		sizeof(PhysicalFile::AreaID) * cUseInfo_.m_AreaIDsMaxCount;

	AutoMemory mem(areaIDArraySize);//自動的にmemoryをalloc/freeする。
	PhysicalFile::AreaID* const areaIDArray =
		static_cast<PhysicalFile::AreaID*>(mem.get());
#ifdef DEBUG
	SydRecordSizeMessage
		<< "Check use rate of file: " << getPath()
		<< ModEndl;
#endif

	for (PhysicalFile::PageID pageID = _iDataPageID;
		 pageID <= cUseInfo_.m_LastPageID;
		 pageID++)
	{
#ifdef DEBUG
		Os::Memory::Size iSize = 0; // ページ中の使用バイト数
#endif
		const UseInfo::AreaIDs&	areaIDs = cUseInfo_.getAreaIDs(pageID);

		if (areaIDs.isEmpty() == ModFalse)
		{
			// 使用中の物理ページ…

#ifdef DEBUG
			PhysicalFile::Page* pUseRatePage = 0;
			if (_OutputUseRate()) {
				pUseRatePage = m_pFile->verifyPage(m_cTrans, pageID, Buffer::Page::FixMode::ReadOnly, cProgress_);
			}
#endif

			UseInfo::AreaIDs::ConstIterator	areaID = areaIDs.begin();

			PhysicalFile::AreaNum	areaNum = 0;
			PhysicalFile::AreaID*	areaIDArrayPointer = 0;

			//
			// 使用中の物理ページでも、その中に使用中の物理エリアが
			// 存在しないこともある。
			// そのような場合、登録情報には、物理エリア識別子として
			// Undefinedを登録してある。
			//

			if (*areaID != PhysicalFile::ConstValue::UndefinedAreaID)
			{
				UseInfo::AreaIDs::ConstIterator	areaIDsEnd = areaIDs.end();

				int	arrayIndex = 0;

				while (areaID != areaIDsEnd)
				{
					*(areaIDArray + arrayIndex) = *areaID;
#ifdef DEBUG
					if (_OutputUseRate()) {
						iSize += pUseRatePage->getAreaSize(*areaID);
					}
#endif
					++areaID;

					++arrayIndex;
				}

				areaIDArrayPointer = areaIDArray;

				areaNum = arrayIndex;
			}

#ifdef DEBUG
			if (_OutputUseRate()) {
				SydRecordSizeMessage
					<< "Usage % of page #" << pageID << " : " << iSize * 100 / m_pFile->getPageDataSize()
					<< ModEndl;
				m_pFile->detachPage(pUseRatePage, PhysicalFile::Page::UnfixMode::NotDirty);
			}
#endif

			m_pFile->notifyUsePage(m_cTrans,
								   cProgress_,
								   pageID,
								   areaNum,
								   areaIDArrayPointer);

			if (cProgress_.isGood() == false)
			{
				break;
			}
		}
	}
}

//	FUNCTION public
//	Record::VariableFile::verifyContents --
//		内容の整合性検査を行う
//
//	NOTES
//
//	ARGUMENTS
//		DirectFile::DataPackage& cData_
//			対象のオブジェクトIDとNull Bitmap
//		Admin::Verification::Treatment::Value iTreatment_
//			整合性検査の検査方法
//		Admin::Verification::Progress& cProgress_
//			途中経過を記録するための変数
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VariableFile::
verifyContents(DirectFile::DataPackage& cData_,
			   Admin::Verification::Treatment::Value iTreatment_,
			   Admin::Verification::Progress&		cProgress_)
{
	if (!isMounted(m_cTrans) || !cProgress_.isGood())
		return;

	; _SYDNEY_ASSERT(isAttached());

	// ここでは以下の検査を行う
	// 7．可変長オブジェクトサイズの確認

	if (!m_pReadIterator) {
		m_pReadIterator =
			new VariableIterator(*this,
								 VariableIterator::Operation::Read);
	}
	m_pReadIterator->verifyData(cData_, iTreatment_, cProgress_);
}

//	FUNCTION public
//	Record::VariableFile::read --
//
//	NOTES
//
//	ARGUMENTS
//		Record::VariableFile::DirectFile::DataPackage& cData_
//			固定長ファイルに書かれた情報を格納する変数
//		const TargetFields* pTarget_
//			取得するフィールドを表すクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VariableFile::
read(DirectFile::DataPackage& cData_,
	 const TargetFields* pTarget_)
{
	if (!isMounted(m_cTrans))
		return;

	; _SYDNEY_ASSERT(m_pFile);
	; _SYDNEY_ASSERT(cData_.get());
	; _SYDNEY_ASSERT(pTarget_);

	if (!m_pReadIterator) {
		m_pReadIterator =
			new VariableIterator(*this,
								 VariableIterator::Operation::Read);
	}
	if (!m_pReadIterator->seek(cData_.getVariableID())) {
		// オブジェクトIDが正しくない
#ifdef RECORD_CHECK_NULL
		// 可変長はすべてNULLであるはずなのでチェックする
		m_pReadIterator->assureNull(cData_, pTarget_);
#endif
		// このまま返る
		return;
	}
	try {
		m_pReadIterator->read(cData_, pTarget_);

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		m_pReadIterator->detachObject();
		_SYDNEY_RETHROW;
	}
	if (m_cTrans.isNoVersion())
		m_pReadIterator->detachObject();
}

//	FUNCTION public
//	Record::VariableFile::insert --
//
//	NOTES
//
//	ARGUMENTS
//		const Common::DataArrayData& cObject_
//			挿入するデータ
//		Tools::ObjectID& iFreeID_
//			[IN/OUT]空きエリアリストの先頭として管理情報に記録されているオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Tools::ObjectID
VariableFile::
insert(const DirectFile::DataPackage& cData_,
	   Tools::ObjectID& iFreeID_)
{
	substantiate();

	; _SYDNEY_ASSERT(isMounted(m_cTrans));
	; _SYDNEY_ASSERT(cData_.get());

	prepareFreeAreaManager(true /* do cache page */);

	if (!m_pWriteIterator) {
		m_pWriteIterator =
			new VariableIterator(*this,
								 isBatch()
								 ? VariableIterator::Operation::Batch
								 : VariableIterator::Operation::Write);
	}
	m_pWriteIterator->insert(cData_, iFreeID_);

	discardFreeAreaManager();

	// 書き込みが成功するとイテレーターはそこを指す状態になる
	return m_pWriteIterator->getObjectID();
}

//	FUNCTION public
//	Record::VariableFile::update --
//
//	NOTES
//
//	ARGUMENTS
//		Record::VariableFile::DirectFile::DataPackage& cOldObjectHeader_
//			更新前のnull bitmapと可変長オブジェクトID
//		Record::VariableFile::DirectFile::DataPackage& cNewData_
//			更新後のデータ
//		const TargetFields* pTarget_
//			更新するフィールドを表すクラス
//		Tools::ObjectID& iFreeID_
//			[IN/OUT]空きエリアリストの先頭として管理情報に記録されているオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Tools::ObjectID
VariableFile::
update(const DirectFile::DataPackage& cOldObjectHeader_,
	   const DirectFile::DataPackage& cNewData_,
	   const TargetFields* pTarget_,
	   Tools::ObjectID& iFreeID_)
{
	; _SYDNEY_ASSERT(isMounted(m_cTrans));
	; _SYDNEY_ASSERT(cNewData_.get());

	prepareFreeAreaManager(true /* do cache page */);

	if (!m_pWriteIterator) {
		m_pWriteIterator =
			new VariableIterator(*this,
								 VariableIterator::Operation::Write);
	}
	m_pWriteIterator->update(cOldObjectHeader_, cNewData_, pTarget_, iFreeID_);

	Tools::ObjectID iResult = m_pWriteIterator->getObjectID();
	if (cOldObjectHeader_.getVariableID() != Tools::m_UndefinedObjectID
		&& cOldObjectHeader_.getVariableID() != iResult) {
		// 可変長オブジェクトのIDが変化していたら
		// 以前のものを破棄する
		m_pWriteIterator->expunge(cOldObjectHeader_.getVariableID(), iFreeID_);
	}

	discardFreeAreaManager();

	// 新しいデータの可変長オブジェクトIDを返す
	return iResult;
}

//	FUNCTION public
//	Record::VariableFile::expunge --
//
//	NOTES
//
//	ARGUMENTS
//		Tools::ObjectID iObjectID_
//			削除するオブジェクトのオブジェクトID
//		Tools::ObjectID& iFreeID_
//			[IN/OUT]空きエリアリストの先頭として管理情報に記録されているオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VariableFile::
expunge(Tools::ObjectID iObjectID_,
		Tools::ObjectID& iFreeID_)
{
	; _SYDNEY_ASSERT(isMounted(m_cTrans));

	prepareFreeAreaManager();

	if (!m_pWriteIterator) {
		m_pWriteIterator =
			new VariableIterator(*this,
								 VariableIterator::Operation::Write);
	}
	m_pWriteIterator->expunge(iObjectID_, iFreeID_);

	discardFreeAreaManager();
}

//
//	FUNCTION public
//	Record::VariableFile::detachFile -- 物理ファイルをデタッチする
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
VariableFile::detachFile()
{
	if (m_pReadIterator) delete m_pReadIterator, m_pReadIterator = 0;
	if (m_pWriteIterator) delete m_pWriteIterator, m_pWriteIterator = 0;
	if (m_pFreeAreaManager) delete m_pFreeAreaManager, m_pFreeAreaManager = 0;

	FileBase::detachFile();
}

//	FUNCTION public
//	Record::VariableFile::detachPageAll --
//		アタッチしたすべての物理ページ記述子をデタッチする
//
//	NOTES
//	アタッチしたすべての物理ページ記述子をデタッチする。
//
//	ARGUMENTS
//		bool bSucceeded_
//			trueの場合正常終了時の処理を行う
//			falseの場合attachPageしているページへの変更を捨てる
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VariableFile::
detachPageAll(bool bSucceeded_)
{
	try
	{
		if (bSucceeded_
			|| m_cTrans.getCategory() == Trans::Transaction::Category::ReadOnly
			|| isBatch())
		{
			// 正常終了…

			// バッチモードのときは常に成功時と同じ処理にする
			// 異常時には上位層でrecoverによりトランザクション開始時の版に戻される

			// アタッチしたすべての物理ページ記述子をデタッチする。
			this->m_pFile->detachPageAll();

			if (this->m_pFreeAreaManager)
				// freePageすべきページをすべて解放する
				this->m_pFreeAreaManager->freePageAll();

		}
		else
		{
			// 異常終了…

			// アタッチしたすべての物理ページ記述子をデタッチし、
			// ページの内容をアタッチ前の状態に戻す。

			//
			// 可変長フィールドを格納するファイルが使う
			// 物理ファイルは“空き領域管理機能付き物理ファイル”。
			// 空き領域管理機能付き物理ファイルの
			// PhysicalFile::File::recoverPageAll()内では、必要に応じて
			// “空き領域管理表”の修復も行う。
			// そのとき、空き領域管理表をフィックスするために
			// トランザクション記述子が必要となる。
			// なので、トランザクション記述子を引数として受け取る方の
			// PhysicalFile::File::recoverPageAll()を呼び出す。
			//

			this->m_pFile->recoverPageAll(this->m_cTrans);

			if (this->m_pFreeAreaManager)
				// discard all the reservation for freePage
				m_pFreeAreaManager->discardFreePage();
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// 失敗したので、利用可能性をOFFにする

		SydErrorMessage << "Fatal Error. detach page failed." << ModEndl;

		Checkpoint::Database::setAvailability(
			m_cMetaData.getLockName(), false);

		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Record::VariableFile::prepareFreeAreaManager --
//
//	NOTES
//
//	ARGUMENTS
//		bool bDoCache_ = false
//			ページをキャッシュするか
//				エラー処理のため、この引数に関係なくキャッシュする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VariableFile::
prepareFreeAreaManager(bool bDoCache_)
{
	; _SYDNEY_ASSERT(isMounted(m_cTrans));

	if (!m_pFreeAreaManager) {
		m_pFreeAreaManager = new FreeAreaManager(m_cTrans,
												 *m_pFile,
												 _iDataPageID,
												 bDoCache_);
	} else {
//		m_pFreeAreaManager->setDoCache(bDoCache_);
	}
}

//	FUNCTION public
//	Record::VariableFile::discardFreeAreaManager --
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VariableFile::
discardFreeAreaManager()
{
	; _SYDNEY_ASSERT(m_pFreeAreaManager);
	m_pFreeAreaManager->clearCachePage();
}

//////////////////
// 内部メソッド //
//////////////////

//	FUNCTION protected
//	Record::VariableFile::setStorategy --
//
//	NOTES
//
//	ARGUMENTS
//		const Record::MetaData& cMetaData_
//			メタデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VariableFile::
setStorategy(const MetaData& cMetaData_)
{
	// 格納戦略を設定する
	m_cStorageStrategy.m_PhysicalFileType = _ePhysicalFileType;
	m_cStorageStrategy.m_PageUseRate = PhysicalFile::ConstValue::DefaultPageUseRate;
	m_cStorageStrategy.m_VersionFileInfo._pageSize = cMetaData_.getVariablePageSize();

	Os::Path	cstrPath(cMetaData_.getDirectoryPath());
	cstrPath.addPart(getPathPart());

	m_cStorageStrategy.m_VersionFileInfo._path._masterData = cstrPath;

	if (!cMetaData_.isTemporary())
	{
		m_cStorageStrategy.m_VersionFileInfo._path._versionLog = cstrPath;
		m_cStorageStrategy.m_VersionFileInfo._path._syncLog = cstrPath;
	}

	m_cStorageStrategy.m_VersionFileInfo._sizeMax._masterData =
		PhysicalFile::ConstValue::DefaultFileMaxSize;
	m_cStorageStrategy.m_VersionFileInfo._sizeMax._versionLog =
		PhysicalFile::ConstValue::DefaultFileMaxSize;
	m_cStorageStrategy.m_VersionFileInfo._sizeMax._syncLog =
		PhysicalFile::ConstValue::DefaultFileMaxSize;

	m_cStorageStrategy.m_VersionFileInfo._extensionSize._masterData =
		PhysicalFile::ConstValue::DefaultFileExtensionSize;
	m_cStorageStrategy.m_VersionFileInfo._extensionSize._versionLog =
		PhysicalFile::ConstValue::DefaultFileExtensionSize;
	m_cStorageStrategy.m_VersionFileInfo._extensionSize._syncLog =
		PhysicalFile::ConstValue::DefaultFileExtensionSize;

	bool bMounted;
	if ( cMetaData_.getFileID().getBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Mounted::Key) ,bMounted ) ) {
		m_cStorageStrategy.m_VersionFileInfo._mounted = bMounted;
	} else {
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// バッファリング戦略を設定する
	m_cBufferingStrategy.m_VersionFileInfo._category =
		cMetaData_.isTemporary() ?
		Buffer::Pool::Category::Temporary
		: (cMetaData_.isReadOnly() ?
		   Buffer::Pool::Category::ReadOnly
		   : Buffer::Pool::Category::Normal);
}

//	FUNCTION private
//	Record::VariableFile::getPathPart --
//		サブディレクトリーのパス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//	パス名
//
//	EXCEPTIONS

//virtual
const ModUnicodeString&
VariableFile::
getPathPart() const
{
	return _cstrFileName;
}

//	FUNCTION private
//	Record::VariableFile::substantiate -- 実体を作成する
//
//	NOTES
//
//	ARGUMENTS
// 		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VariableFile::
substantiate()
{
	if (isMounted(m_cTrans))
		return;

	const bool bIsAttached = isAttached();
	if (!bIsAttached) {
		attachFile();
		; _SYDNEY_ASSERT(m_pFile);
	}

	try {
		// 実体である物理ファイルを生成する

		m_pFile->create(m_cTrans);

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		try {
			// サブディレクトリーを破棄する
			//
			//【注意】	サブディレクトリは
			//			実体である物理ファイルの生成時に
			//			必要に応じて生成されるが、
			//			エラー時には削除されないので、
			//			この関数で削除する必要がある

			ModOsDriver::File::rmAll(getPath(), ModTrue);

		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{

			// 元に戻せなかったので、利用不可にする

			Checkpoint::Database::setAvailability(
				m_cMetaData.getLockName(), false);
		}

		if (!bIsAttached)
			detachFile();
		_SYDNEY_RETHROW;
	}

	if (!bIsAttached)
		detachFile();
}

//
//	Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
