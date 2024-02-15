// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Parameter.h --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_PARAMETER_H
#define __SYDNEY_FULLTEXT2_PARAMETER_H

#include "FullText2/Module.h"
#include "Common/Configuration.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::ParameterInteger -- Integer用
//	FullText2::ParameterIntegerInRange -- Integer用
//
//	NOTES
//
typedef Common::Configuration::ParameterInteger			ParameterInteger;
typedef Common::Configuration::ParameterIntegerInRange	ParameterIntegerInRange;

//
//	CLASS
//	FullText2::ParameterInteger64 -- Integer64用
//
//	NOTES
//
typedef Common::Configuration::ParameterInteger64		ParameterInteger64;

//
//	CLASS
//	FullText2::ParameterString -- String用
//
//	NOTES
//
typedef Common::Configuration::ParameterString			ParameterString;

//
//	CLASS
//	FullText2::ParameterBoolean -- Boolean用
//
//	NOTES
//
typedef Common::Configuration::ParameterBoolean			ParameterBoolean;

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_PARAMETER_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
