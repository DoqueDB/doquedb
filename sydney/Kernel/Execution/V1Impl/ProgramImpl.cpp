// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// V1Impl/ProgramImpl.cpp -- 問合わせ実行プログラム(v1)
// 
// Copyright (c) 1999, 2001, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::V1Impl";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Execution/V1Impl/ProgramImpl.h"

#include "Common/DataArrayData.h"
#include "Common/Functional.h"

#include "Exception/DynamicParameterNotMatch.h"
#include "Exception/NotSupported.h"

#include "Execution/Program.h"

#include "Lock/Mode.h"
#include "Lock/Timeout.h"

#include "Plan/ColumnInfo.h"
#include "Plan/Configuration.h"
#include "Plan/RelationInterface.h"
#include "Plan/ParameterNode.h"
#include "Plan/TableInfo.h"
#include "Plan/Tuple.h"
#include "Plan/Variable.h"
#include "Plan/TypeDef.h"

#include "Schema/Database.h"

#include "Trans/Transaction.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_V1IMPL_BEGIN

///////////////////////////////////////////////////////////
// Execution::V1Impl::ProgramImpl::V1Interface

// FUNCTION public
//	Execution::V1Impl::ProgramImpl::V1Interface::setPlan -- set plan (old version)
//
// NOTES
//
// ARGUMENTS
//	const SHORTVECTOR<Plan::RelationInterfacePointer>& vecPlan_
//	const SHORTVECTOR<Plan::ScalarInterfacePointer>& vecPlaceHolder_
//	bool bUpdate_
//	Trans::Transaction& cTrans_
//	Schema::Database* pDatabase_
//	bool bPreparing_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ProgramImpl::V1Interface::
setPlan(const SHORTVECTOR<Plan::RelationInterfacePointer>& vecPlan_,
		const SHORTVECTOR<Plan::ScalarInterfacePointer>& vecPlaceHolder_,
		bool bUpdate_, Trans::Transaction& cTrans_, Schema::Database* pDatabase_,
		bool bPreparing_)
{
	m_vecPlan = vecPlan_;
	m_vecParameter = vecPlaceHolder_;
	m_bUpdate = bUpdate_;
	m_pTransaction = &cTrans_;
	m_iDatabaseID = pDatabase_->getID();
	m_bPreparing = bPreparing_;

	if (!m_vecParameter.ISEMPTY()) {
		SHORTVECTOR<Plan::ScalarInterfacePointer>::CONSTITERATOR iterator = vecPlaceHolder_.begin();
		const SHORTVECTOR<Plan::ScalarInterfacePointer>::CONSTITERATOR end = vecPlaceHolder_.end();
		do {
			if (!POINTERISNULL(*iterator))
				m_cParameterInfo.add((*iterator)->getColumnInfo());
		} while (++iterator != end);
	}
}

// FUNCTION public
//	Execution::V1Impl::ProgramImpl::V1Interface::setPlan -- set plan from prepare statement
//
// NOTES
//
// ARGUMENTS
//	const Interface::V1Program* pSource_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ProgramImpl::V1Interface::
setPlan(const Interface::IV1Program* pSource_, Trans::Transaction& cTrans_)
{
	const This* pOther = _SYDNEY_DYNAMIC_CAST(const This*, pSource_);
	; _SYDNEY_ASSERT(pOther);
	m_vecPlan = pOther->m_vecPlan;
	m_vecDeterminedPlan = pOther->m_vecDeterminedPlan;
	m_vecParameter = pOther->m_vecParameter;
	m_cParameterInfo = pOther->m_cParameterInfo;
	m_bUpdate = pOther->m_bUpdate;
	m_pTransaction = &cTrans_;
	m_iDatabaseID = pOther->m_iDatabaseID;
	m_bPrepared = pOther->m_bPrepared;
}

// FUNCTION public
//	Execution::V1Impl::ProgramImpl::V1Interface::determine -- determine actually executed plan (old version)
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

//virtual
void
ProgramImpl::V1Interface::
determine()
{
	// If already determined, do nothing
	if (!m_vecDeterminedPlan.ISEMPTY()) return;

	// lock database
	lockDatabase(Lock::Timeout::Unlimited);

	// 保留にしていた部分があれば保留部分を確定し、Programに設定する
	m_vecDeterminedPlan.reserve(m_vecPlan.GETSIZE());
	SHORTVECTOR<Plan::RelationInterfacePointer>::ITERATOR iterator = m_vecPlan.begin();
	const SHORTVECTOR<Plan::RelationInterfacePointer>::ITERATOR& last = m_vecPlan.end();
	for (; iterator != last; ++iterator) {

#ifndef NO_TRACE
		if (_PLAN_IS_OUTPUT_OPTIMIZATION_PROCESS(Plan::Configuration::TraceLevel::Normal)) {
			OSTRSTREAM stream;
			(*iterator)->toString(stream);
			_PLAN_OPTIMIZATION_MESSAGE << "Determining:\n" << stream.getString() << ModEndl;
		}
#endif

		if (!(*iterator)->isDetermined()) {
			// determineの中でProgramに追加される場合もある
			Plan::RelationInterfacePointer pResult = (*iterator)->determine(m_cProgram);
			if (m_bPreparing && POINTERISNULL(pResult)) {
				// Prepare中には確定できなかった
#ifndef NO_TRACE
				if (_PLAN_IS_OUTPUT_OPTIMIZATION_PROCESS(Plan::Configuration::TraceLevel::Normal)) {
					_PLAN_OPTIMIZATION_MESSAGE << "Can't determine for peparing" << ModEndl;
				}
#endif
				m_bPrepared = false;
				m_vecDeterminedPlan.erase(m_vecDeterminedPlan.begin(), m_vecDeterminedPlan.end());
				// Planのキャッシュを捨てるためterminateを呼ぶ
				(*iterator)->terminate(true /* abandon plan */);
				SHORTVECTOR<Plan::RelationInterfacePointer>::ITERATOR rev = m_vecPlan.begin();
				for (; rev != iterator; ++rev)
					(*rev)->terminate(true /* abandon plan */);

				return;
			}
			m_bPrepared = m_bPreparing;
			addDeterminedPlan(pResult);
		} else
			addDeterminedPlan(*iterator);
	}

#ifndef NO_TRACE
	if (_PLAN_IS_OUTPUT_OPTIMIZATION(Plan::Configuration::TraceLevel::Normal)) {
		int i = 0;
		SHORTVECTOR<Plan::RelationInterfacePointer>::ITERATOR iterator = m_vecDeterminedPlan.begin();
		const SHORTVECTOR<Plan::RelationInterfacePointer>::ITERATOR& last = m_vecDeterminedPlan.end();
		for (; iterator != last; ++iterator) {
			OSTRSTREAM stream;
			(*iterator)->toString(stream);
			_PLAN_OPTIMIZATION_MESSAGE << "ResultPlan[" << i++ << "]:\n"
									   << stream.getString() << ModEndl;
		}
	}
#endif
}

// FUNCTION public
//	Execution::V1Impl::ProgramImpl::V1Interface::assignParameter -- assign parameter value
//
// NOTES
//
// ARGUMENTS
//	const Common::DataArrayData* pData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ProgramImpl::V1Interface::
assignParameter(const Common::DataArrayData* pData_)
{
	int iDataCount = (pData_ == 0) ? 0 : pData_->getCount();
	int iTupleCount = m_vecParameter.GETSIZE();

	if (iDataCount < iTupleCount) {
		// パラメーターとPlaceHolderの数が一致しない
		_SYDNEY_THROW2(Exception::DynamicParameterNotMatch,
					   iDataCount, iTupleCount);
	}
	if (iTupleCount > 0) {
		SHORTVECTOR<Plan::ScalarInterfacePointer>::ConstIterator iterator = m_vecParameter.begin();
		int i = 0;
		do {
			if (!POINTERISNULL(*iterator)) {
				Plan::ParameterNode* pParameter = _SYDNEY_DYNAMIC_CAST(Plan::ParameterNode*, (*iterator).get());
				; _SYDNEY_ASSERT(pParameter);
				pParameter->assign(pData_->getElement(i));
			}
			++iterator;
		} while (++i < iTupleCount);
	}
}

// FUNCTION public
//	Execution::V1Impl::ProgramImpl::V1Interface::terminate -- terminate
//
// NOTES
//
// ARGUMENTS
//	bool bForce_ /* = false */
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ProgramImpl::V1Interface::
terminate(bool bForce_ /* = false */)
{
	try {
		// preparedでなければterminateでPlanを破棄するようにtrueを指示する
		FOREACH(m_vecDeterminedPlan.begin(), m_vecDeterminedPlan.end(),
				Common::Functional::MemberFunction(&Plan::RelationInterface::terminate,
												   !m_bPrepared || bForce_));
		FOREACH(m_vecPlan.begin(), m_vecPlan.end(),
				Common::Functional::MemberFunction(&Plan::RelationInterface::terminate,
												   !m_bPrepared || bForce_));
		if (!m_bPrepared || bForce_)
			m_vecDeterminedPlan.erase(m_vecDeterminedPlan.begin(), m_vecDeterminedPlan.end());
	} catch (...) {
		if (!m_bSucceeded) {
			// エラー発生時の呼び出しなので重複して例外を投げないためここでは無視する
			;
		} else {
			// 再度terminateを呼んでみる
			m_bSucceeded = false;
			terminate(bForce_);
			_SYDNEY_RETHROW;
		}
	}
}

// FUNCTION private
//	Execution::V1Impl::ProgramImpl::V1Interface::lockDatabase -- lock database
//
// NOTES
//
// ARGUMENTS
//	Lock::Timeout::Value iTimeout_
//		lock timeout
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ProgramImpl::V1Interface::
lockDatabase(Lock::Timeout::Value iTimeout_)
{
	Lock::Mode::Value eMode;
	Lock::Duration::Value eDuration;
	m_pTransaction->getAdequateLock(Lock::Name::Category::Database,
									!m_bUpdate,
									eMode, eDuration);

	if (eMode != Lock::Mode::N) {
#ifndef NO_TRACE
		if (_PLAN_IS_OUTPUT_EXECUTION(Plan::Configuration::TraceLevel::Lock)) {
			_PLAN_EXECUTION_MESSAGE
				<< "lockDatabase: " << m_iDatabaseID
				<< " " << eMode << ":" << eDuration << ModEndl;
		}
#endif
		m_pTransaction->lock(Lock::DatabaseName(m_iDatabaseID),
							 eMode, eDuration,
							 iTimeout_);
	}
}

//////////////////////////////////////
// Execution::V1Impl::ProgramImpl

// FUNCTION public
//	Execution::V1Impl::ProgramImpl::getV2Interface -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Interface::IV2Program*
//
// EXCEPTIONS

//virtual
Interface::IV2Program*
ProgramImpl::
getV2Interface()
{
	return 0;
}

_SYDNEY_EXECUTION_V1IMPL_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
//	Copyright (c) 1999, 2001, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
