//
// ModNlpUnaJp.cpp -
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
#include "UnaDynamicCast.h"
#include "ModNlpUnaJp/Module.h"
#include "ModNlpUnaJp/ModNlpUnaJp.h"
#include "ModNlpUnaJp/ModNlpResourceUnaJp.h"
#include "ModNlpUnaJp/ModNlpAnalyzerUnaJp.h"


_UNA_USING
_UNA_UNAJP_USING

ModNlpUnaJp::ModNlpUnaJp()
{}

ModNlpUnaJp::~ModNlpUnaJp()
{}

ModNlpLocalResource* 
ModNlpUnaJp::createResourceObject()
{
  DEBUGPRINT("ModNlpUnaJp::createResourceObject\n",0);  
  return new ModNlpResourceUnaJp();
}

ModNlpLocalAnalyzer* 
ModNlpUnaJp::createAnalyzerObject(ModNlpAnalyzer *pAnalyzer_)
{
	DEBUGPRINT("ModNlpUnaJp::createAnalyzerObject\n",0);  
 	ModNlpLocalAnalyzer*p = new ModNlpAnalyzerUnaJp();
	p->setParent(pAnalyzer_);
	return p;
}

//
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
