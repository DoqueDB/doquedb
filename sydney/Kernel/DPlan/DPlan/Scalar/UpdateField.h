// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/UpdateField.h --
// 
// Copyright (c) 2012, 2016, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_DPLAN_SCALAR_UPDATEFIELD_H
#define __SYDNEY_DPLAN_SCALAR_UPDATEFIELD_H

#include "DPlan/Scalar/Module.h"
#include "DPlan/Declaration.h"

#include "Plan/Scalar/UpdateField.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_SCALAR_BEGIN

////////////////////////////////////
//	CLASS
//	DPlan::Scalar::UpdateField -- Interface for schema field
//
//	NOTES
class UpdateField
	: public Plan::Scalar::UpdateField
{
public:
	typedef Plan::Scalar::UpdateField Super;
	typedef UpdateField This;

	// constructor
	static Super* create(Opt::Environment& cEnvironment_,
						 Plan::Scalar::Field* pField_,
						 Plan::Interface::IScalar* pInput_);
	static Super* create(Opt::Environment& cEnvironment_,
						 Plan::Scalar::Field* pField_,
						 const PAIR<Plan::Interface::IScalar*, Plan::Interface::IScalar*>& cInput_);

protected:
private:
	// never constructed
	UpdateField();
	~UpdateField();
};

_SYDNEY_DPLAN_SCALAR_END
_SYDNEY_DPLAN_END
_SYDNEY_END

#endif // __SYDNEY_DPLAN_SCALAR_UPDATEFIELD_H

//
//	Copyright (c) 2012, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
