// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileBase.cpp -- DirectFileとVariableFileで共通する実装
// 
// Copyright (c) 2002, 2003, 2005, 2006, 2023 Ricoh Company, Ltd.
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

#include "Record/FileBase.h"
#include "Record/MetaData.h"
#include "Record/OpenParameter.h"

#include "Checkpoint/Database.h"
#include "Exception/FakeError.h"
#include "Exception/IllegalFileAccess.h"
#include "FileCommon/AutoAttach.h"
#include "FileCommon/OpenMode.h"
#include "PhysicalFile/Manager.h"

_SYDNEY_USING
_SYDNEY_RECORD_USING

//	FUNCTION public
//	Record::FileBase::FileBase --
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

FileBase::
FileBase(const Trans::Transaction& cTrans_,
		const MetaData& cMetaData_)
	: m_cTrans(cTrans_), m_pFile(0), m_cMetaData(cMetaData_),
	  m_pOpenParam(0)
{ }

//	FUNCTION public
//	Record::FileBase::~FileBase --
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

FileBase::
~FileBase()
{
	if (m_pFile)
		detachFile();
}

//	FUNCTION public
//	Record::FileBase::getSize --
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ファイルサイズ
//
//	EXCEPTIONS

ModUInt64
FileBase::
getSize() const
{
	if (isMounted(m_cTrans)) {
		; _SYDNEY_ASSERT(m_pFile);
		return m_pFile->getSize();
	}
	return 0;
}

//	FUNCTION public
//	Record::FileBase::create --
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
FileBase::
create()
{
	m_cStorageStrategy.m_VersionFileInfo._mounted = true;
}

//	FUNCTION public
//	Record::FileBase::destroy --
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
FileBase::
destroy()
{
	enum {
		None,
		File
	} eStatus = None;

	AutoAttachFile file(*this);
	; _SYDNEY_ASSERT(m_pFile);

	// マウントの有無や実体の存在の有無を確認せずに
	// とにかく削除する
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	try {
		// 実体である物理ファイルを破棄する

		m_pFile->destroy(m_cTrans);
		eStatus = File;

		// サブディレクトリーを破棄する

		ModOsDriver::File::rmAll(getPath(), ModTrue);

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		try {
			// 再び試みる

			switch (eStatus) {
			case None:
			default:
				m_pFile->destroy(m_cTrans);
				// thru
			case File:
				ModOsDriver::File::rmAll(getPath(), ModTrue);
			}
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{

			// 再度の試みにも失敗したので、利用不可にする

			Checkpoint::Database::setAvailability(
				m_cMetaData.getLockName(), false);
		}
		_SYDNEY_RETHROW;
	}

	m_cStorageStrategy.m_VersionFileInfo._mounted = false;
}

//	FUNCTION public
//	Record::FileBase::isAccessible --
//		構成する OS ファイルが存在するか調べる
//
//	NOTES
//
//	ARGUMENTS
//		bool			force
//			true
//				構成する OS ファイルの存在を実際に調べる
//			false または指定されないとき
//				構成する OS ファイルの存在を必要があれば調べる
//
//	RETURN
//		true
//			存在する
//		false
//			存在しない
//
//	EXCEPTIONS

bool
FileBase::
isAccessible(bool force) const
{
	return (isAttached()) ?
		m_pFile->isAccessible(force) :
		FileCommon::AutoPhysicalFile(
			m_cStorageStrategy, m_cBufferingStrategy,
			m_cMetaData.getLockName())->isAccessible(force);
}

//	FUNCTION public
//	Record::FileBase::isMounted -- マウントされているか調べる
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
FileBase::isMounted(const Trans::Transaction& trans) const
{
	return (isAttached()) ?
		m_pFile->isMounted(trans) :
		FileCommon::AutoPhysicalFile(
			m_cStorageStrategy, m_cBufferingStrategy,
			m_cMetaData.getLockName())->isMounted(trans);
}

//	FUNCTION public
//	Record::FileBase::mount --
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
FileBase::
mount()
{
	AutoAttachFile file(*this);

	if (!isMounted(m_cTrans)) {

		// マウントされていなければ、マウントしてみる

		; _SYDNEY_ASSERT(m_pFile);
		m_pFile->mount(m_cTrans);

		// マウントされたことを記録する

		m_cStorageStrategy.m_VersionFileInfo._mounted = true;
	}
}

//	FUNCTION public
//	Record::FileBase::unmount --
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
FileBase::
unmount()
{
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかくアンマウントする
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	AutoAttachFile file(*this);

	; _SYDNEY_ASSERT(m_pFile);
	m_pFile->unmount(m_cTrans);

	m_cStorageStrategy.m_VersionFileInfo._mounted = false;
}

//	FUNCTION public
//	Record::FileBase::flush --
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
FileBase::
flush()
{
	AutoAttachFile file(*this);

	if (isMounted(m_cTrans)) {
		; _SYDNEY_ASSERT(m_pFile);
		m_pFile->flush(m_cTrans);
	}
}

//	FUNCTION public
//	Record::FileBase::startBackup --
//
//	NOTES
//
//	ARGUMENTS
//		const bool bRestorable_
//			trueの場合そのときに見ている版に戻せるようにバックアップする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FileBase::
startBackup(const bool bRestorable_)
{
	AutoAttachFile file(*this);

	if (isMounted(m_cTrans)) {
		; _SYDNEY_ASSERT(m_pFile);
		m_pFile->startBackup(m_cTrans, bRestorable_);
	}
}

//	FUNCTION public
//	Record::FileBase::endBackup --
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
FileBase::
endBackup()
{
	AutoAttachFile file(*this);

	if (isMounted(m_cTrans)) {
		; _SYDNEY_ASSERT(m_pFile);
		m_pFile->endBackup(m_cTrans);
	}
}

//	FUNCTION public
//	Record::FileBase::recover --
//
//	NOTES
//
//	ARGUMENTS
//		const Trans::TimeStamp& cPoint_
//			障害回復するポイント	
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FileBase::
recover(const Trans::TimeStamp& cPoint_)
{
	AutoAttachFile file(*this);

	if (isMounted(m_cTrans)) {
		; _SYDNEY_ASSERT(m_pFile);
		m_pFile->recover(m_cTrans, cPoint_);

		if (!isAccessible())

			// リカバリの結果
			// 実体である OS ファイルが存在しなくなったので、
			// サブディレクトリを削除する

			ModOsDriver::File::rmAll(getPath(), ModTrue);
	}
}

//	FUNCTION public
//	Record::FileBase::restore --
//
//	NOTES
//
//	ARGUMENTS
//		const Trans::TimeStamp& cPoint_
//			障害回復するポイント	
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FileBase::
restore(const Trans::TimeStamp& cPoint_)
{
	AutoAttachFile file(*this);

	if (isMounted(m_cTrans)) {
		; _SYDNEY_ASSERT(m_pFile);
		m_pFile->restore(m_cTrans, cPoint_);
	}
}

//	FUNCTION public
//	Record::FileBase::move --
//
//	NOTES
//
//	ARGUMENTS
//	 bool bUndo_ = false
//		trueの場合エラー処理のためのmoveなのでエラー処理しない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FileBase::
move(bool bUndo_)
{
	// 新しいパス名を作って、移動する
	//
	//【注意】	move の中で getPath の返すリファレンスを変更するので、
	//			移動前のパス名はコピーしてから、move を呼び出す

	Os::Path oldPath(getPath());
	Os::Path newPath(m_cMetaData.getDirectoryPath());
	newPath.addPart(getPathPart());

	move(oldPath, newPath, bUndo_);
}

//	FUNCTION public
//	Record::FileBase::attachFile --
//
//	NOTES
//
//	ARGUMENTS
//		const OpenParameter* pOpenParam_ = 0
//			オープン時のパラメーター
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FileBase::
attachFile(const OpenParameter* pOpenParam_)
{
	; _SYDNEY_ASSERT(!m_pFile);
	m_pFile = PhysicalFile::Manager::attachFile( m_cStorageStrategy, m_cBufferingStrategy ,m_cMetaData.getLockName() );
	; _SYDNEY_ASSERT(m_pFile);

	m_pOpenParam = pOpenParam_;
}

//	FUNCTION public
//	Record::FileBase::detachFile --
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
FileBase::
detachFile()
{
	if (m_pFile) {
		PhysicalFile::Manager::detachFile(m_pFile);
		; _SYDNEY_ASSERT(!m_pFile);

		m_pOpenParam = 0;
	}
}

//	FUNCTION public
//	Record::FileBase::isAttached --
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		アタッチされていればtrue
//
//	EXCEPTIONS

bool
FileBase::
isAttached() const
{
	return (m_pFile != 0);
}

//	FUNCTION public
//	Record::FileBase::getPath --
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ファイルのパス名
//
//	EXCEPTIONS

const ModUnicodeString&
FileBase::
getPath() const
{
	return m_cStorageStrategy.m_VersionFileInfo._path._masterData;
}

// Batch mode?
bool
FileBase::
isBatch()
{
	return m_pOpenParam && m_pOpenParam->m_iOpenMode == FileCommon::OpenMode::Batch;
}

//////////////////////////////////
// 物理ファイルのためのメソッド //
//////////////////////////////////

//	FUNCTION public
//	Record::FileBase::sync -- 同期をとる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			同期を取るトランザクションのトランザクション記述子
//		bool&				incomplete
//			true
//				今回の同期処理でファイルを持つ
//				オブジェクトの一部に処理し残しがある
//			false
//				今回の同期処理でファイルを持つ
//				オブジェクトを完全に処理してきている
//
//				同期処理の結果、ファイルを処理し残したかを設定する
//		bool&				modified
//			true
//				今回の同期処理でファイルを持つ
//				オブジェクトの一部が既に更新されている
//			false
//				今回の同期処理でファイルを持つ
//				オブジェクトはまだ更新されていない
//
//				同期処理の結果、ファイルが更新されたかを設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FileBase::sync(
	const Trans::Transaction& trans, bool& incomplete, bool& modified)
{
	if (isMounted(m_cTrans))
		if (isAttached())
			m_pFile->sync(trans, incomplete, modified);
		else
			FileCommon::AutoPhysicalFile(
				m_cStorageStrategy, m_cBufferingStrategy,
				m_cMetaData.getLockName())->sync(trans, incomplete, modified);
}

//////////////////
// 内部メソッド //
//////////////////

//	FUNCTION private
//	Record::FileBase::move -- move の下請け
//
//	NOTES
//
//	ARGUMENTS
//   ModUnicodeString& cstrOldPath_
//		移動前のパス名
//   ModUnicodeString& cstrNewPath_
//		移動先のパス名
//	 bool bUndo_ = false
//		trueの場合エラー処理のためのmoveなのでエラー処理しない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FileBase::
move(ModUnicodeString& cstrOldPath_, ModUnicodeString& cstrNewPath_,
	 bool bUndo_)
{
	//【注意】	新しいパス名を設定する前にアタッチしておく必要がある

	AutoAttachFile file(*this);

	bool bMoved = false;
	const bool accessible = isAccessible();

	// 新しいパス名を設定する

	m_cStorageStrategy.m_VersionFileInfo._path._masterData = cstrNewPath_;
	if (!m_cMetaData.isTemporary())	{
		m_cStorageStrategy.m_VersionFileInfo._path._versionLog = cstrNewPath_;
		m_cStorageStrategy.m_VersionFileInfo._path._syncLog = cstrNewPath_;
	}

	try {
		// ファイルを移動する

		m_pFile->move(m_cTrans, m_cStorageStrategy.m_VersionFileInfo._path);
		bMoved = true;

		if (accessible)

			// 古いサブディレクトリーを破棄する

			ModOsDriver::File::rmAll(cstrOldPath_);

		_SYDNEY_FAKE_ERROR("Record::FileBase::move",Exception::IllegalFileAccess(moduleName, srcFile, __LINE__));

	} catch (...) {
		if (!bUndo_)

			// 元に戻す

			try {
				if (bMoved)
					move(cstrNewPath_, cstrOldPath_, true);
				else if (accessible)

					// 実体があるときのみ、新しいサブディレクトリを破棄する
					//
					//【注意】	サブディレクトリは
					//			実体である物理ファイルの移動時に
					//			必要に応じて生成されるが、
					//			エラー時には削除されないので、
					//			この関数で削除する必要がある

					ModOsDriver::File::rmAll(cstrNewPath_);
			} catch (...) {

				// 元に戻せなかったので、利用不可にする

				Checkpoint::Database::setAvailability(
					m_cMetaData.getLockName(), false);
			}
		_SYDNEY_RETHROW;
	}
}

//
//	Copyright (c) 2002, 2003, 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
