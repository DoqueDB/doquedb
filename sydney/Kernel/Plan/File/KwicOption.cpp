// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File/KwicOption.cpp --
// 
// Copyright (c) 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::File";
}

#include "SyDefault.h"

#include "Plan/File/KwicOption.h"

#include "Plan/AccessPlan/Cost.h"
#include "Plan/Utility/ObjectSet.h"

#include "Common/DataArrayData.h"

#include "Execution/Interface/IProgram.h"

#include "Opt/Algorithm.h"
#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_FILE_BEGIN

namespace Impl
{
	class KwicOptionImpl
		: public File::KwicOption
	{
	public:
		typedef File::KwicOption Super;
		typedef KwicOptionImpl This;

		// constructor
		KwicOptionImpl()
			: Super(),
			  m_pKwicSize(0),
			  m_pLanguage(0),
			  m_cProperty(-1,-1),
			  m_pFile(0)
		{}

	//////////////////
	// KwicOption::
		virtual void setKwicSize(Interface::IScalar* pScalar_) {m_pKwicSize = pScalar_;}
		virtual void setLanguage(Interface::IScalar* pScalar_) {m_pLanguage = pScalar_;}
		virtual void setFile(Interface::IFile* pFile_) {m_pFile = pFile_;}

		virtual Interface::IScalar* getKwicSize() {return m_pKwicSize;}
		virtual Interface::IScalar* getLanguage() {return m_pLanguage;}
		virtual Interface::IFile* getFile() {return m_pFile;}

		virtual const PAIR<int, int>&
							generateProperty(Execution::Interface::IProgram& cProgram_);

	protected:
	private:
		Interface::IScalar* m_pKwicSize;
		Interface::IScalar* m_pLanguage;
		PAIR<int, int> m_cProperty;

		Interface::IFile* m_pFile;
	};
}

/////////////////////////////////////
// File::Impl::KwicOptionImpl
/////////////////////////////////////

// FUNCTION public
//	File::Impl::KwicOptionImpl::generateProperty -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	const PAIR<int, int>&
//
// EXCEPTIONS

//virtual
const PAIR<int, int>&
Impl::KwicOptionImpl::
generateProperty(Execution::Interface::IProgram& cProgram_)
{
	if (m_cProperty.first < 0) {
		m_cProperty.first = cProgram_.addVariable(new Common::DataArrayData);
		m_cProperty.second = cProgram_.addVariable(new Common::DataArrayData);
	}
	return m_cProperty;
}

//////////////////////////////////
// File::KwicOption
//////////////////////////////////

// FUNCTION public
//	File::KwicOption::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	KwicOption*
//
// EXCEPTIONS

//static
KwicOption*
KwicOption::
create(Opt::Environment& cEnvironment_)
{
	AUTOPOINTER<This> pResult = new Impl::KwicOptionImpl;
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION private
//	File::KwicOption::registerToEnvironment -- register to environment
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

void
KwicOption::
registerToEnvironment(Opt::Environment& cEnvironment_)
{
	m_iID = cEnvironment_.addObject(this);
}

_SYDNEY_PLAN_FILE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
