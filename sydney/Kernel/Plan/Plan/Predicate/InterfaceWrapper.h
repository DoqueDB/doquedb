// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/InterfaceWrapper.h --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_PREDICATE_INTERFACEWRAPPER_H
#define __SYDNEY_PLAN_PREDICATE_INTERFACEWRAPPER_H

#include "Plan/Interface/IPredicate.h"


_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Predicate::InterfaceWrapper -- Wrapper interface for predicate
//
//	NOTES
//		This class is not constructed directly
class InterfaceWrapper
	: public Interface::IPredicate
{
public:
	typedef Interface::IPredicate Super;
	typedef InterfaceWrapper This;

	// destructor
	virtual ~InterfaceWrapper() {}

////////////////////////////
// Interface::IPredicate
	virtual void require(Opt::Environment& cEnvironment_,
						 Interface::ICandidate* pCandidate_)
	{return m_pPredicate->require(cEnvironment_, pCandidate_);}
	virtual void retrieve(Opt::Environment& cEnvironment_)
	{return m_pPredicate->retrieve(cEnvironment_);}
	virtual void retrieve(Opt::Environment& cEnvironment_,
						  Interface::ICandidate* pCandidate_)
	{return m_pPredicate->retrieve(cEnvironment_, pCandidate_);}
	virtual void use(Opt::Environment& cEnvironment_,
					 Interface::ICandidate* pCandidate_)
	{return m_pPredicate->use(cEnvironment_, pCandidate_);}
	virtual bool delay(Opt::Environment& cEnvironment_,
					   Interface::ICandidate* pCandidate_,
					   Scalar::DelayArgument& cArgument_)
	{return m_pPredicate->delay(cEnvironment_, pCandidate_, cArgument_);}

	virtual Interface::IPredicate* check(Opt::Environment& cEnvironment_,
										 const CheckArgument& cArgument_)
	{return m_pPredicate->check(cEnvironment_, cArgument_);}

	virtual bool isChecked()
	{return m_pPredicate->isChecked();}
	virtual CheckedInterface* getChecked()
	{return m_pPredicate->getChecked();}

	virtual Interface::IPredicate* getNotChecked()
	{return m_pPredicate->getNotChecked();}

	virtual bool hasSubquery()
	{return m_pPredicate->hasSubquery();}

	virtual bool isFetch() {return m_pPredicate->isFetch();}
	virtual bool getFetchKey(Opt::Environment& cEnvironment_,
							 Utility::ScalarSet& cFetchKey_)
	{return m_pPredicate->getFetchKey(cEnvironment_, cFetchKey_);}
	virtual Interface::IPredicate* createFetch(Opt::Environment& cEnvironment_,
											   Utility::ScalarSet& cFetchKey_)
	{return m_pPredicate->createFetch(cEnvironment_, cFetchKey_);}
	virtual bool isNeedIndex() {return m_pPredicate->isNeedIndex();}

	virtual bool estimateCost(Opt::Environment& cEnvironment_,
							  AccessPlan::Cost& cResult_)
	{return m_pPredicate->estimateCost(cEnvironment_,
									   cResult_);}
	virtual void setEstimateRate(const AccessPlan::Cost::Value& cValue_)
	{m_pPredicate->setEstimateRate(cValue_);}
	virtual AccessPlan::Cost::Value getEstimateRate()
	{return m_pPredicate->getEstimateRate();}

	virtual int generateKey(Opt::Environment& cEnvironment_,
							Execution::Interface::IProgram& cProgram_,
							Execution::Interface::IIterator* pIterator_,
							Candidate::AdoptArgument& cArgument_)
	{return m_pPredicate->generateKey(cEnvironment_, cProgram_, pIterator_, cArgument_);}

	virtual void adoptIndex(Opt::Environment& cEnvironment_,
							Execution::Interface::IProgram& cProgram_,
							Execution::Action::FileAccess* pFileAccess_,
							Candidate::File* pFile_,
							Candidate::AdoptArgument& cArgument_)
	{return m_pPredicate->adoptIndex(cEnvironment_,
									 cProgram_,
									 pFileAccess_,
									 pFile_,
									 cArgument_);}

////////////////////////
// Interface::IScalar
	virtual void explain(Opt::Environment* pEnvironment_,
						 Opt::Explain& cExplain_)
	{m_pPredicate->explain(pEnvironment_, cExplain_);}
//	virtual const STRING& getName();
	virtual Check::Value check(Opt::Environment& cEnvironment_,
							   const Scalar::CheckArgument& cArgument_)
	{return m_pPredicate->check(cEnvironment_,
								cArgument_);}
	virtual bool isRefering(Interface::IRelation* pRelation_)
	{return m_pPredicate->isRefering(pRelation_);}
	virtual void getUsedTable(Utility::RelationSet& cResult_)
	{m_pPredicate->getUsedTable(cResult_);}
	virtual void getUsedField(Utility::FieldSet& cResult_)
	{m_pPredicate->getUsedField(cResult_);}
	virtual void getUnknownKey(Opt::Environment& cEnvironment_,
							   Predicate::CheckUnknownArgument& cResult_)
	{m_pPredicate->getUnknownKey(cEnvironment_, cResult_);}
	virtual bool isKnownNull(Opt::Environment& cEnvironment_)
	{return m_pPredicate->isKnownNull(cEnvironment_);}
	virtual bool isKnownNotNull(Opt::Environment& cEnvironment_)
	{return m_pPredicate->isKnownNotNull(cEnvironment_);}
	virtual bool hasParameter()
	{return m_pPredicate->hasParameter();}
	virtual int generate(Opt::Environment& cEnvironment_,
						 Execution::Interface::IProgram& cProgram_,
						 Execution::Interface::IIterator* pIterator_,
						 Candidate::AdoptArgument& cArgument_)
	{return m_pPredicate->generate(cEnvironment_, cProgram_, pIterator_, cArgument_);}

	virtual bool equalsOperand(const Plan::Interface::IScalar* arg) const
	{return m_pPredicate->equalsOperand(arg);}

////////////////////////
// Tree::Node::
	Super::Type getType() const
	{return m_pPredicate->getType();}
	virtual ModUnicodeString getValue() const
	{return m_pPredicate->getValue();}
	virtual const Common::Data* getData() const
	{return m_pPredicate->getData();}

	virtual ModSize getOptionSize() const
	{return m_pPredicate->getOptionSize();}
	virtual const Tree::Node::Super* getOptionAt(ModInt32 iPosition_) const
	{return m_pPredicate->getOptionAt(iPosition_);}

	virtual ModSize getOperandSize() const
	{return m_pPredicate->getOperandSize();}
	virtual const Tree::Node::Super* getOperandAt(ModInt32 iPosition_) const
	{return m_pPredicate->getOperandAt(iPosition_);}

	///////////////////////////
	// Interface::ISqlNode::
	virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
								  const Plan::Sql::QueryArgument& cArgument_) const
	{return m_pPredicate->toSQLStatement(cEnvironment_, cArgument_);}

	virtual void setParameter(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Execution::Interface::IIterator* pIterator_,
							  DExecution::Action::StatementConstruction& cExec_,
							  const Plan::Sql::QueryArgument& cArgument_)
	{m_pPredicate->setParameter(cEnvironment_, cProgram_, pIterator_, cExec_, cArgument_);}

//////////////////
// accessors
	Interface::IPredicate* getPredicate() {return m_pPredicate;}

protected:
	// constructor
	InterfaceWrapper(Interface::IPredicate* pPredicate_)
		: Super(pPredicate_->getType()),
		  m_pPredicate(pPredicate_)
	{}

private:
	Interface::IPredicate* m_pPredicate;
};

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_PREDICATE_INTERFACEWRAPPER_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
