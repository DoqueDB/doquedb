// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File/Parameter.cpp --
// 
// Copyright (c) 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::File";
}

#include "SyDefault.h"

#include "Plan/File/Parameter.h"

#include "Plan/AccessPlan/Cost.h"
#include "Plan/Utility/ObjectSet.h"

#include "LogicalFile/OpenOption.h"

#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_FILE_BEGIN

namespace Impl
{
	class ParameterImpl
		: public File::Parameter
	{
	public:
		typedef File::Parameter Super;
		typedef ParameterImpl This;

		// constructor
		ParameterImpl()
			: Super(),
			  m_cOpenOption(),
			  m_cCost(),
			  m_pPredicate(0),
			  m_pOrder(0),
			  m_bIsLimit(0),
			  m_bIsGetByBitSet(false),
			  m_bIsSearchByBitSet(false)
		{}
		ParameterImpl(const LogicalFile::OpenOption& cOpenOption_,
					  const AccessPlan::Cost& cCost_)
			: Super(),
			  m_cOpenOption(cOpenOption_),
			  m_cCost(cCost_),
			  m_pPredicate(0),
			  m_pOrder(0),
			  m_bIsLimit(0),
			  m_bIsGetByBitSet(false),
			  m_bIsSearchByBitSet(false)
		{}

	//////////////////
	// Parameter::
		virtual LogicalFile::OpenOption& getOpenOption() {return m_cOpenOption;}
		virtual AccessPlan::Cost& getCost() {return m_cCost;}

		virtual Interface::IPredicate* getPredicate() {return m_pPredicate;}
		virtual Order::Specification* getOrder() {return m_pOrder;}
		virtual bool isLimited() {return m_bIsLimit;}
		virtual bool isGetByBitSet() {return m_bIsGetByBitSet;}
		virtual bool isSearchByBitSet() {return m_bIsSearchByBitSet;}

		virtual void setPredicate(Interface::IPredicate* pPredicate_)
		{m_pPredicate = pPredicate_;}
		virtual void setOrder(Order::Specification* pOrder_)
		{m_pOrder = pOrder_;}
		virtual void setIsLimited(bool bFlag_)
		{m_bIsLimit = bFlag_;}
		virtual void setIsGetByBitSet(bool bFlag_)
		{m_bIsGetByBitSet = bFlag_;}
		virtual void setIsSearchByBitSet(bool bFlag_)
		{m_bIsSearchByBitSet = bFlag_;}

	protected:
	private:
		LogicalFile::OpenOption m_cOpenOption;
		AccessPlan::Cost m_cCost;

		Plan::Interface::IPredicate* m_pPredicate;
		Plan::Order::Specification* m_pOrder;
		bool m_bIsLimit;
		bool m_bIsGetByBitSet;
		bool m_bIsSearchByBitSet;
	};
}

//////////////////////////////////
// File::Parameter
//////////////////////////////////

// FUNCTION public
//	File::Parameter::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Parameter*
//
// EXCEPTIONS

//static
Parameter*
Parameter::
create(Opt::Environment& cEnvironment_)
{
	AUTOPOINTER<This> pResult = new Impl::ParameterImpl;
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	File::Parameter::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const LogicalFile::OpenOption& cOpenOption_
//	const AccessPlan::Cost& cCost_
//	
// RETURN
//	Parameter*
//
// EXCEPTIONS

//static
Parameter*
Parameter::
create(Opt::Environment& cEnvironment_,
	   const LogicalFile::OpenOption& cOpenOption_,
	   const AccessPlan::Cost& cCost_)
{
	AUTOPOINTER<This> pResult = new Impl::ParameterImpl(cOpenOption_, cCost_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	File::Parameter::erase -- erase object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	File::Parameter* pThis_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Parameter::
erase(Opt::Environment& cEnvironment_,
	  This* pThis_)
{
	if (pThis_) {
		pThis_->eraseFromEnvironment(cEnvironment_);
		delete pThis_;
	}
}

// FUNCTION private
//	File::Parameter::registerToEnvironment -- register to environment
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Parameter::
registerToEnvironment(Opt::Environment& cEnvironment_)
{
	m_iID = cEnvironment_.addObject(this);
}

// FUNCTION private
//	File::Parameter::eraseFromEnvironment -- erase from environment
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Parameter::
eraseFromEnvironment(Opt::Environment& cEnvironment_)
{
	cEnvironment_.eraseObject(m_iID);
}

_SYDNEY_PLAN_FILE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
