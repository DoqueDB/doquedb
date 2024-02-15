// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operator/SystemColumn.cpp --
// 
// Copyright (c) 2009, 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Execution::Operator";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Operator/SystemColumn.h"
#include "Execution/Operator/Class.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"
#include "Common/ColumnMetaData.h"
#include "Common/DataArrayData.h"
#include "Common/Integer64Data.h"
#include "Common/StringData.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "LogicalFile/AutoLogicalFile.h"
#include "LogicalFile/FileDriver.h"
#include "LogicalFile/FileDriverManager.h"

#include "Opt/Explain.h"

#include "Schema/Column.h"
#include "Schema/File.h"
#include "Schema/Function.h"
#include "Schema/Index.h"
#include "Schema/Partition.h"
#include "Schema/Privilege.h"

#include "Statement/FunctionDefinition.h"
#include "Statement/ParameterDeclarationList.h"
#include "Statement/ReturnsClause.h"
#include "Statement/RoutineBody.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_OPERATOR_BEGIN

namespace
{
	struct _Explain
	{
		enum {
			ColumnMetaData = 0,
			FileSize,
			IndexHint,
			PrivilegeFlag,
			PrivilegeObjectType,
			PrivilegeObjectID,
			PartitionCategory,
			FunctionRoutine,
			DatabasePath,
			DatabaseMasterURL,
		};
	};
	const char* const _pszExplainName[] =
	{
		"column meta data",
		"file size",
		"index hint",
		"privilege flag",
		"privilege object type",
		"privilege object",
		"partition category",
		"function routine",
		"database path",
		"database masterurl",
	};
}

namespace SystemColumnImpl
{
	// CLASS local
	//	Execution::Operator::Impl::Base -- base class of system column implementation class
	//
	// NOTES
	class Base
		: public Operator::SystemColumn
	{
	public:
		typedef Base This;
		typedef Operator::SystemColumn Super;

		Base()
			: Super(),
			  m_cRowID(),
			  m_cData()
		{}
		Base(int iRowIDID_,
			 int iDataID_)
			: Super(),
			  m_cRowID(iRowIDID_),
			  m_cData(iDataID_)
		{}
		virtual ~Base() {}

	///////////////////////////
	// Operator::SystemColumn::

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
	//	int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
		// explain operator
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_) = 0;
		// set target data
		virtual void setTarget(Common::Data::Pointer pData_) = 0;
		// function main
		virtual void setValue(Trans::Transaction& cTrans_,
							  Schema::Database* pDatabase_,
							  Schema::Object::ID::Value iID_) = 0;

		// variable data which is used as key
		Action::RowIDHolder m_cRowID;
		// variable of result data to which fetched data is assigned
		Action::DataHolder m_cData;
	};

	// CLASS local
	//	Execution::Operator::SystemColumnImpl::ColumnMetaData --
	//					implementation class of column meta data system column
	//
	// NOTES
	class ColumnMetaData
		: public Base
	{
	public:
		typedef ColumnMetaData This;
		typedef Base Super;

		ColumnMetaData()
			: Super(),
			  m_pData(0)
		{}
		ColumnMetaData(int iRowIDID_,
					   int iDataID_)
			: Super(iRowIDID_, iDataID_),
			  m_pData(0)
		{}
		~ColumnMetaData() {}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	///////////////////////////////
	// SystemColumnImpl::Base
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
		virtual void setTarget(Common::Data::Pointer pData_);
		virtual void setValue(Trans::Transaction& cTrans_,
							  Schema::Database* pDatabase_,
							  Schema::Object::ID::Value iID_);

		enum {ELEMENT_NUM = 7};
		Common::DataArrayData* m_pData;
	};

	// CLASS local
	//	Execution::Operator::SystemColumnImpl::FileSize -- implementation class of file size system column
	//
	// NOTES
	class FileSize
		: public Base
	{
	public:
		typedef FileSize This;
		typedef Base Super;

		FileSize()
			: Super(),
			  m_pData(0)
		{}
		FileSize(int iRowIDID_,
				 int iDataID_)
			: Super(iRowIDID_, iDataID_),
			  m_pData(0)
		{}
		~FileSize() {}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	///////////////////////////////
	// SystemColumnImpl::Base
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
		virtual void setTarget(Common::Data::Pointer pData_);
		virtual void setValue(Trans::Transaction& cTrans_,
							  Schema::Database* pDatabase_,
							  Schema::Object::ID::Value iID_);

		Common::Integer64Data* m_pData;
	};

	// CLASS local
	//	Execution::Operator::SystemColumnImpl::IndexHint -- implementation class of index hint system column
	//
	// NOTES
	class IndexHint
		: public Base
	{
	public:
		typedef IndexHint This;
		typedef Base Super;

		IndexHint()
			: Super(),
			  m_pData(0)
		{}
		IndexHint(int iRowIDID_,
				  int iDataID_)
			: Super(iRowIDID_, iDataID_),
			  m_pData(0)
		{}
		~IndexHint() {}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	///////////////////////////////
	// SystemColumnImpl::Base
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
		virtual void setTarget(Common::Data::Pointer pData_);
		virtual void setValue(Trans::Transaction& cTrans_,
							  Schema::Database* pDatabase_,
							  Schema::Object::ID::Value iID_);

		Common::StringData* m_pData;
	};

	// CLASS local
	//	Execution::Operator::SystemColumnImpl::PrivilegeFlag --
	//						implementation class of privilege flag system column
	//
	// NOTES
	class PrivilegeFlag
		: public Base
	{
	public:
		typedef PrivilegeFlag This;
		typedef Base Super;

		PrivilegeFlag()
			: Super(),
			  m_pData(0)
		{}
		PrivilegeFlag(int iRowIDID_,
					  int iDataID_)
			: Super(iRowIDID_, iDataID_),
			  m_pData(0)
		{}
		~PrivilegeFlag() {}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	///////////////////////////////
	// SystemColumnImpl::Base
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
		virtual void setTarget(Common::Data::Pointer pData_);
		virtual void setValue(Trans::Transaction& cTrans_,
							  Schema::Database* pDatabase_,
							  Schema::Object::ID::Value iID_);

		Common::DataArrayData* m_pData;
	};

	// CLASS local
	//	Execution::Operator::SystemColumnImpl::PrivilegeObjectType --
	//						implementation class of privilege objectType system column
	//
	// NOTES
	class PrivilegeObjectType
		: public Base
	{
	public:
		typedef PrivilegeObjectType This;
		typedef Base Super;

		PrivilegeObjectType()
			: Super(),
			  m_pData(0)
		{}
		PrivilegeObjectType(int iRowIDID_,
							int iDataID_)
			: Super(iRowIDID_, iDataID_),
			  m_pData(0)
		{}
		~PrivilegeObjectType() {}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	///////////////////////////////
	// SystemColumnImpl::Base
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
		virtual void setTarget(Common::Data::Pointer pData_);
		virtual void setValue(Trans::Transaction& cTrans_,
							  Schema::Database* pDatabase_,
							  Schema::Object::ID::Value iID_);

		Common::StringData* m_pData;
	};

	// CLASS local
	//	Execution::Operator::SystemColumnImpl::PrivilegeObjectID --
	//						implementation class of privilege objectID system column
	//
	// NOTES
	class PrivilegeObjectID
		: public Base
	{
	public:
		typedef PrivilegeObjectID This;
		typedef Base Super;

		PrivilegeObjectID()
			: Super(),
			  m_pData(0)
		{}
		PrivilegeObjectID(int iRowIDID_,
						  int iDataID_)
			: Super(iRowIDID_, iDataID_),
			  m_pData(0)
		{}
		~PrivilegeObjectID() {}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	///////////////////////////////
	// SystemColumnImpl::Base
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
		virtual void setTarget(Common::Data::Pointer pData_);
		virtual void setValue(Trans::Transaction& cTrans_,
							  Schema::Database* pDatabase_,
							  Schema::Object::ID::Value iID_);

		Common::StringData* m_pData;
	};

	// CLASS local
	//	Execution::Operator::SystemColumnImpl::PartitionCategory --
	//						implementation class of partition category system column
	//
	// NOTES
	class PartitionCategory
		: public Base
	{
	public:
		typedef PartitionCategory This;
		typedef Base Super;

		PartitionCategory()
			: Super(),
			  m_pData(0)
		{}
		PartitionCategory(int iRowIDID_,
						  int iDataID_)
			: Super(iRowIDID_, iDataID_),
			  m_pData(0)
		{}
		~PartitionCategory() {}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	///////////////////////////////
	// SystemColumnImpl::Base
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
		virtual void setTarget(Common::Data::Pointer pData_);
		virtual void setValue(Trans::Transaction& cTrans_,
							  Schema::Database* pDatabase_,
							  Schema::Object::ID::Value iID_);

		Common::StringData* m_pData;
	};

	// CLASS local
	//	Execution::Operator::SystemColumnImpl::FunctionRoutine --
	//						implementation class of function routine system column
	//
	// NOTES
	class FunctionRoutine
		: public Base
	{
	public:
		typedef FunctionRoutine This;
		typedef Base Super;

		FunctionRoutine()
			: Super(),
			  m_pData(0)
		{}
		FunctionRoutine(int iRowIDID_,
						   int iDataID_)
			: Super(iRowIDID_, iDataID_),
			  m_pData(0)
		{}
		~FunctionRoutine() {}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	///////////////////////////////
	// SystemColumnImpl::Base
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
		virtual void setTarget(Common::Data::Pointer pData_);
		virtual void setValue(Trans::Transaction& cTrans_,
							  Schema::Database* pDatabase_,
							  Schema::Object::ID::Value iID_);

		Common::StringData* m_pData;
	};

	// CLASS local
	//	Execution::Operator::SystemColumnImpl::DatabasePath --
	//						implementation class of database path system column
	//
	// NOTES
	class DatabasePath
		: public Base
	{
	public:
		typedef DatabasePath This;
		typedef Base Super;

		DatabasePath()
			: Super(),
			  m_pData(0)
		{}
		DatabasePath(int iRowIDID_,
					 int iDataID_)
			: Super(iRowIDID_, iDataID_),
			  m_pData(0)
		{}
		~DatabasePath() {}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	///////////////////////////////
	// SystemColumnImpl::Base
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
		virtual void setTarget(Common::Data::Pointer pData_);
		virtual void setValue(Trans::Transaction& cTrans_,
							  Schema::Database* pDatabase_,
							  Schema::Object::ID::Value iID_);

		Common::DataArrayData* m_pData;
	};

	// CLASS local
	//	Execution::Operator::SystemColumnImpl::DatabaseMasterURL --
	//						implementation class of database masterURL system column
	//
	// NOTES
	class DatabaseMasterURL
		: public Base
	{
	public:
		typedef DatabaseMasterURL This;
		typedef Base Super;

		DatabaseMasterURL()
			: Super(),
			  m_pData(0)
		{}
		DatabaseMasterURL(int iRowIDID_,
						  int iDataID_)
			: Super(iRowIDID_, iDataID_),
			  m_pData(0)
		{}
		~DatabaseMasterURL() {}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	///////////////////////////////
	// SystemColumnImpl::Base
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
		virtual void setTarget(Common::Data::Pointer pData_);
		virtual void setValue(Trans::Transaction& cTrans_,
							  Schema::Database* pDatabase_,
							  Schema::Object::ID::Value iID_);

		Common::StringData* m_pData;
	};
}

/////////////////////////////////////////////////////
// Execution::Operator::SystemColumnImpl::Base

// FUNCTION public
//	Operator::SystemColumnImpl::Base::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::Base::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put("calculate ");
	explainOperator(cProgram_, cExplain_);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" -> ");
		m_cData.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Operator::SystemColumnImpl::Base::initialize -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::Base::
initialize(Interface::IProgram& cProgram_)
{
	// convert ID to object
	if (m_cData.isInitialized() == false) {
		m_cRowID.initialize(cProgram_);
		m_cData.initialize(cProgram_);

		setTarget(m_cData.get());
	}
}

// FUNCTION public
//	Operator::SystemColumnImpl::Base::terminate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::Base::
terminate(Interface::IProgram& cProgram_)
{
	m_cRowID.terminate(cProgram_);
	m_cData.terminate(cProgram_);
}

// FUNCTION public
//	Operator::SystemColumnImpl::Base::execute -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

//virtual
Action::Status::Value
SystemColumnImpl::Base::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		setValue(*cProgram_.getTransaction(),
				 cProgram_.getDatabase(),
				 m_cRowID->getValue());
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::SystemColumnImpl::Base::finish -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::Base::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::SystemColumnImpl::Base::reset -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::Base::
reset(Interface::IProgram& cProgram_)
{
}

// FUNCTION public
//	Operator::SystemColumnImpl::Base::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
SystemColumnImpl::Base::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cRowID.serialize(archiver_);
	m_cData.serialize(archiver_);
}

///////////////////////////////////////////
// Operator::SystemColumnImpl::ColumnMetaData::

// FUNCTION public
//	Operator::SystemColumnImpl::ColumnMetaData::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
SystemColumnImpl::ColumnMetaData::
getClassID() const
{
	return Class::getClassID(Class::Category::ColumnMetaData);
}

// FUNCTION private
//	Operator::SystemColumnImpl::ColumnMetaData::explainOperator -- explain operator
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::ColumnMetaData::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Explain::ColumnMetaData]);
}

// FUNCTION private
//	Operator::SystemColumnImpl::ColumnMetaData::setTarget -- 
//
// NOTES
//
// ARGUMENTS
//	Common::Data::Pointer pData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::ColumnMetaData::
setTarget(Common::Data::Pointer pData_)
{
	m_pData = _SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pData_.get());
	; _SYDNEY_ASSERT(m_pData);

	m_pData->clear();
	m_pData->reserve(ELEMENT_NUM);

	for (int i = 0; i < ELEMENT_NUM; ++i) {
		m_pData->pushBack(new Common::StringData);
	}
}

// FUNCTION private
//	Operator::SystemColumnImpl::ColumnMetaData::setValue -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Schema::Database* pDatabase_
//	Schema::Object::ID::Value iID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::ColumnMetaData::
setValue(Trans::Transaction& cTrans_,
		 Schema::Database* pDatabase_,
		 Schema::Object::ID::Value iID_)
{
	// get column object
	Schema::Column* pSchemaColumn = Schema::Column::get(iID_, pDatabase_, cTrans_);
	if (pSchemaColumn == 0) {
		m_pData->setNull(true);
		return;
	}

	m_pData->setNull(false);

	// column data type
	const Schema::Column::DataType& cColumnType = pSchemaColumn->getType();
	// target ColumnMetaData
	Common::ColumnMetaData cMetaData(cColumnType);

	// Set following values in character string
	//   - SQL data type
	//   - type name
	//   - length or precision(numeric type)
	//   - scale(numeric type)
	//   - array element size(0: not array, -1: unlimited, others: array size)
	//   - default value
	//   - column hint
	; _SYDNEY_ASSERT(m_pData->getCount() >= ELEMENT_NUM);

	ModUnicodeOstrStream stream;
	Common::StringData* pData = 0;

	int i = 0;
	//   - SQL data type
	pData = _SYDNEY_DYNAMIC_CAST(Common::StringData*, m_pData->getElement(i++).get());
	stream << static_cast<int>(cMetaData.getType());
	pData->setValue(ModUnicodeString(stream.getString()));
	stream.clear();

	//   - type name
	pData = _SYDNEY_DYNAMIC_CAST(Common::StringData*, m_pData->getElement(i++).get());
	pData->setValue(cMetaData.getTypeName());

	//   - length or precision(numeric type)
	pData = _SYDNEY_DYNAMIC_CAST(Common::StringData*, m_pData->getElement(i++).get());
	if (cColumnType.getFlag() == Common::SQLData::Flag::Unlimited
		|| (cColumnType.getFlag() == Common::SQLData::Flag::Variable && cColumnType.getLength() == 0))
		stream << "-1";
	else
		stream << cColumnType.getLength();
	pData->setValue(ModUnicodeString(stream.getString()));
	stream.clear();

	//   - scale(numeric type)
	pData = _SYDNEY_DYNAMIC_CAST(Common::StringData*, m_pData->getElement(i++).get());
	stream << cColumnType.getScale();
	pData->setValue(ModUnicodeString(stream.getString()));
	stream.clear();

	//   - array element size(0: not array, -1: unlimited, others: array size)
	pData = _SYDNEY_DYNAMIC_CAST(Common::StringData*, m_pData->getElement(i++).get());
	stream << cColumnType.getMaxCardinality();
	pData->setValue(ModUnicodeString(stream.getString()));
	stream.clear();

	//   - default value
	const Schema::Default& cColumnDefault = pSchemaColumn->getDefault();
	pData = _SYDNEY_DYNAMIC_CAST(Common::StringData*, m_pData->getElement(i++).get());
	if (cColumnDefault.isNull()) {
		pData->setNull(true);
	} else {
		pData->setValue(cColumnDefault.toString());
	}

	//   - column hint
	const Schema::Hint* pHint = pSchemaColumn->getHint();
	pData = _SYDNEY_DYNAMIC_CAST(Common::StringData*, m_pData->getElement(i++).get());
	if (!pHint)
		pData->setNull(true);
	else {
		pData->setValue(pHint->getWholeString());
	}
}

///////////////////////////////////////////
// Operator::SystemColumnImpl::FileSize::

// FUNCTION public
//	Operator::SystemColumnImpl::FileSize::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
SystemColumnImpl::FileSize::
getClassID() const
{
	return Class::getClassID(Class::Category::FileSize);
}

// FUNCTION private
//	Operator::SystemColumnImpl::FileSize::explainOperator -- explain operator
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::FileSize::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Explain::FileSize]);
}

// FUNCTION private
//	Operator::SystemColumnImpl::FileSize::setTarget -- 
//
// NOTES
//
// ARGUMENTS
//	Common::Data::Pointer pData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::FileSize::
setTarget(Common::Data::Pointer pData_)
{
	m_pData = _SYDNEY_DYNAMIC_CAST(Common::Integer64Data*, pData_.get());
	; _SYDNEY_ASSERT(m_pData);
}

// FUNCTION private
//	Operator::SystemColumnImpl::FileSize::setValue -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Schema::Database* pDatabase_
//	Schema::Object::ID::Value iID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::FileSize::
setValue(Trans::Transaction& cTrans_,
		 Schema::Database* pDatabase_,
		 Schema::Object::ID::Value iID_)
{
	// get schema file object
	Schema::File* pSchemaFile = Schema::File::get(iID_, pDatabase_, cTrans_);
	if (pSchemaFile == 0) {
		m_pData->setNull(true);
		return;
	}

	LogicalFile::AutoLogicalFile cLogicalFile;

	LogicalFile::FileDriver* pDriver =
		LogicalFile::FileDriverManager::getDriver(pSchemaFile->getDriverID());
	; _SYDNEY_ASSERT(pDriver);

	(void)cLogicalFile.attach(*pDriver, pSchemaFile->getFileID());
	m_pData->setValue(cLogicalFile.getSize(cTrans_));
}

///////////////////////////////////////////
// Operator::SystemColumnImpl::IndexHint::

// FUNCTION public
//	Operator::SystemColumnImpl::IndexHint::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
SystemColumnImpl::IndexHint::
getClassID() const
{
	return Class::getClassID(Class::Category::IndexHint);
}

// FUNCTION private
//	Operator::SystemColumnImpl::IndexHint::explainOperator -- explain operator
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::IndexHint::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Explain::IndexHint]);
}

// FUNCTION private
//	Operator::SystemColumnImpl::IndexHint::setTarget -- 
//
// NOTES
//
// ARGUMENTS
//	Common::Data::Pointer pData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::IndexHint::
setTarget(Common::Data::Pointer pData_)
{
	m_pData = _SYDNEY_DYNAMIC_CAST(Common::StringData*, pData_.get());
	; _SYDNEY_ASSERT(m_pData);
}

// FUNCTION private
//	Operator::SystemColumnImpl::IndexHint::setValue -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Schema::Database* pDatabase_
//	Schema::Object::ID::Value iID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::IndexHint::
setValue(Trans::Transaction& cTrans_,
		 Schema::Database* pDatabase_,
		 Schema::Object::ID::Value iID_)
{
	// get schema index object
	Schema::Index* pSchemaIndex = Schema::Index::get(iID_, pDatabase_, cTrans_);
	if (pSchemaIndex == 0) {
		m_pData->setNull(true);
		return;
	}

	const Schema::Hint* pHint = pSchemaIndex->getHint();
	if (pHint == 0)
		m_pData->setNull(true);
	else
		m_pData->setValue(pHint->getWholeString());
}

///////////////////////////////////////////
// Operator::SystemColumnImpl::PrivilegeFlag::

// FUNCTION public
//	Operator::SystemColumnImpl::PrivilegeFlag::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
SystemColumnImpl::PrivilegeFlag::
getClassID() const
{
	return Class::getClassID(Class::Category::PrivilegeFlag);
}

// FUNCTION private
//	Operator::SystemColumnImpl::PrivilegeFlag::explainOperator -- explain operator
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::PrivilegeFlag::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Explain::PrivilegeFlag]);
}

// FUNCTION private
//	Operator::SystemColumnImpl::PrivilegeFlag::setTarget -- 
//
// NOTES
//
// ARGUMENTS
//	Common::Data::Pointer pData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::PrivilegeFlag::
setTarget(Common::Data::Pointer pData_)
{
	m_pData = _SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pData_.get());
	; _SYDNEY_ASSERT(m_pData);
}

// FUNCTION private
//	Operator::SystemColumnImpl::PrivilegeFlag::setValue -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Schema::Database* pDatabase_
//	Schema::Object::ID::Value iID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::PrivilegeFlag::
setValue(Trans::Transaction& cTrans_,
		 Schema::Database* pDatabase_,
		 Schema::Object::ID::Value iID_)
{
	// obtain privilege object using ID
	Schema::Privilege* pSchemaPrivilege = Schema::Privilege::get(iID_, pDatabase_, cTrans_);
	if (pSchemaPrivilege == 0) {
		m_pData->setNull(true);
		return;
	}

	// get privilege value
	const ModVector<Common::Privilege::Value>& vecValue = pSchemaPrivilege->getValue();
	ModSize nValue = vecValue.getSize();
	; _SYDNEY_ASSERT(nValue <= static_cast<ModSize>(Common::Privilege::Category::ValueNum));

	m_pData->clear();
	m_pData->reserve(nValue);
	for (ModSize i = 0; i < nValue; ++i) {
		if (vecValue[i] == Common::Privilege::All) {
			m_pData->pushBack(new Common::StringData(
								 Schema::Database::getBuiltInRoleName(
										  static_cast<Common::Privilege::Category::Value>(i))));
		} else if (vecValue[i] == Common::Privilege::None) {
			continue;
		} else {
			// for now, indivisual privilege can't be changed
			// so, privilege value should be All or None
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

//////////////////////////////////////////////////
// Operator::SystemColumnImpl::PrivilegeObjectType::

// FUNCTION public
//	Operator::SystemColumnImpl::PrivilegeObjectType::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
SystemColumnImpl::PrivilegeObjectType::
getClassID() const
{
	return Class::getClassID(Class::Category::PrivilegeObjectType);
}

// FUNCTION private
//	Operator::SystemColumnImpl::PrivilegeObjectType::explainOperator -- explain operator
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::PrivilegeObjectType::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Explain::PrivilegeObjectType]);
}

// FUNCTION private
//	Operator::SystemColumnImpl::PrivilegeObjectType::setTarget -- 
//
// NOTES
//
// ARGUMENTS
//	Common::Data::Pointer pData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::PrivilegeObjectType::
setTarget(Common::Data::Pointer pData_)
{
	m_pData = _SYDNEY_DYNAMIC_CAST(Common::StringData*, pData_.get());
	; _SYDNEY_ASSERT(m_pData);
}

// FUNCTION private
//	Operator::SystemColumnImpl::PrivilegeObjectType::setValue -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Schema::Database* pDatabase_
//	Schema::Object::ID::Value iID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::PrivilegeObjectType::
setValue(Trans::Transaction& cTrans_,
		 Schema::Database* pDatabase_,
		 Schema::Object::ID::Value iID_)
{
	// for now, always null
	m_pData->setNull();
}

///////////////////////////////////////////
// Operator::SystemColumnImpl::PrivilegeObjectID::

// FUNCTION public
//	Operator::SystemColumnImpl::PrivilegeObjectID::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
SystemColumnImpl::PrivilegeObjectID::
getClassID() const
{
	return Class::getClassID(Class::Category::PrivilegeObjectID);
}

// FUNCTION private
//	Operator::SystemColumnImpl::PrivilegeObjectID::explainOperator -- explain operator
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::PrivilegeObjectID::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Explain::PrivilegeObjectID]);
}

// FUNCTION private
//	Operator::SystemColumnImpl::PrivilegeObjectID::setTarget -- 
//
// NOTES
//
// ARGUMENTS
//	Common::Data::Pointer pData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::PrivilegeObjectID::
setTarget(Common::Data::Pointer pData_)
{
	m_pData = _SYDNEY_DYNAMIC_CAST(Common::StringData*, pData_.get());
	; _SYDNEY_ASSERT(m_pData);
}

// FUNCTION private
//	Operator::SystemColumnImpl::PrivilegeObjectID::setValue -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Schema::Database* pDatabase_
//	Schema::Object::ID::Value iID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::PrivilegeObjectID::
setValue(Trans::Transaction& cTrans_,
		 Schema::Database* pDatabase_,
		 Schema::Object::ID::Value iID_)
{
	// for now, always null
	m_pData->setNull();
}

///////////////////////////////////////////
// Operator::SystemColumnImpl::PartitionCategory::

// FUNCTION public
//	Operator::SystemColumnImpl::PartitionCategory::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
SystemColumnImpl::PartitionCategory::
getClassID() const
{
	return Class::getClassID(Class::Category::PartitionCategory);
}

// FUNCTION private
//	Operator::SystemColumnImpl::PartitionCategory::explainOperator -- explain operator
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::PartitionCategory::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Explain::PartitionCategory]);
}

// FUNCTION private
//	Operator::SystemColumnImpl::PartitionCategory::setTarget -- 
//
// NOTES
//
// ARGUMENTS
//	Common::Data::Pointer pData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::PartitionCategory::
setTarget(Common::Data::Pointer pData_)
{
	m_pData = _SYDNEY_DYNAMIC_CAST(Common::StringData*, pData_.get());
	; _SYDNEY_ASSERT(m_pData);
}

// FUNCTION private
//	Operator::SystemColumnImpl::PartitionCategory::setValue -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Schema::Database* pDatabase_
//	Schema::Object::ID::Value iID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::PartitionCategory::
setValue(Trans::Transaction& cTrans_,
		 Schema::Database* pDatabase_,
		 Schema::Object::ID::Value iID_)
{
	// obtain function object using ID
	Schema::Partition* pSchemaPartition = Schema::Partition::get(iID_, pDatabase_, cTrans_);
	if (pSchemaPartition == 0) {
		m_pData->setNull(true);
		return;
	}
	switch (pSchemaPartition->getCategory()) {
	case Schema::Partition::Category::ReadOnly:
		{
			m_pData->setValue(STRING("read only"));
			break;
		}
	case Schema::Partition::Category::Normal:
	default:
		{
			m_pData->setValue(STRING(""));
			break;
		}
	}
}

///////////////////////////////////////////
// Operator::SystemColumnImpl::FunctionRoutine::

// FUNCTION public
//	Operator::SystemColumnImpl::FunctionRoutine::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
SystemColumnImpl::FunctionRoutine::
getClassID() const
{
	return Class::getClassID(Class::Category::FunctionRoutine);
}

// FUNCTION private
//	Operator::SystemColumnImpl::FunctionRoutine::explainOperator -- explain operator
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::FunctionRoutine::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Explain::FunctionRoutine]);
}

// FUNCTION private
//	Operator::SystemColumnImpl::FunctionRoutine::setTarget -- 
//
// NOTES
//
// ARGUMENTS
//	Common::Data::Pointer pData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::FunctionRoutine::
setTarget(Common::Data::Pointer pData_)
{
	m_pData = _SYDNEY_DYNAMIC_CAST(Common::StringData*, pData_.get());
	; _SYDNEY_ASSERT(m_pData);
}

// FUNCTION private
//	Operator::SystemColumnImpl::FunctionRoutine::setValue -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Schema::Database* pDatabase_
//	Schema::Object::ID::Value iID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::FunctionRoutine::
setValue(Trans::Transaction& cTrans_,
		 Schema::Database* pDatabase_,
		 Schema::Object::ID::Value iID_)
{
	// obtain function object using ID
	Schema::Function* pSchemaFunction = Schema::Function::get(iID_, pDatabase_, cTrans_);
	if (pSchemaFunction == 0) {
		m_pData->setNull(true);
		return;
	}

	// get function definiton statement object
	const Statement::FunctionDefinition* pRoutine =
		_SYDNEY_DYNAMIC_CAST(const Statement::FunctionDefinition*,
							 pSchemaFunction->getRoutine());
	if (pRoutine == 0) {
		m_pData->setNull(true);
		return;
	}
	OSTRSTREAM stream;
	stream << "("
		   <<pRoutine->getParam()->toSQLStatement()
		   << ") "
		   << pRoutine->getReturns()->toSQLStatement()
		   << " "
		   << pRoutine->getRoutine()->toSQLStatement();

	m_pData->setValue(STRING(stream.getString()));
}

///////////////////////////////////////////
// Operator::SystemColumnImpl::DatabasePath::

// FUNCTION public
//	Operator::SystemColumnImpl::DatabasePath::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
SystemColumnImpl::DatabasePath::
getClassID() const
{
	return Class::getClassID(Class::Category::DatabasePath);
}

// FUNCTION private
//	Operator::SystemColumnImpl::DatabasePath::explainOperator -- explain operator
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::DatabasePath::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Explain::DatabasePath]);
}

// FUNCTION private
//	Operator::SystemColumnImpl::DatabasePath::setTarget -- 
//
// NOTES
//
// ARGUMENTS
//	Common::Data::Pointer pData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::DatabasePath::
setTarget(Common::Data::Pointer pData_)
{
	m_pData = _SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pData_.get());
	; _SYDNEY_ASSERT(m_pData);
}

// FUNCTION private
//	Operator::SystemColumnImpl::DatabasePath::setValue -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Schema::Database* pDatabase_
//	Schema::Object::ID::Value iID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::DatabasePath::
setValue(Trans::Transaction& cTrans_,
		 Schema::Database* pDatabase_,
		 Schema::Object::ID::Value iID_)
{
	// obtain database object using ID
	Schema::Database* pSchemaDatabase = Schema::Database::get(iID_, cTrans_);
	if (pSchemaDatabase == 0) {
		m_pData->setNull(true);
		return;
	}

	// get database path
	ModVector<ModUnicodeString> vecPath;
	pSchemaDatabase->getPath(vecPath);

	m_pData->clear();
	m_pData->reserve(vecPath.getSize());
	for (int i = 0; i < vecPath.getSize(); ++i) {
		m_pData->pushBack(new Common::StringData(vecPath[i]));
	}
}

///////////////////////////////////////////
// Operator::SystemColumnImpl::DatabaseMasterURL::

// FUNCTION public
//	Operator::SystemColumnImpl::DatabaseMasterURL::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
SystemColumnImpl::DatabaseMasterURL::
getClassID() const
{
	return Class::getClassID(Class::Category::DatabaseMasterURL);
}

// FUNCTION private
//	Operator::SystemColumnImpl::DatabaseMasterURL::explainOperator -- explain operator
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::DatabaseMasterURL::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Explain::DatabaseMasterURL]);
}

// FUNCTION private
//	Operator::SystemColumnImpl::DatabaseMasterURL::setTarget -- 
//
// NOTES
//
// ARGUMENTS
//	Common::Data::Pointer pData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::DatabaseMasterURL::
setTarget(Common::Data::Pointer pData_)
{
	m_pData = _SYDNEY_DYNAMIC_CAST(Common::StringData*, pData_.get());
	; _SYDNEY_ASSERT(m_pData);
}

// FUNCTION private
//	Operator::SystemColumnImpl::DatabaseMasterURL::setValue -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Schema::Database* pDatabase_
//	Schema::Object::ID::Value iID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
SystemColumnImpl::DatabaseMasterURL::
setValue(Trans::Transaction& cTrans_,
		 Schema::Database* pDatabase_,
		 Schema::Object::ID::Value iID_)
{
	// obtain database object using ID
	Schema::Database* pSchemaDatabase = Schema::Database::get(iID_, cTrans_);
	if (pSchemaDatabase == 0) {
		m_pData->setNull(true);
		return;
	}

	// get database masterURL
	const ModUnicodeString& cstrMasterURL = pSchemaDatabase->getMasterURL();
	if (cstrMasterURL.getLength() == 0) {
		m_pData->setNull(true);
		return;
	}
	m_pData->setValue(cstrMasterURL);
}

//////////////////////////////
// Operator::SystemColumn::

// FUNCTION public
//	Operator::SystemColumn::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	Schema::Column* pSchemaColumn_
//	int iRowIDID_
//	int iDataID_
//	
// RETURN
//	SystemColumn*
//
// EXCEPTIONS

//static
SystemColumn*
SystemColumn::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   Schema::Column* pSchemaColumn_,
	   int iRowIDID_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult;
	switch (pSchemaColumn_->getFunction(*cProgram_.getTransaction())) {
	case Schema::Column::Function::ColumnMetaData:
		{
			pResult = new SystemColumnImpl::ColumnMetaData(iRowIDID_, iDataID_);
			break;
		}
	case Schema::Column::Function::FileSize:
		{
			pResult = new SystemColumnImpl::FileSize(iRowIDID_, iDataID_);
			break;
		}
	case Schema::Column::Function::IndexHint:
		{
			pResult = new SystemColumnImpl::IndexHint(iRowIDID_, iDataID_);
			break;
		}
	case Schema::Column::Function::PrivilegeFlag:
		{
			pResult = new SystemColumnImpl::PrivilegeFlag(iRowIDID_, iDataID_);
			break;
		}
	case Schema::Column::Function::PrivilegeObjectType:
		{
			pResult = new SystemColumnImpl::PrivilegeObjectType(iRowIDID_, iDataID_);
			break;
		}
	case Schema::Column::Function::PrivilegeObjectID:
		{
			pResult = new SystemColumnImpl::PrivilegeObjectID(iRowIDID_, iDataID_);
			break;
		}
	case Schema::Column::Function::PartitionCategory:
		{
			pResult = new SystemColumnImpl::PartitionCategory(iRowIDID_, iDataID_);
			break;
		}
	case Schema::Column::Function::FunctionRoutine:
		{
			pResult = new SystemColumnImpl::FunctionRoutine(iRowIDID_, iDataID_);
			break;
		}
	case Schema::Column::Function::DatabasePath:
		{
			pResult = new SystemColumnImpl::DatabasePath(iRowIDID_, iDataID_);
			break;
		}
	case Schema::Column::Function::DatabaseMasterURL:
		{
			pResult = new SystemColumnImpl::DatabaseMasterURL(iRowIDID_, iDataID_);
			break;
		}
	default:
		_SYDNEY_THROW0(Exception::NotSupported);;
	}
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Operator::SystemColumn::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	SystemColumn*
//
// EXCEPTIONS

//static
SystemColumn*
SystemColumn::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::ColumnMetaData:
		{
			return new SystemColumnImpl::ColumnMetaData;
		}
	case Class::Category::FileSize:
		{
			return new SystemColumnImpl::FileSize;
		}
	case Class::Category::IndexHint:
		{
			return new SystemColumnImpl::IndexHint;
		}
	case Class::Category::PrivilegeFlag:
		{
			return new SystemColumnImpl::PrivilegeFlag;
		}
	case Class::Category::PrivilegeObjectType:
		{
			return new SystemColumnImpl::PrivilegeObjectType;
		}
	case Class::Category::PrivilegeObjectID:
		{
			return new SystemColumnImpl::PrivilegeObjectID;
		}
	case Class::Category::PartitionCategory:
		{
			return new SystemColumnImpl::PartitionCategory;
		}
	case Class::Category::FunctionRoutine:
		{
			return new SystemColumnImpl::FunctionRoutine;
		}
	case Class::Category::DatabasePath:
		{
			return new SystemColumnImpl::DatabasePath;
		}
	case Class::Category::DatabaseMasterURL:
		{
			return new SystemColumnImpl::DatabaseMasterURL;
		}
	default:
		_SYDNEY_THROW0(Exception::NotSupported);;
	}
}

_SYDNEY_EXECUTION_OPERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2009, 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
