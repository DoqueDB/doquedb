// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/CombinatorImpl.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2018, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Predicate::Impl";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Predicate/Impl/CombinatorImpl.h"
#include "Plan/Predicate/Fetch.h"
#include "Plan/Candidate/Table.h"
#include "Plan/Interface/IRelation.h"
#include "Plan/Relation/Selection.h"
#include "Plan/Relation/Table.h"
#include "Plan/Utility/Trace.h"

#include "Exception/NotSupported.h"

#include "Opt/Configuration.h"

#include "Os/Limits.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace
{
	AccessPlan::Cost::Value _maxCost(const AccessPlan::Cost::Value& cCost0_,
									 const AccessPlan::Cost::Value& cCost1_)
	{
		if (cCost0_.isInfinity()) return cCost1_;
		if (cCost1_.isInfinity()) return cCost0_;
		return MAX(cCost0_, cCost1_);
	}
}

////////////////////////////////////////
//Plan::Predicate::Impl::AndRewriter::

// FUNCTION public
//	Predicate::Impl::AndRewriter::operator -- 
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
Impl::AndRewriter::
operator()(Interface::IPredicate* p_)
{
	PAIR<Interface::IRelation*, Interface::IPredicate*> cResult =
		(*m_ppRelation)->rewrite(m_cEnvironment,
								 p_,
								 m_cArgument);

	*m_ppRelation = cResult.first;
	if (cResult.second) {
		m_pvecPredicate->PUSHBACK(cResult.second);
	}
}

////////////////////////////////////////////////////////
//	Plan::Predicate::Impl::AndRewriteEstimator::

// FUNCTION public
//	Predicate::Impl::AndRewriteEstimator::operator -- 
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
Impl::AndRewriteEstimator::
operator()(Interface::IPredicate* p_)
{
	if (m_iVal > 0) {
		int iValue = p_->estimateRewrite(m_cEnvironment);
		if (iValue > 0 && m_iVal < Os::Limits<int>::getMax() / iValue) {
			m_iVal *= iValue;
		} else {
			m_iVal = 0;
		}
	}
}

////////////////////////////////////////
//Plan::Predicate::Impl::OrRewriter::

// FUNCTION public
//	Predicate::Impl::OrRewriter::operator -- 
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
Impl::OrRewriter::
operator()(Interface::IPredicate* p_)
{
	if (m_cArgument.m_bCheckUnion == false) {
		m_cArgument.m_bNoRelationChange = true;
	}
	Interface::IPredicate::RewriteResult cResult =
		p_->rewrite(m_cEnvironment,
					m_pRelation,
					m_cArgument);

	SIZE iPosition;
	Utility::FieldSet cFieldSet;
	Utility::RelationSet cTableSet;

	if (cResult.getPredicate()) {
		cResult.getPredicate()->getUsedTable(cTableSet);
		cResult.getPredicate()->getUsedField(cFieldSet);
	}

	if (cTableSet.getSize() > 1) {
		// if predicate is join predicate, each element should be divided into union operand
		(*m_pvecResult).PUSHBACK(Element(cResult.getRelation(),
										 MAKEPAIR(PredicateMap(),
												  FieldSetVector())));
		iPosition = (*m_pvecResult).GETSIZE() - 1;

	} else if (cResult.getRelation() == m_pRelation
			   && m_pRelation->getType() != Tree::Node::Table) {
		; _SYDNEY_ASSERT(cResult.getPredicate());

		// gather predicates refering same one table
		UseTableMap::Iterator found = m_mapUseTable.find(cTableSet);
		if (found != m_mapUseTable.end()) {
			// use entry
			iPosition = (*found).second;
		} else {
			// create new entry
			(*m_pvecResult).PUSHBACK(Element(m_pRelation,
											 MAKEPAIR(PredicateMap(),
													  FieldSetVector())));
			iPosition = (*m_pvecResult).GETSIZE() - 1;
			m_mapUseTable[cTableSet] = iPosition;
		}

	} else {
		// map is refered even if predicate == 0 to create map entry
		Map::Iterator found = m_mapRewrite.find(cResult.getRelation());
		if (found != m_mapRewrite.end()) {
			// use entry
			iPosition = (*found).second;
		} else {
			// create new entry
			(*m_pvecResult).PUSHBACK(Element(cResult.getRelation(),
											 MAKEPAIR(PredicateMap(),
													  FieldSetVector())));
			iPosition = (*m_pvecResult).GETSIZE() - 1;
			m_mapRewrite[cResult.getRelation()] = iPosition;
		}
	}
	if (cResult.getPredicate() != 0) {
		PAIR<PredicateMap, FieldSetVector>& cMapPair =
			(*m_pvecResult)[iPosition].second;
		PredicateMap::ITERATOR found = cMapPair.first.find(cFieldSet);
		if (found != cMapPair.first.end()) {
			// already registered
			(*found).second.PUSHBACK(cResult.getPredicate());
		} else {
			// new fieldset
			cMapPair.first[cFieldSet].PUSHBACK(cResult.getPredicate());
			// record fieldset in order of appearance
			cMapPair.second.PUSHBACK(cFieldSet);
		}
	}
}

// FUNCTION public
//	Predicate::Impl::OrRewriteEstimator::operator -- 
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
Impl::OrRewriteEstimator::
operator()(Interface::IPredicate* p_)
{
	if (m_iVal > 0) {
		int iValue = p_->estimateRewrite(m_cEnvironment);
		if (iValue > 0) {
			if (p_->hasSubquery()) {
				// sub query are counted one by one
			} else {
				Utility::RelationSet cTableSet;
				p_->getUsedTable(cTableSet);
				if (cTableSet.getSize() != 1) {
					// join predicates are counted one by one
				} else {
					; _SYDNEY_ASSERT(cTableSet.getSize() == 1);
					m_cSingle.add(*cTableSet.begin());
					iValue = 0; // don't add here
				}
			}
			if (m_iVal < Os::Limits<int>::getMax() - iValue) {
				m_iVal += iValue;
			} else {
				m_iVal = 0;
			}
		} else {
			m_iVal = 0;
		}
	}
}

// FUNCTION public
//	Predicate::Impl::OrRewriteEstimator::calculate -- 
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
Impl::OrRewriteEstimator::
calculate()
{
	if (m_iVal > 0 && m_iVal < Os::Limits<int>::getMax() - static_cast<int>(m_cSingle.getSize())) {
		m_iVal += m_cSingle.getSize();
	}
}

////////////////////////////////////////
//Plan::Predicate::Impl::OrMerger::

// FUNCTION public
//	Predicate::Impl::OrMerger::operator() -- 
//
// NOTES
//
// ARGUMENTS
//	OrRewriter::Element& cElement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::OrMerger::
operator()(OrRewriter::Element& cElement_)
{
	Interface::IRelation* pResult = cElement_.first;
	if (cElement_.second.second.ISEMPTY() == false) {
		VECTOR<Interface::IPredicate*> vecPredicate;
		Opt::ForEach(cElement_.second.second,
					 PredicateMerger(m_cEnvironment,
									 cElement_.second.first,
									 &vecPredicate));
		Interface::IPredicate* pPredicate =
			Combinator::create(m_cEnvironment,
							   Tree::Node::Or,
							   vecPredicate);
		pResult = Relation::Selection::create(m_cEnvironment,
											  pPredicate,
											  pResult);
#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Process)) {
			_OPT_OPTIMIZATION_MESSAGE << "OrMerger: create selection for "
									  << Utility::Trace::toString(m_cEnvironment, pPredicate)
									  << ModEndl;
		}
#endif
	}
	m_pvecResult->PUSHBACK(pResult);
}

////////////////////////////////////////
//Plan::Predicate::Impl::PredicateMerger::

// FUNCTION public
//	Predicate::Impl::PredicateMerger::operator() -- 
//
// NOTES
//
// ARGUMENTS
//	const Utility::FieldSet& cFieldSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::PredicateMerger::
operator()(const Utility::FieldSet& cFieldSet_)
{
	const VECTOR<Interface::IPredicate*>& vecPredicate = m_mapPredicate[cFieldSet_];
	if (vecPredicate.ISEMPTY() == false) {
		Interface::IPredicate* pPredicate =
			Combinator::create(m_cEnvironment,
							   Tree::Node::Or,
							   vecPredicate);
		m_pvecResult->PUSHBACK(pPredicate);
	}
}

////////////////////////////////////////
//Plan::Predicate::Impl::IndexMap::

// FUNCTION public
//	Predicate::Impl::IndexMap::add -- add file->predicate relationship
//
// NOTES
//
// ARGUMENTS
//	Interface::IFile* pFile_
//	Interface::IPredicate* pPredicate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::IndexMap::
add(Interface::IFile* pFile_,
	Interface::IPredicate* pPredicate_)
{
	if (pPredicate_->isFetch()) {
		// fetch predicate is added to head
		(*this)[pFile_].PUSHFRONT(pPredicate_);
	} else {
		(*this)[pFile_].PUSHBACK(pPredicate_);
	}
}

////////////////////////////////////
// Plan::Predicate::Impl::IndexChecker

// FUNCTION public
//	Plan::Predicate::Impl::IndexChecker::operator -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IPredicate* p_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::IndexChecker::
operator()(Interface::IPredicate* p_)
{
	if (!p_->isChecked()) {
		Interface::IPredicate* pResult = p_->check(m_cEnvironment,
												   m_cArgument);
		if (pResult->isChecked()) {
			CheckedInterface* pChecked = pResult->getChecked();
			; _SYDNEY_ASSERT(pChecked);
			if (pChecked->isFetch()) {
				m_pvecChecked->PUSHFRONT(pChecked);
			} else {
				m_pvecChecked->PUSHBACK(pChecked);
			}
			if (Interface::IPredicate* pNotChecked = pResult->getNotChecked()) {
				// partly, there exist not checked predicates
				m_pvecNotChecked->PUSHBACK(pNotChecked);
			}
			if (m_ppCandidate) {
				if (*m_ppCandidate == 0) {
					*m_ppCandidate = pChecked->getTable();
				} else if (*m_ppCandidate != pChecked->getTable()) {
					// no common table
					*m_ppCandidate = 0;
					m_ppCandidate = 0; // no more check
				}
			}
			if (m_pFileSet) {
				// take intersection of using index files
				if (m_pFileSet->isEmpty()) *m_pFileSet = pChecked->getFile();
				else m_pFileSet->intersect(pChecked->getFile());
				if (m_pFileSet->isEmpty()) m_pFileSet = 0; // no more check
			}
			if (m_pIndexMap) {
				// add to file->predicate map here
				pChecked->getFile().foreachElement(
									   boost::bind(&IndexMap::add,
												   m_pIndexMap,
												   _1,
												   pChecked));
			}
		} else {
			m_pvecNotChecked->PUSHBACK(pResult);
		}
	} else {
		// if p_ is already checked, that means p_ is processed by other tables
		if (m_pFileSet) {
			m_pFileSet->removeAll();
			m_pFileSet = 0;
		}
		m_pvecChecked->PUSHBACK(p_);
	}
}

//////////////////////////////////////////////////
//	Plan::Predicate::Impl::IndexRecomposer

// FUNCTION public
//	Plan::Predicate::Impl::IndexRecomposer::operator -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IPredicate* p_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::IndexRecomposer::
operator()(Interface::IPredicate* p_)
{
	// if p_ is already processed by file with other predicates, skip
	if (m_cDetermined.isContaining(p_)) return;

	; _SYDNEY_ASSERT(p_->isChecked());

	CheckedInterface* pChecked = p_->getChecked();
	; _SYDNEY_ASSERT(pChecked);

	const Utility::FileSet& cFileSet = pChecked->getFile();

	Utility::FileSet::ConstIterator iterator = cFileSet.begin();
	const Utility::FileSet::ConstIterator last = cFileSet.end();
	int iMax = 0;
	Utility::FileSet::ConstIterator found = last;

	for (; iterator != last; ++iterator) {
		// get one entry from file->predicates map
		IndexMap::ConstIterator mapfound = m_cIndexMap.find(*iterator);
		if (mapfound != m_cIndexMap.end()) {
			const VECTOR<Interface::IPredicate*>& vecChecked = (*mapfound).second;

			if (iMax < static_cast<int>(vecChecked.GETSIZE())) {
				// record the index file which can process p_
				// with most number of predicates
				iMax = vecChecked.GETSIZE();
				found = iterator;
			}
		}
	}
	if (iMax <= 1) {
		// this predicate is processed alone
		if (p_->isFetch()) {
			(*m_pvecChecked).PUSHFRONT(p_);
		} else {
			(*m_pvecChecked).PUSHBACK(p_);
		}
		m_cDetermined.add(p_);

	} else {
		; _SYDNEY_ASSERT(iMax > 1);
		; _SYDNEY_ASSERT(found != last);

		// any index file should been found

		Interface::IFile* pFoundFile = *found;

		const VECTOR<Interface::IPredicate*>& vecChecked = m_cIndexMap[pFoundFile];
		; _SYDNEY_ASSERT(vecChecked.GETSIZE() > 1);
		; _SYDNEY_ASSERT(vecChecked[0]->isChecked());

		Utility::FileSet cTmpFileSet;
		cTmpFileSet.add(pFoundFile);
		Candidate::Table* pTable = vecChecked[0]->getChecked()->getTable();
		Interface::IPredicate* pElement = 0;

		if (vecChecked[0]->isFetch()) {
			// divide fetch and other predicates
			VECTOR<Interface::IPredicate*>::CONSTITERATOR split =
				Opt::FindLast(vecChecked.begin(),
							  vecChecked.end(),
							  boost::bind(&Interface::IPredicate::isFetch,
										  _1));
			; _SYDNEY_ASSERT(split != vecChecked.end());
			++split;

			// fetch and other predicates
			VECTOR<Interface::IPredicate*> vecFetch;
			VECTOR<Interface::IPredicate*> vecOther;
			Utility::ScalarSet cFetchKey;

			// set fetch part and other part
			if (split - vecChecked.begin() == 1) {
				vecFetch.assign(vecChecked.begin(), split);
			} else {
				vecFetch.reserve(split - vecChecked.begin());
				VECTOR<Interface::IPredicate*>::CONSTITERATOR iterator = vecChecked.begin();
				const VECTOR<Interface::IPredicate*>::CONSTITERATOR last = split;
				for (; iterator != last; ++iterator) {
					if ((*iterator)->getFetchKey(m_cEnvironment,
												 cFetchKey)) {
						vecFetch.PUSHBACK(*iterator);
					} else {
						vecOther.PUSHBACK(*iterator);
					}
				}
			}
			if (vecFetch.ISEMPTY() == false
				&& split != vecChecked.end()) {
				// add normal predicates for fetching if can
				VECTOR<Interface::IPredicate*>::CONSTITERATOR iterator = split;
				const VECTOR<Interface::IPredicate*>::CONSTITERATOR last = vecChecked.end();
				for (; iterator != last; ++iterator) {
					Interface::IPredicate* pFetchForm = (*iterator)->createFetch(m_cEnvironment,
																				 cFetchKey);
					if (pFetchForm) {
						vecFetch.PUSHBACK(pFetchForm);
					} else {
						vecOther.PUSHBACK(*iterator);
					}
				}
			}

			bool bMergeFetch = (m_eType == Tree::Node::And);

			if (vecOther.ISEMPTY() == false) {
				VECTOR<Interface::IPredicate*> vecResult;
				// create fetch part
				if (vecFetch.ISEMPTY() == false) {
					vecResult.PUSHBACK(createChecked(vecFetch,
													 pTable,
													 cTmpFileSet,
													 bMergeFetch));
				}
				// create other part
				vecResult.PUSHBACK(createChecked(vecOther,
												 pTable,
												 cTmpFileSet));
				// combine it
				pElement = createChecked(vecResult,
										 pTable,
										 cTmpFileSet,
										 false); // no need to merge any more
			} else {
				// only fetch part
				pElement = createChecked(vecFetch,
										 pTable,
										 cTmpFileSet,
										 bMergeFetch);
			}
		} else {
			// create checkedinterface with the set of predicates
			// which has been selected by above loop
			pElement = createChecked(vecChecked,
									 pTable,
									 cTmpFileSet);
		}

		if (vecChecked[0]->isFetch()) {
			(*m_pvecChecked).PUSHFRONT(pElement);
		} else {
			(*m_pvecChecked).PUSHBACK(pElement);
		}
		// set all the predicate using the index file as determined
		m_cDetermined.add(vecChecked.begin(),
						  vecChecked.end());
		// use found file
		m_pFileSet->intersect(cTmpFileSet);
	}
}

// FUNCTION private
//	Predicate::Impl::IndexRecomposer::createChecked -- 
//
// NOTES
//
// ARGUMENTS
//	const VECTOR<Interface::IPredicate*>& vecChecked_
//	Candidate::Table* pTable_
//	const Utility::FileSet& cFileSet_
//	bool bMergeFetch_ = false
//	
// RETURN
//	CheckedInterface*
//
// EXCEPTIONS

CheckedInterface*
Impl::IndexRecomposer::
createChecked(const VECTOR<Interface::IPredicate*>& vecChecked_,
			  Candidate::Table* pTable_,
			  const Utility::FileSet& cFileSet_,
			  bool bMergeFetch_ /* = false */)
{
	Interface::IPredicate* pPredicate = Combinator::create(m_cEnvironment,
														   m_eType,
														   vecChecked_);
	if (bMergeFetch_ && vecChecked_.GETSIZE() > 1 && cFileSet_.isEmpty() == false) {
		// merge fetch
		Relation::Table::AutoReset cAutoReset =
			pTable_->getTable()->setEstimateFile(*cFileSet_.begin());

		Interface::IPredicate* pFetch = Fetch::merge(m_cEnvironment,
													 pPredicate,
													 vecChecked_);
		if (pFetch) pPredicate = pFetch;
	}
	return CheckedInterface::create(m_cEnvironment,
									pPredicate,
									vecChecked_,
									pTable_,
									cFileSet_,
									m_bNoTop);
}

//////////////////////////////////////////////////
//	Plan::Predicate::Impl::AndUnknownChecker

// FUNCTION public
//	Plan::Predicate::Impl::AndUnknownChecker::operator -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IPredicate* p_
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Impl::AndUnknownChecker::
operator()(Interface::IPredicate* p_)
{
	Predicate::CheckUnknownArgument cArgument;
	if (p_->getCheckUnknown(m_cEnvironment, cArgument)) {
		if (m_bFirst) {
			m_cKey = cArgument.m_cKey;
			m_bFirst = false;
		} else {
			return (cArgument.m_cKey == m_cKey);
		}
	}
	return false;
}

//////////////////////////////////////////////////
//	Plan::Predicate::Impl::OrUnknownChecker

// FUNCTION public
//	Plan::Predicate::Impl::OrUnknownChecker::operator -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IPredicate* p_
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Impl::OrUnknownChecker::
operator()(Interface::IPredicate* p_)
{
	Predicate::CheckUnknownArgument cArgument;
	if (p_->getCheckUnknown(m_cEnvironment, cArgument)) {
		m_cKey.merge(cArgument.m_cKey);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////
//	Plan::Predicate::Impl::AndCostEstimator

// FUNCTION public
//	Predicate::Impl::AndCostEstimator::operator -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IPredicate* p_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::AndCostEstimator::
operator()(Interface::IPredicate* p_)
{
	AccessPlan::Cost cMyResult;
	cMyResult.setTupleCount(m_cResult.getTupleCount());
	cMyResult.setTableCount(m_cResult.getTableCount());
	cMyResult.setIsSetCount(m_cResult.isSetCount());
	cMyResult.setIsSetRate(m_cResult.isSetRate());

	if (p_->estimateCost(m_cEnvironment,
						 cMyResult)) {
		if (m_bResult) {
			AccessPlan::Cost::Value cOverhead = m_cTemp.getOverhead() + cMyResult.getOverhead();
			AccessPlan::Cost::Value cTotalCost = m_cTemp.getTotalCost() + cMyResult.getTotalCost();
			AccessPlan::Cost::Value cTupleCount = MIN(m_cTemp.getTupleCount(), cMyResult.getTupleCount());
			AccessPlan::Cost::Value cRetrieveCost = m_cTemp.getRetrieveCost() + cMyResult.getRetrieveCost();
			AccessPlan::Cost::Value cRate = MIN(m_cTemp.getRate(), cMyResult.getRate());

			m_cTemp.setOverhead(cOverhead);
			m_cTemp.setTotalCost(cTotalCost);
			m_cTemp.setTupleCount(cTupleCount);
			m_cTemp.setTableCount(cMyResult.getTableCount());
			m_cTemp.setRetrieveCost(cRetrieveCost);
			m_cTemp.setRate(cRate);
		} else {
			m_cTemp = cMyResult;
			m_bResult = true;
		}
	}
}

// FUNCTION public
//	Predicate::Impl::AndCostEstimator::getResult -- 
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
Impl::AndCostEstimator::
getResult()
{
	if (m_bResult) {
		m_cResult = m_cTemp;
	}
	return m_bResult;
}

//////////////////////////////////////////////////
//	Plan::Predicate::Impl::OrCostEstimator

// FUNCTION public
//	Predicate::Impl::OrCostEstimator::operator -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IPredicate* p_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::OrCostEstimator::
operator()(Interface::IPredicate* p_)
{
	AccessPlan::Cost cMyResult;
	cMyResult.setTupleCount(m_cResult.getTupleCount());
	cMyResult.setTableCount(m_cResult.getTableCount());
	cMyResult.setIsSetCount(m_cResult.isSetCount());
	cMyResult.setIsSetRate(m_cResult.isSetRate());

	if (p_->estimateCost(m_cEnvironment,
						 cMyResult)) {
		if (m_bResult) {
			AccessPlan::Cost::Value cOverhead = m_cTemp.getOverhead() + cMyResult.getOverhead();
			AccessPlan::Cost::Value cTotalCost = m_cTemp.getTotalCost() + cMyResult.getTotalCost();
			AccessPlan::Cost::Value cTupleCount = _maxCost(m_cTemp.getTupleCount(), cMyResult.getTupleCount());
			AccessPlan::Cost::Value cRetrieveCost = m_cTemp.getRetrieveCost() + cMyResult.getRetrieveCost();
			AccessPlan::Cost::Value cRate = _maxCost(m_cTemp.getRate(), cMyResult.getRate());

			m_cTemp.setOverhead(cOverhead);
			m_cTemp.setTotalCost(cTotalCost);
			m_cTemp.setTupleCount(cTupleCount);
			m_cTemp.setTableCount(cMyResult.getTableCount());
			m_cTemp.setRetrieveCost(cRetrieveCost);
			m_cTemp.setRate(cRate);
		} else {
			m_cTemp = cMyResult;
			m_bResult = true;
		}
	}
}

// FUNCTION public
//	Predicate::Impl::OrCostEstimator::getResult -- 
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
Impl::OrCostEstimator::
getResult()
{
	if (m_bResult) {
		m_cResult = m_cTemp;
	}
	return m_bResult;
}

////////////////////////////////////
//	Predicate::Impl::NotImpl

// FUNCTION public
//	Predicate::Impl::NotImpl::rewrite -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IRelation* pRelation_
//	Predicate::RewriteArgument& cArgument_
//	
// RETURN
//	Interface::IPredicate::RewriteResult
//
// EXCEPTIONS

//virtual
Interface::IPredicate::RewriteResult
Impl::NotImpl::
rewrite(Opt::Environment& cEnvironment_,
		Interface::IRelation* pRelation_,
		Predicate::RewriteArgument& cArgument_)
{
	Interface::IPredicate::RewriteResult cOperandResult =
		getOperand()->rewrite(cEnvironment_,
							  pRelation_,
							  cArgument_);
	if (cOperandResult.getRelation() == pRelation_
		&& cOperandResult.getPredicate() != getOperand()) {
		if (cOperandResult.getPredicate()) {
			return RewriteResult(pRelation_,
								 Combinator::create(cEnvironment_,
													Tree::Node::Not,
													cOperandResult.getPredicate()));
		} else {
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
	return Super::rewrite(cEnvironment_,
						  pRelation_,
						  cArgument_);
}

// FUNCTION public
//	Predicate::Impl::NotImpl::check -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
Impl::NotImpl::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	Interface::IPredicate* pOperand = getOperand();
	; _SYDNEY_ASSERT(!pOperand->isChecked());

	CheckArgument cOperandArgument(cArgument_);
	cOperandArgument.m_bNoTop = true;

	Interface::IPredicate* pCheckedOperand = pOperand->check(cEnvironment_,
															 cOperandArgument);
	Interface::IPredicate* pResult = Combinator::create(cEnvironment_,
														getType(),
														pCheckedOperand);
	if (pCheckedOperand->isChecked()) {
		// create (NOT p)
		if (pCheckedOperand->getChecked()->isUseIndex() == false) {
			// no index can be used for operand
			pResult = 
				CheckedInterface::create(cEnvironment_,
										 pResult);
		} else {
			if (pCheckedOperand->getNotChecked()) {
				// operand can use index but partially not checked
				// -> treat whole operand as not checked
				;

			} else {
				// operand can use index
				pResult =
					CheckedInterface::create(cEnvironment_,
											 pResult,
											 pCheckedOperand);
			}
		}
	}
	return pResult;
}

// FUNCTION public
//	Predicate::Impl::NotImpl::estimateCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Cost& cResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::NotImpl::
estimateCost(Opt::Environment& cEnvironment_,
			 AccessPlan::Cost& cResult_)
{
	AccessPlan::Cost cOperandCost;
	cOperandCost.setIsSetCount(cResult_.isSetCount());
	cOperandCost.setIsSetRate(cResult_.isSetRate());
	cOperandCost.setTupleCount(cResult_.getTupleCount());
	cOperandCost.setTableCount(cResult_.getTableCount());

	if (getOperand()->estimateCost(cEnvironment_,
								   cOperandCost)) {
		if (cResult_.isSetRate()) {
			cResult_.setRate(1.0 - cOperandCost.getRate().get());
		} else {
			cResult_.setRate(1);
		}
		if (cResult_.isSetCount()) {
			AccessPlan::Cost::Value cCount = cResult_.getTableCount();
			if (cCount.isInfinity()) {
				// use default
				cCount = 100000;
			}
			cOperandCost.setTupleCount(_maxCost(1,
												cCount - cOperandCost.getTupleCount()));
		} else {
			cOperandCost.setTupleCount(AccessPlan::Cost::Value());
		}
		cResult_ = cOperandCost;
		return true;
	}
	return false;
}

// FUNCTION public
//	Predicate::Impl::NotImpl::checkRate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Utility::RelationSet& cTable_
//	
// RETURN
//	AccessPlan::Cost::Value
//
// EXCEPTIONS

//virtual
AccessPlan::Cost::Value
Impl::NotImpl::
checkRate(Opt::Environment& cEnvironment_,
		  const Utility::RelationSet& cTable_)
{
	AccessPlan::Cost::Value cResult = getOperand()->checkRate(cEnvironment_,
															  cTable_);
	if (cResult < 1.0) {
		return 1.0 - cResult.get();
	}
	return 1.0;
}

// FUNCTION public
//	Predicate::Impl::NotImpl::generate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Impl::NotImpl::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	// get variable ID corresponding to the predicate
	int iID = getNodeVariable(pIterator_, cArgument_);
	if (iID < 0) {
		int iOperandID = getOperand()->generate(cEnvironment_, cProgram_, pIterator_, cArgument_);

		Execution::Interface::IPredicate* pNot =
			Execution::Predicate::Combinator::Not::create(cProgram_,
														  pIterator_,
														  iOperandID);
		iID = pNot->getID();
		// register predicate <-> id relationship
		setNodeVariable(pIterator_, cArgument_, iID);
	}
	return iID;
}

// FUNCTION private
//	Predicate::Impl::NotImpl::toSQLStatement
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	STRING
//
// EXCEPTIONS

//virtual
STRING
Impl::NotImpl::
toSQLStatement(Opt::Environment& cEnvironment_, const Plan::Sql::QueryArgument& cArgument_) const
{
	OSTRSTREAM cStream;
	cStream << _pszOperatorName[_Explain::Not];
	cStream << '(' <<getOperand()->toSQLStatement(cEnvironment_, cArgument_) << ')';
	return cStream.getString();
}

// FUNCTION public
//	Predicate::Impl::NotImpl::setParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec
//	const Plan::Sql::QueryArgument& cArgument_
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
Impl::NotImpl::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	cExec.append(_pszOperatorName[_Explain::Not]).append('(');
	getOperand()->setParameter(cEnvironment_, cProgram_, pIterator_, cExec, cArgument_);
	cExec.append(')');
}



// FUNCTION private
//	Predicate::Impl::NotImpl::explainOperator -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
const char*
Impl::NotImpl::
explainOperator() const
{
	return _pszOperatorName[_Explain::Not];
}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2018, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
