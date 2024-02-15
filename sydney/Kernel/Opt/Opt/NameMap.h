// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Opt/NameMap.h --
// 
// Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_OPT_NAMEMAP_H
#define __SYDNEY_OPT_NAMEMAP_H

#include "Opt/Module.h"
#include "Opt/Declaration.h"
#include "Opt/Environment.h"

#include "Plan/Relation/Declaration.h"
#include "Plan/Scalar/Declaration.h"

#include "Common/Object.h"

_SYDNEY_BEGIN
_SYDNEY_OPT_BEGIN

////////////////////////////////////
//	CLASS
//	Opt::NameMap -- Represents correspondence between name and scalar info
//
//	NOTES
class NameMap
	: public Common::Object
{
public:
	typedef NameMap This;
	typedef Common::Object Super;

	typedef PAIR<STRING, Plan::Interface::IRelation*> RelationEntry;
	typedef Plan::Relation::RowElement* ScalarEntry;
	typedef Plan::Interface::IPredicate* PredicateEntry;
	typedef VECTOR<RelationEntry> NameRelationVector;
	typedef MAP<STRING, ScalarEntry, CaseInsensitiveComparator> NameScalarMap;
	typedef MAP<STRING, PredicateEntry, CaseInsensitiveComparator> NamePredicateMap;
	typedef MAP<Plan::Interface::IRelation*, NameScalarMap, LESS<Plan::Interface::IRelation*> > Map;

	// constructor
	static This* create(Environment& cEnvironment_);

	// destructor
	~NameMap() {} // no subclasses

	// add new correspondence
	NameScalarMap& add(const STRING& cstrName_,
					   Plan::Interface::IRelation* pRelation_);

	// add new correspondence for scalar
	void addElement(NameScalarMap& cScalarMap_,
					const STRING& cstrName_,
					ScalarEntry cEntry_);

	// add new predicate
	void addPredicate(const STRING& cstrStatement_,
					  PredicateEntry cEntry_);

	// merge namemap
	void merge(const NameMap& cMap_);

	// get relation by name
	Plan::Interface::IRelation* get(const STRING& cstrName_);
	// get scalar by name
	PAIR<Plan::Interface::IRelation*, Plan::Relation::RowElement*>
					get(Plan::Interface::IRelation* pRelation_,
						const STRING& cstrName_);
	// get predicate by name predicate
	PredicateEntry getPredicate(const STRING& cstrStatement_);

	// check existence of map by relation
	bool isHasMap(Plan::Interface::IRelation* pRelation_);
	// get map by relation
	NameScalarMap& getMap(Plan::Interface::IRelation* pRelation_);
	// get name by relation
	STRING* getCorrelationName(Plan::Interface::IRelation* pRelation_);

protected:
private:
	// costructor
	NameMap();

	NameRelationVector m_cVector;
	Map m_cMap;

	NamePredicateMap m_cPredicateMap;
};

_SYDNEY_OPT_END
_SYDNEY_END

#endif // __SYDNEY_OPT_NAMEMAP_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
