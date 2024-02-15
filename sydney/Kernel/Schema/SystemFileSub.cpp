// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SystemFileSub.cpp -- システム表を構成するファイルの下請け関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Schema/SystemFileSub.h"
#include "Schema/FakeError.h"
#include "Schema/FileID.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/Parameter.h"
#include "Schema/PathParts.h"
#include "Schema/SystemFile.h"
#include "Schema/Utility.h"

#include "Common/Assert.h"
#include "Common/DataInstance.h"
#include "Common/StringArrayData.h"

#include "Exception/Object.h"
#include "Exception/RecoveryFailed.h"
#include "Exception/Unexpected.h"

#include "LogicalFile/File.h"
#include "LogicalFile/FileDriver.h"
#include "LogicalFile/FileDriverManager.h"
#include "LogicalFile/FileDriverTable.h"
#include "LogicalFile/FileID.h"
#include "LogicalFile/OpenOption.h"

#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"
#include "Os/SysConf.h"

#include "Record/FileOption.h"
#include "FileCommon/FileOption.h"
#include "FileCommon/OpenOption.h"

#include "Trans/Transaction.h"

#include "ModAutoPointer.h"
#include "ModError.h"
#include "ModException.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace {

////////////////////////////
// 索引の種類ごとのPrefix //
////////////////////////////

const char* _pszIndexPrefix[SystemTable::IndexFile::Category::ValueNum] =
{
	"BTR_",										// B+木
	"VCT_"										// ベクター
};

////////////////////////////
// FileIDをセットする関数 //
////////////////////////////

// レコードファイルのファイルIDをセットする

void
_SetRecordFileID(SystemTable::RecordFile& cRecord_,
				 const Os::Path& cPath_,
				 SystemTable::Attribute::Value eAttr_,
				 bool bMounted_)
{
	// 格納エリア
	cRecord_.setAreaPath(cPath_);

	// ファイル属性
	cRecord_.setAttribute(eAttr_);

	// 常に一時ファイルを使うように設定されていたら
	// システム表のファイルもそのようにセットする

	if (Manager::Configuration::isAlwaysTemporary())

		// 一時ファイルかどうかをセットする

		cRecord_.getFileID().setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(FileCommon::FileOption::Temporary::Key),
										true);

	// バージョン
	// 本当はcreate時に入るのだが、Schemaは毎回FileIDを作り直しているので
	// 0をハードコーディングで入れる必要がある。
	// いずれFileIDを保存する物理ファイルを作らなければならない…。

	cRecord_.getFileID().setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(FileCommon::FileOption::Version::Key),
									0);

	// ページ中のオブジェクト数の最小値
	// 本当はcreate時に入るのだが、Schemaは毎回FileIDを作り直しているので
	// 4をハードコーディングで入れる必要がある。
	// いずれFileIDを保存する物理ファイルを作らなければならない…。

	cRecord_.getFileID().setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(Record::FileOption::MinimumObjectPerPage::Key),
									4);

	// Mount フラグ
	// 引数で指定される
	cRecord_.getFileID().setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(FileCommon::FileOption::Mounted::Key),
									bMounted_);

	// 【注意】	以下はデフォルトのまま

//	// ファイルの最大サイズ (KB 単位)
//
//	cFileID_.setInteger(
//		_SYDNEY_SCHEMA_PARAMETER_KEY(FileCommon::FileOption::FileSizeMax::Key), /**/);
//
	// ページサイズ (KB 単位)

	cRecord_.getFileID().setInteger(
	   _SYDNEY_SCHEMA_PARAMETER_KEY(FileCommon::FileOption::PageSize::Key), Os::SysConf::PageSize::get() >> 10);

//	// 1 データページあたりの領域使用率
//
//	cFileID_.setInteger(
//		_SYDNEY_SCHEMA_PARAMETER_KEY(FileCommon::FileOption::UseRatePerPage::Key), /**/);
//
//	// コンパクション実行領域使用率
//
//	cFileID_.setInteger(
//		_SYDNEY_SCHEMA_PARAMETER_KEY(FileCommon::FileOption::CompactionRate::Key), /**/);
}

// B+木ファイルのファイルIDをセットする

void
_SetBtreeFileID(SystemTable::IndexFile& cIndex_,
				const Os::Path& cPath_,
				SystemTable::Attribute::Value eAttr_,
				bool bMounted_)
{
	// 常に一時ファイルを使うように設定されていたら
	// システム表のファイルもそのようにセットする

	if (Manager::Configuration::isAlwaysTemporary())

		// 一時ファイルかどうかをセットする

		cIndex_.getFileID().setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(FileCommon::FileOption::Temporary::Key),
									   true);

	// バージョン
	// 本当はcreate時に入るのだが、Schemaは毎回作り直しているので
	// 0をハードコーディングで入れる必要がある。
	// いずれFileIDを保存する物理ファイルを作らなければならない…。

	cIndex_.getFileID().setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(FileCommon::FileOption::Version::Key),
#ifdef SYD_SCHEMA_BTREE2
								   2);
#else
								   0);
#endif

	// Mount フラグ
	// 引数で指定される
	cIndex_.getFileID().setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(FileCommon::FileOption::Mounted::Key),
								   bMounted_);

	// キーについてユニーク

	FileID::setMode(cIndex_.getFileID(),
					_SYDNEY_SCHEMA_PARAMETER_KEY(FileCommon::FileOption::Unique::Key),
					_SYDNEY_SCHEMA_PARAMETER_VALUE(FileCommon::FileOption::Unique::KeyField));

	// フィールド情報をセットする
	//		Key1:	キーの型
	//		Key2:	バリューの型

	int i = 0;
	int k = 0;

	// OID

	cIndex_.getFileID().setInteger(_SYDNEY_SCHEMA_FORMAT_KEY(FileCommon::FileOption::FieldType::Key, i),
								   LogicalFile::ObjectID().getType());

	i++;

	// コンストラクト時に指定されたキーのデータ型(Key)

	cIndex_.getFileID().setInteger(_SYDNEY_SCHEMA_FORMAT_KEY(FileCommon::FileOption::FieldType::Key, i),
								   cIndex_.getKeyType());
	i++;
	k++;

	// コンストラクト時に指定されたバリューのデータ型(BtreeにはKeyとして登録)

	cIndex_.getFileID().setInteger(_SYDNEY_SCHEMA_FORMAT_KEY(FileCommon::FileOption::FieldType::Key, i),
								   cIndex_.getValueType());
	i++;
	k++;

	// フィールドの数
	cIndex_.getFileID().setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(FileCommon::FileOption::FieldNumber::Key),
								   i);

	// キーの数
	cIndex_.getFileID().setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(FileCommon::FileOption::KeyFieldNumber::Key),
								   k);

	// 【注意】	以下はデフォルトのまま

//	// ファイルの最大サイズ (KB 単位)
//
//	cIndex_.getFileID().setInteger(
//		_SYDNEY_SCHEMA_PARAMETER_KEY(Btree::FileOption::FileSizeMax::Key), /**/);
//
	// ページサイズ (KB 単位)

	cIndex_.getFileID().setInteger(
	   _SYDNEY_SCHEMA_PARAMETER_KEY(FileCommon::FileOption::PageSize::Key), Os::SysConf::PageSize::get() >> 10);

//	// 1 ノードページあたりのオブジェクト数
//
//	cIndex_.getFileID().setInteger(
//		_SYDNEY_SCHEMA_PARAMETER_KEY(Btree::FileOption::KeyObjectPerNode::Key), /**/);

	// 格納エリア
	cIndex_.setAreaPath(cPath_);

	// ファイル属性
	cIndex_.setAttribute(eAttr_);
}

// ベクターファイルのファイルIDをセットする

void
_SetVectorFileID(SystemTable::IndexFile& cIndex_,
				 const Os::Path& cPath_,
				 SystemTable::Attribute::Value eAttr_,
				 bool bMounted_)
{
	// 常に一時ファイルを使うように設定されていたら
	// システム表のファイルもそのようにセットする

	if (Manager::Configuration::isAlwaysTemporary())

		// 一時ファイルかどうかをセットする

		cIndex_.getFileID().setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(FileCommon::FileOption::Temporary::Key),
									   true);

	// バージョン
	// 本当はcreate時に入るのだが、Schemaは毎回作り直しているので
	// 0をハードコーディングで入れる必要がある。
	// いずれFileIDを保存する物理ファイルを作らなければならない…。

	cIndex_.getFileID().setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(FileCommon::FileOption::Version::Key),
#ifdef SYD_SCHEMA_VECTOR2
								   2);
#else
								   0);
#endif

	// Mount フラグ
	// 引数で指定される
	cIndex_.getFileID().setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(FileCommon::FileOption::Mounted::Key),
								   bMounted_);

	// フィールド情報をセットする
	//		Key1:	キーの型
	//		Key2:	バリューの型

	int i = 0;

	// コンストラクト時に指定されたデータ型(Key)

	; _SYDNEY_ASSERT(Common::Data::isFixedSize(cIndex_.getKeyType()));
	cIndex_.getFileID().setInteger(_SYDNEY_SCHEMA_FORMAT_KEY(FileCommon::FileOption::FieldType::Key, i),
								   cIndex_.getKeyType());
	i++;

	// コンストラクト時に指定されたバリューのデータ型

	; _SYDNEY_ASSERT(Common::Data::isFixedSize(cIndex_.getValueType()));
	cIndex_.getFileID().setInteger(_SYDNEY_SCHEMA_FORMAT_KEY(FileCommon::FileOption::FieldType::Key, i),
								   cIndex_.getValueType());
	i++;

	// フィールドの数
	cIndex_.getFileID().setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(FileCommon::FileOption::FieldNumber::Key),
								   i);

	// ページサイズ (KB 単位)
	cIndex_.getFileID().setInteger(
	   _SYDNEY_SCHEMA_PARAMETER_KEY(FileCommon::FileOption::PageSize::Key), Os::SysConf::PageSize::get() >> 10);

	// 格納エリア
	cIndex_.setAreaPath(cPath_);

	// ファイル属性
	cIndex_.setAttribute(eAttr_);
}

} // namespace

//////////////////////////////////////////
//	Schema::SystemTable::ConsistFile	//
//////////////////////////////////////////

//	FUNCTION public
//	Schema::SystemTable::ConsistFile::destruct --
//		システム表を格納するファイルが保持するポインターの後処理
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
SystemTable::ConsistFile::
destruct()
{
	getFile().detach();
	delete m_pFileID, m_pFileID = 0;
}

//	FUNCTION public
//	Schema::SystemTable::ConsistFile::create -- システム表の保管場所を生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::ConsistFile::
create(Trans::Transaction& cTrans_)
{
	// レコードファイルを生成する
	// fileIDはアクセスのたびに作るので保存しておく必要はない

	(void) getFile().create(cTrans_);
}

//	FUNCTION public
//	Schema::SystemTable::ConsistFile::isAccessible -- システム表の保管場所が生成されているか
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

bool
SystemTable::ConsistFile::
isAccessible()
{
	return getFile().isAccessible();
}

//	FUNCTION public
//	Schema::SystemTable::ConsistFile::drop -- システム表の保管場所を抹消する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::ConsistFile::
drop(Trans::Transaction& cTrans_)
{
	// 初期化はされているはず

	; _SYDNEY_ASSERT(getFile().isAttached());

	// ファイルを破棄する
	// detachもこの中で行われる

	getFile().destroy(cTrans_, true);
}

//	FUNCTION public
//	Schema::SystemTable::ConsistFile::move -- システム表の保管場所を移動する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//		const Os::Path& cNewPath_
//			移動後のパス
//		bool bUndo_ = false
//			trueの場合エラー処理中なので重ねてのエラー処理はしない
//		bool bRecovery_ = false
//			trueなら変更前のパスにファイルがなくても変更後にあればOKとする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::ConsistFile::
move(Trans::Transaction& cTrans_,
	 const Os::Path& cNewPath_,
	 bool bUndo_,
	 bool bRecovery_)
{
	if (bRecovery_) {
		// リカバリー時には移動前と移動後のどちらにあるか調べる
		if (!getFile().isAccessible()) {
			// 移動前にファイルがない
			// -> 移動後にあるか調べる

			setAreaPath(cNewPath_);

			// レコードファイルドライバーを取得する
			LogicalFile::FileDriver* pDriver =
				LogicalFile::FileDriverManager::getDriver(LogicalFile::FileDriverID::Record);
			getFile().attach(*pDriver, getFileID());

			if (!getFile().isAccessible()) {
				// 移動後にもないので例外送出
				_SYDNEY_THROW0(Exception::RecoveryFailed);
			}
			// 移動後にあるのでそのまま終了
			return;
		}
	}

	// 移動前のパスを覚えておく
	Os::Path cPrevPath(m_cPath);
	cPrevPath.addPart(getPathPart());

	// 移動後のパスを作る
	Os::Path cNewPath(cNewPath_);
	cNewPath.addPart(getPathPart());

	// ディレクトリーを設定する
	Utility::File::AutoRmDir cAutoRmDir;
	cAutoRmDir.setDir(cNewPath);

	// 移動してしまったかを覚えておく
	bool bMoved = false;

	try {
		Common::StringArrayData cData;
		cData.setElement(0, cNewPath);

		// ファイルを移動する
		(void) getFile().move(cTrans_, cData);
		// 移動したら移動後のパスを自動で破棄できない
		cAutoRmDir.disable();
		bMoved = true;
		SCHEMA_FAKE_ERROR("Schema::SystemFileSub", "Move", "Moved");

		// ファイルIDを新しいパスに変更する
		setAreaPath(cNewPath_);
		SCHEMA_FAKE_ERROR("Schema::SystemFileSub", "Move", "SetPath");

		// 移動前のディレクトリーを消去するのは
		// RecoveryやRedoがやってくれる

	} catch (...) {

		// UNDO中ならエラー処理中をしない
		if (!bUndo_) {

			if (bMoved) {
				// 新しいファイルIDでアタッチしなおしてmoveする
				setAreaPath(cNewPath_);
				Common::StringArrayData cData;
				cData.setElement(0, cPrevPath);

				// ファイルを移動する
				(void) getFile().move(cTrans_, cData);
				// 移動したら再び移動後のパスを破棄できる
				cAutoRmDir.enable();
			}
		}

		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Schema::SystemTable::ConsistFile::mount -- ファイルを mount する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		const LogicalFile::FileID&
//			ファイル記述子
//
//	EXCEPTIONS
const LogicalFile::FileID&
SystemTable::ConsistFile::
mount(Trans::Transaction& cTrans_)
{
	*m_pFileID = getFile().mount(cTrans_);
	return *m_pFileID;
}

//	FUNCTION public
//	Schema::SystemTable::ConsistFile::unmount -- ファイルを unmount する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		const LogicalFile::FileID&
//			ファイル記述子
//
//	EXCEPTIONS
const LogicalFile::FileID&
SystemTable::ConsistFile::
unmount(Trans::Transaction& cTrans_)
{
	*m_pFileID = getFile().unmount(cTrans_);
	return *m_pFileID;
}

//	FUNCTION public
//	Schema::SystemTable::ConsistFile::flush -- ファイルを flush する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
SystemTable::ConsistFile::
flush(Trans::Transaction& cTrans_)
{
	getFile().flush(cTrans_);
}

//	FUNCTION public
//	Schema::SystemTable::ConsistFile::sync -- 不要な版を破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			不要な版を破棄する処理を行う
//			トランザクションのトランザクション記述子
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
SystemTable::ConsistFile::sync(
	Trans::Transaction& trans, bool& incomplete, bool& modified)
{
	getFile().sync(trans, incomplete, modified);
}

//	FUNCTION public
//	Schema::SystemTable::ConsistFile::startBackup -- バックアップを開始する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//		bool bRestorable_ = true
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
SystemTable::ConsistFile::
startBackup(Trans::Transaction& cTrans_, bool bRestorable_)
{
	getFile().startBackup(cTrans_, bRestorable_);
}

//	FUNCTION public
//	Schema::SystemTable::ConsistFile::endBackup -- バックアップを終了する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::ConsistFile::
endBackup(Trans::Transaction& cTrans_)
{
	getFile().endBackup(cTrans_);
}

//	FUNCTION public
//	Schema::SystemTable::ConsistFile::recover -- 障害から回復する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
SystemTable::ConsistFile::
recover(Trans::Transaction& cTrans_, const Trans::TimeStamp& cPoint_)
{
	getFile().recover(cTrans_, cPoint_);
}

//	FUNCTION public
//	Schema::SystemTable::ConsistFile::restore --
//		ある時点に開始された版管理するトランザクションが
//		参照する版を最新版とする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
SystemTable::ConsistFile::
restore(Trans::Transaction& cTrans_, const Trans::TimeStamp& cPoint_)
{
	getFile().restore(cTrans_, cPoint_);
}

//	FUNCTION public
//	Schema::SystemTable::ConsistFile::open --
//		システム表の保管場所であるファイルをオープンする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//		const LogicalFile::OpenOption& cOpenOption_
//			ファイルドライバーに渡すオープンオプション
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::ConsistFile::
open(Trans::Transaction& cTrans_, const LogicalFile::OpenOption& cOpenOption_)
{
	if (!getFile().isOpened()) {
		getFile().open(cTrans_, cOpenOption_);
	}
}

//	FUNCTION public
//	Schema::SystemTable::ConsistFile::close --
//		システム表の保管場所であるファイルをクローズする
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
SystemTable::ConsistFile::
close()
{
	if (getFile().isOpened()) {
		getFile().close();
	}
}

//	FUNCTION public
//	Schema::SystemTable::ConsistFile::setAreaPath --
//		FileIDのパス名部分をセットする
//
//	NOTES
//
//	ARGUMENTS
//		const Os::Path& cPath_
//			システムファイルを格納するトップディレクトリーパス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::ConsistFile::
setAreaPath(const Os::Path& cPath_)
{
	getFileID().setString(_SYDNEY_SCHEMA_FORMAT_KEY(FileCommon::FileOption::Area::Key, 0),
						  Os::Path(cPath_).addPart(getPathPart()));
	m_cPath = cPath_;
}

//	FUNCTION public
//	Schema::SystemTable::ConsistFile::setAttribute --
//		ファイルの属性を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Attribute::Value eAttr_
//			ファイル属性
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::ConsistFile::
setAttribute(Attribute::Value eAttr_)
{
	bool bReadOnly = (eAttr_ & Attribute::ReadOnly ? true : false);
	getFileID().setBoolean(FileCommon::FileOption::ReadOnly::Key, bReadOnly);
}

//	FUNCTION public
//	Schema::SystemTable::ConsistFile::getFileID --
//		システム表を格納するファイルを構成するファイルのFileIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		システム表を格納するファイルを構成するファイルのFileIDへの参照
//
//	EXCEPTIONS

LogicalFile::FileID&
Schema::SystemTable::ConsistFile::
getFileID()
{
	if (!m_pFileID) {
		m_pFileID = new LogicalFile::FileID();
	}
	return *m_pFileID;
}

//////////////////////////////////////
//	Schema::SystemTable::RecordFile	//
//////////////////////////////////////

// FUNCTION public
//	Schema::SystemTable::RecordFile::~RecordFile -- デストラクター
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS

SystemTable::RecordFile::
~RecordFile()
{
	delete m_pTupleData, m_pTupleData = 0;
}

//	FUNCTION public
//	Schema::SystemTable::RecordFile::initialize --
//		システムファイルが使うレコードファイルの初期化を行う
//
//	NOTES
//
//	ARGUMENTS
//		Schema::SystemTable::SystemFile& cFile_
//			システムファイル
//		const Os::Path& cPath_
//			システム表のトップディレクトリーパス名
//	   	SystemTable::Attribute::Value eAttr_
//			システム表のファイル属性
//		bool bMounted_ = true
//			システム表がマウントされているかを示す
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::RecordFile::
initialize(SystemFile& cFile_, const Os::Path& cPath_,
		   SystemTable::Attribute::Value eAttr_,
		   bool bMounted_)
{
	if (!getFile().isAttached()) {

		// FileIDの中身をセットする
		_SetRecordFileID(*this, cPath_, eAttr_, bMounted_);

		// ロック名情報をセットする
		cFile_.setLockName(getFileID());

		// フィールド情報をセットする
		cFile_.setFieldInfo(getFileID());

		// レコードファイルドライバーを取得する
		LogicalFile::FileDriver* pDriver =
			LogicalFile::FileDriverManager::getDriver(LogicalFile::FileDriverID::Record);

		// ファイルをアタッチする
		getFile().attach(*pDriver, getFileID());
	}
}

//	FUNCTION public
//	Schema::SystemTable::RecordFile::getPathPart --
//		システムファイルが使うレコードファイルのパス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		レコードファイルを格納するパス名に使う文字列
//
//	EXCEPTIONS

ModCharString
SystemTable::RecordFile::
getPathPart() const
{
	return ModCharString(PathParts::SystemTable::Record);
}

// ファイルから値を取得するのに用いるデータを得る
Common::DataArrayData&
SystemTable::RecordFile::
getTupleData()
{
	if (!m_pTupleData) {
		m_pTupleData = new Common::DataArrayData;
		int iFieldNumber = 0;
		if (!getFileID().getInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(FileCommon::FileOption::FieldNumber::Key), iFieldNumber)) {
			// ありえない
			; _SYDNEY_ASSERT(false);
			_SYDNEY_THROW0(Exception::Unexpected);
		}
		m_pTupleData->reserve(iFieldNumber);
		for (int i = 0; i < iFieldNumber; ++i) {
			int iFieldType;
			if (!getFileID().getInteger(_SYDNEY_SCHEMA_FORMAT_KEY(FileCommon::FileOption::FieldType::Key, i), iFieldType)) {
				// ありえない
				; _SYDNEY_ASSERT(false);
				_SYDNEY_THROW0(Exception::Unexpected);
			}
			m_pTupleData->pushBack(Common::DataInstance::create(static_cast<Common::DataType::Type>(iFieldType)));
		}
	}
	return *m_pTupleData;
}

//////////////////////////////////////
//	Schema::SystemTable::IndexFile	//
//////////////////////////////////////

// FUNCTION public
//	Schema::SystemTable::IndexFile::~IndexFile -- デストラクター
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS

SystemTable::IndexFile::
~IndexFile()
{
	delete m_pTupleData, m_pTupleData = 0;
}

//	FUNCTION public
//	Schema::SystemTable::IndexFile::initialize --
//		システムファイルが使う索引ファイルの初期化を行う
//
//	NOTES
//
//	ARGUMENTS
//		SystemFile& cFile_
//			システムファイル
//		const Os::Path& cPath_
//			システム表のトップディレクトリーパス名
//		SystemTable::Attribute::Value eAttr_
//			システム表のファイル属性
//		bool bMounted_ = true
//			システム表がマウントされているかを示す
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::IndexFile::
initialize(SystemFile& cFile_,
		   const Os::Path& cPath_,
		   SystemTable::Attribute::Value eAttr_,
		   bool bMounted_)
{
	if (!getFile().isAttached()) {

		// FileIDの中身をセットする
		switch (getCategory()) {
		case Category::Btree:
		{
			_SetBtreeFileID(*this, cPath_, eAttr_, bMounted_);

			// ロック名情報をセットする
			cFile_.setLockName(getFileID());

			// B+木ファイルドライバーを取得する
			LogicalFile::FileDriver* pDriver =
				LogicalFile::FileDriverManager::getDriver(LogicalFile::FileDriverID::Btree);

			// ファイルをアタッチしたものをセットする
			getFile().attach(*pDriver, getFileID());
			break;
		}
		case Category::Vector:
		{
			_SetVectorFileID(*this, cPath_, eAttr_, bMounted_);

			// ロック名情報をセットする
			cFile_.setLockName(getFileID());

			// ベクターファイルドライバーを取得する
			LogicalFile::FileDriver* pDriver =
				LogicalFile::FileDriverManager::getDriver(LogicalFile::FileDriverID::Vector);

			// ファイルをアタッチしたものをセットする
			getFile().attach(*pDriver, getFileID());
			break;
		}
		default:
		{
			; _SYDNEY_ASSERT(false);
			break;
		}
		}
	}
}

//	FUNCTION public
//	Schema::SystemTable::IndexFile::getPathPart --
//		システムファイルが使う索引ファイルのパス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		索引ファイルを格納するパス名に使う文字列
//
//	EXCEPTIONS

ModCharString
SystemTable::IndexFile::
getPathPart() const
{
	return ModCharString(_pszIndexPrefix[static_cast<int>(m_eCategory)]).append(getName());
}

//	FUNCTION public
//	Schema::SystemTable::IndexFile::makeInsertTuple --
//		システムファイルが使う索引ファイルに挿入するタプルを作る
//
//	NOTES
//
//	ARGUMENTS
//		Common::DataArrayData* pTuple_
//			挿入するデータを得るのに使うタプルデータ
//
//	RETURN
//		索引ファイルに挿入するタプル
//
//	EXCEPTIONS

Common::DataArrayData*
SystemTable::IndexFile::
makeInsertTuple(Common::DataArrayData* pTuple_)
{
	ModAutoPointer<Common::DataArrayData> pResult = new Common::DataArrayData;
	pResult->reserve(m_eCategory == Category::Btree ? 3 : 2);

	; _SYDNEY_ASSERT(pTuple_->getCount() >= static_cast<int>(m_iKeyPosition));
	; _SYDNEY_ASSERT(pTuple_->getCount() >= static_cast<int>(m_iValuePosition));

	if (m_eCategory == Category::Btree) {
		pResult->pushBack(new LogicalFile::ObjectID());
	}
	pResult->pushBack(pTuple_->getElement(m_iKeyPosition));
	pResult->pushBack(pTuple_->getElement(m_iValuePosition));

	return pResult.release();
}

//	FUNCTION public
//	Schema::SystemTable::IndexFile::makeExpungeKey --
//		システムファイルが使う索引ファイルから削除するキーとなるタプルを作る
//
//	NOTES
//
//	ARGUMENTS
//		Common::DataArrayData* pTuple_
//			索引のキーとバリューを得るのに使うタプル
//
//	RETURN
//		索引ファイルの削除キーとなるタプル
//
//	EXCEPTIONS

Common::DataArrayData*
SystemTable::IndexFile::
makeExpungeKey(Common::DataArrayData* pTuple_)
{
	ModAutoPointer<Common::DataArrayData> pResult = new Common::DataArrayData;
	pResult->reserve(m_eCategory == Category::Btree ? 2 : 1);

	; _SYDNEY_ASSERT(pTuple_->getCount() > static_cast<int>(m_iKeyPosition));
	; _SYDNEY_ASSERT(pTuple_->getCount() > static_cast<int>(m_iValuePosition));

	pResult->pushBack(pTuple_->getElement(m_iKeyPosition));
	if (m_eCategory == Category::Btree) {
		// Btreeの場合はキーとバリューで指定する
		pResult->pushBack(pTuple_->getElement(m_iValuePosition));
	}
	return pResult.release();
}

// 索引から値を取得するのに用いるデータを得る
Common::DataArrayData&
SystemTable::IndexFile::
getTupleData(bool bIncludeKey_ /* = false */)
{
	int iFieldNumber = (bIncludeKey_ ? 2 : 1);

	if (!m_pTupleData || m_pTupleData->getCount() != iFieldNumber) {
		// データを取得するためのタプルデータを作る
		if (m_pTupleData) delete m_pTupleData;
		m_pTupleData = new Common::DataArrayData;
		m_pTupleData->reserve(iFieldNumber);
		if (bIncludeKey_)
			m_pTupleData->pushBack(Common::DataInstance::create(getKeyType()));
		m_pTupleData->pushBack(Common::DataInstance::create(getValueType()));
	}
	return *m_pTupleData;
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
