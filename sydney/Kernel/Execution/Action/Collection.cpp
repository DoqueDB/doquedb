// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/Collection.cpp --
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
const char moduleName[] = "Execution::Action";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Action/Class.h"
#include "Execution/Action/Collection.h"
#include "Execution/Interface/ICollection.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

////////////////////////////////////
// Execution::Action::Collection

// FUNCTION public
//	Action::Collection::explain -- explain
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Collection::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	if (isInitialized()) {
		m_pCollection->explain(pEnvironment_, cProgram_, cExplain_);
	} else {
		cProgram_.getCollection(m_iCollectionID)->explain(pEnvironment_, cProgram_, cExplain_);
	}
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(":#").put(m_iCollectionID);
	}
}

// FUNCTION public
//	Action::Collection::initialize -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Collection::
initialize(Interface::IProgram& cProgram_)
{
	if (!isInitialized()) {
		m_pCollection = cProgram_.getCollection(m_iCollectionID);
		m_pCollection->initialize(cProgram_);
		m_cGetData.initialize(cProgram_);
		m_cPutData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Action::Collection::terminate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Collection::
terminate(Interface::IProgram& cProgram_)
{
	if (m_pCollection) {
		m_cGetData.terminate(cProgram_);
		m_cPutData.terminate(cProgram_);
		m_pCollection->terminate(cProgram_);
		m_pCollection = 0;
	}
}

// FUNCTION public
//	Action::Collection::explainGetData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Collection::
explainGetData(Interface::IProgram& cProgram_,
			   Opt::Explain& cExplain_)
{
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		if (m_cGetData.isValid()) {
			cExplain_.pushNoNewLine();
			cExplain_.put(" -> ");
			m_cGetData.explain(cProgram_, cExplain_);
			cExplain_.popNoNewLine();
		}
	}
}

// FUNCTION public
//	Action::Collection::explainPutData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Collection::
explainPutData(Interface::IProgram& cProgram_,
			   Opt::Explain& cExplain_)
{
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		if (m_cPutData.isValid()) {
			cExplain_.pushNoNewLine();
			cExplain_.put(" <- ");
			m_cPutData.explain(cProgram_, cExplain_);
			cExplain_.popNoNewLine();
		}
	}
}

// FUNCTION public
//	Action::Collection::prepareGetInterface -- for get
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
Collection::
prepareGetInterface()
{
	if ((m_pGet = m_pCollection->getGetInterface()) == 0) {
		// get interface is not available
		_SYDNEY_THROW0(Exception::NotSupported);
	}
}

// FUNCTION public
//	Action::Collection::get -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Collection::
get(Interface::IProgram& cProgram_)
{
	; _SYDNEY_ASSERT(m_pGet);
	return m_pGet->getData(cProgram_, m_cGetData.get());
}

// FUNCTION public
//	Action::Collection::get -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	int iPosition_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Collection::
get(Interface::IProgram& cProgram_, int iPosition_)
{
	; _SYDNEY_ASSERT(m_pGet);
	return m_pGet->getData(cProgram_, m_cGetData.get(), iPosition_);
}

// FUNCTION public
//	Action::Collection::reset -- 
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
Collection::
reset()
{
	; _SYDNEY_ASSERT(m_pGet);
	return m_pGet->reset();
}

// FUNCTION public
//	Action::Collection::preparePutInterface -- for put
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
Collection::
preparePutInterface()
{
	if ((m_pPut = m_pCollection->getPutInterface()) == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
}

// FUNCTION public
//	Action::Collection::put -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Collection::
put(Interface::IProgram& cProgram_)
{
	; _SYDNEY_ASSERT(m_pPut);
	return m_pPut->putData(cProgram_, m_cPutData.getData());
}

// FUNCTION public
//	Action::Collection::getLastPosition -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
Collection::
getLastPosition()
{
	; _SYDNEY_ASSERT(m_pPut);
	return m_pPut->getLastPosition();
}

// FUNCTION public
//	Action::Collection::shift -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Collection::
shift(Interface::IProgram& cProgram_)
{
	; _SYDNEY_ASSERT(m_pPut);
	return m_pPut->shift(cProgram_);
}

// FUNCTION public
//	Action::Collection::flush -- 
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
Collection::
flush()
{
	; _SYDNEY_ASSERT(m_pPut);
	m_pPut->flush();
}

// FUNCTION public
//	Action::Collection::finish -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Collection::
finish(Interface::IProgram& cProgram_)
{
	if (m_pGet) m_pGet->finish(cProgram_);
	if (m_pPut) m_pPut->finish(cProgram_);
}

// FUNCTION public
//	Action::Collection::clear -- 
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
Collection::
clear()
{
	m_pCollection->clear();
}

// FUNCTION public
//	Action::Collection::isEmpty -- 
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
Collection::
isEmpty()
{
	return m_pCollection->isEmpty();
}

// FUNCTION public
//	Action::Collection::isEmptyGrouping -- 
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
Collection::
isEmptyGrouping()
{
	return m_pCollection->isEmptyGrouping();
}


// FUNCTION public
//	Action::Collection::isGetNextOperand ----
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
Collection::
isGetNextOperand()
{
	return m_pCollection->isGetNextOperand();
}


// FUNCTION public
//	Action::Collection::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Collection*
//
// EXCEPTIONS

//static
Collection*
Collection::
getInstance(int iCategory_)
{
	// never new-ed
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Action::Collection::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Collection::
serialize(ModArchive& archiver_)
{
	archiver_(m_iCollectionID);
	m_cGetData.serialize(archiver_);
	m_cPutData.serialize(archiver_);
}

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
