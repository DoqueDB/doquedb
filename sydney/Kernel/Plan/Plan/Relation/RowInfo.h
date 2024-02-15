// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/RowInfo.h --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_RELATION_ROWINFO_H
#define __SYDNEY_PLAN_RELATION_ROWINFO_H

#include "Plan/Relation/Module.h"
#include "Plan/Relation/RowElement.h"
#include "Plan/Utility/ObjectSet.h"
#include "Plan/Declaration.h"

#include "Common/Object.h"
#include "Common/Data.h"

#include "Execution/Declaration.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

///////////////////////////////////////////////////////////////////////
//	CLASS
//	Plan::Relation::RowInfo -- Represents specification of relation row
//
//	NOTES
class RowInfo
	: public VECTOR< PAIR<STRING, RowElement*> >
{
public:
	typedef RowInfo This;

	typedef RowElement::Position Position;
	typedef Position Size;
	typedef PAIR<STRING, RowElement*> Element;
	typedef VECTOR<Element> Vector;
	typedef Vector Super;
	typedef Vector::ITERATOR Iterator;
	typedef Vector::CONSTITERATOR ConstIterator;

	// constructor
	static This* create(Opt::Environment& cEnvironment_);
	static This* create(Opt::Environment& cEnvironment_,
						const Vector& vecElement_);
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IRelation* pRelation_,
						Position iStart_,
						Position iEnd_);

	// destructor
	~RowInfo() {} // no subclasses

	// append contents
	void append(RowInfo* pRowInfo_);

	// get scalar name in this row spec
	const STRING& getScalarName(Opt::Environment& cEnvironment_,
								Position iPosition_);
	const STRING& getScalarName(Opt::Environment& cEnvironment_,
								Element& cElement_);
	// get scalar interface in this row spec
	Interface::IScalar* getScalar(Opt::Environment& cEnvironment_,
								  Position iPosition_);
	// mark scalar as retrieve
	void retrieve(Opt::Environment& cEnvironment_,
				  Position iPosition_);
	// mark all scalars as retrieve
	void retrieveAll(Opt::Environment& cEnvironment_);

	// get distinct key
	bool getDistinctKey(Opt::Environment& cEnvironment_,
						Utility::RowElementSet& cResult_);

	// create result set metadata
	Common::Data::Pointer createMetaData(Opt::Environment& cEnvironment_);

	// generate variable
	int generate(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Execution::Interface::IIterator* pIterator_,
				 Candidate::AdoptArgument& cArgument_);
	
	int generateFromType(Opt::Environment& cEnvironment_,
						 Execution::Interface::IProgram& cProgram_,
						 Execution::Interface::IIterator* pIterator_,
						 Candidate::AdoptArgument& cArgument_);

	// TEMPLATE FUNCTION public
	//	Plan::Relation::RowInfo::foreachEntry -- scan over elements
	//
	// TEMPLATE ARGUMENTS
	//	class Function_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	Function_ function_
	//	
	// RETURN
	//	Function_
	//
	// EXCEPTIONS

	template<class Function_>
	Function_
	foreachEntry(Function_ function_)
	{
		return Opt::ForEachMap(*this, function_);
	}

	// TEMPLATE FUNCTION public
	//	Plan::Relation::RowInfo::foreachElement -- scan over elements
	//
	// TEMPLATE ARGUMENTS
	//	class Function_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	Function_ function_
	//	
	// RETURN
	//	Function_
	//
	// EXCEPTIONS

	template<class Function_>
	Function_
	foreachElement(Function_ function_)
	{
		ITERATOR iterator = begin();
		const ITERATOR last = end();
		for (; iterator != last; ++iterator) {
			function_((*iterator).second);
		}
		return function_;
	}

	// TEMPLATE FUNCTION public
	//	Plan::Relation::RowInfo::mapElement -- 
	//
	// TEMPLATE ARGUMENTS
	//	class Container_
	//	class Function_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	Container_& cContainer_
	//	Function_ function_
	//	
	// RETURN
	//	Function_
	//
	// EXCEPTIONS

	template<class Container_, class Function_>
	Function_
	mapElement(Container_& cContainer_, Function_ function_)
	{
		ITERATOR iterator = begin();
		const ITERATOR last = end();
		for (; iterator != last; ++iterator) {
			cContainer_.pushBack(function_((*iterator).second));
		}
		return function_;
	}

	// TEMPLATE FUNCTION public
	//	Plan::Relation::RowInfo::find -- scan over elements
	//
	// TEMPLATE ARGUMENTS
	//	class Function_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	Function_ function_
	//	
	// RETURN
	//	ITERATOR
	//
	// EXCEPTIONS

	template<class Function_>
	ITERATOR
	find(Function_ function_)
	{
		return Opt::Find(begin(), end(), function_);
	}

protected:
private:
	// costructor
	RowInfo();
	explicit RowInfo(const Vector& vecElement_);

	// register to environment
	void registerToEnvironment(Opt::Environment& cEnvironment_);
};

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_RELATION_ROWINFO_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
