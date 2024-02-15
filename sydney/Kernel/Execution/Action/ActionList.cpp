// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/ActionList.cpp --
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
const char moduleName[] = "Execution::Action";
}

#include "boost/bind.hpp"

#include "SyDefault.h"

#include "Execution/Action/ActionList.h"
#include "Execution/Interface/IAction.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/Serialize.h"

#include "Exception/Unexpected.h"

#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

namespace
{
	class _ActionExplainer
	{
	public:
		_ActionExplainer(Opt::Environment* pEnvironment_,
						 Interface::IProgram& cProgram_,
						 Opt::Explain& cExplain_)
			: m_pEnvironment(pEnvironment_),
			  m_cProgram(cProgram_),
			  m_cExplain(cExplain_),
			  m_iLine(1) // 1-base
		{}

		void operator()(Interface::IAction* pAction_)
		{
			if (m_iLine > 1) m_cExplain.newLine(true);
			if (m_iLine < 1000) m_cExplain.putChar(' ');
			if (m_iLine < 100) m_cExplain.putChar(' ');
			if (m_iLine < 10) m_cExplain.putChar(' ');
			m_cExplain.put(m_iLine).put(":");
			m_cExplain.pushIndent();
			pAction_->explain(m_pEnvironment,
							  m_cProgram,
							  m_cExplain);
			m_cExplain.popIndent();
			++m_iLine;
		}
		void operator()(int iID_)
		{
			(*this)(m_cProgram.getAction(iID_));
		}

	private:
		Opt::Environment* m_pEnvironment;
		Interface::IProgram& m_cProgram;
		Opt::Explain& m_cExplain;
		int m_iLine;
	};
} // namespace

////////////////////////////////////
// Execution::Action::ActionList

// FUNCTION public
//	Action::ActionList::explain -- 
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
ActionList::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	if (!m_vecID.ISEMPTY()) {
		cExplain_.pushIndent();
		cExplain_.newLine(true /* force */);
		if (!m_vecAction.ISEMPTY()) { // initialized
			FOREACH(m_vecAction,
					_ActionExplainer(pEnvironment_,
									 cProgram_,
									 cExplain_));
		} else {
			FOREACH(m_vecID,
					_ActionExplainer(pEnvironment_,
									 cProgram_,
									 cExplain_));
		}
		cExplain_.popIndent(true /* force new line */);
	}
}

// FUNCTION public
//	Action::ActionList::initialize -- 
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
ActionList::
initialize(Interface::IProgram& cProgram_)
{
	// convert each action ID into action object
	if (m_vecAction.ISEMPTY()) {
		m_vecAction.reserve(m_vecID.GETSIZE());
		Opt::MapContainer(m_vecID, m_vecAction,
						  boost::bind(&Interface::IProgram::getAction,
									  &cProgram_,
									  _1));
		FOREACH(m_vecAction,
				boost::bind(&Interface::IAction::initialize,
							_1,
							boost::ref(cProgram_)));						  
	}
}

// FUNCTION public
//	Action::ActionList::terminate -- 
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
ActionList::
terminate(Interface::IProgram& cProgram_)
{
	// clear object list
	FOREACH(m_vecAction,
			boost::bind(&Interface::IAction::terminate,
						_1,
						boost::ref(cProgram_)));
	m_vecAction.clear();
}

// FUNCTION public
//	Action::ActionList::execute -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

Action::Status::Value
ActionList::
execute(Interface::IProgram& cProgram_)
{
	// call startUp
	Action::Status::Value eResult = startUp(cProgram_);
	if (eResult != Action::Status::Break) {
		// call execute method for each object in the object list
		VECTOR<Interface::IAction*>::ITERATOR iterator = m_vecAction.begin();
		const VECTOR<Interface::IAction*>::ITERATOR last = m_vecAction.end();
		for (; iterator != last;) {
			eResult = (*iterator)->execute(cProgram_, *this);
			switch (eResult) {
			case Action::Status::Success:
			case Action::Status::False:
				{
					break;
				}
			case Action::Status::Continue:
			case Action::Status::Break:
				{
					return eResult;
				}
			default:
				{
					_SYDNEY_THROW0(Exception::Unexpected);
				}
			}
			if (m_bSetNext) {
				m_bSetNext = false;
				if (m_iNext < 0)
					break;
				else
					iterator = m_vecAction.begin() + m_iNext;
			} else {
				++iterator;
			}
		}
		return Action::Status::Success;
	}
	return eResult;
}

// FUNCTION public
//	Action::ActionList::accumulate -- 
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
ActionList::
accumulate(Interface::IProgram& cProgram_)
{
	FOREACH(m_vecAction,
			boost::bind(&Interface::IAction::accumulate,
						_1,
						boost::ref(cProgram_),
						boost::ref(*this)));
}

// FUNCTION public
//	Action::ActionList::finish -- 
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
ActionList::
finish(Interface::IProgram& cProgram_)
{
	// call finish method for each object in the object list
	FOREACH(m_vecAction,
			boost::bind(&Interface::IAction::finish,
						_1,
						boost::ref(cProgram_)));
}

// FUNCTION public
//	Action::ActionList::setPointer -- 
//
// NOTES
//
// ARGUMENTS
//	int iPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ActionList::
setPointer(int iPosition_)
{
	if (iPosition_ >= 0 && iPosition_ > m_vecAction.GETSIZE()) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	m_bSetNext = true;
	m_iNext = iPosition_;
}

// FUNCTION public
//	Action::ActionList::serialize -- serialize
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
ActionList::
serialize(ModArchive& archiver_)
{
	Utility::SerializeValue(archiver_, m_vecID);
}

// FUNCTION private
//	Action::ActionList::startUp -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

Action::Status::Value
ActionList::
startUp(Interface::IProgram& cProgram_)
{
	// call startUp method for each object in the object list
	VECTOR<Interface::IAction*>::ITERATOR iterator = m_vecAction.begin();
	const VECTOR<Interface::IAction*>::ITERATOR last = m_vecAction.end();
	for (; iterator != last; ++iterator) {
		Action::Status::Value eResult = (*iterator)->startUp(cProgram_, *this);
		switch (eResult) {
		case Action::Status::Success:
		case Action::Status::False:
			{
				break;
			}
		case Action::Status::Continue:
		case Action::Status::Break:
			{
				return eResult;
			}
		default:
			{
				_SYDNEY_THROW0(Exception::Unexpected);
			}
		}
	}
	return Action::Status::Success;
}

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
