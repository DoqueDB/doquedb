// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File/Parameter.h --
// 
// Copyright (c) 2008, 2009, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_FILE_PARAMETER_H
#define __SYDNEY_PLAN_FILE_PARAMETER_H

#include "Plan/Declaration.h"
#include "Plan/Utility/Algorithm.h"

#include "Common/Object.h"

_SYDNEY_BEGIN

namespace LogicalFile
{
	class OpenOption;
}

_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_FILE_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::File::Parameter -- 
//
//	NOTES
class Parameter
	: public Common::Object
{
public:
	typedef Common::Object Super;
	typedef Parameter This;
	typedef int Position;

	// constructor
	static This* create(Opt::Environment& cEnvironment_);
	static This* create(Opt::Environment& cEnvironment_,
						const LogicalFile::OpenOption& cOpenOption_,
						const AccessPlan::Cost& cCost_);

	// destructor
	virtual ~Parameter() {}

	// erase object
	static void erase(Opt::Environment& cEnvironment_,
					  This* pThis_);

	// get OpenOption
	virtual LogicalFile::OpenOption& getOpenOption() = 0;
	// get cost for the openoption
	virtual AccessPlan::Cost& getCost() = 0;

	// get parameter source data
	virtual Interface::IPredicate* getPredicate() = 0;
	virtual Order::Specification* getOrder() = 0;
	virtual bool isLimited() = 0;
	virtual bool isGetByBitSet() = 0;
	virtual bool isSearchByBitSet() = 0;

	// set parameter source data
	virtual void setPredicate(Interface::IPredicate* pPredicate_) = 0;
	virtual void setOrder(Order::Specification* pOrder_) = 0;
	virtual void setIsLimited(bool bFlag_) = 0;
	virtual void setIsGetByBitSet(bool bFlag_) = 0;
	virtual void setIsSearchByBitSet(bool bFlag_) = 0;

protected:
	// constructor
	Parameter() : Super(), m_iID(-1) {}

private:
	// register to environment
	void registerToEnvironment(Opt::Environment& cEnvironment_);
	// erase from environment
	void eraseFromEnvironment(Opt::Environment& cEnvironment_);

	int m_iID;
};

_SYDNEY_PLAN_FILE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_FILE_PARAMETER_H

//
//	Copyright (c) 2008, 2009, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
