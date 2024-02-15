// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Interface/IFile.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Interface";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Interface/IFile.h"
#include "Plan/File/SchemaFile.h"
#include "Plan/File/SessionVariable.h"

#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_INTERFACE_BEGIN

////////////////////////////////////
//	Plan::Interface::IFile

// FUNCTION public
//	Interface::IFile::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::File* pSchemaFile_
//	
// RETURN
//	IFile*
//
// EXCEPTIONS

IFile*
IFile::
create(Opt::Environment& cEnvironment_,
	   Schema::File* pSchemaFile_)
{
	if (This* pFile = cEnvironment_.getFile(pSchemaFile_)) {
		return pFile;
	} else {
		AUTOPOINTER<This> pResult = new File::SchemaFile(pSchemaFile_);
		pResult->registerToEnvironment(cEnvironment_);
		return pResult.release();
	}
}

// FUNCTION public
//	Interface::IFile::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pVariable_
//	
// RETURN
//	IFile*
//
// EXCEPTIONS

//static
IFile*
IFile::
create(Opt::Environment& cEnvironment_,
	   Interface::IScalar* pVariable_)
{
	if (This* pFile = cEnvironment_.getFile(pVariable_)) {
		return pFile;
	} else {
		AUTOPOINTER<This> pResult = new File::SessionVariable(pVariable_);
		pResult->registerToEnvironment(cEnvironment_);
		return pResult.release();
	}
}

// FUNCTION private
//	Interface::IFile::registerToEnvironment -- register to environment
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
IFile::
registerToEnvironment(Opt::Environment& cEnvironment_)
{
	// add as file
	cEnvironment_.addFile(this);
	// use super class
	Super::registerToEnvironment(cEnvironment_);
}

_SYDNEY_PLAN_INTERFACE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
