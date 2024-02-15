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
#ifndef __UNA_MODNLPUNAJP_H
#define __UNA_MODNLPUNAJP_H

#include "ModNLPLocal.h"

_UNA_BEGIN
_UNA_UNAJP_BEGIN

//
// CLASS
//	ModNlpUnaJp -- Concrete Factory Class
//
// NOTES
//	The factory class to generate ModNlpAnalyzerUnaJp and ModNlpResourceUnaJp.
//
class ModNlpUnaJp: public ModNlpLocal
{
public:
//
// FUNCTION public
//	ModNlpUnaJp::ModNlpUnaJp -- constructor
//
// NOTES
//	default constructor of ModNlpUnaJp
//
// ARGUMENTS
//	none
//
// RETURN
//	none
//
// EXCEPTIONS
//	none
//
	ModNlpUnaJp();
//
// FUNCTION public
//	ModNlpUnaJp::~ModNlpUnaJp -- destructor
//
// NOTES
//	destructor of ModNlpUnaJp
//
// ARGUMENTS
//	none
//
// RETURN
//	none
//
// EXCEPTIONS
//	none
//
	virtual ~ModNlpUnaJp();
//
// FUNCTION public
//	ModNlpUnaJp::createResourceObject -- construct ModNlpLocalResource
//
// NOTES
//	construct ModNlpLocalResource
//
// ARGUMENTS
//	none
//
// RETURN
//	ModNlpLocalResource*
//		the address of ModNlpLocalResource
//
// EXCEPTIONS
//	exception of lower modules
//
  ModNlpLocalResource* createResourceObject();

//
// FUNCTION public
//	ModNlpUnaJp::createAnalyzerObject -- construct ModNlpLocalAnalyzer
//
// NOTES
//	construct ModNlpLocalAnalyzer
//
// ARGUMENTS
//	none
//
// RETURN
//	ModNlpLocalAnalyzer*
//		the address of ModNlpLocalAnalyzer
//
// EXCEPTIONS
//	exception of lower modules
//
	ModNlpLocalAnalyzer* createAnalyzerObject(ModNlpAnalyzer *pAnalyzer_);

};

_UNA_UNAJP_END
_UNA_END

#endif//__UNA_MODNLPUNAJP_H

//
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
