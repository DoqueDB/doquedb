// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VariableIterator.cpp -- 可変長フィールドを格納するファイルをアクセスするイテレーター
// 
// Copyright (c) 2001, 2003, 2004, 2006, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"
#include "Record/VariableIterator.h"
#include "Record/VariableField.h"
#include "Record/VariableFile.h"
#include "Record/LinkedObject.h"
#include "Record/MetaData.h"
#include "Record/OpenParameter.h"
#include "Record/PhysicalPosition.h"
#include "Record/TargetFields.h"
#include "Record/Tools.h"
#include "Record/Message_BadFreeObjectID.h"
#include "Record/Message_InconsistentPageObjectNumber.h"
#include "Record/Message_InconsistentVariableSize.h"
#include "Record/Message_ObjectNotFound.h"
#include "Record/Debug.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/NullData.h"
#include "Common/DecimalData.h"

#include "Buffer/Page.h"
#include "Exception/Unexpected.h"
#include "LogicalFile/ObjectID.h"
#include "PhysicalFile/Page.h"

_SYDNEY_USING
_SYDNEY_RECORD_USING

namespace
{
	// VariableFileの置き換え優先度
	const Buffer::ReplacementPriority::Value
		_ePriority = Buffer::ReplacementPriority::Low;

	// Operation::ValueからLinkedObject::Operation::Valueを得るための表
	const LinkedObject::Operation::Value
		_AccessModeTable[VariableIterator::Operation::ValueNum] =
	{
		LinkedObject::Operation::Read,		// Read
		LinkedObject::Operation::Write,		// Write
		LinkedObject::Operation::Batch		// Batch
	};

	// Linked Object をアタッチ・デタッチする
	class AutoAttachObject {
	public:
		AutoAttachObject(LinkedObject& cLinkedObject_ ,Tools::ObjectID iTopObjectID_)
			: m_cLinkedObject(cLinkedObject_)
		{
			m_cLinkedObject.attachObject(iTopObjectID_);
		}
		AutoAttachObject(LinkedObject& cLinkedObject_ ,Tools::ObjectID iTopObjectID_ ,Admin::Verification::Progress& cProgress_)
			: m_cLinkedObject(cLinkedObject_)
		{
			m_cLinkedObject.verifyObject(iTopObjectID_ ,cProgress_);
		}
		~AutoAttachObject()
		{
			m_cLinkedObject.detachObject();
		}
	private:
		LinkedObject& m_cLinkedObject;
	};



} // namespace

//	FUNCTION public
//	Record::VariableIterator::VariableIterator --
//
//	NOTES
//
//	ARGUMENTS
//		Record::VariableFile& cFile_
//			イテレーターでアクセスするファイル
//		Record::VariableIterator::Operation::Value eOperation_
//			イテレーターで行う操作の種類
//
//	RETURN
//		なし
//
//	EXCEPTIONS

VariableIterator::
VariableIterator(VariableFile& cFile_, Operation::Value eOperation_)
	: m_cFile(cFile_),
	  m_eOperation(eOperation_),
	  m_iObjectID(Tools::m_UndefinedObjectID),
	  m_pLinkedObject(0)
{
	// ファイルはアタッチしていなければならない
	; _SYDNEY_ASSERT(cFile_.m_pFile);
}

//	FUNCTION public
//	Record::VariableIterator::~VariableIterator --
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

VariableIterator::
~VariableIterator()
{
	delete m_pLinkedObject, m_pLinkedObject = 0;
}

#ifdef OBSOLETE
//	FUNCTION public
//	Record::VariableIterator::isValid --
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		有効ならtrue
//
//	EXCEPTIONS

bool
VariableIterator::
isValid() const
{
	return (m_iObjectID < Tools::m_UndefinedObjectID);
}
#endif //OBSOLETE

//	FUNCTION public
//	Record::VariableIterator::getObjectID --
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		イテレーターが現在指しているオブジェクトID
//
//	EXCEPTIONS

Tools::ObjectID
VariableIterator::
getObjectID() const
{
	return m_iObjectID;
}

//	FUNCTION public
//	Record::VariableIterator::seek --
//
//	NOTES
//
//	ARGUMENTS
//		Record::Tools::ObjectID iObjectID_
//			移動するオブジェクトID
//
//	RETURN
//		指定されたオブジェクトIDが正しいものであればtrue
//
//	EXCEPTIONS

bool
VariableIterator::
seek(Tools::ObjectID iObjectID_)
{
	if (iObjectID_ == Tools::m_UndefinedObjectID) {
		return false;
	}

	// LinkedObjectがなければ用意する
	initializeLinkedObject();

#ifdef DEBUG
	PhysicalPosition pos(iObjectID_);
	; _SYDNEY_ASSERT(m_cFile.m_pFile->isUsedPage(m_cFile.m_cTrans, pos.m_PageID));
#endif

	m_pLinkedObject->attachObject(iObjectID_);
	m_iObjectID = iObjectID_;
	return true;
}

//	FUNCTION public
//	Record::VariableIterator::read --
//
//	NOTES
//
//	ARGUMENTS
//		Record::DirectFile::DataPackage& cData_
//			固定長レコードの情報を格納する変数
//		const TargetFields* pTarget_
//			取得するフィールド
//			0のときは全フィールドを取得する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VariableIterator::
read(DirectFile::DataPackage& cData_,
	 const TargetFields* pTarget_)
{
	; _SYDNEY_ASSERT(cData_.get());
	// すでにattachObjectされていなければならない
	_SYDNEY_ASSERT(m_pLinkedObject);
	_SYDNEY_ASSERT(m_pLinkedObject->isAttached());

	// ヘッダーを読み込む
	readHeader(cData_);

	// フィールド値を得るためのイテレーター
	VariableField cField(m_cFile.m_cMetaData, m_cObjectHeader, *m_pLinkedObject);

	// 所得すべきフィールドIDを得ながら可変長の値をセットしていく
	TargetIterator cTargetIterator(pTarget_, &m_cFile.m_cMetaData);
	do {
		Tools::FieldNum iFieldID = cTargetIterator.getNext();

		// File::divideTargetsで固定長から分離されている
		_SYDNEY_ASSERT(m_cFile.m_cMetaData.isVariable(iFieldID));

		if (cData_.isNull(iFieldID)) {
			// NullデータはDirectFileのほうでセットされている
#ifdef RECORD_CHECK_NULL
			if (!cData_.get()->getElement(cTargetIterator.getIndex())->isNull()) {
				_SYDNEY_THROW0(Exception::Unexpected);
			}
#endif
			continue;
		}
		// それ以外のデータを読み込む
		bool bResult = cField.seek(iFieldID);
		_SYDNEY_ASSERT(bResult);

		Common::Data::Pointer pData
			= cData_.get()->getElement(cTargetIterator.getIndex());
		cField.readField(*pData);

#ifdef DEBUG
		SydRecordDebugMessage
			<< "Read variable field: "
			<< iFieldID
			<< " data: "
			<< cData_.get()->getElement(cTargetIterator.getIndex())->getString()
			<< ModEndl;
#endif

	} while (cTargetIterator.hasNext());
}

#ifdef RECORD_CHECK_NULL
//	FUNCTION public
//	Record::VariableIterator::assureNull --
//
//	NOTES
//		読み込んだデータがすべてNullであるべきときにそれを確認する
//
//	ARGUMENTS
//		DirectFile::DataPackage& cData_
//			確認するデータ
//		TargetFields* pTarget_ = 0
//			確認するフィールド
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VariableIterator::
assureNull(DirectFile::DataPackage& cData_,
		   const TargetFields* pTarget_)
{
	TargetIterator cTargetIterator(pTarget_, &m_cFile.m_cMetaData);
	do {
		(void)cTargetIterator.getNext();

		// NullデータはDirectFileのほうでセットされている
		if (!cData_.get()->getElement(cTargetIterator.getIndex())->isNull()) {
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	} while (cTargetIterator.hasNext());
}
#endif

//	FUNCTION public
//	Record::VariableIterator::insert --
//
//	NOTES
//		挿入が成功すると挿入したページを指す
//
//	ARGUMENTS
//		DirectFile::DataPackage& cData_
//			挿入するデータ
//		Tools::ObjectID& iFreeID_
//			空き領域リストの先頭
//		TargetFields* pTarget_ = 0
//			更新時にフィールドとデータの位置を対応付けるのに用いる
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VariableIterator::
insert(const DirectFile::DataPackage& cData_,
	   Tools::ObjectID& iFreeID_,
	   const TargetFields* pTarget_)
{
	; _SYDNEY_ASSERT(cData_.get());

	if (!isNecessaryToInsert(cData_)) {
		// 挿入する必要がない
		m_iObjectID = Tools::m_UndefinedObjectID;
		return;
	}

	// ターゲットが0なら可変長だけを指すものに変える
	const TargetFields* pTarget = pTarget_ ? pTarget_ : m_cFile.m_cMetaData.getVariableFields();

	// ヘッダー情報を作ると同時に必要なサイズを得る
	Os::Memory::Size iTotalSize = makeHeader(cData_, pTarget);
	if (iTotalSize == 0) {
		// 可変長フィールドの合計サイズが0ということは
		// すべてnullということ
		invalidate();
		return;
	}

	// LinkedObjectがなければ用意する
	initializeLinkedObject();

	// LinkedObjectを作成する
#ifdef DEBUG
	SydRecordSizeMessage
		<< "Create object for " << m_cFile.getPath()
		<< ModEndl;
#endif
	Tools::ObjectID iResult = m_pLinkedObject->create(iTotalSize, iFreeID_);
	; _SYDNEY_ASSERT(iResult < Tools::m_UndefinedObjectID);

	// 書き込みのためにアタッチする
	m_pLinkedObject->attachObject(iResult);

	// ヘッダーを書き込む
	writeHeader(cData_, pTarget);

	// フィールドにアクセスするためのイテレーター
	VariableField cField(m_cFile.m_cMetaData, m_cObjectHeader, *m_pLinkedObject);

	// 書き込むべきフィールドIDを得ながら可変長の値をセットしていく
	TargetIterator cTargetIterator(pTarget, &m_cFile.m_cMetaData);
	do {
		Tools::FieldNum iFieldID = cTargetIterator.getNext();
		; _SYDNEY_ASSERT(iFieldID != 0);
		; _SYDNEY_ASSERT(m_cFile.m_cMetaData.isVariable(iFieldID));

		if (cData_.isNull(iFieldID)) {
			// Nullデータは書き込まない
			continue;
		}
		// それ以外のデータは書き込む
		bool bResult = cField.seek(iFieldID);
		; _SYDNEY_ASSERT(bResult);

		cField.updateField(
			cData_.get()->getElement(cTargetIterator.getIndex()));

#ifdef DEBUG
		SydRecordDebugMessage
			<< "Insert variable field: "
			<< iFieldID
			<< " data: "
			<< cData_.get()->getElement(cTargetIterator.getIndex())->getString()
			<< ModEndl;
#endif

	} while (cTargetIterator.hasNext());

	// 本来はここでdetachすべきでない
	detachObject();

	m_iObjectID = iResult;
}

//	FUNCTION public
//	Record::VariableIterator::update --
//
//	NOTES
//
//	ARGUMENTS
//		DirectFile::DataPackage& cOldObjectHeader_
//			更新前のnull bitmapと可変長オブジェクトID
//		DirectFile::DataPackage& cNewData_
//			更新後のデータ
//		const TargetFields* pTarget_
//			更新するフィールド
//		Tools::ObjectID& iFreeID_
//			空き領域リストの先頭
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VariableIterator::
update(const DirectFile::DataPackage& cOldObjectHeader_,
	   const DirectFile::DataPackage& cNewData_,
	   const TargetFields* pTarget_,
	   Tools::ObjectID& iFreeID_)
{
	; _SYDNEY_ASSERT(cNewData_.get());
	; _SYDNEY_ASSERT(pTarget_);

	// 更新に使うデータを格納するPackage
	DirectFile::DataPackage cTmpData(m_cFile.m_cMetaData);
	TargetFields cTmpTarget(m_cFile.m_cMetaData.getVariableFieldNumber());

	// insertに渡すデータ
	// 更新対象になっていない可変長フィールドがあったら
	// それを補完したものを指すように変更される
	const DirectFile::DataPackage* pData = &cNewData_;
	const TargetFields* pTarget = pTarget_;

	// 可変長フィールドのうち更新対象になっていないフィールドがあれば
	// 読み込んでセットする
	if (cOldObjectHeader_.getVariableID() != Tools::m_UndefinedObjectID
		&& pTarget_->getSize() < m_cFile.m_cMetaData.getVariableFieldNumber()) {
		readOldData(cOldObjectHeader_, cNewData_, pTarget_, cTmpData, cTmpTarget);
		pData = &cTmpData;
		pTarget = &cTmpTarget;
	}

	// インサートと同じ処理である
	insert(*pData, iFreeID_, pTarget);
}

//	FUNCTION public
//	Record::VariableIterator::expunge --
//
//	NOTES
//
//	ARGUMENTS
//		Tools::ObjectID iObjectID_
//			破棄するオブジェクトID
//		Tools::ObjectID& iFreeID_
//			空き領域リストの先頭
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VariableIterator::
expunge(Tools::ObjectID iObjectID_, Tools::ObjectID& iFreeID_)
{
	if (iObjectID_ != Tools::m_UndefinedObjectID) {
		// LinkedObjectがなければ用意する
		initializeLinkedObject();

		// リンクオブジェクトをすべて破棄する
#ifdef DEBUG
		SydRecordSizeMessage
			<< "Delete object for " << m_cFile.getPath()
			<< ModEndl;
#endif
		m_pLinkedObject->deleteAll(iObjectID_, iFreeID_);

		// どこも指していない状態にする
		m_iObjectID = Tools::m_UndefinedObjectID;
	}
}

//	FUNCTION public
//	Record::VariableIterator::detachObject --
//		オブジェクトを指す変数をクリアする
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
VariableIterator::
detachObject()
{
	; _SYDNEY_ASSERT(m_pLinkedObject);
	m_pLinkedObject->detachObject();
}

///////////////////////
// 整合性検査用の関数
///////////////////////

//	FUNCTION public
//	Record::VariableIterator::verifyData --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VariableIterator::
verifyData(DirectFile::DataPackage& cData_,
		   Admin::Verification::Treatment::Value iTreatment_,
		   Admin::Verification::Progress& cProgress_)
{
	if (!cProgress_.isGood()) {
		return;
	}
	// サイズの合計とエリアサイズの合計が合っているか

	// LinkedObjectがなければ用意する
	initializeLinkedObject();

	AutoAttachObject obj( *m_pLinkedObject ,cData_.getVariableID() ,cProgress_);
	if (!cProgress_.isGood()) {
		return;
	}

	// ヘッダーを読み込み、ヘッダーサイズとヘッダーに書かれたフィールドのサイズを合計する
	readHeader(cData_);
	Os::Memory::Size iSumSize = 0;
	ModSize n = m_cObjectHeader.m_vecData.getSize();

	for (ModSize i = 0; i < n; ++i) {
		// ヘッダーのサイズを得る
		iSumSize += m_cFile.m_cMetaData.getHeaderSize(m_cObjectHeader.m_vecData[i].first);
		// 各フィールドが占めるサイズを得る
		iSumSize += m_cObjectHeader.m_vecData[i].second.m_iFieldSize;
	}

	// 合計がリンクオブジェクトのエリアサイズから
	// リンクオブジェクトが使っているヘッダーの分を引いた値と一致するはず

	Os::Memory::Size iActualSize = m_pLinkedObject->getTotalSizeVerify(cProgress_);
	if (iActualSize != iSumSize) {
		_SYDNEY_VERIFY_INCONSISTENT(
			cProgress_,
			m_cFile.getPath(),
			Message::InconsistentVariableSize(cData_.getVariableID(),
											  iSumSize, iActualSize));
	}
}

//////////////
// 内部関数 //
//////////////

//	FUNCTION private
//	Record::VariableIterator::initializeLinkedObject --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS

void
VariableIterator::
initializeLinkedObject()
{
	if (!m_pLinkedObject) {
		m_pLinkedObject = new LinkedObject(_AccessModeTable[m_eOperation],
										   *m_cFile.m_pFreeAreaManager);
	}
}

//	FUNCTION private
//	Record::VariableIterator::invalidate --
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
VariableIterator::
invalidate()
{
	if (m_pLinkedObject
		&& m_pLinkedObject->isAttached()) {
		m_pLinkedObject->detachObject();
	}
	m_iObjectID = Tools::m_UndefinedObjectID;
}

//	FUNCTION private
//	Record::VariableIterator::readHeader --
//
//	NOTES
//		m_pLinkedObjectはattachObjectかresetの直後であること
//
//	ARGUMENTS
//		DirectFile::DataPackage& cData_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VariableIterator::
readHeader(const DirectFile::DataPackage& cData_)
{
	; _SYDNEY_ASSERT(m_pLinkedObject);
	; _SYDNEY_ASSERT(m_pLinkedObject->isAttached());

	// まず内容を入れる変数をクリアしておく
	m_cObjectHeader.clear();

#ifdef DEBUG
	Os::Memory::Size iSumSize = 0;
#endif

	// すべての可変長フィールドについて調べる
	TargetIterator cTargetIterator(m_cFile.m_cMetaData.getVariableFields(),
								   &m_cFile.m_cMetaData);
	m_cObjectHeader.m_vecData.reserve(
		m_cFile.m_cMetaData.getVariableFieldNumber());
	do {
		Tools::FieldNum iFieldID = cTargetIterator.getNext();
		; _SYDNEY_ASSERT(iFieldID != 0);
		; _SYDNEY_ASSERT(m_cFile.m_cMetaData.isVariable(iFieldID));

		if (cData_.isNull(iFieldID)) {
			// NULLなので飛ばす

#ifdef DEBUG
			SydRecordDebugMessage
				<< "readHeader: FieldID = " << iFieldID
				<< " skip NULL"
				<< ModEndl;
#endif

			continue;
		}
		m_cObjectHeader.m_vecData.pushBack(ObjectHeader::Element(iFieldID, readObjectSize(iFieldID)));

#ifdef DEBUG
		SydRecordDebugMessage
			<< "readHeader: FieldID = " << iFieldID
			<< " size = " << m_cObjectHeader.m_vecData.getBack().second.m_iFieldSize
			<< ModEndl;
		iSumSize += m_cObjectHeader.m_vecData.getBack().second.m_iFieldSize;
#endif

	} while (cTargetIterator.hasNext());
#ifdef DEBUG
	SydRecordDebugMessage
		<< "readHeader: sum size = " << iSumSize
		<< ModEndl;
#endif
}

//	FUNCTION private
//	Record::VariableIterator::writeHeader --
//
//	NOTES
//
//	ARGUMENTS
//		DirectFile::DataPackage& cData_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VariableIterator::
writeHeader(const DirectFile::DataPackage& cData_,
			const TargetFields* pTarget_)
{
	; _SYDNEY_ASSERT(m_pLinkedObject);
	; _SYDNEY_ASSERT(m_pLinkedObject->isAttached());

#ifdef DEBUG
	Os::Memory::Size iSumSize = 0;
#endif

	// pTarget_には更新後にnullでないすべての可変長フィールドが
	// 含まれているはず
	TargetIterator cTargetIterator(pTarget_, &m_cFile.m_cMetaData);
	ModSize i = 0;
	do {
		Tools::FieldNum iFieldID = cTargetIterator.getNext();
		; _SYDNEY_ASSERT(iFieldID != 0);
		; _SYDNEY_ASSERT(m_cFile.m_cMetaData.isVariable(iFieldID));

		if (cData_.isNull(iFieldID)) {
			// NULLなので飛ばす

#ifdef DEBUG
			SydRecordDebugMessage
				<< "writeHeader: FieldID = " << iFieldID
				<< " skip NULL"
				<< ModEndl;
#endif

			continue;
		}
		; _SYDNEY_ASSERT(m_cObjectHeader.m_vecData[i].first == iFieldID);
		writeObjectSize(m_cObjectHeader.m_vecData[i].second, iFieldID);
		++i;

#ifdef DEBUG
		SydRecordDebugMessage
			<< "writeHeader: FieldID = " << iFieldID
			<< " size = " << m_cObjectHeader.m_vecData[i-1].second.m_iFieldSize
			<< ModEndl;
		iSumSize += m_cObjectHeader.m_vecData[i-1].second.m_iFieldSize;
#endif

	} while (cTargetIterator.hasNext());
#ifdef DEBUG
	SydRecordDebugMessage
		<< "writeHeader: sum size = " << iSumSize
		<< ModEndl;
#endif
}

//	FUNCTION private
//	Record::VariableIterator::makeHeader --
//
//	NOTES
//
//	ARGUMENTS
//		DirectFile::DataPackage& cData_
//			作成のもとになるデータ
//
//	RETURN
//		固定長フィールドすべてを格納するのに必要なサイズ
//
//	EXCEPTIONS

Os::Memory::Size
VariableIterator::
makeHeader(const DirectFile::DataPackage& cData_,
		   const TargetFields* pTarget_)
{
	// すべての可変長フィールドについて調べる
	TargetIterator cTargetIterator(pTarget_, &m_cFile.m_cMetaData);

	// まず内容を入れる変数をクリアしておく
	m_cObjectHeader.clear();
	m_cObjectHeader.m_vecData.reserve(
		m_cFile.m_cMetaData.getVariableFieldNumber());

	Os::Memory::Size iResult = 0;
	do {
		Tools::FieldNum iFieldID = cTargetIterator.getNext();
		; _SYDNEY_ASSERT(iFieldID != 0);
		; _SYDNEY_ASSERT(m_cFile.m_cMetaData.isVariable(iFieldID));

		if (cData_.isNull(iFieldID)) {
			// NULLなので飛ばす

#ifdef DEBUG
			SydRecordDebugMessage
				<< "makeHeader: FieldID = " << iFieldID
				<< " skip NULL"
				<< ModEndl;
#endif

			continue;
		}

		m_cObjectHeader.m_vecData.pushBack(
			ObjectHeader::Element(
				iFieldID,
				makeObjectSize(
					*cData_.get()->getElement(cTargetIterator.getIndex()),
					iFieldID)));

		// データ部分に使うサイズを総サイズに加える
		iResult += m_cObjectHeader.m_vecData.getBack().second.m_iFieldSize;

		// ヘッダーに使うサイズを加える
		iResult += m_cFile.m_cMetaData.getHeaderSize(iFieldID);

#ifdef DEBUG
		SydRecordDebugMessage
			<< "makeHeader: FieldID = " << iFieldID
			<< " size = " << m_cObjectHeader.m_vecData.getBack().second.m_iFieldSize
			<< ModEndl;
#endif

	} while (cTargetIterator.hasNext());

	return iResult;
}

//	FUNCTION private
//	Record::VariableIterator::readObjectSize --
//
//	NOTES
//
//	ARGUMENTS
//		Record::Tools::FieldNum iFieldID_
//			フィールドの位置
//
//	RETURN
//		ObjectSize
//
//	EXCEPTIONS

VariableIterator::ObjectSize
VariableIterator::
readObjectSize(Tools::FieldNum iFieldID_)
{
	; _SYDNEY_ASSERT(m_cFile.m_cMetaData.isVariable(iFieldID_));
	; _SYDNEY_ASSERT(m_pLinkedObject);

	ObjectSize cResult;

	if (!m_cFile.m_cMetaData.isArray(iFieldID_)) {
		// 配列でない場合は圧縮前後のサイズが入っている

		// 高速化のためじか読みする
		m_pLinkedObject->read(&cResult, Tools::m_FieldLengthArchiveSize * 2);
	} else {
		// 配列の場合はまず要素数が入っている
		Tools::ElementNum iNum;
		m_pLinkedObject->read(&iNum, Tools::m_ElementNumArchiveSize);
		cResult.m_iElementNumber = iNum;

		if (m_cFile.m_cMetaData.isVariableArray(iFieldID_)) {
			// 可変長要素の場合はその後に総サイズが入っている
			Tools::FieldLength iLength;
			m_pLinkedObject->read(&iLength, Tools::m_FieldLengthArchiveSize);
			cResult.m_iFieldSize = iLength;

		} else {
			// 固定長要素の場合はその型からサイズが計算できる
			// null bitmap + 要素数 * 型のサイズ
			Tools::DataType elementType = m_cFile.m_cMetaData.getElementType(iFieldID_);
			if (elementType._name != Common::DataType::Decimal) 
			{
				cResult.m_iFieldSize =
					Tools::getBitmapSize(iNum)
					+ (iNum	 * FileCommon::DataManager::getFixedCommonDataArchiveSize(elementType._name));
			}
			else
			{
				cResult.m_iFieldSize =
					Tools::getBitmapSize(iNum)
					+ (iNum * Common::DecimalData::getDumpSizeBy(elementType._length, elementType._scale));
			}
		}
	}
	return cResult;
}

//	FUNCTION private
//	Record::VariableIterator::writeObjectSize --
//
//	NOTES
//
//	ARGUMENTS
//		const Record::VariableIterator::ObjectSize& cSize_
//		Record::Tools::FieldNum iFieldID_
//			フィールドの位置
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VariableIterator::
writeObjectSize(const ObjectSize& cSize_, Tools::FieldNum iFieldID_)
{
	; _SYDNEY_ASSERT(m_cFile.m_cMetaData.isVariable(iFieldID_));
	; _SYDNEY_ASSERT(m_pLinkedObject);

	if (!m_cFile.m_cMetaData.isArray(iFieldID_)) {
		// 配列でない場合は圧縮前後のサイズを入れる
		Tools::FieldLength iLength = cSize_.m_iUncompressedSize;
		m_pLinkedObject->write(&iLength, Tools::m_FieldLengthArchiveSize);
		iLength = cSize_.m_iFieldSize;
		m_pLinkedObject->write(&iLength, Tools::m_FieldLengthArchiveSize);

	} else {
		// 配列の場合はまず要素数を入れる
		Tools::ElementNum iNum = cSize_.m_iElementNumber;
		m_pLinkedObject->write(&iNum, Tools::m_ElementNumArchiveSize);

		if (m_cFile.m_cMetaData.isVariableArray(iFieldID_)) {
			// 可変長要素の場合はその後に総サイズを入れる
			Tools::FieldLength iLength = cSize_.m_iFieldSize;
			m_pLinkedObject->write(&iLength, Tools::m_FieldLengthArchiveSize);
		}
	}
}

//	FUNCTION private
//	Record::VariableIterator::makeObjectSize --
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Data& cData_
//		Record::Tools::FieldNum iFieldID_
//			フィールドの位置
//
//	RETURN
//		なし
//
//	EXCEPTIONS

VariableIterator::ObjectSize
VariableIterator::
makeObjectSize(const Common::Data& cData_,
			   Tools::FieldNum iFieldID_)
{
	; _SYDNEY_ASSERT(m_cFile.m_cMetaData.isVariable(iFieldID_));

	ObjectSize cResult;

	if (!m_cFile.m_cMetaData.isArray(iFieldID_)) {
		// 配列でない場合
		; _SYDNEY_ASSERT(cData_.getType() != Common::DataType::Array);

		// 圧縮前後のサイズを得る
		Tools::FieldLength iUncompressedSize;
		Tools::FieldLength iFieldSize;
		Tools::getVariableSize(cData_,
							   m_cFile.m_cMetaData.getDataType(iFieldID_),
							   iUncompressedSize,
							   iFieldSize);
		cResult.m_iUncompressedSize = iUncompressedSize;
		cResult.m_iFieldSize = iFieldSize;

	} else {
		// 配列の場合
		; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::Array);
		const Common::ArrayData* pArray =
			_SYDNEY_DYNAMIC_CAST(const Common::ArrayData*, &cData_);
		; _SYDNEY_ASSERT(pArray);
		cResult.m_iElementNumber = pArray->getCount();

		if (!m_cFile.m_cMetaData.isVariableArray(iFieldID_)) {
			// 固定長要素の場合はその型からサイズが計算できる
			// null bitmap + 要素数 * 型のサイズ

			Tools::DataType elementType = m_cFile.m_cMetaData.getElementType(iFieldID_);
			if (elementType._name != Common::DataType::Decimal) 
			{
				cResult.m_iFieldSize =
					Tools::getBitmapSize(cResult.m_iElementNumber)
					+ (cResult.m_iElementNumber
						* FileCommon::DataManager::getFixedCommonDataArchiveSize(elementType._name));
			}
			else
			{
				cResult.m_iFieldSize =
					Tools::getBitmapSize(cResult.m_iElementNumber)
					+ (cResult.m_iElementNumber
						* Common::DecimalData::getDumpSizeBy(elementType._length, elementType._scale));
			}
		} else {
			// 可変長要素の場合は実際に要素を調べる
			// 配列に対応するデータは常にDataArrayDataである
			; _SYDNEY_ASSERT(pArray->getElementType() == Common::DataType::Data);
			const Tools::DataType& cElementType
				= m_cFile.m_cMetaData.getElementType(iFieldID_);
			const Common::DataArrayData* pDataArray =
				_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, &cData_);
			; _SYDNEY_ASSERT(pDataArray);

			// Nullでない各データのダンプサイズを合計する
			Os::Memory::Size iSize = 0;
			for (int i = 0; i < cResult.m_iElementNumber; ++i) {
				const Common::Data* pData = pDataArray->getElement(i).get();
				if (pData && !pData->isNull()) {
					; _SYDNEY_ASSERT(pData->isAbleToDump());

					if (pData->getType() == Common::DataType::String)
					{
						const Common::StringData* pStringData
							= _SYDNEY_DYNAMIC_CAST(const Common::StringData*,
												   pData);
						iSize += pStringData->getDumpSize(
							cElementType._encodingForm);
					}
					else
					{
						iSize += pData->getDumpSize();
					}

					// 可変長要素の場合NULLでない各要素ごとに
					// 圧縮前後のサイズが書き込まれるのでその分を加える
					iSize += Tools::m_FieldLengthArchiveSize * 2;
				}
			}
			// null bitmapのサイズを加える
			iSize += Tools::getBitmapSize(cResult.m_iElementNumber);

			cResult.m_iFieldSize = iSize;
		}
	}
	return cResult;
}

//	FUNCTION private
//	Record::VariableIterator::isNecessaryToInsert --
//
//	NOTES
//
//	ARGUMENTS
//		const Record::DirectFile::DataPackage& cData_
//			挿入するデータ
//
//	RETURN
//		true...挿入の必要がある
//
//	EXCEPTIONS

bool
VariableIterator::
isNecessaryToInsert(const DirectFile::DataPackage& cData_)
{
	TargetIterator cTargetIterator(m_cFile.m_cMetaData.getVariableFields(),
								   &m_cFile.m_cMetaData);
	do {
		Tools::FieldNum iFieldID = cTargetIterator.getNext();
		if (iFieldID != 0 && m_cFile.m_cMetaData.isVariable(iFieldID)) {
			if (!cData_.isNull(iFieldID)) {
				// NULLでないデータがある
				return true;
			}
		}
	} while (cTargetIterator.hasNext());

	// NULLでないデータがなかったので挿入する必要なし
	return false;
}

//	FUNCTION private
//	Record::VariableIterator::readOldData --
//		更新前のデータを必要なら読み込む
//
//	NOTES
//
//	ARGUMENTS
//		const DirectFile::DataPackage& cOldObjectHeader_
//			更新前のnull bitmapと可変長ID
//		const DirectFile::DataPackage& cNewData_
//			更新後のデータ
//		const TargetFields* pTarget_
//			更新で指定されているターゲット
//		DirectFile::DataPackage& cData_
//			[OUT]NewDataで抜けている更新前のデータも補完したデータ
//		TargetFields& cTarget_
//			[OUT]pTarget_で抜けている可変長フィールドも補完したターゲット
//
//	RETURN
//
//	EXCEPTIONS

void
VariableIterator::
readOldData(const DirectFile::DataPackage& cOldObjectHeader_,
			const DirectFile::DataPackage& cNewData_,
			const TargetFields* pTarget_,
			DirectFile::DataPackage& cData_,
			TargetFields& cTarget_)
{
	// データを用意する
	cData_.allocate();
	cData_.get()->reserve(m_cFile.m_cMetaData.getVariableFieldNumber());
	// null bitmapはFile.cppで適切なものに変更されているので更新後のものを使えばよい
	cData_.setNullBitMap(cNewData_.getNullBitMap());

	// LinkedObjectがなければ用意する
	initializeLinkedObject();

	// 読み込むオブジェクトをアタッチしてヘッダーを読み込んでおく
	AutoAttachObject obj( *m_pLinkedObject ,cOldObjectHeader_.getVariableID() );
	readHeader(cOldObjectHeader_);

	// フィールド値を得るためのイテレーター
	VariableField cField(m_cFile.m_cMetaData, m_cObjectHeader, *m_pLinkedObject);

	TargetIterator cAllFieldIterator(m_cFile.m_cMetaData.getVariableFields(),
									 &m_cFile.m_cMetaData);
	TargetIterator cTargetIterator(pTarget_, &m_cFile.m_cMetaData);

	// すべてのフィールドのうちTargetIteratorに載っていないものを探す
	// 載っているものはデータをコピーしておく
	; _SYDNEY_ASSERT(cTargetIterator.hasNext());
	Tools::FieldNum iTargetFieldID = cTargetIterator.getNext();

	do {
		; _SYDNEY_ASSERT(cAllFieldIterator.hasNext());
		Tools::FieldNum iFieldID = cAllFieldIterator.getNext();
		; _SYDNEY_ASSERT(m_cFile.m_cMetaData.isVariable(iFieldID));

		if (iFieldID == iTargetFieldID) {
			// 更新後にあるのでデータをコピーする
			const Common::DataArrayData::Pointer& cElement = 
				cNewData_.get()->getElement(cTargetIterator.getIndex());
			cData_.get()->pushBack(cElement);
			if (cElement->isNull())
				cData_.getNullBitMap().set(iFieldID);
			else
				cData_.getNullBitMap().reset(iFieldID);
			cTarget_.addFieldNumber(iFieldID);
#ifdef DEBUG
			SydRecordDebugMessage
				<< "Copy field for update: "
				<< iFieldID
				<< " data: "
				<< cData_.get()->getValue().getBack()->getString()
				<< ModEndl;
#endif

			if (cTargetIterator.hasNext()) {
				// ターゲットのほうもひとつ進める
				iTargetFieldID = cTargetIterator.getNext();
			} else {
				// 最後なのですべてのフィールドより大きい値にしておく
				iTargetFieldID = m_cFile.m_cMetaData.getFieldNumber();
			}
			continue;
		}
		; _SYDNEY_ASSERT(iFieldID < iTargetFieldID);

		// Nullなら何もしない
		if (cOldObjectHeader_.isNull(iFieldID)) {
			continue;
		}

		// 読み込んでコピーする
		bool bResult = cField.seek(iFieldID);
		_SYDNEY_ASSERT(bResult);
		cData_.get()->pushBack(cField.readField());
		cTarget_.addFieldNumber(iFieldID);

#ifdef DEBUG
		SydRecordDebugMessage
			<< "Read old field for update: "
			<< iFieldID
			<< " data: "
			<< cData_.get()->getValue().getBack()->getString()
			<< ModEndl;
#endif

	} while (cAllFieldIterator.hasNext());
}

//
//	Copyright (c) 2001, 2003, 2004, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
