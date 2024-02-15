// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileInformation.cpp -- レコードファイル管理情報クラス
// 
// Copyright (c) 2000, 2001, 2003, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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
#include "Record/FileInformation.h"

#include "Common/Assert.h"
#include "Common/Message.h"
#include "Common/DateTimeData.h"
#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"
#include "PhysicalFile/File.h"
#include "PhysicalFile/Page.h"
#include "PhysicalFile/File.h"
#include "Record/DirectIterator.h"
#include "Record/File.h"
#include "Record/Module.h"
#include "Record/PhysicalPosition.h"
#include "Record/Tools.h"
#include "Admin/Verification.h"

_SYDNEY_USING
_SYDNEY_RECORD_USING

namespace {
	// DirectFileのヘッダーページの置き換え優先度
	const Buffer::ReplacementPriority::Value
		_ePriority = Buffer::ReplacementPriority::Middle;

	// ヘッダーページに記録するもののサイズを得る
	const Os::Memory::Size _SyncProgressSize = sizeof(FileInformation::SyncProgress::Value);
	const Os::Memory::Size _TimeSize = Common::DateTimeData::getArchiveSize();
	const Os::Memory::Size _ObjectNumSize = Tools::m_ObjectNumArchiveSize;
	const Os::Memory::Size _ObjectIDSize = Common::ObjectIDData::getArchiveSize();
	const Os::Memory::Size _BlockSize =	sizeof(int) +	// VersionNumber
										_TimeSize +		// LastModifiedTime
										_ObjectNumSize +// ObjectNumber
										_ObjectIDSize * 4; // First、Last、FirstFree、FirstFreeVariable
	const Os::Memory::Size _AreaSize = _SyncProgressSize + (_BlockSize << 1);

	const Os::Memory::Offset _SyncProgressValueOffset = 0;
	const Os::Memory::Offset _FirstBlockOffset = _SyncProgressValueOffset + _SyncProgressSize;
	const Os::Memory::Offset _SecondBlockOffset = _FirstBlockOffset + _BlockSize;

	// DoRepairからFixModeを得るための表
	Buffer::Page::FixMode::Value _FixModeTable[2 /* true or false */] =
	{
		// false = Not do repair
		Buffer::Page::FixMode::ReadOnly,
		// true = Do repair
//		Buffer::Page::FixMode::Write | Buffer::Page::FixMode::Discardable
		Buffer::Page::FixMode::Write
	};

} // namespace

//
//	FUNCTION public
//	Record::FileInformation::FileInformation -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//	管理情報が書き込んである物理ファイルの記述子と管理情報があるページの ID
//	を指定してコンストラクトする。
//
//	ARGUMENTS
//	const Trans::Transaction&	cTrans_
//		トランザクション記述子
//	PhysicalFile::File&			cFile_
//		物理ファイル記述子
//	PhysicalFile::PageID		iPageID_
//		管理情報を格納してある物理ページのID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
FileInformation::
FileInformation(
	const Trans::Transaction&			cTrans_,
	PhysicalFile::File&					cFile_,
	PhysicalFile::PageID				iPageID_,
	OpenOperation::Value				eOperation_,
	Admin::Verification::Progress*		pProgress_)
	: m_cTrans(cTrans_),
	  m_eOperation(eOperation_),
	  m_pProgress(pProgress_),
	  m_cFile(cFile_),
	  m_iPageID(iPageID_),
	  m_pPage(0),
	  m_cCurrent(),
	  m_cSave(),
	  m_Status(NotDirty)
{
	; _SYDNEY_ASSERT(m_eOperation != OpenOperation::Verify || m_pProgress);
}

//
//	FUNCTION public
//	Record::FileInformation::FileInformation -- デストラクタ
//
//	NOTES
//	デストラクタ。
//	コンストラクタでアタッチしたリソースを全て解放。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	YET! まだあるかもしれない。
//
FileInformation::
~FileInformation()
{
	if (m_pPage)
		detachPage(true /* force */);
}

#ifdef OBSOLETE
//
//	FUNCTION public
//	Record::FileInformation::getLastUpdateTime -- 最終更新時刻を取得
//
//	NOTES
//	最終更新時刻を取得
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Common::DateTimeData&
//		最終更新時刻オブジェクトへの参照
//
//	EXCEPTIONS
//	なし
//
const Common::DateTimeData&
FileInformation::
getLastModifiedTime() const
{
	return m_cCurrent.m_cLastModifiedTime;
}
#endif //OBSOLETE

//
//	FUNCTION public
//	Record::FileInformation::getInsertedObjectCount -- 挿入されているオブジェクト数を取得
//
//	NOTES
//	挿入されているオブジェクト数を取得
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	
//
//	EXCEPTIONS
//	なし
//
Tools::ObjectNum
FileInformation::
getInsertedObjectNum() const
{
	return m_cCurrent.m_InsertedObjectNum;
}

//
//	FUNCTION public
//	Record::FileInformation::getFirstObjectID -- 先頭オブジェクトのオブジェクトID
//
//	NOTES
//	先頭オブジェクトのオブジェクトID
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	
//
//	EXCEPTIONS
//	なし
//
Tools::ObjectID
FileInformation::
getFirstObjectID() const
{
	return m_cCurrent.m_FirstObjectID;
}

//
//	FUNCTION public
//	Record::FileInformation::getLastObjectID -- 最終オブジェクトのオブジェクトID
//
//	NOTES
//	最終オブジェクトのオブジェクトID
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	
//
//	EXCEPTIONS
//	なし
//
Tools::ObjectID
FileInformation::
getLastObjectID() const
{
	return m_cCurrent.m_LastObjectID;
}

//
//	FUNCTION public
//	Record::FileInformation::getFirstFreeObjectID --
//		空きオブジェクトIDリストの先頭を取得
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	
//
//	EXCEPTIONS
//	なし
//
Tools::ObjectID
FileInformation::
getFirstFreeObjectID() const
{
	return m_cCurrent.m_FirstFreeObjectID;
}

//
//	FUNCTION public
//	Record::FileInformation::getFirstFreeVariableObjectID --
//		空きオブジェクトIDリストの先頭を取得(可変長)
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	
//
//	EXCEPTIONS
//	なし
//
Tools::ObjectID
FileInformation::
getFirstFreeVariableObjectID() const
{
	return m_cCurrent.m_FirstFreeVariableObjectID;
}

#ifdef OBSOLETE
//
//	FUNCTION public
//	Record::FileInformation::getAreaSize -- 管理情報をファイルに書き込むのに必要なバイト数を求める
//
//	NOTES
//	管理情報をファイルに書き込むのに必要なバイト数を求める
//	(メンバ変数の総数や型が変化したら修正すること)
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
// static
ModSize
FileInformation::
getAreaSize()
{
	return _AreaSize;
}
#endif //OBSOLETE

//
// マニピュレータ
//

#ifdef OBSOLETE
//
//	FUNCTION public
//	Record::FileInformation::setInsertedObjectCount -- 挿入されているオブジェクト数を設定
//
//	NOTES
//	挿入されているオブジェクト数を設定
//
//	ARGUMENTS
//	const ModInt64 ulObjectCount_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileInformation::
setInsertedObjectNum(
	const Tools::ObjectNum	InsertedObjectNum_)
{
	m_cCurrent.m_InsertedObjectNum = InsertedObjectNum_;
	m_cCurrent.touch();
}
#endif OBSOLETE

#ifdef OBSOLETE
//
//	FUNCTION public
//	Record::FileInformation::setFirstObjectID -- 先頭オブジェクトのオブジェクトID を設定
//
//	NOTES
//	先頭オブジェクトのオブジェクトIDを設定
//
//	ARGUMENTS
//	const ModUInt64 cObjectID_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileInformation::
setFirstObjectID(const Tools::ObjectID	FirstObjectID_)
{
	m_cCurrent.m_FirstObjectID = FirstObjectID_;
	m_cCurrent.touch();
}
#endif //OBSOLETE

#ifdef OBSOLETE
//
//	FUNCTION public
//	Record::FileInformation::setLastObjectID -- 最終オブジェクトのオブジェクトIDを設定
//
//	NOTES
//	最終オブジェクトのオブジェクトIDを設定
//
//	ARGUMENTS
//	const ModUInt64 cObjectID_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileInformation::
setLastObjectID(const Tools::ObjectID	LastObjectID_)
{
	m_cCurrent.m_LastObjectID = LastObjectID_;
	m_cCurrent.touch();
}
#endif //OBSOLETE

//
//	FUNCTION public
//	Record::FileInformation::setFirstFreeObjectID --
//		空きオブジェクトIDリストの先頭を設定
//
//	NOTES
//
//	ARGUMENTS
//	Tools::ObjectID FirstFreeObjectID_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileInformation::
setFirstFreeObjectID(const Tools::ObjectID	FirstFreeObjectID_)
{
	m_cCurrent.m_FirstFreeObjectID = FirstFreeObjectID_;
	m_cCurrent.touch();
}

//
//	FUNCTION public
//	Record::FileInformation::setFirstFreeVariableObjectID --
//		空きオブジェクトIDリストの先頭を設定(可変長)
//
//	NOTES
//
//	ARGUMENTS
//	Tools::ObjectID FirstFreeVariableObjectID_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileInformation::
setFirstFreeVariableObjectID(const Tools::ObjectID	FirstFreeVariableObjectID_)
{
	m_cCurrent.m_FirstFreeVariableObjectID = FirstFreeVariableObjectID_;
	m_cCurrent.touch();
}

//	FUNCTION public
//	Record::FileInformation::validate --
//		ファイル上の管理情報を必要なら書き換える
//
//	NOTES
//
//	ARGUMENTS
//		Record::DirectIterator& cIterator_
//			操作を行ったオブジェクトを指すイテレーター
//		Record::FileInformation::ValidateOperation::Value eOperation_
//			操作の種類
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
FileInformation::
validate(DirectIterator& cIterator_, ValidateOperation::Value eOperation_)
{
	switch (eOperation_) {
	case ValidateOperation::Read:
		// 何もしない
		break;
	case ValidateOperation::Insert:
	{
		// オブジェクト総数が増える
		++m_cCurrent.m_InsertedObjectNum;

		Tools::ObjectID iObjectID = cIterator_.getObjectID();

		// 最大、最小のオブジェクトIDが変わるか調べる
		if (m_cCurrent.m_FirstObjectID == Tools::m_UndefinedObjectID
			|| iObjectID < m_cCurrent.m_FirstObjectID) {
			m_cCurrent.m_FirstObjectID = iObjectID;
		}
		if (m_cCurrent.m_LastObjectID == Tools::m_UndefinedObjectID
			|| iObjectID > m_cCurrent.m_LastObjectID) {
			m_cCurrent.m_LastObjectID = iObjectID;
		}
		m_cCurrent.touch();
		break;
	}
	case ValidateOperation::Update:
	{
//		// 更新時刻を書き換えるためにDirtyにする
//		m_cCurrent.touch();
		break;
	}
	case ValidateOperation::Expunge:
	{
		// オブジェクト総数が減る
		--m_cCurrent.m_InsertedObjectNum;

		Tools::ObjectID iObjectID = cIterator_.getObjectID();

		// 空きオブジェクトIDリストの先頭を書き換える
		m_cCurrent.m_FirstFreeObjectID = iObjectID;

		if (m_cCurrent.m_InsertedObjectNum == 0) {
			m_cCurrent.m_FirstObjectID = m_cCurrent.m_LastObjectID = Tools::m_UndefinedObjectID;

		} else {
			// 最大、最小のオブジェクトIDが変わるか調べる
			if (iObjectID == m_cCurrent.m_FirstObjectID) {
				// 次のページIDを得る
				m_cCurrent.m_FirstObjectID = cIterator_.getNextObjectID();
			}
			if (iObjectID == m_cCurrent.m_LastObjectID) {
				// 前のページIDを得る
				m_cCurrent.m_LastObjectID = cIterator_.getPrevObjectID();
			}
		}

		m_cCurrent.touch();
	}
	default:
		break;
	}
}

//
//	FUNCTION public
//	Record::FileInformation::reload -- ファイル上の管理情報を再読み込み
//
//	NOTES
//	ファイル上の管理情報のうち正しい方をメンバ変数に読み込む。
//	doRepair_ が true の場合は修繕も行なう。
//
//	ARGUMENTS
//	const bool doRepair_
//		true の場合は修繕も行なう。 false ならば修繕しない。
//	bool bKeepAttach_ = false
//		trueの場合アタッチしたまま返る
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	ページからの読み込み、または修繕に失敗した時などに例外が発生。
//
void
FileInformation::
reload(const bool DoRepair_, bool bKeepAttach_)
{
	attachPage(_FixModeTable[DoRepair_]);

	try {
		SyncProgress::Value	progress = readSyncProgress();

		// 正しい方の管理情報を読み込む
		switch (progress) {
		case SyncProgress::NotWriting:
		case SyncProgress::WritingSecondBlock:
		{
			// 第一ブロックは正しいはずである
			readFirstBlock();

			if (DoRepair_ && progress == SyncProgress::WritingSecondBlock) // 修繕する
			{
				// 修繕する(データメンバを第2ブロックに書き込む)
				writeSecondBlock();
			}
			break;
		}
		case SyncProgress::WritingFirstBlock:
		{
			// 第二ブロックは正しいはずである
			readSecondBlock();

			if (DoRepair_)
			{
				// 修繕する(データメンバを第1ブロックに書き込む)
				writeFirstBlock();
			}
		}
		default:
			; _SYDNEY_ASSERT(false);
			break;
		}

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		detachPage();
		_SYDNEY_RETHROW;
	}
	if (!bKeepAttach_) {
		detachPage();
	}
}

//
//	FUNCTION public
//	Record::FileInformation::sync -- データメンバをファイルに書き込む
//
//	NOTES
//	データメンバをファイルに書き込む
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
FileInformation::
sync()
{
	// データに変更がなければ何もする必要はない
	if (!m_cCurrent.m_bDirty) {
		return;
	}

	//
	// エラーが起きた場合は例外を再送してエラー処理をしてもらう
	// 必要がある処理
	//

	// メンバ変数「最終更新時刻」に現在の時刻をセット
//	m_cCurrent.m_cLastModifiedTime.setCurrent();

	if (!m_pPage) {
		attachPage(_FixModeTable[true]);
	}

	try {
		// 進行状況を書いてからブロックを書く

		// 第一ブロック
		writeSyncProgress(SyncProgress::WritingFirstBlock);
		writeFirstBlock();

		// 第二ブロック
		writeSyncProgress(SyncProgress::WritingSecondBlock);
		writeSecondBlock();

		// 終了
		writeSyncProgress(SyncProgress::NotWriting);

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		detachPage();
		_SYDNEY_RETHROW;
	}

	detachPage();
	m_Status = Sync;
}

//
//	FUNCTION public
//	Record::FileInformation::recover -- sync後にエラーが起きたら内容を書き戻す
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
//	なし
//
void
FileInformation::
recover()
{
	// Sync前だったら何もする必要はない
	if (m_Status != Sync) {
		return;
	}

	// 保存してあったデータに書き戻す
	m_cCurrent = m_cSave;

	//
	// エラーが起きた場合は例外を再送してエラー処理をしてもらう
	// 必要がある処理
	//

	// メンバ変数「最終更新時刻」に現在の時刻をセット
//	m_cCurrent.m_cLastModifiedTime.setCurrent();

	if (!m_pPage) {
		attachPage(_FixModeTable[true]);
	}

	try {
		// 進行状況を書いてからブロックを書く

		// 第一ブロック
		writeSyncProgress(SyncProgress::WritingFirstBlock);
		writeFirstBlock();

		// 第二ブロック
		writeSyncProgress(SyncProgress::WritingSecondBlock);
		writeSecondBlock();

		// 終了
		writeSyncProgress(SyncProgress::NotWriting);

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		detachPage();
		_SYDNEY_RETHROW;
	}

	detachPage();
}

//
//	FUNCTION public
//	Record::FileInformation::touch -- dirty状態にする
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
//	なし
//
void
FileInformation::
touch()
{
	m_cCurrent.touch();
}

//
//	FUNCTION private
//	Record::FileInformation::readSyncProgress -- 進行状況の領域にアクセス
//
//	NOTES
//  進行状況の領域にアクセス
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	SyncProgress::Value eProgress_
//
//	EXCEPTIONS
//	なし
//
FileInformation::SyncProgress::Value
FileInformation::
readSyncProgress()
{
	; _SYDNEY_ASSERT(m_pConstPageTop);
	SyncProgress::Value value;
	Os::Memory::copy(&value, m_pConstPageTop, sizeof(SyncProgress::Value));
	return value;
//	return *syd_reinterpret_cast<const SyncProgress::Value*>(m_pConstPageTop);
}

//
//	FUNCTION private
//	Record::FileInformation::writeSyncProgress -- 進行状況の領域にアクセス
//
//	NOTES
//  進行状況の領域にアクセス
//
//	ARGUMENTS
//	SyncProgress::Value			eProgress_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileInformation::
writeSyncProgress(SyncProgress::Value Progress_)
{
	; _SYDNEY_ASSERT(m_pPageTop);
	Os::Memory::copy(m_pPageTop, &Progress_, sizeof(SyncProgress::Value));
//	*syd_reinterpret_cast<SyncProgress::Value*>(m_pPageTop) = Progress_;
}

//
//	FUNCTION private
//	Record::FileInformation::readFirstBlock -- 第1ブロックのデータメンバを読む
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
//	なし
//
void
FileInformation::
readFirstBlock()
{
	readBlock(m_pConstPageTop + _FirstBlockOffset);
}

//
//	FUNCTION private
//	Record::FileInformation::writeFirstBlock -- 第1ブロックにデータメンバを書く
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
//	なし
//
void
FileInformation::
writeFirstBlock()
{
	writeBlock(m_pPageTop + _FirstBlockOffset);
}

//
//	FUNCTION private
//	Record::FileInformation::readSecondBlock -- 第1ブロックのデータメンバを読む
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
//	なし
//
void
FileInformation::
readSecondBlock()
{
	readBlock(m_pConstPageTop + _SecondBlockOffset);
}

//
//	FUNCTION private
//	Record::FileInformation::writeSecondBlock -- 第1ブロックにデータメンバを書く
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
//	なし
//
void
FileInformation::
writeSecondBlock()
{
	writeBlock(m_pPageTop + _SecondBlockOffset);
}

//
//	FUNCTION private
//	Record::FileInformation::readBlock -- ブロックを読み込む
//
//	NOTES
//
//	ARGUMENTS
//	const char* pPointer_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileInformation::
readBlock(const char* pPointer_)
{
	const char*	pPointer = pPointer_;

	// ファイルのバージョン
	int iVersion;
	Os::Memory::copy(&iVersion, pPointer, sizeof(int));
//	int iVersion = *syd_reinterpret_cast<const int*>(pPointer);
	if (iVersion < 0 || iVersion >= Vers::VersionNum) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	m_cCurrent.m_eFileVersion = static_cast<Vers::Value>(iVersion);
	pPointer += sizeof(int);
	
	// 最終更新時刻
//	pPointer = FileCommon::DataManager::readFixedCommonData(m_cCurrent.m_cLastModifiedTime, pPointer);
	pPointer += _TimeSize;

	// 挿入されているオブジェクト数
	Os::Memory::copy(&m_cCurrent.m_InsertedObjectNum, pPointer, Tools::m_ObjectNumArchiveSize);
//	m_cCurrent.m_InsertedObjectNum =
//		*syd_reinterpret_cast<const Tools::ObjectNum*>(pPointer);
	pPointer += Tools::m_ObjectNumArchiveSize;

	// 先頭オブジェクトのオブジェクト識別子
	pPointer +=
		Tools::readObjectID(pPointer, m_cCurrent.m_FirstObjectID);

	// 最終オブジェクトのオブジェクト識別子
	pPointer +=
		Tools::readObjectID(pPointer, m_cCurrent.m_LastObjectID);

	// 先頭未使用オブジェクトのオブジェクト識別子
	pPointer +=
		Tools::readObjectID(pPointer, m_cCurrent.m_FirstFreeObjectID);

	// 先頭未使用可変長オブジェクトのオブジェクト識別子
	pPointer +=
		Tools::readObjectID(pPointer, m_cCurrent.m_FirstFreeVariableObjectID);

	// 読み終わったときのデータを保存しておく
	m_cSave = m_cCurrent;
	m_cSave.m_bDirty = m_cCurrent.m_bDirty = false;
}

//
//	FUNCTION private
//	Record::FileInformation::writeBlock -- ブロックを書き込む
//
//	NOTES
//
//	ARGUMENTS
//	char* pPointer_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileInformation::
writeBlock(char* pPointer_)
{
	char*	pPointer = pPointer_;

	// ファイルバージョン
	int iFileVersion = static_cast<int>(m_cCurrent.m_eFileVersion);
	Os::Memory::copy(pPointer, &iFileVersion, sizeof(int));
//	*syd_reinterpret_cast<int*>(pPointer) = static_cast<int>(m_cCurrent.m_eFileVersion);
	pPointer += sizeof(int);

	// 最終更新時刻
//	pPointer = FileCommon::DataManager::writeFixedCommonData(m_cCurrent.m_cLastModifiedTime, pPointer);
	pPointer += _TimeSize;
	; _SYDNEY_ASSERT(pPointer == pPointer_ + sizeof(int) + _TimeSize);

	// 挿入されているオブジェクト数
	Os::Memory::copy(pPointer, &m_cCurrent.m_InsertedObjectNum, Tools::m_ObjectNumArchiveSize);
//	*syd_reinterpret_cast<Tools::ObjectNum*>(pPointer) = m_cCurrent.m_InsertedObjectNum;
	pPointer += Tools::m_ObjectNumArchiveSize;
	; _SYDNEY_ASSERT(pPointer == pPointer_ + sizeof(int) + _TimeSize + _ObjectNumSize);

	// 先頭オブジェクトのオブジェクト識別子
	pPointer += Tools::writeObjectID(pPointer, m_cCurrent.m_FirstObjectID);
	; _SYDNEY_ASSERT(pPointer == pPointer_ + sizeof(int) + _TimeSize + _ObjectNumSize + _ObjectIDSize);

	// 最終オブジェクトのオブジェクト識別子
	pPointer += Tools::writeObjectID(pPointer, m_cCurrent.m_LastObjectID);
	; _SYDNEY_ASSERT(pPointer == pPointer_ + sizeof(int) + _TimeSize + _ObjectNumSize + _ObjectIDSize * 2);

	// 先頭未使用オブジェクトのオブジェクト識別子
	pPointer += Tools::writeObjectID(pPointer, m_cCurrent.m_FirstFreeObjectID);
	; _SYDNEY_ASSERT(pPointer == pPointer_ + sizeof(int) + _TimeSize + _ObjectNumSize + _ObjectIDSize * 3);

	// 先頭未使用可変長オブジェクトのオブジェクト識別子
	pPointer += Tools::writeObjectID(pPointer, m_cCurrent.m_FirstFreeVariableObjectID);
	; _SYDNEY_ASSERT(pPointer == pPointer_ + sizeof(int) + _TimeSize + _ObjectNumSize + _ObjectIDSize * 4);
	; _SYDNEY_ASSERT(pPointer == pPointer_ + _BlockSize);

	// ページが変更された
	m_pPage->dirty();
}

//	FUNCTION public
//	Record::FileInformation::attachPage -- ページをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Page::FixMode::Value eFixMode_
//			フィックスモード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
FileInformation::
attachPage(Buffer::Page::FixMode::Value eFixMode_)
{
	if (!m_pPage) {
		switch (m_eOperation) {
		case OpenOperation::Read:
		case OpenOperation::Update:
		case OpenOperation::Batch:
		{
			m_pPage = m_cFile.attachPage(m_cTrans,
										 m_iPageID,
										 eFixMode_,
										 _ePriority);
			break;
		}
		case OpenOperation::Verify:
		{
			; _SYDNEY_ASSERT(m_pProgress);
			m_pPage = m_cFile.verifyPage(m_cTrans,
										 m_iPageID,
										 eFixMode_,
										 *m_pProgress);
			break;
		}
		default:
			; _SYDNEY_ASSERT(false);
			break;
		}
	}

	if (eFixMode_ == Buffer::Page::FixMode::ReadOnly) {
		m_pConstPageTop = (*m_pPage).operator const char*();
	} else {
		m_pPageTop = (*m_pPage).operator char*();
	}
}

//	FUNCTION public
//	Record::FileInformation::detachPage -- ページをデタッチする
//
//	NOTES
//
//	ARGUMENTS
//	bool bForce_ = false
//		true .. detach anyway
//		false.. skip detache in batch mode
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
FileInformation::
detachPage(bool bForce_ /* = false */)
{
	; _SYDNEY_ASSERT(m_pPage);

	if (bForce_
		|| (m_eOperation != OpenOperation::Batch)) {
		m_cFile.detachPage(m_pPage);
		m_pPage = 0;
		m_pPageTop = 0;
		; _SYDNEY_ASSERT(!m_pConstPageTop);	// unionなので同時に0になる

		m_Status = NotDirty;
	}
}

//
//	Copyright (c) 2000, 2001, 2003, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
