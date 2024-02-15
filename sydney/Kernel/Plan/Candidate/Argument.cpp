// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Argument.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Candidate";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/File.h"
#include "Plan/Candidate/Table.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Predicate/ChosenInterface.h"

#include "Common/Assert.h"
#include "Common/BitSet.h"

#include "Execution/Interface/IProgram.h"

#include "Opt/Algorithm.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_CANDIDATE_USING

//////////////////////////////////////////
// Plan::Candidate::Inquiry

// FUNCTION public
//	Candidate::Inquiry::isReferTable -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	Schema::Table* pSchemaTable_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Inquiry::
isReferTable(Opt::Environment& cEnvironment_,
			 Interface::ICandidate* pCandidate_,
			 Schema::Table* pSchemaTable_)
{
	return pCandidate_->inquiry(cEnvironment_,
								InquiryArgument(InquiryArgument::Target::ReferTable, 
												pSchemaTable_))
		== InquiryArgument::Target::ReferTable;
}

//////////////////////////////////////////
// Plan::Candidate::AdoptArgument

// FUNCTION public
//	Candidate::AdoptArgument::setCandidate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::ICandidate* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
AdoptArgument::
setCandidate(Interface::ICandidate* pCandidate_)
{
	if (m_pCandidate) {
		m_vecPrecedingCandidate.PUSHFRONT(m_pCandidate);
	}
	m_pCandidate = pCandidate_;
}

// FUNCTION public
//	Candidate::AdoptArgument::pushScope -- push if-else scope
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
AdoptArgument::
pushScope()
{
	m_vecmapNodeVariable.PUSHBACK(MAP<int, int, LESS<int> >());
}

// FUNCTION public
//	Candidate::AdoptArgument::popScope -- pop if-else scope
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
AdoptArgument::
popScope()
{
	if (m_vecmapNodeVariable.ISEMPTY() == false) {
		m_vecmapNodeVariable.POPBACK();
	}
}

// FUNCTION public
//	Candidate::AdoptArgument::getNodeVariable -- get node-variable correspondence
//
// NOTES
//
// ARGUMENTS
//	int iNodeID_
//	Execution::Interface::IIterator* pIterator_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
AdoptArgument::
getNodeVariable(int iNodeID_)
{
	if (m_vecmapNodeVariable.ISEMPTY() == false) {
		VECTOR<MAP<int, int, LESS<int> > >::ITERATOR iterator = m_vecmapNodeVariable.end();
		VECTOR<MAP<int, int, LESS<int> > >::ITERATOR last = m_vecmapNodeVariable.begin();
		while (iterator != last) {
			--iterator;
			MAP<int, int, LESS<int> >::ITERATOR found = (*iterator).find(iNodeID_);
			if (found != (*iterator).end()) {
				return (*found).second;
			}
		}
	}
	return -1;
}

// FUNCTION public
//	Candidate::AdoptArgument::setNodeVariable -- set node-variable correspondence
//
// NOTES
//
// ARGUMENTS
//	int iNodeID_
//	int iDataID_
//	
// RETURN
//	bool	true when correspondence has been added successfully
//
// EXCEPTIONS

bool
AdoptArgument::
setNodeVariable(int iNodeID_,
				int iDataID_)
{
	if (m_vecmapNodeVariable.ISEMPTY() == false) {
		// add to last scope
		m_vecmapNodeVariable.GETBACK()[iNodeID_] = iDataID_;
		return true;
	}
	return false;
}

//////////////////////////////////////////
// Plan::Candidate::CheckIndexArgument

// FUNCTION public
//	Candidate::CheckIndexArgument::setAnd -- 
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
CheckIndexArgument::
setAnd()
{
	m_bInAnd = true;
}

// FUNCTION public
//	Candidate::CheckIndexArgument::setOr -- 
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
CheckIndexArgument::
setOr()
{
// searchbitset can be used when or is under and
//	m_bUseSearchBitSet = false;
	m_bInOr = true;
	m_bInAnd = false;
	m_bTop = false;
}

// FUNCTION public
//	Candidate::CheckIndexArgument::isScanBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
CheckIndexArgument::
isScanBitSet(Opt::Environment& cEnvironment_,
			 Candidate::Table* pTable_)
{
	Predicate::CheckNeedScanArgument cArgument(pTable_, m_bTop);

	return (m_vecBitSet.ISEMPTY() == false
			&& m_vecBitSet[0]->isNeedScan(cArgument) == false)
		&& (
			(m_pOrderScan
			 && m_pOrderScan->getFile(cEnvironment_)
			 && m_pOrderScan->getFile(cEnvironment_)->isSearchByBitSet())
			||
			(m_pOrderScan == 0
			 && (m_vecBitSet.GETSIZE() > 1
				 || m_vecBitSet[0]->getFile(cEnvironment_) == 0
				 || m_vecBitSet[0]->getFile(cEnvironment_)->isAlwaysBitSet()
				 || m_vecSearchBitSet.ISEMPTY() == false)));
}

// FUNCTION public
//	Candidate::CheckIndexArgument::isSearchBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
CheckIndexArgument::
isSearchBitSet(Opt::Environment& cEnvironment_)
{
	return (m_bInAnd && m_vecSearchBitSet.ISEMPTY() == false)
		|| (m_bInOr && m_vecSearchBitSet.ISEMPTY() == false
			&& m_vecNeedScan.ISEMPTY()
			&& m_vecIndexScan.ISEMPTY()
			&& m_vecFetch.ISEMPTY()
			&& (m_vecBitSet.ISEMPTY()
				|| Opt::IsAll(m_vecBitSet,
							  boost::bind(&Predicate::ChosenInterface::isSearchByBitSet,
										  _1))));
}

// FUNCTION public
//	Candidate::CheckIndexArgument::isEmpty -- 
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
CheckIndexArgument::
isEmpty()
{
	return m_vecNeedScan.ISEMPTY()
		&& m_vecBitSet.ISEMPTY()
		&& m_vecSearchBitSet.ISEMPTY()
		&& m_vecIndexScan.ISEMPTY()
		&& m_vecFetch.ISEMPTY()
		&& m_pOrderScan == 0;
}


// FUNCTION public
//	Candidate::CheckIndexArgument::isOnlyBitSet -- 
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
CheckIndexArgument::
isOnlyBitSet()
{
	return m_vecNeedScan.ISEMPTY()
		&& m_vecSearchBitSet.ISEMPTY()
		&& m_vecIndexScan.ISEMPTY()
		&& m_vecFetch.ISEMPTY();
}


// FUNCTION public
//	Candidate::CheckIndexArgument::isOnlySearchByBitSet -- 
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
CheckIndexArgument::
hasBitSet()
{
	return (m_vecBitSet.getSize() > 0);
}

// FUNCTION public
//	Candidate::CheckIndexArgument::isLimitAvailable -- 
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
CheckIndexArgument::
isLimitAvailable()
{
	return (m_vecNeedScan.ISEMPTY()
			&& m_vecIndexScan.ISEMPTY()
			&& m_vecFetch.ISEMPTY()
			&& ((m_bInAnd == true
				 && (m_vecSearchBitSet.GETSIZE() == 1
					 || (m_pOrderScan && m_pOrderScan->isAbleToSearchByBitSet())))
				|| (m_bInAnd == false
					&& m_vecSearchBitSet.ISEMPTY())));
}

//////////////////////////////////////////
// Plan::Candidate::AdoptIndexArgument

/////////////////////////////////////////////////
// Plan::Candidate::AdoptIndexArgument::Element

// FUNCTION public
//	Candidate::AdoptIndexArgument::Element::prepareBitSetData -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
AdoptIndexArgument::Element::
prepareBitSetData(Execution::Interface::IProgram& cProgram_)
{
	if (m_iBitSetID < 0) {
		// prepare bitsetID
		Common::Data::Pointer pBitSet = new Common::BitSet;
		m_iBitSetID = cProgram_.addVariable(pBitSet);
	}
	return m_iBitSetID;
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
