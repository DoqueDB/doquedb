// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LinkedObject.cpp -- リンクオブジェクトクラス
// 
// Copyright (c) 2001, 2006, 2023 Ricoh Company, Ltd.
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
#ifdef DEBUG
#include <stdio.h>
#endif
#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Record/LinkedObject.h"
#include "Record/Module.h"
#include "Record/FreeAreaManager.h"
#include "Record/PhysicalPosition.h"
#include "Record/UseInfo.h"
#include "Record/Debug.h"
#include "Record/VariableField.h"

#include "Common/Assert.h"
#include "Common/Message.h"
#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"
#include "FileCommon/DataManager.h"
#include "PhysicalFile/File.h"
#include "PhysicalFile/Page.h"

#include "Admin/Verification.h"

_SYDNEY_USING
_SYDNEY_RECORD_USING

//	FUNCTION public
//	Record::LinkedObject::LinkedObject -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	Record::LinkedObject::Operation::Value eOperation_
//		リンクオブジェクトの使い方
//
//	FreeAreaManager&			cFreeAreaManager_
//		リンクオブジェクトの書き込み先
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
LinkedObject::LinkedObject(Operation::Value		eOperation_,
						   FreeAreaManager&		cFreeAreaManager_)
	: m_eOperation(eOperation_),
	  m_rFreeAreaManager(cFreeAreaManager_),
	  m_iTopObjectID(Tools::m_UndefinedObjectID),
	  m_iNextObjectID(Tools::m_UndefinedObjectID),
	  m_pPage(0), m_pPointer(0), m_iRestSize(0), m_iHeaderSize(0),
	  m_bDirty(false)
{
}

//
//	FUNCTION public
//	Record::LinkedObject::~LinkedObject -- デストラクタ
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
//	なし
//
LinkedObject::~LinkedObject()
{
	if (m_pPage) {
		detachPage(m_pPage, m_eOperation);
	}
}

//	FUNCTION public
//	Record::LinkedObject::create -- 
//		リンクオブジェクトを作成する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Memory::Size ulRequiredSize_
//			必要なバイト数
//		Record::Tools::ObjectID& iFreeID_
//			[IN/OUT]空きオブジェクトリストの先頭
//		Os::Memory::Size& ulAllocatedSize_
//			[OUT]確保できたバイト数
//
//	REUTRN
//		先頭オブジェクトのID
//
//	EXCEPTIONS

Tools::ObjectID
LinkedObject::
create(Os::Memory::Size	ulRequiredSize_,
	   Tools::ObjectID& iFreeID_)
{
	Os::Memory::Size ulLinkHeaderSize = Tools::m_ObjectTypeArchiveSize;
	Tools::ObjectID iNextObjectID = Tools::m_UndefinedObjectID;

	Tools::ObjectType iObjectType = Tools::m_NormalObjectType;

	// エリアを作る
	PhysicalFile::PageID	pageID =
		PhysicalFile::ConstValue::UndefinedPageID;
	PhysicalFile::AreaID	areaID =
		PhysicalFile::ConstValue::UndefinedAreaID;

	if (ulRequiredSize_ + ulLinkHeaderSize
		<= m_rFreeAreaManager.getPageDataSize()) {
		// すべて書けるのでリンクなしで作る
		m_rFreeAreaManager.getFreeArea(ulRequiredSize_ + ulLinkHeaderSize,
									   pageID, areaID,
									   m_eOperation);
										// 要求したサイズが欲しい

	} else {
		// すべて書くことはできないのでリンクをつけたエリアを作る
		iObjectType |= Tools::m_LinkedObjectType;
		ulLinkHeaderSize += Tools::m_ObjectIDArchiveSize;
		Os::Memory::Size ulAllocatedSize = 0;

		m_rFreeAreaManager.getFreeArea(m_rFreeAreaManager.getPageDataSize() - ulLinkHeaderSize,
									   pageID, areaID,
									   iFreeID_,
									   ulAllocatedSize, /* 実際のサイズはここへ */
									   m_eOperation);
										// 要求したサイズより小さくてもよい

		; _SYDNEY_ASSERT(ulAllocatedSize > ulLinkHeaderSize);

		// 余る分を書き込むエリアを作る(再帰呼び出し)
		iNextObjectID = create(ulRequiredSize_ + ulLinkHeaderSize - ulAllocatedSize,
							   iFreeID_);
	}

	; _SYDNEY_ASSERT(
		pageID != PhysicalFile::ConstValue::UndefinedPageID);
	; _SYDNEY_ASSERT(
		areaID != PhysicalFile::ConstValue::UndefinedAreaID);

	PhysicalFile::Page* pPage = attachPage(pageID, m_eOperation);
	; _SYDNEY_ASSERT(pPage != 0);

	try {
		char*	areaPointer =
			static_cast<char*>(Tools::getAreaTop(pPage, areaID));
		// 注意：この領域は永続化されているので、
		// 　　　アクセスは物理フォーマットに注意すること

		// オブジェクト種を書き込む
		*areaPointer++ = iObjectType;

		if (Tools::isLinkedObjectType(iObjectType)) {
			// 次のオブジェクトIDを書き込む
			; _SYDNEY_ASSERT(iNextObjectID < Tools::m_UndefinedObjectID);
			areaPointer +=
				Tools::writeObjectID(areaPointer, iNextObjectID);
		}
		m_bDirty = true;

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// ★注意★
		// getFreeAreaした後一度もdetachせずにエラーが起きたら
		// NotDirtyでdetachしても大丈夫のはずである

		// detachPageAllが使えるようになったら
		// ここのエラー処理も不要になる
		// freeUsedAreaは必要か？

		m_rFreeAreaManager.freeUsedArea(areaID, pPage, iFreeID_);
		detachPage(pPage, Operation::Read);

		// これまでに作成したリンクの先も破棄する
		if (iNextObjectID < Tools::m_UndefinedObjectID) {
			deleteAll(iNextObjectID, iFreeID_);
		}

		_SYDNEY_RETHROW;
	}

#ifdef DEBUG
	static char tmpBuf[13];
	static char tmpBuf2[13];
	::sprintf(tmpBuf, "%012llx", PhysicalPosition::getObjectID(pageID, areaID));
	SydRecordDebugMessage
		<< "LinkedObject create: " << tmpBuf
		<< ModEndl;
	SydRecordSizeMessage
		<< "LinkedObject create: " << tmpBuf
		<< " Size = " << ModDec << pPage->getAreaSize(areaID) << ModEndl;
#endif

	detachPage(pPage, m_eOperation);

	return PhysicalPosition::getObjectID(pageID, areaID);
}

//	FUNCTION public
//	Record::LinkedObject::attachObject -- 
//		リンクオブジェクトのアクセスを開始する
//
//	NOTES
//
//	ARGUMENTS
//		Tools::ObjectID iObjectID_
//			アクセスするリンクオブジェクトの先頭オブジェクトID
//
//	REUTRN
//		なし
//
//	EXCEPTIONS

void
LinkedObject::
attachObject(Tools::ObjectID iObjectID_)
{
	attachLinkedObject(iObjectID_);
	m_iTopObjectID = iObjectID_;
}

//	FUNCTION public
//	Record::LinkedObject::verifyObject -- 
//		リンクオブジェクトのアクセスを開始する
//
//	NOTES
//
//	ARGUMENTS
//		Tools::ObjectID iObjectID_
//			アクセスするリンクオブジェクトの先頭オブジェクトID
//
//	REUTRN
//		なし
//
//	EXCEPTIONS

void
LinkedObject::
verifyObject(Tools::ObjectID iObjectID_ ,Admin::Verification::Progress& cProgress_)
{
	verifyLinkedObject(iObjectID_ ,cProgress_);
	m_iTopObjectID = iObjectID_;
}

//	FUNCTION public
//	Record::LinkedObject::detachObject -- 
//		リンクオブジェクトのアクセスを終了する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	REUTRN
//		なし
//
//	EXCEPTIONS

void
LinkedObject::
detachObject()
{
	detachLinkedObject();
	m_iTopObjectID = Tools::m_UndefinedObjectID;
}

//	FUNCTION public
//	Record::LinkedObject::isAttached -- 
//		リンクオブジェクトのアクセスが開始されているか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	REUTRN
//		なし
//
//	EXCEPTIONS

bool
LinkedObject::
isAttached() const
{
	return (m_pPage != 0);
}

#ifdef OBSOLETE
//	FUNCTION public
//	Record::LinkedObject::reset -- 
//		現在の位置を先頭にする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	REUTRN
//		なし
//
//	EXCEPTIONS

void
LinkedObject::
reset()
{
	; _SYDNEY_ASSERT(isAttached());

	PhysicalPosition pos(m_iTopObjectID);

	// 現在見ているページが先頭でなければアタッチしなおすだけでよい
	if (m_pPage->getID() != pos.m_PageID) {
		detachLinkedObject();
		attachLinkedObject(m_iTopObjectID);
	}
	// 同じページなら先頭からヘッダー分を飛ばす
	m_pConstPointer = Tools::getConstAreaTop(m_pPage, pos.m_AreaID);
	m_pConstPointer += m_iHeaderSize;
	m_iRestSize = m_pPage->getAreaSize(pos.m_AreaID) - m_iHeaderSize;
}
#endif //OBSOLETE

//	FUNCTION public
//	Record::LinkedObject::read -- 
//		現在の位置からオブジェクトを読み込む
//
//	NOTES
//
//	ARGUMENTS
//		void* pPointer_
//			読み込んだデータを格納するアドレス
//		Os::Memory::Size iSize_
//			読み込むデータサイズ
//
//	REUTRN
//		なし
//
//	EXCEPTIONS

void
LinkedObject::
read(void* pPointer_, Os::Memory::Size iSize_)
{
	if (iSize_) {
		char* pPointer = syd_reinterpret_cast<char*>(pPointer_);
		Os::Memory::Size iSize = iSize_;

		for (;;) {
			if (m_iRestSize >= iSize) {
				// 現在見ているオブジェクトに格納されている
				Os::Memory::copy(pPointer, m_pConstPointer, iSize);
				m_pConstPointer += iSize;
				m_iRestSize -= iSize;
				return;
			}
			// 現在見ているオブジェクトにすべて格納されてはいない
			; _SYDNEY_ASSERT(hasNext());

			if (m_iRestSize > 0) {
				Os::Memory::copy(pPointer, m_pConstPointer, m_iRestSize);
				pPointer += m_iRestSize;
				iSize -= m_iRestSize;
			}
			detachLinkedObject();
			attachLinkedObject(m_iNextObjectID);
		}
	}
}

//	FUNCTION public
//	Record::LinkedObject::write -- 
//		現在の位置からオブジェクトを書き込む
//
//	NOTES
//
//	ARGUMENTS
//		const void* pPointer_
//			書き込むデータが格納されているアドレス
//		Os::Memory::Size iSize_
//			書き込むデータサイズ
//
//	REUTRN
//		なし
//
//	EXCEPTIONS

void
LinkedObject::
write(const void* pConstPointer_, Os::Memory::Size iSize_)
{
	; _SYDNEY_ASSERT((m_eOperation == Operation::Write)
					 ||
					 (m_eOperation == Operation::Batch));

	if (iSize_) {
		const char* pPointer = syd_reinterpret_cast<const char*>(pConstPointer_);
		Os::Memory::Size iSize = iSize_;

		for (;;) {
			if (m_iRestSize >= iSize) {
				// 現在見ているオブジェクトに格納できる
				Os::Memory::copy(m_pPointer, pPointer, iSize);
				m_pPointer += iSize;
				m_iRestSize -= iSize;
				m_bDirty = true;
				return;
			}
			// 現在見ているオブジェクトに格納できない
			; _SYDNEY_ASSERT(hasNext());

			if (m_iRestSize > 0) {
				Os::Memory::copy(m_pPointer, pPointer, m_iRestSize);
				pPointer += m_iRestSize;
				iSize -= m_iRestSize;
				m_bDirty = true;
			}
			detachLinkedObject();
			attachLinkedObject(m_iNextObjectID);
		}
	}
}

//	FUNCTION public
//	Record::LinkedObject::write -- 
//		現在の位置からオブジェクトを書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Common::DataType::Type eType_
//			データ型
//		Common::DataType::Type eElementType_
//			データが配列のとき要素の型
//		Common::DataArrayData::Pointer& pData_
//			結果を格納する
//		Os::Memory::Size iSize_
//			書き込むデータサイズ
//
//	REUTRN
//		bool
//			いつのエリアに書き込めた場合はtrue、書き込めなかった場合はfalse
//
//	EXCEPTIONS

bool
LinkedObject::
write(const Tools::DataType& cType_,
	  const Tools::DataType& cElementType_,
	  const Common::DataArrayData::Pointer& pData_,
	  Os::Memory::Size iSize_)
{
	; _SYDNEY_ASSERT((m_eOperation == Operation::Write)
					 ||
					 (m_eOperation == Operation::Batch));
	if (iSize_) {
		if (m_iRestSize == 0)
		{
			// 現在見ているオブジェクトに格納できない
			detachLinkedObject();
			attachLinkedObject(m_iNextObjectID);
		}

		if (m_iRestSize < iSize_) return false;

		VariableField::dumpData(cType_, cElementType_, pData_, m_pPointer);
		m_pPointer += iSize_;
		m_iRestSize -= iSize_;
		m_bDirty = true;
	}
	return true;
}

//	FUNCTION public
//	Record::LinkedObject::skip -- 
//		現在の位置からオブジェクトを読み飛ばす
//
//	NOTES
//
//	ARGUMENTS
//		Os::Memory::Size iSize_
//			読み飛ばすデータサイズ
//
//	REUTRN
//		なし
//
//	EXCEPTIONS

void
LinkedObject::
skip(Os::Memory::Size iSize_)
{
	Os::Memory::Size iSize = iSize_;

	for (;;) {
		if (m_iRestSize >= iSize) {
			// 現在見ているオブジェクトに指すべき場所がある
			m_pConstPointer += iSize;
			m_iRestSize -= iSize;
			return;
		}
		// 現在見ているオブジェクトに指すべき位置がない

		; _SYDNEY_ASSERT(hasNext());
		if (m_iRestSize > 0) {
			iSize -= m_iRestSize;
		}
		detachLinkedObject();
		attachLinkedObject(m_iNextObjectID);
	}
}

//	FUNCTION public
//	Record::LinkedObject::deleteAll -- 
//		リンクオブジェクトの破棄を行う
//
//	NOTES
//		リンクをたどってすべてのエリアを開放する
//
//	ARGUMENTS
//		Tools::ObjectID iObjectID_
//			破棄を開始するオブジェクトID
//		Tools::ObjectID& iFreeID_
//			[IN/OUT]空き領域オブジェクトIDの先頭
//
//	REUTRN
//		なし
//
//	EXCEPTIONS

void
LinkedObject::
deleteAll(Tools::ObjectID iObjectID_, Tools::ObjectID& iFreeID_)
{
	PhysicalFile::File& cFile = m_rFreeAreaManager.getFile();
	Tools::ObjectID		iObjectID = iObjectID_;
	PhysicalFile::Page* pPage = 0;

	try {
		do {
			// 読み込みを行なうエリアをアタッチ
			PhysicalPosition pos(iObjectID);
			pPage = attachPage(pos.m_PageID, Operation::Expunge);
			; _SYDNEY_ASSERT(pPage != 0);

#ifdef DEBUG
			static char tmpBuf[13];
			::sprintf(tmpBuf, "%012llx", iObjectID);
			SydRecordDebugMessage
				<< "LinkedObject freeArea: " << tmpBuf
				<< ModEndl;
			SydRecordSizeMessage
				<< "LinkedObject freeArea: " << tmpBuf
				<< " Size = " << ModDec << pPage->getAreaSize(pos.m_AreaID) << ModEndl;
#endif

			const char*	areaTop =
				static_cast<const char*>(
					Tools::getConstAreaTop(pPage, pos.m_AreaID));
			; _SYDNEY_ASSERT(areaTop != 0);

			const char* areaPointer = areaTop;
			// 注意：この領域は永続化されているので、
			// 　　　アクセスは物理フォーマットに注意すること

			Tools::ObjectType objectType = *areaPointer++;
			if (Tools::isLinkedObjectType(objectType)) {
				// 次のオブジェクトのObjectIDを得る
				areaPointer += Tools::readObjectID(areaPointer, iObjectID);

			} else {
				iObjectID = Tools::m_UndefinedObjectID;
			}

			// 読み終ったオブジェクトを削除する
			m_rFreeAreaManager.freeUsedArea(pos.m_AreaID, pPage, iFreeID_);
			m_bDirty = true;
			detachPage(pPage, Operation::Expunge);
			; _SYDNEY_ASSERT(!pPage);
			
		} while (iObjectID != Tools::m_UndefinedObjectID);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		if (pPage) {
			detachPage(pPage, Operation::Read);
		}
		_SYDNEY_RETHROW;	// 例外処理が終ったので再送
	}
}

//	FUNCTION public
//	Record::LinkedObject::getTotalSizeVerify -- 
//		リンクオブジェクトが提供しているサイズの合計を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	REUTRN
//		なし
//
//	EXCEPTIONS

Os::Memory::Size
LinkedObject::
getTotalSizeVerify(Admin::Verification::Progress& cProgress_)
{
	; _SYDNEY_ASSERT(isAttached());

	Os::Memory::Size iResult = 0;
	Tools::ObjectID iObjectID = m_iTopObjectID;

	// 呼び出し側でdetachObjectを呼ぶのでtry-catchで囲む必要はない
	for (;;) {
		PhysicalPosition pos(iObjectID);
		; _SYDNEY_ASSERT(pos.m_PageID == m_pPage->getID());

		// 現在アタッチされているページのサイズを加える
		iResult += m_pPage->getAreaSize(pos.m_AreaID);
		// ヘッダー部分のサイズを引く
		iResult -= Tools::m_ObjectTypeArchiveSize;
		if (hasNext()) {
			// オブジェクトIDの分も引く
			iResult -= Tools::m_ObjectIDArchiveSize;

			// 次のエリアを調べる
			iObjectID = m_iNextObjectID;
			detachLinkedObject();
			verifyLinkedObject(m_iNextObjectID ,cProgress_);
		} else {
			break;
		}
	}
	return iResult;
}

//	FUNCTION public
//	Record::LinkedObject::setUseInfo -- 
//
//	NOTES
//
//	ARGUMENTS
//		Tools::ObjectID iObjectID_
//			登録を開始するオブジェクトID
//		Record::UseInfo& cUseInfo_
//			登録情報
//
//	REUTRN
//		なし
//
//	EXCEPTIONS

void
LinkedObject::
setUseInfo(Tools::ObjectID iObjectID_, UseInfo& cUseInfo_ ,Admin::Verification::Progress& cProgress_)
{
	PhysicalFile::File& cFile = m_rFreeAreaManager.getFile();
	Tools::ObjectID		iObjectID = iObjectID_;
	PhysicalFile::Page* pPage = 0;

	try {
		do {
			// 読み込みを行なうエリアをアタッチ
			PhysicalPosition pos(iObjectID);
			pPage = verifyPage(pos.m_PageID, Operation::Read ,cProgress_);
			if (!cProgress_.isGood()) {
				if (pPage) {
					detachPage(pPage, Operation::Read);
				}
				return;
			}
			; _SYDNEY_ASSERT(pPage != 0);

			const char*	areaTop =
				static_cast<const char*>(
					Tools::getConstAreaTop(pPage, pos.m_AreaID));
			; _SYDNEY_ASSERT(areaTop != 0);

			const char* areaPointer = areaTop;
			// 注意：この領域は永続化されているので、
			// 　　　アクセスは物理フォーマットに注意すること

			Tools::ObjectType objectType = *areaPointer++;
			if (Tools::isLinkedObjectType(objectType)) {
				// 次のオブジェクトのObjectIDを得る
				areaPointer += Tools::readObjectID(areaPointer, iObjectID);

			} else {
				iObjectID = Tools::m_UndefinedObjectID;
			}

			// 読み終ったオブジェクトIDを登録する
			cUseInfo_.append(pos.m_PageID, pos.m_AreaID);

			detachPage(pPage, Operation::Read);
			; _SYDNEY_ASSERT(!pPage);
			
		} while (iObjectID != Tools::m_UndefinedObjectID);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		if (pPage) {
			detachPage(pPage, Operation::Read);
		}
		_SYDNEY_RETHROW;	// 例外処理が終ったので再送
	}
}

//	FUNCTION public
//	Record::LinkedObject::attachLinkedObject -- 
//		リンクオブジェクトの１つをアタッチする
//
//	NOTES
//		ObjectIDが正常な値であるかの検査は呼び出し側で行っておく
//
//	ARGUMENTS
//		Tools::ObjectID iObjectID_
//			アクセスするリンクオブジェクトのオブジェクトID
//
//	REUTRN
//		なし
//
//	EXCEPTIONS

void
LinkedObject::
attachLinkedObject(Tools::ObjectID iObjectID_)
{
	PhysicalPosition pos(iObjectID_);
	
	if (m_pPage && m_pPage->getID() != pos.m_PageID) {
		detachLinkedObject();
	}

	if (m_pPage == 0)
	{
		// 使用するエリアをアタッチ
		m_pPage = attachPage(pos.m_PageID, m_eOperation);
	}
	; _SYDNEY_ASSERT(m_pPage);

#ifdef DEBUG
	static char tmpBuf[13];
	static char tmpBuf2[13];
	::sprintf(tmpBuf, "%012llx", iObjectID_);
	SydRecordDebugMessage
		<< "LinkedObject attach: " << tmpBuf
		<< ModEndl;
#endif

	setConstPointer(pos);
}

//	FUNCTION public
//	Record::LinkedObject::verifyLinkedObject -- 
//		リンクオブジェクトの１つをVerifyモードでアタッチする
//
//	NOTES
//		ObjectIDが正常な値であるかの検査は呼び出し側で行っておく
//
//	ARGUMENTS
//		Tools::ObjectID iObjectID_
//			アクセスするリンクオブジェクトのオブジェクトID
//
//	REUTRN
//		なし
//
//	EXCEPTIONS

void
LinkedObject::
verifyLinkedObject(Tools::ObjectID iObjectID_ ,Admin::Verification::Progress& cProgress_)
{
	if (m_pPage) {
		detachLinkedObject();
	}

	// 使用するエリアをアタッチ
	PhysicalPosition pos(iObjectID_);
	m_pPage = verifyPage(pos.m_PageID, m_eOperation ,cProgress_);
	if (!cProgress_.isGood()) {
		return;
	}

#ifdef DEBUG
	static char tmpBuf[13];
	static char tmpBuf2[13];
	::sprintf(tmpBuf, "%012llx", iObjectID_);
	SydRecordDebugMessage
		<< "LinkedObject verify: " << tmpBuf
		<< ModEndl;
#endif

	setConstPointer(pos);
}

//	FUNCTION public
//	Record::LinkedObject::setConstPointer -- 
//		リンクオブジェクトの１つをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//
//	REUTRN
//		なし
//
//	EXCEPTIONS

void
LinkedObject::
setConstPointer(PhysicalPosition& pos)
{
	// アタッチ直後はクリーンな状態である
	m_bDirty = false;

	try	{
		Os::Memory::Size iUsedSize = 0;

		m_pConstPointer = Tools::getConstAreaTop(m_pPage, pos.m_AreaID);
		; _SYDNEY_ASSERT(m_pConstPointer);

		Tools::ObjectType iObjectType;
		Os::Memory::copy(&iObjectType, m_pConstPointer, Tools::m_ObjectTypeArchiveSize);
//		Tools::ObjectType iObjectType =
//			*syd_reinterpret_cast<const Tools::ObjectType*>(m_pConstPointer);
		m_pConstPointer += Tools::m_ObjectTypeArchiveSize;
		iUsedSize += Tools::m_ObjectTypeArchiveSize;

		if (Tools::isLinkedObjectType(iObjectType)) {
			// 次のオブジェクトのObjectIDを得る
			ModSize ulReadSize = Tools::readObjectID(m_pConstPointer, m_iNextObjectID);
			; _SYDNEY_ASSERT(m_iNextObjectID != Tools::m_UndefinedObjectID);
			m_pConstPointer += ulReadSize;
			iUsedSize += ulReadSize;

		} else {
			m_iNextObjectID = Tools::m_UndefinedObjectID;
		}

		// ヘッダーサイズをセットする
		m_iHeaderSize = iUsedSize;
		// 残りサイズを計算する
		m_iRestSize = m_pPage->getAreaSize(pos.m_AreaID) - iUsedSize;

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// 失敗したときはデタッチしてから例外再送
		detachPage(m_pPage, Operation::Read);
		_SYDNEY_RETHROW;
	}
	//成功したときはdetachObjectが呼ばれるまでデタッチしない
}

//	FUNCTION public
//	Record::LinkedObject::detachLinkedObject -- 
//		リンクオブジェクトの１つをデタッチする
//
//	NOTES
//
//	ARGUMENTS
//
//	REUTRN
//		なし
//
//	EXCEPTIONS

void
LinkedObject::
detachLinkedObject()
{
	detachPage(m_pPage, m_eOperation);
}

//	FUNCTION public
//	Record::LinkedObject::attachPage -- 
//		物理ページをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//
//	REUTRN
//		アタッチしたページ
//
//	EXCEPTIONS

PhysicalFile::Page*
LinkedObject::
attachPage(PhysicalFile::PageID iPageID_, Operation::Value eOperation_)
{
	// FreeAreaManagerを通しでアタッチする
	return m_rFreeAreaManager.attachPage(iPageID_, eOperation_);
}

//	FUNCTION public
//	Record::LinkedObject::attachPage -- 
//		物理ページをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//
//	REUTRN
//		アタッチしたページ
//
//	EXCEPTIONS

PhysicalFile::Page*
LinkedObject::
verifyPage(PhysicalFile::PageID iPageID_, Operation::Value eOperation_ ,Admin::Verification::Progress& cProgress_)
{
	// FreeAreaManagerを通しでアタッチする
	return m_rFreeAreaManager.verifyPage(iPageID_, eOperation_ ,cProgress_);
}

//	FUNCTION public
//	Record::LinkedObject::detachPage -- 
//		物理ページをデタッチする
//
//	NOTES
//
//	ARGUMENTS
//
//	REUTRN
//
//	EXCEPTIONS

void
LinkedObject::
detachPage(PhysicalFile::Page*& pPage_, Operation::Value eOperation_)
{
	// FreeAreaManagerを通しでデタッチする
	// WriteでFixしていてNotDirtyでデタッチするなど
	// 特別なUnfixModeが与えられたときはデタッチされる
	m_rFreeAreaManager.detachPage(pPage_, eOperation_, m_bDirty);
}

//	FUNCTION private
//	Record::LinkedObject::hasNext -- 
//		次のオブジェクトのオブジェクトIDがあるかを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	REUTRN
//		次のオブジェクトIDがあればtrue
//
//	EXCEPTIONS

bool
LinkedObject::
hasNext() const
{
	return (m_iNextObjectID < Tools::m_UndefinedObjectID);
}

//
//	Copyright (c) 2001, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
