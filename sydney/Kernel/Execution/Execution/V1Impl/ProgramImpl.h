// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// V1Impl/ProgramImpl.h -- プログラム(v1)
// 
// Copyright (c) 1999, 2002, 2004, 2005, 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_V1IMPL_PROGRAMIMPL_H
#define __SYDNEY_EXECUTION_V1IMPL_PROGRAMIMPL_H

#include "Execution/V1Impl/Module.h"
#include "Execution/Interface/IProgramBridge.h"
#include "Execution/Interface/IV1Program.h"

#include "Plan/Configuration.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_V1IMPL_BEGIN

// CLASS
//	Execution::V1Impl::ProgramImpl -- Program implementation for v1
//
// NOTES

class ProgramImpl
		: public Interface::IProgramBridge
{
public:
	typedef ProgramImpl This;
	typedef Interface::IProgramBridge Super;

	// CLASS
	//	Execution::V1Impl::ProgramImpl::V1Interface --
	//
	// NOTES
	class V1Interface
		: public Interface::IV1Program
	{
	public:
		typedef V1Interface This;
		typedef Interface::IV1Program Super;

		//constructor
		V1Interface(Execution::Program& cProgram_)
			: Super(),
			  m_vecPlan(), m_vecDeterminedPlan(), m_vecParameter(), m_pConnection(0),
			  m_bUpdate(false), m_bPreparing(false), m_bPrepared(false),
			  m_bDelayedRetrieval(false), m_bSucceeded(false),
			  m_pTransaction(0), m_iDatabaseID(0), m_cParameterInfo(),
			  m_cProgram(cProgram_)
		{}
		// destructor
		~V1Interface() {}

	////////////////////////////
	// Interface::IV1Program
		// set plan (old version)
		virtual void setPlan(const SHORTVECTOR<Plan::RelationInterfacePointer>& vecPlan_,
							 const SHORTVECTOR<Plan::ScalarInterfacePointer>& vecPlaceHolder_,
							 bool bUpdate_, Trans::Transaction& cTrans_, Schema::Database* pDatabase_,
							 bool bPreparing_);
		// set plan from prepare statement
		virtual void setPlan(const Super* pSource_, Trans::Transaction& cTrans_);

		// plan is set?
		virtual bool isEmpty() const
		{
			return m_vecPlan.ISEMPTY();
		}
		// Check is there any place holder
		virtual bool isHasParameter() const
		{
			return !m_vecParameter.ISEMPTY();
		}

		// determine actually executed plan (old version)
		virtual void determine();
		// add determined plan (old version)
		virtual void addDeterminedPlan(const Plan::RelationInterfacePointer& pPlan_)
		{
			m_vecDeterminedPlan.PUSHBACK(pPlan_);

#ifndef NO_TRACE
			if (_PLAN_IS_OUTPUT_OPTIMIZATION_PROCESS(Plan::Configuration::TraceLevel::Normal)) {
				OSTRSTREAM stream;
				pPlan_->toString(stream);
				_PLAN_OPTIMIZATION_MESSAGE << "Add determined plan:\n" << stream.getString() << ModEndl;
			}
#endif
		}

		// get the number of plans executed
		virtual int getPlanSize() const
		{
			return m_vecDeterminedPlan.GETSIZE();
		}

		// get a plan at specified position
		virtual Plan::RelationInterfacePointer getPlan(int iPos_) const
		{
			return m_vecDeterminedPlan[iPos_];
		}

		// assign parameter value
		virtual void assignParameter(const Common::DataArrayData* pData_);

		// set Connection
		virtual void setConnection(Communication::Connection* pConnection_)
		{
			m_pConnection = pConnection_;
		}
		// get specified connection
		virtual Communication::Connection* getConnection() const
		{
			return m_pConnection;
		}

		// initialize
		virtual void initialize()
		{
			// 実行中にエラーが起きたかを判断するためのフラグをリセットしておく
			m_bSucceeded = false;
		}
		// terminate
		virtual void terminate(bool bForce_ = false);

		// set as succeeded
		virtual void succeeded()
		{
			m_bSucceeded = true;
		}

		// accessors
		virtual Trans::Transaction* getTransaction()
		{return m_pTransaction;}
		virtual bool isPreparing() const
		{return m_bPreparing;}
		virtual const Plan::ColumnInfoSet& getParameterInfo() const
		{return m_cParameterInfo;}

		virtual void setDelayedRetrieval(bool bFlag_)
		{m_bDelayedRetrieval = bFlag_;}
		virtual bool isDelayedRetrieval() const
		{return m_bDelayedRetrieval;}
	protected:
	private:
		void lockDatabase(Lock::Timeout::Value iTimeout_);

		SHORTVECTOR<Plan::RelationInterfacePointer> m_vecPlan;
		SHORTVECTOR<Plan::RelationInterfacePointer> m_vecDeterminedPlan;
		SHORTVECTOR<Plan::ScalarInterfacePointer> m_vecParameter;
		Plan::ColumnInfoSet m_cParameterInfo;
		Communication::Connection* m_pConnection;
		bool m_bUpdate;
		bool m_bPreparing;
		bool m_bPrepared;
		bool m_bDelayedRetrieval;
		bool m_bSucceeded;
		Trans::Transaction* m_pTransaction;
		UINT32 m_iDatabaseID;
		Execution::Program& m_cProgram;
	};

	// constructor
	ProgramImpl(Execution::Program& cProgram_)
		: Super(),
		  m_cInterface(cProgram_)
	{}

	// destructor
	~ProgramImpl() {}

///////////////////////////////
// Interface::IProgramBridge
	virtual Version::Value getVersion()
	{
		return Version::V1;
	}
	virtual Interface::IV1Program* getV1Interface()
	{
		return &m_cInterface;
	}
	virtual Interface::IV2Program* getV2Interface();

protected:
private:
	V1Interface m_cInterface;
};

_SYDNEY_EXECUTION_V1IMPL_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_V1IMPL_PROGRAMIMPL_H

//
//	Copyright (c) 1999, 2002, 2004, 2005, 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
