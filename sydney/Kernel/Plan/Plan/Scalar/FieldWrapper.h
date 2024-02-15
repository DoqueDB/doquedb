// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/FieldWrapper.h --
// 
// Copyright (c) 2009, 2010, 2011, 2012, 2013, 2016, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_FIELDWRAPPER_H
#define __SYDNEY_PLAN_SCALAR_FIELDWRAPPER_H

#include "Plan/Scalar/Field.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Scalar::FieldWrapper -- Interface for schema field
//
//	NOTES
class FieldWrapper
	: public Field
{
public:
	typedef Field Super;
	typedef FieldWrapper This;

	// destructor
	virtual ~FieldWrapper() {}

	// accessor
	Field* getWrappedField() {return m_pField;}

////////////////////////////////////
// Field::
	virtual Relation::Table* getTable() {return m_pField->getTable();}
	virtual Interface::IFile* getFile() {return m_pField->getFile();}
	virtual Scalar::Field* getField(Interface::IFile* pFile_)
	{return m_pField->getField(pFile_);}
	virtual Schema::Field* getSchemaField() {return m_pField->getSchemaField();}
	virtual Schema::Column* getSchemaColumn() {return m_pField->getSchemaColumn();}
	virtual bool isColumn() {return m_pField->isColumn();}
	virtual bool isRowID() {return m_pField->isRowID();}
	virtual bool isLob() {return m_pField->isLob();}
	virtual bool isObjectID() {return m_pField->isObjectID();}
	virtual bool isUnique(Opt::Environment& cEnvironment_)
	{return m_pField->isUnique(cEnvironment_);}
	virtual bool isDelayable() {return m_pField->isDelayable();}
	virtual bool isPutKey() {return m_pField->isPutKey();}
	virtual bool isDelayed(Candidate::Table* pCandidate_)
	{return m_pField->isDelayed(pCandidate_);}
	virtual Field* getDelayedKey(Opt::Environment& cEnvironment_)
	{return m_pField->getDelayedKey(cEnvironment_);}
	virtual bool isRowIDAvailable(Opt::Environment& cEnvironment_)
	{return m_pField->isRowIDAvailable(cEnvironment_);}
	virtual bool hasAlternativeValue(Opt::Environment& cEnvironment_)
	{return m_pField->hasAlternativeValue(cEnvironment_);}
	virtual Interface::IScalar* getAlternativeValue(Opt::Environment& cEnvironment_)
	{return m_pField->getAlternativeValue(cEnvironment_);}
	virtual Field* createFetchKey(Opt::Environment& cEnvironment_)
	{return m_pField->createFetchKey(cEnvironment_);}

	virtual Field* checkRetrieve(Opt::Environment& cEnvironment_)
	{m_pField = m_pField->checkRetrieve(cEnvironment_);return this;}
	virtual Field* checkPut(Opt::Environment& cEnvironment_)
	{m_pField = m_pField->checkPut(cEnvironment_);return this;}
	virtual void createRetrieveFile(Opt::Environment& cEnvironment_,
									Utility::FileSet& cFileSet_)
	{m_pField->createRetrieveFile(cEnvironment_, cFileSet_);}
	virtual bool isChecked()
	{return m_pField->isChecked();}
	virtual CheckedField* getChecked()
	{return m_pField->getChecked();}
	virtual bool isUpdate()
	{return m_pField->isUpdate();}
	virtual UpdateField* getUpdate()
	{return m_pField->getUpdate();}

	virtual bool isWrapper()
	{return true;}
	virtual FieldWrapper* getWrapper()
	{return this;}

	virtual bool isFunction()
	{return m_pField->isFunction();}
	virtual Interface::IScalar* getFunction()
	{return m_pField->getFunction();}
	virtual bool isChoice()
	{return m_pField->isChoice();}
	virtual bool checkAvailability(Interface::IPredicate* pPredicate_)
	{return m_pField->checkAvailability(pPredicate_);}
	virtual void addField(Interface::IFile* pFile_,
						  Utility::FieldSet& cFieldSet_)
	{m_pField->addField(pFile_,
						cFieldSet_);}
	virtual void addLocator(Opt::Environment& cEnvironment_,
							Execution::Interface::IIterator* pIterator_,
							int iLocatorID_)
	{m_pField->addLocator(cEnvironment_,
						  pIterator_,
						  iLocatorID_);}
	// get locator from environment
	virtual int getLocator(Opt::Environment& cEnvironment_,
						   Execution::Interface::IIterator* pIterator_)
	{return m_pField->getLocator(cEnvironment_,
								 pIterator_);}

/////////////////////////////////////
// Interface::IScalar::
	virtual void explain(Opt::Environment* pEnvironment_,
						 Opt::Explain& cExplain_)
	{return m_pField->explain(pEnvironment_, cExplain_);}
	virtual const STRING& getName() {return m_pField->getName();}
	virtual Interface::IScalar* setExpectedType(Opt::Environment& cEnvironment_,
												const Scalar::DataType& cType_)
	{return m_pField->setExpectedType(cEnvironment_, cType_);}
	virtual Interface::IScalar* createCast(Opt::Environment& cEnvironment_,
										   const DataType& cToType_,
										   bool bForComparison_,
										   Tree::Node::Type eType_ = Tree::Node::Undefined)
	{
		Interface::IScalar* pCast =
			m_pField->createCast(cEnvironment_, cToType_, bForComparison_, eType_);
		if (pCast == m_pField) {
			return this;
		} else {
			return Super::createCast(cEnvironment_,
									 cToType_,
									 bForComparison_,
									 eType_);
		}
	}
	virtual Check::Value check(Opt::Environment& cEnvironment_,
							   const CheckArgument& cArgument_)
	{return m_pField->check(cEnvironment_,
							cArgument_);}
	virtual bool isRefering(Interface::IRelation* pRelation_)
	{return m_pField->isRefering(pRelation_);}
	virtual void getUsedTable(Utility::RelationSet& cResult_)
	{m_pField->getUsedTable(cResult_);}
	virtual void getUsedField(Utility::FieldSet& cResult_)
	{m_pField->getUsedField(cResult_);}
	virtual void getUnknownKey(Opt::Environment& cEnvironment_,
							   Predicate::CheckUnknownArgument& cResult_)
	{m_pField->getUnknownKey(cEnvironment_, cResult_);}
	virtual bool isKnownNull(Opt::Environment& cEnvironment_)
	{return m_pField->isKnownNull(cEnvironment_);}
	virtual bool isKnownNotNull(Opt::Environment& cEnvironment_)
	{return m_pField->isKnownNotNull(cEnvironment_);}
	virtual bool hasParameter()
	{return m_pField->hasParameter();}
	virtual bool isArbitraryElement()
	{return m_pField->isArbitraryElement();}
	virtual bool isExpandElement()
	{return m_pField->isExpandElement();}	
	virtual bool hasField(Interface::IFile* pFile_)
	{return m_pField->hasField(pFile_);}
	virtual bool isEquivalent(Interface::IScalar* pScalar_)
	{return pScalar_->isEquivalent(m_pField);}
	virtual Interface::IScalar* addOption(Opt::Environment& cEnvironment_,
										  Interface::IScalar* pOption_)
	{return m_pField->addOption(cEnvironment_, pOption_);}
	virtual void require(Opt::Environment& cEnvironment_,
						 Interface::ICandidate* pCandidate_)
	{m_pField->require(cEnvironment_, pCandidate_);}
	virtual void retrieve(Opt::Environment& cEnvironment_)
	{m_pField->retrieve(cEnvironment_);}
	virtual void retrieve(Opt::Environment& cEnvironment_,
						 Interface::ICandidate* pCandidate_)
	{m_pField->retrieve(cEnvironment_, pCandidate_);}

	virtual void retrieveFromCascade(Opt::Environment& cEnvironment_,
									 Sql::Query* pQuery_)
	{m_pField->retrieveFromCascade(cEnvironment_, pQuery_);}
	
	virtual void use(Opt::Environment& cEnvironment_,
					 Interface::ICandidate* pCandidate_)
	{m_pField->use(cEnvironment_, pCandidate_);}
	virtual bool delay(Opt::Environment& cEnvironment_,
					   Interface::ICandidate* pCandidate_,
					   Scalar::DelayArgument& cArgument_)
	{return m_pField->delay(cEnvironment_, pCandidate_, cArgument_);}
	virtual void setMetaData(Opt::Environment& cEnvironment_,
							 Common::ColumnMetaData& cMetaData_)
	{m_pField->setMetaData(cEnvironment_, cMetaData_);}
	virtual int generate(Opt::Environment& cEnvironment_,
						 Execution::Interface::IProgram& cProgram_,
						 Execution::Interface::IIterator* pIterator_,
						 Candidate::AdoptArgument& cArgument_)
	{return m_pField->generate(cEnvironment_, cProgram_, pIterator_, cArgument_);}
	virtual int generateFromType(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_)
	{return m_pField->generateFromType(cEnvironment_, cProgram_, pIterator_, cArgument_);}

///////////////////////////
// Interface::ISqlNode::		
	virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
								  const Plan::Sql::QueryArgument& cArgument_) const
	{return m_pField->toSQLStatement(cEnvironment_, cArgument_);}

	virtual void setParameter(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Execution::Interface::IIterator* pIterator_,
							  DExecution::Action::StatementConstruction& cExec_,
							  const Plan::Sql::QueryArgument& cArgument_)
	{m_pField->setParameter(cEnvironment_, cProgram_, pIterator_, cExec_, cArgument_);}	

/////////////////////////////////////
// Node::
	virtual ModUnicodeString getValue() const
	{return m_pField->getValue();}
	virtual const Common::Data* getData() const
	{return m_pField->getData();}

protected:
	// constructor
	FieldWrapper(Field* pField_)
		: Super(),
		  m_pField(pField_)
	{}
	// constructor
	FieldWrapper(Field* pField_,
				 const DataType& cDataType_)
		: Super(cDataType_),
		  m_pField(pField_)
	{}

	// register to environment
	void registerToEnvironment(Opt::Environment& cEnvironment_)
	{
		// skip Field's implementation
		Interface::IScalar::registerToEnvironment(cEnvironment_);
	}

private:
	Field* m_pField;
};

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_FIELDWRAPPER_H

//
//	Copyright (c) 2009, 2010, 2011, 2012, 2013, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
