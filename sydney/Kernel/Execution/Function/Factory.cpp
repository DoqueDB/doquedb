// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Factory.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Function";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Function/Factory.h"

#include "Execution/Function/Arithmetic.h"
#include "Execution/Function/Cardinality.h"
#include "Execution/Function/Case.h"
#include "Execution/Function/CharJoin.h"
#include "Execution/Function/Choice.h"
#include "Execution/Function/Coalesce.h"
#include "Execution/Function/Concatenate.h"
#include "Execution/Function/Copy.h"
#include "Execution/Function/CurrentTimestamp.h"
#include "Execution/Function/ElementReference.h"
#include "Execution/Function/ExpandSynonym.h"
#include "Execution/Function/Length.h"
#include "Execution/Function/Normalize.h"
#include "Execution/Function/Overlay.h"
#include "Execution/Function/SubString.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_FUNCTION_BEGIN

// FUNCTION public
//	Function::Factory::create -- constructor: 0-arguments
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	LogicalFile::TreeNodeInterface::Type eType_
//	int iDataID_
//	
// RETURN
//	Interface::IAction*
//
// EXCEPTIONS

//static
Interface::IAction*
Factory::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   LogicalFile::TreeNodeInterface::Type eType_,
	   int iDataID_)
{
	switch (eType_) {
	case LogicalFile::TreeNodeInterface::CurrentTimestamp:
		{
			return CurrentTimestamp::create(cProgram_,
											pIterator_,
											iDataID_);
		}
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Function::Factory::create -- constructor: 1-argument
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	LogicalFile::TreeNodeInterface::Type eType_
//	int iOperandID_
//	int iDataID_
//	
// RETURN
//	Interface::IAction*
//
// EXCEPTIONS

//static 
Interface::IAction*
Factory::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   LogicalFile::TreeNodeInterface::Type eType_,
	   int iOperandID_,
	   int iDataID_)
{
	switch (eType_) {
	case LogicalFile::TreeNodeInterface::Negative:
	case LogicalFile::TreeNodeInterface::Absolute:
		{
			return Arithmetic::create(cProgram_,
									  pIterator_,
									  eType_,
									  iOperandID_,
									  iDataID_);
		}
	case LogicalFile::TreeNodeInterface::CharLength:
		{
			return Length::Char::create(cProgram_,
										pIterator_,
										iOperandID_,
										iDataID_);
		}
	case LogicalFile::TreeNodeInterface::OctetLength:
		{
			return Length::Octet::create(cProgram_,
										 pIterator_,
										 iOperandID_,
										 iDataID_);
		}
	case LogicalFile::TreeNodeInterface::Copy:
		{
			return Copy::create(cProgram_,
								pIterator_,
								iOperandID_,
								iDataID_);
		}
	case LogicalFile::TreeNodeInterface::Cardinality:
		{
			return Cardinality::create(cProgram_,
									   pIterator_,
									   iOperandID_,
									   iDataID_);
		}
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Function::Factory::create -- constructor: 2-arguments
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	LogicalFile::TreeNodeInterface::Type eType_
//	const PAIR<int
//	int>& cOperandID_
//	int iDataID_
//	
// RETURN
//	Interface::IAction*
//
// EXCEPTIONS

//static 
Interface::IAction*
Factory::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   LogicalFile::TreeNodeInterface::Type eType_,
	   const PAIR<int, int>& cOperandID_,
	   int iDataID_)
{
	switch (eType_) {
	case LogicalFile::TreeNodeInterface::Add:
	case LogicalFile::TreeNodeInterface::Subtract:
	case LogicalFile::TreeNodeInterface::Multiply:
	case LogicalFile::TreeNodeInterface::Divide:
	case LogicalFile::TreeNodeInterface::Modulus:
		{
			return Arithmetic::create(cProgram_,
									  pIterator_,
									  eType_,
									  cOperandID_.first,
									  cOperandID_.second,
									  iDataID_);
		}
	case LogicalFile::TreeNodeInterface::StringConcatenate:
		{
			return Concatenate::create(cProgram_,
									   pIterator_,
									   cOperandID_.first,
									   cOperandID_.second,
									   iDataID_);
		}
	case LogicalFile::TreeNodeInterface::Coalesce:
		{
			return Coalesce::create(cProgram_,
									pIterator_,
									cOperandID_.first,
									cOperandID_.second,
									iDataID_);
		}
	case LogicalFile::TreeNodeInterface::CoalesceDefault:
		{
			return Coalesce::Default::create(cProgram_,
											 pIterator_,
											 cOperandID_.first,
											 cOperandID_.second,
											 iDataID_);
		}
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Function::Factory::create -- constructor: N-arguments
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	LogicalFile::TreeNodeInterface::Type eType_
//	const VECTOR<int>& vecOperandID_
//	int iDataID_
//	
// RETURN
//	Interface::IAction*
//
// EXCEPTIONS

//static 
Interface::IAction*
Factory::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   LogicalFile::TreeNodeInterface::Type eType_,
	   const VECTOR<int>& vecOperandID_,
	   int iDataID_)
{
	switch (eType_) {
	case LogicalFile::TreeNodeInterface::GetMax:
		{
			return Choice::create(cProgram_,
								  pIterator_,
								  eType_,
								  vecOperandID_,
								  iDataID_);
		}
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Function::Factory::create -- constructor: 1-argument 1-option
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	LogicalFile::TreeNodeInterface::Type eType_
//	int iOperandID_
//	int iOptionID_
//	int iDataID_
//	
// RETURN
//	Interface::IAction*
//
// EXCEPTIONS

//static 
Interface::IAction*
Factory::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   LogicalFile::TreeNodeInterface::Type eType_,
	   int iOperandID_,
	   int iOptionID_,
	   int iDataID_)
{
	switch (eType_) {
	case LogicalFile::TreeNodeInterface::Normalize:
		{
			return Normalize::create(cProgram_,
									 pIterator_,
									 iOperandID_,
									 iOptionID_,
									 iDataID_);
		}
	case LogicalFile::TreeNodeInterface::ExpandSynonym:
		{
			return ExpandSynonym::create(cProgram_,
										 pIterator_,
										 iOperandID_,
										 iOptionID_,
										 iDataID_);
		}
	case LogicalFile::TreeNodeInterface::ElementReference:
		{
			return ElementReference::create(cProgram_,
											pIterator_,
											iOperandID_,
											iOptionID_,
											iDataID_);
		}
	case LogicalFile::TreeNodeInterface::CharJoin:
		{
			VECTOR<int> vecID(1);
			vecID[0] = iOperandID_;
			return CharJoin::create(cProgram_,
									pIterator_,
									vecID,
									iOptionID_,
									iDataID_);
		}
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Function::Factory::create -- constructor: 2-arguments 1-option
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	LogicalFile::TreeNodeInterface::Type eType_
//	const PAIR<int
//	int>& cOperandID_
//	int iOptionID_
//	int iDataID_
//	
// RETURN
//	Interface::IAction*
//
// EXCEPTIONS

//static 
Interface::IAction*
Factory::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   LogicalFile::TreeNodeInterface::Type eType_,
	   const PAIR<int, int>& cOperandID_,
	   int iOptionID_,
	   int iDataID_)
{
	switch (eType_) {
	case LogicalFile::TreeNodeInterface::CharJoin:
		{
			VECTOR<int> vecID(2);
			vecID[0] = cOperandID_.first;
			vecID[1] = cOperandID_.second;
			return CharJoin::create(cProgram_,
									pIterator_,
									vecID,
									iOptionID_,
									iDataID_);
		}
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Function::Factory::create -- constructor: N-arguments 1-option
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	LogicalFile::TreeNodeInterface::Type eType_
//	const VECTOR<int>& vecOperandID_
//	int iOptionID_
//	int iDataID_
//	
// RETURN
//	Interface::IAction*
//
// EXCEPTIONS

//static 
Interface::IAction*
Factory::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   LogicalFile::TreeNodeInterface::Type eType_,
	   const VECTOR<int>& vecOperandID_,
	   int iOptionID_,
	   int iDataID_)
{
	switch (eType_) {
	case LogicalFile::TreeNodeInterface::CharJoin:
		{
			return CharJoin::create(cProgram_,
									pIterator_,
									vecOperandID_,
									iOptionID_,
									iDataID_);
		}
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Function::Factory::create -- constructor: 1-argument 2-options
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	LogicalFile::TreeNodeInterface::Type eType_
//	int iOperandID_
//	const PAIR<int
//	int>& cOptionID_
//	int iDataID_
//	
// RETURN
//	Interface::IAction*
//
// EXCEPTIONS

//static 
Interface::IAction*
Factory::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   LogicalFile::TreeNodeInterface::Type eType_,
	   int iOperandID_,
	   const PAIR<int, int>& cOptionID_,
	   int iDataID_)
{
	switch (eType_) {
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Function::Factory::create -- constructor: 2-arguments 2-options
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	LogicalFile::TreeNodeInterface::Type eType_
//	const PAIR<int
//	int>& cOperandID_
//	const PAIR<int
//	int>& cOptionID_
//	int iDataID_
//	
// RETURN
//	Interface::IAction*
//
// EXCEPTIONS

//static 
Interface::IAction*
Factory::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   LogicalFile::TreeNodeInterface::Type eType_,
	   const PAIR<int, int>& cOperandID_,
	   const PAIR<int, int>& cOptionID_,
	   int iDataID_)
{
	switch (eType_) {
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Function::Factory::create -- constructor: N-arguments 2-options
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	LogicalFile::TreeNodeInterface::Type eType_
//	const VECTOR<int>& vecOperandID_
//	const PAIR<int
//	int>& cOptionID_
//	int iDataID_
//	
// RETURN
//	Interface::IAction*
//
// EXCEPTIONS

//static 
Interface::IAction*
Factory::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   LogicalFile::TreeNodeInterface::Type eType_,
	   const VECTOR<int>& vecOperandID_,
	   const PAIR<int, int>& cOptionID_,
	   int iDataID_)
{
	switch (eType_) {
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Function::Factory::create -- constructor: 1-argument N-options
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	LogicalFile::TreeNodeInterface::Type eType_
//	int iOperandID_
//	const VECTOR<int>& vecOptionID_
//	int iDataID_
//	
// RETURN
//	Interface::IAction*
//
// EXCEPTIONS

//static 
Interface::IAction*
Factory::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   LogicalFile::TreeNodeInterface::Type eType_,
	   int iOperandID_,
	   const VECTOR<int>& vecOptionID_,
	   int iDataID_)
{
	switch (eType_) {
	case LogicalFile::TreeNodeInterface::SubString:
		{
			SIZE n = vecOptionID_.GETSIZE();
			; _SYDNEY_ASSERT(n == 1 || n == 2);

			return SubString::create(cProgram_,
									 pIterator_,
									 iOperandID_,
									 vecOptionID_[0],
									 (n == 2) ? vecOptionID_[1] : -1,
									 iDataID_);
		}
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Function::Factory::create -- constructor: 2-arguments N-options
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	LogicalFile::TreeNodeInterface::Type eType_
//	const PAIR<int
//	int>& cOperandID_
//	const VECTOR<int>& vecOptionID_
//	int iDataID_
//	
// RETURN
//	Interface::IAction*
//
// EXCEPTIONS

//static 
Interface::IAction*
Factory::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   LogicalFile::TreeNodeInterface::Type eType_,
	   const PAIR<int, int>& cOperandID_,
	   const VECTOR<int>& vecOptionID_,
	   int iDataID_)
{
	switch (eType_) {
	case LogicalFile::TreeNodeInterface::Overlay:
		{
			SIZE n = vecOptionID_.GETSIZE();
			; _SYDNEY_ASSERT(n == 1 || n == 2);

			return Overlay::create(cProgram_,
								   pIterator_,
								   cOperandID_.first,
								   cOperandID_.second,
								   vecOptionID_[0],
								   (n == 2) ? vecOptionID_[1] : -1,
								   iDataID_);
		}
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Function::Factory::create -- constructor: N-arguments N-options
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	LogicalFile::TreeNodeInterface::Type eType_
//	const VECTOR<int>& vecOperandID_
//	const VECTOR<int>& vecOptionID_
//	int iDataID_
//	
// RETURN
//	Interface::IAction*
//
// EXCEPTIONS

//static 
Interface::IAction*
Factory::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   LogicalFile::TreeNodeInterface::Type eType_,
	   const VECTOR<int>& vecOperandID_,
	   const VECTOR<int>& vecOptionID_,
	   int iDataID_)
{
	switch (eType_) {
	case LogicalFile::TreeNodeInterface::Case:
		{
			return Case::create(cProgram_,
								pIterator_,
								vecOperandID_,
								vecOptionID_,
								iDataID_);
		}
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

_SYDNEY_EXECUTION_FUNCTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
