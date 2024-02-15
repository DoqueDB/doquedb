// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// GetFactory.cpp -- Function to get instance of factory class
// 
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
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
}

#include "ModNlpUnaJp/Module.h"
#include "ModNlpUnaJp/ModNlpUnaJp.h"

_UNA_USING

#ifdef UNA_DLL

//
// FUNCTION global
//	getFactoryInstance -- Function to get instance of factory class
//
// NOTES
//	Function to get instance of factory class
//
// ARGUMENTS
//	none
//
// RETURN
//	ModNlpUnaJp::ModNlpUnaJp*
//		instance of factory class
//
extern "C"
UNA_UNAJP_FUNCTION
UNAJP::ModNlpUnaJp*
getFactoryInstance()
{
	return new UNAJP::ModNlpUnaJp();
}

#endif

//
//	Copyright (c) 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
