// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Cascade.cpp -- Resource holder for distributed access to sub-servers
// 
// Copyright (c) 2012, 2013, 2015, 2023 Ricoh Company, Ltd.
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
	const char moduleName[] = "DServer";
	const char srcFile[] = __FILE__;
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "DServer/Cascade.h"
#include "DServer/DataSource.h"
#include "DServer/Session.h"

#include "Common/Assert.h"
#include "Common/BinaryData.h"
#include "Common/IntegerData.h"
#include "Common/Message.h"

#include "Exception/CascadeNotFound.h"
#include "Exception/ModLibraryError.h"

#include "Opt/Algorithm.h"

#include "Os/AutoRWLock.h"
#include "Os/RWLock.h"

#include "Schema/Cascade.h"
#include "Schema/Database.h"

_SYDNEY_USING
_SYDNEY_DSERVER_USING

namespace _Impl
{
	// lock for protecting following resources
	Os::RWLock _rwLock;

	// Map dbid->(Map svrname->datasource) of datasource
	typedef Common::VectorMap<ModUnicodeString,
							  DataSource*,
							  Opt::CaseInsensitiveComparator> DataSourceMap;
	typedef Common::VectorMap<Schema::Object::ID::Value,
							  DataSourceMap,
							  ModLess<Schema::Object::ID::Value> > CascadeMap;

	CascadeMap _mapCascade;

	// get datasource map
	DataSourceMap& getDataSourceMap(const Schema::Database& cDatabase_,
									Trans::Transaction& cTrans_);
	// add datasource entry to the map
	void addDataSource(Trans::Transaction& cTrans_,
					   const Schema::Cascade* pCascade_,
					   _Impl::DataSourceMap& cMap_);
	// delete datasource map for a database
	void resetDataSourceMap(const Schema::Database& cDatabase_);

	// clear datasource map
	void clearDataSourceMap();

	// clear one datasource map
	void clearOneMap(DataSourceMap& cMap_);

	// encode xid
	ModUnicodeString encodeXid(const Common::DataArrayData& cTuple_);
	// forget transaction branch
	void forgetBranch(const ModUnicodeString& cstrServerName_, DataSource* pDataSource_);

	// システムデータベース
	ModUnicodeString _cstrSystemDatabase("$$SystemDB");
	
} // namespace

//////////////////////////
// $$$::_Impl::

// FUNCTION local
//	$$$::_Impl::getDataSourceMap -- 
//
// NOTES
//
// ARGUMENTS
//	const Schema::Database& cDatabase_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	_Impl::DataSourceMap&
//
// EXCEPTIONS

_Impl::DataSourceMap&
_Impl::
getDataSourceMap(const Schema::Database& cDatabase_,
				 Trans::Transaction& cTrans_)
{
	Os::AutoTryRWLock lock(_rwLock);
	lock.lock(Os::RWLock::Mode::Read);
	CascadeMap::Iterator iterator = _mapCascade.find(cDatabase_.getID());
	if (iterator == _mapCascade.end()) {
		
		lock.unlock();
		lock.lock(Os::RWLock::Mode::Write);

		// もう一度確認
		iterator = _mapCascade.find(cDatabase_.getID());
		if (iterator == _mapCascade.end()) {
		
			DataSourceMap mapDataSource;

			try {
				// get all cascade for the database
				ModVector<Schema::Cascade*> vecSchemaCascade =
					cDatabase_.getCascade(cTrans_);
				Opt::ForEach(vecSchemaCascade,
							 boost::bind(&_Impl::addDataSource,
										 boost::ref(cTrans_),
										 _1,
										 boost::ref(mapDataSource)));
			} catch (...) {
				// clear datasourcemap
				clearOneMap(mapDataSource);
				throw;
			}

			iterator
				= _mapCascade.insert(cDatabase_.getID(), mapDataSource).first;
		}
	}
	return (*iterator).second;
}

// FUNCTION local
//	$$$::_Impl::addDataSource -- add datasource entry to the map
//
// NOTES
//
// ARGUMENTS
//	const Schema::Database& cDatabase_
//	Trans::Transaction& cTrans_
//	const Schema::Cascade* pCascade_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
_Impl::
addDataSource(Trans::Transaction& cTrans_,
			  const Schema::Cascade* pCascade_,
			  _Impl::DataSourceMap& cMap_)
{
	const Schema::Cascade::Target& cCascadeTarget = pCascade_->getTarget();
	ModAutoPointer<DataSource> pDataSource = DataSource::create(cCascadeTarget.m_cstrHost,
																cCascadeTarget.m_cstrPort,
																cCascadeTarget.m_cstrDatabase);
	//
	// ヒューリスティックに解決したトランザクションブランチがあるか確認し、
	// あれば、すべて忘れる
	//

	forgetBranch(pCascade_->getName(), pDataSource.get());
	
	cMap_.insert(pCascade_->getName(), pDataSource.release());
}

// FUNCTION local
//	$$$::_Impl::resetDataSourceMap -- delete datasource map for a database
//
// NOTES
//
// ARGUMENTS
//	const Schema::Database& cDatabase_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
_Impl::
resetDataSourceMap(const Schema::Database& cDatabase_)
{
	Os::AutoRWLock lock(_rwLock, Os::RWLock::Mode::Write);
	CascadeMap::Iterator iterator = _mapCascade.find(cDatabase_.getID());
	if (iterator != _mapCascade.end()) {
		DataSourceMap& cMap = (*iterator).second;
		clearOneMap(cMap);
		_mapCascade.erase(iterator);
	}
}

// FUNCTION local
//	$$$::_Impl::clearDataSourceMap -- clear datasource map
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
_Impl::
clearDataSourceMap()
{
	Os::AutoRWLock lock(_rwLock, Os::RWLock::Mode::Write);
	CascadeMap::Iterator cIterator = _mapCascade.begin();
	const CascadeMap::Iterator cLast = _mapCascade.end();
	for (; cIterator != cLast; ++cIterator) {
		DataSourceMap& cMap = (*cIterator).second;
		clearOneMap(cMap);
	}
	_mapCascade.clear();
}

// FUNCTION local
//	$$$::_Impl::clearOneMap -- 
//
// NOTES
//
// ARGUMENTS
//	DataSourceMap& cMap_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
_Impl::
clearOneMap(DataSourceMap& cMap_)
{
	DataSourceMap::Iterator iterator = cMap_.begin();
	const DataSourceMap::Iterator last = cMap_.end();
	for (; iterator != last; ++iterator) {
		DataSource::erase((*iterator).second);
	}
	cMap_.clear();
}

//
//	FUNCTION local
//	_$$::_Impl::encodeXid
//		-- トランザクションブランチ識別子を文字列に符号化する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cTuple_
//		トランザクションブランチ識別子
//
//	RETURN
//	ModUnicodeString
//		トランザクションブランチ識別子のSQL文表記
//
//	EXCEPTIONS
//
ModUnicodeString
_Impl::encodeXid(const Common::DataArrayData& cTuple_)
{
	if (cTuple_.getCount() < 1)
		_SYDNEY_THROW0(Exception::BadArgument);

	ModUnicodeOstrStream s;

	{
		// グローバルトランザクション識別子
		
		Common::Data::Pointer p = cTuple_.getElement(0);
		if (p->isNull() || p->getType() != Common::DataType::Binary)
			_SYDNEY_THROW0(Exception::BadArgument);
		const Common::BinaryData& b
			= _SYDNEY_DYNAMIC_CAST(const Common::BinaryData&, *p);
		
		s << "X'" << b.encodeString() << "'";
	}

	if (cTuple_.getCount() >= 2)
	{
		// トランザクションブランチ限定子
		
		Common::Data::Pointer p = cTuple_.getElement(1);
		if (p->isNull() == false &&
			p->getType() == Common::DataType::Binary)
		{
			const Common::BinaryData& b
				= _SYDNEY_DYNAMIC_CAST(const Common::BinaryData&, *p);
			
			if (b.getSize() > 0)
			{
				s << ", X'" << b.encodeString() << "'";

				if (cTuple_.getCount() >= 3)
				{
					// フォーマット識別子
					
					p = cTuple_.getElement(2);
					if (p->isNull() == false &&
						p->getType() == Common::DataType::Integer)
					{
						const Common::IntegerData& d
							= _SYDNEY_DYNAMIC_CAST(const Common::IntegerData&,
												   *p);

						s << ", " << d.getValue();
					}
				}
			}
		}
	}

	return ModUnicodeString(s.getString());
}

//
// 	FUNCTION local
//	_$$::_Impl::forgetBranch
//		-- 子サーバにヒューリスティックに解決したトランザクションブランチが
//		   あったら、すべて忘れる
//
//	NOTES
//
//	ARGUMENTS
//	DServer::DataSource* pDataSource_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
_Impl::forgetBranch(const ModUnicodeString& cstrServerName_, DataSource* pDataSource_)
{
	Session* pSession = pDataSource_->createSession(_cstrSystemDatabase,
													cstrServerName_);

	try
	{
		//
		//	xa recover を実行し、ヒューリスティックに解決した
		//	トランザクションブランチのリストを得る
		//
	
		ResultSet* pResultSet = pSession->executeStatement(
			ModUnicodeString("xa recover"));
	
		ResultSet::Status::Value stat;
		ModVector<ModUnicodeString> vecXid;
	
		try
		{
			do
			{
				Common::DataArrayData cTuple;
				stat = pResultSet->getNextTuple(&cTuple);
			
				if (stat == ResultSet::Status::Data)
				{
					ModUnicodeOstrStream s;
			
					// 文字列に変換する
					s << "xa forget "
					  << encodeXid(cTuple);

					// 後で実行するので、配列に格納する
					vecXid.pushBack(ModUnicodeString(s.getString()));
				}
			}
			while (stat == ResultSet::Status::MetaData ||
				   stat == ResultSet::Status::Data ||
				   stat == ResultSet::Status::EndOfData ||
				   stat == ResultSet::Status::HasMoreData);
		}
		catch (Exception::Object& e)
		{
			SydErrorMessage << e << ModEndl;
		}
		catch (ModException& e)
		{
			SydErrorMessage << Exception::ModLibraryError(moduleName,
														  srcFile,
														  __LINE__,
														  e)
							<< ModEndl;
		}
		catch (...)
		{
			SydErrorMessage << "Unexpected Exception" << ModEndl;
		}

		pResultSet->close();
		ResultSet::erase(pResultSet);

		//
		//	ヒューリスティックに解決したトランザクションブランチに
		//	xa forget を実行する
		//

		ModVector<ModUnicodeString>::Iterator i = vecXid.begin();
		for (; i != vecXid.end(); ++i)
		{
			pResultSet = pSession->executeStatement(*i);
		
			try
			{
				pResultSet->getStatus();
			}
			catch (Exception::Object& e)
			{
				SydErrorMessage << e << ModEndl;
			}
			catch (ModException& e)
			{
				SydErrorMessage << Exception::ModLibraryError(moduleName,
															  srcFile,
															  __LINE__,
															  e)
								<< ModEndl;
			}
			catch (...)
			{
				SydErrorMessage << "Unexpected Exception" << ModEndl;
			}
		
			pResultSet->close();
			ResultSet::erase(pResultSet);
		}
	}
	catch (...)
	{
		Session::erase(pSession);
		_SYDNEY_RETHROW;
	}
	Session::erase(pSession);
}

//////////////////////////
// DServer::Cascade::

// FUNCTION public
//	DServer::Cascade::initialize -- initialize
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

//static
void
Cascade::
initialize()
{
}

// FUNCTION public
//	DServer::Cascade::terminate -- terminate
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

//static
void
Cascade::
terminate()
{
	_Impl::clearDataSourceMap();
}

// FUNCTION public
//	DServer::Cascade::resetCascade -- tell cascade change
//
// NOTES
//
// ARGUMENTS
//	const Schema::Database& cDatabase_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Cascade::
resetCascade(const Schema::Database& cDatabase_)
{
	_Impl::resetDataSourceMap(cDatabase_);
}

// FUNCTION public
//	DServer::Cascade::createSession -- create new session to access the server
//
// NOTES
//
// ARGUMENTS
//	const Schema::Database& cDatabase_
//	const ModUnicodeString& cstrServerName_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Session*
//
// EXCEPTIONS

//static
Session*
Cascade::
createSession(const Schema::Database& cDatabase_,
			  const ModUnicodeString& cstrServerName_,
			  Trans::Transaction& cTrans_)
{
	_Impl::DataSourceMap& cMap = _Impl::getDataSourceMap(cDatabase_, cTrans_);
	_Impl::DataSourceMap::Iterator found = cMap.find(cstrServerName_);
	if (found == cMap.end()) {
		_SYDNEY_THROW2(Exception::CascadeNotFound,
					   cstrServerName_, cDatabase_.getName());
	}
	DataSource* pDataSource = (*found).second;
	; _SYDNEY_ASSERT(pDataSource);

	return pDataSource->createSession(cDatabase_.getName(), cstrServerName_);
}

// FUNCTION public
//	DServer::Cascade::getSession -- create new sessions to access all the sub-server
//
// NOTES
//
// ARGUMENTS
//	const Schema::Database& cDatabase_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Common::LargeVector<Session*>
//
// EXCEPTIONS

//static
Common::LargeVector<Session*>
Cascade::
getSession(const Schema::Database& cDatabase_,
		   Trans::Transaction& cTrans_)
{
	Common::LargeVector<Session*> vecResult;

	_Impl::DataSourceMap& cMap = _Impl::getDataSourceMap(cDatabase_, cTrans_);
	_Impl::DataSourceMap::Iterator iterator = cMap.begin();
	const _Impl::DataSourceMap::Iterator last = cMap.end();
	for (; iterator != last; ++iterator) {
		vecResult.pushBack((*iterator).second->createSession(cDatabase_.getName(), (*iterator).first));
	}
	return vecResult;
}

// FUNCTION public
//	DServer::Cascade::getSession -- create new sessions to access all the sub-server
//
// NOTES
//
// ARGUMENTS
//	const Schema::Database& cDatabase_
//	Trans::Transaction& cTrans_
//	const ModUnicodeString& cstrUserName_
//	const ModUnicodeString& cstrPassword_
//	
// RETURN
//	Common::LargeVector<Session*>
//
// EXCEPTIONS

//static
Common::LargeVector<Session*>
Cascade::
getSession(const Schema::Database& cDatabase_,
		   Trans::Transaction& cTrans_,
		   const ModUnicodeString& cstrUserName_,
		   const ModUnicodeString& cstrPassword_)
{
	Common::LargeVector<Session*> vecResult;

	_Impl::DataSourceMap& cMap = _Impl::getDataSourceMap(cDatabase_, cTrans_);
	_Impl::DataSourceMap::Iterator iterator = cMap.begin();
	const _Impl::DataSourceMap::Iterator last = cMap.end();
	for (; iterator != last; ++iterator) {
		vecResult.pushBack(
			(*iterator).second->createSession(cDatabase_.getName(),
											  (*iterator).first,
											  cstrUserName_,
											  cstrPassword_));
	}
	return vecResult;
}

// FUNCTION public
//	DServer::Cascade::eraseSession -- erase all sessions created by getSession
//
// NOTES
//
// ARGUMENTS
//	Common::LargeVector<Session*>& vecSession_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Cascade::
eraseSession(Common::LargeVector<Session*>& vecSession_)
{
	Opt::ForEach(vecSession_,
				 boost::bind(&DServer::Session::erase,
							 _1));
}

//
// Copyright (c) 2012, 2013, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
