// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File/KwicOption.h --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_FILE_KWICOPTION_H
#define __SYDNEY_PLAN_FILE_KWICOPTION_H

#include "Plan/Declaration.h"

#include "Common/Object.h"

#include "Execution/Declaration.h"

#include "Opt/Algorithm.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_FILE_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::File::KwicOption -- 
//
//	NOTES
class KwicOption
	: public Common::Object
{
public:
	typedef Common::Object Super;
	typedef KwicOption This;

	// constructor
	static This* create(Opt::Environment& cEnvironment_);

	// destructor
	virtual ~KwicOption() {}

	// set scalar node to get kwic size specification
	virtual void setKwicSize(Interface::IScalar* pScalar_) = 0;
	// set scalar node to get language column data
	virtual void setLanguage(Interface::IScalar* pScalar_) = 0;

	// set index file
	virtual void setFile(Interface::IFile* pFile_) = 0;

	// get scalar node to get kwic size specification
	virtual Interface::IScalar* getKwicSize() = 0;
	// get scalar node to get language column data
	virtual Interface::IScalar* getLanguage() = 0;

	// get index file
	virtual Interface::IFile* getFile() = 0;

	// generate data to hold property key and value
	virtual const PAIR<int, int>&
					generateProperty(Execution::Interface::IProgram& cProgram_) = 0;

protected:
	// constructor
	KwicOption() : Super(), m_iID(-1) {}

private:
	// register to environment
	void registerToEnvironment(Opt::Environment& cEnvironment_);

	int m_iID;
};

_SYDNEY_PLAN_FILE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_FILE_KWICOPTION_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
