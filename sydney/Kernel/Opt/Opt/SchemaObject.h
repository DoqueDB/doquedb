// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SchemaObject.h -- schema object for validity check
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_OPT_SCHEMAOBJECT_H
#define __SYDNEY_OPT_SCHEMAOBJECT_H

#include "Opt/Module.h"

#include "Schema/Column.h"
#include "Schema/File.h"
#include "Schema/Table.h"

#include "ModArchive.h"

_SYDNEY_BEGIN

namespace Trans
{
	class Transaction;
}

_SYDNEY_OPT_BEGIN

// STRUCT
//	Opt::SchemaObject --
//
// NOTES

struct SchemaObject
{
	Schema::Object::ID::Value m_iID;
	Schema::Object::ID::Value m_iDatabaseID;
	Schema::Object::Timestamp m_iTimestamp;

	SchemaObject()
		: m_iID(Schema::Object::ID::Invalid),
		  m_iDatabaseID(Schema::Object::ID::Invalid),
		  m_iTimestamp(0)
	{}
	explicit SchemaObject(const Schema::Object& cObject_)
		: m_iID(cObject_.getID()),
		  m_iDatabaseID(cObject_.getDatabaseID()),
		  m_iTimestamp(cObject_.getTimestamp())
	{}

	// comparison operator for Map
	bool operator<(const SchemaObject& cObject_) const
	{
		return m_iDatabaseID < cObject_.m_iDatabaseID
			|| (m_iDatabaseID == cObject_.m_iDatabaseID
				&& (m_iID < cObject_.m_iID
					|| (m_iID == cObject_.m_iID
						&& m_iTimestamp < cObject_.m_iTimestamp)));
	}

	void serialize(ModArchive& archiver_)
	{
		archiver_(m_iID);
		archiver_(m_iDatabaseID);
		archiver_(m_iTimestamp);
	}

	bool isValidTable(Trans::Transaction* pTrans_)
	{
		return Schema::Table::isValid(m_iID, m_iDatabaseID, m_iTimestamp, *pTrans_);
	}
	bool isValidColumn(Trans::Transaction* pTrans_)
	{
		return Schema::Column::isValid(m_iID, m_iDatabaseID, m_iTimestamp, *pTrans_);
	}
	bool isValidFile(Trans::Transaction* pTrans_)
	{
		return Schema::File::isValid(m_iID, m_iDatabaseID, m_iTimestamp, *pTrans_);
	}
};

_SYDNEY_OPT_END
_SYDNEY_END

#endif // __SYDNEY_OPT_SCHEMAOBJECT_H

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
