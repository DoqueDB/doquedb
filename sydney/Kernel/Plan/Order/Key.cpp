// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Order/Key.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "SyDefault.h"

#include "Plan/Order/Key.h"
#include "Plan/Order/Argument.h"
#include "Plan/Order/CheckedKey.h"

#include "Plan/Interface/ICandidate.h"
#include "Plan/Interface/IFile.h"
#include "Plan/Scalar/Argument.h"
#include "Plan/Scalar/CheckedField.h"
#include "Plan/Scalar/Field.h"

#include "Common/Assert.h"

#include "DExecution/Action/StatementConstruction.h"

#include "Exception/Unexpected.h"

#include "Opt/Environment.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_ORDER_BEGIN

namespace
{
	const char* const _pszExplainDirection[] = 
	{
		" asc",
		" desc",
		"",
	};
}

namespace Impl
{
	// CLASS
	//	Order::Impl::KeyImpl --
	//
	// NOTES

	class KeyImpl
		: public Order::Key
	{
	public:
		typedef KeyImpl This;
		typedef Order::Key Super;

		// for getSortParameter
		class SortDirection
			: public Tree::Node
		{
		public:
			explicit SortDirection(Direction::Value eDirection_)
				: Tree::Node(Tree::Node::SortDirection),
				  m_cValue(eDirection_ == Direction::Descending ? "1" : "0")
			{}
		/////////////////
		// Tree::Node::
			virtual ModUnicodeString getValue() const {return m_cValue;}



		protected:
		private:
			ModUnicodeString m_cValue;
		};

		// costructor
		KeyImpl(Interface::IScalar* pScalar_,
				Direction::Value eDirection_)
			: Super(),
			  m_pScalar(pScalar_),
			  m_eDirection(eDirection_),
			  m_cOption(eDirection_)
		{}
		~KeyImpl() {}

	////////////////
	// Key::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Opt::Explain& cExplain_);
		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_)
		{m_pScalar->require(cEnvironment_, pCandidate_);}
		virtual Order::Key* check(Opt::Environment& cEnvironment_,
								  const CheckArgument& cArgument_);
		virtual Interface::IScalar* getScalar() {return m_pScalar;}
		virtual Direction::Value getDirection() {return m_eDirection;}

		virtual bool isFunction()
		{
			return m_pScalar->isField()
				&& m_pScalar->getField()->isFunction();
		}

	////////////////
	// Interface::ISqlNode			
		virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const;
		
		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec_,
								  const Plan::Sql::QueryArgument& cArgument_);		
							 

	////////////////
	// Tree::Node::
		virtual ModSize getOptionSize() const {return 1;}
		virtual const Tree::Node::Super* getOptionAt(ModInt32 iPosition_) const
		{
			return (iPosition_ == 0) ? &m_cOption : 0;
		}

		virtual ModSize getOperandSize() const {return 1;}
		virtual const Tree::Node::Super* getOperandAt(ModInt32 iPosition_) const
		{
			if (m_pScalar && m_pScalar->isField()) {
				return m_pScalar->getField()->getOriginal();
			}
			return m_pScalar;
		}

	protected:
	private:
		Interface::IScalar* m_pScalar;
		Direction::Value m_eDirection;
		SortDirection m_cOption;
	};

} // namespace

//////////////////////////
// Order::Impl::KeyImpl

// FUNCTION public
//	Order::Impl::KeyImpl::explain -- 
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
Impl::KeyImpl::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	getScalar()->explain(pEnvironment_, cExplain_);
	cExplain_.put(_pszExplainDirection[getDirection()]);
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Order::Impl::KeyImpl::check -- search for applicable index file
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	
// RETURN
//	Order::Key*
//
// EXCEPTIONS

//virtual
Order::Key*
Impl::KeyImpl::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	; _SYDNEY_ASSERT(Interface::IScalar::Check::isOn(getScalar()->check(cEnvironment_,
																		cArgument_),
													 Interface::IScalar::Check::Referred));

	Utility::FileSet cSortFile;
	if (getScalar()->isField()) {
		// search for sortable files
		Scalar::Field* pField = getScalar()->getField();
		if (pField->isFunction()) {
			// function field may be sorted by retrieved file
			Scalar::Field* pChecked = pField->checkRetrieve(cEnvironment_);
			cSortFile.merge(pChecked->getChecked()->getFileSet());
		} else if(cArgument_.m_bGrouping) {
			Scalar::Field::getGroupingFile(cEnvironment_,
										   Scalar::GetFileArgument(
											   getScalar(),
											   0,
											   cSortFile));
		} else {
			Scalar::Field::getSortFile(cEnvironment_,
									   Scalar::GetFileArgument(
										   getScalar(),
										   0,
										   cSortFile));
		}
	}

	if (cSortFile.isEmpty()) {
		return 0;
	}
	return CheckedKey::create(cEnvironment_,
							  this,
							  cSortFile);
}


// FUNCTION public
//	Order::Impl::KeyImpl::createSQL
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//
//	
// RETURN
//	STRING
//
// EXCEPTIONS

//virtual
STRING
Impl::KeyImpl::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	OSTRSTREAM cStream;
	cStream << m_pScalar->toSQLStatement(cEnvironment_, cArgument_);
	if (m_eDirection == Direction::Descending) {
		cStream <<" desc ";
	}
	return cStream.getString();
}


// FUNCTION public
//	Order::Impl::KeyImpl::setParameter
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//
//	
// RETURN
//	STRING
//
// EXCEPTIONS

//virtual
void
Impl::KeyImpl::setParameter(Opt::Environment& cEnvironment_,
							Execution::Interface::IProgram& cProgram_,
							Execution::Interface::IIterator* pIterator_,
							DExecution::Action::StatementConstruction& cExec_,
							const Plan::Sql::QueryArgument& cArgument_)
{
	m_pScalar->setParameter(cEnvironment_,
							cProgram_,
							pIterator_,
							cExec_,
							cArgument_);
	if (m_eDirection == Direction::Descending)  cExec_.append(" desc ");
}
//////////////////////////
// Plan::Order::Key

// FUNCTION public
//	Order::Key::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pScalar_
//	Direction::Value eDirection_ = Direction::Ascending
//	
// RETURN
//	Order::Key*
//
// EXCEPTIONS

//static
Order::Key*
Key::
create(Opt::Environment& cEnvironment_,
	   Interface::IScalar* pScalar_,
	   Direction::Value eDirection_ /* = Direction::Ascending */)
{
	AUTOPOINTER<This> pResult = new Impl::KeyImpl(pScalar_, eDirection_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Order::Key::erase -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	This* pKey_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Key::
erase(Opt::Environment& cEnvironment_,
	  This* pKey_)
{
	pKey_->eraseFromEnvironment(cEnvironment_);
	delete pKey_;
}

// FUNCTION public
//	Order::Key::compare -- is key compatible to another key?
//
// NOTES
//
// ARGUMENTS
//	Key* pKey1_
//	Key* pKey2_
//	
// RETURN
//	int
//		0 ... pKey1 and pKey2 is identical
//		1 ... pKey1 can be used instead of pKey2
//		2 ... pKey2 can be used instead of pKey1
//		-1... pKey1 and pKey2 is not compatible
//
// EXCEPTIONS

//static
int
Key::
compare(Key* pKey1_,
		Key* pKey2_)
{
	if (pKey1_->getScalar() == pKey2_->getScalar()) {
		if (pKey1_->getDirection() == pKey2_->getDirection()) {
			return 0;
		}
		if (pKey2_->getDirection() == Direction::Unknown) {
			return 1;
		}
		if (pKey1_->getDirection() == Direction::Unknown) {
			// use second argument
			return 2;
		}
	}
	return -1;
}

// FUNCTION public
//	Order::Key::isChecked -- is index is cheked
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

//virtual
bool
Key::
isChecked()
{
	return false;
}

// FUNCTION public
//	Order::Key::getChecked -- get checked interface
//
// NOTES
//
// ARGUMENTS
//	Nothing
//	
// RETURN
//	CheckedKey*
//
// EXCEPTIONS

//virtual
CheckedKey*
Key::
getChecked()
{
	// default: this is not called
	_SYDNEY_ASSERT(false);
	_SYDNEY_THROW0(Exception::Unexpected);
}

_SYDNEY_PLAN_ORDER_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
