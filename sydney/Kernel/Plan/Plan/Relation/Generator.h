// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Generator.h --
// 
// Copyright (c) 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_RELATION_GENERATOR_H
#define __SYDNEY_PLAN_RELATION_GENERATOR_H

#include "Plan/Relation/Module.h"
#include "Plan/Declaration.h"

#include "Analysis/Declaration.h"

#include "Execution/Declaration.h"

#include "Opt/Declaration.h"

_SYDNEY_BEGIN

namespace Statement
{
	class Object;
}

_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

////////////////////////////////////
// CLASS
//	Plan::Relation::Generator -- generator class
//
// NOTES
//	Instance is not created
class Generator
{
public:
	static bool generate(Opt::Environment& cEnvironment_,
						 const Analysis::Interface::IAnalyzer* pAnalyzer_,
						 Statement::Object* pStatement_);
protected:
private:
	Generator(); // don't implement constructor
	~Generator();// don't implement destructor
};

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_RELATION_GENERATOR_H

//
//	Copyright (c) 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
