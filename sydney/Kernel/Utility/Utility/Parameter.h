// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Parameter.h --
// 
// Copyright (c) 2002, 2003, 2005, 2009, 2017, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_UTILITY_PARAMETER_H
#define __TRMEISTER_UTILITY_PARAMETER_H

#include "Utility/Module.h"

#include "Common/Configuration.h"

_TRMEISTER_BEGIN
_TRMEISTER_UTILITY_BEGIN

//
//	CLASS
//	Utility::ParameterInteger -- Integer用
//
//	NOTES
//
typedef Common::Configuration::ParameterInteger			ParameterInteger;
typedef Common::Configuration::ParameterIntegerInRange	ParameterIntegerInRange;

//
//	CLASS
//	Utility::ParameterString -- String用
//
//	NOTES
//
typedef Common::Configuration::ParameterString			ParameterString;

//
//	CLASS
//	Utility::ParameterBoolean -- Boolean用
//
//	NOTES
//
typedef Common::Configuration::ParameterBoolean			ParameterBoolean;

//
//	CLASS
//	Utility::ParameterMessage -- Message用
//
//	NOTES
//
typedef Common::Configuration::ParameterMessage			ParameterMessage;

_TRMEISTER_UTILITY_END
_TRMEISTER_END

#endif //__TRMEISTER_UTILITY_PARAMETER_H

//
//	Copyright (c) 2002, 2003, 2005, 2009, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
