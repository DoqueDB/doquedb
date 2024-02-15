// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Row.h --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_CANDIDATE_ROW_H
#define __SYDNEY_PLAN_CANDIDATE_ROW_H

#include "Plan/Candidate/Module.h"
#include "Plan/Declaration.h"
#include "Plan/Utility/ObjectSet.h"

#include "Common/Object.h"

#include "Execution/Declaration.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

////////////////////////////////////////////////////////////////
// CLASS
//	Plan::Candidate::Row -- row information in access plan
//
// NOTES
//	This class is NOT a subclass of Interface::ICandidate

class Row
	: public Common::Object
{
public:
	typedef Row This;
	typedef Common::Object Super;
	typedef VECTOR<Interface::IScalar*> Vector;
	typedef Vector::Iterator Iterator;
	typedef Vector::ConstIterator ConstIterator;

	// constructor
	static This* create(Opt::Environment& cEnvironment_);
	static This* create(Opt::Environment& cEnvironment_,
						const Vector& vecScalar_);

	// destructor
	static void erase(Opt::Environment& cEnvironment_,
					  This* pThis_);

	// accesor
	Iterator begin() {return m_vecScalar.begin();}
	Iterator end() {return m_vecScalar.end();}
	ConstIterator begin() const {return m_vecScalar.begin();}
	ConstIterator end() const {return m_vecScalar.end();}
	int getSize() {return m_vecScalar.GETSIZE();}
	bool isEmpty() {return m_vecScalar.ISEMPTY();}

	/////////////////////////////
	// creating
	/////////////////////////////

	// add one scalar
	void addScalar(Interface::IScalar* pScalar_);
	// append another row
	void addRow(const This& cRow_);

	/////////////////////////////
	// generating
	/////////////////////////////

	// create delayable subset
	bool delay(Opt::Environment& cEnvironment_,
			   Interface::ICandidate* pCandidate_,
			   RowDelayArgument& cArgument_);
	// create delayable subset (get keys and non-delayed as another Row)
	This* delay(Opt::Environment& cEnvironment_,
				Interface::ICandidate* pCandidate_,
				bool bMinimum_ = false);

	// generate output data
	int generate(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Execution::Interface::IIterator* pIterator_,
				 Candidate::AdoptArgument& cArgument_);
	int generateFromType(Opt::Environment& cEnvironment_,
						 Execution::Interface::IProgram& cProgram_,
						 Execution::Interface::IIterator* pIterator_,
						 Candidate::AdoptArgument& cArgument_);

	// TEMPLATE FUNCTION
	//	Plan::Candidate::Row::foreachElement -- apply function for each element
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
	template <class Function_>
	Function_
	foreachElement(Function_ function_)
	{
		if (getSize() == 1) {
			function_(*begin());
			return function_;
		}
		return FOREACH(begin(), end(), function_);
	}

protected:
private:
	friend class AUTOPOINTER<This>;

	// constructor
	Row()
		: Super(),
		  m_iID(-1),
		  m_vecScalar()
	{}
	Row(const Vector& vecScalar_)
		: Super(),
		  m_iID(-1),
		  m_vecScalar(vecScalar_)
	{}
	// destructor
	~Row() {}

	// register to environment
	void registerToEnvironment(Opt::Environment& cEnvironment_);
	// erase from environment
	void eraseFromEnvironment(Opt::Environment& cEnvironment_);

	// id in environment
	int m_iID;
	// scalar list
	Vector m_vecScalar;
};

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_CANDIDATE_ROW_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
