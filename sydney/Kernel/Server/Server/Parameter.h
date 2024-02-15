// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Parameter.h --
// 
// Copyright (c) 2002, 2003, 2017, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SERVER_PARAMETER_H
#define __SYDNEY_SERVER_PARAMETER_H

#include "Server/Module.h"
#include "Common/Configuration.h"

_SYDNEY_BEGIN
_SYDNEY_SERVER_BEGIN

//
//	CLASS
//	Server::ParameterInteger -- Integer用
//
//	NOTES
//
typedef Common::Configuration::ParameterInteger			ParameterInteger;
typedef Common::Configuration::ParameterIntegerInRange	ParameterIntegerInRange;

//
//	CLASS
//	Server::ParameterString -- String用
//
//	NOTES
//
typedef Common::Configuration::ParameterString			ParameterString;

//
//	CLASS
//	Server::ParameterBoolean -- Boolean用
//
//	NOTES
//
typedef Common::Configuration::ParameterBoolean			ParameterBoolean;

//
//	CLASS
//	Server::ParameterMessage -- Message用
//
//	NOTES
//
typedef Common::Configuration::ParameterMessage			ParameterMessage;

_SYDNEY_SERVER_END
_SYDNEY_END

#endif //__SYDNEY_SERVER_PARAMETER_H

//
//	Copyright (c) 2002, 2003, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
