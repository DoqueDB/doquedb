// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DirectIterator.cpp -- 代表オブジェクトを格納するファイルを走査するイテレーター
// 
// Copyright (c) 2001, 2003, 2004, 2005, 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
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
#include "Record/DirectIterator.h"
#include "Record/DirectField.h"
#include "Record/DirectFile.h"
#include "Record/MetaData.h"
#include "Record/OpenParameter.h"
#include "Record/PhysicalPosition.h"
#include "Record/TargetFields.h"
#include "Record/Tools.h"
#include "Record/Message_BadFreeObjectID.h"
#include "Record/Message_InconsistentPageObjectNumber.h"
#include "Record/Message_ObjectNotFound.h"
#include "Record/Debug.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/NullData.h"
#include "Buffer/Page.h"
#include "Exception/Unexpected.h"
#include "LogicalFile/ObjectID.h"
#include "PhysicalFile/Page.h"

_SYDNEY_USING
_SYDNEY_RECORD_USING

namespace
{
	// DirectFileの置き換え優先度
	const Buffer::ReplacementPriority::Value
		_ePriority = Buffer::ReplacementPriority::Low;

	// Operation::ValueからFixModeを得るための表
	const Buffer::Page::FixMode::Value
		_FixModeTable[DirectIterator::Operation::ValueNum] =
	{
		// Read
		Buffer::Page::FixMode::ReadOnly,
		// Write
		Buffer::Page::FixMode::Write | Buffer::Page::FixMode::Discardable,
		// Insert
		Buffer::Page::FixMode::Write | Buffer::Page::FixMode::Discardable,
		// Expunge
		Buffer::Page::FixMode::Write | Buffer::Page::FixMode::Discardable,
		// Verify
		Buffer::Page::FixMode::ReadOnly,
		// Batch
		Buffer::Page::FixMode::Write
	};
	// Operation::ValueからUnfixModeを得るための表
	const PhysicalFile::Page::UnfixMode::Value
		_UnfixModeTable[DirectIterator::Operation::ValueNum] =
	{
		// Read
		PhysicalFile::Page::UnfixMode::NotDirty,
		// Write
		PhysicalFile::Page::UnfixMode::Dirty,
		// Insert
		PhysicalFile::Page::UnfixMode::Dirty,
		// Expunge
		PhysicalFile::Page::UnfixMode::Dirty,
		// Verify
		PhysicalFile::Page::UnfixMode::NotDirty,
		// Batch
		PhysicalFile::Page::UnfixMode::Dirty
	};

	namespace _SearchObjectID
	{
		// CheckObjectNumberの実体を定義する
		// ひとつでもあればOK
		bool _CheckAny(DirectIterator::PageHeader& cHeader_)
		{
			return cHeader_.m_iObjectNumber > 0;
		}
		// すべてが埋まっていなければOK
		bool _CheckNotAll(DirectIterator::PageHeader& cHeader_)
		{
			return cHeader_.m_iObjectNumber != cHeader_.m_iObjectPerPage;
		}

		// CheckBitmapの実体を定義する
		// trueならOK
		bool _CheckExist(DirectIterator::PageHeader& cHeader_, DirectIterator::AreaID i_)
		{
			return cHeader_.isExist(i_);
		}
		// falseならOK
		bool _CheckNotExist(DirectIterator::PageHeader& cHeader_, DirectIterator::AreaID i_)
		{
			return !cHeader_.isExist(i_);
		}
	} // namespace _SearchObjectID

} // namespace

//	FUNCTION public
//	Record::DirectIterator::DirectIterator --
//
//	NOTES
//
//	ARGUMENTS
//		Record::DirectFile& cFile_
//			イテレーターで走査するファイル
//		Tools::ObjectID iFirstID_ = Tools::m_UndefinedObjectID
//		Tools::ObjectID iLastID_ = Tools::m_UndefinedObjectID
//			読み込み時に最初と最後のIDを用いる
//
//	RETURN
//		なし
//
//	EXCEPTIONS

DirectIterator::
DirectIterator(DirectFile& cFile_)
	: m_cFile(cFile_),
	  m_pPage(0),
	  m_iObjectID(0),
	  m_iPageID(PhysicalFile::ConstValue::UndefinedPageID),
	  m_iAreaID(PhysicalFile::ConstValue::UndefinedAreaID),
	  m_eDirection(Direction::Forward),
	  m_iStartObjectID(Tools::m_UndefinedObjectID),
	  m_iEndObjectID(Tools::m_UndefinedObjectID),
	  m_cPageList(&Page::_prev, &Page::_next),
	  m_pBatchCachePage(0),
	  m_bFetch(false),
	  m_cAscendingIterator(cFile_.m_cMetaData.getObjectPerPage()),
	  m_cDescendingIterator(cFile_.m_cMetaData.getObjectPerPage() - 1)
{
	// ファイルはアタッチしていなければならない
	; _SYDNEY_ASSERT(cFile_.m_pFile);
}

DirectIterator::
DirectIterator(DirectFile& cFile_,
			   Tools::ObjectID iFirstID_,
			   Tools::ObjectID iLastID_)
	: m_cFile(cFile_),
	  m_pPage(0),
	  m_iObjectID(0),
	  m_iPageID(PhysicalFile::ConstValue::UndefinedPageID),
	  m_iAreaID(PhysicalFile::ConstValue::UndefinedAreaID),
	  m_cPageList(&Page::_prev, &Page::_next),
	  m_pBatchCachePage(0),
	  m_bFetch(false),
	  m_cAscendingIterator(cFile_.m_cMetaData.getObjectPerPage()),
	  m_cDescendingIterator(cFile_.m_cMetaData.getObjectPerPage() - 1)
{
	// オープンオプションつきでアタッチしていなければならない
	; _SYDNEY_ASSERT(cFile_.m_pFile);
	; _SYDNEY_ASSERT(cFile_.m_pOpenParam);

	if (cFile_.m_pOpenParam->m_iOpenMode == FileCommon::OpenMode::Read) {
#ifdef OBSOLETE
		// スタート地点のIDを記録する
		if (m_cFile.m_pOpenParam->m_bSortOrder) {
			// 降順
			m_iStartObjectID = iLastID_;
			m_iEndObjectID = iFirstID_;
			m_eDirection = Direction::Backward;

		} else {
#endif
			// 昇順
			m_iStartObjectID = iFirstID_;
			m_iEndObjectID = iLastID_;
			m_eDirection = Direction::Forward;
#ifdef OBSOLETE
		}
#endif
	}
}

//	FUNCTION public
//	Record::DirectIterator::~DirectIterator --
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

DirectIterator::
~DirectIterator()
{
	detachPage(Operation::Read);
}

//	FUNCTION public
//	Record::DirectIterator::seek --
//
//	NOTES
//
//	ARGUMENTS
//		Record::Tools::ObjectID iObjectID_
//			移動するオブジェクトID
//		bool bKeepAttach_ = false
//			ページをアタッチしたままにするか
//		Operation::Value eValue_ = Operation::Read
//			bKeepAttach_がtrueのとき、アタッチするモード
//
//	RETURN
//		指定されたオブジェクトIDが正しいものであればtrue
//
//	EXCEPTIONS

bool
DirectIterator::
seek(Tools::ObjectID iObjectID_, bool bKeepAttach_, Operation::Value eValue_)
{
	if (iObjectID_ >= Tools::m_UndefinedObjectID) {
		return false;
	}

	m_bFetch = true;

	PhysicalPosition pos(iObjectID_);
	; _SYDNEY_ASSERT(m_cFile.m_pFile->isUsedPage(m_cFile.m_cTrans, pos.m_PageID));

	m_iPageID = pos.m_PageID;
	m_iAreaID = pos.m_AreaID;
	attachPage(eValue_);
	try {
		readHeader();
		if (m_cPageHeader.isExist(pos.m_AreaID)) {
			if (!bKeepAttach_) detachPage(Operation::Read); // 変更されてない
			return true;
		}

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		detachPage(Operation::Read);
		_SYDNEY_RETHROW;
	}
	detachPage(Operation::Read);
	return false;
}

//	FUNCTION public
//	Record::DirectIterator::next --
//
//	NOTES
//		次の使用中ページを指すようにする
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		次の使用中ページを指すことができればtrue
//
//	EXCEPTIONS

bool
DirectIterator::
next()
{
	bool bMove;//次のオブジェクトに移るか？

	//ObjectIDの初期値
	if (m_iObjectID == 0) {
		// seekせずにnextが呼べるのはStartObjectIDを指定して
		// コンストラクトしたときのみ
		; _SYDNEY_ASSERT(m_iStartObjectID < Tools::m_UndefinedObjectID);
		m_iObjectID = m_iStartObjectID;

		//※bug：同時実行の際、検索開始の直前に挿入されることが起きうるので
		//m_iObjectID の初期値を、コンストラクタ時に決定されるm_iStargObjectIDとするのは間違い。
		//その時点でのヘッダページの内容を（排他区間内で）読み込むべき。

		PhysicalPosition pos(m_iObjectID);
		m_iPageID = pos.m_PageID;
		m_iAreaID = pos.m_AreaID;
		bMove = false;//移らない。削除済みの検査が必要
	} else if (m_iObjectID == Tools::m_UndefinedObjectID) {
		// 一度最後まで行ってしまったら何度やっても同じ
		return false;
	} else {
		//ObjectIDの初期値があるばあい
		bMove = true;//移る。
	}

	// nextはアタッチした状態で返す
	attachPage(Operation::Read);
	readHeader();
	//次のオブジェクトに移るか
	if (bMove || ! m_cPageHeader.isExist(m_iAreaID)) {
		// 見つけたらアタッチしたページを置き換えるモードで
		// オブジェクトIDを探す
#ifdef OBSOLETE
		switch (m_eDirection) {
		case Direction::Forward:
#endif
			m_iObjectID = getNextObjectID(SearchPage::Replace);
#ifdef OBSOLETE
			break;
		case Direction::Backward:
			m_iObjectID = getPrevObjectID(SearchPage::Replace);
			break;
		default:
			_SYDNEY_ASSERT(0);
		}
#endif
	}

	if (isValid()) {
		return true;
	} else {
		// seekに失敗したらアタッチしていない状態にする
		detachPage(Operation::Read);
		return false;
	}
}

#ifdef OBSOLETE
//	FUNCTION public
//	Record::DirectIterator::reset --
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		常にtrue
//
//	EXCEPTIONS

bool
DirectIterator::
reset()
{
	invalidate();
	return true;
}
#endif //OBSOLETE

//	FUNCTION public
//	Record::DirectIterator::invalidate --
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
DirectIterator::
invalidate()
{
	m_iObjectID = 0;
	m_iPageID = PhysicalFile::ConstValue::UndefinedPageID;
	m_iAreaID = PhysicalFile::ConstValue::UndefinedAreaID;
}

//	FUNCTION public
//	Record::DirectIterator::isValid --
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
DirectIterator::
isValid() const
{
	return (m_iObjectID > 0 && m_iObjectID < Tools::m_UndefinedObjectID);
}

// StartIDを上書きする
void
DirectIterator::
setStartObjectID(const Tools::ObjectID& iFirstID_)
{
	m_iStartObjectID = iFirstID_;
}

// LastIDを上書きする
void
DirectIterator::
setEndObjectID(const Tools::ObjectID& iLastID_)
{
	m_iEndObjectID = iLastID_;
}

#ifdef OBSOLETE
//	FUNCTION public
//	Record::DirectIterator::getPageID --
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		イテレーターが現在指しているページID
//
//	EXCEPTIONS

PhysicalFile::PageID
DirectIterator::
getPageID() const
{
	return m_iPageID;
}
#endif //OBSOLETE

//	FUNCTION public
//	Record::DirectIterator::getObjectID --
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
DirectIterator::
getObjectID() const
{
	return PhysicalPosition::getObjectID(m_iPageID, m_iAreaID);
}

//	FUNCTION public
//	Record::DirectIterator::getNextObjectID --
//		イテレーターが指している位置の1つ後の使用中オブジェクトIDを得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//		要求されたオブジェクトID
//
//	EXCEPTIONS

Tools::ObjectID
DirectIterator::
getNextObjectID(SearchPage::Value eSearchPage_)
{
	m_cAscendingIterator.setCurrent(m_iAreaID + 1);

	return searchObjectID(m_cAscendingIterator,
						  &_SearchObjectID::_CheckExist,
						  &_SearchObjectID::_CheckAny,
						  &DirectIterator::getNextPageID,
						  eSearchPage_);
}

//	FUNCTION public
//	Record::DirectIterator::getPrevObjectID --
//		イテレーターが指している位置の1つ前の使用中オブジェクトIDを得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//		要求されたオブジェクトID
//
//	EXCEPTIONS

Tools::ObjectID
DirectIterator::
getPrevObjectID(SearchPage::Value eSearchPage_)
{
	m_cDescendingIterator.setCurrent(m_iAreaID - 1);

	return searchObjectID(m_cDescendingIterator,
						  &_SearchObjectID::_CheckExist,
						  &_SearchObjectID::_CheckAny,
						  &DirectIterator::getPrevPageID,
						  eSearchPage_);
}

//	FUNCTION public
//	Record::DirectIterator::getNextFreeObjectID --
//		イテレーターが指している位置の1つ後の未使用オブジェクトIDを得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//		要求されたオブジェクトID
//
//	EXCEPTIONS

Tools::ObjectID
DirectIterator::
getNextFreeObjectID(SearchPage::Value eSearchPage_)
{
	m_cAscendingIterator.setCurrent(m_iAreaID + 1);

	return searchObjectID(m_cAscendingIterator,
						  &_SearchObjectID::_CheckNotExist,
						  &_SearchObjectID::_CheckNotAll,
						  &DirectIterator::getNextPageID,
						  eSearchPage_);
}

//	FUNCTION public
//	Record::DirectIterator::getNextObjectIDVerify --
//		イテレーターが指している位置の1つ後の使用中オブジェクトIDを得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//		要求されたオブジェクトID
//
//	EXCEPTIONS

Tools::ObjectID
DirectIterator::
getNextObjectIDVerify(Admin::Verification::Progress* pProgress_)
{
	_SYDNEY_ASSERT(pProgress_);

	m_cAscendingIterator.setCurrent(m_iAreaID + 1);

	Admin::Verification::Progress cTmp(pProgress_->getConnection());
	Tools::ObjectID iResult = searchObjectID(m_cAscendingIterator,
											 &_SearchObjectID::_CheckExist,
											 &_SearchObjectID::_CheckAny,
											 &DirectIterator::getNextPageID,
											 DirectIterator::SearchPage::Verify,
											 &cTmp);
	*pProgress_ += cTmp;
	return iResult;
}

//	FUNCTION public
//	Record::DirectIterator::read --
//
//	NOTES
//
//	ARGUMENTS
//		const TargetFields* pTarget_
//			取得するフィールド
//			0のときは全フィールドを取得する
//		Record::DirectFile::DataPackage& cData_
//			固定長レコードの情報を格納する変数
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
DirectIterator::
read(const TargetFields* pTarget_,
	 DirectFile::DataPackage& cData_)
{
	; _SYDNEY_ASSERT(cData_.get());

	// readが呼ばれる前にページはアタッチされている
	// ヘッダーも読み込まれている
	; _SYDNEY_ASSERT(m_pPage);
	; _SYDNEY_ASSERT(m_pConstPointer);
	;_SYDNEY_ASSERT(m_cPageHeader.isExist(m_iAreaID));

	// ポインターをオブジェクトの先頭を指す位置に移動させる
	m_pConstPointer += m_iAreaID * m_cFile.m_cMetaData.getObjectSize();

	// 以下でオブジェクトの内容を読み込む
	const char* pPointer = m_pConstPointer;

	// オブジェクトのヘッダー情報を読み込む
	pPointer = readObjectHeader(pPointer, cData_);

	// フィールド値を得るためのイテレーター
	DirectField cField(m_cFile.m_cMetaData, pPointer);

	// 所得すべきフィールドIDを得ながら固定長の値をセットしていく
	TargetIterator cTargetIterator(pTarget_, &m_cFile.m_cMetaData);

	do {
		Tools::FieldNum iFieldID = cTargetIterator.getNext();

		if (iFieldID == 0) {
			// 0番はオブジェクトIDなのでそのままセットする
			Common::Data::Pointer p
				= cData_.get()->getElement(cTargetIterator.getIndex());
			; _SYDNEY_ASSERT(p->getType() == Common::DataType::ObjectID);
			_SYDNEY_DYNAMIC_CAST(Common::ObjectIDData*, p.get())
				->setValue(getObjectID());

#ifdef DEBUG
		SydRecordDebugMessage
			<< "Read fixed field: "
			<< iFieldID
			<< " data: "
			<< cData_.get()->getElement(cTargetIterator.getIndex())->getString()
			<< ModEndl;
#endif

			continue;
		}
		if (cData_.isNull(iFieldID)) {
			// Nullデータなのでそのままセットする
			cData_.get()->getElement(cTargetIterator.getIndex())->setNull();

#ifdef DEBUG
		SydRecordDebugMessage
			<< "Read fixed field: "
			<< iFieldID
			<< " data: "
			<< cData_.get()->getElement(cTargetIterator.getIndex())->getString()
			<< ModEndl;
#endif

			continue;
		}
		// 可変長のNULLデータを埋めるために
		// File::divideTargetsにより分離されたものではなく
		// 必要なすべてのフィールドが入ったターゲットが渡されている
		// のでここで可変長フィールドを飛ばす
		if (m_cFile.m_cMetaData.isVariable(iFieldID)) {
			continue;
		}
		// それ以外のデータはページから読み込む
		cField.seek(iFieldID);
		Common::Data::Pointer pData
			= cData_.get()->getElement(cTargetIterator.getIndex());
		cField.readField(*pData);

#ifdef DEBUG
		SydRecordDebugMessage
			<< "Read fixed field: "
			<< iFieldID
			<< " data: "
			<< cData_.get()->getElement(cTargetIterator.getIndex())->getString()
			<< ModEndl;
#endif

	} while (cTargetIterator.hasNext());
}

//	FUNCTION public
//	Record::DirectIterator::readObjectInfo --
//
//	NOTES
//
//	ARGUMENTS
//		DirectFile::DataPackage& cData_
//			結果を格納する
//
//	RETURN
//		イテレーターの位置のnull bitmapと可変長オブジェクトID
//
//	EXCEPTIONS

void
DirectIterator::
readObjectHeader(DirectFile::DataPackage& cData_)
{
	; _SYDNEY_ASSERT(m_pPage);

	// ポインターをオブジェクトの先頭を指す位置に移動させる
	skipHeader();
	m_pConstPointer += m_iAreaID * m_cFile.m_cMetaData.getObjectSize();

	(void) readObjectHeader(m_pConstPointer, cData_);
}

//	FUNCTION public
//	Record::DirectIterator::readVariableID --
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		可変長オブジェクトID
//
//	EXCEPTIONS

Tools::ObjectID
DirectIterator::
readVariableID()

{
	; _SYDNEY_ASSERT(m_pPage);

	// ポインターをオブジェクトの先頭を指す位置に移動させる
	skipHeader();
	m_pConstPointer += m_iAreaID * m_cFile.m_cMetaData.getObjectSize();

	// 以下でオブジェクトの内容を読み込む
	const char* pPointer = m_pConstPointer;
	// オブジェクトの先頭からNull Bitmapが格納されている
	pPointer += Tools::getBitmapSize(m_cFile.m_cMetaData.getFieldNumber() - 1);

	// 可変長がある場合はNull Bitmapの次に
	// 可変長フィールドのオブジェクトIDが入っている
	Tools::ObjectID iVariableID;
	pPointer += Tools::readObjectID(pPointer, iVariableID);

	return iVariableID;
}

//	FUNCTION public
//	Record::DirectIterator::insert --
//
//	NOTES
//		挿入が成功すると挿入したページを指す
//
//	ARGUMENTS
//		DirectFile::DataPackage& cData_
//			挿入するデータ
//		Tools::ObjectID iObjectID_ = Tools::m_UndefinedObjectID
//			Undefinedでないとき指定されたオブジェクトIDの位置を使う
//		bool bUseFreeObjectID_ = false
//			削除オブジェクトIDリストのエントリーを使っている
//
//	RETURN
//		bUseFreeObjectID_=trueのとき、空きオブジェクト領域に記録されていた
//		次の空きオブジェクト領域のID
//
//	EXCEPTIONS

Tools::ObjectID
DirectIterator::
insert(DirectFile::DataPackage& cData_, Tools::ObjectID iObjectID_,
	   bool bUseFreeObjectID_)
{
	m_iObjectID = iObjectID_;
	if (m_iObjectID != Tools::m_UndefinedObjectID) {
		PhysicalPosition pos(iObjectID_);
		m_iPageID = pos.m_PageID;
		m_iAreaID = pos.m_AreaID;

	} else {
		m_iPageID = PhysicalFile::ConstValue::UndefinedPageID;
		m_iAreaID = PhysicalFile::ConstValue::UndefinedAreaID;
	}

	// 書き込む前の情報をsaveしておく
	Tools::ObjectID iExpungedID = Tools::m_UndefinedObjectID;
	bool bHeaderRead = false;

	Operation::Value eOperation = m_cFile.isBatch() ? Operation::Batch : Operation::Insert;
	attachPage(eOperation);

#ifdef DEBUG
	SydRecordDebugMessage
		<< "DirectIterator insert: " << ModHex << getObjectID()
		<< " using FreeObjectID: " << ModHex << iObjectID_
		<< ModDec << ModEndl;
#endif

	// ページヘッダーを読み込む
	readHeader();
	bHeaderRead = true;
	if (m_cPageHeader.isExist(m_iAreaID)) {
		SydErrorMessage
			<< "Used areaID is requested to be inserted.(" << iObjectID_ << ")";
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	// ヘッダー情報を書き換えて書き込む
	m_cPageHeader.setInsert(m_iAreaID);
	writeHeader();

	// ポインターをオブジェクトを書き込む先頭の位置に移動させる
	m_pPointer += m_iAreaID * m_cFile.m_cMetaData.getObjectSize();

	// 以下でオブジェクトの内容を書き込む
	char* pPointer = m_pPointer;

	// 削除オブジェクトIDリストから得ている場合は
	// 現在の領域に次の削除オブジェクトIDが書いてあるので
	// 取得する
	if (bUseFreeObjectID_) {
		(void) Tools::readObjectID(pPointer, iExpungedID);
	}

	// オブジェクトの先頭からNull Bitmapを格納する
	pPointer = writeObjectHeader(pPointer, cData_);

	// フィールド値を得るためのイテレーター
	DirectField cField(m_cFile.m_cMetaData, pPointer);

	// 書き込むべきフィールドIDを得ながら固定長の値をセットしていく
	TargetIterator cTargetIterator(m_cFile.m_cMetaData.getDirectFields(),
								   &m_cFile.m_cMetaData);

	do {
		Tools::FieldNum iFieldID = cTargetIterator.getNext();
		; _SYDNEY_ASSERT(!m_cFile.m_cMetaData.isVariable(iFieldID));

		if (iFieldID == 0) {
			// 0番はオブジェクトIDなので書き込まない
			continue;
		}
		if (cData_.isNull(iFieldID)) {
			// Nullデータは書き込まない
			continue;
		}
		// それ以外のデータは書き込む
		cField.seek(iFieldID);
		cField.updateField(
			cData_.get()->getElement(cTargetIterator.getIndex()).get());

#ifdef DEBUG
		SydRecordDebugMessage
			<< "Insert fixed field: "
			<< iFieldID
			<< " data: "
			<< cData_.get()->getElement(cTargetIterator.getIndex())->getString()
			<< ModEndl;
#endif

	} while (cTargetIterator.hasNext());

	// detachPageAllを使う場合は物理ページ記述子へのポインターを0にするだけ
	detachPage(eOperation);

	return iExpungedID;
}

//	FUNCTION public
//	Record::DirectIterator::update --
//
//	NOTES
//
//	ARGUMENTS
//		DirectFile::DataPackage& cData_
//			更新後のデータ
//		const TargetFields* pTarget_
//			更新するフィールド
//			0ならObjectIDを除くすべてのフィールドが対象
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
DirectIterator::
update(DirectFile::DataPackage& cData_,
	   const TargetFields* pTarget_)
{
	; _SYDNEY_ASSERT(cData_.get());
	; _SYDNEY_ASSERT(pTarget_);

	// 取り消しはdetachPageで行うので
	// ここでのエラー処理は不要
	// insertやexpungeも同様であるが
	// updateはインターフェースの変更があるので先行して修正した
	// ただしdetachPageはそのまま残っているので後で除去する必要がある

	attachPage(Operation::Write);

	// ページヘッダーを読み飛ばす
	skipHeader();

	// ポインターをオブジェクトを書き込む先頭の位置に移動させる
	m_pPointer += m_iAreaID * m_cFile.m_cMetaData.getObjectSize();

	// 以下でオブジェクトの内容を書き込む
	char* pPointer = m_pPointer;
	// オブジェクトの情報を書き込む
	pPointer = writeObjectHeader(pPointer, cData_);

	// フィールド値を得るためのイテレーター
	DirectField cField(m_cFile.m_cMetaData, pPointer);

	// 書き込むべきフィールドIDを得ながら固定長の値をセットしていく
	TargetIterator cTargetIterator(pTarget_, &m_cFile.m_cMetaData);

	// 可変長だけを更新する場合はいきなりhasNext()==falseなので
	// 他の場合とは異なりwhileを先にする

	while (cTargetIterator.hasNext()) {

		Tools::FieldNum iFieldID = cTargetIterator.getNext();

		// 可変長のデータはFile::divideTargetsで分離されているはず
		; _SYDNEY_ASSERT(!m_cFile.m_cMetaData.isVariable(iFieldID));

		// Nullデータは書き込まない
		if (cData_.isNull(iFieldID)) {
			continue;
		}
		// それ以外のデータは書き込む
		cField.seek(iFieldID);
		cField.updateField(
			cData_.get()->getElement(cTargetIterator.getIndex()).get());

#ifdef DEBUG
		SydRecordDebugMessage
			<< "Update fixed field: "
			<< iFieldID
			<< " data: "
			<< cData_.get()->getElement(cTargetIterator.getIndex())->getString()
			<< ModEndl;
#endif
	}

	// detachPageAllを使う場合は物理ページ記述子へのポインターを0にするだけ
	detachPage(Operation::Write);
}

//	FUNCTION public
//	Record::DirectIterator::expunge --
//
//	NOTES
//
//	ARGUMENTS
//		Tools::ObjectID iFreeObjectID_
//			現在の空きオブジェクトIDリストの先頭
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
DirectIterator::
expunge(Tools::ObjectID iFreeObjectID_)
{
	// エラー処理はdiscardableの機構で行うのでエラー処理はしない
	attachPage(Operation::Expunge);

	// ページヘッダーを読み込む
	readHeader();

	if (!m_cPageHeader.isExist(m_iAreaID)) {
		SydErrorMessage
			<< "Unused areaID is requested to be expunged.(" << m_iObjectID << ")";
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	// ヘッダー情報を書き換えて書き込む
	m_cPageHeader.setExpunge(m_iAreaID);
	writeHeader();

	// ポインターをオブジェクトを書き込む先頭の位置に移動させる
	m_pPointer += m_iAreaID * m_cFile.m_cMetaData.getObjectSize();

	(void) Tools::writeObjectID(m_pPointer, iFreeObjectID_);

	// detachPageAllを使う場合は物理ページ記述子へのポインターを0にするだけ
	detachPage(Operation::Write);
}

///////////////////////
// 整合性検査用の関数
///////////////////////

//	FUNCTION public
//	Record::DirectIterator::nextPage --
//
//	NOTES
//
//	ARGUMENTS
//   Admin::Verification::Progress* pProgress_ = 0
//		整合性検査の途中経過へのポインター
// 		0以外が指定されたときはattachPageの代わりにverifyPageを使う
//
//	RETURN
//		移動した先のページID
//		移動できなかった場合はUndefinedが返る
//
//	EXCEPTIONS

PhysicalFile::PageID
DirectIterator::
nextPage(Admin::Verification::Progress* pProgress_)
{
	if (m_iPageID == PhysicalFile::ConstValue::UndefinedPageID) {
		// ヘッダーページを指しておく
		// 下のgetNextPageIDでヘッダーの次のページが得られる
		m_iPageID = m_cFile.getInformationPageID();

	} else {
		detachPage(Operation::Verify);
	}

	m_iPageID = m_cFile.m_pFile->getNextPageID(m_cFile.m_cTrans, m_iPageID);
	if (m_iPageID != PhysicalFile::ConstValue::UndefinedPageID) {
		attachPage(Operation::Verify, pProgress_);
		if (pProgress_ && !pProgress_->isGood()) {
			return m_iPageID = PhysicalFile::ConstValue::UndefinedPageID;
		}
		readHeader();
		m_iAreaID = PhysicalFile::ConstValue::UndefinedAreaID;
		if (m_cPageHeader.m_iObjectNumber > 0) {
			for (ModSize i = 0; i < m_cPageHeader.m_iObjectPerPage; ++i) {
				if (m_cPageHeader.isExist(i)) {
					m_iAreaID = i;
					break;
				}
			}
		}
	}

	return m_iPageID;
}

//	FUNCTION public
//	Record::DirectIterator::isExist --
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		イテレーターの位置にオブジェクトがあればtrue
//
//	EXCEPTIONS

bool
DirectIterator::
isExist()
{
	return m_iAreaID != PhysicalFile::ConstValue::UndefinedAreaID;
}

//	FUNCTION public
//	Record::DirectIterator::verifyPageData --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//		イテレーターが載っているページのオブジェクト数
//
//	EXCEPTIONS

ModSize
DirectIterator::
verifyPageData(Admin::Verification::Treatment::Value iTreatment_,
			   Admin::Verification::Progress& cProgress_)
{
	// attachPage、readHeaderされていることが前提
	; _SYDNEY_ASSERT(m_pPage);

	// ビットマップの1の数とオブジェクト数の一致を調べる
	ModSize iBits = 0;
	for (ModSize i = 0; i < m_cPageHeader.m_iObjectPerPage; ++i) {
		if (m_cPageHeader.isExist(i))
			iBits++;
	}
	if (iBits != m_cPageHeader.m_iObjectNumber) {
		_SYDNEY_VERIFY_INCONSISTENT(
			cProgress_,
			m_cFile.getPath(),
			Message::InconsistentPageObjectNumber(m_iPageID,
												  m_cPageHeader.m_iObjectNumber,
												  iBits));
		// 以降の処理を行わないのでdetachしておく
		detachPage(Operation::Read);
	}
	return m_cPageHeader.m_iObjectNumber;
}

//	FUNCTION public
//	Record::DirectIterator::verifyFreeObjectID --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//		次のID
//
//	EXCEPTIONS

Tools::ObjectID
DirectIterator::
verifyFreeObjectID(Admin::Verification::Treatment::Value iTreatment_,
				   Admin::Verification::Progress& cProgress_,
				   Tools::ObjectID iFreeID_)
{
	; _SYDNEY_ASSERT(iFreeID_ < Tools::m_UndefinedObjectID);
	PhysicalPosition pos(iFreeID_);

	if (!m_cFile.m_pFile->isUsedPage(m_cFile.m_cTrans, pos.m_PageID)) {
		// 使用されていないページを指すはずがない
		_SYDNEY_VERIFY_INCONSISTENT(
			cProgress_,
			m_cFile.getPath(),
			Message::ObjectNotFound(iFreeID_));
		if (m_pPage) detachPage(Operation::Read);
		return Tools::m_UndefinedObjectID;
	}

	if (m_pPage && m_iPageID != pos.m_PageID) {
		// ページが変わっていたらデタッチする
		detachPage(Operation::Read);
	}
	m_iPageID = pos.m_PageID;
	m_iAreaID = pos.m_AreaID;
	if (!m_pPage) {
		attachPage(Operation::Read);
	}

	try {
		readHeader();
		if (m_cPageHeader.isExist(m_iAreaID)) {
			// 削除オブジェクトIDに対応するビットが立っていてはいけない
			_SYDNEY_VERIFY_INCONSISTENT(
				cProgress_,
				m_cFile.getPath(),
				Message::BadFreeObjectID(iFreeID_));
			detachPage(Operation::Read);
			return Tools::m_UndefinedObjectID;
		}

		// ポインターをオブジェクトを書き込む先頭の位置に移動させる
		m_pConstPointer += m_iAreaID * m_cFile.m_cMetaData.getObjectSize();

		// 次のオブジェクトIDを読み込む
		Tools::ObjectID iNextID;
		(void) Tools::readObjectID(m_pConstPointer, iNextID);

		return iNextID;
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		detachPage(Operation::Read);
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Record::DirectIterator::attachPage --
//
//	NOTES
//
//	ARGUMENTS
//	   Record::DirectIterator::Operation::Value eValue_
//			操作の種類
//	   Admin::Verification::Progress* pProgress_ = 0
//			整合性検査の途中経過へのポインター
// 			0以外が指定されたときはattachPageの代わりにverifyPageを使う
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
DirectIterator::
attachPage(Operation::Value eValue_, Admin::Verification::Progress* pProgress_)
{
	if (m_cFile.m_cTrans.isNoVersion() == false && m_bFetch)
	{
		m_pPage = getCachePage(m_iPageID);
	}
	else
	{
		if (eValue_ == Operation::Batch) {
			m_pPage = m_pBatchCachePage;
		}

		if (m_pPage &&
			(m_pPage->getID() != m_iPageID
			 || _FixModeTable[m_eOperation] != _FixModeTable[eValue_]))
		{
			; _SYDNEY_ASSERT((eValue_ == Operation::Read)
							 ||
							 (eValue_ == Operation::Verify)
							 ||
							 (eValue_ == Operation::Batch));
			detachPage(eValue_);
			m_pBatchCachePage = 0;
		}
		if (!m_pPage) {
			if (m_iPageID == PhysicalFile::ConstValue::UndefinedPageID) {
				; _SYDNEY_ASSERT((eValue_ == Operation::Insert)
								 ||
								 (eValue_ == Operation::Batch));
				if (!isValid()) {
					// 挿入に使うページが見つからなかったので
					// 新たに確保する
					m_pPage =
						m_cFile.m_pFile->allocatePage2(m_cFile.m_cTrans,
													   _FixModeTable[eValue_]);
					m_iPageID = m_pPage->getID();
					if (eValue_ == Operation::Batch) {
						m_pBatchCachePage = m_pPage;
					} else {
						m_cFile.m_mapAttachedPage.insert(m_iPageID, m_pPage);
					}
					m_iAreaID = 0;
					m_iObjectID = getObjectID();
				}
			}
			if (m_cFile.m_mapAttachedPage.getSize())
			{
				DirectFile::AttachedPageMap::Iterator iterator
					= m_cFile.m_mapAttachedPage.find(m_iPageID);
				if (iterator != m_cFile.m_mapAttachedPage.end())
					m_pPage = (*iterator).second;
			}
			if (m_pPage == 0)
			{
				m_pPage = attachPage(m_iPageID, eValue_, pProgress_);
				if (eValue_ == Operation::Batch) {
					m_pBatchCachePage = m_pPage;
				} else if (eValue_ != Operation::Read && eValue_ != Operation::Verify) {
					m_cFile.m_mapAttachedPage.insert(m_iPageID, m_pPage);
				}
			}
			m_eOperation = eValue_;
		}
	}
}

//	FUNCTION public
//	Record::DirectIterator::detachPage --
//
//	NOTES
//
//	ARGUMENTS
//		Record::DirectIterator::Operation::Value eValue_
//			操作の種類
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
DirectIterator::
detachPage(Operation::Value eValue_)
{
	if (m_cFile.m_cTrans.isNoVersion() == false && m_bFetch)
	{
		clearCachePage();
	}
	else
	{
		if (m_pPage == 0) return;
	
		if (_UnfixModeTable[eValue_] == PhysicalFile::Page::UnfixMode::Dirty) {
			// discardableなのでここでは実際にデタッチせずに
			// Dirtyフラグだけ立てる
			m_pPage->dirty();
		}

		if (eValue_ != Operation::Batch) {
			// デタッチする
			DirectFile::AttachedPageMap::Iterator i
				= m_cFile.m_mapAttachedPage.find(m_pPage->getID());
			if (i == m_cFile.m_mapAttachedPage.end())
			{
				// マップで管理されていない
				detachPage(m_pPage, eValue_);
			}
		}
	}
	m_pPage = 0;
}

//////////////
// 内部関数 //
//////////////

//	FUNCTION private
//	Record::DirectIterator::attachPage --
//
//	NOTES
//
//	ARGUMENTS
//		PhysicalFile::PageID iPageID_
//			アタッチするページID
//			省略した場合はイテレーターが現在指している値が用いられる
//		Record::DirectIterator::Operation::Value eValue_
//			操作の種類
//	   Admin::Verification::Progress* pProgress_ = 0
//			整合性検査の途中経過へのポインター
// 			0以外が指定されたときはattachPageの代わりにverifyPageを使う
//
//	RETURN
//		ページ記述子が取得できたら0以外
//
//	EXCEPTIONS

PhysicalFile::Page*
DirectIterator::
attachPage(PhysicalFile::PageID iPageID_, Operation::Value eValue_,
		   Admin::Verification::Progress* pProgress_)
{
	; _SYDNEY_ASSERT(iPageID_ != PhysicalFile::ConstValue::UndefinedPageID);
	switch (eValue_) {
	case Operation::Verify:
	{
		if (pProgress_) {
			PhysicalFile::Page* page;
			
			Admin::Verification::Progress cTmp(pProgress_->getConnection());
			page = m_cFile.m_pFile->verifyPage( m_cFile.m_cTrans,
											   iPageID_,
											   _FixModeTable[eValue_],
												cTmp);
			*pProgress_ += cTmp;
			if (cTmp.isGood()) {
				return page;
			} else {
				if (page) {
					detachPage(page ,eValue_);
				}
				return 0;
			}
		}
	}
	default:
		break;
	}
	return m_cFile.m_pFile->attachPage(m_cFile.m_cTrans,
									   iPageID_,
									   _FixModeTable[eValue_],
									   _ePriority);
}

//	FUNCTION private
//	Record::DirectIterator::detachPage --
//
//	NOTES
//
//	ARGUMENTS
//		PhysicalFile::Page*& pPage_
//			アタッチしていたページ記述子
//		Record::DirectIterator::Operation::Value eValue_
//			操作の種類
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
DirectIterator::
detachPage(PhysicalFile::Page*& pPage_,
		   Operation::Value eValue_)
{
	if (pPage_ == 0) return;
	
	m_cFile.m_pFile->detachPage(pPage_,
								_UnfixModeTable[eValue_]);
}

//	FUNCTION private
//	Record::DirectIterator::readHeader --
//
//	NOTES
//
//	ARGUMENTS
//		PhysicalFile::Page& cPage_
//			ヘッダーを読み込むページ
//		DirectIterator::PageHeader& cPageHeader_
//			[OUT]読み込んだ内容を入れる変数
//
//	RETURN
//		ヘッダーの次のアドレス
//
//	EXCEPTIONS

// static
const char*
DirectIterator::
readHeader(PhysicalFile::Page& cPage_, DirectIterator::PageHeader& cPageHeader_)
{
	const char* pPointer = cPage_.operator const char*();

	// 先頭から
	// 1．オブジェクト数
	// 2．ビットマップ
	// の順に書き込まれている

	Os::Memory::copy(&(cPageHeader_.m_iObjectNumber), pPointer, sizeof(ModSize));
	pPointer += sizeof(ModSize);

	// 読み込む
	pPointer += Tools::readBitmap(pPointer,
								  0, cPageHeader_.m_iObjectPerPage,
								  cPageHeader_.m_cBitMap);

	return pPointer;
}

void
DirectIterator::
readHeader()
{
	; _SYDNEY_ASSERT(m_pPage);
	m_cPageHeader.m_iObjectPerPage = m_cFile.m_cMetaData.getObjectPerPage();
	m_pConstPointer = readHeader(*m_pPage, m_cPageHeader);
}

//	FUNCTION private
//	Record::DirectIterator::writeHeader --
//
//	NOTES
//
//	ARGUMENTS
//		PhysicalFile::Page& cPage_
//			ヘッダーを書き込むページ
//		const DirectIterator::PageHeader& cPageHeader_
//			書き込む内容が入っている変数
//
//	RETURN
//		ヘッダーの次を指すポインター
//
//	EXCEPTIONS

// static
char*
DirectIterator::
writeHeader(PhysicalFile::Page& cPage_,
			const DirectIterator::PageHeader& cPageHeader_)
{
	char* pPointer = cPage_.operator char*();

	// 先頭から
	// 1．オブジェクト数
	// 2．ビットマップ
	// の順に書き込む

	Os::Memory::copy(pPointer, &(cPageHeader_.m_iObjectNumber), sizeof(ModSize));
//	*syd_reinterpret_cast<ModSize*>(pPointer) = cPageHeader_.m_iObjectNumber;
	pPointer += sizeof(ModSize);

	pPointer += Tools::writeBitmap(pPointer,
								   0, cPageHeader_.m_iObjectPerPage,
								   cPageHeader_.m_cBitMap);

	return pPointer;
}

void
DirectIterator::
writeHeader()
{
	; _SYDNEY_ASSERT(m_pPage);
	m_pPointer = writeHeader(*m_pPage, m_cPageHeader);
}

//	FUNCTION private
//	Record::DirectIterator::skipHeader --
//
//	NOTES
//
//	ARGUMENTS
//		PhysicalFile::Page& cPage_
//			ヘッダーを読み飛ばすページ
//		const DirectIterator::PageHeader& cPageHeader_
//			読み飛ばす内容が入っている変数
//
//	RETURN
//		ヘッダーの次を指すポインター
//
//	EXCEPTIONS

// static
const char*
DirectIterator::
skipHeader(PhysicalFile::Page& cPage_,
		   const DirectIterator::PageHeader& cPageHeader_)
{
	const char* pPointer = cPage_.operator const char*();

	// 先頭から
	// 1．オブジェクト数
	// 2．ビットマップ
	// の順に書き込まれている

	pPointer += sizeof(ModSize);
	pPointer += Tools::getBitmapSize(cPageHeader_.m_iObjectPerPage);

	return pPointer;
}

void
DirectIterator::
skipHeader()
{
	; _SYDNEY_ASSERT(m_pPage);
	m_cPageHeader.m_iObjectPerPage = m_cFile.m_cMetaData.getObjectPerPage();
	m_pConstPointer = skipHeader(*m_pPage, m_cPageHeader);
}

//	FUNCTION private
//	Record::DirectIterator::readObjectHeader --
//		オブジェクトの情報を読み込む
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS

const char*
DirectIterator::
readObjectHeader(const char* pPointer_,
				 DirectFile::DataPackage& cData_)
{
	const char* pPointer = pPointer_;
	// オブジェクトの先頭からNull Bitmapが格納されている
	pPointer += Tools::readBitmap(pPointer, 1,
								  m_cFile.m_cMetaData.getFieldNumber(),
								  cData_.getNullBitMap());
	if (m_cFile.m_cMetaData.hasVariable()) {
		// 可変長がある場合はNull Bitmapの次に
		// 可変長フィールドのオブジェクトIDが入っている
		Tools::ObjectID iVariableID;
		pPointer += Tools::readObjectID(pPointer, iVariableID);
		cData_.setVariableID(iVariableID);
	}
	return pPointer;
}

//	FUNCTION private
//	Record::DirectIterator::writeObjectHeader --
//		オブジェクトの情報を書き出す
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS

char*
DirectIterator::
writeObjectHeader(char* pPointer_,
				  const DirectFile::DataPackage& cData_)
{
	char* pPointer = pPointer_;
	// 先頭からNull Bitmapを格納する
	pPointer += Tools::writeBitmap(pPointer, 1,
								   m_cFile.m_cMetaData.getFieldNumber(),
								   cData_.getNullBitMap());
	if (m_cFile.m_cMetaData.hasVariable()) {
		// 可変長がある場合はNull Bitmapの次に
		// 可変長フィールドのオブジェクトIDを入れる
		pPointer += Tools::writeObjectID(pPointer, cData_.getVariableID());
	}
	return pPointer;
}

//	FUNCTION private
//	Record::DirectIterator::searchObjectID --
//		現在位置からさまざまな条件でオブジェクトIDを探す
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//		見つかったオブジェクトID
//		見つからなかったときはTools::m_UndefinedObjectIDが返る
//
//	EXCEPTIONS

Tools::ObjectID
DirectIterator::
searchObjectID(AreaIDIterator& cIterator_,
			   CheckBitmap cCheckBitmap_,
			   CheckObjectNumber cCheckObjectNumber_,
			   GetPageID cGetPageID_,
			   SearchPage::Value eSearchPage_,
			   Admin::Verification::Progress* pProgress_/*=0*/)
{
	const Operation::Value ope = (eSearchPage_ == SearchPage::Verify)? Operation::Verify : Operation::Read;
	bool bAttached = false;
	if (!m_pPage) {
		attachPage(ope ,pProgress_);
		if (pProgress_ && !pProgress_->isGood()) {
			return Tools::m_UndefinedObjectID;
		}
		bAttached = true;
		readHeader();
	}
	; _SYDNEY_ASSERT(m_pPage);
	// 同一ページ内で探す
	for (AreaID i = cIterator_.getCurrent(); cIterator_.hasNext();
		 i = cIterator_.getNext())
	{
		if ((*cCheckBitmap_)(m_cPageHeader, i))
		{
			if (eSearchPage_ == SearchPage::Replace
				|| eSearchPage_ == SearchPage::Verify)
			{
				m_iAreaID = i;
			}
			if (bAttached)
			{
				detachPage(ope);
			}
			return PhysicalPosition::getObjectID(m_iPageID, i);
		}
	}

	if (bAttached) {
		detachPage(ope);
	}

	if (eSearchPage_ == SearchPage::Verify)
	{
		// 他のページを探さない
		return Tools::m_UndefinedObjectID;
	}

	// 同一ページになかったのでページを順次探す
	DirectIterator::PageHeader cPageHeader;
	PhysicalFile::Page* pPage = 0;
	cPageHeader.m_iObjectPerPage = m_cFile.m_cMetaData.getObjectPerPage();

	try { // while節の中にtry-catchがあると重いので外に出した

		PhysicalFile::PageID iPageID = m_iPageID;
		while ((iPageID = (this->*cGetPageID_)(iPageID))
			   != PhysicalFile::ConstValue::UndefinedPageID) {
			if (iPageID == m_cFile.getInformationPageID()) {
				// ヘッダーページを見ていたら終了
				break;
			}
			// ページをアタッチしてヘッダーを読み込む
			pPage = attachPage(iPageID, ope ,pProgress_);
			if (pProgress_ && !pProgress_->isGood()) {
				return Tools::m_UndefinedObjectID;
			}
			; _SYDNEY_ASSERT(pPage);
			const char* pPointer = readHeader(*pPage, cPageHeader);

			// オブジェクトがこのページにあるなら探す
			if ((*cCheckObjectNumber_)(cPageHeader)) {
				for (AreaID ii = cIterator_.reset(); cIterator_.hasNext();
					 ii = cIterator_.getNext()) {
					if ((*cCheckBitmap_)(cPageHeader, ii)) {
						switch (eSearchPage_) {
						case SearchPage::Read:
						{
							detachPage(pPage, ope);
							return PhysicalPosition::getObjectID(iPageID, ii);
						}
						case SearchPage::Replace:
						{
							// 新しいページをIteratorが指すページにする
							detachPage(ope);
							m_pPage = pPage;
							m_iPageID = iPageID;
							m_iAreaID = ii;
							m_cPageHeader = cPageHeader;
							m_pConstPointer = pPointer;
							m_iObjectID
								= PhysicalPosition::getObjectID(iPageID, ii);
							return m_iObjectID;
						}
						default:
							break;
						}
					}
				}
				// CheckObjectNumberがtrueなのにすべてのCheckBitMapが失敗するはずがない
				detachPage(pPage, ope);
				_SYDNEY_THROW0(Exception::Unexpected);
			}
			detachPage(pPage, ope);
		}

		// ひとつも見つからなかった
		return Tools::m_UndefinedObjectID;

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		if (pPage) {
			detachPage(pPage, ope);
		}
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION private
//	Record::DirectIterator::getNextPageID -- 次のページIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFIle::PageID uiCurrentPageID_
//		現在のページID
//
//	RETURN
//	PhysicalFile::PageID
//		次のページID
//		存在していない場合はPhysicalFile::ConstValue::UndefinedPageID
//
//	EXCEPTIONS
//
PhysicalFile::PageID
Record::DirectIterator::getNextPageID(PhysicalFile::PageID uiCurrentPageID_)
{
	if (m_iEndObjectID == Tools::m_UndefinedObjectID)
		return PhysicalFile::ConstValue::UndefinedPageID;
	
	PhysicalPosition pos(m_iEndObjectID);
	uiCurrentPageID_++;
	if (pos.m_PageID < uiCurrentPageID_)
		return PhysicalFile::ConstValue::UndefinedPageID;
	return uiCurrentPageID_;
}

//
//	FUNCTION private
//	Record::DirectIterator::getPrevPageID -- 前のページIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFIle::PageID uiCurrentPageID_
//		現在のページID
//
//	RETURN
//	PhysicalFile::PageID
//		前のページID
//		存在していない場合はPhysicalFile::ConstValue::UndefinedPageID
//
//	EXCEPTIONS
//
PhysicalFile::PageID
Record::DirectIterator::getPrevPageID(PhysicalFile::PageID uiCurrentPageID_)
{
	if (m_iStartObjectID == Tools::m_UndefinedObjectID
		|| uiCurrentPageID_ == 0)
		return PhysicalFile::ConstValue::UndefinedPageID;
	
	PhysicalPosition pos(m_iStartObjectID);
	uiCurrentPageID_--;
	if (pos.m_PageID > uiCurrentPageID_)
		return PhysicalFile::ConstValue::UndefinedPageID;
	return uiCurrentPageID_;
}

//
//	FUNCTION private
//	Record::DirectIterator::getCachePage -- キャッシュしているページを検索する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		ページID
//
//	RETURN
//	PhysicalFile::Page*
//		物理ページ
//
//	EXCEPTIONS
//
PhysicalFile::Page*
Record::DirectIterator::getCachePage(PhysicalFile::PageID uiPageID_)
{
	int n = 0;
	PageList::Iterator j = m_cPageList.end();
	PageList::Iterator i = m_cPageList.begin();
	for (; i != m_cPageList.end(); ++i)
	{
		if ((*i).m_pPage->getID() == uiPageID_)
		{
			j = i;
			break;
		}
		if (++n > 10)
			j = i;
	}

	PhysicalFile::Page* pPage = 0;
	
	if (j != m_cPageList.end())
	{
		if ((*j).m_pPage->getID() != uiPageID_)
		{
			// 違うページなので、置き換える
			m_cFile.m_pFile->detachPage(
				(*j).m_pPage,
				PhysicalFile::Page::UnfixMode::NotDirty);
			(*j).m_pPage
				= m_cFile.m_pFile->attachPage(m_cFile.m_cTrans,
											  uiPageID_,
											  Buffer::Page::FixMode::ReadOnly);
		}
		// 先頭にする
		m_cPageList.splice(m_cPageList.begin(), m_cPageList, j);

		pPage = (*j).m_pPage;
	}
	else
	{
		// 見つからなかった
		Page* p = new Page(
			m_cFile.m_pFile->attachPage(m_cFile.m_cTrans,
										uiPageID_,
										Buffer::Page::FixMode::ReadOnly));
		m_cPageList.pushFront(*p);

		pPage = p->m_pPage;
	}

	return pPage;
}

//
//	FUNCTION private
//	Record::DirectIterator::clearCachePage -- キャッシュを破棄する
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
DirectIterator::clearCachePage()
{
	PageList::Iterator i = m_cPageList.begin();
	while (i != m_cPageList.end())
	{
		Page* p = &(*i);
		++i;

		m_cFile.m_pFile->detachPage(p->m_pPage,
									PhysicalFile::Page::UnfixMode::NotDirty);

		m_cPageList.erase(*p);
		
		delete p;
	}
}

//
//	Copyright (c) 2001, 2003, 2004, 2005, 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
