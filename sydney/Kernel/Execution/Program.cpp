// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Program.cpp -- 問合わせ実行プログラム
// 
// Copyright (c) 1999, 2001, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Execution/Interface/IProgramBridge.h"
#ifdef USE_OLDER_VERSION
#include "Execution/Interface/IV1Program.h"
#include "Execution/V1Impl/ProgramImpl.h"
#endif
#include "Execution/V2Impl/ProgramImpl.h"

#include "Exception/NotSupported.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN

//////////////////////////////////////
// Execution::Program

// FUNCTION public
//	Execution::Program::~Program -- デストラクタ
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

//virtual
Program::
~Program()
{
	try {
		delete m_pImpl, m_pImpl = 0;
	} catch (...) {
		// ignore exceptions in destructor
	}
}

// FUNCTION public
//	Execution::Program::setImplementation -- 実装クラスを設定する
//
// NOTES
//
// ARGUMENTS
//	Version::Value iVersion_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Program::
setImplementation(Version::Value iVersion_)
{
	if (m_pImpl) delete m_pImpl;
	switch (iVersion_)
	{
#ifdef USE_OLDER_VERSION
	case Version::V1:
		{
			m_pImpl = new V1Impl::ProgramImpl(*this);
			break;
		}
#endif
	case Version::V2:
		{
			m_pImpl = new V2Impl::ProgramImpl;
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
}

#ifdef USE_OLDER_VERSION
// FUNCTION public
//	Execution::Program::setPlan -- 実行するプランを設定する
//
// NOTES
//
// ARGUMENTS
//	const SHORTVECTOR<Plan::RelationInterfacePointer>& vecPlan_
//	const SHORTVECTOR<Plan::ScalarInterfacePointer>& vecPlaceHolder_
//	bool bUpdate_
//	Trans::Transaction& cTrans_
//	Schema::Database* pDatabase_
//	bool bPreparing_ = false
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Program::
setPlan(const SHORTVECTOR<Plan::RelationInterfacePointer>& vecPlan_,
		const SHORTVECTOR<Plan::ScalarInterfacePointer>& vecPlaceHolder_,
		bool bUpdate_, Trans::Transaction& cTrans_, Schema::Database* pDatabase_,
		bool bPreparing_ /* = false */)
{
	m_pImpl->getV1Interface()->setPlan(vecPlan_, vecPlaceHolder_, bUpdate_, cTrans_, pDatabase_, bPreparing_);
}

// FUNCTION public
//	Execution::Program::setPlan -- Prepareで作ったものから設定する
//
// NOTES
//
// ARGUMENTS
//	const Program& cSource_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Program::
setPlan(const Program& cSource_, Trans::Transaction& cTrans_)
{
	m_pImpl->getV1Interface()->setPlan(cSource_.m_pImpl->getV1Interface(), cTrans_);
}

// FUNCTION public
//	Execution::Program::isEmpty -- 実行するプランがあるか
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
Program::
isEmpty() const
{
	return m_pImpl->getV1Interface()->isEmpty();
}

// FUNCTION public
//	Execution::Program::isHasParameter -- Check is there any place holder
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
Program::
isHasParameter() const
{
	return m_pImpl->getV1Interface()->isHasParameter();
}

// FUNCTION public
//	Execution::Program::determine -- 実際に実行するプランを設定する
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
Program::
determine()
{
	m_pImpl->getV1Interface()->determine();
}

// FUNCTION public
//	Execution::Program::addDeterminedPlan -- 実際に実行するプランを追加する
//
// NOTES
//
// ARGUMENTS
//	const Plan::RelationInterfacePointer& pPlan_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Program::
addDeterminedPlan(const Plan::RelationInterfacePointer& pPlan_)
{
	m_pImpl->getV1Interface()->addDeterminedPlan(pPlan_);
}

// FUNCTION public
//	Execution::Program::getPlanSize -- 実行するプランの数を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
Program::
getPlanSize() const
{
	return m_pImpl->getV1Interface()->getPlanSize();
}

// FUNCTION public
//	Execution::Program::getPlan -- プランをひとつ得る
//
// NOTES
//
// ARGUMENTS
//	int iPos_
//	
// RETURN
//	Plan::RelationInterfacePointer
//
// EXCEPTIONS

Plan::RelationInterfacePointer
Program::
getPlan(int iPos_) const
{
	return m_pImpl->getV1Interface()->getPlan(iPos_);
}

// FUNCTION public
//	Execution::Program::assignParameter -- パラメーターの値を設定する
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

void
Program::
assignParameter(const Common::DataArrayData* pData_)
{
	m_pImpl->getV1Interface()->assignParameter(pData_);
}

// FUNCTION public
//	Execution::Program::setConnection -- Connectionを設定する
//
// NOTES
//
// ARGUMENTS
//	Communication::Connection* pConnection_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Program::
setConnection(Communication::Connection* pConnection_)
{
	m_pImpl->getV1Interface()->setConnection(pConnection_);
}

// FUNCTION public
//	Execution::Program::getConnection -- 設定されているConnectionを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Communication::Connection*
//
// EXCEPTIONS

Communication::Connection*
Program::
getConnection() const
{
	return m_pImpl->getV1Interface()->getConnection();
}

// FUNCTION public
//	Execution::Program::initialize -- 初期化処理をする
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
Program::
initialize()
{
	m_pImpl->getV1Interface()->initialize();
}

// FUNCTION public
//	Execution::Program::terminate -- 後処理をする
//
// NOTES
//
// ARGUMENTS
//	bool bForce_ = false
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Program::
terminate(bool bForce_ /* = false */)
{
	m_pImpl->getV1Interface()->terminate(bForce_);
}

// FUNCTION public
//	Execution::Program::succeeded -- 正常終了したことをセットする
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
Program::
succeeded()
{
	m_pImpl->getV1Interface()->succeeded();
}

// FUNCTION public
//	Execution::Program::getTransaction -- アクセサ
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Trans::Transaction*
//
// EXCEPTIONS

Trans::Transaction*
Program::
getTransaction()
{
	return m_pImpl->getV1Interface()->getTransaction();
}

// FUNCTION public
//	Execution::Program::isPreparing -- アクセサ
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
Program::
isPreparing() const
{
	return m_pImpl->getV1Interface()->isPreparing();
}

// FUNCTION public
//	Execution::Program::getParameterInfo -- アクセサ
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Plan::ColumnInfoSet&
//
// EXCEPTIONS

const Plan::ColumnInfoSet&
Program::
getParameterInfo() const
{
	return m_pImpl->getV1Interface()->getParameterInfo();
}

// FUNCTION public
//	Execution::Program::setDelayedRetrieval -- アクセサ
//
// NOTES
//
// ARGUMENTS
//	bool bFlag_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Program::
setDelayedRetrieval(bool bFlag_)
{
	m_pImpl->getV1Interface()->setDelayedRetrieval(bFlag_);
}

// FUNCTION public
//	Execution::Program::isDelayedRetrieval -- アクセサ
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
Program::
isDelayedRetrieval() const
{
	return m_pImpl->getV1Interface()->isDelayedRetrieval();
}
#endif

// FUNCTION public
//	Execution::Program::getProgram -- get new interface
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Interface::IProgram*
//
// EXCEPTIONS

Interface::IProgram*
Program::
getProgram()
{
	return m_pImpl->getV2Interface()->getProgram();
}

// FUNCTION public
//	Execution::Program::setProgram -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram* pProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Program::
setProgram(Interface::IProgram* pProgram_)
{
	m_pImpl->getV2Interface()->setProgram(pProgram_);
}

// FUNCTION public
//	Execution::Program::releaseProgram -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Interface::IProgram*
//
// EXCEPTIONS

Interface::IProgram*
Program::
releaseProgram()
{
	return m_pImpl->getV2Interface()->releaseProgram();
}

_SYDNEY_EXECUTION_END
_SYDNEY_END

//
//	Copyright (c) 1999, 2001, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
