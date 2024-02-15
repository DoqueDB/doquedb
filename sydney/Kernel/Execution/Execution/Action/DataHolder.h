// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Action/DataHolder.h --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_ACTION_DATAHOLDER_H
#define __SYDNEY_EXECUTION_ACTION_DATAHOLDER_H

#ifndef __SYDNEY_DYNAMICCAST_H
#error "SyDynamicCast.h need include"
#endif

#include "Execution/Action/Module.h"
#include "Execution/Declaration.h"

#include "Common/BinaryData.h"
#include "Common/BitSet.h"
#include "Common/DataArrayData.h"
#include "Common/DoubleData.h"
#include "Common/IntegerData.h"
#include "Common/LanguageData.h"
#include "Common/Object.h"
#include "Common/StringData.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/WordData.h"
#include "Opt/Declaration.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

////////////////////////////////////
//	CLASS
//	Execution::Action::DataHolder -- wrapping class for data access in various actions
//
//	NOTES

//////////////////////////////////////////////////////
// DataHolderBase
//////////////////////////////////////////////////////
class DataHolderBase
	: public Common::Object
{
public:
	typedef Common::Object Super;
	typedef DataHolderBase This;

	DataHolderBase()
		: Super(),
		  m_iDataID(-1)
	{}
	DataHolderBase(int iDataID_)
		: Super(),
		  m_iDataID(iDataID_)
	{}
	DataHolderBase(const This& cOther_)
		: Super(),
		  m_iDataID(cOther_.m_iDataID)
	{}

	virtual ~DataHolderBase() {}

	// explain
	void explain(Interface::IProgram& cProgram_,
				 Opt::Explain& cExplain_) const;
	// accessor
	void setDataID(int iID_) {m_iDataID = iID_;}
	int getDataID() {return m_iDataID;}
	// for serialize
	void serialize(ModArchive& archiver_);

	// is valid holder?
	bool isValid() const {return m_iDataID >= 0;}

protected:
	// initialize necessary members
	Common::Data* initializeData(Interface::IProgram& cProgram_);
	// end using members
	void terminateData(Interface::IProgram& cProgram_);

private:
	int m_iDataID;
};

//////////////////////////////////////////////////////
// DataHolderImpl
//////////////////////////////////////////////////////
template <class X_>
class DataHolderImpl
	: public DataHolderBase
{
public:
	typedef DataHolderBase Super;
	typedef DataHolderImpl<X_> This;
	typedef X_ Data;

	DataHolderImpl()
		: Super(),
		  m_pObject(0),
		  m_pData(0),
		  m_bUnfoldable(false)
	{}
	DataHolderImpl(int iDataID_)
		: Super(iDataID_),
		  m_pObject(0),
		  m_pData(0),
		  m_bUnfoldable(false)
	{}
	DataHolderImpl(const This& cOther_)
		: Super(cOther_),
		  m_pObject(0),
		  m_pData(0),
		  m_bUnfoldable(false)
	{}
	virtual ~DataHolderImpl() {}

	// initialize necessary members
	void initialize(Interface::IProgram& cProgram_)
	{
		if (isValid()) {
			m_pData = _SYDNEY_DYNAMIC_CAST(Data*, m_pObject = initializeData(cProgram_));
			m_bUnfoldable = m_pObject && m_pObject->isApplicable(Common::Data::Function::Unfold);
		}
	}
	// end using members
	void terminate(Interface::IProgram& cProgram_)
	{
		terminateData(cProgram_);
		m_pObject = 0;
		m_pData = 0;
		m_pUnfold = Common::Data::Pointer();
		m_bUnfoldable = false;
	}

	// accessor
	Data* get() {return m_pData;}
	const Data* getData() const {return m_bUnfoldable ? unfold() : m_pData;}

	Data* operator->() {return get();}
	const Data* operator->() const {return getData();}
	Data& operator*() {return *get();}
	const Data& operator*() const {return *getData();}

	// check initialized
	bool isInitialized() const {return m_pObject != 0;}

//////////////////////////////
// DataHolderBase::
//	void explain(Interface::IProgram& cProgram_,
//				 Opt::Explain& cExplain_) const;
//	void setDataID(int iID_);
//	void serialize(ModArchive& archiver_);
//	bool isValid();

protected:
private:
	virtual const Data* unfold() const
	{
		if (m_pObject) {
			m_pUnfold = m_pObject->apply(Common::Data::Function::Unfold);
			return _SYDNEY_DYNAMIC_CAST(const Data*, m_pUnfold.get());
		} else {
			return 0;
		}
	}
	// data object converted from m_iDataID
	Common::Data* m_pObject;
	Data* m_pData;
	mutable Common::Data::Pointer m_pUnfold;
	bool m_bUnfoldable;
};

///////////////////////////////////////////////////////
// CLASS
//	Execution::Action::ArrayDataHolder --
//
// NOTES
class ArrayDataHolder
	: public DataHolderImpl<Common::DataArrayData>
{
public:
	typedef DataHolderImpl<Common::DataArrayData> Super;
	typedef ArrayDataHolder This;

	ArrayDataHolder()
		: Super(),
		  m_cUnfoldArray()
	{}
	ArrayDataHolder(int iDataID_)
		: Super(iDataID_),
		  m_cUnfoldArray()
	{}
	ArrayDataHolder(const This& cOther_)
		: Super(cOther_),
		  m_cUnfoldArray()
	{}
	~ArrayDataHolder() {}

protected:
private:
	virtual const Common::DataArrayData* unfold() const;

	mutable Common::DataArrayData m_cUnfoldArray;
};

//////////////////////////////////////////////////////
// other typedef
//////////////////////////////////////////////////////

typedef DataHolderImpl<Common::Data> DataHolder;
typedef DataHolderImpl<Common::BitSet> BitSetHolder;
typedef DataHolderImpl<Common::UnsignedIntegerData> RowIDHolder;
typedef DataHolderImpl<Common::UnsignedIntegerData> UnsignedIntegerDataHolder;
typedef DataHolderImpl<Common::IntegerData> IdentityHolder;
typedef DataHolderImpl<Common::IntegerData> IntegerDataHolder;
typedef DataHolderImpl<Common::StringData> StringDataHolder;
typedef DataHolderImpl<Common::BinaryData> BinaryDataHolder;
typedef DataHolderImpl<Common::LanguageData> LanguageDataHolder;
typedef DataHolderImpl<Common::DoubleData> DoubleDataHolder;
typedef DataHolderImpl<Common::WordData> WordDataHolder;

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ACTION_DATAHOLDER_H

//
//	Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
