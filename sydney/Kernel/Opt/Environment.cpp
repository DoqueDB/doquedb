// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Environment.cpp -- Environment in analysis and optimization
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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
	const char moduleName[] = "Opt";
	const char srcFile[] = __FILE__;
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Opt/Environment.h"

#include "Opt/Argument.h"
#include "Opt/Configuration.h"
#include "Opt/NameMap.h"
#include "Opt/SchemaObject.h"


#include "Common/Assert.h"

#include "Communication/Connection.h"

#include "Exception/Unexpected.h"

#include "Execution/Program.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "Plan/Interface/IFile.h"
#include "Plan/Relation/Argument.h"
#include "Plan/Relation/RowElement.h"
#include "Plan/Relation/RowInfo.h"
#include "Plan/Relation/Table.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Utility/ObjectSet.h"

#include "Schema/File.h"
#include "Schema/Table.h"

_SYDNEY_BEGIN
_SYDNEY_OPT_BEGIN

////////////////////////////
namespace Impl
{
	/////////////////////////////////////////////////////////////////////
	// CLASS
	//	Impl::EnvironmentImpl -- implementation of Environment interface
	//
	// NOTES
	//	Opt::Environment is refered by many classes,
	//	implementation of the interface is suppressed so that
	//	changing of implementation do not force recompilation of many classes.
	//	no-name namespace can't be used for this class for avoiding loss of symbol in debugging.
	//
	class EnvironmentImpl
		: public Environment
	{
	public:
		typedef EnvironmentImpl This;
		typedef Environment Super;

		// constructor
		EnvironmentImpl(const EnvironmentArgument& cArgument_)
			: Super(),
			  m_pNullConstant(0),
			  m_pDefaultConstant(0),
			  m_cArgument(cArgument_)
		{}

		// destructor
		~EnvironmentImpl() {destruct();}

		// accessor
		virtual Schema::Database* getDatabase() {return m_cArgument.m_pDatabase;}
		virtual Trans::Transaction& getTransaction() {return m_cArgument.m_cTransaction;}
		virtual Execution::Interface::IProgram* getProgram() {return m_cArgument.m_pProgram->getProgram();}
		virtual Communication::Connection* getConnection() {return m_cArgument.m_pConnection;}
		virtual Common::DataArrayData* getParameter() {return m_cArgument.m_pParameter;}
		virtual bool isPrepare() {return m_cArgument.m_bPrepare;}
		virtual bool isRecovery() {return m_cArgument.m_bRecovery;}
		virtual bool isRollback() {return m_cArgument.m_bRollback;}
		virtual bool isUndo() {return m_cArgument.m_bUndo;}
		virtual int getProtocolVersion()
		{
			return m_cArgument.m_pConnection
				? m_cArgument.m_pConnection->getMasterID()
				: 0;
		}
		virtual bool hasCascade()
		{
			return m_cArgument.m_pDatabase->hasCascade(m_cArgument.m_cTransaction);
		}

		virtual void setProgram(Execution::Interface::IProgram* pProgram_)
		{m_cArgument.m_pProgram->setProgram(pProgram_);}

		// interface
		virtual AutoPop pushStatus(Status::Value iStatus_);
		virtual AutoPop eraseStatus(Status::Value iStatus_);
		virtual void popStatus();
		virtual void addStatus(Status::Value iStatus_);
		virtual bool checkStatus(Status::Value iStatus_);

		virtual AutoPop pushNameScope(Scope::Value iScope_ = Scope::Normal);
		virtual void popNameScope();

		virtual bool isInExists();
		virtual bool isGrouping();
		virtual void setGrouping();
		virtual void addGroupingColumn(Plan::Relation::RowElement* pRowElement_);
		virtual void removeGroupingColumn(Plan::Interface::IScalar* pRowElement_);
		virtual bool isGroupingColumn(Plan::Relation::RowElement* pRowElement_);
		virtual Plan::Utility::RowElementSet& getGroupingColumn();

		virtual void addOuterReference(Plan::Interface::IRelation* pRelation_,
									   Plan::Relation::RowElement* pRowElement_);
		virtual void retrieveOuterReference();
		virtual bool hasOuterReference();

		virtual const Plan::Utility::RelationSet& getOuterRelation();

		virtual int addNode(Plan::Tree::Node* pNode_);
		virtual void addRowInfo(Plan::Relation::RowInfo* pRowInfo_);
		virtual void addTable(Plan::Relation::Table* pTable_);
		virtual void addColumn(Plan::Scalar::Field* pColumn_);
		virtual void addFile(Plan::Interface::IFile* pFile_);
		virtual int addObject(Common::Object* pObject_);

		virtual void addContains(Plan::Interface::IScalar* pColumn_,
								 Plan::Predicate::Contains* pContains_);
		virtual const VECTOR<Plan::Predicate::Contains*>&
					getContains(Plan::Interface::IScalar* pColumn_);
		virtual const VECTOR<Plan::Predicate::Contains*>&
					getContainsByAnyOperand(Plan::Interface::IScalar* pColumn_);

		virtual void eraseNode(int iID_);
		virtual void eraseObject(int iID_);

		virtual void addKnownNull(Plan::Interface::IScalar* pScalar_);
		virtual void addKnownNotNull(Plan::Interface::IScalar* pScalar_);

		virtual bool isKnownNull(Plan::Interface::IScalar* pScalar_);
		virtual bool isKnownNotNull(Plan::Interface::IScalar* pScalar_);

		virtual bool isSimpleTable(Schema::Table* pSchemaTable_);
		virtual void setSimpleTable(Schema::Table* pSchemaTable_);

		virtual bool isUpdateTable(Schema::Table* pSchemaTable_);
		virtual void setUpdateTable(Schema::Table* pSchemaTable_);

		virtual bool isInsertTable(Schema::Table* pSchemaTable_);
		virtual void setInsertTable(Schema::Table* pSchemaTable_);

		virtual bool isReferedTable(Schema::Table* pSchemaTable_);
		virtual void addReferedTable(Schema::Table* pSchemaTable_);
		virtual const VECTOR<Schema::Table*>& getReferedTable();

		virtual bool isSubqueryTable(Schema::Table* pSchemaTable_);
		virtual void addSubqueryTable(Schema::Table* pSchemaTable_);

		virtual void addIndexScan(Schema::File* pSchemaFile_);
		virtual bool isIndexScan(Schema::File* pSchemaFile_);

		virtual Plan::Interface::IRelation*
						getRelation(const STRING& cstrName_);

		virtual NameMap* getNameMap();
		virtual NameMap* getNameMap(Plan::Interface::IRelation* pRelation_);
		virtual void addNameMap(NameMap* pNameMap_);

		virtual Plan::Relation::RowElement*
						searchScalar(const STRING& cstrTableName_,
									 const STRING& cstrColumnName_);
		virtual Plan::Relation::RowElement*
						searchScalar(const STRING& cstrColumnName_);

		virtual void addScalar(Plan::Interface::IRelation* pRelation_,
							   const STRING& cstrName_,
							   Plan::Relation::RowElement* pRowElement_);

		virtual Plan::Interface::IPredicate*
						searchPredicate(const STRING& cstrStatement_);
		virtual void addPredicate(Plan::Interface::IRelation* pRelation_,
								  const STRING& cstrStatement_,
								  Plan::Interface::IPredicate* pPredicate_);

		virtual const STRING& getCorrelationName(Plan::Interface::IRelation* pRelation_);
		virtual void setAliasName(Plan::Relation::RowElement* pRowElement_,
								  const STRING& cstrName_);
		virtual const STRING& getAliasName(Plan::Relation::RowElement* pRowElement_);

		virtual Plan::Interface::IFile* getFile(const Schema::File* pSchemaFile_);
		virtual Plan::Interface::IFile* getFile(Plan::Interface::IScalar* pVariable_);

		virtual void setPlaceHolder(int iNumber_,
									Plan::Scalar::Value* pVariable_);
		virtual Plan::Scalar::Value* getPlaceHolder(int iNumber_);
		virtual const VECTOR<Plan::Scalar::Value*>& getPlaceHolder();

		virtual void setSessionVariable(const STRING& cstrName_,
										Plan::Scalar::Value* pVariable_);
		virtual Plan::Scalar::Value* getSessionVariable(const STRING& cstrName_);

		virtual Plan::Interface::IScalar*
						setDistinctFunction(Plan::Interface::IScalar* pOperand_,
										Plan::Interface::IScalar* pDistinct_);
		virtual Plan::Interface::IScalar*
						getDistinctFunction(Plan::Interface::IScalar* pOperand_);

		virtual void setNullConstant(Plan::Scalar::Value* pValue_);
		virtual void setDefaultConstant(Plan::Scalar::Value* pValue_);
		virtual Plan::Scalar::Value* getNullConstant();
		virtual Plan::Scalar::Value* getDefaultConstant();

		virtual void addLocator(Plan::Interface::IScalar* pScalar_,
								Execution::Interface::IIterator* pIterator_,
								int iLocatorID_);
		virtual int getLocator(Plan::Interface::IScalar* pScalar_,
							   Execution::Interface::IIterator* pIterator_);


	protected:
	private:
		void destruct();
		bool isCheckObsolete(const Schema::Table* pSchemaTable_);
		void addOuterReferenceToScope(Plan::Interface::IRelation* pRelation_,
									  Plan::Relation::RowElement* pRowElement_,
									  int iScope_);
		void addContainsByAnyOperand(Plan::Interface::IScalar* pColumn_,
									 Plan::Predicate::Contains* pContains_);

		// stack size for scope management
		enum {
			StackSize = 100
		};
		// stack element for scope management
		struct NameScope
		{
			typedef MAP<Plan::Relation::RowElement*,
						STRING,
						Plan::Utility::RowElementSet::Comparator> AliasMap;

			Scope::Value m_eScope;
			NameMap* m_pNameMap;
			bool m_bGrouping;
			Plan::Utility::RowElementSet m_cGroupingColumn;
			Plan::Utility::RowElementSet m_cOuterReference;
			Plan::Utility::RelationSet m_cOuterRelation;

			AliasMap m_mapAlias;
			bool m_bPop; // if true, namescope has been popped

			NameScope()
				: m_eScope(Scope::Normal),
				  m_pNameMap(0),
				  m_bGrouping(false),
				  m_cGroupingColumn(),
				  m_cOuterReference(),
				  m_cOuterRelation(),
				  m_mapAlias(),
				  m_bPop(false)
			{}
			NameScope(Scope::Value eScope_, NameMap* pNameMap_)
				: m_eScope(eScope_),
				  m_pNameMap(pNameMap_),
				  m_bGrouping(false),
				  m_cGroupingColumn(),
				  m_cOuterReference(),
				  m_cOuterRelation(),
				  m_mapAlias(),
				  m_bPop(false)
			{}
		};
		struct LocatorID
		{
			int m_iID;

			LocatorID() : m_iID(-1) {}
			LocatorID(int iID_) : m_iID(iID_) {}
			LocatorID(const LocatorID& cOther_) : m_iID(cOther_.m_iID) {}

			LocatorID& operator=(int iID_) {m_iID = iID_;return *this;}
			operator int() {return m_iID;}
		};

		// map for schema file object
		typedef MAP< Opt::SchemaObject,
					 Plan::Interface::IFile*,
					 LESS<Opt::SchemaObject> > FileMap;
		typedef MAP< STRING,
					 Plan::Interface::IFile*,
					 CaseInsensitiveComparator > VariableFileMap;
		// map for contains predicate
		typedef MAP< Plan::Interface::IScalar*,
					 VECTOR<Plan::Predicate::Contains*>,
					 Plan::Utility::ScalarSet::Comparator > ContainsMap;
		// map for distinct function
		typedef MAP< Plan::Interface::IScalar*,
					 Plan::Interface::IScalar*,
					 Plan::Utility::ScalarSet::Comparator > FunctionMap;
		// map for session variable
		typedef MAP< STRING,
					 Plan::Scalar::Value*,
					 CaseInsensitiveComparator > VariableNameMap;
		// map for locator
		typedef MAP< int,
					 MAP< int,
						  LocatorID,
						  LESS<int> >,
					 LESS<int> > LocatorMap;

		// scope management
		SHORTVECTOR<Status::Value> m_vecStatus;
		SHORTVECTOR<NameScope> m_vecName;
		SHORTVECTOR<int> m_vecNameSP;

		// pool of objects
		VECTOR<NameMap*> m_vecNameMapPool;
		VECTOR<Plan::Tree::Node*> m_vecNodePool;
		VECTOR<Plan::Relation::RowInfo*> m_vecRowInfoPool;
		VECTOR<Common::Object*> m_vecObjectPool;

		// table infomation
		Plan::Utility::SchemaTableSet m_cSimpleTable;
		Plan::Utility::SchemaTableSet m_cInsertTable;
		Plan::Utility::SchemaTableSet m_cUpdateTable;
		Plan::Utility::SchemaTableSet m_cReferedTable;
		Plan::Utility::SchemaTableSet m_cSubqueryTable;

		
		// instance management
		FileMap m_mapFile;
		VariableFileMap m_mapVariableFile;
		ContainsMap m_mapContains;
		ContainsMap m_mapContainsByAnyOperand;
		FunctionMap m_mapFunction;
		LocatorMap m_mapLocator;
		VariableNameMap m_mapVariableName;

		// scalar information
		Plan::Utility::ScalarSet m_cKnownNotNull;
		Plan::Utility::ScalarSet m_cKnownNull;

		// index information
		Plan::Utility::SchemaFileSet m_cIndexScan;

		// variable
		VECTOR<Plan::Scalar::Value*> m_vecPlaceHolder;

		// constant value
		Plan::Scalar::Value* m_pNullConstant;
		Plan::Scalar::Value* m_pDefaultConstant;

		// argument
		EnvironmentArgument m_cArgument;


	};

	// CLASS
	//	$$::_Destructor -- destructor
	//
	// NOTES
	template <class Object_>
	inline
	void _DestructElement(Object_* p_)
	{
		delete p_;
	}
	template <class Object_>
	inline
	void _Destruct(VECTOR<Object_*>& vecTarget_)
	{
		FOREACH(vecTarget_.begin(),
				vecTarget_.end(),
				_DestructElement<Object_>);
		vecTarget_.erase(vecTarget_.begin(), vecTarget_.end());
	}

	// CONSTANT local
	// _emptyContains --
	//
	// NOTES
	const VECTOR<Plan::Predicate::Contains*> _emptyContains;

} // namespace EnvironmentImpl
////////////////////////////

////////////////////////////
//	Impl::EnvironmentImpl

// FUNCTION public
//	Impl::EnvironmentImpl::pushStatus -- add new status in analysis
//
// NOTES
//
// ARGUMENTS
//	Status::Value iStatus_
//	
// RETURN
//	Environment::AutoPop
//
// EXCEPTIONS

//virtual
Environment::AutoPop
Impl::EnvironmentImpl::
pushStatus(Status::Value iStatus_)
{
	if (m_vecStatus.GETSIZE() == 0) {
		m_vecStatus.reserve(StackSize);
		m_vecStatus.PUSHBACK(iStatus_);
	} else if ((iStatus_ & Status::Reset) != 0) {
		m_vecStatus.PUSHBACK(iStatus_);
	} else {
		m_vecStatus.PUSHBACK(m_vecStatus.GETBACK() | iStatus_);
	}
	return AutoPop(this, &Super::popStatus);
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::eraseStatus -- 
//
// NOTES
//
// ARGUMENTS
//	Status::Value iStatus_
//	
// RETURN
//	Environment::AutoPop
//
// EXCEPTIONS

//virtual
Environment::AutoPop
Impl::EnvironmentImpl::
eraseStatus(Status::Value iStatus_)
{
	if (m_vecStatus.GETSIZE() > 0) {
		m_vecStatus.PUSHBACK(m_vecStatus.GETBACK() & ~iStatus_);
		return AutoPop(this, &Super::popStatus);
	}
	// do nothing
	return AutoPop(0,0);
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::popStatus -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
popStatus()
{
	if (m_vecStatus.GETSIZE() > 0) {
		m_vecStatus.POPBACK();
	}
}

// FUNCTION public
//	Impl::EnvironmentImpl::addStatus -- 
//
// NOTES
//
// ARGUMENTS
//	Status::Value iStatus_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
addStatus(Status::Value iStatus_)
{
	if (m_vecStatus.GETSIZE() > 0) {
		m_vecStatus.GETBACK() |= iStatus_;
	}
}

// FUNCTION local
//	Impl::EnvironmentImpl::checkStatus -- check status value
//
// NOTES
//
// ARGUMENTS
//	Status::Value iStatus_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::EnvironmentImpl::
checkStatus(Status::Value iStatus_)
{
	if (m_vecStatus.GETSIZE() > 0) {
		return (m_vecStatus.GETBACK() & iStatus_) == iStatus_;
	}
	return false;
}

// FUNCTION public
//	Impl::EnvironmentImpl::pushNameScope -- go into new scope for naming
//
// NOTES
//
// ARGUMENTS
//	Scope::Value iScope_ = Scope::Normal
//
// RETURN
//	Environment::AutoPop
//
// EXCEPTIONS

//virtual
Environment::AutoPop
Impl::EnvironmentImpl::
pushNameScope(Scope::Value iScope_ /* = Scope::Normal */)
{
	if (m_vecName.GETSIZE() == 0) {
		m_vecName.reserve(StackSize);
		m_vecNameSP.reserve(StackSize);
	} else if (iScope_ == Scope::JoinedTable
			   && m_vecName[m_vecNameSP.GETBACK()].m_eScope == Scope::JoinedTable) {
		// joined tables share one name scope
		return AutoPop(0 ,0);
	}
	m_vecName.PUSHBACK(NameScope(iScope_, 0));
	m_vecNameSP.PUSHBACK(m_vecName.GETSIZE() - 1);

	return AutoPop(this, &Super::popNameScope);
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::popNameScope -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
popNameScope()
{
	if (m_vecNameSP.GETSIZE() > 0) {
		if (m_vecName[m_vecNameSP.GETBACK()].m_eScope == Scope::JoinedTable) {
			; _SYDNEY_ASSERT(m_vecNameSP.GETSIZE() > 1);
			// mark current name scope as popped for next pop
			m_vecName[m_vecNameSP.GETBACK()].m_bPop = true;
			// swap current and previous name scope instead of popping
			Opt::Swap(m_vecNameSP.end() - 1, m_vecNameSP.end() - 2);
		} else {
			// just pop name scope
			m_vecNameSP.POPBACK();
			while (m_vecNameSP.GETSIZE() > 0
				   && m_vecName[m_vecNameSP.GETBACK()].m_bPop) {
				// pop scopes which has been swaped at previous popping
				m_vecNameSP.POPBACK();
			}
		}
	}
}

// FUNCTION public
//	Impl::EnvironmentImpl::isInExists -- check current status is in exists predicate
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::EnvironmentImpl::
isInExists()
{
	return m_vecNameSP.GETSIZE() > 0
		&& m_vecName[m_vecNameSP.GETBACK()].m_eScope == Scope::Exists;
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::isGrouping -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::EnvironmentImpl::
isGrouping()
{
	return m_vecNameSP.GETSIZE() > 0
		&& m_vecName[m_vecNameSP.GETBACK()].m_bGrouping;
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::setGrouping -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
setGrouping()
{
	if (m_vecNameSP.GETSIZE() > 0) {
		m_vecName[m_vecNameSP.GETBACK()].m_bGrouping = true;
	}
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::addGroupingColumn -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Relation::RowElement* pRowElement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
addGroupingColumn(Plan::Relation::RowElement* pRowElement_)
{
	m_vecName[m_vecNameSP.GETBACK()].m_cGroupingColumn.add(pRowElement_);
}


// FUNCTION public
//	Opt::Impl::EnvironmentImpl::removeGroupingColumn -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Relation::RowElement* pRowElement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
removeGroupingColumn(Plan::Interface::IScalar* pScalar_)
{
	Plan::Utility::RowElementSet::Iterator ite =
		m_vecName[m_vecNameSP.GETBACK()].m_cGroupingColumn.begin();
	Plan::Utility::RowElementSet::Iterator end =
		m_vecName[m_vecNameSP.GETBACK()].m_cGroupingColumn.end();
	for (; ite != end; ++ite) {
		if (pScalar_
			&& (*ite)->getScalar()
			&& pScalar_ == (*ite)->getScalar()) {
				m_vecName[m_vecNameSP.GETBACK()].m_cGroupingColumn.remove(*ite);
				break;
		}	
	}
}


// FUNCTION public
//	Opt::Impl::EnvironmentImpl::isGroupingColumn -- check whether a rowelement is grouping column
//
// NOTES
//
// ARGUMENTS
//	Plan::Relation::RowElement* pRowElement_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::EnvironmentImpl::
isGroupingColumn(Plan::Relation::RowElement* pRowElement_)
{
	return m_vecName[m_vecNameSP.GETBACK()].m_cGroupingColumn.isContaining(pRowElement_);
}



// FUNCTION public
//	Opt::Impl::EnvironmentImpl::getGroupingColumn --
//
// NOTES
//
// ARGUMENTS
//	
//	
// RETURN
//	Plan::Utility::RowElementSet&
//
// EXCEPTIONS

//virtual
Plan::Utility::RowElementSet&
Impl::EnvironmentImpl::
getGroupingColumn()
{
	return m_vecName[m_vecNameSP.GETBACK()].m_cGroupingColumn;
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::addOuterReference -- add outer reference
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IRelation* pRelation_
//	Plan::Relation::RowElement* pRowElement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
addOuterReference(Plan::Interface::IRelation* pRelation_,
				  Plan::Relation::RowElement* pRowElement_)
{
	addOuterReferenceToScope(pRelation_,
							 pRowElement_,
							 m_vecNameSP.GETBACK());
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::retrieveOuterReference -- set retrieved for all outer reference
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
retrieveOuterReference()
{
	m_vecName[m_vecNameSP.GETBACK()].m_cOuterReference.foreachElement(
									  boost::bind(&Plan::Relation::RowElement::retrieve,
												  _1,
												  boost::ref(*this)));
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::hasOuterReference -- has outer reference?
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::EnvironmentImpl::
hasOuterReference()
{
	return m_vecName[m_vecNameSP.GETBACK()].m_cOuterReference.isEmpty() == false;
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::getOuterRelation -- get outer references' relation
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Plan::Utility::RelationSet&
//
// EXCEPTIONS

//virtual
const Plan::Utility::RelationSet&
Impl::EnvironmentImpl::
getOuterRelation()
{
	return m_vecName[m_vecNameSP.GETBACK()].m_cOuterRelation;
}

// FUNCTION public
//	Impl::EnvironmentImpl::addNode -- add new plan::tree::node
//
// NOTES
//	object life of node is controled by this class
//
// ARGUMENTS
//	Plan::Tree::Node* pNode_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Impl::EnvironmentImpl::
addNode(Plan::Tree::Node* pNode_)
{
	; _SYDNEY_ASSERT(m_vecNodePool.find(pNode_) == m_vecNodePool.end());
	m_vecNodePool.PUSHBACK(pNode_);
	return m_vecNodePool.GETSIZE() - 1;
}

// FUNCTION public
//	Impl::EnvironmentImpl::addRowInfo -- add rowinfo to pool
//
// NOTES
//
// ARGUMENTS
//	Plan::Relation::RowInfo* pRowInfo_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
addRowInfo(Plan::Relation::RowInfo* pRowInfo_)
{
	; _SYDNEY_ASSERT(m_vecRowInfoPool.find(pRowInfo_) == m_vecRowInfoPool.end());
	m_vecRowInfoPool.PUSHBACK(pRowInfo_);
}

// FUNCTION public
//	Impl::EnvironmentImpl::addTable -- add table info for validity check
//
// NOTES
//
// ARGUMENTS
//	Plan::Relation::Table* pTable_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
addTable(Plan::Relation::Table* pTable_)
{
	; _SYDNEY_ASSERT(pTable_->getSchemaTable());
	if (isCheckObsolete(pTable_->getSchemaTable())) {
		getProgram()->addTable(*pTable_->getSchemaTable());
	}
}

// FUNCTION public
//	Impl::EnvironmentImpl::addColumn -- add column info for validity check
//
// NOTES
//
// ARGUMENTS
//	Plan::Scalar::Field* pColumn_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
addColumn(Plan::Scalar::Field* pColumn_)
{
	if (pColumn_->isColumn()) {
		; _SYDNEY_ASSERT(pColumn_->getSchemaColumn());
		if (isCheckObsolete(pColumn_->getTable()->getSchemaTable())) {
			getProgram()->addColumn(*pColumn_->getSchemaColumn());
		}
	}
}

// FUNCTION public
//	Impl::EnvironmentImpl::addFile -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IFile* pFile_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
addFile(Plan::Interface::IFile* pFile_)
{
	if (pFile_->getSchemaFile()) {
		m_mapFile[Opt::SchemaObject(*pFile_->getSchemaFile())] = pFile_;
		if (isCheckObsolete(pFile_->getSchemaFile()->getTable(getTransaction()))) {
			getProgram()->addFile(*pFile_->getSchemaFile());
		}
	} else if (pFile_->getSessionVariable()) {
		m_mapVariableFile[pFile_->getSessionVariable()->getName()] = pFile_;
	}
}

// FUNCTION public
//	Opt::EnvironmentImpl::addObject -- add Common::Object to memory management
//
// NOTES
//
// ARGUMENTS
//	Common::Object* pObject_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Impl::EnvironmentImpl::
addObject(Common::Object* pObject_)
{
	; _SYDNEY_ASSERT(m_vecObjectPool.find(pObject_) == m_vecObjectPool.end());
	m_vecObjectPool.PUSHBACK(pObject_);
	return m_vecObjectPool.GETSIZE() - 1;
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::addContains -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IScalar* pColumn_
//	Plan::Predicate::Contains* pContains_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
addContains(Plan::Interface::IScalar* pColumn_,
			Plan::Predicate::Contains* pContains_)
{
	m_mapContains[pColumn_].PUSHBACK(pContains_);
	if (pColumn_->getType() != Plan::Tree::Node::Field) {
		// add operands too
		Plan::Utility::FieldSet cFieldSet;
		pColumn_->getUsedField(cFieldSet);
		cFieldSet.foreachElement(boost::bind(&This::addContainsByAnyOperand,
											 this,
											 _1,
											 pContains_));
	}
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::getContains -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IScalar* pColumn_
//	
// RETURN
//	const VECTOR<Plan::Predicate::Contains*>&
//
// EXCEPTIONS

//virtual
const VECTOR<Plan::Predicate::Contains*>&
Impl::EnvironmentImpl::
getContains(Plan::Interface::IScalar* pColumn_)
{
	// 複合索引の場合
	if (pColumn_->getType() == Plan::Tree::Node::List) {
		ContainsMap::ITERATOR ite = m_mapContains.begin();
		for (; ite != m_mapContains.end(); ite++) {
			if ((*ite).first->equalsOperand(pColumn_)) {
				return (*ite).second;
			}
		}
		return _emptyContains;
	} else {
		ContainsMap::ITERATOR found = m_mapContains.find(pColumn_);
		if (found == m_mapContains.end()) {
			return _emptyContains;
		}
		return (*found).second;
	}
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::getContainsByAnyOperand -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IScalar* pColumn_
//	
// RETURN
//	const VECTOR<Plan::Predicate::Contains*>&
//
// EXCEPTIONS

//virtual
const VECTOR<Plan::Predicate::Contains*>&
Impl::EnvironmentImpl::
getContainsByAnyOperand(Plan::Interface::IScalar* pColumn_)
{
	ContainsMap::ITERATOR found = m_mapContains.find(pColumn_);
	if (found == m_mapContains.end()) {
		if (pColumn_->getType() == Plan::Tree::Node::Field) {
			// search for any operand
			found = m_mapContainsByAnyOperand.find(pColumn_);
			if (found != m_mapContainsByAnyOperand.end()) {
				// found
				return (*found).second;
			}
		}
		// not found in any map
		return _emptyContains;
	}
	return (*found).second;
}

// FUNCTION public
//	Impl::EnvironmentImpl::eraseNode -- 
//
// NOTES
//
// ARGUMENTS
//	int iID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
eraseNode(int iID_)
{
	; _SYDNEY_ASSERT(iID_ >= 0 && iID_ < m_vecNodePool.GETSIZE());
	m_vecNodePool[iID_] = 0;
}

// FUNCTION public
//	Impl::EnvironmentImpl::eraseObject -- 
//
// NOTES
//
// ARGUMENTS
//	int iID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
eraseObject(int iID_)
{
	; _SYDNEY_ASSERT(iID_ >= 0 && iID_ < m_vecObjectPool.GETSIZE());
	m_vecObjectPool[iID_] = 0;
}

// FUNCTION public
//	Impl::EnvironmentImpl::addKnownNull -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IScalar* pScalar_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
addKnownNull(Plan::Interface::IScalar* pScalar_)
{
	if (!(pScalar_->isArbitraryElement()
		  || (pScalar_->isField()
			  && pScalar_->getField()->isExpandElement()))) {
		m_cKnownNull.add(pScalar_);
	}
}

// FUNCTION public
//	Impl::EnvironmentImpl::addKnownNotNull -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IScalar* pScalar_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
addKnownNotNull(Plan::Interface::IScalar* pScalar_)
{
	if (!(pScalar_->isArbitraryElement()
		  || (pScalar_->isField()
			  && pScalar_->getField()->isExpandElement()))) {
		m_cKnownNotNull.add(pScalar_);
	}
}

// FUNCTION public
//	Impl::EnvironmentImpl::isKnownNull -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IScalar* pScalar_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::EnvironmentImpl::
isKnownNull(Plan::Interface::IScalar* pScalar_)
{
	return m_cKnownNull.isContaining(pScalar_);
}

// FUNCTION public
//	Impl::EnvironmentImpl::isKnownNotNull -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IScalar* pScalar_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::EnvironmentImpl::
isKnownNotNull(Plan::Interface::IScalar* pScalar_)
{
	return m_cKnownNotNull.isContaining(pScalar_);
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::isSimpleTable -- 
//
// NOTES
//
// ARGUMENTS
//	Schema::Table* pSchemaTable_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::EnvironmentImpl::
isSimpleTable(Schema::Table* pSchemaTable_)
{
	return m_cSimpleTable.isContaining(pSchemaTable_);
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::setSimpleTable -- 
//
// NOTES
//
// ARGUMENTS
//	Schema::Table* pSchemaTable_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
setSimpleTable(Schema::Table* pSchemaTable_)
{
	m_cSimpleTable.add(pSchemaTable_);
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::isUpdateTable -- 
//
// NOTES
//
// ARGUMENTS
//	Schema::Table* pSchemaTable_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::EnvironmentImpl::
isUpdateTable(Schema::Table* pSchemaTable_)
{
	return m_cUpdateTable.isContaining(pSchemaTable_);
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::setUpdateTable -- 
//
// NOTES
//
// ARGUMENTS
//	Schema::Table* pSchemaTable_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
setUpdateTable(Schema::Table* pSchemaTable_)
{
	m_cUpdateTable.add(pSchemaTable_);
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::isInsertTable -- 
//
// NOTES
//
// ARGUMENTS
//	Schema::Table* pSchemaTable_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::EnvironmentImpl::
isInsertTable(Schema::Table* pSchemaTable_)
{
	return m_cInsertTable.isContaining(pSchemaTable_);
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::setInsertTable -- 
//
// NOTES
//
// ARGUMENTS
//	Schema::Table* pSchemaTable_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
setInsertTable(Schema::Table* pSchemaTable_)
{
	m_cInsertTable.add(pSchemaTable_);
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::isReferedTable -- check whether a table is refered
//
// NOTES
//
// ARGUMENTS
//	Schema::Table* pSchemaTable_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::EnvironmentImpl::
isReferedTable(Schema::Table* pSchemaTable_)
{
	return m_cReferedTable.isContaining(pSchemaTable_);
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::addReferedTable -- add a table to be refered
//
// NOTES
//
// ARGUMENTS
//	Schema::Table* pSchemaTable_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
addReferedTable(Schema::Table* pSchemaTable_)
{
	m_cReferedTable.add(pSchemaTable_);
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::getReferedTable -- get all the table refered in order of schema::id
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const VECTOR<Schema::Table*>&
//
// EXCEPTIONS

//virtual
const VECTOR<Schema::Table*>&
Impl::EnvironmentImpl::
getReferedTable()
{
	return m_cReferedTable.getVector();
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::isSubqueryTable -- 
//
// NOTES
//
// ARGUMENTS
//	Schema::Table* pSchemaTable_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::EnvironmentImpl::
isSubqueryTable(Schema::Table* pSchemaTable_)
{
	return m_cSubqueryTable.isContaining(pSchemaTable_);
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::addSubqueryTable -- 
//
// NOTES
//
// ARGUMENTS
//	Schema::Table* pSchemaTable_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
addSubqueryTable(Schema::Table* pSchemaTable_)
{
	m_cSubqueryTable.add(pSchemaTable_);
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::addIndexScan -- 
//
// NOTES
//
// ARGUMENTS
//	Schema::File* pSchemaFile_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
addIndexScan(Schema::File* pSchemaFile_)
{
	if (pSchemaFile_->getIndexID() != Schema::ObjectID::Invalid) {
		m_cIndexScan.add(pSchemaFile_);
	}
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::isIndexScan -- 
//
// NOTES
//
// ARGUMENTS
//	Schema::File* pSchemaFile_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::EnvironmentImpl::
isIndexScan(Schema::File* pSchemaFile_)
{
	return pSchemaFile_->getIndexID() != Schema::ObjectID::Invalid
		&& m_cIndexScan.isContaining(pSchemaFile_);
}

// FUNCTION public
//	Impl::EnvironmentImpl::getRelation -- get relation by a name
//
// NOTES
//
// ARGUMENTS
//	const STRING& cstrName_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
Impl::EnvironmentImpl::
getRelation(const STRING& cstrName_)
{
	Plan::Interface::IRelation* pResult = 0;
	if (m_vecNameSP.GETSIZE() > 0) {
		SHORTVECTOR<int>::Iterator iterator = m_vecNameSP.end();
		const SHORTVECTOR<int>::Iterator first = m_vecNameSP.begin();
		do {
			--iterator;
			if (m_vecName[*iterator].m_pNameMap
				&& ((pResult = m_vecName[*iterator].m_pNameMap->get(cstrName_)) != 0)) {
				break;
			}
		} while (iterator != first);
	}
	return pResult;
}

// FUNCTION public
//	Impl::EnvironmentImpl::getNameMap -- get namemap for current namescope
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	NameMap*
//
// EXCEPTIONS

//virtual
NameMap*
Impl::EnvironmentImpl::
getNameMap()
{
	if (m_vecNameSP.GETSIZE() > 0) {
		NameScope& cEntry = m_vecName[m_vecNameSP.GETBACK()];
		if (cEntry.m_pNameMap == 0) {
			cEntry.m_pNameMap = NameMap::create(*this);
		}
		return cEntry.m_pNameMap;
	}
	return 0;
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::getNameMap -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IRelation* pRelation_
//	
// RETURN
//	NameMap*
//
// EXCEPTIONS

//virtual
NameMap*
Impl::EnvironmentImpl::
getNameMap(Plan::Interface::IRelation* pRelation_)
{
	NameMap* pResult = 0;
	if (m_vecNameSP.GETSIZE() > 0) {
		SHORTVECTOR<int>::Iterator iterator = m_vecNameSP.end();
		const SHORTVECTOR<int>::Iterator first = m_vecNameSP.begin();
		do {
			--iterator;
			if (NameMap* pMap = m_vecName[*iterator].m_pNameMap) {
				if (pMap->isHasMap(pRelation_)) {
					pResult = pMap;
					break;
				}
			}
		} while (iterator != first);
	}
	return pResult;
}

// FUNCTION public
//	Impl::EnvironmentImpl::addNameMap -- register new namemap to object pool
//
// NOTES
//
// ARGUMENTS
//	NameMap* pNameMap_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
addNameMap(NameMap* pNameMap_)
{
	; _SYDNEY_ASSERT(m_vecNameMapPool.find(pNameMap_) == m_vecNameMapPool.end());
	m_vecNameMapPool.PUSHBACK(pNameMap_);
}

// FUNCTION public
//	Impl::EnvironmentImpl::searchScalar -- search scalar using table name and column name
//
// NOTES
//
// ARGUMENTS
//	const STRING& cstrTableName_
//	const STRING& cstrColumnName_
//	
// RETURN
//	Plan::Relation::RowElement*
//
// EXCEPTIONS

//virtual
Plan::Relation::RowElement*
Impl::EnvironmentImpl::
searchScalar(const STRING& cstrTableName_,
			 const STRING& cstrColumnName_)
{
	Plan::Relation::RowElement* pResult = 0;
	Plan::Interface::IRelation* pRelation = 0;

	if (m_vecNameSP.GETSIZE() > 0) {
		SHORTVECTOR<int> vecOuterScope;
		int nScope = 0;
		SHORTVECTOR<int>::Iterator iterator = m_vecNameSP.end();
		const SHORTVECTOR<int>::Iterator first = m_vecNameSP.begin();
		do {
			if (nScope && m_vecName[*iterator].m_bPop == false) {
				// record namescape to set outerreference
				vecOuterScope.PUSHBACK(*iterator);
			}
			--iterator;

			if (m_vecName[*iterator].m_bPop == false) ++nScope;

			if (m_vecName[*iterator].m_pNameMap
				&& ((pRelation = m_vecName[*iterator].m_pNameMap->get(cstrTableName_)) != 0)) {
				pResult = m_vecName[*iterator].m_pNameMap->get(pRelation, cstrColumnName_).second;

				// if in group by clause, set as grouping column
				if (pResult && checkStatus(Status::GroupBy)) {
					// add to current scope
					m_vecName[m_vecNameSP.GETBACK()].m_cGroupingColumn.add(pResult);
				}
				break;
			}
		} while (iterator != first);

		if (pResult && nScope > 1) {
			Opt::ForEach(vecOuterScope,
						 boost::bind(&This::addOuterReferenceToScope,
									 this,
									 pRelation,
									 pResult,
									 _1));
		}
	}

	return pResult;
}

// FUNCTION public
//	Impl::EnvironmentImpl::searchScalar -- search scalar using column name
//
// NOTES
//
// ARGUMENTS
//	const STRING& cstrColumnName_
//	
// RETURN
//	Environment::SearchScalarResult
//
// EXCEPTIONS

//virtual
Plan::Relation::RowElement*
Impl::EnvironmentImpl::
searchScalar(const STRING& cstrColumnName_)
{
	Plan::Relation::RowElement* pResult = 0;
	Plan::Interface::IRelation* pRelation = 0;

	if (m_vecNameSP.GETSIZE() > 0) {
		SHORTVECTOR<int> vecOuterScope;
		int nScope = 0;
		SHORTVECTOR<int>::Iterator iterator = m_vecNameSP.end();
		const SHORTVECTOR<int>::Iterator first = m_vecNameSP.begin();
		do {
			if (nScope && m_vecName[*iterator].m_bPop == false) {
				// record namescape to set outerreference
				vecOuterScope.PUSHBACK(*iterator);
			}
			--iterator;

			if (m_vecName[*iterator].m_bPop == false) ++nScope;

			if (m_vecName[*iterator].m_pNameMap) {
				PAIR<Plan::Interface::IRelation*, Plan::Relation::RowElement*> cResult =
					m_vecName[*iterator].m_pNameMap->get(0, cstrColumnName_);
				if (cResult.second != 0) {
					pRelation = cResult.first;
					pResult = cResult.second;
					// if in group by clause, set as grouping column
					if (checkStatus(Status::GroupBy)) {
						// add to current scope
						m_vecName[m_vecNameSP.GETBACK()].m_cGroupingColumn.add(pResult);
					}
					break;
				}
			}
		} while (iterator != first);

		if (pResult && nScope > 1) {
			Opt::ForEach(vecOuterScope,
						 boost::bind(&This::addOuterReferenceToScope,
									 this,
									 pRelation,
									 pResult,
									 _1));
		}
	}
		
	return pResult;
}

// FUNCTION public
//	Impl::EnvironmentImpl::addScalar -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IRelation* pRelation_
//	const STRING& cstrName_
//	Plan::Relation::RowElement* pRowElement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
addScalar(Plan::Interface::IRelation* pRelation_,
		  const STRING& cstrName_,
		  Plan::Relation::RowElement* pRowElement_)
{
	NameMap* pNameMap = getNameMap(pRelation_);
	if (pNameMap == 0) {
		pNameMap = getNameMap();
	}
	pNameMap->addElement(pNameMap->add(STRING(), pRelation_),
						 cstrName_,
						 pRowElement_);
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::searchPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	const STRING& cstrStatement_
//	
// RETURN
//	Plan::Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Plan::Interface::IPredicate*
Impl::EnvironmentImpl::
searchPredicate(const STRING& cstrStatement_)
{
	// only search in inner-most scope
	Plan::Interface::IPredicate* pResult = 0;

	if (m_vecNameSP.GETSIZE() > 0) {
		int iSP = m_vecNameSP.GETBACK();
		if (m_vecName[iSP].m_pNameMap) {
			pResult = m_vecName[iSP].m_pNameMap->getPredicate(cstrStatement_);
		}
	}
	return pResult;
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::addPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IRelation* pRelation_
//	const STRING& cstrStatement_
//	Plan::Interface::IPredicate* pPredicate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
addPredicate(Plan::Interface::IRelation* pRelation_,
			 const STRING& cstrStatement_,
			 Plan::Interface::IPredicate* pPredicate_)
{
	NameMap* pNameMap = getNameMap(pRelation_);
	if (pNameMap == 0) {
		pNameMap = getNameMap();
	}
	pNameMap->addPredicate(cstrStatement_,
						   pPredicate_);
}

// FUNCTION public
//	Impl::EnvironmentImpl::getCorrelationName -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IRelation* pRelation_
//	
// RETURN
//	const STRING&
//
// EXCEPTIONS

//virtual
const STRING&
Impl::EnvironmentImpl::
getCorrelationName(Plan::Interface::IRelation* pRelation_)
{
	// search for outmost scope having the relation
	SHORTVECTOR<NameScope>::Iterator iterator = m_vecName.begin();
	const SHORTVECTOR<NameScope>::Iterator last = m_vecName.end();
	for (; iterator != last; ++iterator) {
		STRING* pString = 0;
		if ((*iterator).m_pNameMap
			&& (pString = (*iterator).m_pNameMap->getCorrelationName(pRelation_)) != 0) {
			return *pString;
		}
	}
	static STRING cstrEmpty = "";
	return cstrEmpty;
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::setAliasName -- set alias name of a rowelement
//
// NOTES
//
// ARGUMENTS
//	Plan::Relation::RowElement* pRowElement_
//	const STRING& cstrName_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
setAliasName(Plan::Relation::RowElement* pRowElement_,
			 const STRING& cstrName_)
{
	m_vecName[m_vecNameSP.GETBACK()].m_mapAlias[pRowElement_] = cstrName_;
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::getAliasName -- get alias name of a rowelement
//
// NOTES
//
// ARGUMENTS
//	Plan::Relation::RowElement* pRowElement_
//	
// RETURN
//	const STRING&
//
// EXCEPTIONS

//virtual
const STRING&
Impl::EnvironmentImpl::
getAliasName(Plan::Relation::RowElement* pRowElement_)
{
	// search for outmost scope having the rowelement
	SHORTVECTOR<NameScope>::Iterator iterator = m_vecName.begin();
	const SHORTVECTOR<NameScope>::Iterator last = m_vecName.end();
	for (; iterator != last; ++iterator) {
		NameScope::AliasMap::Iterator found = (*iterator).m_mapAlias.find(pRowElement_);
		if (found != (*iterator).m_mapAlias.end()) {
			return (*found).second;
		}
	}
	return pRowElement_->getScalarName(*this);
}

// FUNCTION public
//	Impl::EnvironmentImpl::getFile -- 
//
// NOTES
//
// ARGUMENTS
//	const Schema::File* pSchemaFile_
//	
// RETURN
//	Plan::Interface::IFile*
//
// EXCEPTIONS

//virtual
Plan::Interface::IFile*
Impl::EnvironmentImpl::
getFile(const Schema::File* pSchemaFile_)
{
	FileMap::Iterator found = m_mapFile.find(Opt::SchemaObject(*pSchemaFile_));
	if (found == m_mapFile.end()) {
		return 0;
	} else {
		return (*found).second;
	}
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::getFile -- get file object related to a variable
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IScalar* pVariable_
//	
// RETURN
//	Plan::Interface::IFile*
//
// EXCEPTIONS

//virtual
Plan::Interface::IFile*
Impl::EnvironmentImpl::
getFile(Plan::Interface::IScalar* pVariable_)
{
	VariableFileMap::Iterator found = m_mapVariableFile.find(pVariable_->getName());
	if (found == m_mapVariableFile.end()) {
		return 0;
	} else {
		return (*found).second;
	}
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::setPlaceHolder -- 
//
// NOTES
//
// ARGUMENTS
//	int iNumber_
//	Plan::Scalar::Value* pVariable_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
setPlaceHolder(int iNumber_,
			   Plan::Scalar::Value* pVariable_)
{
	ExpandContainer(m_vecPlaceHolder, iNumber_ + 1, static_cast<Plan::Scalar::Value*>(0));
	if (m_vecPlaceHolder[iNumber_] != 0) {
		// duplicate number -> illegal
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	m_vecPlaceHolder[iNumber_] = pVariable_;
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::getPlaceHolder -- 
//
// NOTES
//
// ARGUMENTS
//	int iNumber_
//	
// RETURN
//	Plan::Scalar::Value*
//
// EXCEPTIONS

//virtual
Plan::Scalar::Value*
Impl::EnvironmentImpl::
getPlaceHolder(int iNumber_)
{
	if (iNumber_ < m_vecPlaceHolder.GETSIZE()) {
		return m_vecPlaceHolder[iNumber_];
	}
	return 0;
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::getPlaceHolder -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const VECTOR<Plan::Scalar::Value*>&
//
// EXCEPTIONS

//virtual
const VECTOR<Plan::Scalar::Value*>&
Impl::EnvironmentImpl::
getPlaceHolder()
{
	return m_vecPlaceHolder;
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::setSessionVariable -- 
//
// NOTES
//
// ARGUMENTS
//	const STRING& cstrName_
//	Plan::Scalar::Value* pVariable_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
setSessionVariable(const STRING& cstrName_,
				   Plan::Scalar::Value* pVariable_)
{
	m_mapVariableName[cstrName_] = pVariable_;
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::getSessionVariable -- 
//
// NOTES
//
// ARGUMENTS
//	const STRING& cstrName_
//	
// RETURN
//	Plan::Scalar::Value*
//
// EXCEPTIONS

//virtual
Plan::Scalar::Value*
Impl::EnvironmentImpl::
getSessionVariable(const STRING& cstrName_)
{
	VariableNameMap::Iterator found = m_mapVariableName.find(cstrName_);
	if (found == m_mapVariableName.end()) {
		return 0;
	} else {
		return (*found).second;
	}
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::setDistinctFunction -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IScalar* pOperand_
//	Plan::Interface::IScalar* pDistinct_
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS

//virtual
Plan::Interface::IScalar*
Impl::EnvironmentImpl::
setDistinctFunction(Plan::Interface::IScalar* pOperand_,
					Plan::Interface::IScalar* pDistinct_)
{
	m_mapFunction[pOperand_] = pDistinct_;
	return pDistinct_;
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::getDistinctFunction -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IScalar* pOperand_
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS

//virtual
Plan::Interface::IScalar*
Impl::EnvironmentImpl::
getDistinctFunction(Plan::Interface::IScalar* pOperand_)
{
	FunctionMap::Iterator found = m_mapFunction.find(pOperand_);
	if (found != m_mapFunction.end()) {
		return (*found).second;
	}
	return 0;
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::setNullConstant -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Scalar::Value* pValue_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
setNullConstant(Plan::Scalar::Value* pValue_)
{
	m_pNullConstant = pValue_;
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::setDefaultConstant -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Scalar::Value* pValue_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
setDefaultConstant(Plan::Scalar::Value* pValue_)
{
	m_pDefaultConstant = pValue_;
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::getNullConstant -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Plan::Scalar::Value*
//
// EXCEPTIONS

//virtual
Plan::Scalar::Value*
Impl::EnvironmentImpl::
getNullConstant()
{
	return m_pNullConstant;
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::getDefaultConstant -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Plan::Scalar::Value*
//
// EXCEPTIONS

//virtual
Plan::Scalar::Value*
Impl::EnvironmentImpl::
getDefaultConstant()
{
	return m_pDefaultConstant;
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::addLocator -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IScalar* pScalar_
//	Execution::Interface::IIterator* pIterator_
//	int iLocatorID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::EnvironmentImpl::
addLocator(Plan::Interface::IScalar* pScalar_,
		   Execution::Interface::IIterator* pIterator_,
		   int iLocatorID_)
{
	m_mapLocator[pScalar_->getID()][pIterator_->getID()] = iLocatorID_;
}

// FUNCTION public
//	Opt::Impl::EnvironmentImpl::getLocator -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IScalar* pScalar_
//	Execution::Interface::IIterator* pIterator_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Impl::EnvironmentImpl::
getLocator(Plan::Interface::IScalar* pScalar_,
		   Execution::Interface::IIterator* pIterator_)
{
	return m_mapLocator[pScalar_->getID()][pIterator_->getID()];
}


// FUNCTION private
//	Impl::EnvironmentImpl::destruct -- destructor
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::EnvironmentImpl::
destruct()
{
	_Destruct(m_vecNameMapPool);
	_Destruct(m_vecNodePool);
	_Destruct(m_vecRowInfoPool);
	_Destruct(m_vecObjectPool);

	m_pNullConstant = 0;
	m_pDefaultConstant = 0;
}

// FUNCTION private
//	Opt::Impl::EnvironmentImpl::isCheckObsolete -- 
//
// NOTES
//
// ARGUMENTS
//	const Schema::Table* pSchemaTable_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Impl::EnvironmentImpl::
isCheckObsolete(const Schema::Table* pSchemaTable_)
{
	return pSchemaTable_->isTemporary() == false
		&& pSchemaTable_->isSystem() == false;
}

// FUNCTION private
//	Opt::Impl::EnvironmentImpl::addOuterReferenceToScope -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IRelation* pRelation_
//	Plan::Relation::RowElement* pRowElement_
//	int iScope_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::EnvironmentImpl::
addOuterReferenceToScope(Plan::Interface::IRelation* pRelation_,
						 Plan::Relation::RowElement* pRowElement_,
						 int iScope_)
{
	m_vecName[iScope_].m_cOuterRelation.add(pRelation_);
	m_vecName[iScope_].m_cOuterReference.add(pRowElement_);
}

// FUNCTION private
//	Opt::Impl::EnvironmentImpl::addContainsByAnyOperand -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IScalar* pColumn_
//	Plan::Predicate::Contains* pContains_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::EnvironmentImpl::
addContainsByAnyOperand(Plan::Interface::IScalar* pColumn_,
						Plan::Predicate::Contains* pContains_)
{
	m_mapContainsByAnyOperand[pColumn_].PUSHBACK(pContains_);
}

////////////////////////
//	Opt::Environment

// FUNCTION public
//	Opt::Environment::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const EnvironmentArgument& cArgument_
//	
// RETURN
//	AUTOPOINTER<Environment>
//
// EXCEPTIONS

//static
AUTOPOINTER<Environment>
Environment::
create(const EnvironmentArgument& cArgument_)
{
	return new Impl::EnvironmentImpl(cArgument_);
}

_SYDNEY_OPT_END
_SYDNEY_END

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
