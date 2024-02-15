// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Order/CheckedSpecification.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Order";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Order/Argument.h"
#include "Plan/Order/CheckedSpecification.h"
#include "Plan/Order/ChosenSpecification.h"
#include "Plan/Order/Key.h"

#include "Plan/Candidate/File.h"
#include "Plan/Candidate/Table.h"
#include "Plan/File/Argument.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Interface/IScalar.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Predicate/ChosenInterface.h"
#include "Plan/Relation/Table.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Tree/Monadic.h"
#include "Plan/Tree/Nadic.h"

#include "Common/Assert.h"

#include "Execution/Interface/IProgram.h"

#include "LogicalFile/AutoLogicalFile.h"

#include "Opt/Environment.h"
#include "Opt/Explain.h"

#include "Schema/Field.h"
#include "Schema/File.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_ORDER_BEGIN

namespace Impl
{
	// TEMPLATE CLASS
	//	Order::Impl::GenericCheckedSpecificationImpl
	//
	// TEMPLATE ARGUMENT
	//	class Handle_
	//
	// NOTES

	template <class Handle_>
	class GenericCheckedSpecificationImpl
		: public Handle_
	{
	public:
		typedef GenericCheckedSpecificationImpl<Handle_> This;
		typedef Handle_ Super;

		// destructor
		~GenericCheckedSpecificationImpl() {}

	///////////////////////////////
	// CheckedSpecification::
		virtual bool createCheckOrderArgument(Opt::Environment& cEnvironment_,
											  Interface::IFile* pFile_,
											  File::CheckOrderArgument& cArgument_);
		virtual Specification* choose(Opt::Environment& cEnvironment_,
									  const Order::ChooseArgument& cArgument_);
		virtual bool isKeyRetrievable(Opt::Environment& cEnvironment_,
									  Interface::IPredicate* pPredicate_);

	///////////////////////////////
	// Specification::
		virtual Specification::Size getKeySize() {return getSize();}
		virtual Order::Key* getKey(Specification::Position iPos_) {return getOperandi(iPos_);}
		virtual bool isRefering(Interface::IRelation* pRelation_,
								Specification::Position iPosition_)
		{
			Order::Key* pKey = getKey(iPosition_);
			return pKey && pKey->getScalar()->isRefering(pRelation_);
		}

	protected:
		// constructor
		GenericCheckedSpecificationImpl(Specification* pSpecification_,
										const Utility::FileSet& cFile_,
										typename Handle_::Argument cArgument_)
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
			: Super(cArgument_)
		{
			setArgument(pSpecification_,
						cFile_);
		}
#else
			: Super(pSpecification_,
					cFile_,
					cArgument_)
		{}
#endif
		virtual Specification* createSpecification(Opt::Environment& cEnvironment_,
												   const VECTOR<Order::Key*>& vecKey);
		virtual bool hasFunctionKey()
		{
			return isAny(boost::bind(&Key::isFunction,
									 _1));
		}
	private:
		// choose one file for ordering
		bool chooseFile(Opt::Environment& cEnvironment_,
						const Order::ChooseArgument& cArgument_,
						File::CheckOrderArgument& cCheckArgument_);
		// check one file for availability
		bool checkFile(Opt::Environment& cEnvironment_,
					   Candidate::Table* pTable_,
					   Interface::IFile* pFile_,
					   File::CheckOrderArgument& cCheckArgument_);
		// check key retrievable for a file
		bool isKeyRetrievable(Opt::Environment& cEnvironment_,
							  Predicate::ChosenInterface* pChosen_,
							  Interface::IFile* pFile_);
		// check key retrievable for a file and a key
		bool isKeyRetrievable(Opt::Environment& cEnvironment_,
							  Predicate::ChosenInterface* pChosen_,
							  Interface::IFile* pFile_,
							  Order::Key* pKey_);
	};

	// CLASS
	//	Order::Impl::MonadicCheckedSpecificationImpl
	//
	// NOTES

	class MonadicCheckedSpecificationImpl
		: public GenericCheckedSpecificationImpl< Tree::Monadic<CheckedSpecification, Order::Key> >
	{
	public:
		typedef MonadicCheckedSpecificationImpl This;
		typedef GenericCheckedSpecificationImpl< Tree::Monadic<CheckedSpecification, Order::Key> > Super;

		// constructor
		MonadicCheckedSpecificationImpl(Specification* pSpecification_,
										const Utility::FileSet& cFile_,
										Order::Key* pKey_)
			: Super(pSpecification_,
					cFile_,
					pKey_)
		{}
		// destructor
		virtual ~MonadicCheckedSpecificationImpl() {}
		

	///////////////////////////////
	// CheckedSpecification::

	///////////////////////////////
	// Specification::
		virtual int generateKey(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Execution::Interface::IIterator* pIterator_,
								Candidate::AdoptArgument& cArgument_)
		{
			VECTOR<int> vecID;
			vecID.PUSHBACK(getOperand()->getScalar()->generate(cEnvironment_,
															   cProgram_,
															   pIterator_,
															   cArgument_));
			return cProgram_.addVariable(vecID);
		}

	protected:
	private:
	///////////////////////////
	// Order::Specification::
		virtual void explainOperand(Opt::Environment* pEnvironment_,
									Opt::Explain& cExplain_);
	};


	// CLASS
	//	Order::Impl::MonadicCheckedSpecificationImpl
	//
	// NOTES
	class MonadicWithMonadicOptionCheckedSpecificationImpl
		: public Tree::MonadicOption< MonadicCheckedSpecificationImpl, Interface::IScalar>
	{
	public:
		typedef MonadicWithMonadicOptionCheckedSpecificationImpl This;
		typedef Tree::MonadicOption< MonadicCheckedSpecificationImpl, Interface::IScalar> Super;

		// constructor
		MonadicWithMonadicOptionCheckedSpecificationImpl(Specification* pSpecification_,
														 const Utility::FileSet& cFile_,
														 Order::Key* pKey_,
														 Interface::IScalar* pOption_)
			: Super(pSpecification_,
					cFile_,
					pKey_,
					pOption_)

		{}
		// destructor
		~MonadicWithMonadicOptionCheckedSpecificationImpl() {}
		
		///////////////////////////
		// Order::GenericCheckedSpecificationImpl::
		virtual Specification* createSpecification(Opt::Environment& cEnvironment_,
										 const VECTOR<Order::Key*>& vecKey_);

		
		///////////////////////////
		// Order::Specification::
		virtual bool isGroupBy() {return true;}

		
	private:

		///////////////////////////
		// Order::Specification::

		virtual void explainOption(Opt::Environment* pEnvironment_,
									Opt::Explain& cExplain_);
	};
	

	// CLASS local
	//	Order::Impl::NadicCheckedSpecificationImpl
	//
	// NOTES

	class NadicCheckedSpecificationImpl
		: public GenericCheckedSpecificationImpl< Tree::Nadic<CheckedSpecification, Order::Key> >
	{
	public:
		typedef NadicCheckedSpecificationImpl This;
		typedef GenericCheckedSpecificationImpl< Tree::Nadic<CheckedSpecification, Order::Key> > Super;

		// constructor
		NadicCheckedSpecificationImpl(Specification* pSpecification_,
									  const Utility::FileSet& cFile_,
									  Super::Argument vecKey_)
			: Super(pSpecification_,
					cFile_,
					vecKey_)
		{}
		// destructor
		~NadicCheckedSpecificationImpl() {}

	///////////////////////////////
	// CheckedSpecification::

	///////////////////////////////
	// Specification::
		virtual int generateKey(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Execution::Interface::IIterator* pIterator_,
								Candidate::AdoptArgument& cArgument_)
		{
			VECTOR<int> vecID;
			Opt::MapContainer(begin(), end(),
							  vecID,
							  boost::bind(&Interface::IScalar::generate,
										  boost::bind(&Order::Key::getScalar,
													  _1),
										  boost::ref(cEnvironment_),
										  boost::ref(cProgram_),
										  pIterator_,
										  boost::ref(cArgument_)));
			return cProgram_.addVariable(vecID);
		}

	protected:
	private:
	///////////////////////////
	// Order::Specification::
		virtual void explainOperand(Opt::Environment* pEnvironment_,
									Opt::Explain& cExplain_);
	};
} // namespace

////////////////////////////////////////////////
// Order::Impl::GenericCheckedSpecificationImpl

// TEMPLATE FUNCTION public
//	Order::Impl::GenericCheckedSpecificationImpl<Handle_>::createCheckOrderArgument --
//		create argument for checking file
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	File::CheckOrderArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
template <class Handle_>
bool
Impl::GenericCheckedSpecificationImpl<Handle_>::
createCheckOrderArgument(Opt::Environment& cEnvironment_,
 						 Interface::IFile* pFile_,
						 File::CheckOrderArgument& cArgument_)
{
	int n = getKeySize();

	Order::Direction::Value eDirection = Order::Direction::Unknown;
	bool bNeedCreate = false;

	// check keys whether field in the file is used
	int iKey = 0;
	for (; iKey < n; ++iKey) {
		Order::Key* pKey = getKey(iKey);
		if (pKey->getScalar()->isField() == false
			|| pKey->getScalar()->hasField(pFile_) == false) {
			break;
		}
		if (pKey->getDirection() != Order::Direction::Unknown) {
			if (eDirection == Order::Direction::Unknown) {
				eDirection = pKey->getDirection();
			} else if (eDirection != pKey->getDirection()) {
				// incompatible direction
				break;
			}
		} else {
			// need create another key object
			bNeedCreate = true;
		}
	}

	if (iKey == 0 || iKey < cArgument_.m_iMaxKeySize) {
		return false;
	}

	bNeedCreate = bNeedCreate || (iKey < n);

	if (eDirection == Order::Direction::Unknown) {
		eDirection = Order::Direction::Ascending;
	}

	Order::Specification* pOrder = this;

	if (bNeedCreate) {
		// scan over key and create key again
		VECTOR<Order::Key*> vecKey;
		vecKey.reserve(iKey);

		for (int i = 0; i < iKey; ++i) {
			Order::Key* pKey = getKey(i);
			if (pKey->getDirection() == Order::Direction::Unknown) {
				// create key using specified order direction
				pKey = Order::Key::create(cEnvironment_,
										  pKey->getScalar(),
										  eDirection);
			}
			vecKey.PUSHBACK(pKey);
		}
		pOrder = createSpecification(cEnvironment_, vecKey);
		
		if (isBitSetSort()) pOrder->setBitSetSort();
	}
	cArgument_.m_pOrder = pOrder;
	cArgument_.m_iMaxKeySize = iKey;
	return true;
}

// TEMPLATE FUNCTION public
//	Order::Impl::GenericCheckedSpecificationImpl<Handle_>::choose -- choose index
//
// TEMPLATE ARGUMENTS
//	class Handle_
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Order::ChooseArgument& cArgument_
//	
// RETURN
//	Specification*
//
// EXCEPTIONS

//virtual
template <class Handle_>
Specification*
Impl::GenericCheckedSpecificationImpl<Handle_>::
choose(Opt::Environment& cEnvironment_,
	   const Order::ChooseArgument& cArgument_)
{
	if (getFile().isEmpty()) {
		// no index files can be used
		return 0;
	}
	File::CheckOrderArgument cCheckArgument(cArgument_.m_pTable);
	cCheckArgument.skipEstimate();
	Candidate::File* pChosenFile = 0;
	bool bCanPartition = false;

	if (cArgument_.m_pPredicate
		&& cArgument_.m_pPredicate->isChosen()) {
		// check availability using index for predicates
		pChosenFile = cArgument_.m_pPredicate->getChosen()->chooseOrder(cEnvironment_,
																		this,
																		cCheckArgument);
		if (pChosenFile) {
			; _SYDNEY_ASSERT(cCheckArgument.m_pPredicate->isChosen());
			bCanPartition = pChosenFile->checkPartitionBy(cEnvironment_,
														  this);
			cCheckArgument.m_pPredicate->getChosen()->setFile(cEnvironment_,
															  pChosenFile);
		}
		if (hasAlternativeValue(cEnvironment_)
			&& cArgument_.m_cLimit.isSpecified()) {
			AccessPlan::Limit cLimit(cArgument_.m_cLimit);
			cLimit.setIntermediate();
			cArgument_.m_pPredicate->getChosen()->checkLimit(cEnvironment_,
															 cLimit);
		}
	}
	if (pChosenFile == 0) {
		// check availability using index for scanning
		if (chooseFile(cEnvironment_, cArgument_, cCheckArgument)) {
			pChosenFile = cCheckArgument.m_pFile->createCandidate(cEnvironment_,
																  cArgument_.m_pTable,
																  cCheckArgument.m_pParameter);
		}
	}
	if (pChosenFile) {
		// remember schema file when table is updated
		pChosenFile->setForUpdate(cEnvironment_);

		; _SYDNEY_ASSERT(cCheckArgument.m_pOrder && cCheckArgument.m_pOrder->getKeySize() > 0);
		ChosenSpecification* pResult =
			ChosenSpecification::create(cEnvironment_,
										cCheckArgument.m_pPredicate ? 0 : pChosenFile,
										cCheckArgument.m_pPredicate,
										cCheckArgument.m_pOrder,
										getSpecification());
		if (pResult->isPartial()) {
			pResult->require(cEnvironment_,
							 cArgument_.m_pTable);
		}
		if (bCanPartition) {
			pResult->setPartitionKey(getPartitionKey());
		}
		return pResult;
	}
	require(cEnvironment_,
			cArgument_.m_pTable);
	return 0;
}

// TEMPLATE FUNCTION public
//	Order::Impl::GenericCheckedSpecificationImpl<Handle_>::isKeyRetrievable -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
template <class Handle_>
bool
Impl::GenericCheckedSpecificationImpl<Handle_>::
isKeyRetrievable(Opt::Environment& cEnvironment_,
				 Interface::IPredicate* pPredicate_)
{
	; _SYDNEY_ASSERT(pPredicate_->isChosen());

	Predicate::ChosenInterface* pChosen = pPredicate_->getChosen();
	return getFile().isAny(boost::bind(&This::isKeyRetrievable,
									   this,
									   boost::ref(cEnvironment_),
									   pChosen,
									   _1));
}

// TEMPLATE FUNCTION private
//	Order::Impl::GenericCheckedSpecificationImpl<Handle_>::chooseFile -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Order::ChooseArgument& cArgument_
//	File::CheckOrderArgument& cCheckArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
template <class Handle_>
Specification*
Impl::GenericCheckedSpecificationImpl<Handle_>::
createSpecification(Opt::Environment& cEnvironment_,
					const VECTOR<Order::Key*>& vecKey_)
{
	return Order::Specification::create(cEnvironment_, vecKey_, false);
}



// TEMPLATE FUNCTION private
//	Order::Impl::GenericCheckedSpecificationImpl<Handle_>::chooseFile -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Order::ChooseArgument& cArgument_
//	File::CheckOrderArgument& cCheckArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
template <class Handle_>
bool
Impl::GenericCheckedSpecificationImpl<Handle_>::
chooseFile(Opt::Environment& cEnvironment_,
		   const Order::ChooseArgument& cArgument_,
		   File::CheckOrderArgument& cCheckArgument_)
{
	if (getFile().getSize() <= 1) {
		cCheckArgument_.skipEstimate();
	}

	Utility::FileSet::ConstIterator min =
		Opt::FindLast(getFile().begin(),
					  getFile().end(),
					  boost::bind(
							  &This::checkFile,
							  this,
							  boost::ref(cEnvironment_),
							  cArgument_.m_pTable,
							  _1,
							  boost::ref(cCheckArgument_)));
	if (min != getFile().end()) {
		// 'min' points to the file to be used
		return true;
	}
	// no index files can be used
	return false;
}

// TEMPLATE FUNCTION private
//	Order::Impl::GenericCheckedSpecificationImpl<Handle_>::checkFile -- check one file for availability
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::Table* pTable_
//	Interface::IFile* pFile_
//	File::CheckOrderArgument& cCheckArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

template <class Handle_>
bool
Impl::GenericCheckedSpecificationImpl<Handle_>::
checkFile(Opt::Environment& cEnvironment_,
		  Candidate::Table* pTable_,
		  Interface::IFile* pFile_,
		  File::CheckOrderArgument& cCheckArgument_)
{
	File::CheckOrderArgument cMyCheckArgument(pTable_);
	if (createCheckOrderArgument(cEnvironment_,
								 pFile_,
								 cMyCheckArgument)) {
		cMyCheckArgument.skipEstimate();
		// check whether file can scan
		if (pFile_->getSchemaFile()->isAbleToScan(pFile_->hasAllTuples(cEnvironment_,
																	   pTable_->getTable()))
			&& pTable_->checkFile(cEnvironment_,
								  pFile_,
								  0,
								  cMyCheckArgument,
								  AccessPlan::Cost(),
								  boost::bind(&LogicalFile::AutoLogicalFile::getSearchParameter,
											  _1,
											  static_cast<const LogicalFile::TreeNodeInterface*>(0),
											  _2))
			&& pTable_->checkFile(cEnvironment_,
								  pFile_,
								  cMyCheckArgument.m_pParameter,
								  cMyCheckArgument,
								  AccessPlan::Cost(),
								  boost::bind(&LogicalFile::AutoLogicalFile::getSortParameter,
											  _1,
											  cMyCheckArgument.m_pOrder,
											  _2))) {
			cCheckArgument_ = cMyCheckArgument;
			cCheckArgument_.m_pParameter->setOrder(this);
			return true;
		} else if(isGroupBy()
				  && pFile_->getSchemaFile()->isAbleToBitSetSort()
				  && pTable_->checkFile(cEnvironment_,
										pFile_,
										0,
										cMyCheckArgument,
										AccessPlan::Cost(),
										boost::bind(&LogicalFile::AutoLogicalFile::getSearchParameter,
													_1,
													static_cast<const LogicalFile::TreeNodeInterface*>(0),
													_2))
				  && pTable_->checkFile(cEnvironment_,
										pFile_,
										cMyCheckArgument.m_pParameter,
										cMyCheckArgument,
										AccessPlan::Cost(),
										boost::bind(&LogicalFile::AutoLogicalFile::getSortParameter,
													_1,
													cMyCheckArgument.m_pOrder,
													_2))
			) {

			cCheckArgument_ = cMyCheckArgument;
			cCheckArgument_.m_pOrder->setBitSetSort();
			cCheckArgument_.m_pParameter->setOrder(this);
			return true;
		}
	}
	return false;
}

// TEMPLATE FUNCTION private
//	Order::Impl::GenericCheckedSpecificationImpl<Handle_>::isKeyRetrievable -- check key retrievable for a file
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Predicate::ChosenInterface* pChosen_
//	Interface::IFile* pFile_
//	
// RETURN
//	bool
//
// EXCEPTIONS

template <class Handle_>
bool
Impl::GenericCheckedSpecificationImpl<Handle_>::
isKeyRetrievable(Opt::Environment& cEnvironment_,
				 Predicate::ChosenInterface* pChosen_,
				 Interface::IFile* pFile_)
{
	return isAll(boost::bind(&This::isKeyRetrievable,
							 this,
							 boost::ref(cEnvironment_),
							 pChosen_,
							 pFile_,
							 _1));
}

// FUNCTION private
//	Order::Impl::GenericCheckedSpecificationImpl<Handle_>::isKeyRetrievable -- check key retrievable for a file and a key
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Predicate::ChosenInterface* pChosen_
//	Interface::IFile* pFile_
//	Order::Key* pKey_
//	
// RETURN
//	bool
//
// EXCEPTIONS

template <class Handle_>
bool
Impl::GenericCheckedSpecificationImpl<Handle_>::
isKeyRetrievable(Opt::Environment& cEnvironment_,
				 Predicate::ChosenInterface* pChosen_,
				 Interface::IFile* pFile_,
				 Order::Key* pKey_)
{
	Interface::IScalar* pScalar = pKey_->getScalar();
	if (pScalar->isField()) {
		Predicate::CheckRetrievableArgument cArgument(pScalar->getField());
		return pChosen_->isRetrievable(cEnvironment_,
									   pFile_,
									   cArgument);
	}
	return false;
}

////////////////////////////////////////////////
// Order::Impl::MonadicCheckedSpecificationImpl

// FUNCTION private
//	Order::Impl::MonadicCheckedSpecificationImpl::explainOperand -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::MonadicCheckedSpecificationImpl::
explainOperand(Opt::Environment* pEnvironment_,
			   Opt::Explain& cExplain_)
{
	getOperand()->explain(pEnvironment_, cExplain_);
}


////////////////////////////////////////////////
// Order::Impl::MonadicCheckedSpecificationImpl

// FUNCTION private
//	Order::Impl::MonadicCheckedSpecificationImpl::explainOperand -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::MonadicWithMonadicOptionCheckedSpecificationImpl::
explainOption(Opt::Environment* pEnvironment_,
			   Opt::Explain& cExplain_)
{

	if(getOption() != 0) {
		getOption()->explain(pEnvironment_, cExplain_);
	}

}


////////////////////////////////////////////////
// Order::Impl::MonadicCheckedSpecificationImpl

// FUNCTION private
//	Order::Impl::MonadicWithMonadicOptionCheckedSpecificationImpl::createSpecification
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	VECTOR<Order::Key*>& vecKey_
//	
// RETURN
//	Specification
//
// EXCEPTIONS

//virtual
Specification*
Impl::MonadicWithMonadicOptionCheckedSpecificationImpl::
createSpecification(Opt::Environment& cEnvironment_,
					const VECTOR<Order::Key*>& vecKey_)
{
	return Order::Specification::create(cEnvironment_, vecKey_, true);
}


////////////////////////////////////////////////
// Order::Impl::NadicCheckedSpecificationImpl

// FUNCTION private
//	Order::Impl::NadicCheckedSpecificationImpl::explainOperand -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::NadicCheckedSpecificationImpl::
explainOperand(Opt::Environment* pEnvironment_,
			   Opt::Explain& cExplain_)
{
	joinOperand(boost::bind(&Operand::explain,
							_1,
							pEnvironment_,
							boost::ref(cExplain_)),
				boost::bind(&Opt::Explain::putChar,
							&cExplain_,
							','));
}

//////////////////////////
// Order::CheckedSpecification

// FUNCTION public
//	Order::CheckedSpecification::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Specification* pSpecification_
//	Order::Key* pKey_
//	const Utility::FileSet& cFile_
//	
// RETURN
//	CheckedSpecification*
//
// EXCEPTIONS

CheckedSpecification*
CheckedSpecification::
create(Opt::Environment& cEnvironment_,
	   Specification* pSpecification_,
	   Order::Key* pKey_,
	   const Utility::FileSet& cFile_)
{
	AUTOPOINTER<This> pResult;
	if(pSpecification_->getOptionSize() == 0) {
		pResult = new Impl::MonadicCheckedSpecificationImpl(pSpecification_,
															cFile_,	
															pKey_);
	} else if(pSpecification_->getOptionSize() == 1) {
		if(pSpecification_->getOptionAt(0)->getType() == TreeNodeInterface::GroupBy) {
			pResult =
				new Impl::MonadicWithMonadicOptionCheckedSpecificationImpl(pSpecification_,
																		   cFile_,
																		   pKey_,
																		   pSpecification_->getGroupByOption());
		} else {
			_SYDNEY_ASSERT(0);
		}
	} else {
		_SYDNEY_ASSERT(0);
	}
	if (pSpecification_
		&& pSpecification_->isBitSetSort())		
		pResult->setBitSetSort();
	
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Order::CheckedSpecification::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Specification* pSpecification_
//	const VECTOR<Order::Key*>& vecKey_
//	const Utility::FileSet& cFile_
//	
// RETURN
//	CheckedSpecification*
//
// EXCEPTIONS

CheckedSpecification*
CheckedSpecification::
create(Opt::Environment& cEnvironment_,
	   Specification* pSpecification_,
	   const VECTOR<Order::Key*>& vecKey_,
	   const Utility::FileSet& cFile_)
{
	AUTOPOINTER<This> pResult =
		new Impl::NadicCheckedSpecificationImpl(pSpecification_,
												cFile_,
												vecKey_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

_SYDNEY_PLAN_ORDER_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
