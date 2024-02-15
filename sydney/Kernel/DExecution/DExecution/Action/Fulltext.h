// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Action/Fulltext.h --
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

#ifndef __SYDNEY_DEXECUTION_ACTION_FULLTEXT_H
#define __SYDNEY_DEXECUTION_ACTION_FULLTEXT_H



#include "DExecution/Action/Module.h"

#include "Common/WordData.h"

#include "Execution/Action/ActionList.h"
#include "Execution/Interface/IObject.h"

#include "Plan/Interface/IScalar.h"


#include "DExecution/Action/Module.h"

_SYDNEY_BEGIN
_SYDNEY_DEXECUTION_BEGIN
_SYDNEY_DEXECUTION_ACTION_BEGIN

//////////////////////////////////////////////////////////////////
//	CLASS
//	DExecution::Action::Fulltext -- Base class for the classes which represents thread
//
//	NOTES
//		This class is not constructed directly
class Fulltext
	: public Execution::Interface::IObject
{
public:
	typedef Execution::Interface::IObject Object;
	typedef Fulltext This;

	class ConditionParticle
	{
	public:
		ConditionParticle(){}

		virtual ~ConditionParticle(){};

		virtual Common::WordData* getWordData() const = 0;
		virtual const STRING& getTerm() const  = 0;
		virtual unsigned int getDocumentFrequency() const = 0;
		virtual void setDocumentFrequency(unsigned int iDF_) = 0;
 		virtual double getWeight() const = 0;
		virtual void setWeight(double dWeight_) = 0;
		virtual void setOuter(Fulltext* pFulltext_) = 0;
	};


	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Execution::Interface::IProgram& cProgram_,
						const LogicalFile::FileID&,
						const Plan::Interface::IScalar* pOperand_);
	
	// destructor
	~Fulltext() {};

	// initialize
	virtual void initialize(Execution::Interface::IProgram& cProgram_) = 0;
	virtual void terminate(Execution::Interface::IProgram& cProgram_) = 0;
	virtual void setCollectionSize(SIZE iSize_) = 0;
	virtual void setAvgLength(SIZE iSize_) = 0;

	virtual SIZE getCollectionSize() = 0;

	virtual bool isEmpty() = 0;
	
	virtual void setExtractor(const ModUnicodeString& cExtractor) = 0;
	// リミットを設定する(オプション)
	virtual void setLimit(SIZE uiLimit_) = 0;

	virtual void setRelatedLimit(SIZE uiLimit_) = 0;

	virtual STRING getCandidateTerm() = 0;
	
	// 候補を得る
	virtual bool nextCandidate() = 0;
	
	virtual bool hasNextCandidate() = 0;	

	// DF値を設定する
	virtual void setDocumentFrequency(unsigned int iDF_) = 0;

	virtual void setDocumentFrequency(const Common::WordData* pWordData)	= 0;
	
	// 関連文書から単語を拡張する
	virtual void expandPool(const STRING&  cstrRelavance_,
							const ModLanguageSet& cLang_) = 0;


	// 最終結果を得る
	virtual bool nextSelection() = 0;
	virtual ConditionParticle& getSelection() = 0;

	virtual STRING toStatement() = 0;
	
	// 内部状態をクリアする
	virtual void clear() = 0;

	virtual void explain(Opt::Environment* pEnvironment_,
						 Execution::Interface::IProgram& cProgram_,
						 Opt::Explain& cExplain_) = 0;	

	// for serialize
	static Execution::Interface::IObject* getInstance(int iCategory_);



///////////////////////////////
// Common::Externalizable
//	int getClassID() const;

///////////////////////////////
// ModSerializer
//	void serialize(ModArchive& archiver_);

protected:
	// constructor
	Fulltext() {}

	virtual void registerToProgram(Execution::Interface::IProgram& cProgram_);	

private:

};

///////////////////////////////////
// CLASS
//	Action::FulltextHolder --
//
// NOTES

class FulltextHolder
	: public Common::Object
{
public:
	typedef Common::Object Super;
	typedef FulltextHolder This;

	FulltextHolder()
		: Super(),
		  m_iID(-1),
		  m_pFulltext(0)
	{}
	
	FulltextHolder(int iID_)
		: Super(),
		  m_iID(iID_),
		  m_pFulltext(0)
	{}
	~FulltextHolder() {}



	int getID() {return m_iID;}
	void setID(int iID_) {m_iID = iID_;}

	// initialize Fulltext instance
	void initialize(Execution::Interface::IProgram& cProgram_);
	// terminate Fulltext instance
	void terminate(Execution::Interface::IProgram& cProgram_);
	// clear Locator instance
	void clear();

	// -> operator
	Fulltext* operator->() const {return m_pFulltext;}

	// accessor
	Fulltext* get() const {return m_pFulltext;}
	bool isInitialized() {return m_pFulltext != 0;}

	// serializer
	void serialize(ModArchive& archiver_);

	// explain
	void explain(Opt::Environment* pEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Opt::Explain& cExplain_);


protected:

	
private:
	int m_iID;
	Fulltext* m_pFulltext;

};

_SYDNEY_DEXECUTION_ACTION_END
_SYDNEY_DEXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_DEXECUTION_ACTION_TERM_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
