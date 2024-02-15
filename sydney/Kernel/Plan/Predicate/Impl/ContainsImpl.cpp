// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/ContainsImpl.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2017, 2023 Ricoh Company, Ltd.
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

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Predicate/Impl/ContainsImpl.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Predicate/CheckedInterface.h"

#include "Plan/AccessPlan/Source.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/File.h"
#include "Plan/Candidate/Table.h"
#include "Plan/File/KwicOption.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Interface/IFile.h"
#include "Plan/Interface/IRelation.h"
#include "Plan/Relation/RowInfo.h"
#include "Plan/Relation/Table.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Scalar/Function.h"

#include "Common/Assert.h"

#include "Exception/FullTextIndexNeeded.h"
#include "Exception/InvalidRankFrom.h"
#include "Exception/NotSupported.h"
#include "Exception/PrepareFailed.h"

#include "Execution/Action/Argument.h"
#include "Execution/Action/FileAccess.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Operator/BitSet.h"
#include "Execution/Operator/FileGetProperty.h"
#include "Execution/Operator/FileOperation.h"
#include "Execution/Operator/Iterate.h"

#include "Opt/Environment.h"
#include "Opt/Explain.h"

#include "Schema/Column.h"
#include "Schema/File.h"
#include "Schema/Index.h"
#include "Schema/Key.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace
{
	const char* const _pszOperatorName = " contains ";
}

/////////////////////////////////////////////
//	Plan::Predicate::Impl::ContainsImpl

// FUNCTION public
//	Predicate::Impl::ContainsImpl::createKwicOption -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pKwicSize_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ContainsImpl::
createKwicOption(Opt::Environment& cEnvironment_,
				 Interface::IScalar* pKwicSize_)
{
	if (m_pKwicOption == 0) {
		// in preparation, kwicoption with placeholder can't be adopted here
		if (cEnvironment_.isPrepare()
			&& pKwicSize_->hasParameter()) {
			_SYDNEY_THROW0(Exception::PrepareFailed);
		}
		m_pKwicOption = File::KwicOption::create(cEnvironment_);
		m_pKwicOption->setKwicSize(pKwicSize_);
	}
}

// FUNCTION public
//	Predicate::Impl::ContainsImpl::check -- 
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
Impl::ContainsImpl::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	// if right operand or options are using non-constant, not supported
	if (!getOperand1()->isRefering(0)
		|| !isAllOption(boost::bind(&Interface::IScalar::isRefering,
									_1,
									static_cast<Interface::IRelation*>(0)))) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	// check 1st operand's status
	Interface::IScalar::Check::Value iStatus =
		getOperand0()->check(cEnvironment_,
							 cArgument_);

	if (Interface::IScalar::Check::isOn(iStatus, Interface::IScalar::Check::NotYet)) {
		// operand can not be evaluated for now
		return this;
	}

	// get searchable files for operand0
	Utility::FileSet cFile;
	Candidate::Table* pCandidate = 0;
	switch (getOperand0()->getType()) {
	case Tree::Node::Field:
		if (!Scalar::Field::getSearchFile(cEnvironment_,
										  Scalar::GetFileArgument(
											  getOperand0(),
											  this,
											  cFile))) {
			_SYDNEY_THROW0(Exception::FullTextIndexNeeded);
		}
		// get table candidate
		pCandidate = Scalar::Field::getCandidate(cEnvironment_,
												 getOperand0(),
												 cArgument_.m_pCandidate);
		break;
	case Tree::Node::List:
		if (!Scalar::Function::getSearchFile(cEnvironment_,
											 Scalar::GetFileArgument(
												 getOperand0(),
												 this,
												 cFile))) {
			_SYDNEY_THROW0(Exception::FullTextIndexNeeded);
		}
		// get table candidate
		pCandidate = Scalar::Function::getCandidate(cEnvironment_,
													getOperand0(),
													cArgument_.m_pCandidate);
		
		break;
	default:
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	if (pCandidate == 0) {
		return this;
	}

	if (m_pKwicOption) {
		// set language column if exists
		(void) cFile.isAny(boost::bind(&This::findLanguageColumn,
									   this,
									   boost::ref(cEnvironment_),
									   pCandidate,
									   _1));
	}

	return CheckedInterface::create(cEnvironment_,
									this,
									pCandidate,
									cFile);
}

// FUNCTION public
//	Predicate::Impl::ContainsImpl::adoptIndex -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Action::FileAccess* pFileAccess_
//	Candidate::File* pFile_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ContainsImpl::
adoptIndex(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Execution::Action::FileAccess* pFileAccess_,
		   Candidate::File* pFile_,
		   Candidate::AdoptArgument& cArgument_)
{
	if (m_pExpand) {
		// mark operand result row info as retrieved
		Plan::Relation::RowInfo* pRowInfo = m_pExpand->getRowInfo(cEnvironment_);
		if (pRowInfo) {
			pRowInfo->retrieveAll(cEnvironment_);
		}

		AccessPlan::Source cSource;
		Interface::ICandidate* pCandidate =
			m_pExpand->createAccessPlan(cEnvironment_, 
										cSource);
		if (pCandidate == 0) {
			// can't optimize
			_SYDNEY_THROW0(Exception::NotSupported);
		}

		Candidate::AdoptArgument cArgument;
		Execution::Interface::IIterator* pExpand =
			pCandidate->adopt(cEnvironment_, cProgram_, cArgument);
		if (pExpand == 0) {
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		pCandidate->generateDelayed(cEnvironment_, cProgram_, pExpand);
		// generate expand resultrow data
		int iDataID = m_pExpand->getRowInfo(cEnvironment_)->generate(cEnvironment_,
																	 cProgram_,
																	 pExpand,
																	 cArgument);
		// create insert action
		pExpand->addCalculation(cProgram_,
								Execution::Operator::FileOperation::Insert::create(
									   cProgram_,
									   pExpand,
									   pFileAccess_->getID(),
									   iDataID));

		// create iterateall action and set to fileaccess
		Execution::Operator::Iterate* pIterateAll =
			Execution::Operator::Iterate::All::create(cProgram_,
													  0,
													  pExpand->getID());

		pFileAccess_->addStartUp(cProgram_,
								 pIterateAll->getID());
	}
	if (m_pKwicOption) {
		// record processing index file
		m_pKwicOption->setFile(pFile_->getFile());

		// create getproperty action
		const PAIR<int, int>& cPropertyID = m_pKwicOption->generateProperty(cProgram_);
		Execution::Operator::FileGetProperty* pAction =
			Execution::Operator::FileGetProperty::create(cProgram_,
														 0,
														 pFileAccess_->getID(),
														 cPropertyID.first,
														 cPropertyID.second);
		pFileAccess_->addStartUp(cProgram_,
								 pAction->getID());
	}
	if (m_pRankFrom) {
		// mark operand result row info as retrieved
		Plan::Relation::RowInfo* pRowInfo = m_pRankFrom->getRowInfo(cEnvironment_);
		if (pRowInfo) {
			pRowInfo->retrieveAll(cEnvironment_);
		}
		; _SYDNEY_ASSERT(pRowInfo->getSize() == 1);
		; _SYDNEY_ASSERT(pRowInfo->getScalar(cEnvironment_, 0));
		; _SYDNEY_ASSERT(pRowInfo->getScalar(cEnvironment_, 0)->getDataType().isUnsignedType());

		AccessPlan::Source cSource;
		Interface::ICandidate* pCandidate =
			m_pRankFrom->createAccessPlan(cEnvironment_, 
										  cSource);
		if (pCandidate == 0) {
			// can't optimize
			_SYDNEY_THROW0(Exception::InvalidRankFrom);
		}

		Candidate::AdoptArgument cArgument;
		Execution::Interface::IIterator* pRankFrom =
			pCandidate->adopt(cEnvironment_, cProgram_, cArgument);
		if (pRankFrom == 0) {
			_SYDNEY_THROW0(Exception::InvalidRankFrom);
		}
		pCandidate->generateDelayed(cEnvironment_, cProgram_, pRankFrom);
		// generate expand resultrow data
		int iDataID = pRowInfo->generate(cEnvironment_,
										 cProgram_,
										 pRankFrom,
										 cArgument);
		// create bitset data to hold rank from data
		int iBitSetID = cProgram_.addVariable(Common::Data::Pointer(new Common::BitSet));
		pFile_->setRankBitSetID(iBitSetID);

		// create insert action
		pRankFrom->addCalculation(cProgram_,
								  Execution::Operator::BitSet::Collection::create(
									   cProgram_,
									   pRankFrom,
									   iDataID,
									   iBitSetID));

		// create iterateall action and set to fileaccess
		Execution::Operator::Iterate* pIterateAll =
			Execution::Operator::Iterate::All::create(cProgram_,
													  0,
													  pRankFrom->getID());

		pFileAccess_->addStartUp(cProgram_,
								 pIterateAll->getID(),
								 true /* before open */);
	}
}

// FUNCTION public
//	Predicate::Impl::ContainsImpl::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ContainsImpl::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	getOperand0()->explain(pEnvironment_, cExplain_);
	cExplain_.put(_pszOperatorName);
	cExplain_.popNoNewLine();
	getOperand1()->explain(pEnvironment_, cExplain_);
}


// FUNCTION public
//	Predicate::Impl::ContainsImpl::createSQL -- 
//
// NOTES
//
// ARGUMENTS
//	
//	
// RETURN
//	STRING
//
// EXCEPTIONS

//virtual
STRING
Impl::ContainsImpl::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	OSTRSTREAM cStream;
	cStream << getOperand0()->toSQLStatement(cEnvironment_, cArgument_);
	cStream << _pszOperatorName;
	cStream << getOperand1()->toSQLStatement(cEnvironment_, cArgument_);
	if (m_pExpand) {
		cStream << "expand (from (";
		cStream << m_pExpand->generateSQL(cEnvironment_)->toSQLStatement(cEnvironment_, cArgument_);
		cStream << "))";
	}
	if (m_pRankFrom) {
		cStream << "rank from (";
		cStream << m_pRankFrom->generateSQL(cEnvironment_)->toSQLStatement(cEnvironment_, cArgument_);
		cStream << ")";
	}
	return cStream.getString();
}



// FUNCTION public
//	Predicate::Impl::ContainsImpl::generate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Impl::ContainsImpl::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	_SYDNEY_THROW0(Exception::FullTextIndexNeeded);
}

// FUNCTION public
//	Predicate::Impl::ContainsImpl::getOptionSize -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModSize
//
// EXCEPTIONS

//virtual
ModSize
Impl::ContainsImpl::
getOptionSize() const
{
	return Super::getOptionSize()
		+ (m_pKwicOption ? 1 : 0)
		+ (m_pExpand ? 1 : 0);

	// m_pRankFrom is not needed for option
}

// FUNCTION public
//	Predicate::Impl::ContainsImpl::getOptionAt -- 
//
// NOTES
//
// ARGUMENTS
//	ModInt32 iPosition_
//	
// RETURN
//	const Tree::Node::Super*
//
// EXCEPTIONS

//virtual
const Tree::Node::Super*
Impl::ContainsImpl::
getOptionAt(ModInt32 iPosition_) const
{
	ModInt32 n = Super::getOptionSize();
	if (iPosition_ < n) {
		return Super::getOptionAt(iPosition_);
	}
	if (m_pKwicOption) ++n;
	if (iPosition_ < n) {
		return syd_reinterpret_cast<const Tree::Node::Super*>(m_pKwicOption->getKwicSize());
	}
	if (m_pExpand) ++n;
	if (iPosition_ < n) {
		return syd_reinterpret_cast<const Tree::Node::Super*>(m_pExpand);
	}

	// m_pRankFrom is not needed for option

	return 0;
}

// FUNCTION private
//	Predicate::Impl::ContainsImpl::findLanguageColumn -- find language column of operand
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::Table* pTable_
//	Interface::IFile* pFile_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Impl::ContainsImpl::
findLanguageColumn(Opt::Environment& cEnvironment_,
				   Candidate::Table* pTable_,
				   Interface::IFile* pFile_)
{
	Trans::Transaction& cTrans = cEnvironment_.getTransaction();
	Schema::Index* pIndex = pFile_->getSchemaFile()->getIndex(cTrans);
	; _SYDNEY_ASSERT(pIndex);
	; _SYDNEY_ASSERT(pIndex->isOffline() == false);

	const ModVector<Schema::Key*>& vecKey = pIndex->getKey(cTrans);
	ModSize nKey = vecKey.getSize();
	if (nKey > 1) {
		// search for language typed column from second key and after that
		for (ModSize iKey = 1; iKey < nKey; ++iKey) {
			Schema::Column* pColumn = vecKey[iKey]->getColumn(cTrans);
			if (pColumn->getType().getType() == Common::SQLData::Type::Language) {
				// found
				Interface::IScalar* pScalar = pTable_->getTable()->getScalar(cEnvironment_,
																			 pColumn->getPosition());
				// mark as retrieved
				pScalar->retrieve(cEnvironment_, pTable_);

				// set to kwic option
				m_pKwicOption->setLanguage(pScalar);

				return true;
			}
		}
	}
	return false;
}

// FUNCTION private
//	Predicate::Impl::ContainsImpl::addToEnvironment -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ContainsImpl::
addToEnvironment(Opt::Environment& cEnvironment_)
{
	cEnvironment_.addContains(getOperand0(), this);
}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2017, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
