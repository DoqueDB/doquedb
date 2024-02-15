// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Session.h --
// 
// Copyright (c) 2002, 2004, 2005, 2007, 2008, 2011, 2012, 2013, 2015, 2017, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SERVER_SESSION_H
#define __SYDNEY_SERVER_SESSION_H

#include "Server/Module.h"
#include "Server/SessionID.h"
#include "Server/Transaction.h"
#include "Server/Type.h"

#include "Schema/Object.h"

#include "Common/Object.h"
#include "Common/BitSet.h"
#include "Common/DataArrayData.h"
#include "Common/DateTimeData.h"
#include "Common/LargeVector.h"
#include "Common/Privilege.h"

#include "Opt/Explain.h"

#include "Os/CriticalSection.h"

#include "ModMap.h"
#include "ModPair.h"
#include "ModUnicodeString.h"
#include "ModVector.h"

_SYDNEY_BEGIN

namespace Opt
{
	class Planner;
}

namespace DServer
{
	class Session;
}

namespace Schema
{
	class Database;
}

_SYDNEY_SERVER_BEGIN

//
//	CLASS
//	Server::Session -- Sydneyのサーバ側のセッションオブジェクト
//
//	NOTES
//	
//
class Session : public Common::Object
{
public:
	// CLASS
	//	Session::BitSetVariable -- 
	//
	// NOTES
	class BitSetVariable
	{
	public:
		// constructor
		BitSetVariable(const ModUnicodeString& cstrValName_)
			:m_cstrValName(cstrValName_),
			 m_iTableId(-1),
			 m_cBitSet()
			{}

		// 変数名を取得する
		const ModUnicodeString& getName() const { return m_cstrValName; }

		// BitSet値を取得する
		Common::BitSet& getValue()  { return m_cBitSet; }

		// テーブル名を取得する
		Schema::Object::ID::Value getSchemaTableID() const { return m_iTableId; }

		// テーブル名をセットする
		void setSchemaTableID(Schema::Object::ID::Value iTableId_) { m_iTableId = iTableId_; }
	
		// destructor
		~BitSetVariable() {}

	private:
		// 変数名
		ModUnicodeString m_cstrValName;
		// テーブル名
		Schema::Object::ID::Value m_iTableId;
		// ビットセット値
		Common::BitSet m_cBitSet;
	};

	
	//コンストラクタ
	Session();
	//デストラクタ
	virtual ~Session();

	//
	//	FUNCTION public
	//	Server::Session::setDatabaseName -- データベース名を設定する
	//
	void setDatabaseName(const ModUnicodeString& cstrName_)
	{
		m_cstrDatabaseName = cstrName_;
	}

	//
	//
	//	FUNCTION public
	//	Server::Session::getDatabaseName -- データベース名を得る
	//
	const ModUnicodeString& getDatabaseName() const
	{
		return m_cstrDatabaseName;
	}

	//
	//	FUNCTION public
	//	Server::Session::setUserName -- Set user name
	//
	void setUserName(const ModUnicodeString& cstrName_)
	{
		m_cstrUserName = cstrName_;
	}

	//
	//
	//	FUNCTION public
	//	Server::Session::getUserName -- Get user name
	//
	const ModUnicodeString& getUserName() const
	{
		return m_cstrUserName;
	}

	//
	//	FUNCTION public
	//	Server::Session::setUserID -- Set user ID
	//
	void setUserID(int iID_)
	{
		m_iUserID = iID_;
	}

	//
	//	FUNCTION public
	//	Server::Session::getUserID -- Get user ID
	//
	int getUserID() const
	{
		return m_iUserID;
	}

	//
	//	FUNCTION public
	//	Server::Session::setIsSuperUser -- Set superuser or not
	//
	void setIsSuperUser(bool bSuperUser_)
	{
		m_bSuperUser = bSuperUser_;
	}

	//
	//	FUNCTION public
	//	Server::Session::isSuperUser -- Get superuser or not
	//
	bool isSuperUser() const
	{
		return m_bSuperUser;
	}

	//
	//	FUNCTION public
	//	Server::Session::getTransaction -- トランザクションを得る
	//
	Transaction& getTransaction()
	{
		return _transaction;
	}

	//	FUNCTION public
	//	Server::Session::getID -- セッションIDを得る

	ID
	getID() const
	{
		return _transaction.getSessionID();
	}

	//
	//	FUNCTION public
	//	Server::Session::tryLock -- ロックを試みる
	//
	//	NOTES
	//	InstanceManagerからしか呼ばれないので、排他されていない
	//
	bool tryLock(int iStatementType_)
	{
		if (m_bLocked == false)
		{
			m_bLocked = true;
			m_iStatementType = iStatementType_;
			return true;
		}
		return false;
	}

	//
	//	FUNCTION public
	//	Server::Session::unlock -- アンロックする
	//
	//	NOTES
	//	InstanceManagerからしか呼ばれないので、排他されていない
	//
	void unlock()
	{
		m_bLocked = false;
		m_iStatementType = 0;
	}

	//
	//	FUNCTION public
	//	Server::Session::isLocked -- ロックされているか
	//
	//	NOTES
	//	InstanceManagerからしか呼ばれないので、排他されていない
	//
	bool isLocked()
	{
		return m_bLocked;
	}

	//
	//	FUNCTION public
	//	Server::Session::getStatementType -- 実行中のSQL文の種別を得る
	//
	int getStatementType() const { return m_iStatementType; }

	//
	//	FUNCTION public
	//	Server::Session::changeStatementType -- 実行中のSQL文種別を変更する
	//
	//	NOTES
	//	AutoSessionからしか呼ばれないので、排他されていない
	//
	void changeStatementType(int iStatementType_)
	{
		if (m_bLocked == true)
			m_iStatementType = iStatementType_;
	}

	// check privilege
	bool checkPrivilege(Common::Privilege::Category::Value eCategory_,
						Common::Privilege::Value iValue_);
	// initialize privilege
	bool isPrivilegeInitialized() const {return m_bPrivilegeInitialized;}
	void initializePrivilege(Trans::Transaction& cTrans_,
							 const Schema::Database& cDatabase_);

	// PrepareIDを登録する
	void pushPrepareID(int id_);
	// PrepareIDが登録されているかチェックする
	bool checkPrepareID(int id_);
	// PrepareIDを削除する
	void popPrepareID(int id_);

	// PreparePlanを登録する
	int pushPreparePlan(Opt::Planner* pPlanner_,
						const ModUnicodeString& cSQL_);
	// SQL文を取得する
	const ModUnicodeString* getPrepareSQL(int id_);
	// PreparePlanを取得する
	Opt::Planner* getPreparePlan(int id_);
	// PreparePlanを削除する
	void popPreparePlan(int id_);
	
	// BitSet変数を生成する
	BitSetVariable* generateBitSetVariable(const ModUnicodeString cstrName_);
	// BitSet変数を取得する
	BitSetVariable* getBitSetVariable(const ModUnicodeString& cstrName_);
	// start explain
	void startExplain(Opt::Explain::Option::Value iValue_);
	// end explain
	void endExplain();
	// get explain option
	Opt::Explain::Option::Value getExplain();

	//セッションが利用可能かどうかを設定する
	SYD_SERVER_FUNCTION static bool setAvailability(ID iSessionID_,
													bool bFlag_);
	//セッションが利用可能か調べる
	SYD_SERVER_FUNCTION static bool isAvailable(ID iSessionID_);
	//セッションの利用可能性エントリを削除する
	static void eraseAvailability(ID iSessionID_);
	//セッションの利用可能性エントリをすべて削除する
	static void clearAvailability();

	// セッションを得る
	static Session* getSession(const ID iSessionID_);

	//	FUNCTION public
	//	Session::Thread::isAvailable -- セッションが利用可能か調べる

	bool
	isAvailable() const
	{
		return isAvailable(getID());
	}

	// 開始時間を取得する
	const Common::DateTimeData& getStartTime() const
		{ return m_cStartTime; }

	// カスケード先のセッションを得る
	Common::LargeVector<DServer::Session*>& getCascadeSession()
		{ return _transaction.getCascadeSession(); }

	// データベースIDを得る
	Schema::ObjectID::Value getDatabaseID();
	// スレーブデータベースか否か
	bool isSlaveDatabase();
	// データベースIDを設定する
	void setDatabaseInfo(Schema::ObjectID::Value iDatabaseID_,
						 bool bSlave_);
	// データベースIDをマップに設定する
	static void setDatabaseInfo(const ModUnicodeString& cDatabaseName_,
								Schema::ObjectID::Value iDatabaseID_,
								bool bSlave_);

	// 実行中SQL文を設定する
	//  -- AutoSession のデストラクタで 0 を設定している
	void setCurrentSQL(const ModUnicodeString* pSQL_,
					   Common::DataArrayData* pParameter_);
	
	// 実行中のSQL文を取得する
	//	-- 戻り値は呼び出し側で開放する必要がある
	ModUnicodeString* getCurrentSQL();
	
private:
	typedef ModMap<int, int, ModLess<int> > PrepareMap;
	
	//トランザクション
	Transaction	_transaction;
	//データベース名
	ModUnicodeString m_cstrDatabaseName;
	//User name
	ModUnicodeString m_cstrUserName;
	//User ID
	int m_iUserID;
	//SuperUser or not
	bool m_bSuperUser;
	//ロックされているか
	bool m_bLocked;
	//現在実行中のSQL文の種別
	int m_iStatementType;
	//PrepareIDのマップ
	PrepareMap m_mapPrepareID;

	//PreparePlanの配列
	ModVector<ModPair<Opt::Planner*, ModUnicodeString*> > m_vecPreparePlan;

	typedef Common::VectorMap<ModUnicodeString, BitSetVariable*, ModLess<ModUnicodeString> > BitSetValueMap;
	BitSetValueMap m_mapBitSetValue;
	
	//Privilege
	bool m_bPrivilegeInitialized;
	Common::Privilege::Value m_vecPrivilege[Common::Privilege::Category::ValueNum];

	// Explain
	ModVector<Opt::Explain::Option::Value> m_vecExplain;
	// 開始時間
	Common::DateTimeData m_cStartTime;

	// 実行中のSQL文
	const ModUnicodeString* m_pCurrentSQL;
	// 実行中のSQL文のパラメータ
	Common::DataArrayData* m_pCurrentParameter;
	// 排他制御用
	Os::CriticalSection m_cLatch;
};

_SYDNEY_SERVER_END
_SYDNEY_END

#endif //__SYDNEY_SERVER_SESSION_H

//
//	Copyright (c) 2002, 2004, 2005, 2007, 2008, 2011, 2012, 2013, 2015, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
