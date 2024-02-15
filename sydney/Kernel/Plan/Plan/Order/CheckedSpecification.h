// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Order/CheckedSpecification.h --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_ORDER_CHECKEDSPECIFICATION_H
#define __SYDNEY_PLAN_ORDER_CHECKEDSPECIFICATION_H

#include "Plan/Order/SpecificationWrapper.h"
#include "Plan/Interface/IFile.h"
#include "Plan/Utility/ObjectSet.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_ORDER_BEGIN

///////////////////////////////////////////////////////////
//	CLASS
//	Plan::Order::CheckedSpecification -- Sort checkedSpecification
//
//	NOTES
class CheckedSpecification
	: public SpecificationWrapper
{
public:
	typedef SpecificationWrapper Super;
	typedef CheckedSpecification This;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Specification* pSpecification_,
						Order::Key* pKey_,
						const Utility::FileSet& cFile_);

	static This* create(Opt::Environment& cEnvironment_,
						Specification* pSpecification_,
						const VECTOR<Order::Key*>& vecKey_,
						const Utility::FileSet& cFile_);

	// destructor
	virtual ~CheckedSpecification() {}

	// is partial sort?
	bool isPartial()
	{return getKeySize() < getSpecification()->getKeySize();}

	// create argument for checking file
	virtual bool createCheckOrderArgument(Opt::Environment& cEnvironment_,
										  Interface::IFile* pFile_,
										  File::CheckOrderArgument& cArgument_) = 0;

	// choose index
	virtual Specification* choose(Opt::Environment& cEnvironment_,
								  const Order::ChooseArgument& cArgument_) = 0;

	// check retrievable for a predicate
	virtual bool isKeyRetrievable(Opt::Environment& cEnvironment_,
								  Interface::IPredicate* pPredicate_) = 0;

//////////////////////////////////
// Specification::
//	virtual void explain(Opt::Environment* pEnvironment_,
//						 Opt::Explain& cExplain_);
//	virtual Size getKeySize();
//	virtual Order::Key* getKey(Position iPos_);
	virtual void setPartitionKey(const VECTOR<Interface::IScalar*>& vecPartitionKey_)
	{; /* do nothing */}
	virtual const VECTOR<Interface::IScalar*>& getPartitionKey()
	{return getSpecification()->getPartitionKey();}
//	virtual bool isRefering(Interface::IRelation* pRelation_,
//							Position iPosition_);
//	virtual void require(Opt::Environment& cEnvironment_,
//						 Interface::ICandidate* pCandidate_);
//	virtual GeneratedSpecification* generate(Opt::Environment& cEnvironment_,
//											 Execution::Interface::IProgram& cProgram_,
//											 Execution::Interface::IIterator* pIterator_,
//											 Candidate::AdoptArgument& cArgument_,
//											 Candidate::Row* pRow_,
//											 const Candidate::RowDelayArgument& cDelayArgument_);
	virtual Specification* check(Opt::Environment& cEnvironment_,
								 const CheckArgument& cArgument_)
	{return this;}

	// is index is cheked
	virtual bool isChecked()
	{return true;}

	// get checked interface
	virtual CheckedSpecification* getChecked()
	{return this;}
 
	//accessors
	const Utility::FileSet& getFile()
	{return m_cFile;}

protected:
	// constructor
	CheckedSpecification(Specification* pSpecification_,
						 const Utility::FileSet& cFile_)
		: Super(pSpecification_),
		  m_cFile(cFile_),
		  m_bGrouping(false)
	{}
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
	CheckedSpecification()
		: Super()
	{}
	void setArgument(Specification* pSpecification_,
					 const Utility::FileSet& cFile_)
	{
		setSpecification(pSpecification_);
		m_cFile = cFile_;
	}
#endif

private:
	Utility::FileSet m_cFile;
	bool m_bGrouping;
};

_SYDNEY_PLAN_ORDER_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_ORDER_CHECKEDSPECIFICATION_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
