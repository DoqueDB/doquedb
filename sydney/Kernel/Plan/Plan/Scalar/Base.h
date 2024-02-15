// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Base.h --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2014, 2016, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_BASE_H
#define __SYDNEY_PLAN_SCALAR_BASE_H

#include "Plan/Interface/IScalar.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

///////////////////////////////////////////////////////////////////////////
//	CLASS
//	Plan::Scalar::Base -- base class for implementation classes of IScalar
//
//	NOTES
class Base
	: public Interface::IScalar
{
public:
	typedef Interface::IScalar Super;
	typedef Base This;

	// destructor
	virtual ~Base() {}

/////////////////////////////////////
// Interface::IScalar
	virtual void explain(Opt::Environment* pEnvironment_,
						 Opt::Explain& cExplain_);
	virtual Interface::IScalar* setExpectedType(Opt::Environment& cEnvironment_,
												const Scalar::DataType& cType_);
	virtual Interface::IScalar* createCast(Opt::Environment& cEnvironment_,
										   const DataType& cToType_,
										   bool bForComparison_,
										   Tree::Node::Type eType_ = Tree::Node::Undefined);
	virtual bool isKnownNull(Opt::Environment& cEnvironment_);
	virtual bool isKnownNotNull(Opt::Environment& cEnvironment_);
	virtual bool hasParameter();
	virtual bool isArbitraryElement();
	virtual bool isField();
	virtual Scalar::Field* getField();
	virtual bool hasField(Interface::IFile* pFile_);
	virtual bool isOperation();
	virtual bool isEquivalent(Interface::IScalar* pScalar_);
	virtual Interface::IScalar* addOption(Opt::Environment& cEnvironment_,
										  Interface::IScalar* pOption_);
	virtual Common::Data::Pointer preCalculate(Opt::Environment& cEnvironment_);

	virtual void retrieveFromCascade(Opt::Environment& cEnvironment_,
									 Sql::Query* pQuery_);


	virtual void use(Opt::Environment& cEnvironment_,
					 Interface::ICandidate* pCandidate_);

	virtual bool isSubquery() const;

	virtual bool equalsOperand(const Plan::Interface::IScalar* arg) const;
	

/////////////////////////////////////
// Node::

protected:
	// constructor
	explicit Base(Type eType_)
		: Super(eType_),
		  m_mapCast()
	{}
	Base(Type eType_, const DataType& cType_)
		: Super(eType_, cType_),
		  m_mapCast()
	{}

	bool isNeedCast(const DataType& cToType_,
					bool bForComparison_,
					Tree::Node::Type eType_ = Tree::Node::Undefined);
private:
	// cast node cache
	virtual Interface::IScalar* getCast(Opt::Environment& cEnvironment_,
										const DataType& cToType_,
										bool bForComparison_);
	virtual void setCast(Opt::Environment& cEnvironment_,
						 const DataType& cToType_,
						 bool bForComparison_,
						 Interface::IScalar* pResult_);

	typedef MAP< DataType, Interface::IScalar*, LESS<DataType> >CastMap;
	CastMap m_mapCast[2];
};

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_BASE_H

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2014, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
