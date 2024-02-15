// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Parallel/OpenMP.cpp --
// 
// Copyright (c) 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Parallel";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Parallel/Class.h"
#include "Execution/Parallel/OpenMP.h"
#include "Execution/Parallel/Program.h"

#include "Execution/Action/ActionList.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/Serialize.h"

#include "Common/Assert.h"

#include "Opt/Explain.h"

#include "Utility/OpenMP.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_PARALLEL_BEGIN

namespace Impl
{
	// CLASS local
	//	$$$::Impl::OpenMPImpl --
	//
	// NOTES

	class OpenMPImpl
		: public Parallel::OpenMP,
		  public _SYDNEY::Utility::OpenMP
	{
	public:
		typedef Parallel::OpenMP Super1;
		typedef _SYDNEY::Utility::OpenMP Super2;
		typedef OpenMPImpl This;

		// constructor
		OpenMPImpl()
			: Super1(),
			  Super2(),
			  m_vecJobList(),
			  m_iMaxList(0),
			  m_iMaxAssigned(0)
		{}
		// descructor
		~OpenMPImpl() {}

	///////////////////////////////
	// Interface::IParallel::
		virtual int addList(Interface::IProgram& cProgram_);
		virtual Action::ActionList& getList(Interface::IProgram& cProgram_);
		virtual void setReturnData(Interface::IProgram& cProgram_,
								   int iDataID_);

	///////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);
		virtual Action::Status::Value execute(Interface::IProgram& cProgram_,
											  Action::ActionList& cActionList_);
	//	virtual void accumulate(Interface::IProgram& cProgram_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);
	//	virtual void undone(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	/////////////////////////
	// Utility::OpenMP::
		virtual void prepare();
		virtual void parallel();
		virtual void dispose();

	protected:
	private:
		int getNextJob();

		struct JobInfo
			: private PAIR<Action::ActionList, Parallel::Program>
		{
			typedef JobInfo This;
			typedef PAIR<Action::ActionList, Parallel::Program> Super;

			JobInfo()
				: Super()
			{}
			JobInfo(const Action::ActionList& cActionList_)
				: Super(cActionList_, Parallel::Program())
			{}
			JobInfo(const JobInfo& cOther_)
				: Super(cOther_)
			{}

			Action::ActionList& getList()
			{
				return first;
			}

			void setProgram(Interface::IProgram* pProgram_)
			{
				second.setProgram(pProgram_);
			}
			void resetData()
			{
				second.resetData();
			}
			void returnData()
			{
				second.returnData();
			}
			void setReturnData(int iDataID_)
			{
				second.setReturnData(iDataID_);
			}
			void initialize()
			{
				first.initialize(second);
			}
			void terminate()
			{
				first.terminate(second);
			}
			void execute()
			{
				(void) first.execute(second);
			}
			void finish()
			{
				first.finish(second);
			}
			void serialize(ModArchive& archiver_)
			{
				first.serialize(archiver_);
				second.serialize(archiver_);
			}
		};

		VECTOR<JobInfo> m_vecJobList;

		int m_iMaxList;
		int m_iMaxAssigned;
	};
} // namespace Impl

///////////////////////////////
// $$$::Impl::OpenMPImpl
///////////////////////////////

// FUNCTION public
//	Parallel::Impl::OpenMPImpl::addList -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Impl::OpenMPImpl::
addList(Interface::IProgram& cProgram_)
{
	m_vecJobList.PUSHBACK(JobInfo());
	return m_vecJobList.GETSIZE() - 1;
}

// FUNCTION public
//	Parallel::Impl::OpenMPImpl::getList -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Action::ActionList&
//
// EXCEPTIONS

//virtual
Action::ActionList&
Impl::OpenMPImpl::
getList(Interface::IProgram& cProgram_)
{
	; _SYDNEY_ASSERT(m_vecJobList.ISEMPTY() == false);
	return m_vecJobList.GETBACK().getList();
}

// FUNCTION public
//	Parallel::Impl::OpenMPImpl::setReturnData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	int iDataID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::OpenMPImpl::
setReturnData(Interface::IProgram& cProgram_,
			  int iDataID_)
{
	; _SYDNEY_ASSERT(m_vecJobList.ISEMPTY() == false);
	return m_vecJobList.GETBACK().setReturnData(iDataID_);
}

// FUNCTION public
//	Parallel::Impl::OpenMPImpl::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::OpenMPImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put("parallel");
	int n = m_vecJobList.GETSIZE();
	for (int i = 0; i < n; ++i) {
		cExplain_.newLine(true).put("@").put(i);
		m_vecJobList[i].getList().explain(pEnvironment_,
										  cProgram_,
										  cExplain_);
	}
}

// FUNCTION public
//	Parallel::Impl::OpenMPImpl::initialize -- 
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
Impl::OpenMPImpl::
initialize(Interface::IProgram& cProgram_)
{
	FOREACH(m_vecJobList,
			boost::bind(&JobInfo::setProgram,
						_1,
						&cProgram_));

	FOREACH(m_vecJobList,
			boost::bind(&JobInfo::initialize,
						_1));
}

// FUNCTION public
//	Parallel::Impl::OpenMPImpl::terminate -- 
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
Impl::OpenMPImpl::
terminate(Interface::IProgram& cProgram_)
{
	FOREACH(m_vecJobList,
			boost::bind(&JobInfo::terminate,
						_1));
}

// FUNCTION public
//	Parallel::Impl::OpenMPImpl::execute -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

//virtual
Action::Status::Value
Impl::OpenMPImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	bool bParallel = (m_vecJobList.GETSIZE() > 1);
	{
		// run action list in openMP
		run(bParallel);
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Parallel::Impl::OpenMPImpl::finish -- 
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
Impl::OpenMPImpl::
finish(Interface::IProgram& cProgram_)
{
	FOREACH(m_vecJobList,
			boost::bind(&JobInfo::finish,
						_1));
}

// FUNCTION public
//	Parallel::Impl::OpenMPImpl::reset -- 
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
Impl::OpenMPImpl::
reset(Interface::IProgram& cProgram_)
{
	// do nothing
}

// FUNCTION public
//	Parallel::Impl::OpenMPImpl::getClassID -- 
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
Impl::OpenMPImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::OpenMP);
}

// FUNCTION public
//	Parallel::Impl::OpenMPImpl::serialize -- 
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
Impl::OpenMPImpl::
serialize(ModArchive& archiver_)
{
	Execution::Utility::SerializeObject(archiver_,
										m_vecJobList);
}

// FUNCTION public
//	Parallel::Impl::OpenMPImpl::prepare -- 
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

//virtual
void
Impl::OpenMPImpl::
prepare()
{
	m_iMaxAssigned = -1;
	m_iMaxList = m_vecJobList.GETSIZE() - 1;
	FOREACH(m_vecJobList,
			boost::bind(&JobInfo::resetData,
						_1));
}

// FUNCTION public
//	Parallel::Impl::OpenMPImpl::parallel -- 
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

//virtual
void
Impl::OpenMPImpl::
parallel()
{
	int i = getNextJob();
	while (i >= 0) {
		m_vecJobList[i].execute();
		i = getNextJob();
	}
}

// FUNCTION public
//	Parallel::Impl::OpenMPImpl::dispose -- 
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

//virtual
void
Impl::OpenMPImpl::
dispose()
{
	FOREACH(m_vecJobList,
			boost::bind(&JobInfo::returnData,
						_1));
}

// FUNCTION private
//	Parallel::Impl::OpenMPImpl::getNextJob -- 
//
// NOTES
//
// ARGUMENTS
//	int iThreadID_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
Impl::OpenMPImpl::
getNextJob()
{
	Os::AutoCriticalSection l(m_cLatch);
	if (m_iMaxAssigned < m_iMaxList) {
		return ++m_iMaxAssigned;
	}
	return -1;
}

////////////////////////////////////
// Execution::Parallel::OpenMP

// FUNCTION public
//	Parallel::OpenMP::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	
// RETURN
//	OpenMP*
//
// EXCEPTIONS

//static
OpenMP*
OpenMP::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_)
{
	AUTOPOINTER<This> pResult = new Impl::OpenMPImpl;
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();

}

// FUNCTION public
//	Parallel::OpenMP::getInstance -- 
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	OpenMP*
//
// EXCEPTIONS

//static
OpenMP*
OpenMP::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::OpenMP);
	return new Impl::OpenMPImpl;
}

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
