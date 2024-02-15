// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DirectFile.cpp -- 代表オブジェクトを格納するファイル
// 
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2010, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"
#include "Record/DirectFile.h"
#include "Record/DirectIterator.h"
#include "Record/FileInformation.h"
#include "Record/MetaData.h"
#include "Record/PhysicalPosition.h"
#include "Record/TargetFields.h"
#include "Record/UseInfo.h"
#include "Record/VariableFile.h"
#include "Record/Message_DiscordObjectNum.h"
#include "Record/Message_ExistLastObject.h"
#include "Record/Message_ExistTopObject.h"
#include "Record/Message_InconsistentHeader.h"
#include "Record/Message_ObjectNotFound.h"
#include "Record/Message_VerifyFailed.h"
#include "Record/Message_VerifyOnGoing.h"

#include "Checkpoint/Database.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "FileCommon/FileOption.h"
#include "FileCommon/AutoAttach.h"

#include "Exception/FakeError.h"
#include "Exception/IllegalFileAccess.h"
#include "Exception/BadArgument.h"
#include "Exception/Cancel.h"

#include "LogicalFile/Estimate.h"

#include "PhysicalFile/Manager.h"

_SYDNEY_USING
_SYDNEY_RECORD_USING

namespace
{
	// DirectFileの種別
	const PhysicalFile::Type _ePhysicalFileType = PhysicalFile::PageManageType;

	// DirectFileのファイル名
	const ModUnicodeString _cstrFileName(_TRMEISTER_U_STRING("Fixed"));

	// ヘッダーのページID
	PhysicalFile::PageID _iInformationPageID = 0;

} // namespace

///////////////////////////////////
// DirectFile::DataPackageの定義 //
///////////////////////////////////

//	FUNCTION public
//	Record::DirectFile::DataPackage::DataPackage --
//
//	NOTES
//
//	ARGUMENTS
//		const MetaData& cMetaData_
//			作成するファイルの情報を表すメタデータ
//		const Record::TargetFields* pTargets_ = 0
//			更新時に対象となるフィールド
//		const Common::DataArrayData* pData_ = 0
//			挿入/更新時に渡されるデータ
//			これが0なら読み込みである
//		bool bIsUpdate_ = false
//			trueなら更新用
//
//	RETURN
//		なし
//
//	EXCEPTIONS

DirectFile::DataPackage::
DataPackage(const MetaData& cMetaData_,
			const TargetFields* pTargets_,
			const Common::DataArrayData* pData_)
	: m_pConstData(0), m_cNull(),
	  m_iVariableID(Tools::m_UndefinedObjectID),
	  m_bAllocated(false),
	  m_cMetaData(cMetaData_), m_pTarget(pTargets_)
{
	// 引数にデータがあれば設定する
	if (pData_) {
		setData(pData_);
	}
}

//	FUNCTION public
//	Record::DirectFile::DataPackage::~DataPackage --
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

DirectFile::DataPackage::~DataPackage()
{
	free();
}

//	FUNCTION public
//	Record::DirectFile::DataPackage::allocate --
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
DirectFile::DataPackage::allocate(Common::DataArrayData* pData_)
{
	if (m_pData == 0) {
		if (pData_ == 0) {
			pData_ = new Common::DataArrayData;
			m_bAllocated = true;
		}
		m_pData = pData_;
	}
}

// FUNCTION public
//	Record::DirectFile::DataPackage::reallocate -- replace existing data by allocated data
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
DirectFile::DataPackage::
reallocate()
{
	if (!m_bAllocated) {
		m_pData = 0;
	}
	allocate();
}

//	FUNCTION public
//	Record::DirectFile::DataPackage::free --
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
DirectFile::DataPackage::free()
{
	if (m_pData) {
		if (m_bAllocated) {
			delete m_pData;
			m_bAllocated = false;
		}
		m_pData = 0;
	}
}

//	FUNCTION public
//	Record::DirectFile::DataPackage::get --
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		そのまま返す
//
//	EXCEPTIONS

Common::DataArrayData*
DirectFile::DataPackage::
get()
{
	return m_pData;
}

const Common::DataArrayData*
DirectFile::DataPackage::
get() const
{
	return m_pConstData;
}

//	FUNCTION public
//	Record::DirectFile::DataPackage::release --
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		そのまま返す
//
//	EXCEPTIONS

Common::DataArrayData*
DirectFile::DataPackage::
release()
{
	m_bAllocated = false;
	return m_pData;
}

//	FUNCTION public
//	Record::DirectFile::DataPackage::getVariableID --
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		そのまま返す
//
//	EXCEPTIONS

Tools::ObjectID
DirectFile::DataPackage::
getVariableID() const
{
	return m_iVariableID;
}

//	FUNCTION public
//	Record::DirectFile::DataPackage::setVariableID --
//
//	NOTES
//
//	ARGUMENTS
//		Tools::ObjectID iVariableID_
//			セットするID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
DirectFile::DataPackage::
setVariableID(Tools::ObjectID iVariableID_)
{
	m_iVariableID = iVariableID_;
}

//	FUNCTION public
//	Record::DirectFile::DataPackage::getNullBitMap --
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		そのまま返す
//
//	EXCEPTIONS

Tools::BitMap&
DirectFile::DataPackage::
getNullBitMap()
{
	return m_cNull;
}

const Tools::BitMap&
DirectFile::DataPackage::
getNullBitMap() const
{
	return m_cNull;
}

//	FUNCTION public
//	Record::DirectFile::DataPackage::setNullBitMap --
//
//	NOTES
//
//	ARGUMENTS
//		const Tools::BitMap& cNull_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
DirectFile::DataPackage::
setNullBitMap(const Tools::BitMap& cNull_)
{
	m_cNull = cNull_;
}

//	FUNCTION public
//	Record::DirectFile::DataPackage::isNull --
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		そのまま返す
//
//	EXCEPTIONS

bool
DirectFile::DataPackage::
isNull(Tools::FieldNum iFieldID_) const
{
	return m_cNull.test(iFieldID_);
}

//	FUNCTION public
//	Record::DirectFile::DataPackage::setData --
//
//	NOTES
//
//	ARGUMENTS
//		const Common::DataArrayData* pData_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
DirectFile::DataPackage::setData(const Common::DataArrayData* pData_)
{
	; _SYDNEY_ASSERT(pData_);

	bool copied = false;

	// 処理対象のフィールドごとに調べる
	bool bHasCompress = m_cMetaData.hasCompress();

	TargetIterator ite(m_pTarget, &m_cMetaData);
	while (ite.hasNext()) {
		const Tools::FieldNum fieldID = ite.getNext();
		const Tools::FieldNum i = ite.getIndex();

		const Common::Data::Pointer& p0 = pData_->getElement(i);
		bool bIsNull = p0->isNull();

		if (!bIsNull
			&& bHasCompress
			&& m_cMetaData.getCompress(fieldID) &&
			p0->isApplicable(Common::Data::Function::Compressed)) {

			// 圧縮されるフィールドがあり、
			// 与えられたデータが圧縮可能である

			if (!copied) {

				// これまでに与えられたフィールドデータが
				// コピーされていなければ、まず、コピーする

				reallocate();
				*m_pData = *pData_;
				copied = true;
			}

			// データを圧縮したものに置き換える
			//	ここではDataのクラスが変わるのでインスタンスを変えるしかない
			Common::Data::Pointer pCompressed =
				p0->apply(Common::Data::Function::Compressed);

			m_pData->setElement(i, pCompressed);

			continue;
		}

		const Tools::DataType& type = m_cMetaData.getDataType(fieldID);
		const Tools::DataType& elmType = m_cMetaData.getElementType(fieldID);

		if (type._name == Common::DataType::String) {
			if (!bIsNull) {
				switch (p0->getType()) {
				case Common::DataType::String:
					if (_SYDNEY_DYNAMIC_CAST(
							const Common::StringData&, *p0).
								getEncodingForm() == type._encodingForm)
						break;
					// thru

				default:
					if (!copied) {
						reallocate();
						*m_pData = *pData_;
						copied = true;
					}

					//	ここではDataのクラスが変わるのでインスタンスを変えるしかない

					Common::Data::Pointer tmp =
						p0->cast(Common::StringData(type._encodingForm));
					m_pData->setElement(i, tmp);
				}
			}
		} else if (type._name == Common::DataType::Array &&
				   elmType._name == Common::DataType::String &&
				   p0->getType() == Common::DataType::Array) {

			const Common::DataArrayData& src =
				_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&, *p0);
			const unsigned int n = src.getCount();

			for (unsigned int j = 0; j < n; ++j) {
				const Common::Data::Pointer& p1 = src.getElement(j);
				if (!p1->isNull()) {
					switch (p1->getType()) {
					case Common::DataType::String:
						if (_SYDNEY_DYNAMIC_CAST(
								const Common::StringData&, *p1).
									getEncodingForm() == elmType._encodingForm)
							break;
						// thru

					default:
						if (!copied) {
							reallocate();
							*m_pData = *pData_;
							copied = true;
						}

						//	配列型についてはDataArrayDataのインスタンスが変わらなければよいので
						//  setElementを使う

						Common::Data::Pointer tmp =
							p1->cast(Common::StringData(elmType._encodingForm));

						Common::Data::Pointer p2 = m_pData->getElement(i);
						Common::DataArrayData& dst =
							_SYDNEY_DYNAMIC_CAST(Common::DataArrayData&, *p2);
						dst.setElement(j, tmp);
					}
				}
			}
		}
	}

	if (!copied) {
		free();
		m_pConstData = pData_;
	}

	setNullBitMap();
}

//	FUNCTION public
//	Record::DirectFile::DataPackage::mergeData --
//
//	NOTES
//  引数で与えられたDataPackageのもつビットマップテーブル
//	（m_cNull）の内容をマージする。
//
//	ARGUMENTS
//		const DataPackage& cOther_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void 
DirectFile::DataPackage::
mergeData(const DirectFile::DataPackage& cOther_)
{
	// 更新時にターゲットの指定がないなら1番から始める必要がある
	TargetIterator cTargetIterator(cOther_.m_pTarget, &(cOther_.m_cMetaData));
	// いきなりhasNext() == falseのときがあるのでwhileを先に出す
	while (cTargetIterator.hasNext()) {//for each item in cOther
		Tools::FieldNum iFieldID = cTargetIterator.getNext();
		if (cOther_.m_cNull.test(iFieldID))
			m_cNull.set(iFieldID);
		else
			m_cNull.reset(iFieldID);
	}
}

//	FUNCTION private
//	Record::DirectFile::DataPackage::setNullBitMap --
//
//	NOTES
//		NullDataを探すと同時にデータ型の不整合もチェックする
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
DirectFile::DataPackage::
setNullBitMap()
{
	// Nullビットマップを初期化する
	m_cNull.initialize(1, m_cMetaData.getFieldNumber());
	
	// データに値が入っていないときは入れることができない
	// データに値が入っているときのみ処理する

	; _SYDNEY_ASSERT(m_pConstData);

	// 更新時にターゲットの指定がないなら1番から始める必要がある
	int n = m_pConstData->getCount();
	TargetIterator cTargetIterator(m_pTarget, &m_cMetaData);
	// いきなりhasNext() == falseのときがあるのでwhileを先に出す
	while (cTargetIterator.hasNext()) {
		Tools::FieldNum iFieldID = cTargetIterator.getNext();
		Tools::FieldNum iIndex = cTargetIterator.getIndex();
		if (iFieldID == 0) {
			// オブジェクトIDの型を調べる
			if (!checkType(iFieldID, iIndex)) {
				; _SYDNEY_ASSERT(false);
				_SYDNEY_THROW0(Exception::BadArgument);
			}

		} else if (n <= iIndex
				   || m_pConstData->getElement(iIndex)->isNull()) {
			// Nullならビットマップにセットする
			m_cNull.set(iFieldID);

		} else {
			// Nullでないなら型を調べる
			if (!checkType(iFieldID, iIndex)) {
				; _SYDNEY_ASSERT(false);
				_SYDNEY_THROW0(Exception::BadArgument);
			}
			m_cNull.reset(iFieldID);
		}
	}
}

//	FUNCTION private
//	Record::DirectFile::DataPackage::checkType --
//
//	NOTES
//
//	ARGUMENTS
//		const Record::Tools::FieldNum iFieldID_
//		const Record::Tools::FieldNum iIndex_
//
//	RETURN
//
//	EXCEPTIONS

bool
DirectFile::DataPackage::
checkType(Tools::FieldNum iFieldID_, Tools::FieldNum iIndex_)
{
	return (m_pConstData->getElement(iIndex_)->getType()
			== m_cMetaData.getDataType(iFieldID_)._name);
}

//
//	FUNCTION public
//	Record::DirectFile::DataPackage::setTargetField --
//
void
DirectFile::DataPackage::setTargetField(const TargetFields* pTargets_)
{
	m_pTarget = pTargets_;
}

//////////////////////
// DirectFileの定義 //
//////////////////////

//	FUNCTION public
//	Record::DirectFile::DirectFile --
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

DirectFile::
DirectFile(const Trans::Transaction& cTrans_,
		   const MetaData& cMetaData_)
	: FileBase(cTrans_, cMetaData_),
	  m_pReadIterator(0), m_pWriteIterator(0),
	  m_iMarkedObjectID(Tools::m_UndefinedObjectID),
	  m_pFileInformation(0), m_pReadFileInformation(0),
	  m_mapAttachedPage()
{
	setStorategy(cMetaData_);
}

//	FUNCTION public
//	Record::DirectFile::~DirectFile --
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

DirectFile::
~DirectFile()
{
	if (m_pReadIterator) delete m_pReadIterator, m_pReadIterator = 0;
	if (m_pWriteIterator) delete m_pWriteIterator, m_pWriteIterator = 0;
	if (m_pFileInformation) delete m_pFileInformation, m_pFileInformation = 0;
	if (m_pReadFileInformation) delete m_pReadFileInformation, m_pReadFileInformation = 0;
}

//	FUNCTION public
//	Record::DirectFile::getCount --
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
DirectFile::
getCount() const
{
	if (!isMounted(m_cTrans))
		return 0;

	// ヘッダーを読み込む
	FileInformation& cFileInfo = const_cast<DirectFile*>(this)->readFileInformation();

	return cFileInfo.getInsertedObjectNum();
}

//	FUNCTION public
//	Record::DirectFile::getOverhead --
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
DirectFile::
getOverhead() const
{
	if (!isMounted(m_cTrans))
		return 0.0;

	// Btree2の実装にあわせて1件あたりのコストにする ⇒ ProcessCostと同じ
	return getProcessCost();
}

//	FUNCTION public
//	Record::DirectFile::getProcessCost --
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		1件あたりの処理にかかる時間
//
//	EXCEPTIONS

double
DirectFile::
getProcessCost() const
{
	if (!isMounted(m_cTrans))
		return 0.0;

	// 一件得るコストは一ページ得るコストとほぼ同等
	const double dCostFileToMemory =
		static_cast<const double>(
			LogicalFile::Estimate::getTransferSpeed(
				LogicalFile::Estimate::File));

	return static_cast<double>(m_cStorageStrategy.m_VersionFileInfo._pageSize) / m_cMetaData.getObjectPerPage() / dCostFileToMemory;
}

//	FUNCTION public
//	Record::DirectFile::startVerification -- 物理ファイルの整合性検査を開始する
//
//	NOTES
//
//	ARGUMENTS
//	AAdmin::Verification::Treatment::Value iTreatment_
//		整合性検査の検査方法
//	Admin::Verification::Progress&	cProgress_
//		整合性検査の途中経過への参照
//	Record::VariableFile* pVariableFile_
//		可変長ファイルへのポインター
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void 
DirectFile::
startVerification(const Trans::Transaction&		Transaction_,
						const unsigned int				iTreatment_,
						Admin::Verification::Progress&	cProgress_,
						VariableFile* pVariableFile_)
{
	if (isMounted(m_cTrans)) {
		bool bDirect = false;
		bool bVariable = false;
		try {
			m_pFile->startVerification(m_cTrans, iTreatment_, cProgress_);
			bDirect = true;
			if (pVariableFile_) {
				pVariableFile_->startVerification(m_cTrans, iTreatment_, cProgress_);
				bVariable = true;
			}
		} catch (...) {
			if (bVariable) pVariableFile_->endVerification(m_cTrans, cProgress_);
			if (bDirect) m_pFile->endVerification(m_cTrans, cProgress_);
			_SYDNEY_RETHROW;
		}
	}
}

//	FUNCTION public
//	Record::DirectFile::endVerification -- 物理ファイルの整合性検査を終了する
//
//	NOTES
//
//	ARGUMENTS
//	Admin::Verification::Progress&	cProgress_
//		整合性検査の途中経過への参照
//	Record::VariableFile* pVariableFile_
//		可変長ファイルへのポインター
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void 
DirectFile::
endVerification(const Trans::Transaction&		Transaction_,
						 Admin::Verification::Progress&	cProgress_,
						 VariableFile* pVariableFile_)
{
	if (isMounted(m_cTrans)) {
		// cProgress_ の中身に拠らず endVerification() は実行。
		m_pFile->endVerification(m_cTrans, cProgress_);
		if (pVariableFile_) {
			pVariableFile_->endVerification(m_cTrans, cProgress_);
		}
	}
}

//	FUNCTION public
//	Record::DirectFile::verifyPhysicalFile -- 物理ファイルの整合性検査を行う
//
//	NOTES
//
//	ARGUMENTS
//	AAdmin::Verification::Treatment::Value iTreatment_
//		整合性検査の検査方法
//	Admin::Verification::Progress&	cProgress_
//		整合性検査の途中経過への参照
//	Record::VariableFile* pVariableFile_
//		可変長ファイルへのポインター
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DirectFile::
verifyPhysicalFile(Admin::Verification::Treatment::Value iTreatment_,
				   Admin::Verification::Progress&	cProgress_,
				   VariableFile* pVariableFile_)
{
	if (!isMounted(m_cTrans) || !cProgress_.isGood())
		return;

	; _SYDNEY_ASSERT(isAttached());

	bool bNeedToEnd = false;

	try {
		// 物理ファイルマネージャーに整合性検査の開始を指示する
		startVerification(m_cTrans, iTreatment_, cProgress_ ,pVariableFile_);
		bNeedToEnd = true;

		if (cProgress_.isGood()) {
			// 物理ファイルマネージャーに固定長ファイル内で
			// 使用しているすべての物理ページと
			// 可変長ファイル内で使用しているすべての物理ページとエリアを通知する
			// 同時に可変長ファイルの内容についても検査する
			notifyUsePage(iTreatment_, cProgress_, pVariableFile_);
		}

		this->detachPageAll(true);

		// 物理ファイルマネージャーに整合性検査の終了を指示する
		endVerification(m_cTrans, cProgress_ ,pVariableFile_);
		bNeedToEnd = false;

	} catch (...) {
		if (bNeedToEnd) {
			// endVerification 前は全てのページをデタッチする。
			this->detachPageAll(true);
			// 物理ファイルマネージャーに整合性検査の終了を指示する
			endVerification(m_cTrans, cProgress_ ,pVariableFile_);
		}
		_SYDNEY_VERIFY_ABORTED(cProgress_,
							   getPath(),
							   Message::VerifyFailed());
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Record::DirectFile::verifyContents --
//		固定長ファイル内の整合性を検査する
//
//	NOTES
//
//	ARGUMENTS
//   Admin::Verification::Treatment::Value iTreatment_
//		整合性検査の検査方法
//   Admin::Verification::Progress& cProgress_
//		整合性検査の途中経過への参照
//	 Record::VariableFile* pVariableFile_
//		可変長ファイルへのポインター
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
DirectFile::
verifyContents(Admin::Verification::Treatment::Value iTreatment_,
			   Admin::Verification::Progress& cProgress_)
{
	bool bContinue = (iTreatment_ & Admin::Verification::Treatment::Continue);

	if (!isMounted(m_cTrans) || !cProgress_.isGood())
		return;

	; _SYDNEY_ASSERT(isAttached());

	// この中で検査するのは
	// 2．先頭オブジェクト検査
	// 3．最終オブジェクト検査
	// 4．ページごとのオブジェクト数検査
	// 5．ページごとのオブジェクト数とオブジェクト総数の検査
	// 6．削除オブジェクトIDリンクの検査
	// である
	// 1．オブジェクト総数の検査は5でやることと同じことである

	const int n = 5;
	const ModUnicodeString name("record file");
	int i = 0;
	_SYDNEY_VERIFY_INFO(cProgress_, getPath(), Message::VerifyOnGoing(name, i++, n), iTreatment_);

	// 管理情報を読み込む
	FileInformation& cFileInfo = readFileInformation();

	// オブジェクト数が0なら先頭/最終オブジェクトはUndefinedである
	if (cFileInfo.getInsertedObjectNum() == 0) {
		if (cFileInfo.getFirstObjectID() != Tools::m_UndefinedObjectID
			|| cFileInfo.getLastObjectID() != Tools::m_UndefinedObjectID) {
			// ヘッダー情報がおかしい
			// オブジェクト総数とトップオブジェクト情報のどちらが
			// 正しいか分からないのでCorrectできない
			_SYDNEY_VERIFY_INCONSISTENT(
				cProgress_,
				getPath(),
				Message::InconsistentHeader(cFileInfo.getInsertedObjectNum(),
											cFileInfo.getFirstObjectID(),
											cFileInfo.getLastObjectID()));
		}
		// どちらにせよオブジェクト数が0ならこれ以上何もしなくてよい
		_SYDNEY_VERIFY_INFO(cProgress_, getPath(), Message::VerifyOnGoing(name, n, n), iTreatment_);
		return;
	}

	; _SYDNEY_ASSERT(cFileInfo.getInsertedObjectNum() > 0);

	if (cFileInfo.getFirstObjectID() >= Tools::m_UndefinedObjectID
		|| cFileInfo.getLastObjectID() >= Tools::m_UndefinedObjectID) {
		// ヘッダー情報がおかしい
		// オブジェクト総数とトップオブジェクト情報のどちらが
		// 正しいか分からないのでCorrectできない
		_SYDNEY_VERIFY_INCONSISTENT(
			cProgress_,
			getPath(),
			Message::InconsistentHeader(cFileInfo.getInsertedObjectNum(),
										cFileInfo.getFirstObjectID(),
										cFileInfo.getLastObjectID()));
		return;
	}

	// 読み込み用イテレーターがなければ作る
	if (!m_pReadIterator) {
		m_pReadIterator = new DirectIterator(*this,
											 cFileInfo.getFirstObjectID(),
											 cFileInfo.getLastObjectID());
	}

	// 2．先頭オブジェクトIDの検査
	_SYDNEY_VERIFY_INFO(cProgress_, getPath(), Message::VerifyOnGoing(name, i++, n), iTreatment_);
	{
		Admin::Verification::Progress cTmp(cProgress_.getConnection());
		verifyFirstObjectID(iTreatment_, cTmp, cFileInfo);
		cProgress_ += cTmp;
	}
	if (!cProgress_.isGood() && !bContinue)
		return;

	// 4．ページごとのオブジェクト数検査
	// 5．ページごとのオブジェクト数とオブジェクト総数の検査
	_SYDNEY_VERIFY_INFO(cProgress_, getPath(), Message::VerifyOnGoing(name, i++, n), iTreatment_);
	{
		Admin::Verification::Progress cTmp(cProgress_.getConnection());
		verifyObjectNumber(iTreatment_, cTmp, cFileInfo);
		cProgress_ += cTmp;
	}
	if (!cProgress_.isGood() && !bContinue)
		return;

	// 3．最終オブジェクトIDの検査
	// ★注意★
	// 最終オブジェクトは最後の方のページにあるはずなので
	// ページごとの検査が終了したときにやったほうが得である
	_SYDNEY_VERIFY_INFO(cProgress_, getPath(), Message::VerifyOnGoing(name, i++, n), iTreatment_);
	{
		Admin::Verification::Progress cTmp(cProgress_.getConnection());
		verifyLastObjectID(iTreatment_, cTmp, cFileInfo);
		cProgress_ += cTmp;
	}
	if (!cProgress_.isGood() && !bContinue)
		return;

	// 6．削除オブジェクトIDリンクの検査
	// ★注意★
	// オブジェクトIDリンクはページ順になっていないので
	// 4、5の検査と同時に行えない
	_SYDNEY_VERIFY_INFO(cProgress_, getPath(), Message::VerifyOnGoing(name, i++, n), iTreatment_);
	{
		Admin::Verification::Progress cTmp(cProgress_.getConnection());
		verifyFreeObjectID(iTreatment_, cTmp, cFileInfo);
		cProgress_ += cTmp;
	}
	if (!cProgress_.isGood() && !bContinue)
		return;
	_SYDNEY_VERIFY_INFO(cProgress_, getPath(), Message::VerifyOnGoing(name, i++, n), iTreatment_);
}

//	FUNCTION public
//	Record::DirectFile::read --
//
//	NOTES
//
//	ARGUMENTS
//		Tools::ObjectID iObjectID_
//			読み込むオブジェクトのオブジェクトID
//			Undefinedが指定されたときは現在指しているオブジェクトの次を取得する
//		const TargetFields* pTarget_
//			取得するフィールドを表すクラス
//		Record::DirectFile::DataPackage& cData_
//			[OUT]固定長ファイルに書かれた情報を格納する変数
//
//	RETURN
//		bool
//			結果が得られた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS

bool
DirectFile::
read(Tools::ObjectID iObjectID_,
	 const TargetFields* pTarget_,
	 DataPackage& cData_)
{
	if (!isMounted(m_cTrans))
		return false;

	; _SYDNEY_ASSERT(m_pFile);

	bool	bSucceed = false;	// 反復子の移動に成功したら true

	if (!m_pReadIterator) {

		// イテレータがないということは初めてreadがよばれたことになる。
		
		// 最新の管理情報を読み込む
		FileInformation& cFileInfo = readFileInformation();
	
		if (cFileInfo.getInsertedObjectNum() == 0) {
			// オブジェクト数がゼロならばデータは一つもないので
			// ヌルポインタを返す
			return false;
		}

		// イテレーターを作成する
		m_pReadIterator = new DirectIterator(*this,
											 cFileInfo.getFirstObjectID(),
											 cFileInfo.getLastObjectID());
	}

	// イテレーターを読み込むデータを指す位置に移動させる
	// 同時にページをattachする

	if (iObjectID_ != Tools::m_UndefinedObjectID) {
		// オブジェクトIDが指定されているのでそのIDにseekする
		bSucceed = m_pReadIterator->seek(iObjectID_, true /* bKeepAttach */);

	} else {
		bSucceed = m_pReadIterator->next();
	}
	if (!bSucceed) {
		// seekに失敗したら値に0が入ったまま返る
		return bSucceed;
	}

	// この時点でイテレーターが指すページがアタッチされている

	try {
		// 返り値を用意する
		cData_.allocate();

		// イテレーターの位置にある固定長データを読み込む
		// 読み込んだらページをdetachする
		m_pReadIterator->read(pTarget_, cData_);

		if (iObjectID_ != Tools::m_UndefinedObjectID) {
			// オブジェクトIDが指定されているので
			// データの取得に成功したらイテレーターを無効化する
			m_pReadIterator->invalidate();
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// detachPageAllを使う場合は物理ページ記述子へのポインターを0にするだけ
		m_pReadIterator->detachPage(DirectIterator::Operation::Read);
		_SYDNEY_RETHROW;
	}

	return bSucceed;
}

//	FUNCTION public
//	Record::DirectFile::readObjectHeader --
//
//	NOTES
//
//	ARGUMENTS
//		Tools::ObjectID iObjectID_
//			読み込むオブジェクトのオブジェクトID
//		Record::DirectFile::DataPackage& cData_
//			[OUT]固定長ファイルに書かれた情報を格納する変数
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
DirectFile::
readObjectHeader(Tools::ObjectID iObjectID_, DataPackage& cData_)
{
	if (!isMounted(m_cTrans))
		return;

	; _SYDNEY_ASSERT(m_pFile);

	// 書き込み用のイテレーターがなければ作る
	if (!m_pWriteIterator) {
		m_pWriteIterator = new DirectIterator(*this);
	}
	// 与えられたオブジェクトの位置を指すように移動する
	if (!m_pWriteIterator->seek(iObjectID_, true /* keep attach */)) {
		// seekできないときはエラー
		SydErrorMessage
			<< "Record::readVariable can't find a object. ID="
			<< iObjectID_
			<< ModEndl;
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	// 読み込む
	m_pWriteIterator->readObjectHeader(cData_);
	// detachしてしまうので、NULLビットマップをコピーする
	cData_.getNullBitMap().save();
	m_pWriteIterator->detachPage(DirectIterator::Operation::Read);
}

//	FUNCTION public
//	Record::DirectFile::readVariableID --
//
//	NOTES
//
//	ARGUMENTS
//		Tools::ObjectID iObjectID_
//			読み込むオブジェクトのオブジェクトID
//
//	RETURN
//		読み込んだ可変長のID
//
//	EXCEPTIONS

Tools::ObjectID
DirectFile::
readVariableID(Tools::ObjectID iObjectID_)
{
	// 削除でしか呼ばれないのでVacantではありえない

	; _SYDNEY_ASSERT(isMounted(m_cTrans));
	; _SYDNEY_ASSERT(m_pFile);

	// 書き込み用のイテレーターがなければ作る
	if (!m_pWriteIterator) {
		m_pWriteIterator = new DirectIterator(*this);
	}
	// 与えられたオブジェクトの位置を指すように移動する
	if (!m_pWriteIterator->seek(iObjectID_, true /* keep attach */)) {
		// seekできないときはエラー
		SydErrorMessage
			<< "Record::readVariable can't find a object. ID="
			<< iObjectID_
			<< ModEndl;
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	// 読み込む
	Tools::ObjectID iResult = m_pWriteIterator->readVariableID();
	m_pWriteIterator->detachPage(DirectIterator::Operation::Read);

	return iResult;
}

//	FUNCTION public
//	Record::DirectFile::insert --
//
//	NOTES
//
//	ARGUMENTS
//		const Common::DataArrayData& cObject_
//			挿入するデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Tools::ObjectID
DirectFile::
insert(DataPackage& cData_)
{
	substantiate();

	; _SYDNEY_ASSERT(isMounted(m_cTrans));
	; _SYDNEY_ASSERT(cData_.get());

	// 書き込み用のイテレーターがなければ作る
	if (!m_pWriteIterator) {
		m_pWriteIterator = new DirectIterator(*this);
	}

	// 管理情報はFile.cppで読み込まれているはず
	; _SYDNEY_ASSERT(m_pFileInformation);

	// ヘッダーにある空きオブジェクトIDリストの先頭があれば使う
	Tools::ObjectID iFreeObjectID =
		m_pFileInformation->getFirstFreeObjectID();
	bool bUseFreeID = (iFreeObjectID != Tools::m_UndefinedObjectID);

	// 空きオブジェクトIDがなければ最後のIDの次から探す
	if (!bUseFreeID && m_pFileInformation->getInsertedObjectNum() > 0) {
		Tools::ObjectID iLastObjectID = m_pFileInformation->getLastObjectID();
		if (m_pWriteIterator->seek(iLastObjectID, true /* keep attached */,
								   isBatch() ? DirectIterator::Operation::Batch
								   : DirectIterator::Operation::Write)) {
			iFreeObjectID = m_pWriteIterator->getNextFreeObjectID();
			// アタッチ済みのページと異なるIDだったらいったんデタッチする
			if (PhysicalPosition(iLastObjectID).getPageID() != PhysicalPosition(iFreeObjectID).getPageID()) {
				m_pWriteIterator->detachPage(DirectIterator::Operation::Read);
			}
		}
	}

	Tools::ObjectID iExpungedID =
		m_pWriteIterator->insert(cData_, iFreeObjectID, bUseFreeID);

	// 挿入が成功するとイテレーターは挿入したオブジェクトを指す
	Tools::ObjectID iResult = m_pWriteIterator->getObjectID();

	// 管理情報を正しい状態にする
	m_pFileInformation->setFirstFreeObjectID(iExpungedID);
	m_pFileInformation->validate(*m_pWriteIterator, FileInformation::ValidateOperation::Insert);

	// 管理情報を書き込むのはFile.cppで行う

	return iResult;
}

//	FUNCTION public
//	Record::DirectFile::update --
//
//	NOTES
//
//	ARGUMENTS
//		Tools::ObjectID iObjectID_
//			更新するオブジェクトのオブジェクトID
//		const TargetFields* pTarget_
//			更新するフィールドを表すクラス
//		Record::DirectFile::DataPackage& cData_
//			更新するデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
DirectFile::
update(Tools::ObjectID iObjectID_,
	   DataPackage& cData_,
	   const TargetFields* pTarget_)
{
	// updateは実体のないときに呼ばれるはずがない

	; _SYDNEY_ASSERT(isMounted(m_cTrans));
	; _SYDNEY_ASSERT(cData_.get());

	// 書き込み用のイテレーターがなければ作る
	if (!m_pWriteIterator) {
		m_pWriteIterator = new DirectIterator(*this);
	}
	if (!m_pWriteIterator->seek(iObjectID_, true, DirectIterator::Operation::Write)) {
		// seekできないときはエラー
		SydErrorMessage
			<< "Record::update can't find a object to be updated. ID="
			<< iObjectID_
			<< ModEndl;
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	m_pWriteIterator->update(cData_, pTarget_);

	// 管理情報を正しい状態にする
	// 管理情報はFile.cppで読み込まれているはず
	; _SYDNEY_ASSERT(m_pFileInformation);
	m_pFileInformation->validate(*m_pWriteIterator, FileInformation::ValidateOperation::Update);

	// 管理情報を書き込むのはFile.cppで行う
}

//	FUNCTION public
//	Record::DirectFile::expunge --
//
//	NOTES
//
//	ARGUMENTS
//		Tools::ObjectID iObjectID_
//			削除するオブジェクトのオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
DirectFile::
expunge(Tools::ObjectID iObjectID_)
{
	// 削除は実体のないときに呼ばれるはずがない

	; _SYDNEY_ASSERT(isMounted(m_cTrans));

	// 管理情報はFile.cppで読み込まれているはず
	; _SYDNEY_ASSERT(m_pFileInformation);

	// 書き込み用のイテレーターがなければ作る
	if (!m_pWriteIterator) {
		m_pWriteIterator
			= new DirectIterator(*this,
								 m_pFileInformation->getFirstObjectID(),
								 m_pFileInformation->getLastObjectID());
	} else {
		m_pWriteIterator->setStartObjectID(m_pFileInformation->getFirstObjectID());
		m_pWriteIterator->setEndObjectID(m_pFileInformation->getLastObjectID());
	}
	// イテレーターを削除するオブジェクトを指すようにする
	if (!m_pWriteIterator->seek(iObjectID_, true, DirectIterator::Operation::Expunge)) {
		// seekできないときはエラー
		SydErrorMessage
			<< "Record::expunge can't find a object to be expunge. ID="
			<< iObjectID_
			<< ModEndl;
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// 現在の空きオブジェクトIDリストの先頭を渡して削除を実行する
	Tools::ObjectID iFreeID = m_pFileInformation->getFirstFreeObjectID();

	m_pWriteIterator->expunge(iFreeID);

	// 管理情報を正しい状態にする
	m_pFileInformation->validate(*m_pWriteIterator, FileInformation::ValidateOperation::Expunge);

	// 管理情報を書き込むのはFile.cppで行う
}

//
//	FUNCTION public
//	Record::DirectFile::mark -- 巻き戻しの位置を記録する
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
DirectFile::
mark()
{
	; _SYDNEY_ASSERT(isMounted(m_cTrans));
	; _SYDNEY_ASSERT(m_pReadIterator);

	m_iMarkedObjectID = m_pReadIterator->getObjectID();
}

//
//	FUNCTION public
//	Record::DirectFile::rewind -- 記録した位置に戻る
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
//		レコードファイルがオープンされていない
//	[YET!]
//
void
DirectFile::
rewind()
{
	; _SYDNEY_ASSERT(isMounted(m_cTrans));
	
	if (m_iMarkedObjectID == Tools::m_UndefinedObjectID) {
		reset();

	} else {
		; _SYDNEY_ASSERT(m_pReadIterator);
		bool bResult = m_pReadIterator->seek(m_iMarkedObjectID);
		; _SYDNEY_ASSERT(bResult);
	}
}

//
//	FUNCTION public
//	Record::DirectFile::reset -- カーソルをリセットする
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
//		レコードファイルがオープンされていない
//	[YET!]
//
void
DirectFile::
reset()
{
	if (isMounted(m_cTrans))
		// イテレーターを破棄してしまう
		delete m_pReadIterator, m_pReadIterator = 0;
}

//	FUNCTION public
//	Record::DirectFile::detachFile --
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
DirectFile::
detachFile()
{
	if (m_pReadIterator) delete m_pReadIterator, m_pReadIterator = 0;
	if (m_pWriteIterator) delete m_pWriteIterator, m_pWriteIterator = 0;

	FileBase::detachFile();

	if (m_pFileInformation) delete m_pFileInformation, m_pFileInformation = 0;
	if (m_pReadFileInformation) delete m_pReadFileInformation, m_pReadFileInformation = 0;
}

//	FUNCTION public
//	Record::DirectFile::detachPageAll --
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
DirectFile::
detachPageAll(bool bSucceeded_)
{
	if (m_pReadIterator)
		m_pReadIterator->detachPage(DirectIterator::Operation::Read);
	
	if (bSucceeded_
		|| m_cTrans.getCategory() == Trans::Transaction::Category::ReadOnly
		|| isBatch())
	{
		// 正常終了…

		// バッチモードのときは常に成功時と同じ処理にする
		// 異常時には上位層でrecoverによりトランザクション開始時の版に戻される

		// アタッチしたすべての物理ページ記述子をデタッチする。

		this->m_pFile->detachPageAll();

		if (m_pFileInformation) {
			m_pFileInformation->releasePage();//先に detachPageAll() しているので、
											  //同一ページを参照しているこちら側は外す。
		}
	}
	else
	{
		// 異常終了…

		// アタッチしたすべての物理ページ記述子をデタッチし、
		// ページの内容をアタッチ前の状態に戻す。

		//
		// 代表オブジェクトを格納するファイルが使う
		// 物理ファイルは“物理ページ管理機能付き物理ファイル”。
		// 物理ページ管理機能付き物理ファイルの
		// PhysicalFile::File::recoverPageAll()内では、
		// トランザクション記述子は不要なので、引数がない方の
		// PhysicalFile::File::recoverPageAll()を呼び出す。
		//

		this->m_pFile->recoverPageAll();

		if (m_pFileInformation) {
			// 失敗したときはFileInformationを書き戻す
			m_pFileInformation->releasePage();
			m_pFileInformation->recover();
		}
	}

	// イテレーターの場所のページ記述子を放棄（同一ページを参照しているこちら側は外す。）
	if (m_pReadIterator) {
		m_pReadIterator->releasePage();
	}
	if (m_pWriteIterator) {
		m_pWriteIterator->releasePage();
	}
	m_mapAttachedPage.erase(m_mapAttachedPage.begin(), m_mapAttachedPage.end());
}

//	FUNCTION public
//	Record::DirectFile::getInformationPageID --
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ヘッダーページのID
//
//	EXCEPTIONS

PhysicalFile::PageID
DirectFile::
getInformationPageID() const
{
	return _iInformationPageID;
}

//	FUNCTION public
//	Record::DirectFile::readFileInformation --
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ファイル管理情報
//
//	EXCEPTIONS

FileInformation&
DirectFile::
readFileInformation()
{
	; _SYDNEY_ASSERT(isMounted(m_cTrans));

	if (!m_pReadFileInformation) {
		m_pReadFileInformation =
			new FileInformation(m_cTrans,
								*m_pFile,
								_iInformationPageID,
								FileInformation::OpenOperation::Read);
		m_pReadFileInformation->reload(false /* do not repair */);

	} else {
		// 読み書き用トランザクションの場合は毎回読み直す
		if (m_cTrans.getCategory() == Trans::Transaction::Category::ReadWrite) {
			m_pReadFileInformation->reload(false /* do not repair */);
		}
	}

	return *m_pReadFileInformation;
}

//	FUNCTION public
//	Record::DirectFile::readFileInformationForUpdate --
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ファイル管理情報
//
//	EXCEPTIONS

FileInformation&
DirectFile::
readFileInformationForUpdate()
{
	substantiate();

	; _SYDNEY_ASSERT(isMounted(m_cTrans));

	if (!m_pFileInformation) {
		m_pFileInformation =
			new FileInformation(m_cTrans,
								*m_pFile,
								_iInformationPageID,
								isBatch()
								? FileInformation::OpenOperation::Batch
								: FileInformation::OpenOperation::Update);
	}
	m_pFileInformation->reload(true /* DoRepair */, true /* keep attach */);

	return *m_pFileInformation;
}

//	FUNCTION public
//	Record::DirectFile::syncFileInformation --
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
DirectFile::
syncFileInformation()
{
	; _SYDNEY_ASSERT(isMounted(m_cTrans));
	; _SYDNEY_ASSERT(m_pFileInformation);

	m_pFileInformation->sync();
}

//////////////////
// 内部メソッド //
//////////////////

//	FUNCTION protected
//	Record::DirectFile::setStorategy --
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
DirectFile::
setStorategy(const MetaData& cMetaData_)
{
	// 格納戦略を設定する
	m_cStorageStrategy.m_PhysicalFileType = _ePhysicalFileType;
	m_cStorageStrategy.m_PageUseRate = PhysicalFile::ConstValue::DefaultPageUseRate;
	m_cStorageStrategy.m_VersionFileInfo._pageSize = cMetaData_.getDirectPageSize();

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
//	Record::DirectFile::getPathPart --
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
DirectFile::
getPathPart() const
{
	return _cstrFileName;
}

//	FUNCTION private
//	Record::DirectFile::notifyUsePage --
//		物理ファイルマネージャに、レコードファイル内で使用している
//		すべての物理ページを通知する
//
//	NOTES
//
//	ARGUMENTS
//   Admin::Verification::Treatment::Value iTreatment_
//		整合性検査の検査方法
//   Admin::Verification::Progress& cProgress_
//		整合性検査の途中経過への参照
//	 Record::VariableFile* pVariableFile_
//		可変長ファイルへのポインター
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
DirectFile::
notifyUsePage(Admin::Verification::Treatment::Value iTreatment_,
			  Admin::Verification::Progress& cProgress_,
			  VariableFile* pVariableFile_)
{
	bool bContinue = (iTreatment_ & Admin::Verification::Treatment::Continue);

	if (!isMounted(m_cTrans))
		return;

	; _SYDNEY_ASSERT(m_pFile);

	// 管理情報を読み込む
	FileInformation cFileInfo(
		m_cTrans,
		*m_pFile,
		_iInformationPageID,
		FileInformation::OpenOperation::Verify,
		&cProgress_);
	cFileInfo.reload(false);// OpenOperation::Verify を指定しているので、verify モードで attach()

	// 管理情報のページは常に存在する
	m_pFile->notifyUsePage(m_cTrans,
						   cProgress_,
						   _iInformationPageID);

	if (!cProgress_.isGood() && !bContinue) {
		return;
	}

	// 次のページを順にたどって登録する
	// ★注意★
	// DirectFileは空き領域管理をしていないので単純にページをたどればよい
	// VariableIDを読み込むためにイテレーターを使う
	// また、VariableIDを読み込むたびに可変長ファイルの整合性検査も行う
	// これはオブジェクト数が大きくなったときにメモリ上に載せるデータ量が
	// 必要以上に大きくならないようにするためである

	UseInfo cUseInfo;

	if (!m_pReadIterator) {
		m_pReadIterator = new DirectIterator(*this);
	}

	const int n = m_pFile->getLastPageID(m_cTrans);
	m_pFile->detachPageAll();
	const int step = ModMax(5, (n + 4) / 5);
	const ModUnicodeString name("physical file");
	int i = step;
	int sum = 0;
	_SYDNEY_VERIFY_INFO(cProgress_, getPath(), Message::VerifyOnGoing(name, sum, n), iTreatment_);

	if (pVariableFile_) {
		pVariableFile_->prepareFreeAreaManager();//cache off
	}
	PhysicalFile::PageID iPageID;
	while ( (iPageID = m_pReadIterator->nextPage(&cProgress_) )
		            != PhysicalFile::ConstValue::UndefinedPageID) {

		if (!cProgress_.isGood() && !bContinue) {
			return;
		}
		if (!--i) {
			sum += step;
			_SYDNEY_VERIFY_INFO(cProgress_, getPath(), Message::VerifyOnGoing(name, sum, n), iTreatment_);
			i = step;
		}

		// 使用するページIDを物理ファイルに登録（通知）する
		{
			Admin::Verification::Progress cTmp(cProgress_.getConnection());
			m_pFile->notifyUsePage(m_cTrans,
								   cTmp,
								   iPageID);
			cProgress_ += cTmp;
		}
		if (!cProgress_.isGood()) {
			if (bContinue) {
				continue;
			} else {
				m_pReadIterator->detachPage(DirectIterator::Operation::Read);
				return;
			}
		}

		if (pVariableFile_) {
			// このページに載っているオブジェクトから可変長オブジェクトIDを読む

			if (!m_pReadIterator->isExist()) {
				// このページにはオブジェクトはないので次へ
				continue;
			}

			// キャンセル対応（∵VariableFile が有効な場合は時間がかかることがある）
			Trans::Transaction& cTrans = const_cast<Trans::Transaction&>(m_cTrans);
			if (cTrans.isCanceledStatement()) _SYDNEY_THROW0(Exception::Cancel);

			// スコープの最後で detachPageAll() する。（∵実行メモリ削減）
			AutoDetachPageAll<VariableFile> pages(pVariableFile_);

			// 同じページ内にあるだけのオブジェクトIDについてセットする
			do {
				DataPackage cObjectHeader(m_cMetaData);
				m_pReadIterator->readObjectHeader(cObjectHeader);
				if (cObjectHeader.getVariableID() != Tools::m_UndefinedObjectID) {
					// ここで物理ファイルの内容を検査する
					{
						Admin::Verification::Progress cTmp(cProgress_.getConnection());
						pVariableFile_->verifyContents(cObjectHeader, iTreatment_, cTmp);
						cProgress_ += cTmp;
					}
					if (!cProgress_.isGood()) {
						if (bContinue) {
							continue;
						} else {
							m_pReadIterator->detachPage(DirectIterator::Operation::Read);
							return;
						}
					}
					// 物理ファイルに使用中のページIDとエリアIDを知らせるために集めておく
					{
						Admin::Verification::Progress cTmp(cProgress_.getConnection());
						pVariableFile_->setUseInfo(cObjectHeader.getVariableID(), cUseInfo ,cTmp);
						cProgress_ += cTmp;
					}
					if (!cProgress_.isGood() && !bContinue) {
						m_pReadIterator->detachPage(DirectIterator::Operation::Read);
						return;
					}
				}
			} while (m_pReadIterator->getNextObjectIDVerify(&cProgress_) != Tools::m_UndefinedObjectID);
			if (!cProgress_.isGood() && !bContinue) {
				return;
			}

			pages.succeeded();
		}
	}
	_SYDNEY_VERIFY_INFO(cProgress_, getPath(), Message::VerifyOnGoing(name, n, n), iTreatment_);

	// whileループが最後までいったらReadIteratorは
	// どこもアタッチしていない状態になっているはずである

	// 空き領域リストのページIDとエリアIDがあれば、物理ファイルに知らせるために集めておく
	if (pVariableFile_) {
		Tools::ObjectID iFreeID = cFileInfo.getFirstFreeVariableObjectID();
		if (iFreeID != Tools::m_UndefinedObjectID)
		{
			// スコープの最後で detachPageAll() する。
			AutoDetachPageAll<VariableFile> pages(pVariableFile_);

			// FreeVariableObjectのリンクの構造は、通常の LinkedObject とは異なる。
			// （先頭位置に、オブジェクトタイプを持たない）
			// その為に、通常の LinkedObject 用の setUseInfo() は使用できない。
			{
				Admin::Verification::Progress cTmp(cProgress_.getConnection());
				pVariableFile_->setFreeAreaUseInfo(iFreeID, cUseInfo ,cTmp);
				cProgress_ += cTmp;
			}
			if (!cProgress_.isGood() && !bContinue) {
				return;
			}
			pages.succeeded();
		}
	}

	if (pVariableFile_ && cUseInfo.m_Table.getSize()) {
		// スコープの最後で detachPageAll() する。
		AutoDetachPageAll<VariableFile> pages(pVariableFile_);

		// 集めたページIDとエリアIDを使って可変長ファイルの
		// 物理ファイルに対して整合性検査を行う
		Admin::Verification::Progress cTmp(cProgress_.getConnection());
		pVariableFile_->verifyPhysicalFile(cUseInfo, iTreatment_, cTmp);
		cProgress_ += cTmp;
		if (!cProgress_.isGood() && !bContinue) {
			return;
		}
		pVariableFile_->discardFreeAreaManager();
		pages.succeeded();
	}
}

//	FUNCTION private
//	Record::DirectFile::verifyFirstObjectID --
//		固定長ファイル内の整合性を検査する
//
//	NOTES
//
//	ARGUMENTS
//   Admin::Verification::Treatment::Value iTreatment_
//		整合性検査の検査方法
//   Admin::Verification::Progress& cProgress_
//		整合性検査の途中経過への参照
//	 Record::FileInformation& cFileInfo_
//		ヘッダー情報
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
DirectFile::
verifyFirstObjectID(Admin::Verification::Treatment::Value iTreatment_,
					Admin::Verification::Progress& cProgress_,
					const FileInformation& cFileInfo_)
{
	if (!isMounted(m_cTrans))
		return;

	; _SYDNEY_ASSERT(m_pReadIterator);

	// 先頭のオブジェクトIDにseekする
	if (!m_pReadIterator->seek(cFileInfo_.getFirstObjectID(), true /* keep attached */)) {
		// 先頭オブジェクトが正しい状態でない
		_SYDNEY_VERIFY_INCONSISTENT(
			cProgress_,
			getPath(),
			Message::ObjectNotFound(cFileInfo_.getFirstObjectID()));
		return;
	}
	try {
		Tools::ObjectID iPrevID = m_pReadIterator->getPrevObjectID();
		if (iPrevID != Tools::m_UndefinedObjectID) {
			// 先頭の前にオブジェクトがある
			_SYDNEY_VERIFY_INCONSISTENT(
				cProgress_,
				getPath(),
				Message::ExistTopObject(cFileInfo_.getFirstObjectID(),
										iPrevID));
			m_pReadIterator->detachPage(DirectIterator::Operation::Read);
			return;
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		m_pReadIterator->detachPage(DirectIterator::Operation::Read);
		_SYDNEY_RETHROW;
	}
	m_pReadIterator->detachPage(DirectIterator::Operation::Read);
}

//	FUNCTION private
//	Record::DirectFile::verifyLastObjectID --
//		固定長ファイル内の整合性を検査する
//
//	NOTES
//
//	ARGUMENTS
//   Admin::Verification::Treatment::Value iTreatment_
//		整合性検査の検査方法
//   Admin::Verification::Progress& cProgress_
//		整合性検査の途中経過への参照
//	 Record::FileInformation& cFileInfo_
//		ヘッダー情報
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
DirectFile::
verifyLastObjectID(Admin::Verification::Treatment::Value iTreatment_,
				   Admin::Verification::Progress& cProgress_,
				   const FileInformation& cFileInfo_)
{
	if (!isMounted(m_cTrans))
		return;

	; _SYDNEY_ASSERT(m_pReadIterator);

	// 最終オブジェクトIDにseekする
	if (!m_pReadIterator->seek(cFileInfo_.getLastObjectID(), true /* keep attached */)) {
		// 先頭オブジェクトが正しい状態でない
		_SYDNEY_VERIFY_INCONSISTENT(
			cProgress_,
			getPath(),
			Message::ObjectNotFound(cFileInfo_.getLastObjectID()));
		return;
	}
	try {
		Tools::ObjectID iNextID = m_pReadIterator->getNextObjectID();
		if (iNextID != Tools::m_UndefinedObjectID) {
			// 最終の後にオブジェクトがある
			_SYDNEY_VERIFY_INCONSISTENT(
				cProgress_,
				getPath(),
				Message::ExistLastObject(cFileInfo_.getLastObjectID(),
										 iNextID));
			m_pReadIterator->detachPage(DirectIterator::Operation::Read);
			return;
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		m_pReadIterator->detachPage(DirectIterator::Operation::Read);
		_SYDNEY_RETHROW;
	}
	m_pReadIterator->detachPage(DirectIterator::Operation::Read);
}

//	FUNCTION private
//	Record::DirectFile::verifyObjectNumber --
//		固定長ファイル内の整合性を検査する
//
//	NOTES
//
//	ARGUMENTS
//   Admin::Verification::Treatment::Value iTreatment_
//		整合性検査の検査方法
//   Admin::Verification::Progress& cProgress_
//		整合性検査の途中経過への参照
//	 Record::FileInformation& cFileInfo_
//		ヘッダー情報
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
DirectFile::
verifyObjectNumber(Admin::Verification::Treatment::Value iTreatment_,
				   Admin::Verification::Progress& cProgress_,
				   const FileInformation& cFileInfo_)
{
	bool bContinue = (iTreatment_ & Admin::Verification::Treatment::Continue);

	if (!isMounted(m_cTrans))
		return;

	; _SYDNEY_ASSERT(m_pReadIterator);

	// 以下の処理のためにイテレーターを初期化しておく
	m_pReadIterator->invalidate();

	// ページごとの整合性検査を行いながらオブジェクト数を合計する
	PhysicalFile::PageID iPageID;
	Tools::ObjectNum iObjectNum = 0;
	while ((iPageID = m_pReadIterator->nextPage()) 
		   != PhysicalFile::ConstValue::UndefinedPageID) {

		Admin::Verification::Progress cTmp(cProgress_.getConnection());
		iObjectNum += m_pReadIterator->verifyPageData(iTreatment_, cTmp);
		cProgress_ += cTmp;
		if (!cProgress_.isGood() && !bContinue) {
			return;
		}
	}

	if (iObjectNum != cFileInfo_.getInsertedObjectNum()) {
		_SYDNEY_VERIFY_INCONSISTENT(
			cProgress_,
			getPath(),
			Message::DiscordObjectNum(cFileInfo_.getInsertedObjectNum(),
									  iObjectNum));
		return;
	}
}

//	FUNCTION private
//	Record::DirectFile::verifyFreeObjectID --
//		固定長ファイル内の整合性を検査する
//
//	NOTES
//
//	ARGUMENTS
//   Admin::Verification::Treatment::Value iTreatment_
//		整合性検査の検査方法
//   Admin::Verification::Progress& cProgress_
//		整合性検査の途中経過への参照
//	 Record::FileInformation& cFileInfo_
//		ヘッダー情報
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
DirectFile::
verifyFreeObjectID(Admin::Verification::Treatment::Value iTreatment_,
				   Admin::Verification::Progress& cProgress_,
				   const FileInformation& cFileInfo_)
{
	bool bContinue = (iTreatment_ & Admin::Verification::Treatment::Continue);

	if (!isMounted(m_cTrans))
		return;

	; _SYDNEY_ASSERT(m_pReadIterator);

	// 以下の処理のためにイテレーターを初期化しておく
	m_pReadIterator->invalidate();

	// 削除オブジェクトIDを順にたどりながら検査する
	Tools::ObjectID iFreeObjectID = cFileInfo_.getFirstFreeObjectID();

	Admin::Verification::Progress cTmp(cProgress_.getConnection());
	while (iFreeObjectID != Tools::m_UndefinedObjectID) {
		iFreeObjectID =
			m_pReadIterator->verifyFreeObjectID(iTreatment_,
												cTmp,
												iFreeObjectID);
		if (!cTmp.isGood() && !bContinue) {
			cProgress_ += cTmp;
			return;
		}
	}
	cProgress_ += cTmp;
}

//	FUNCTION private
//	Record::DirectFile::substantiate -- 実体を作成する
//
//	NOTES
//
//	ARGUMENTS
// 		なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
DirectFile::
substantiate()
{
	if (isMounted(m_cTrans))
		return;

	enum {
		None,
		Directory,
		File
	} eStatus = None;

	const bool bIsAttached = isAttached();
	if (!bIsAttached) {
		attachFile();
		; _SYDNEY_ASSERT(m_pFile);
	}

	eStatus = Directory;

	try {

		// ファイルを作る
		m_pFile->create(m_cTrans);
		eStatus = File;

		////////////////////////////
		// ヘッダーページを作成する
		////////////////////////////
		//
		// ページをアロケートして管理情報を書き込む
		//
		m_pFile->allocatePage(m_cTrans, _iInformationPageID);
		// コンストラクターがメンバーに初期値を入れてくれる
		FileInformation cFileInfo(
			m_cTrans,
			*m_pFile,
			_iInformationPageID,
			FileInformation::OpenOperation::Update);
		cFileInfo.touch();
		cFileInfo.sync();

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		try {
			detachPageAll(false);

			switch (eStatus) {
			case File:
				m_pFile->destroy(m_cTrans);
				// thru
			case Directory:

				// サブディレクトリーを破棄する
				//
				//【注意】	サブディレクトリは
				//			実体である物理ファイルの生成時に
				//			必要に応じて生成されるが、
				//			エラー時には削除されないので、
				//			この関数で削除する必要がある

				ModOsDriver::File::rmAll(getPath(), ModTrue);
			}
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

	detachPageAll(true);

	if (!bIsAttached)
		detachFile();
}

//
//	Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
