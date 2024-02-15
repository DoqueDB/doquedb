// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Session.cpp --
// 
// Copyright (c) 2002, 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2013, 2015, 2017, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Server";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Server/Session.h"

#include "Common/Assert.h"

#include "Exception/DuplicateVariable.h"
#include "Exception/SessionNotExist.h"
#include "Exception/Unexpected.h"
#include "Exception/VariableNotFound.h"

#include "Schema/Database.h"

#include "Trans/AutoTransaction.h"

#include "Opt/Optimizer.h"
#include "Opt/Planner.h"

#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"

#include "ModMap.h"

_SYDNEY_USING
_SYDNEY_SERVER_USING

namespace
{

//
//	VARIABLE local
//	_$$::_cCriticalSection -- セッションID取得時の排他制御用
//
Os::CriticalSection _cCriticalSection;

//
//	VARIABLE local
//	_$$::_iNewSessionID -- 新しいセッションID
//
ID _iNewSessionID = 0;

//
//	FUNCTION local
//	_$$::_getNewSessionID() -- 新しいセッションIDを得る
//
ID _getNewSessionID()
{
	Os::AutoCriticalSection cAuto(_cCriticalSection);
	return ++_iNewSessionID;
}

//
//	VARIABLE local
//	_$$::_cAvailabilityCriticalSection
//
//	NOTES
//	利用可能性の排他制御用のクリティカルセクション
//
Os::CriticalSection _cAvailabilityCriticalSection;

//
//	VARIABLE private
//	_$$::_mapAvailability
//
//	NOTES
//	セッションIDごとの利用可能性を管理するマップ
//
ModMap<ID, bool, ModLess<ID> > _mapAvailability;


//
//	VARIABLE private
//	_$$::_mapSession
//
//	NOTES
//	セッションIDとインスタンスの紐付
//
Common::VectorMap<ID, Session*, ModLess<ID> > _cSessionMap;

//
//	CLASS local
//	_$$::_DatabaseLess
//
//	NOTES
//	データベース名用のModLess
//
class _DatabaseLess
{
public:
	ModBoolean operator() (const ModUnicodeString& v1,
						   const ModUnicodeString& v2) const
		{
			return (v1.compare(v2, ModFalse) < 0) ? ModTrue : ModFalse;
		}
};

//
//	VARIABLE local
//	_$$::_cDatabaseLatch
//
//	NOTES
//	以下のマップを保護するためのラッチ
//
Os::CriticalSection _cDatabaseLatch;

//
//	VARIABLE local
//	_$$::_mapDatabaseID
//
//	NOTES
//	データベース名とデータベースIDのマップ
//
typedef ModMap<ModUnicodeString,
			   ModPair<Schema::ObjectID::Value, bool>,
			   _DatabaseLess> _DatabaseMap;
_DatabaseMap _mapDatabaseID;

//
//	FUNCTION local
//	_$$::_getDatabaseInfo
//
//	NOTES
//	データベース名からデータベース情報を得る
//
ModPair<Schema::ObjectID::Value, bool>
_getDatabaseInfo(const ModUnicodeString& cDatabaseName_)
{
	Os::AutoCriticalSection cAuto(_cDatabaseLatch);

	_DatabaseMap::Iterator i = _mapDatabaseID.find(cDatabaseName_);
	if (i == _mapDatabaseID.end())
	{
		// まだないので、スキーマから取得する

		Trans::AutoTransaction pTrans(Trans::Transaction::attach());
		pTrans->begin(Schema::ObjectID::SystemTable,
					  Trans::Transaction::Category::ReadWrite);
		
		// データベースの利用不可をチェックしない
		Schema::ObjectID::Value iDatabaseID
			= Schema::Database::getID(cDatabaseName_, *pTrans, true);
		
		pTrans->commit();

		// スレーブデータベースか否かはわからないので、
		// とりあえずスレーブではないとする
		
		ModPair<_DatabaseMap::Iterator, ModBoolean> r
			= _mapDatabaseID.insert(cDatabaseName_,
									ModPair<Schema::ObjectID::Value, bool>(
										iDatabaseID, false));

		i = r.first;
	}

	return (*i).second;
}
		
//
//	FUNCTION local
//	_$$::_setDatabaseInfo
//
//	NOTES
//	データベースIDを登録する
//
void
_setDatabaseInfo(const ModUnicodeString& cDatabaseName_,
				 Schema::ObjectID::Value iDatabaseID_,
				 bool bSlave_)
{
	Os::AutoCriticalSection cAuto(_cDatabaseLatch);

	_mapDatabaseID[cDatabaseName_]
		= ModPair<Schema::ObjectID::Value, bool>(iDatabaseID_, bSlave_);
}

}	// namespace {

//
//	FUNCTION public
//	Server::Session::Session -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
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
Session::Session()
	: _transaction(_getNewSessionID()),
	  m_iUserID(-1),
	  m_bSuperUser(true),
	  m_bLocked(false),
	  m_iStatementType(0),
	  m_bPrivilegeInitialized(false),
	  m_vecExplain(),
	  m_pCurrentSQL(0), m_pCurrentParameter(0)
{
	// 現在日時を設定する
	m_cStartTime.setCurrent();
	
	Os::AutoCriticalSection cAuto(_cCriticalSection);
	_cSessionMap.insert(getID(), this);
}

//
//	FUNCTION public
//	Server::Session::~Session -- デストラクタ
//
//	NOTES
//	デストラクタ。
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
Session::~Session()
{
	//スキーマに終了をレポートする
	Schema::Database::release(getID());

	//利用可能性エントリを削除する
	eraseAvailability(getID());

//	//最適化結果を削除する
//	PrepareMap::Iterator i = m_mapPrepareID.begin();
//	for (; i != m_mapPrepareID.end(); ++i)
//	{
//		Opt::Optimizer::erasePrepareStatement((*i).first);
//	}
	ModVector<ModPair<Opt::Planner*, ModUnicodeString*> >::Iterator iterator
		= m_vecPreparePlan.begin();
	const ModVector<ModPair<Opt::Planner*, ModUnicodeString*> >::Iterator last
		= m_vecPreparePlan.end();
	for (; iterator != last; ++iterator)
	{
		delete (*iterator).first;
		delete (*iterator).second;
	}
		
	{
		Os::AutoCriticalSection cAuto(_cCriticalSection);
		_SYDNEY_ASSERT(_cSessionMap[getID()] != 0);			
		_cSessionMap.erase(getID());
	}
	BitSetValueMap::Iterator bitSetValIte = m_mapBitSetValue.begin();
	for (; bitSetValIte != m_mapBitSetValue.end(); ++bitSetValIte)
		delete (*bitSetValIte).second;

}

// FUNCTION public
//	Server::Session::checkPrivilege -- check privilege
//
// NOTES
//
// ARGUMENTS
//	Common::Privilege::Category::Value eCategory_
//	Common::Privilege::Value iValue_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Session::
checkPrivilege(Common::Privilege::Category::Value eCategory_,
			   Common::Privilege::Value iValue_)
{
	; _SYDNEY_ASSERT(m_bPrivilegeInitialized);
	; _SYDNEY_ASSERT(eCategory_ >= 0 && eCategory_ < Common::Privilege::Category::ValueNum);

	return (m_vecPrivilege[eCategory_] & iValue_) == iValue_;
}

// FUNCTION public
//	Server::Session::initializePrivilege -- initialize privilege
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::Database& cDatabase_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Session::
initializePrivilege(Trans::Transaction& cTrans_,
					const Schema::Database& cDatabase_)
{
	if (!m_bPrivilegeInitialized) {
		if (isSuperUser()) {
			// superuser has all the privileges
			Common::Privilege::Value* p = m_vecPrivilege;
			for (int i = 0; i < Common::Privilege::Category::ValueNum; ++i, ++p) {
				*p = Common::Privilege::All;
			}
		} else {
			// get from database
			cDatabase_.getPrivilege(cTrans_, m_iUserID,
									m_vecPrivilege, Common::Privilege::Category::ValueNum);
		}
		m_bPrivilegeInitialized = true;
	}
}

//
//	FUNCTION public
//	Server::Session::pushPrepareID -- PrepareIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//	int id_
//		PrepareID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Session::pushPrepareID(int id_)
{
	m_mapPrepareID.insert(id_, 0);
}

//
//	FUNCTION public
//	Server::Session::checkPrepareID -- PrepareIDの存在をチェックする
//
//	NOTES
//
//	ARGUMENTS
//	int id_
//	   	PrepareID
//
//	RETURN
//	bool
//		存在している場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Session::checkPrepareID(int id_)
{
	bool bResult = false;
	PrepareMap::Iterator i = m_mapPrepareID.find(id_);
	if (i != m_mapPrepareID.end())
		bResult = true;
	return bResult;
}

//
//	FUNCTION public
//	Server::Session::popPrepareID -- PrepareIDを削除する
//
//	NOTES
//
//	ARGUMENTS
//	int id_
//		PrepareID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Session::popPrepareID(int id_)
{
	PrepareMap::Iterator i = m_mapPrepareID.find(id_);
	if (i != m_mapPrepareID.end())
		m_mapPrepareID.erase(i);
}

// FUNCTION public
//	Server::Session::pushPreparePlan -- PreparePlanを登録する
//
// NOTES
//
// ARGUMENTS
//	Opt::Planner* pPlanner_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
Session::
pushPreparePlan(Opt::Planner* pPlanner_, const ModUnicodeString& cSQL_)
{
	ModUnicodeString* sql = new ModUnicodeString(cSQL_);
	int i = m_vecPreparePlan.getSize();
	m_vecPreparePlan.pushBack(
		ModPair<Opt::Planner*, ModUnicodeString*>(pPlanner_, sql));
	return i;
}

// FUNCTION public
//	Server::Session::getPreparePlan -- PreparePlanを取得する
//
// NOTES
//
// ARGUMENTS
//	int id_
//	
// RETURN
//	Opt::Planner*
//
// EXCEPTIONS

Opt::Planner*
Session::
getPreparePlan(int id_)
{
	if (id_ >= 0 && id_ < static_cast<int>(m_vecPreparePlan.getSize()))
		return m_vecPreparePlan[id_].first;
	return 0;
}

// FUNCTION public
//	Server::Session::getPrepareSQL -- Prepare時のSQL文を取得する
//
// NOTES
//
// ARGUMENTS
//	int id_
//	
// RETURN
//	const ModUnicodeString*
//
// EXCEPTIONS

const ModUnicodeString*
Session::
getPrepareSQL(int id_)
{
	if (id_ >= 0 && id_ < static_cast<int>(m_vecPreparePlan.getSize()))
		return m_vecPreparePlan[id_].second;
	return 0;
}

// PreparePlanを削除する
void
Session::
popPreparePlan(int id_)
{
	if (id_ >= 0 && id_ < static_cast<int>(m_vecPreparePlan.getSize()))
	{
		ModPair<Opt::Planner*, ModUnicodeString*>& e = m_vecPreparePlan[id_];
		delete e.first, e.first = 0;
		delete e.second, e.second = 0;
	}
}


// FUNCTION public
//	Server::Session::generateBitSetVariable -- BitSet変数を生成する
//
// NOTES
//
// ARGUMENTS
//	int id_
//	
// RETURN
//	Session::BitSetVariable*
//
// EXCEPTIONS

Session::BitSetVariable*
Session::
generateBitSetVariable(const ModUnicodeString cstrName_)
{
	if (m_mapBitSetValue.find(cstrName_) != m_mapBitSetValue.end())
		_SYDNEY_THROW1(Exception::DuplicateVariable, cstrName_);
	
	ModAutoPointer<BitSetVariable> pResult = new BitSetVariable(cstrName_);
	m_mapBitSetValue.insert(cstrName_, pResult.release());
	return pResult.get();
}



// FUNCTION public
//	Server::Session::getBitSetVariable
//
// NOTES
//
// ARGUMENTS
//	ModUnicodeString cstrName_
//	
// RETURN
//	Session::BitSetVariable*
//
// EXCEPTIONS

Session::BitSetVariable*
Session::
getBitSetVariable(const ModUnicodeString& cstrName_)
{

	BitSetValueMap::ConstIterator ite = m_mapBitSetValue.find(cstrName_);
	if (ite == m_mapBitSetValue.end())
		_SYDNEY_THROW1(Exception::VariableNotFound, cstrName_);

	return (*ite).second;
}




// FUNCTION public
//	Server::Session::startExplain -- start explain
//
// NOTES
//
// ARGUMENTS
//	Opt::Explain::Option::Value iValue_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Session::
startExplain(Opt::Explain::Option::Value iValue_)
{
	m_vecExplain.pushBack(iValue_);
}

// FUNCTION public
//	Server::Session::endExplain -- end explain
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
Session::
endExplain()
{
	m_vecExplain.popBack();
}

// FUNCTION public
//	Server::Session::getExplain -- get explain option
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Opt::Explain::Option::Value
//
// EXCEPTIONS

Opt::Explain::Option::Value
Session::
getExplain()
{
	return m_vecExplain.isEmpty()
		? Opt::Explain::Option::None
		: m_vecExplain.getBack();
}

//
//	FUNCTION public static
//	Server::Session::setAvailability
//		-- セッションが利用可能かどうかを設定する
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iSessionID_
//		セッションID
//	bool bFlag_
//		利用可能性
//
//	RETURN
//	bool
//		設定されていた使用可能性
//
//	EXCEPTIONS
//	なし
//
bool
Session::setAvailability(ID iSessionID_, bool bFlag_)
{
	Os::AutoCriticalSection cAuto(_cAvailabilityCriticalSection);

	bool bPrev = true;

	ModMap<ID, bool, ModLess<ID> >::Iterator i = _mapAvailability.find(iSessionID_);
	if (i != _mapAvailability.end())
	{
		bPrev = (*i).second;
	}

	_mapAvailability[iSessionID_] = bFlag_;

	return bPrev;
}

//
//	FUNCTION public static
//	Server::Session::isAvailable
//		-- セッションが利用可能か調べる
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iSessionID_
//		セッションID
//
//	RETURN
//	bool
//		利用可能ならtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//
bool
Session::isAvailable(ID iSessionID_)
{
	Os::AutoCriticalSection cAuto(_cAvailabilityCriticalSection);

	bool bPrev = true;

	ModMap<ID, bool, ModLess<ID> >::Iterator i = _mapAvailability.find(iSessionID_);
	if (i != _mapAvailability.end())
	{
		bPrev = (*i).second;
	}

	return bPrev;
}

//
//	FUNCTION public static
//	Server::Session::eraseAvailability
//		-- セッションの利用可能性エントリを削除する
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iSessionID_
//		セッションID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Session::eraseAvailability(ID iSessionID_)
{
	Os::AutoCriticalSection cAuto(_cAvailabilityCriticalSection);

	ModMap<ID, bool, ModLess<ID> >::Iterator i = _mapAvailability.find(iSessionID_);
	if (i != _mapAvailability.end())
	{
		_mapAvailability.erase(i);
	}
}

//
//	FUNCTION public static
//	Server::Session::clearAvailability
//		-- セッションの利用可能性エントリをすべて削除する
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
Session::clearAvailability()
{
	Os::AutoCriticalSection cAuto(_cAvailabilityCriticalSection);

	_mapAvailability.erase(_mapAvailability.begin(), _mapAvailability.end());
}


//
//	FUNCTION public static
//	Server::Session::getSession
//		-- セッションを取得する
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
Session*
Session::getSession(ID iSessionID_)
{
	
	Os::AutoCriticalSection cAuto(_cCriticalSection);
	
	Common::VectorMap<ID, Session*, ModLess<ID> >::ConstIterator ite = _cSessionMap.find(iSessionID_);
	if (ite == _cSessionMap.end()) {
		_SYDNEY_THROW1(Exception::SessionNotExist, iSessionID_);
	}
	return (*ite).second;
}

//
//	FUNCTION public
//	Server::Sesion::getDatabaseID -- データベースIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Schema::ObjectID::Value
//		データベースID
//
//	EXCEPTIONS
//
Schema::ObjectID::Value
Session::getDatabaseID()
{
	ModPair<Schema::ObjectID::Value, bool> info
		= _getDatabaseInfo(getDatabaseName());
		
	return info.first;
}

//
//	FUNCTION public
//	Server::Sesion::isSlaveDatabase -- スレーブデータベースか否か
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		スレーブデータベースの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Session::isSlaveDatabase()
{
	ModPair<Schema::ObjectID::Value, bool> info
		= _getDatabaseInfo(getDatabaseName());
		
	return info.second;
}

//
//	FUNCTION public
//	Server::Session::setDatabaseInfo -- データベース情報を設定する
//
//	NOTES
//
//	ARGUMENTS
//	Schema::ObjectID::Value iDatabaseID_
//		データベースID
//	bool bSlave_
//		スレーブデータベースか否か
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Session::setDatabaseInfo(Schema::ObjectID::Value iDatabaseID_,
						 bool bSlave_)
{
	// マップに登録
	_setDatabaseInfo(getDatabaseName(), iDatabaseID_, bSlave_);
}

//
//	FUNCTION public static
//	Server::Session::setDatabaseInfo -- データベース情報を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cDatabaseName_
//		データベース名
//	Schema::ObjectID::Value iDatabaseID_
//		データベースID
//	bool bSlave_
//		スレーブデータベースか否か
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Session::setDatabaseInfo(const ModUnicodeString& cDatabaseName_,
						 Schema::ObjectID::Value iDatabaseID_,
						 bool bSlave_)
{
	// マップに登録
	_setDatabaseInfo(cDatabaseName_, iDatabaseID_, bSlave_);
}

//
//	FUNCTION public
//	Server::Session::setCurrentSQL -- 実行中のSQL文を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString* pSQL_
//		実行中のSQL文
//	Common::DataArrayData* pParamter_
//		実行中SQL文のパラメータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Session::setCurrentSQL(const ModUnicodeString* pSQL_,
					   Common::DataArrayData* pParameter_)
{
	//【注意】
	//
	//	ここで設定したポインタは、再び setCurrentSQL で 0 を設定するまで、
	//	参照される可能性がある。そのため、呼び出し元は、メモリを開放する前に
	//	setCurrentSQL で 0 を設定する必要がある
	//
	//	ただし、AutoSession のデストラクタで 0 を設定しているので、
	//	通常は、セッションのロック解除まで保持していればよい

	Os::AutoCriticalSection cAuto(m_cLatch);
	m_pCurrentSQL = pSQL_;
	m_pCurrentParameter = pParameter_;
}

//
//	FUNCTION public
//	Server::Session::getCurrentSQL -- 実行中のSQL文を取得する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RERTURN
//	ModUnicodeString*
//		実行中のSQL文へのポインタ。呼び出し側で開放する必要がある
//
//	EXCEPTIONS
//
ModUnicodeString*
Session::getCurrentSQL()
{
	ModUnicodeString* r = 0;
	
	Os::AutoCriticalSection cAuto(m_cLatch);

	if (m_pCurrentSQL)
	{
		// SQL文をコピーする
		r = new ModUnicodeString(*m_pCurrentSQL);

		if (m_pCurrentParameter)
		{
			// パラメータが設定されているので、
			// SQL文中の後ろにパラメータを追加する

			r->append(' ');
			r->append(m_pCurrentParameter->toString());
		}
	}

	return r;
}

//
//	Copyright (c) 2002, 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2013, 2015, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
