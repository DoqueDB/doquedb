// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/WordExtraction.h --
// 
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_DPLAN_CANDIDATE_WORDEXTRACTION_H
#define __SYDNEY_DPLAN_CANDIDATE_WORDEXTRACTION_H

#include "DPlan/Module.h"
#include "DPlan/Declaration.h"


#include "Plan/Candidate/Base.h"
#include "Plan/Candidate/Monadic.h"
#include "DPlan/Predicate/Contains.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_CANDIDATE_BEGIN

////////////////////////////////////////////////////
// CLASS local
//	Plan::Candidate::WordExtraction -- implementation class of Interface::ICandidate for limit
//
// NOTES
class WordExtraction
	: public Plan::Candidate::Monadic<Plan::Candidate::Base>
{
public:
	typedef WordExtraction This;
	typedef Plan::Candidate::Monadic<Plan::Candidate::Base> Super;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Plan::Interface::ICandidate* pOperand_,
						Predicate::Contains* pContains_,
						bool bIsNeedScore_);

	// destructor
	~WordExtraction() {}


/////////////////////////////
// Interface::ICandidate::
//	virtual void createCost(Opt::Environment& cEnvironment_,
//							const AccessPlan::Source& cPlanSource_);
//	virtual const AccessPlan::Cost& getCost();

	virtual Execution::Interface::IIterator* adopt(Opt::Environment& cEnvironment_,
												   Execution::Interface::IProgram& cProgram_,
												   Plan::Candidate::AdoptArgument& cArgument_);

	virtual Plan::Sql::Query* generateSQL(Opt::Environment& cEnvironment_);

	//	Word取得の場合、Cascadeで単語ソートができないため、(order by word.term)
	//	分散マネージャでマージソートを使用した集約ができない。
    //	Score取得の場合は、Cascadeでスコアをソートした結果を分散マネージャでマージする。
    //	(WordもScoreも取得しない場合は、本クラスは使用されない)
	virtual bool isMergeSortAvailable() {return m_bIsNeedScore;}
	
protected:
private:

	// constructor
	WordExtraction(Plan::Interface::ICandidate* pOperand_,
				   DPlan::Predicate::Contains* pContains_,
				   bool bIsNeedScore_)
		: Super(pOperand_),
		  m_pContains(pContains_),
		  m_bIsNeedScore(bIsNeedScore_)
		{}
	


/////////////////////////////
// Interface::ICandidate::
//	virtual void createCost(Opt::Environment& cEnvironment_,
//							const AccessPlan::Source& cPlanSource_);
//	virtual const AccessPlan::Cost& getCost();	

	DPlan::Predicate::Contains* m_pContains;
	bool m_bIsNeedScore;
};

_SYDNEY_DPLAN_CANDIDATE_END
_SYDNEY_DPLAN_END
_SYDNEY_END

#endif // __SYDNEY_DPLAN_CANDIDATE_WORDEXTRACTION_H

//
//	Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
