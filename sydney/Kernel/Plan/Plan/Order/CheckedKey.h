// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Order/CheckedKey.h --
// 
// Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_ORDER_CHECKEDKEY_H
#define __SYDNEY_PLAN_ORDER_CHECKEDKEY_H

#include "Plan/Order/Key.h"
#include "Plan/Utility/ObjectSet.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_ORDER_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Order::CheckedKey -- Wrapper key for representing index of orders
//
//	NOTES
//		This class is not constructed directly
class CheckedKey
	: public Key
{
public:
	typedef Key Super;
	typedef CheckedKey This;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Key* pKey_,
						const Utility::FileSet& cFile_);

	// destructor
	virtual ~CheckedKey() {}

////////////////////////
// Key::
	virtual void explain(Opt::Environment* pEnvironment_,
						 Opt::Explain& cExplain_)
	{m_pKey->explain(pEnvironment_, cExplain_);}
	virtual void require(Opt::Environment& cEnvironment_,
						 Interface::ICandidate* pCandidate_)
	{m_pKey->require(cEnvironment_, pCandidate_);}
	virtual Key* check(Opt::Environment& cEnvironment_,
					   const CheckArgument& cArgument_)
	{return this;}
	virtual bool isChecked()
	{return true;}
	virtual CheckedKey* getChecked()
	{return this;}

	virtual Interface::IScalar* getScalar()
	{return m_pKey->getScalar();}
	virtual Direction::Value getDirection()
	{return m_pKey->getDirection();}

	virtual bool isFunction()
	{return m_pKey->isFunction();}
	
	virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
								  const Plan::Sql::QueryArgument& cArgument_) const
	{return m_pKey->toSQLStatement(cEnvironment_, cArgument_);}

////////////////////////
// Interface::
	virtual ModUnicodeString getValue() const
	{return m_pKey->getValue();}
	virtual const Common::Data* getData() const
	{return m_pKey->getData();}

	virtual ModSize getOptionSize() const
	{return m_pKey->getOptionSize();}
	virtual const Tree::Node::Super* getOptionAt(ModInt32 iPosition_) const
	{return m_pKey->getOptionAt(iPosition_);}

	virtual ModSize getOperandSize() const
	{return m_pKey->getOperandSize();}
	virtual const Tree::Node::Super* getOperandAt(ModInt32 iPosition_) const
	{return m_pKey->getOperandAt(iPosition_);}

//////////////////
// accessors
	Key* getKey() {return m_pKey;}
	const Utility::FileSet& getFile() {return m_cFile;}

protected:
	// constructor
	CheckedKey(Key* pKey_,
			   const Utility::FileSet& cFile_);
private:
	Key* m_pKey;
	Utility::FileSet m_cFile;
};

_SYDNEY_PLAN_ORDER_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_ORDER_CHECKEDKEY_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
