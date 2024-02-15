// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Action/Collection.h --
// 
// Copyright (c) 2009, 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_ACTION_COLLECTION_H
#define __SYDNEY_EXECUTION_ACTION_COLLECTION_H

#include "Execution/Action/Module.h"

#include "Execution/Declaration.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/ICollection.h"

#include "Common/Object.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

////////////////////////////////////
//	CLASS
//	Execution::Action::Collection -- wrapping class for collection access in various actions
//
//	NOTES
class Collection
	: public Common::Object
{
public:
	typedef Common::Object Super;
	typedef Collection This;

	Collection()
		: Super(),
		  m_iCollectionID(-1),
		  m_pCollection(0),
		  m_cGetData(),
		  m_cPutData(),
		  m_pGet(0),
		  m_pPut(0)
	{}
	Collection(int iCollectionID_,
			   int iDataID_)
		: Super(),
		  m_iCollectionID(iCollectionID_),
		  m_pCollection(0),
		  m_cGetData(iDataID_),
		  m_cPutData(iDataID_),
		  m_pGet(0),
		  m_pPut(0)
	{}
	~Collection() {}

	// explain
	void explain(Opt::Environment* pEnvironment_,
				 Interface::IProgram& cProgram_,
				 Opt::Explain& cExplain_);
	// initialize necessary members
	void initialize(Interface::IProgram& cProgram_);
	// end using members
	void terminate(Interface::IProgram& cProgram_);

	// accessor
	int getCollectionID() {return m_iCollectionID;}
	void setCollectionID(int iID_) {m_iCollectionID = iID_;}
	void setDataID(int iID_) {setGetDataID(iID_);setPutDataID(iID_);}
	void setGetDataID(int iID_) {m_cGetData.setDataID(iID_);}
	void setPutDataID(int iID_) {m_cPutData.setDataID(iID_);}

	bool isGetDataValid() {return m_cGetData.isValid();}
	bool isPutDataValid() {return m_cPutData.isValid();}

	int getGetDataID() {return m_cGetData.getDataID();}
	int getPutDataID() {return m_cPutData.getDataID();}

	Interface::ICollection* getCollection() {return m_pCollection;}
	Common::Data* getData() {return getGetData();}
	Common::Data* getGetData() {return m_cGetData.get();}
	const Common::Data* getPutData() {return m_cPutData.getData();}

	void explainGetData(Interface::IProgram& cProgram_,
						Opt::Explain& cExplain_);
	void explainPutData(Interface::IProgram& cProgram_,
						Opt::Explain& cExplain_);

	// check initialized
	bool isInitialized() {return m_pCollection != 0;}

	// for get
	void prepareGetInterface();
	bool get(Interface::IProgram& cProgram_);
	bool get(Interface::IProgram& cProgram_, int iPosition_);
	void reset();
	// for put
	void preparePutInterface();
	bool put(Interface::IProgram& cProgram_);
	int getLastPosition();
	void shift(Interface::IProgram& cProgram_);
	void flush();
	// for both
	void finish(Interface::IProgram& cProgram_);
	void clear();
	bool isEmpty();
	bool isEmptyGrouping();
	bool isGetNextOperand();

	// for serialize
	static This* getInstance(int iCategory_);

	void serialize(ModArchive& archiver_);

protected:
private:
	// instance ID of collection from which tuple data is read
	int m_iCollectionID;
	// data holder for get interface
	DataHolder m_cGetData;
	// data holder for put interface
	DataHolder m_cPutData;

	// collection object converted from m_iCollectionID
	Interface::ICollection* m_pCollection;
	// get/put interface
	Interface::ICollection::Get* m_pGet;
	Interface::ICollection::Put* m_pPut;
};

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ACTION_COLLECTION_H

//
//	Copyright (c) 2009, 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
