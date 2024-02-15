// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Module.h -- MACRO difinition to write source code of ENSTEM
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

#ifndef	__UNA_ENSTEM_MODULE_H
#define	__UNA_ENSTEM_MODULE_H

#include "UnaNameSpace.h"
#include "UnaDLL.h"
#include "UnaAssert.h"

#define	_UNA_ENSTEM_BEGIN	namespace ENSTEM {
#define	_UNA_ENSTEM_END	}

#define _UNA_ENSTEM_USING	using namespace ENSTEM;

#if defined(UNA_DLL)
	#if defined(UNA_STEM_EXPORT_FUNCTION)
		#define UNA_STEM_FUNCTION UNA_EXPORT
	#else
		#define UNA_STEM_FUNCTION UNA_IMPORT
	#endif
#else
	#define UNA_STEM_FUNCTION
#endif

#endif // __UNA_ENSTEM_MODULE_H

//
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
