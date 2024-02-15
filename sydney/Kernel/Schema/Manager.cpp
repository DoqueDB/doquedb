// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.cpp -- マネージャー関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2013, 2017, 2023 Ricoh Company, Ltd.
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

//#define LEAK_BUG

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyInclude.h"

#include "Exception/Cancel.h"
#include "Exception/FakeError.h"
#include "Exception/FileAlreadyExisted.h"

#include "Schema/Manager.h"
#include "Schema/Database.h"
#include "Schema/Externalizable.h"
#include "Schema/Hold.h"
#include "Schema/Message.h"
#include "Schema/NameParts.h"
#include "Schema/ObjectID.h"
#include "Schema/ObjectSnapshot.h"
#include "Schema/Parameter.h"
#include "Schema/PathParts.h"
#include "Schema/Recovery.h"
#include "Schema/Reorganize.h"
#include "Schema/ReorganizePrivilege.h"
#include "Schema/Sequence.h"
#include "Schema/SessionID.h"
#include "Schema/SystemDatabase.h"
#include "Schema/SystemTable_Database.h"
#include "Schema/Table.h"
#include "Schema/TemporaryDatabase.h"
#include "Schema/Utility.h"

#include "Common/Assert.h"
#include "Common/Configuration.h"
#include "Common/Externalizable.h"
#include "Common/Hasher.h"
#include "Common/Message.h"
#include "Common/ObjectPointer.h"
#include "Common/Parameter.h"
#include "Common/SystemParameter.h"

#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"
#include "Os/Limits.h"
#include "Os/Path.h"
#include "Os/Process.h"

#include "Trans/AutoTransaction.h"
#include "Trans/Transaction.h"
#include "Trans/ID.h"

#include "Lock/Client.h"
#include "Lock/Duration.h"
#include "Lock/Item.h"
#include "Lock/Mode.h"
#include "Lock/Name.h"

#include "ModAutoPointer.h"
#include "ModHashMap.h"
#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace {

// Terminateでそれぞれちゃんと破棄するようにしたので
// AutoPointerにする必要はない
// staticなAutoPointerを使うとプロセス終了時に
// 制御不能な順序で解放処理が行われるので不正なアドレスを参照する可能性がある
//#ifdef PURIFY
//#define SCHEMA_USE_STATIC_AUTOPOINTER
//#endif

#ifdef SCHEMA_USE_STATIC_AUTOPOINTER
#define _AutoPointer(__type__)	ModAutoPointer<__type__ >
#define _IsValidPointer(__pointer__)	(__pointer__.get() != 0)
#define _Free(__pointer__)	{__pointer__ = 0;}
#else
#define _AutoPointer(__type__)	__type__*
#define _IsValidPointer(__pointer__)	(__pointer__ != 0)
#define _Free(__pointer__)	{delete __pointer__; __pointer__ = 0;}
#endif

// マネージャーに対する操作を
// スレッド間排他制御するためのクリティカルセクション

Os::CriticalSection	_cCriticalSection;

// デフォルトのエリアのパス名

_AutoPointer(Os::Path)		_areaPath = 0;

// システム表のエリアのパス名

_AutoPointer(Os::Path)		_systemAreaPath = 0;

// デフォルトのエリアのパス名の設定に関するパラメーター名
// デフォルト値の扱いが特殊なのでConfiguration::Parameterクラスは使用できない
const ModCharString	_cstrDefaultAreaPathKey("Schema_DefaultAreaPath");

// システム表のエリアのパス名の設定に関するパラメーター名
// デフォルト値の扱いが特殊なのでConfiguration::Parameterクラスは使用できない
const ModCharString	_cstrSystemAreaPathKey("Schema_SystemAreaPath");

// デフォルトデータベースのスキーマ情報を
// マネージャーの初期化時に読み込むか

Common::Configuration::ParameterBoolean _cPreloadSchema("Schema_PreloadSchema", false, false);

#ifdef OBSOLETE // 常にすぐに削除するのでこのConfigurationは使用されない
// ファイルやディレクトリーをすぐに削除するか
Common::Configuration::ParameterBoolean _cDestroyImmediately("Schema_DestroyImmediately", false, false);
#endif

// ファイルを常に一時ファイルとして作成するか
Common::Configuration::ParameterBoolean _cAlwaysTemporary("Schema_AlwaysTemporary", false, false);

// 既存の名前でスキーマオブジェクトの生成要求がされたときに
// 処理をキャンセルするか
Common::Configuration::ParameterBoolean _cCancelWhenDuplicated("Schema_CancelWhenDuplicated", false, false);

// FileIDにEncodingFormをセットしないか
Common::Configuration::ParameterBoolean _cNoEncodingForm("Schema_NoEncodingForm", false, false);

namespace _Name
{
	// _Nameに属する変数に対する操作を
	// スレッド間排他制御するためのクリティカルセクション
	Os::CriticalSection	_latch;

	// 作成中の名前を管理するためのベクター
	_AutoPointer(ModVector<const Object*>)	_reserve = 0;

	// オブジェクト名の長さの上限
	ModSize _iMaxLength = 50;			// Windowsのフルパスの上限258を超えないため
}

namespace _Path
{
	// _Pathに属する変数に対する操作を
	// スレッド間排他制御するためのクリティカルセクション
	Os::CriticalSection	_latch;

	// 作成を予約したパスを管理するためのベクター
	typedef ModPair<const Object*, ModVector<ModUnicodeString> > _Element;
	_AutoPointer(ModVector<_Element>) _reserve = 0;

	// パス名の長さの上限
	ModSize _iMaxLength = 100;			// Windowsのフルパスの上限258を超えないため
}

// データベースIDを生成するシーケンスファイル
_AutoPointer(Schema::Sequence) _sequence = 0;

// スキーマオブジェクトのキャッシュサイズに関する定義
namespace _ObjectCacheSize
{
	// スキーマオブジェクトのキャッシュの破棄処理を試みる閾値
	Common::Configuration::ParameterInteger _cMaxSize("Schema_ObjectCacheSize", Os::Limits<int>::getMax());

	// 以下の情報にアクセスするための排他オブジェクト
	Os::CriticalSection _latch;
	// キャッシュサイズ個数の現在値
	int _iCurrentSize = 0;
}

// スナップショットに関する定義
namespace _ObjectSnapshot
{
	// _ObjectSnapshotに属する変数に対する操作を
	// スレッド間排他制御するためのクリティカルセクション
	Os::CriticalSection	_latch;

	//	VARIABLE local
	//	_readWriteSnapshot -- 最新のスナップショット
	//
	//	NOTES

	//	VARIABLE local
	//	_currentSnapshot -- 現在のタイムスタンプに対応するスナップショット
	//
	//	NOTES
	typedef Common::ObjectPointer<ObjectSnapshot> SnapshotPointer;
	SnapshotPointer					_currentSnapshot;
	SnapshotPointer					_readWriteSnapshot;

	//	VARIABLE local
	//	_lastReorganize -- 最後にスキーマ情報の変更がコミットされた直後のトランザクションID
	//
	//	NOTES
	Trans::ID						_lastReorganize;

	//	STRUCT local
	//	_SessionMapEntry -- セッションと対応させる情報を保持する構造体
	//
	//	NOTES

	struct _SessionMapEntry
	{
		Trans::Transaction::ID			m_iTransactionID;
		SnapshotPointer					m_pSnapshot;

	public:
		_SessionMapEntry()
			: m_iTransactionID(), m_pSnapshot()
		{ }
		_SessionMapEntry(Trans::Transaction::ID transaction_,
						 const SnapshotPointer& snapshot_)
			: m_iTransactionID(transaction_), m_pSnapshot(snapshot_)
		{ }

	private:
	};

	//	VARIABLE local
	//	_sessions -- セッションとスナップショット関係の情報を対応させるマップ
	//
	//	NOTES
	
	typedef
	ModHashMap<SessionID, _SessionMapEntry, ModHasher<SessionID> > SessionMap;
	_AutoPointer(SessionMap) _sessions = 0;

	// SessionMapのコンストラクターに与えるパラメーター
	const ModSize _sessionMapSize = 3;
	const ModBoolean _sessionMapEnableLink = ModFalse; // Iterationしない

} // namespace _ObjectSnapshot

} // $$$

//////////////////////////
//	 Schema::Manager	//
//////////////////////////

//	FUNCTION
//	Schema::Manager::initialize -- マネージャーを初期化する
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
Manager::
initialize()
{
	// スキーマ関連のシリアル化可能なオブジェクトを
	// 確保するための関数を共通ライブラリーに登録する

	Common::Externalizable::insertFunction(
		Common::Externalizable::SchemaClasses,
		Externalizable::getClassInstance);

	//【注意】	以下の初期化関数は、必要なときに実行されるので、
	//			ここでは呼び出さない

//	Utility::initialize();
//	Configuration::initialize();
	SystemTable::initialize();
//	ObjectName::initialize();
//	ObjectPath::initialize();
//	ObjectTree::initialize();

	// 一時データベースモジュールの初期化を行う
	TemporaryDatabase::initialize();
}

//	FUNCTION
//	Schema::Manager::terminate -- マネージャーを終了処理する
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
Manager::
terminate()
{
	// 一時データベースモジュールの終了処理も行う
	TemporaryDatabase::terminate();
	// メタデータベースの終了処理も行う
	SystemDatabase::terminate();

	SystemTable::terminate();
	ObjectTree::terminate();
	ObjectPath::terminate();
	ObjectName::terminate();
	ObjectSnapshot::terminate();
	Configuration::terminate();

	// Utilityの後処理を行う
	Utility::terminate();
}

//	FUNCTION
//	Schema::Manager::install -- マネージャーのインストール処理を行う
//
//	NOTES
//		MT-unsafeである
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			インストールに使用されるトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Manager::install(Trans::Transaction& cTrans_)
{
	SystemTable::install(cTrans_);
	ObjectTree::install(cTrans_);

	// データベース無指定時に使うデフォルトのデータベースを定義し、
	// 保存しておく

	Database::Pointer pDatabase
		= Database::create(Object::Name(NameParts::Database::Default),
						   cTrans_);
	; _SYDNEY_ASSERT(pDatabase.get());
	pDatabase->create(cTrans_);

	// メタデータベースを「データベースの読み書き」でロック
	SystemTable::hold(cTrans_,
					  Hold::Target::MetaDatabase,
					  Lock::Name::Category::Database,
					  Hold::Operation::ReadWrite);

	// 定義するデータベースの情報を「データベース」表へ登録する
	Schema::SystemTable::Database().store(cTrans_, pDatabase);

	// ObjectIDを永続化する
	Schema::ObjectID::persist(cTrans_, pDatabase->getDatabase(cTrans_));

	// 作成中の登録から削除する
	Manager::ObjectName::withdraw(pDatabase.get());
}

#ifdef OBSOLETE // uninstallは使用されない
//	FUNCTION
//	Schema::Manager::uninstall -- マネージャーのアンインストール処理を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			アンインストールに使用されるトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Manager::uninstall(Trans::Transaction& cTrans_)
{
	// システム表をロックする

	SystemTable::hold(cTrans_,
					  Hold::Target::MetaDatabase,
					  Lock::Name::Category::Database,
					  Hold::Operation::Drop);

	// すべてのデータベースを抹消する
	{
		const ModVector<Database*>& database = ObjectTree::Database::get(cTrans_);

		ModSize n = database.getSize();
		for (ModSize i = 0; i < n; i++) {
			database[i]->drop(cTrans_);
		}
	}

	// 変更を永続化する
	// ★注意★
	// データベースを構成するファイルを削除するために
	// オブジェクトのdeleteもこの中で行われる

	Schema::SystemTable::Database().store(cTrans_);

	// システム表、データベースIDを生成するシーケンスファイルを
	// それぞれ削除する

	ObjectTree::uninstall(cTrans_);
	SystemTable::uninstall(cTrans_);

	// デフォルトのパス名ディレクトリーを削除する

	Utility::File::rmAll(Configuration::getDefaultAreaPath());
	Utility::File::rmAll(Configuration::getSystemAreaPath());
}
#endif

//	FUNCTION
//	Schema::Manager::recover -- マネージャーの障害回復処理を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Trans::TimeStamp& cPoint_
//			障害を回復する時点を表すタイムスタンプ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Manager::recover(Trans::Transaction& cTrans_, const Trans::TimeStamp& cPoint_)
{
	// システム表、データベースIDを生成するシーケンスファイルを
	// それぞれ回復する

	ObjectTree::recover(cTrans_, cPoint_);
	SystemTable::recover(cTrans_, cPoint_);
}

//	FUNCTION
//	Schema::Manager::restore --
//		ある時点に開始された版管理するトランザクションが
//		参照する版を最新版とする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Trans::TimeStamp& cPoint_
//			障害を回復する時点を表すタイムスタンプ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Manager::restore(Trans::Transaction& cTrans_, const Trans::TimeStamp& cPoint_)
{
	// システム表、データベースIDを生成するシーケンスファイルを
	// それぞれ回復する

	ObjectTree::restore(cTrans_, cPoint_);
	SystemTable::restore(cTrans_, cPoint_);
}

//	FUNCTION
//	Schema::Manager::sync -- 不要な版を破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			不要な版を破棄する処理を行う
//			トランザクションのトランザクション記述子
//		bool&				incomplete
//			true
//				今回の同期処理でシステム表を持つ
//				オブジェクトの一部に処理し残しがある
//			false
//				今回の同期処理でシステム表を持つ
//				オブジェクトを完全に処理してきている
//
//				同期処理の結果、システム表を処理し残したかを設定する
//		bool&				modified
//			true
//				今回の同期処理でシステム表を持つ
//				オブジェクトの一部が既に更新されている
//			false
//				今回の同期処理でシステム表を持つ
//				オブジェクトはまだ更新されていない
//
//				同期処理の結果、システム表が更新されたかを設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::sync(Trans::Transaction& trans, bool& incomplete, bool& modified)
{
	ObjectTree::sync(trans, incomplete, modified);
	SystemTable::sync(trans, incomplete, modified);
}

//	FUNCTION
//	Schema::Manager::checkCanceled -- 
//		中断要求の有無を調べる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::Canceled
//			中断された

void
Manager::
checkCanceled(Trans::Transaction& cTrans_)
{
	_SYDNEY_FAKE_ERROR("Schema::Manager_checkCanceled",
					   Exception::Cancel(moduleName, srcFile, __LINE__));

	if (cTrans_.isCanceledStatement()) {
		_SYDNEY_THROW0(Exception::Cancel);
	}
}

//
//	Configuration
//

//	FUNCTION
//	Schema::Manager::Configuration::initialize --
//		マネージャーのうち、設定関連を初期化する
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
Manager::Configuration::
initialize()
{ }

//	FUNCTION
//	Schema::Manager::Configuration::terminate --
//		マネージャーのうち、設定関連を終了処理する
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
Manager::Configuration::
terminate()
{
	// デフォルト、システム表のエリアのパス名を破棄する
	_Free(_areaPath);
	_Free(_systemAreaPath);
}

//	FUNCTION
//	Schema::Manager::Configuration::getDefaultAreaPath --
//		デフォルトのエリアのパス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたデフォルトのエリアのパス名
//
//	EXCEPTIONS

const Os::Path&
Manager::Configuration::
getDefaultAreaPath()
{
	// 念のために初期化しておく
	initialize();

	if (!_IsValidPointer(_areaPath)) {
		Os::AutoCriticalSection m(_cCriticalSection);
		// クリティカルセクションの中でもう一度調べる

		if (!_IsValidPointer(_areaPath)) {
			// デフォルトのエリアのパス名を取得する
			// システムパラメーターにより指定されないときは、
			// カレントワーキングディレクトリーにする

			ModUnicodeString	v;

			_areaPath = new Os::Path(
				(Common::SystemParameter::getValue(_cstrDefaultAreaPathKey, v)) ?
				v : static_cast<ModUnicodeString>(
					Os::Process::getCurrentDirectory()));

			; _SYDNEY_ASSERT(_IsValidPointer(_areaPath));
		}
	}
	return *_areaPath;
}

//	FUNCTION
//	Schema::Manager::Configuration::getSystemAreaPath --
//		システム表のエリアのパス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたシステム表のエリアのパス名
//
//	EXCEPTIONS

const Os::Path&
Manager::Configuration::
getSystemAreaPath()
{
	// 念のために初期化しておく
	initialize();

	if (!_IsValidPointer(_systemAreaPath)) {
		Os::AutoCriticalSection m(_cCriticalSection);
		// クリティカルセクションの中でもう一度調べる

		if (!_IsValidPointer(_systemAreaPath)) {
			// システムのエリアのパス名を取得する
			// システムパラメーターにより指定されないときは、
			// デフォルトのエリアを使う

			ModUnicodeString	v;

			_systemAreaPath = new Os::Path(
				(Common::SystemParameter::getValue(_cstrSystemAreaPathKey, v)) ?
				v :
				((Common::SystemParameter::getValue(_cstrDefaultAreaPathKey, v)) ?
				 v : static_cast<ModUnicodeString>(
					 Os::Process::getCurrentDirectory())));

			; _SYDNEY_ASSERT(_IsValidPointer(_systemAreaPath));
		}
	}

	return *_systemAreaPath;
}

//	FUNCTION
//	Schema::Manager::Configuration::isSchemaPreloaded --
//		デフォルトデータベースのスキーマ情報をマネージャーの
//		初期化時に読み込むかを取得する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			デフォルトデータベースのスキーマ情報をマネージャーの
//			初期化時に読み込む
//		false
//			初期化時に読み込まない
//
//	EXCEPTIONS

bool
Manager::Configuration::
isSchemaPreloaded()
{
	// 念のために初期化しておく
	initialize();

	return _cPreloadSchema;
}

#ifdef OBSOLETE // 常にすぐに削除するのでこのConfigurationは使用されない

//	FUNCTION
//	Schema::Manager::Configuration::isImmediatelyDestroyed --
//		ファイルやディレクトリーをすぐに削除する
//
//	NOTES
//		削除するときに読み取りトランザクションがないことが
//		保証される環境であるかを示す
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			読み取りトランザクションがないように上位がコントロールしているので
//			すぐに削除して構わない
//		false(default)
//			読み取りトランザクションがあるかもしれないので
//			それらがなくなってから削除するようにCheckpointモジュールに
//			登録するのみ
//
//	EXCEPTIONS

bool
Manager::Configuration::
isImmediatelyDestroyed()
{
	// 念のために初期化しておく
	initialize();

	return _cDestroyImmediately;
}
#endif

//	FUNCTION
//	Schema::Manager::Configuration::isAlwaysTemporary --
//		ファイルを常に一時ファイルとして作るかを得る
//
//	NOTES
//		版を作らない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			常に一時ファイルとして作る
//		false(default)
//			一時表を構成するファイルのみ一時ファイルとして作る
//
//	EXCEPTIONS

bool
Manager::Configuration::
isAlwaysTemporary()
{
	// 念のために初期化しておく
	initialize();

	return _cAlwaysTemporary;
}

//	FUNCTION
//	Schema::Manager::Configuration::isCanceledWhenDuplicated --
//		存在する名前でcreateされる処理をキャンセルするかを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			存在する名前でcreate要求がきたら処理をキャンセルしエラーにしない
//		false(default)
//			存在する名前でcreate要求がきたら例外を発生する
//
//	EXCEPTIONS

bool
Manager::Configuration::
isCanceledWhenDuplicated()
{
	// 念のために初期化しておく
	initialize();

	return _cCancelWhenDuplicated;
}

// FUNCTION public
//	Schema::Manager::Configuration::isNoEncodingForm -- B木ファイルのFileIDにEncodingFormを設定しないか
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Manager::Configuration::
isNoEncodingForm()
{
	// 念のために初期化しておく
	initialize();

	return _cNoEncodingForm;
}

//////////////////////////////////
//	Schema::Manager::ObjectTree	//
//////////////////////////////////

//	FUNCTION
//	Schema::Manager::ObjectTree::initialize --
//		マネージャーのうち、オブジェクト木関連を初期化する
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
Manager::ObjectTree::
initialize()
{
	Sequence::initialize();
}

//	FUNCTION
//	Schema::Manager::ObjectTree::terminate --
//		マネージャーのうち、オブジェクト木関連を終了処理する
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
Manager::ObjectTree::
terminate()
{
	Sequence::terminate();
}

//	FUNCTION
//	Schema::Manager::ObjectTree::install --
//		マネージャーのうち、オブジェクト木関連のインストール処理を行う
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
Manager::ObjectTree::
install(Trans::Transaction& cTrans_)
{
	Sequence::install(cTrans_);
}

#ifdef OBSOLETE // uninstallは使用されない
//	FUNCTION
//	Schema::Manager::ObjectTree::uninstall --
//		マネージャーのうち、オブジェクト木 関連のアンインストール処理を行う
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
Manager::ObjectTree::
uninstall(Trans::Transaction& cTrans_)
{
	Sequence::uninstall(cTrans_);
}
#endif

//	FUNCTION
//	Schema::Manager::ObjectTree::recover
//		-- データベース表を障害から回復する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Trans::TimeStamp& cPoint_
//			障害を回復する時点を表すタイムスタンプ
//
//	RETURN
//		なし
//
//
//	EXCEPTIONS

void
Manager::ObjectTree::
recover(Trans::Transaction& cTrans_,
		const Trans::TimeStamp& cPoint_)
{
	Sequence::recover(cTrans_, cPoint_);
}

//	FUNCTION
//	Schema::Manager::ObjectTree::restore --
//		ある時点に開始された版管理するトランザクションが
//		参照する版を最新版とする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Trans::TimeStamp& cPoint_
//			障害を回復する時点を表すタイムスタンプ
//
//	RETURN
//		なし
//
//
//	EXCEPTIONS

void
Manager::ObjectTree::
restore(Trans::Transaction& cTrans_,
		const Trans::TimeStamp& cPoint_)
{
	Sequence::restore(cTrans_, cPoint_);
}

//	FUNCTION
//	Schema::Manager::ObjectTree::sync -- 不要な版を破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			不要な版を破棄する処理を行う
//			トランザクションのトランザクション記述子
//		bool&				incomplete
//			true
//				今回の同期処理でオブジェクト木を持つ
//				オブジェクトの一部に処理し残しがある
//			false
//				今回の同期処理でオブジェクト木を持つ
//				オブジェクトを完全に処理してきている
//
//				同期処理の結果、オブジェクト木を処理し残したかを設定する
//		bool&				modified
//			true
//				今回の同期処理でオブジェクト木を持つ
//				オブジェクトの一部が既に更新されている
//			false
//				今回の同期処理でオブジェクト木を持つ
//				オブジェクトはまだ更新されていない
//
//				同期処理の結果、オブジェクト木が更新されたかを設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::ObjectTree::sync(
	Trans::Transaction& trans, bool& incomplete, bool& modified)
{
	Sequence::sync(trans, incomplete, modified);
}

//////////////////////////////////////////////
//	Schema::Manager::ObjectTree::Sequence	//
//////////////////////////////////////////////

//	FUNCTION
//	Schema::Manager::ObjectTree::Sequence::initialize --
//		マネージャーのうち、オブジェクト木関連を初期化する
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
Manager::ObjectTree::Sequence::
initialize()
{
	if (!_IsValidPointer(_sequence)) {
		// データベース ID の値を生成させるための
		// シーケンスを表すクラスを生成する

		Os::Path cstrPath(Configuration::getSystemAreaPath());
		cstrPath.addPart(PathParts::SystemTable::Schema)
			.addPart(PathParts::Sequence::ObjectID);

		_sequence = new Schema::Sequence(cstrPath, Schema::ObjectID::SystemTable);

		; _SYDNEY_ASSERT(_IsValidPointer(_sequence));
	}
}

//	FUNCTION
//	Schema::Manager::ObjectTree::Sequence::terminate --
//		マネージャーのうち、オブジェクト木関連を終了処理する
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
Manager::ObjectTree::Sequence::
terminate()
{
	_Free(_sequence);
}

//	FUNCTION
//	Schema::Manager::ObjectTree::Sequence::install --
//		マネージャーのうち、オブジェクト木関連のインストール処理を行う
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
Manager::ObjectTree::Sequence::
install(Trans::Transaction& cTrans_)
{
	Os::AutoCriticalSection m(_cCriticalSection);

	// 念のために初期化しておく

	initialize();
	; _SYDNEY_ASSERT(_IsValidPointer(_sequence));

	// データベース ID の値を生成させるための
	// シーケンスを定義する
	// システム表のオブジェクトIDを初期値とする

	_sequence->create(cTrans_, Schema::ObjectID::SystemTable);
}

#ifdef OBSOLETE // uninstallは使用されない
//	FUNCTION
//	Schema::Manager::ObjectTree::Sequence::uninstall --
//		マネージャーのうち、オブジェクト木 関連のアンインストール処理を行う
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
Manager::ObjectTree::Sequence::
uninstall(Trans::Transaction& cTrans_)
{
	Os::AutoCriticalSection m(_cCriticalSection);

	// 念のために初期化しておく

	initialize();
	; _SYDNEY_ASSERT(_IsValidPointer(_sequence));

	// データベース ID の値を生成させるための
	// シーケンスを破棄する

	_sequence->drop(cTrans_);
}
#endif

//	FUNCTION
//	Schema::Manager::ObjectTree::Sequence::recover
//		-- データベース表を障害から回復する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Trans::TimeStamp& cPoint_
//			障害を回復する時点を表すタイムスタンプ
//
//	RETURN
//		なし
//
//
//	EXCEPTIONS

void
Manager::ObjectTree::Sequence::
recover(Trans::Transaction& cTrans_,
		const Trans::TimeStamp& cPoint_)
{
	Os::AutoCriticalSection m(_cCriticalSection);

	// 念のために初期化しておく

	initialize();
	; _SYDNEY_ASSERT(_IsValidPointer(_sequence));

	// データベース ID の値を生成させるための
	// シーケンスをrecoverする

	_sequence->recover(cTrans_, cPoint_);
}

//	FUNCTION
//	Schema::Manager::ObjectTree::Sequence::restore --
//		ある時点に開始された版管理するトランザクションが
//		参照する版を最新版とする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Trans::TimeStamp& cPoint_
//			障害を回復する時点を表すタイムスタンプ
//
//	RETURN
//		なし
//
//
//	EXCEPTIONS

void
Manager::ObjectTree::Sequence::
restore(Trans::Transaction& cTrans_,
		const Trans::TimeStamp& cPoint_)
{
	Os::AutoCriticalSection m(_cCriticalSection);

	// 念のために初期化しておく

	initialize();
	; _SYDNEY_ASSERT(_IsValidPointer(_sequence));

	// データベース ID の値を生成させるための
	// シーケンスをrestoreする

	_sequence->restore(cTrans_, cPoint_);
}

//	FUNCTION
//	Schema::Manager::ObjectTree::Sequence::sync -- 不要な版を破棄する
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
Manager::ObjectTree::Sequence::sync(
	Trans::Transaction& trans, bool& incomplete, bool& modified)
{
	Os::AutoCriticalSection m(_cCriticalSection);

	// 念のために初期化しておく

	initialize();
	; _SYDNEY_ASSERT(_IsValidPointer(_sequence));

	// データベース ID の値を生成させるための
	// シーケンスの不要な版を破棄する

	_sequence->sync(trans, incomplete, modified);
}

//	FUNCTION
//	Schema::Manager::ObjectTree::Sequence::getSequence --
//		新しいデータベース ID の値を生成するためのシーケンスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		得られたシーケンス
//
//	EXCEPTIONS

Schema::Sequence&
Manager::ObjectTree::Sequence::
get()
{
	Os::AutoCriticalSection m(_cCriticalSection);

	// 念のために初期化しておく

	initialize();
	; _SYDNEY_ASSERT(_IsValidPointer(_sequence));

	return *_sequence;
}

//////////////////////////////////////////////
//	Schema::Manager::ObjectTree::Database	//
//////////////////////////////////////////////

//	FUNCTION
//	Schema::Manager::ObjectTree::Database::hold --
//		データベースをロックする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//		Lock::Name::Category::Value eManipulate_
//			操作するオブジェクトのカテゴリー
//		Hold::Operation::Value eOperation_
//			操作の種類
//		Schema::Object::ID::Value iID_
//			ロックするデータベースのID
//
//	RETURN
//		bool
//
//	EXCEPTIONS

bool
Manager::ObjectTree::Database::
hold(Trans::Transaction& cTrans_,
	 Lock::Name::Category::Value eManipulate_,
	 Hold::Operation::Value eOperation_,
	 Object::ID::Value iDatabaseID_,
	 Lock::Timeout::Value iTimeout_)
{
	return Hold::hold(cTrans_,
					  Lock::DatabaseName(iDatabaseID_),
					  Hold::Target::Database,
					  eManipulate_, eOperation_,
					  iTimeout_);
}

//	FUNCTION
//	Schema::Manager::ObjectTree::Database::release --
//		データベースをアンロックする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//		Lock::Name::Category::Value eManipulate_
//			操作するオブジェクトのカテゴリー
//		Hold::Operation::Value eOperation_
//			操作の種類
//		Schema::Object::ID::Value iID_
//			ロックするデータベースのID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::ObjectTree::Database::
release(Trans::Transaction& cTrans_,
		Lock::Name::Category::Value eManipulate_,
		Hold::Operation::Value eOperation_,
		Object::ID::Value iDatabaseID_)
{
	Hold::release(cTrans_,
				  Lock::DatabaseName(iDatabaseID_),
				  Hold::Target::Database,
				  eManipulate_, eOperation_);
}

//	FUNCTION
//	Schema::Manager::ObjectTree::Database::get --
//		システムに存在するすべてのデータベースを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行おうとしているセッションに対応するトランザクション
//
//	RETURN
//		システムに存在するデータベースを定義順にひとつづつ要素とするベクター
//
//	EXCEPTIONS

const ModVector<Database*>&
Manager::ObjectTree::Database::get(Trans::Transaction& cTrans_)
{
	return ObjectSnapshot::get(cTrans_)->getDatabase(cTrans_);
}

//	FUNCTION
//	Schema::Manager::ObjectTree::Database::get --
//		システムに存在するデータベースのうち、
//		あるスキーマオブジェクト ID のデータベースを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	databaseID
//			データベースのスキーマオブジェクト ID
//		Trans::Transaction& cTrans_
//			操作を行おうとしているセッションに対応するスナップショット
//
//	RETURN
//		0 以外の値
//			得られたデータベースを格納する領域の先頭アドレス
//		0
//			システムには指定されたスキーマオブジェクト ID の
//			データベースは存在しない
//
//	EXCEPTIONS

Database*
Manager::ObjectTree::Database::
get(Object::ID::Value databaseID, Trans::Transaction& cTrans_)
{
	return ObjectSnapshot::get(cTrans_)->getDatabase(databaseID, cTrans_);
}

//	FUNCTION
//	Schema::Manager::ObjectTree::getDatabase --
//		システムに存在するデータベースのうち、
//		ある名前のデータベースを表す ID を得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::Name&	databaseName
//			データベースの名前
//		Trans::Transaction& cTrans_
//			操作を行おうとしているセッションに対応するスナップショット
//
//	RETURN
//		0 以外の値
//			得られたデータベースの ID
//		ObjectID::Invalid
//			システムには指定された名前のデータベースは存在しない
//
//	EXCEPTIONS

Object::ID
Manager::ObjectTree::Database::
getID(const Object::Name& databaseName, Trans::Transaction& cTrans_)
{
	return ObjectSnapshot::get(cTrans_)->getDatabaseID(databaseName, cTrans_);
}

//	FUNCTION
//	Schema::Manager::ObjectTree::Database::clearCache --
//		データベースオブジェクトのキャッシュをクリアする
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
Manager::ObjectTree::Database::
clearCache()
{
	// キャッシュのクリアを後回しにしていたスナップショットに対して
	// キャッシュのクリアを試みる
	Schema::ObjectSnapshot::clearDatabaseCache();
}

//	FUNCTION
//	Schema::Manager::ObjectTree::Database::incrementCacheSize --
//		データベースのキャッシュ総数を増やす
//
//	NOTES
//
//	ARGUMENTS
//		int size = 1
//			増やす数
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::ObjectTree::Database::
incrementCacheSize(int size /* = 1 */)
{
	if (size > 0) {
		Os::AutoCriticalSection m(_ObjectCacheSize::_latch);
		_ObjectCacheSize::_iCurrentSize += size;
#ifdef DEBUG
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Increment CacheSize size= " << _ObjectCacheSize::_iCurrentSize
			<< ModEndl;
#endif
	}
}

//	FUNCTION
//	Schema::Manager::ObjectTree::Database::decrementCacheSize --
//		データベースのキャッシュ総数を減らす
//
//	NOTES
//
//	ARGUMENTS
//		int size = 1
//			減らす数
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::ObjectTree::Database::
decrementCacheSize(int size /* = 1*/)
{
	if (size > 0) {
		Os::AutoCriticalSection m(_ObjectCacheSize::_latch);
		if ((_ObjectCacheSize::_iCurrentSize -= size) < 0)
			_ObjectCacheSize::_iCurrentSize = 0;
#ifdef DEBUG
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Decrement CacheSize size= " << _ObjectCacheSize::_iCurrentSize
			<< ModEndl;
#endif
	}
}

//	FUNCTION
//	Schema::Manager::ObjectTree::Database::checkCacheSize --
//		データベースのキャッシュ総数が閾値を超えているか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true .. 閾値を超えている
//		false.. 閾値を超えていない
//
//	EXCEPTIONS

bool
Manager::ObjectTree::Database::
checkCacheSize()
{
	ModSize cacheMaxSize = _ObjectCacheSize::_cMaxSize.get();

	Os::AutoCriticalSection m(_ObjectCacheSize::_latch);
	return (static_cast<ModSize>(_ObjectCacheSize::_iCurrentSize) > cacheMaxSize);
}

// FUNCTION public
//	Schema::Manager::ObjectTree::Database::revokeAll -- call revokeAll for all databases
//
// NOTES
//
// ARGUMENTS
//	int iUserID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::ObjectTree::Database::
revokeAll(int iUserID_)
{
	// Save ID of all database
	ModVector<Schema::Object::ID::Value> vecDatabaseID;

	{
		// Start transaction which is not concerned with any sessions
		Trans::AutoTransaction	pTrans(Trans::Transaction::attach());
		pTrans->begin(Schema::ObjectID::SystemTable,
					  Trans::Transaction::Mode(
						 Trans::Transaction::Category::ReadWrite,
						 Trans::Transaction::IsolationLevel::Serializable,
						 Boolean::False));

		// lock meta-database and system database table
		Manager::SystemTable::hold(*pTrans, Schema::Hold::Target::MetaDatabase,
								   Lock::Name::Category::Table,
								   Hold::Operation::ReadOnly);
		Manager::SystemTable::hold(*pTrans, Hold::Target::MetaTable,
								   Lock::Name::Category::Table,
								   Hold::Operation::ReadOnly);

		// get all databases
		const ModVector<Schema::Database*>& vecDatabase = get(*pTrans);
		ModSize n = vecDatabase.getSize();
		vecDatabaseID.reserve(n);
		for (ModSize i = 0; i < n; ++i) {
			vecDatabaseID.pushBack(vecDatabase[i]->getID());
		}
		// end the transaction
		pTrans->commit();
	}

	// for each database, call revokeall
	ModSize n = vecDatabaseID.getSize();
	for (ModSize i = 0; i < n; ++i) {
		// begin transaction
		Trans::AutoTransaction	pTrans(Trans::Transaction::attach());
		pTrans->begin(Schema::ObjectID::SystemTable,
					  Trans::Transaction::Mode(
						 Trans::Transaction::Category::ReadWrite,
						 Trans::Transaction::IsolationLevel::Serializable,
						 Boolean::False));

		// get database
		Schema::Database* pDatabase =
			Schema::Database::getLocked(*pTrans, vecDatabaseID[i],
										Lock::Name::Category::Tuple,
										Hold::Operation::ReadForWrite,
										Lock::Name::Category::Tuple,
										Hold::Operation::ReadForWrite);

		if (pDatabase) {
			// drop privilege
			SystemTable::ReorganizePrivilege::Drop cExecutor(*pTrans, pDatabase, iUserID_);
			cExecutor.execute();
		}

		// end the transaction
		// [NOTES]
		//	operation can be canceled only for one database.
		pTrans->commit();
	}
}

//////////////////////////////////////////
//	Schema::Manager::ObjectTree::Table	//
//////////////////////////////////////////

//	FUNCTION
//	Schema::Manager::ObjectTree::Table::hold --
//		表をロックする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//		Lock::Name::Category::Value eManipulate_
//			操作するオブジェクトのカテゴリー
//		Hold::Operation::Value eOperation_
//			操作の種類
//		Schema::Object::ID::Value iID_
//			ロックする表が属するデータベースのID
//		Schema::Object::ID::Value iID_
//			ロックする表のID
//
//	RETURN
//		bool
//
//	EXCEPTIONS

bool
Manager::ObjectTree::Table::
hold(Trans::Transaction& cTrans_,
	 Lock::Name::Category::Value eManipulate_,
	 Hold::Operation::Value eOperation_,
	 Object::ID::Value iDatabaseID_,
	 Object::ID::Value iTableID_,
	 Lock::Timeout::Value iTimeout_)
{
	return Hold::hold(cTrans_,
					  Lock::TableName(iDatabaseID_, iTableID_),
					  Hold::Target::Table,
					  eManipulate_, eOperation_,
					  iTimeout_);
}

//	FUNCTION
//	Schema::Manager::ObjectTree::Table::release --
//		表をアンロックする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//		Lock::Name::Category::Value eManipulate_
//			操作するオブジェクトのカテゴリー
//		Hold::Operation::Value eOperation_
//			操作の種類
//		Schema::Object::ID::Value iID_
//			ロックする表が属するデータベースのID
//		Schema::Object::ID::Value iID_
//			ロックする表のID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::ObjectTree::Table::
release(Trans::Transaction& cTrans_,
		Lock::Name::Category::Value eManipulate_,
		Hold::Operation::Value eOperation_,
		Object::ID::Value iDatabaseID_,
		Object::ID::Value iTableID_)
{
	Hold::release(cTrans_,
				  Lock::TableName(iDatabaseID_, iTableID_),
				  Hold::Target::Table,
				  eManipulate_, eOperation_);
}

//////////////////////////////////////
//	 Schema::Manager::ObjectName	//
//////////////////////////////////////

//	FUNCTION public
//	Schema::Manager::ObjectName::AutoWithdraw::~AutoWithdraw -- 
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
Manager::ObjectName::AutoWithdraw::
~AutoWithdraw()
{
	withdraw(m_pObject);
}

//	FUNCTION public
//	Schema::Manager::ObjectName::initialize -- 名前管理モジュールの初期化
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
Manager::ObjectName::
initialize()
{
	Os::AutoCriticalSection m(_Name::_latch);

	if (!_Name::_reserve)
		_Name::_reserve = new ModVector<const Object*>();
}

//	FUNCTION public
//	Schema::Manager::ObjectName::terminate -- 名前管理モジュールの後処理
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
Manager::ObjectName::
terminate()
{
	_Free(_Name::_reserve);
}

//	FUNCTION public
//	Schema::Manager::ObjectName::reserve --
//		作成中のオブジェクトとして登録する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object* pObj_
//			登録するオブジェクト
//
//	RETURN
//		trueの場合正しく登録できた
//		falseの場合すでに同じ名前のオブジェクトが作成中なので登録に失敗した
//
//	EXCEPTIONS

bool
Manager::ObjectName::
reserve(const Schema::Object* pObj_)
{
	Os::AutoCriticalSection m(_Name::_latch);

	// 念のために初期化しておく

	initialize();
	; _SYDNEY_ASSERT(_Name::_reserve);

	ModVector<const Object*>::Iterator iterator = _Name::_reserve->begin();
	const ModVector<const Object*>::Iterator& end = _Name::_reserve->end();
	for (; iterator != end; ++iterator) {
		const Schema::Object* pObj = *iterator;
		if (pObj_->getCategory() == pObj->getCategory()
			&& pObj_->getName() == pObj->getName()
			&& (pObj_->getCategory() == Object::Category::Index
				|| pObj_->getParentID() == pObj->getParentID())
										// 索引は表が異なっても名前の重複は許されない
			&& pObj_->getDatabaseID() == pObj->getDatabaseID())
			return false;
	}
	_Name::_reserve->pushBack(pObj_);
	return true;
}

//	FUNCTION public
//	Schema::Manager::ObjectName::withdraw --
//		作成中のオブジェクトとしての登録を解除する
//
//	NOTES
//		引数は登録に使用したオブジェクトでなければ解除されない
//
//	ARGUMENTS
//		Schema::Object* pObj_
//			登録を解除するオブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::ObjectName::
withdraw(const Schema::Object* pObj_)
{
	Os::AutoCriticalSection m(_Name::_latch);

	// 念のために初期化しておく

	initialize();
	; _SYDNEY_ASSERT(_Name::_reserve);

	ModVector<const Object*>::Iterator iterator = _Name::_reserve->find(pObj_);
	if (iterator != _Name::_reserve->end())
		_Name::_reserve->erase(iterator);
}

// FUNCTION public
//	Schema::Manager::ObjectName::getMaxLength -- 名称の長さの上限
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModSize
//
// EXCEPTIONS

ModSize
Manager::ObjectName::
getMaxLength()
{
	return _Name::_iMaxLength;
}

//////////////////////////////////////
//	 Schema::Manager::ObjectPath	//
//////////////////////////////////////

//	FUNCTION public
//	Schema::Manager::ObjectPath::AutoWithdraw::~AutoWithdraw -- 
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
Manager::ObjectPath::AutoWithdraw::
~AutoWithdraw()
{
	withdraw(m_pObject);
}

//	FUNCTION public
//	Schema::Manager::ObjectPath::initialize -- パス管理モジュールの初期化
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
Manager::ObjectPath::
initialize()
{
	Os::AutoCriticalSection m(_Path::_latch);

	if (!_Path::_reserve)
		_Path::_reserve = new ModVector<_Path::_Element>();
}

//	FUNCTION public
//	Schema::Manager::ObjectPath::terminate -- 名前管理モジュールの後処理
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
Manager::ObjectPath::
terminate()
{
	_Free(_Path::_reserve);
}

//	FUNCTION public
//	Schema::Manager::ObjectPath::reserve --
//		作成するパスとして登録する
//
//	NOTES
//		引数のパスがすでに登録されているかどうかのチェックはしない
//
//	ARGUMENTS
//		const Schema::Object* pObject_
//			登録するオブジェクト
//		const ModUnicodeString& cstrPath_
//		または const ModVector<ModUnicodeString>& vecPath_
//			登録するパス
//
//	RETURN
//		trueの場合正しく登録できた
//		falseの場合すでに同じパスが予約中なので登録に失敗した
//
//	EXCEPTIONS

bool
Manager::ObjectPath::
reserve(const Schema::Object* pObject_, const ModUnicodeString& cstrPath_)
{
	Os::AutoCriticalSection m(_Path::_latch);

	// 念のために初期化しておく

	initialize();
	; _SYDNEY_ASSERT(_Path::_reserve);

	ModVector<_Path::_Element>::Iterator iterator = _Path::_reserve->begin();
	const ModVector<_Path::_Element>::Iterator& end = _Path::_reserve->end();
	for (; iterator != end; ++iterator) {
		const ModVector<ModUnicodeString>& vecPath = (*iterator).second;
		ModSize n = vecPath.getSize();
		for (ModSize i = 0; i < n; ++i) {
			if (Os::Path::compare(cstrPath_, vecPath[i]) != Os::Path::CompareResult::Unrelated) {
				// 関連するパスがあった
				SydInfoMessage << "Path exists: " << cstrPath_
							   << " in " << (*iterator).first->getName()
							   << ModEndl;
				return false;
			}
		}
	}
	_Path::_reserve->pushBack(_Path::_Element(pObject_, ModVector<ModUnicodeString>(1, cstrPath_)));
	return true;
}

bool
Manager::ObjectPath::
reserve(const Schema::Object* pObject_, const ModVector<ModUnicodeString>& vecPath_)
{
	Os::AutoCriticalSection m(_Path::_latch);

	// 念のために初期化しておく

	initialize();
	; _SYDNEY_ASSERT(_Path::_reserve);

	ModVector<_Path::_Element>::Iterator iterator = _Path::_reserve->begin();
	const ModVector<_Path::_Element>::Iterator& end = _Path::_reserve->end();
	for (; iterator != end; ++iterator) {
		const ModVector<ModUnicodeString>& vecPath = (*iterator).second;
		ModSize n = vecPath.getSize();
		for (ModSize i = 0; i < n; ++i) {
			ModSize m = vecPath_.getSize();
			for (ModSize j = 0; j < m; ++j) {
				if (Os::Path::compare(vecPath_[j], vecPath[i]) != Os::Path::CompareResult::Unrelated) {
					// 関連するパスがあった
					SydInfoMessage << "Path exists: " << vecPath_[j]
								   << " in " << (*iterator).first->getName()
								   << ModEndl;
					return false;
				}
			}
		}
	}
	_Path::_reserve->pushBack(_Path::_Element(pObject_, vecPath_));
	return true;
}

//	FUNCTION public
//	Schema::Manager::ObjectPath::withdraw --
//		作成を予約したパスとしての登録を解除する
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Object* pObject_
//			登録していたオブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::ObjectPath::
withdraw(const Schema::Object* pObject_)
{
	Os::AutoCriticalSection m(_Path::_latch);

	if (_Path::_reserve) {
		ModVector<_Path::_Element>::Iterator iterator = _Path::_reserve->begin();
		const ModVector<_Path::_Element>::Iterator& end = _Path::_reserve->end();
		for (; iterator != end; ++iterator) {
			if ((*iterator).first == pObject_) {
				_Path::_reserve->erase(iterator);
				break;
			}
		}
	}
}

// FUNCTION public
//	Schema::Manager::ObjectPath::getMaxLength -- パス名の長さの上限
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModSize
//
// EXCEPTIONS

ModSize
Manager::ObjectPath::
getMaxLength()
{
	return _Path::_iMaxLength;
}

//////////////////////////////////////
//	 Schema::Manager::SystemTable	//
//////////////////////////////////////

// -----------------------------------------
// SystemTableの定義はReorganize.cppに移した
// -----------------------------------------

//////////////////////////////////////
//	Schema::Manager::ObjectSnapshot	//
//////////////////////////////////////

//	FUNCTION
//	Schema::Manager::ObjectSnapshot::initialize --
//		マネージャーのうち、スナップショット関連を初期化する
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
Manager::ObjectSnapshot::
initialize()
{
	using namespace _ObjectSnapshot;

	Os::AutoCriticalSection m(_latch);

	Schema::ObjectSnapshot::initialize();

	// マップを作る
	if (!_IsValidPointer(_sessions)) {
		_sessions = new SessionMap(_sessionMapSize, _sessionMapEnableLink);

		// スキーマ情報のキャッシュのための情報をリセットしておく
		reCache();
	}

	; _SYDNEY_ASSERT(_IsValidPointer(_sessions));
}

//	FUNCTION
//	Schema::Manager::ObjectSnapshot::terminate --
//		マネージャーのうち、スナップショット関連を終了処理する
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
Manager::ObjectSnapshot::
terminate()
{
	using namespace _ObjectSnapshot;

	// 最新のスナップショットを破棄する
	// ★注意★
	// 余計にaddReferenceしているので余計にreleaseする
	_readWriteSnapshot.release();
	_readWriteSnapshot = static_cast<Schema::ObjectSnapshot*>(0);

	// 現在のスナップショットを破棄する
	// ★注意★
	// 余計にaddReferenceしているので余計にreleaseする
	// ObjectPointerを使用しているのでSessionMapにあるポインターがあれば実体は残る
	_currentSnapshot.release();
	_currentSnapshot = static_cast<Schema::ObjectSnapshot*>(0);

	// 再構成により古くなったスナップショットを保持するクラスを解放
	_Free(_sessions);

	Schema::ObjectSnapshot::terminate();
}

//	FUNCTION
//	Schema::Manager::ObjectSnapshot::get --
//		システムに存在するスナップショットのうち、
//		あるセッションID に対応するスナップショットを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			スナップショットに対応するトランザクション記述子
//
//	RETURN
//		0以外
//			得られたスナップショットへのポインター
//		0
//			読み書きトランザクションなので作成されなかった
//
//	EXCEPTIONS

Schema::ObjectSnapshot*
Manager::ObjectSnapshot::
get(Trans::Transaction& cTrans_)
{
	using namespace _ObjectSnapshot;

	Os::AutoCriticalSection m(_latch);

	// 念のために初期化しておく
	initialize();

	if (cTrans_.isNoVersion()) {

		// 版を使わない場合は固定のものを使う
		if (!_readWriteSnapshot.get()) {
			_readWriteSnapshot = Schema::ObjectSnapshot::create(true /* isNoVersion */);
			// terminateを呼ばずに終了したときに不正なタイミングで
			// 解放されることのないように余計にaddReferenceしておく
			_readWriteSnapshot.addReference();
		}

		return _readWriteSnapshot.get();
	}

	; _SYDNEY_ASSERT(_IsValidPointer(_sessions));

	// セッションに対応する情報を取得する
	SessionMap::Iterator iterator = _sessions->find(cTrans_.getSessionID());

	if (iterator == _sessions->end()) {
		// 初めてスナップショットを取得しようとしている
		// -> セッションに対応するエントリーを登録する

		ModPair<SessionMap::Iterator, ModBoolean> cResult =
			_sessions->insert(cTrans_.getSessionID(),
							  _SessionMapEntry(),
							  ModTrue /* do not check existence  */);
		iterator = cResult.first;
	}

	// 作成した、またはマップから得たエントリー
	_SessionMapEntry& entry = SessionMap::getValue(iterator);

	// 最後に再構成をコミットしたときのトランザクションIDと比較し、
	// 最新のスナップショットを使えるか調べる

	if (_lastReorganize < cTrans_.getID()) {
		// 最後に再構成をコミットして以降に開始されたトランザクションである
		// -> 現在のスナップショットとしてよい

		if (!_currentSnapshot.get()) {
			// 現在値に対応するスナップショットを新規作成する
			_currentSnapshot = Schema::ObjectSnapshot::create();
			// terminateを呼ばずに終了したときに不正なタイミングで
			// 解放されることのないように余計にaddReferenceしておく
			_currentSnapshot.addReference();
		}
		// エントリーの登録を変更する
		if (entry.m_pSnapshot.get()
			&& (entry.m_pSnapshot.get() != _currentSnapshot.get())) {
			// 現在のスナップショットからこのセッションに関する部分を解放する
			entry.m_pSnapshot->releaseDatabase(cTrans_.getSessionID());
		}
		entry.m_pSnapshot = _currentSnapshot;
		entry.m_iTransactionID = cTrans_.getID();

	} else {
		// 再構成がコミットされる前に開始されたトランザクションである
		// ->トランザクションが変わっていない限りマップの値を使う

		if (entry.m_iTransactionID != cTrans_.getID()) {
			// トランザクションが変わっているので取得しなおす
			// ★注意★
			// セッション開始時のスナップショットはもう古くて使えない
			// 最新のものが対応しているとも限らない
			// したがって新しいスナップショットを登録する

			if (entry.m_pSnapshot.get()) {
				// 現在のスナップショットからこのセッションに関する部分を解放する
				entry.m_pSnapshot->releaseDatabase(cTrans_.getSessionID());
			}
			// 新たにスナップショットを作る
			ModAutoPointer<Schema::ObjectSnapshot> pTmp = Schema::ObjectSnapshot::create();

			// 現在の登録を変更する
			entry.m_pSnapshot = pTmp.release();
			entry.m_iTransactionID = cTrans_.getID();
		}
	}

	// 登録されているものを返す
	return entry.m_pSnapshot.get();
}

//	FUNCTION
//	Schema::Manager::ObjectSnapshot::erase --
//		システムに存在するスナップショットのうち、
//		あるセッションに対応するスナップショットを表すクラスを破棄する
//
//	NOTES
//		キャッシュと一緒に一時表は消さない
//
//	ARGUMENTS
//		Schema::SessionID iSessionID_
//			スナップショットに対応するセッションのID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::ObjectSnapshot::
erase(SessionID iSessionID_)
{
	using namespace _ObjectSnapshot;

	{
	SnapshotPointer pSnapshot;

	// releaseDatabaseはlatchの外で行う
	{
	Os::AutoCriticalSection m(_latch);

	// セッションで使用された可能性のあるスナップショットについて
	// 終了処理を行う

	// まずは最新版
	if (_readWriteSnapshot.get())
		pSnapshot = _readWriteSnapshot;

	} // scope for AutoCriticalSection

	if (pSnapshot.get())
		pSnapshot->releaseDatabase(iSessionID_);

	} // Scope for pSnapshot

	{
	SnapshotPointer pSnapshot;

	// 次に古い版
	// releaseDatabaseはlatchの外で行う
	{
	Os::AutoCriticalSection m(_latch);

	if (_sessions) {
		// セッションに対応するスナップショット情報を得る
		SessionMap::Iterator iterator = _sessions->find(iSessionID_);
		if (iterator != _sessions->end()) {
			_SessionMapEntry& entry = SessionMap::getValue(iterator);

			// releaseDatabaseするSnapshotをセットし、latchの外でreleaseDatabaseを行う
			pSnapshot = entry.m_pSnapshot;

			// エントリーを破棄する
			_sessions->erase(iterator);
		}
	}
	} // scope for AutoCriticalSection

	if (pSnapshot.get())
		pSnapshot->releaseDatabase(iSessionID_);

	} // Scope for pSnapshot
}

//	FUNCTION
//	Schema::Manager::ObjectSnapshot::eraseCurrent --
//		現在のタイムスタンプに対応するスナップショットを無効化する
//
//	NOTES
//		キャッシュと一緒に一時表は消さない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::ObjectSnapshot::
eraseCurrent()
{
	using namespace _ObjectSnapshot;

	Os::AutoCriticalSection m(_latch);

	if (_currentSnapshot.get()) {
		// ★注意★
		// 余計にaddReferenceしているので余計にreleaseする
		// ObjectPointerを使用しているので_sessionsにあるポインターがあれば実体は残る

		_currentSnapshot.release();
		_currentSnapshot = static_cast<Schema::ObjectSnapshot*>(0);
	}
}

// FUNCTION public
//	Schema::Manager::ObjectSnapshot::eraseDatabase -- すべてのスナップショットからデータベースを消す
//
// NOTES
//
// ARGUMENTS
//	const Schema::Object::ID& cDatabaseID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::ObjectSnapshot::
eraseDatabase(const Schema::Object::ID& cDatabaseID_, Server::SessionID iSessionID_)
{
	using namespace _ObjectSnapshot;

	Os::AutoCriticalSection m(_latch);
	// まずは最新版
	if (_readWriteSnapshot.get())
		_readWriteSnapshot->eraseDatabase(cDatabaseID_, iSessionID_);

	// 次に古い版
	if (_sessions) {
		// セッションに対応するスナップショット情報を得る
		SessionMap::Iterator iterator = _sessions->begin();
		const SessionMap::Iterator last = _sessions->end();
		for (; iterator != last; ++iterator) {
			_SessionMapEntry& entry = SessionMap::getValue(iterator);
			entry.m_pSnapshot->eraseDatabase(cDatabaseID_, iSessionID_);
		}
	}	
}

// FUNCTION public
//	Schema::Manager::ObjectSnapshot::eraseTable -- すべてのスナップショットから表を消す
//
// NOTES
//
// ARGUMENTS
//	const Schema::Object::ID& cDatabaseID_
//	const Schema::Object::ID& cTableID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::ObjectSnapshot::
eraseTable(const Schema::Object::ID& cDatabaseID_,
		   const Schema::Object::ID& cTableID_)
{
	using namespace _ObjectSnapshot;

	Os::AutoCriticalSection m(_latch);
	// 最新版からはTable::doAfterPersistで削除されるので古い版のみ処理する
	if (_sessions) {
		// セッションに対応するスナップショット情報を得る
		SessionMap::Iterator iterator = _sessions->begin();
		const SessionMap::Iterator last = _sessions->end();
		for (; iterator != last; ++iterator) {
			_SessionMapEntry& entry = SessionMap::getValue(iterator);
			entry.m_pSnapshot->eraseTable(cDatabaseID_, cTableID_);
		}
	}	
}

// FUNCTION public
//	Schema::Manager::ObjectSnapshot::eraseIndex -- すべてのスナップショットから索引を消す
//
// NOTES
//
// ARGUMENTS
//	const Schema::Object::ID& cDatabaseID_
//	const Schema::Object::ID& cTableID_
//	const Schema::Object::ID& cIndexID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::ObjectSnapshot::
eraseIndex(const Schema::Object::ID& cDatabaseID_,
		   const Schema::Object::ID& cTableID_,
		   const Schema::Object::ID& cIndexID_)
{
	using namespace _ObjectSnapshot;

	Os::AutoCriticalSection m(_latch);
	// 最新版からはIndex::doAfterPersistで削除されるので古い版のみ処理する
	if (_sessions) {
		// セッションに対応するスナップショット情報を得る
		SessionMap::Iterator iterator = _sessions->begin();
		const SessionMap::Iterator last = _sessions->end();
		for (; iterator != last; ++iterator) {
			_SessionMapEntry& entry = SessionMap::getValue(iterator);
			entry.m_pSnapshot->eraseIndex(cDatabaseID_, cTableID_, cIndexID_);
		}
	}	
}

// FUNCTION public
//	Schema::Manager::ObjectSnapshot::reCache -- スキーマ情報に変化があったときコミット後にキャッシュの更新をする
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
Manager::ObjectSnapshot::
reCache()
{
	using namespace _ObjectSnapshot;

	Os::AutoCriticalSection m(_latch);

	// get new transaction Id
	_lastReorganize = Trans::ID::assign();
	// erase current snapshot
	eraseCurrent();
}

///////////////////////
// Manager::Recovery //
///////////////////////

//	FUNCTION
//	Schema::Manager::Recovery::notifyDone
//		-- リカバリーの後処理を行う
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//
//	EXCEPTIONS

void
Manager::Recovery::
notifyDone()
{
	RecoveryUtility::terminate();
}

//	FUNCTION
//	Schema::Manager::Recovery::notifyBegin --
//		マウント時の回復処理の開始を通知する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database&	database
//			マウントするデータベースを表すデータベースオブジェクト
//
//	RETURN
//
//	EXCEPTIONS

void
Manager::Recovery::
notifyBegin(Schema::Database& database)
{
	RecoveryUtility::initialize(database);
}

//	FUNCTION
//	Schema::Manager::Recovery::notifyDone --
//		マウント時の回復処理の終了を通知する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database&	database
//			マウントするデータベースを表すデータベースオブジェクト
//
//	RETURN
//
//	EXCEPTIONS

void
Manager::Recovery::
notifyDone(Schema::Database& database)
{
	RecoveryUtility::terminate(database);
}

//	FUNCTION
//	Schema::Manager::getCriticalSection --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS

Os::CriticalSection&
Manager::
getCriticalSection()
{
	return _cCriticalSection;
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2013, 2017, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
