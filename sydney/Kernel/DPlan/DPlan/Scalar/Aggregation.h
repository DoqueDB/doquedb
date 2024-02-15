// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Aggregation.h --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_DPLAN_SCALAR_AGGREGATION_H
#define __SYDNEY_DPLAN_SCALAR_AGGREGATION_H

#include "DPlan/Scalar/Module.h"
#include "Plan/Scalar/Aggregation.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_SCALAR_BEGIN

////////////////////////////////////
//	CLASS
//	DPlan::Scalar::Aggregation -- Interface for aggregation
//
//	NOTES
//	This class is not created directly.
//	Implementation class of this interface is described in cpp file.
class Aggregation
{
public:

	// constructor
	static Plan::Scalar::Function* create(Opt::Environment& cEnvironment_,
										  Plan::Tree::Node::Type eOperator_,
										  Plan::Interface::IScalar* pOperand_,
										  const STRING& cstrName_);

	static Plan::Scalar::Function* create(Opt::Environment& cEnvironment_,
										  Plan::Tree::Node::Type eOperator_,
										  Plan::Interface::IScalar* pOperand_,
										  Plan::Interface::IScalar* pOption_,
										  const STRING& cstrName_);	
	
protected:
private:
	// never constructed;
	Aggregation();
	~Aggregation();
};


_SYDNEY_DPLAN_SCALAR_END
_SYDNEY_DPLAN_END
_SYDNEY_END

#endif // __SYDNEY_DPLAN_SCALAR_AGGREGATIONIMPL_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
