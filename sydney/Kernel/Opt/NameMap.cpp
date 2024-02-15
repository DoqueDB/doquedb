// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Opt/NameMap.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Opt";
}

#include "SyDefault.h"

#include "Opt/NameMap.h"
#include "Opt/Environment.h"

#include "Common/Assert.h"

#include "Exception/DuplicateQualifiedName.h"

_SYDNEY_USING
_SYDNEY_OPT_USING

/////////////////////
namespace
{
	// FUNCTION local
	//	$$$::_equalString -- compare names
	//
	// NOTES
	//
	// ARGUMENTS
	//	const STRING& cstrName1_
	//	const STRING& cstrName2_
	//	
	// RETURN
	//	bool
	//
	// EXCEPTIONS

	bool _equalString(const STRING& cstrName1_, const STRING& cstrName2_)
	{
		return cstrName1_.compare(cstrName2_, ModFalse /* case insensitive */) == 0;
	}

} // namespace
/////////////////////

// FUNCTION public
//	NameMap::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Environment& cEnvironment_
//	
// RETURN
//	NameMap*
//
// EXCEPTIONS

//static
NameMap*
NameMap::
create(Environment& cEnvironment_)
{
	AUTOPOINTER<This> pResult = new This;
	cEnvironment_.addNameMap(pResult.get());
	return pResult.release();
}

// FUNCTION public
//	NameMap::add -- add new correspondence
//
// NOTES
//
// ARGUMENTS
//	const STRING& cstrName_
//	Plan::Interface::IRelation* pRelation_
//	
// RETURN
//	NameMap::NameScalarMap&
//
// EXCEPTIONS

NameMap::NameScalarMap&
NameMap::
add(const STRING& cstrName_, Plan::Interface::IRelation* pRelation_)
{
	if (cstrName_.getLength()) {
		if (m_cVector.GETSIZE() == 0) {
			// add new entry
			m_cVector.PUSHBACK(RelationEntry(cstrName_, pRelation_));

		} else {
			// check existing data
			NameRelationVector::Iterator iterator = m_cVector.begin();
			const NameRelationVector::Iterator last = m_cVector.end();
			do {
				if (_equalString(cstrName_, (*iterator).first)) {
					// same name entry already exists
					if (pRelation_ == (*iterator).second) break; // OK
					_SYDNEY_THROW1(Exception::DuplicateQualifiedName, cstrName_);
				}
				if (pRelation_ == (*iterator).second) {
					// replace name
					(*iterator).first = cstrName_;
					break;
				}
			} while (++iterator != last);

			if (iterator == last) {
				// add new entry
				m_cVector.PUSHBACK(RelationEntry(cstrName_, pRelation_));
			}
		}
	}
	Map::Iterator iterator = m_cMap.insert(pRelation_, NameScalarMap()).first;
										// existing entry is not overridden
	; _SYDNEY_ASSERT(iterator != m_cMap.end());

	return (*iterator).second;
}

// FUNCTION public
//	Opt::NameMap::addElement -- add new correspondence for scalar
//
// NOTES
//
// ARGUMENTS
//	NameScalarMap& cScalarMap_
//	const STRING& cstrName_
//	ScalarEntry cEntry_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
NameMap::
addElement(NameScalarMap& cScalarMap_,
		   const STRING& cstrName_,
		   ScalarEntry cEntry_)
{
	cScalarMap_.insert(cstrName_, cEntry_);
}

// FUNCTION public
//	Opt::NameMap::addPredicate -- add new predicate
//
// NOTES
//
// ARGUMENTS
//	const STRING& cstrStatement_
//	PredicateEntry cEntry_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
NameMap::
addPredicate(const STRING& cstrStatement_,
			 PredicateEntry cEntry_)
{
	m_cPredicateMap.insert(cstrStatement_, cEntry_);
}

// FUNCTION public
//	Opt::NameMap::merge -- merge namemap
//
// NOTES
//
// ARGUMENTS
//	const NameMap& cMap_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
NameMap::
merge(const NameMap& cMap_)
{
	m_cVector.insert(m_cVector.end(),
					 cMap_.m_cVector.begin(),
					 cMap_.m_cVector.end());
	m_cMap.insert(cMap_.m_cMap.begin(),
				  cMap_.m_cMap.end());

	// don't merge predicate map
}

// FUNCTION public
//	Opt::NameMap::get -- get relation by name
//
// NOTES
//
// ARGUMENTS
//	const STRING& cstrName_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

Plan::Interface::IRelation*
NameMap::
get(const STRING& cstrName_)
{
	NameRelationVector::Iterator iterator = m_cVector.begin();
	const NameRelationVector::Iterator last = m_cVector.end();
	for (; iterator != last; ++iterator) {
		if (_equalString(cstrName_, (*iterator).first)) {
			return (*iterator).second;
		}
	}
	return 0;
}

// FUNCTION public
//	Opt::NameMap::get -- get scalar by name
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IRelation* pRelation_
//	const STRING& cstrName_
//	
// RETURN
//	PAIR<Plan::Interface::IRelation*, Plan::Relation::RowElement*>
//
// EXCEPTIONS

PAIR<Plan::Interface::IRelation*, Plan::Relation::RowElement*>
NameMap::
get(Plan::Interface::IRelation* pRelation_, const STRING& cstrName_)
{
	Plan::Interface::IRelation* pRelation = 0;
	Plan::Relation::RowElement* pResult = 0;
	if (pRelation_) {
		Map::Iterator iterator = m_cMap.find(pRelation_);
		if (iterator != m_cMap.end()) {
			pResult = ((*iterator).second)[cstrName_];
			pRelation = pRelation_;
		}

	} else {
		Map::Iterator iterator = m_cMap.begin();
		const Map::Iterator last = m_cMap.end();
		for (; iterator != last; ++iterator) {
			NameScalarMap& cMap = (*iterator).second;
			NameScalarMap::Iterator found = cMap.find(cstrName_);
			if (found != cMap.end()) {
				if (pResult) {
					// same name in same scope
					_SYDNEY_THROW1(Exception::DuplicateQualifiedName, cstrName_);
				}
				pResult = (*found).second;
				pRelation = (*iterator).first;
				// continue to check duplicate name
			}
		}
	}
	return MAKEPAIR(pRelation, pResult);
}

// FUNCTION public
//	Opt::NameMap::getPredicate -- get predicate by name predicate
//
// NOTES
//
// ARGUMENTS
//	const STRING& cstrStatement_
//	
// RETURN
//	NameMap::PredicateEntry
//
// EXCEPTIONS

NameMap::PredicateEntry
NameMap::
getPredicate(const STRING& cstrStatement_)
{
	NamePredicateMap::Iterator found = m_cPredicateMap.find(cstrStatement_);
	if (found != m_cPredicateMap.end()) {
		return (*found).second;
	}
	return 0;
}

// FUNCTION public
//	Opt::NameMap::isHasMap -- check existence of map by relation
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IRelation* pRelation_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
NameMap::
isHasMap(Plan::Interface::IRelation* pRelation_)
{
	return m_cMap.find(pRelation_) != m_cMap.end();
}

// FUNCTION public
//	Opt::NameMap::getMap -- get map by relation
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IRelation* pRelation_
//	
// RETURN
//	NameMap::NameScalarMap&
//
// EXCEPTIONS

NameMap::NameScalarMap&
NameMap::
getMap(Plan::Interface::IRelation* pRelation_)
{
	return m_cMap[pRelation_];
}

// FUNCTION public
//	Opt::NameMap::getCorrelationName -- get name by relation
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IRelation* pRelation_
//	
// RETURN
//	STRING*
//
// EXCEPTIONS

STRING*
NameMap::
getCorrelationName(Plan::Interface::IRelation* pRelation_)
{
	// check existing data
	NameRelationVector::Iterator iterator = m_cVector.begin();
	const NameRelationVector::Iterator last = m_cVector.end();
	for (; iterator != last; ++iterator) {
		if (pRelation_ == (*iterator).second) {
			return &((*iterator).first);
		}
	}
	return 0;
}

// FUNCTION private
//	Opt::NameMap::NameMap -- costructor
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

NameMap::
NameMap()
	: m_cVector(), m_cMap()
{}

//
// Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
