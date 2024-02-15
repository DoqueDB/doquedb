// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SqlEncoder.h -- definition for explain plan tree
// 
// Copyright (c) 2007, 2008, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SQL_NODE_H
#define __SYDNEY_PLAN_SQL_NODE_H

#include "Plan/Sql/Module.h"

#include "Common/Object.h"

#include "Plan/Interface/ISqlNode.h"
#include "Plan/Tree/Node.h"
#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SQL_BEGIN

class Node
	: public Common::Object, public Plan::Interface::ISqlNode
{

public:
	
	// constructor
	Node()
		: Common::Object()
	{}
	
	virtual ~Node(){}

	static Node* createSimpleNode(Opt::Environment& cEnvironment_,
								  const STRING& cstrParam_);

	static Node* createSimpleNode(Opt::Environment& cEnvironment_,
								  int iDataID_);

	static Node* createArrayNode(Opt::Environment& cEnvironment_);


	static Node* createArrayPlaceHolderNode(Opt::Environment& cEnvironment_,
											int iDataID_);


	virtual void appendNode(Plan::Interface::ISqlNode* pNode_);


	virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
								  const Plan::Sql::QueryArgument& cArgument_) const;





	
private:
	void registerToEnvironment(Opt::Environment& cEnvironment_);

};

_SYDNEY_PLAN_SQL_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SQL_NODE_H

//
// Copyright (c) 2007, 2008, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
