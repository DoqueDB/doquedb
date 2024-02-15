// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Sequence.cpp -- シーケンス関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2010, 2013, 2023 Ricoh Company, Ltd.
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

#include "Schema/AutoLatch.h"
#include "Schema/Database.h"
#include "Schema/Default.h"
#include "Schema/FakeError.h"
#include "Schema/Sequence.h"
#include "Schema/Message.h"
#include "Schema/Utility.h"
#include "Schema/Message_SequenceNotExist.h"
#include "Schema/Message_SequenceCorrectFailed.h"
#include "Schema/Message_SequenceCreateFailed.h"
#include "Schema/Message_SequenceReadFailed.h"
#include "Schema/Message_SequenceValueNotMatch.h"

#ifdef OBSOLETE
#include "Checkpoint/FileDestroyer.h"
#endif

#include "Common/Assert.h"
#include "Common/IntegerArrayData.h"
#include "Common/Message.h"
#include "Common/Thread.h"

#include "Buffer/Pool.h"

#include "Exception/Object.h"
#include "Exception/RecoveryFailed.h"
#include "Exception/SequenceLimitExceeded.h"

#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"

#include "PhysicalFile/File.h"
#include "PhysicalFile/Manager.h"
#include "PhysicalFile/Page.h"
#include "PhysicalFile/Types.h"

#include "Statement/ValueExpression.h"

#include "ModAutoPointer.h"
#include "ModHashMap.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

#define	_BEGIN_RECOVERY	\
						try {
#define _END_RECOVERY \
						} catch (Exception::Object& e) { \
							SydErrorMessage << "Error recovery failed. FATAL. " << e << ModEndl; \
							/* データベースを使用不可能にする*/ \
							Schema::Database::setAvailability(m_iDatabaseID, false); \
							/* エラー処理中に発生した例外は再送しない */ \
							/* thru. */ \
						} catch (...) { \
							SydErrorMessage << "Error recovery failed. FATAL." << ModEndl; \
							/* データベースを使用不可能にする*/ \
							Schema::Database::setAvailability(m_iDatabaseID, false); \
							/* エラー処理中に発生した例外は再送しない */ \
							/* thru. */ \
						}

namespace {

	namespace _Temporary {

		//	VARIABLE local
		//	_Temporary::_criticalSection --
		//
		//	NOTES

		Os::CriticalSection _criticalSection;

		//	TYPEDEF local
		//	_Temporary::_SequenceMap -- 一時オブジェクト用のシーケンスマップの型
		//
		//	NOTES

		typedef ModPair<ObjectID::Value, ObjectID::Value> IDPair;
		typedef ModPair<ObjectID::Value, IDPair> IDTripler;
		typedef ModMap<IDTripler, Sequence::Value, ModLess<IDTripler> > _SequenceMap;

		IDTripler createKey(ObjectID::Value iID1_, ObjectID::Value iID2_, ObjectID::Value iID3_)
		{
			return IDTripler(iID1_, IDPair(iID2_, iID3_));
		}

		//	VARIABLE local
		//	_Temporary::_map -- 一時オブジェクト用のシーケンスマップ
		//
		//	NOTES

		ModAutoPointer<_SequenceMap> _map = 0;

		// SequenceMapのコンストラクターに与えるパラメーター
		ModSize _sequenceMapSize = 41; // 1ユーザーが同時に使う一時表は2つ、推奨同時ユーザー数20
		ModBoolean _sequenceMapEnableLink = ModFalse; // Iterationしない

		//	FUNCTION local
		//	_Temporary::_initialize --
		//
		//	NOTES

		void
		_initialize()
		{
			if (!_map.get()) _map = new _SequenceMap;
		}

		//	FUNCTION local
		//	_Temporary::_entry --
		//
		//	NOTES

		Sequence::Value&
		_entry(const IDTripler& cID_)
		{
			Os::AutoCriticalSection m(_criticalSection);
			_initialize();

			_SequenceMap::Iterator iterator = _map->find(cID_);
			if (iterator == _map->end()) {
				iterator = _map->insert(cID_, Sequence::Value()).first;
			}
			return (*iterator).second;
		}

		//	FUNCTION local
		//	_Temporary::_erase --
		//
		//	NOTES

		void
		_erase(const IDTripler& cID_)
		{
			Os::AutoCriticalSection m(_criticalSection);
			_initialize();

			_map->erase(cID_);
		}

	} // namespace _Temporary

	namespace _Verification
	{
		const ModUnicodeString _cstrPath;	// スキーマのpathは空文字列

		//	CLASS local
		//	_Verification::_AutoVerification
		//		-- 自動的にendVerificationするためのクラス
		//
		//	NOTES

		class _AutoVerification : public Common::Object
		{
		public:
			_AutoVerification(PhysicalFile::File* pFile_, PhysicalFile::Page*& pPage_,
							 Trans::Transaction& cTrans_,
							 Admin::Verification::Treatment::Value eTreatment_,
							 Admin::Verification::Progress& cResult_)
				: m_pFile(pFile_), m_pPage(pPage_),
				  m_cTrans(cTrans_), m_eTreatment(eTreatment_),
				  m_cResult(cResult_), m_cProgress(cResult_.getConnection()), m_bTerminated(false)
			{}
			~_AutoVerification()
			{
				terminate();
			}
			void terminate()
			{
				if (!m_bTerminated) {
					m_bTerminated = true; // 以下の処理で例外が発生しても二重にterminateを呼ばない
					if (m_pPage) {
						m_pFile->detachPage(m_pPage, PhysicalFile::Page::UnfixMode::NotDirty);
					}

					// ここまでの結果を格納する
					m_cResult += m_cProgress;

					// 途中でisGoodでなくなってもendVerificationは呼ぶので
					// Progressを新たに作る
					Admin::Verification::Progress cTmp(m_cResult.getConnection());
					m_pFile->endVerification(m_cTrans, cTmp);
					m_cResult += cTmp;
				}
			}

			void startVerification()
			{
				m_pFile->startVerification(m_cTrans, m_eTreatment, m_cProgress);
			}
			void verifyPage()
			{
				if (isGood()) {
					m_pPage = m_pFile->verifyPage(m_cTrans,
												  Buffer::Page::FixMode::ReadOnly,
												  m_cProgress);
				}
			}
			bool isGood()
			{
				return m_cProgress.isGood();
			}
		protected:
		private:
			PhysicalFile::File* 		m_pFile;
			PhysicalFile::Page*&		m_pPage;
			Trans::Transaction&			m_cTrans;
			Admin::Verification::Treatment::Value m_eTreatment;
			Admin::Verification::Progress& m_cResult;
			Admin::Verification::Progress m_cProgress;
			bool						m_bTerminated;
		};
	} // namespace _Verification

	//	CLASS
	//	$$$::_AutoLatch -- Sequence ファイル専用の自動ラッチクラス
	//
	//	NOTES

	class _AutoLatch
		: public	AutoLatch
	{
	public:
		_AutoLatch(Trans::Transaction& cTrans_,
				   ObjectID::Value iDatabaseID_,
				   ObjectID::Value iTableID_,
				   ObjectID::Value iColumnID_,
				   bool bForce_ = false)

			// ロック名用のスキーマオブジェクト識別子は以下のようにして求める
			//  Previous implementation always used only DatabaseID for lockname.
			//	However, such lockname decrease insertion performance in multi user environment.
			//	When TableID_ and ColumnID_ is valid, those should be used.
			//	Moreover, ColumnID_ will never conflict with other FileIDs,
			//	so the value itself can be used.

			 : AutoLatch(
				 cTrans_, Lock::FileName(
					 iDatabaseID_,
					 (iTableID_ == ObjectID::Invalid)
					  ? ObjectID::Invalid - Schema::Object::Category::ValueNum
					  : iTableID_,
					 (iColumnID_ == ObjectID::Invalid) ? ObjectID::Invalid : iColumnID_),
					 bForce_)
		{}
	};

} // namespace

/////////////////////////
// Sequence::Valueの定義
/////////////////////////

// FUNCTION public
//	Schema::Sequence::Value::operator== -- 
//
// NOTES
//
// ARGUMENTS
//	const Value& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Sequence::Value::
operator==(const Value& cOther_) const
{
	return ((m_bSigned == cOther_.m_bSigned)
			&& ((m_bSigned && (m_iValue == cOther_.m_iValue))
				|| (!m_bSigned && (m_uValue == cOther_.m_uValue))))
		||
		((!m_bSigned && cOther_.m_bSigned)
		 && (cOther_.m_iValue >= 0)
		 && (m_uValue == cOther_.getUnsigned()))
		||
		((m_bSigned && !cOther_.m_bSigned)
		 && (m_iValue >= 0)
		 && (getUnsigned() == cOther_.m_uValue));
}

// FUNCTION public
//	Schema::Sequence::Value::operator< -- 
//
// NOTES
//
// ARGUMENTS
//	const Value& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Sequence::Value::
operator<(const Value& cOther_) const
{
	return ((m_bSigned == cOther_.m_bSigned)
			&& ((m_bSigned && (m_iValue < cOther_.m_iValue))
				|| (!m_bSigned && (m_uValue < cOther_.m_uValue))))
		||
		((!m_bSigned && cOther_.m_bSigned)
		 && (cOther_.m_iValue >= 0)
		 && (m_uValue < cOther_.getUnsigned()))
		||
		((m_bSigned && !cOther_.m_bSigned)
		 && ((m_iValue < 0)
			 || (getUnsigned() < cOther_.m_uValue)));
}

// FUNCTION public
//	Schema::Sequence::Value::isAbleToAdd -- 値を加えられるか
//
// NOTES
//
// ARGUMENTS
//	Signed::Value iIncrement_
//	const Value& cMax_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Sequence::Value::
isAbleToAdd(Signed::Value iIncrement_, const Value& cMax_) const
{
	// thisは少なくともcMax_と同じ型にキャスト可能であるとする
	if (iIncrement_ > 0) {
		return ((cMax_.m_bSigned
				 && (getSigned() <= cMax_.m_iValue - iIncrement_))
				||
				(!cMax_.m_bSigned
				 && (cMax_.m_uValue >= static_cast<Unsigned::Value>(iIncrement_))
				 && (getUnsigned() <= cMax_.m_uValue - iIncrement_)));
	} else {
		; _SYDNEY_ASSERT(iIncrement_ < 0);
		return ((cMax_.m_bSigned
				 && (getSigned() >= cMax_.m_iValue - iIncrement_))
				||
				(!cMax_.m_bSigned
				 && (cMax_.m_uValue <= ModUInt32Max + iIncrement_)
				 && (getUnsigned() >= cMax_.m_uValue - iIncrement_)));
	}
}

// FUNCTION public
//	Schema::Sequence::Value::getUnsigned -- 値の取得(unsigned)
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Sequence::Unsigned::Value
//
// EXCEPTIONS

Sequence::Unsigned::Value
Sequence::Value::
getUnsigned() const
{
	if (m_bSigned) {
		return static_cast<Unsigned::Value>(m_iValue);
	} else {
		return m_uValue;
	}
}

// FUNCTION public
//	Schema::Sequence::Value::getSigned -- 値の取得(signed)
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Sequence::Signed::Value
//
// EXCEPTIONS

Sequence::Signed::Value
Sequence::Value::
getSigned() const
{
	if (!m_bSigned) {
		return static_cast<Signed::Value>(m_uValue);
	} else {
		return m_iValue;
	}
}

// 物理ファイルに書き込む
void
Sequence::Value::
write(PhysicalFile::Page* pPage_, Trans::Transaction& cTrans_)
{
	if (m_bSigned) {
		Signed::Value iValue = m_iValue;
		pPage_->write(cTrans_, &iValue, 0, sizeof(Signed::Value));
	} else {
		Unsigned::Value uValue = m_uValue;
		pPage_->write(cTrans_, &uValue, 0, sizeof(Unsigned::Value));
	}
}

// 物理ファイルから読み込む
void
Sequence::Value::
read(PhysicalFile::Page* pPage_, Trans::Transaction& cTrans_)
{
	if (m_bSigned) {
		Signed::Value iValue = 0;
		pPage_->read(cTrans_, &iValue, 0, sizeof(Signed::Value));
		m_iValue = iValue;
	} else {
		Unsigned::Value uValue = 0;
		pPage_->read(cTrans_, &uValue, 0, sizeof(Unsigned::Value));
		m_uValue = uValue;
	}
}

/////////////////////////

//	FUNCTION public
//	Schema::Sequence::Sequence --
//		シーケンスを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をしようとしているトランザクション記述子
//		ObjectID::Value databaseID_,
//			このファイルが属するデータベースのＩＤ
//		Schema::Sequence::Unsigned::Value uMax_
//			シーケンスの値の最大値
//		bool bMounted_
//			Mount されているか
//				true  : Mount されている
//				false : Unmount さている
//		Os::Path&		path
//			シーケンスの値を格納するためのファイルのパス名
//		Schema::Object::Scope::Value eScope_
//			シーケンスが属するスキーマオブジェクトのスコープ
//		bool bReadOnly_
//			読み取り専用か
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Sequence::
Sequence(const Os::Path& cPath_,
		 ObjectID::Value databaseID_,
		 Unsigned::Value uMax_,
		 bool bMounted_,
		 Schema::Object::Scope::Value eScope_,
		 bool bReadOnly_)
	: m_pFile(0), m_pPage(0),
	  m_iDatabaseID(databaseID_), m_iTableID(ObjectID::Invalid), m_iColumnID(ObjectID::Invalid),
	  m_cValue(), m_cMin(0), m_cMax(uMax_),
	  m_cInit(0), m_iIncrement(1), m_bCycle(false), m_bGetMax(false),
	  m_cPath(cPath_), m_bLoaded(false), m_bDirty(false), m_cLatch()
{
	if (eScope_ == Schema::Object::Scope::Permanent) {
		attachFile(cPath_, bMounted_, eScope_, bReadOnly_);
	}
}

Sequence::
Sequence(const Os::Path& cPath_,
		 ObjectID::Value iDatabaseID_,
		 ObjectID::Value iTableID_,
		 ObjectID::Value iColumnID_,
		 Unsigned::Value uMax_,
		 bool bMounted_,
		 Schema::Object::Scope::Value eScope_,
		 bool bReadOnly_)
	: m_pFile(0), m_pPage(0),
	  m_iDatabaseID(iDatabaseID_), m_iTableID(iTableID_), m_iColumnID(iColumnID_),
	  m_cValue(), m_cMin(0), m_cMax(uMax_),
	  m_cInit(0), m_iIncrement(1), m_bCycle(false), m_bGetMax(false),
	  m_cPath(cPath_), m_bLoaded(false), m_bDirty(false), m_cLatch()
{
	if (eScope_ == Schema::Object::Scope::Permanent) {
		attachFile(cPath_, bMounted_, eScope_, bReadOnly_);
	}
}

Sequence::
Sequence(const Os::Path& cPath_,
		 ObjectID::Value iDatabaseID_,
		 ObjectID::Value iTableID_,
		 ObjectID::Value iColumnID_,
		 const Default& cDefault_,
		 bool bMounted_,
		 Schema::Object::Scope::Value eScope_,
		 bool bReadOnly_)
	: m_pFile(0), m_pPage(0),
	  m_iDatabaseID(iDatabaseID_), m_iTableID(iTableID_), m_iColumnID(iColumnID_),
	  m_cValue(), m_cMin(), m_cMax(),
	  m_cInit(), m_iIncrement(1), m_bCycle(false), m_bGetMax(false),
	  m_cPath(cPath_), m_bLoaded(false), m_bDirty(false), m_cLatch()
{
	// Defaultの内容からメンバーの値をセットする
	setArgument(cDefault_);

	if (eScope_ == Schema::Object::Scope::Permanent) {
		attachFile(cPath_, bMounted_, eScope_, bReadOnly_);
	}
}

//	FUNCTION public
//	Schema::Sequence::attachFile --
//		シーケンスの値を保持する物理ファイルをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&		path
//			シーケンスの値を格納するためのファイルのパス名
//		bool bMount_ (= true)
//			Mount されているかどうか
//		Schema::Object::Scope::Value eScope_
//			シーケンスが属するスキーマオブジェクトのスコープ
//		bool bReadOnly_
//			読み取り専用か
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Sequence::
attachFile(const Os::Path& cPath_,
		   bool bMount_,
		   Schema::Object::Scope::Value eScope_,
		   bool bReadOnly_)
{
	; _SYDNEY_ASSERT(eScope_ == Schema::Object::Scope::Permanent);

	using namespace PhysicalFile;

	Os::AutoCriticalSection l(m_cLatch);

	// attachしていたらdetachする
	detachFile();

	// 物理ファイルの格納戦略とバッファリング戦略を設定する
	PhysicalFile::File::StorageStrategy		storageStrategy;
	PhysicalFile::File::BufferingStrategy	bufferingStrategy;

	storageStrategy.m_PhysicalFileType = PhysicalFile::NonManageType;
	storageStrategy.m_PageUseRate = PhysicalFile::ConstValue::DefaultPageUseRate;
	storageStrategy.m_VersionFileInfo._mounted = bMount_;
	storageStrategy.m_VersionFileInfo._pageSize = sizeof(Unsigned::Value);
	storageStrategy.m_VersionFileInfo._path._masterData = cPath_;
	storageStrategy.m_VersionFileInfo._path._versionLog = cPath_;
	storageStrategy.m_VersionFileInfo._path._syncLog = cPath_;
	storageStrategy.m_VersionFileInfo._sizeMax._masterData = sizeof(Unsigned::Value);
	storageStrategy.m_VersionFileInfo._sizeMax._versionLog = sizeof(Unsigned::Value);
	storageStrategy.m_VersionFileInfo._sizeMax._syncLog = sizeof(Unsigned::Value);
	storageStrategy.m_VersionFileInfo._extensionSize._masterData = sizeof(Unsigned::Value);
	storageStrategy.m_VersionFileInfo._extensionSize._versionLog = sizeof(Unsigned::Value);
	storageStrategy.m_VersionFileInfo._extensionSize._syncLog = sizeof(Unsigned::Value);

	if (bReadOnly_) {
		// 読み取り専用なら無条件に読み取り専用のバッファーを使う
		bufferingStrategy.m_VersionFileInfo._category = Buffer::Pool::Category::ReadOnly;

	} else {
		// 読み取り専用でないなら一時データベースに属するかで異なる
		switch (eScope_) {
		case Schema::Object::Scope::SessionTemporary:
		case Schema::Object::Scope::Meta:
			bufferingStrategy.m_VersionFileInfo._category = Buffer::Pool::Category::Temporary;
			break;
		case Schema::Object::Scope::Permanent:
		case Schema::Object::Scope::LocalTemporary:
		case Schema::Object::Scope::GlobalTemporary:
		default:
			bufferingStrategy.m_VersionFileInfo._category = Buffer::Pool::Category::Normal;
			break;
		}
	}

	//【注意】	表のオブジェクト識別子は $$$::_AutoLatch を参考のこと
	//
	//			ファイルのオブジェクト識別子は不定でロック名を作っているが、
	//			現状の下位モジュールの仕様では問題ないはず

	m_pFile = PhysicalFile::Manager::attachFile(
		storageStrategy, bufferingStrategy,
		Lock::FileName(m_iDatabaseID,
					   (m_iTableID == ObjectID::Invalid)
					   ? ObjectID::Invalid - Schema::Object::Category::ValueNum
					   : m_iTableID,
					   (m_iColumnID == ObjectID::Invalid) ? ObjectID::Invalid : m_iColumnID));
}

//	FUNCTION public
//	Schema::Sequence::~Sequence --
//		シーケンスを表すクラスのデストラクター
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

Sequence::
~Sequence()
{
	detachFile();
}

// FUNCTION public
//	Schema::Sequence::isAscending -- Ascending Sequenceかを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Sequence::
isAscending() const
{
	return m_iIncrement > 0;
}

// FUNCTION public
//	Schema::Sequence::isGetMax -- 値を明示されても整合するかを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Sequence::
isGetMax() const
{
	return m_bGetMax;
}

//	FUNCTION public
//	Schema::Sequence::detachFile --
//		シーケンスの値を保持するファイルを表すクラスをデタッチする
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
Sequence::
detachFile()
{
	Os::AutoCriticalSection l(m_cLatch);

	if (m_pFile) {
		; _SYDNEY_ASSERT(!m_pPage);
		PhysicalFile::Manager::detachFile(m_pFile);
	}
}

//	FUNCTION public
//	Schema::Sequence::create -- シーケンスを定義する
//
//	NOTES
//		シーケンスの定義によって、その値を格納するファイルが生成される
//		シーケンスの値を格納するファイルは、オープンされたままになる
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//		const Schema::Sequence::Value&	cInit_
//			指定されたとき
//				シーケンスの初期値
//			指定されないとき
//				Schema::Sequence::Invalid が指定されたものとみなす
//		bool bAllowExistence_ = false
//			存在していてもエラーにしない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Sequence::
create(Trans::Transaction& cTrans_, const Value& cInit_, bool bAllowExistence_)
{
	Os::AutoCriticalSection l(m_cLatch);

	// 初期値を値とする
	m_cValue = cInit_;

	if (m_pFile) {
		_AutoLatch	latch(cTrans_, m_iDatabaseID, m_iTableID, m_iColumnID);
		if (bAllowExistence_ && !isVacant()) {
			// 存在していたら破棄する
			m_pFile->destroy(cTrans_);
		}

		m_bLoaded = true;

	} else {
		// m_pFileの設定がされていないとは一時オブジェクトであることを意味する

		// マップに初期値を記録する
		_Temporary::_entry(_Temporary::createKey(m_iDatabaseID, m_iTableID, m_iColumnID)) = m_cValue;
	}
}

void
Sequence::
substantiate(Trans::Transaction& cTrans_, const Value& cInit_)
{
	; _SYDNEY_ASSERT(isVacant());

	// 呼び出し側で排他している

	// 初期値を指定された値にする
	m_cValue = cInit_;

	// エラー処理に用いるための処理進行を表すenum型
	enum {
		None,
		DirectoryCreated,
		FileCreated,
		PageAttached,
		ValueWrote,
		ValueNum
	} eStatus = None;

	// 物理ファイルを格納するディレクトリーを設定する
	Utility::File::AutoRmDir cAutoRmDir;
	cAutoRmDir.setDir(m_cPath);

	try {
		eStatus = DirectoryCreated;
		SCHEMA_FAKE_ERROR("Schema::Sequence", "Create", "Directory");

		// 物理ファイルを生成する
		_AutoLatch	latch(cTrans_, m_iDatabaseID, m_iTableID, m_iColumnID);
		m_pFile->create(cTrans_);
		eStatus = FileCreated;
		SCHEMA_FAKE_ERROR("Schema::Sequence", "Create", "File");

		// 生成されたファイルにシーケンスの初期値を記録する

		// NonManageTypeの物理ファイルなのでページの割り当ては不要
		m_pPage = m_pFile->attachPage(cTrans_, Buffer::Page::FixMode::Write);
		; _SYDNEY_ASSERT(m_pPage);
		eStatus = PageAttached;
		SCHEMA_FAKE_ERROR("Schema::Sequence", "Create", "PageAttached");

		// ページに書き込む
		m_cValue.write(m_pPage, cTrans_);
		eStatus = ValueWrote;
		SCHEMA_FAKE_ERROR("Schema::Sequence", "Create", "ValueWrote");

		// 書き込んだのでDirtyである
		m_pFile->detachPage(m_pPage, PhysicalFile::Page::UnfixMode::Dirty);
		eStatus = FileCreated;
		SCHEMA_FAKE_ERROR("Schema::Sequence", "Create", "PageDetached");

	} catch (...) {

		_BEGIN_RECOVERY;

		_AutoLatch	latch(cTrans_, m_iDatabaseID, m_iTableID, m_iColumnID);
		// 進行状況により取り消し処理をする
		switch (eStatus) {
		case PageAttached:
			// writeに失敗しているのでDirtyではない
			m_pFile->detachPage(m_pPage, PhysicalFile::Page::UnfixMode::NotDirty);
			// thru.
		case ValueWrote:
			// detachPageに失敗しているので何もしようがないが、再度試みてみる
			if (m_pPage)
				m_pFile->detachPage(m_pPage, PhysicalFile::Page::UnfixMode::Dirty);
			// thru.
		case FileCreated:
			m_pFile->destroy(cTrans_);
			// thru.
		case DirectoryCreated:
#ifdef OBSOLETE
			if (bDirectoryCreated) {
				Utility::File::rmAll(m_cPath);
			}
#endif
		case None:
		default:
			break;
		}

		_END_RECOVERY;

		_SYDNEY_RETHROW;
	}
	// 成功したのでエラー処理を解除する
	cAutoRmDir.disable();
}

//	FUNCTION public
//	Schema::Sequence::drop -- シーケンスを破棄する
//
//	NOTES
//		シーケンスの破棄によって、その値を格納するファイルが破棄される
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//
//		bool bForce_ = true
//			trueの場合チェックポイントを待たずにすぐに削除する
//			falseの場合システムパラメーターによる
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Sequence::
drop(Trans::Transaction& cTrans_, bool bForce_)
{
	Os::AutoCriticalSection l(m_cLatch);

	if (m_pFile) {
		; _SYDNEY_ASSERT(!m_pPage);

		// マウントの有無や実体の存在の有無を確認せずに
		// とにかく削除する
		//
		//【注意】	そうしないと下位層で管理している
		//			情報がメンテナンスされない

		// ★注意★
		// Sequenceファイルをdropするときは
		// これを参照するリードトランザクションは存在しないことが
		// 保証されているので直接destroyしても構わない
#ifdef OBSOLETE
		// ★修正★
		// 物理ファイルの削除は場合により
		// Checkpoint::FileDestroyer を使用することになった

		if (true /*bForce_*/) {
			// すぐに削除する
#endif
			_AutoLatch	latch(cTrans_, m_iDatabaseID, m_iTableID, m_iColumnID);
			m_pFile->destroy(cTrans_);
#ifdef OBSOLETE
		} else
			// Checkpoint 処理を待つ
			Checkpoint::FileDestroyer::enter(cTrans_, *m_pFile);
#endif
		// 物理ファイルを格納するディレクトリーを破棄する
		// ファイルは存在しないはずなのでディレクトリーも即座に破棄できる
#ifdef OBSOLETE
		// ここも場合により
		// Checkpoint::FileDestroyer を使用する

		if (true /*bForce_*/)
#endif
			// すぐに削除する
			Utility::File::rmAll(m_cPath);
#ifdef OBSOLETE
		else
			// Checkpoint 処理を待つ
			Checkpoint::FileDestroyer::enter(m_cPath, true);
#endif
	} else {
		// m_pFileの設定がされていないとは一時オブジェクトであることを意味する

		// マップからエントリーを消去する
		_Temporary::_erase(_Temporary::createKey(m_iDatabaseID, m_iTableID, m_iColumnID));
	}
}

//	FUNCTION public
//	Schema::Sequence::mount -- ファイルを mount する
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
Sequence::
mount(Trans::Transaction& cTrans_)
{
	Os::AutoCriticalSection l(m_cLatch);

	if (m_pFile) {
		; _SYDNEY_ASSERT(!m_pPage);

		_AutoLatch	latch(cTrans_, m_iDatabaseID, m_iTableID, m_iColumnID);

		if (!isVacant())
			m_pFile->mount(cTrans_);
	}
}

//	FUNCTION public
//	Schema::Sequence::unmount -- ファイルを unmount する
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
Sequence::
unmount(Trans::Transaction& cTrans_)
{
	Os::AutoCriticalSection l(m_cLatch);

	if (m_pFile) {
		; _SYDNEY_ASSERT(!m_pPage);

		// マウントの有無や実体の存在の有無を確認せずに
		// とにかくアンマウントする
		//
		//【注意】	そうしないと下位層で管理している
		//			情報がメンテナンスされない

		_AutoLatch	latch(cTrans_, m_iDatabaseID, m_iTableID, m_iColumnID);

		m_pFile->unmount(cTrans_);
	}
}

//	FUNCTION public
//	Schema::Sequence::flush -- ファイルをフラッシュする
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
Sequence::
flush(Trans::Transaction& cTrans_)
{
	Os::AutoCriticalSection l(m_cLatch);

	if (m_pFile) {
		; _SYDNEY_ASSERT(!m_pPage);

		_AutoLatch	latch(cTrans_, m_iDatabaseID, m_iTableID, m_iColumnID);

		if (!isVacant())
			m_pFile->flush(cTrans_);
	}
}

//	FUNCTION public
//	Schema::Sequence::sync -- 不要な版を破棄する
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
Sequence::sync(Trans::Transaction& cTrans_, bool& incomplete, bool& modified)
{
	Os::AutoCriticalSection l(m_cLatch);

	if (m_pFile) {
		; _SYDNEY_ASSERT(!m_pPage);

		_AutoLatch	latch(cTrans_, m_iDatabaseID, m_iTableID, m_iColumnID);

		if (!isVacant())
			m_pFile->sync(cTrans_, incomplete, modified);
	}
}

//	FUNCTION public
//	Schema::Sequence::startBackup -- バックアップを開始する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//		bool bRestorable_ = true
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Sequence::
startBackup(Trans::Transaction& cTrans_, bool bRestorable_)
{
	Os::AutoCriticalSection l(m_cLatch);

	//【注意】	版を使用するトランザクションでも
	//			バックアップの開始時に更新操作を行う可能性があるので、
	//			とにかくラッチする

	if (m_pFile) {
		; _SYDNEY_ASSERT(!m_pPage);

		_AutoLatch	latch(cTrans_, m_iDatabaseID, m_iTableID, m_iColumnID, true);

		if (!isVacant())
			m_pFile->startBackup(cTrans_, bRestorable_);
	}
}

//	FUNCTION public
//	Schema::Sequence::endBackup -- バックアップを終了する
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
Sequence::
endBackup(Trans::Transaction& cTrans_)
{
	Os::AutoCriticalSection l(m_cLatch);

	if (m_pFile) {
		; _SYDNEY_ASSERT(!m_pPage);

		_AutoLatch	latch(cTrans_, m_iDatabaseID, m_iTableID, m_iColumnID);

		if (!isVacant())
			m_pFile->endBackup(cTrans_);
	}
}

//	FUNCTION public
//	Schema::Sequence::recover -- 障害から回復する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//		const Trans::TimeStamp& cPoint_
//			回復する時点を表すタイムスタンプ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Sequence::
recover(Trans::Transaction& cTrans_, const Trans::TimeStamp& cPoint_)
{
	Os::AutoCriticalSection l(m_cLatch);

	if (m_pFile) {
		; _SYDNEY_ASSERT(!m_pPage);

		_AutoLatch	latch(cTrans_, m_iDatabaseID, m_iTableID, m_iColumnID);

		if (!isVacant())
			m_pFile->recover(cTrans_, cPoint_);
	}
}

//	FUNCTION public
//	Schema::Sequence::restore --
//		ある時点に開始された版管理するトランザクションが
//		参照する版を最新版とする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//		const Trans::TimeStamp& cPoint_
//			回復する時点を表すタイムスタンプ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Sequence::
restore(Trans::Transaction& cTrans_, const Trans::TimeStamp& cPoint_)
{
	Os::AutoCriticalSection l(m_cLatch);

	if (m_pFile) {
		; _SYDNEY_ASSERT(!m_pPage);

		_AutoLatch	latch(cTrans_, m_iDatabaseID, m_iTableID, m_iColumnID);

		if (!isVacant())
			m_pFile->restore(cTrans_, cPoint_);
	}
}

//	FUNCTION public
//	Schema::Sequence::move -- シーケンスを移動する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//		const Schema::Os::Path& cNewPath_
//			移動先のパス名
//		bool bUndo_ = false
//			エラー処理中のときは重ねてエラー処理しない
//		bool bRecovery_ = false
//			trueなら変更前のパスにファイルがなくても変更後にあればOKとする
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::RecoveryFailed
//			recoveryモードで実行中、ファイルが移動前にも移動後にも存在しない

void
Sequence::
move(Trans::Transaction& cTrans_, const Os::Path& cNewPath_, bool bUndo_, bool bRecovery_)
{
	Os::AutoCriticalSection l(m_cLatch);

	if (m_pFile) {
		; _SYDNEY_ASSERT(!m_pPage);

		_AutoLatch	latch(cTrans_, m_iDatabaseID, m_iTableID, m_iColumnID);

		// マウントの有無や実体の存在の有無を確認せずに
		// とにかく移動する
		//
		//【注意】	そうしないと下位層で管理している
		//			情報がメンテナンスされない

		if (bRecovery_) {
			// リカバリー中は移動前か移動後のどちらのファイルがあるか調べる
			if (!isAccessible(cTrans_)) {
				// 移動前のファイルが存在しないので移動後を調べる
				attachFile(cNewPath_);

				if (!isAccessible(cTrans_)) {
					// 移動後のファイルもないので例外送出
					_SYDNEY_THROW0(Exception::RecoveryFailed);
				}
				// 移動後にあるので覚えているパス名を変更して終了
				m_cPath = cNewPath_;
				return;
			}
		}

		// 物理ファイルの格納戦略のパス部分を設定する
		Version::File::StorageStrategy::Path	path;

		path._masterData = cNewPath_;
		path._versionLog = cNewPath_;
		path._syncLog = cNewPath_;

		m_pFile->move(cTrans_, path);

		// 自身が管理するパス名を新しいパスに変更する
		m_cPath = cNewPath_;
	}
}

//	FUNCTION public
//	Schema::Sequence::getLastValue -- シーケンスの値を得る
//
//	NOTES
//
//	ARGUMENTS
//	   なし
//
//	RETURN
//		得られたシーケンスの値
//
//	EXCEPTIONS

Sequence::Value
Sequence::getLastValue() const
{
	return m_cValue;
}

//	FUNCTION public
//	Schema::Sequence::getNextValue -- シーケンスの値を増やし、その値を得る
//
//	NOTES
//		もし、シーケンスの現在の値が Schema::Sequence::Invalid ならば、
//		得られる値は 0 である
//		Read Writeトランザクション用のオブジェクトは1つしかできないことを前提としている
//		MT-safeである
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作しようとしているトランザクション記述子
//		Schema::Sequence::Value iInitID_ = Schema::Sequence::Invalid
//			ファイルが作成されていないときに初期値として使用する値
//
//	RETURN
//		得られた増分後のシーケンスの値
//
//	EXCEPTIONS

Sequence::Value
Sequence::getNextValue(Trans::Transaction& cTrans_, const Value& cInit_ /* = Invalid */)
{
	Os::AutoCriticalSection l(m_cLatch);

	if (m_pFile) {
		// 通常の表

		if (!m_bLoaded)
			// ファイルから読み込む
			load(cTrans_, cInit_);

		SCHEMA_FAKE_ERROR2("Schema::Sequence", "GetNextValue", "Assign", SequenceLimitExceeded);

		if (m_cValue.isInvalid()) {
			// シーケンスを生成してから
			// 一度も値を増やしていないので、
			// 値を初期化する
			m_cValue = m_cInit;

		} else if (isReachMax(m_cValue)) {
			// シーケンスの値が最大値に達している
			if (m_bCycle) {
				// Minにする
				m_cValue = isAscending() ? m_cMin : m_cMax;
			} else {
				// エラー
				_SYDNEY_THROW0(Exception::SequenceLimitExceeded);
			}
		} else {
			// 現在の値に increment を加えたものを
			// 新しい値とする
			m_cValue += m_iIncrement;
		}
		// ファイルにすぐには反映しない
		// ★注意★
		// 同じファイルに対してはひとつしかインスタンスが作られないことが前提

		m_bDirty = true;

	} else {
		// 一時オブジェクトの場合はマップから値を得る
		Value& cValue = _Temporary::_entry(_Temporary::createKey(m_iDatabaseID, m_iTableID, m_iColumnID));

		SCHEMA_FAKE_ERROR2("Schema::Sequence", "GetNextValue", "Assign", SequenceLimitExceeded);

		if (cValue.isInvalid()) {
			// シーケンスを生成してから
			// 一度も値を増やしていないので、
			// 値を初期化する
			cValue = m_cInit;

		} else if (isReachMax(cValue)) {
			// シーケンスの値が最大値に達している
			if (m_bCycle) {
				// Minにする
				cValue = isAscending() ? m_cMin : m_cMax;
			} else {
				// エラー
				_SYDNEY_THROW0(Exception::SequenceLimitExceeded);
			}
		} else {
			// 現在の値に increment を加えたものを
			// 新しい値とする
			cValue += m_iIncrement;
		}
		m_cValue = cValue;
	}
	return m_cValue;
}

//	FUNCTION public
//	Schema::Sequence::getNextValue -- シーケンスの値の整合性をとる
//
//	NOTES
//		与えられた値が現在の値より大きければ(incrementが負なら小さければ)シーケンスの値をその値にする
//		MT-safeである
//
//	ARGUMENTS
//		Schema::Sequence::Value iValue_
//			整合性をとる値
//		Trans::Transaction& cTrans_
//			操作しようとしているトランザクション記述子
//		Schema::Sequence::Value iInitID_ = Schema::Sequence::Invalid
//			ファイルが作成されていないときに初期値として使用する値
//		bool bPersist_ = true
//			シーケンスの値を変えたらすぐに永続化するか
//
//	RETURN
//		指定された値
//
//	EXCEPTIONS

Sequence::Value
Sequence::getNextValue(const Value& cValue_, Trans::Transaction& cTrans_,
					   const Value& cInit_ /* = Invalid */, bool bPersist_ /* = true */)
{
	Os::AutoCriticalSection l(m_cLatch);

	if (m_pFile) {
		// 通常の表

		if (!m_bLoaded)
			// ファイルから読み込む
			load(cTrans_, cInit_);

		Value prevValue = m_cValue;

		if (m_cValue.isInvalid()) {
			// シーケンスを生成してから
			// 一度も値を増やしていないので、
			// 値を指定された値に初期化する
			m_cValue = cValue_;

		} else if ((isAscending() && (m_cValue < cValue_))
				   || (!isAscending() && (cValue_ < m_cValue))) {
			// シーケンスの値が指定された値より終端に近ければ
			// 整合性がとれていない状態なので新しい値を
			// 指定された値にする
			m_cValue = cValue_;

		} else {
			// 整合が取れた状態なので指定された値を返すのみでよい
			return cValue_;
		}

		// 新しい値をファイルへ記録する
		m_bDirty = true;

		if (bPersist_) {
			try {
				doPersist(cTrans_);

			} catch (...) {
				// m_cValueの値を元に戻す
				m_cValue = prevValue;
				_SYDNEY_RETHROW;
			}
		}

	} else {
		// 一時オブジェクトの場合はこの関数が呼ばれるはずがない
		// -> Identity columnの対応により呼ばれることがある

		// 一時オブジェクトの場合はマップから値を得る
		Value& cValue = _Temporary::_entry(_Temporary::createKey(m_iDatabaseID, m_iTableID, m_iColumnID));

		SCHEMA_FAKE_ERROR2("Schema::Sequence", "GetNextValue", "Assign", SequenceLimitExceeded);

		if (cValue.isInvalid()) {
			// シーケンスを生成してから
			// 一度も値を増やしていないので、
			// 値を指定された値に初期化する
			m_cValue = cValue = cValue_;

		} else if ((isAscending() && (cValue < cValue_))
				   || (!isAscending() && (cValue_ < cValue))) {
			// シーケンスの値が指定された値より終端に近ければ
			// 整合性がとれていない状態なので新しい値を
			// 指定された値にする
			m_cValue = cValue = cValue_;

		} else {
			// 整合が取れた状態なので何もしない
			;
		}
	}
	return cValue_;
}

// m_cValueの値をファイルに書き込む
void
Sequence::
persist(Trans::Transaction& cTrans_)
{
	// 外部からの呼び出しようなのでここで排他する
	Os::AutoCriticalSection l(m_cLatch);
	doPersist(cTrans_);
}

//	FUNCTION private
//	Schema::Sequence::setValue -- シーケンスの値をセットする
//
//	NOTES
//		MT-safeである
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作しようとしているトランザクション記述子
//		const Schema::Sequence::Value& cValue_
//			セットする値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Sequence::setValue(Trans::Transaction& cTrans_,
				   const Value& cValue_)
{
	Os::AutoCriticalSection l(m_cLatch);

	if (m_pFile) {

		if (!m_bLoaded)
			load(cTrans_);

		Value cPrevValue = m_cValue;
		m_cValue = cValue_;
		m_bDirty = true;

		// 新しい値をファイルへ記録する
		try {
			doPersist(cTrans_);

		} catch (...) {
			// m_cValueの値を元に戻す
			m_cValue = cPrevValue;

			_SYDNEY_RETHROW;
		}

	} else {
		_Temporary::_entry(_Temporary::createKey(m_iDatabaseID, m_iTableID, m_iColumnID)) = m_cValue = cValue_;
	}
}

//	FUNCTION public
//	Schema::Sequence::isAccessible -- ファイルがあるかを得る
//
//	NOTES
//		ページをアタッチしてみる
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		true ... ファイルがある
//		false ... ファイルがない
//
//	EXCEPTIONS

bool
Sequence::
isAccessible(Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(m_pFile);
	return m_pFile->isAccessible();
}

//	FUNCTION public
//	Schema::Sequence::verify -- 整合性検査を行う
//
//	NOTES
//
//	ARGUMENTS
//		Admin::Verification::Progress& cResult_
//			検査結果を格納する変数
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		unsigned int		eTreatment_
//			検査の結果見つかった障害に対する対応を表し、
//			Admin::Verification::Treatment::Value の論理和を指定する
//		const Schema::Sequence::Value& cValue_
//			シーケンスに格納されているはずの値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Sequence::
verify(Admin::Verification::Progress& cResult_,
	   Trans::Transaction& cTrans_,
	   Admin::Verification::Treatment::Value eTreatment_,
	   const Value& cValue_)
{
	bool bCascade = (eTreatment_ & Admin::Verification::Treatment::Cascade);
	bool bCorrect = (eTreatment_ & Admin::Verification::Treatment::Correct);
	bool bContinue = (eTreatment_ & Admin::Verification::Treatment::Continue);
	bool bValue = (eTreatment_ & Admin::Verification::Treatment::Data);

	Os::AutoCriticalSection l(m_cLatch);

	; _SYDNEY_ASSERT(m_pFile);
	_AutoLatch	latch(cTrans_, m_iDatabaseID, m_iTableID, m_iColumnID);

	// 格納されているべき値が初期値の場合はファイルがなくても正しい
	if (cValue_.isInvalid() && isVacant()) return;

	// ファイルが存在しているか
	verifyExistence(cResult_, cTrans_, eTreatment_, cValue_);
	if (!cResult_.isGood()) {
		// Continueでも継続できない
		return;
	}

	// 現在の値を読み込む
	// Cascadeなら同時に物理ファイルのチェックもする
	verifyFile(cResult_, cTrans_, eTreatment_);
	if (!cResult_.isGood()) {
		// Continueでも継続できない
		return;
	}

	if (bValue) {
		// 値をチェックする
		verifyValue(cResult_, cTrans_, eTreatment_, cValue_);
	}
}

//	FUNCTION private
//	Schema::Sequence::verifyExistence -- 存在を調べる
//
//	NOTES
//
//	ARGUMENTS
//		Admin::Verification::Progress& cResult_
//			検査結果を格納する変数
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		unsigned int		eTreatment_
//			検査の結果見つかった障害に対する対応を表し、
//			Admin::Verification::Treatment::Value の論理和を指定する
//		const Schema::Sequence::Value& cValue_
//			シーケンスに格納されているはずの値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Sequence::
verifyExistence(Admin::Verification::Progress& cResult_,
				Trans::Transaction& cTrans_,
				Admin::Verification::Treatment::Value eTreatment_,
				const Value& cValue_)
{
	bool bCorrect = (eTreatment_ & Admin::Verification::Treatment::Correct);

	if (!m_pFile->isAccessible()) {
		if (!bCorrect) {
			_SYDNEY_VERIFY_CORRECTABLE(cResult_, _Verification::_cstrPath,
									   Message::SequenceNotExist());
			// Continueだとしてもこれ以上続けられない
			return;

		}

		; _SYDNEY_ASSERT(bCorrect);
		; _SYDNEY_ASSERT(!cValue_.isInvalid());

		// 作成してしまう
		try {
			SCHEMA_FAKE_ERROR("Schema::Sequence", "VerifyExistence", "Create");
			substantiate(cTrans_, cValue_);

		} catch (Exception::Object& e) {
			_SYDNEY_VERIFY_ABORTED(cResult_, _Verification::_cstrPath, e);
			_SYDNEY_RETHROW;

		} catch (...) {
			_SYDNEY_VERIFY_ABORTED(cResult_, _Verification::_cstrPath,
								   Message::SequenceCreateFailed());
			_SYDNEY_RETHROW;
		}
		_SYDNEY_VERIFY_CORRECTED(cResult_, _Verification::_cstrPath,
								 Message::SequenceNotExist());
	}
}

//	FUNCTION private
//	Schema::Sequence::verifyFile -- 物理ファイルの整合性検査
//
//	NOTES
//
//	ARGUMENTS
//		Admin::Verification::Progress& cResult_
//			検査結果を格納する変数
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		unsigned int		eTreatment_
//			検査の結果見つかった障害に対する対応を表し、
//			Admin::Verification::Treatment::Value の論理和を指定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Sequence::
verifyFile(Admin::Verification::Progress& cResult_,
		   Trans::Transaction& cTrans_,
		   Admin::Verification::Treatment::Value eTreatment_)
{
	bool bCascade = (eTreatment_ & Admin::Verification::Treatment::Cascade);

	// 自動的にendVerificationを呼ぶためのクラス
	// ★注意★
	// startVerificationを呼んでなくても呼べる仕様になっているはずである

	_Verification::_AutoVerification v(m_pFile, m_pPage, cTrans_, eTreatment_, cResult_);

	if (bCascade) {
		// 物理ファイルの整合性検査も行う
		// NonManageTypeの物理ファイルはnotifyUsePageは不要
		v.startVerification();
		v.verifyPage();

		if (!v.isGood()) {
			// Continueでも継続できない
			// これまでの経過はデストラクターの中でcResult_に格納される
			return;
		}

	} else if (!m_bLoaded) {
		// 現在の値を読み込むためにページをアタッチする
		m_pPage = m_pFile->attachPage(cTrans_, Buffer::Page::FixMode::ReadOnly);
	}

	if (!m_bLoaded) {
		; _SYDNEY_ASSERT(m_pPage);
		try {
			SCHEMA_FAKE_ERROR("Schema::Sequence", "VerifyFile", "Read");
			m_cValue.read(m_pPage, cTrans_);

		} catch (Exception::Object& e) {
			// ABORTEDを加える前にこれまでの経過を記録する
			v.terminate();
			_SYDNEY_VERIFY_ABORTED(cResult_, _Verification::_cstrPath, e);
			_SYDNEY_RETHROW;

		} catch (...) {
			// ABORTEDを加える前にこれまでの経過を記録する
			v.terminate();
			_SYDNEY_VERIFY_ABORTED(cResult_, _Verification::_cstrPath,
								   Message::SequenceReadFailed());
			_SYDNEY_RETHROW;
		}
	}
	// 後始末は自動的に行われる
}

//	FUNCTION private
//	Schema::Sequence::verifyValue -- 値を調べる
//
//	NOTES
//
//	ARGUMENTS
//		Admin::Verification::Progress& cResult_
//			検査結果を格納する変数
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		unsigned int		eTreatment_
//			検査の結果見つかった障害に対する対応を表し、
//			Admin::Verification::Treatment::Value の論理和を指定する
//		const Schema::Sequence::Value& cValue_
//			シーケンスに格納されているはずの値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Sequence::
verifyValue(Admin::Verification::Progress& cResult_,
			Trans::Transaction& cTrans_,
			Admin::Verification::Treatment::Value eTreatment_,
			const Value& cValue_)
{
	bool bCorrect = (eTreatment_ & Admin::Verification::Treatment::Correct);

	// 与えられた値と実際の値を比較する
	if (!cValue_.isInvalid()
		&& ((isAscending() && (m_cValue < cValue_))
			|| (!isAscending() && (cValue_ < m_cValue)))) {
		// 格納されている値が小さい(incrementが負なら大きい)とき
		if (!bCorrect) {
			_SYDNEY_VERIFY_CORRECTABLE(cResult_, _Verification::_cstrPath,
									   Message::SequenceValueNotMatch());
			// これ以上続けられない
			return;
		} else {
			// 修復する: 値を与えられている値にセットする
			try {
				setValue(cTrans_, cValue_);

			} catch (Exception::Object& e) {
				_SYDNEY_VERIFY_ABORTED(cResult_, _Verification::_cstrPath, e);
				_SYDNEY_RETHROW;

			} catch (...) {
				_SYDNEY_VERIFY_ABORTED(cResult_, _Verification::_cstrPath,
									   Message::SequenceCorrectFailed());
				_SYDNEY_RETHROW;
			}

			_SYDNEY_VERIFY_CORRECTED(cResult_, _Verification::_cstrPath,
									 Message::SequenceValueNotMatch());
		}
	}
}

// FUNCTION private
//	Schema::Sequence::doPersist -- m_cValueの値をファイルに書き込む
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Sequence::
doPersist(Trans::Transaction& cTrans_)
{
	// 呼び出し側で排他している

	if (!m_bDirty) return;

	; _SYDNEY_ASSERT(m_pFile);
	; _SYDNEY_ASSERT(!m_pPage);

	_AutoLatch	latch(cTrans_, m_iDatabaseID, m_iTableID, m_iColumnID);

	if (isVacant()) {
		substantiate(cTrans_, m_cValue);
		m_bDirty = false;
		return;
	}

	// ページをアタッチする

	m_pPage = m_pFile->attachPage(cTrans_,
								  cTrans_.isBatchMode()
								  ? Buffer::Page::FixMode::Write
								  : Buffer::Page::FixMode::Discardable);
	; _SYDNEY_ASSERT(m_pPage);

	// 新しい値をファイルへ記録する

	try {
		SCHEMA_FAKE_ERROR("Schema::Sequence", "GetNextValue", "Write");
		m_cValue.write(m_pPage, cTrans_);

	} catch (...) {

		_BEGIN_RECOVERY;

		// NotDirtyでデタッチすれば戻る
		// Batchモードのときは常にDirtyでデタッチする
		m_pFile->detachPage(m_pPage,
							cTrans_.isBatchMode()
							? PhysicalFile::Page::UnfixMode::Dirty
							: PhysicalFile::Page::UnfixMode::NotDirty);

		_END_RECOVERY;
		_SYDNEY_RETHROW;
	}

	m_pFile->detachPage(m_pPage, PhysicalFile::Page::UnfixMode::Dirty);
	m_bDirty = false;
}

// FUNCTION private
//	Schema::Sequence::load -- m_cValueの値をファイルから読み込む
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Value& cInit_ /* = Invalid */
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Sequence::
load(Trans::Transaction& cTrans_, const Value& cInit_ /* = Invalid */)
{
	// 呼び出し側で排他している
	; _SYDNEY_ASSERT(m_pFile);
	; _SYDNEY_ASSERT(!m_pPage);

	_AutoLatch	latch(cTrans_, m_iDatabaseID, m_iTableID, m_iColumnID);

	if (isVacant()) {
		substantiate(cTrans_, cInit_);
		m_bLoaded = true;
		return;
	}

	// ページをアタッチして現在の値を得る
	m_pPage = m_pFile->attachPage(cTrans_, Buffer::Page::FixMode::ReadOnly);
	; _SYDNEY_ASSERT(m_pPage);

	try {
		SCHEMA_FAKE_ERROR("Schema::Sequence", "GetNextValue", "Read");
		m_cValue.read(m_pPage, cTrans_);

	} catch (...) {

		_BEGIN_RECOVERY;

		m_pFile->detachPage(m_pPage, PhysicalFile::Page::UnfixMode::NotDirty);

		_END_RECOVERY;

		_SYDNEY_RETHROW;
	}
	m_pFile->detachPage(m_pPage, PhysicalFile::Page::UnfixMode::NotDirty);

	m_bLoaded = true;
}

// Defaultの内容からメンバーの値をセットする
void
Sequence::
setArgument(const Default& cDefault_)
{
	; _SYDNEY_ASSERT(cDefault_.isIdentity());

	// Identity Columnの仕様を表すIntegerArrayDataを得る
	const Common::IntegerArrayData& cSpec = cDefault_.getIdentitySpec();
	; _SYDNEY_ASSERT(cSpec.getCount() == Statement::Generator::Option::TypeNum);

	for (int i = 0; i < Statement::Generator::Option::TypeNum; ++i) {
		int iValue = cSpec.getElement(i);
		switch (i) {
		case Statement::Generator::Option::Start:
			{
				m_cInit = iValue;
				break;
			}
		case Statement::Generator::Option::Increment:
			{
				m_iIncrement = iValue;
				break;
			}
		case Statement::Generator::Option::MaxValue:
			{
				m_cMax = iValue;
				break;
			}
		case Statement::Generator::Option::MinValue:
			{
				m_cMin = iValue;
				break;
			}
		case Statement::Generator::Option::Cycle:
			{
				m_bCycle = (iValue != 0);
				break;
			}
		case Statement::Generator::Option::GetMax:
			{
				m_bGetMax = (iValue != 0);
				break;
			}
		default:
			{
				// never reach
				break;
			}
		}
	}
}

// FUNCTION public
//	Schema::Sequence::isReachMax -- 次に割り当てるm_cValueの値が制限値を超えるか
//
// NOTES
//
// ARGUMENTS
//	const Value& cValue_
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Sequence::
isReachMax(const Value& cValue_) const
{
	// 1のときは高速に判断できるので特別扱いする
	return (m_iIncrement == 1) ? !cValue_.isAbleToAddOne(m_cMax)
		: !cValue_.isAbleToAdd(m_iIncrement, (isAscending() ? m_cMax : m_cMin));
}

//	FUNCTION private
//	Schema::Sequence::isVacant -- 実体の有無を調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true .. 実体がない
//		false.. 実体がある
//
//	EXCEPTIONS

bool
Sequence::
isVacant() const
{
	return !m_pFile->isAccessible();
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2010, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
