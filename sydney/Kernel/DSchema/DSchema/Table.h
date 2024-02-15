// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Table.h -- Declaration of classes concerning with distributed table
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_DSCHEMA_TABLE_H
#define	__SYDNEY_DSCHEMA_TABLE_H

#include "DSchema/Module.h"

_SYDNEY_BEGIN

namespace Trans
{
	class Transaction;
}
namespace Schema
{
	class Table;
}

_SYDNEY_DSCHEMA_BEGIN

class Table
{
public:
	static bool isEmpty(Trans::Transaction& cTrans_,
						const Schema::Table& cTable_);
protected:
private:
	// never instantiated
	Table();
	~Table();
};

_SYDNEY_DSCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_DSCHEMA_TABLE_H

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
