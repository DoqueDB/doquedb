// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/Fulltext.cpp --
// 
// Copyright (c) 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "DExecution::Action";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "DExecution/Action/Fulltext.h"
#include "DExecution/Action/Class.h"

#include "Common/Assert.h"
#include "Common/ObjectPointer.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"
#include "Common/Message.h"

#include "Exception/Unexpected.h"
#include "Exception/NotSupported.h"

#include "Execution/Action/Class.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/Serialize.h"

#include "LogicalFile/FileID.h"

#include "Opt/Explain.h"

#include "Os/Math.h"

#include "Plan/Sql/Argument.h"

#include "Statement/Utility.h"
#include "Utility/Term.h"




_SYDNEY_BEGIN
_SYDNEY_DEXECUTION_BEGIN
_SYDNEY_DEXECUTION_ACTION_BEGIN

namespace
{
	const ModLanguageSet _cLang = ModLanguageSet();
	enum Operator {
		AND,
		OR,
		AND_NOT,
		VALUE_NUM
	};
	
	char* _pszStatementName[] =
	{
		" & ",
		" | ",
		" - "
	};

	Operator _getOperator(Plan::Tree::Node::Type eType)
	{
		switch (eType)
		{
		case Plan::Tree::Node::And: 		return AND;
		case Plan::Tree::Node::Or: 			return OR;
		case Plan::Tree::Node::AndNot: 		return AND_NOT;
		default: 							break;
		}
		_SYDNEY_THROW0(Exception::NotSupported);
	}
}


namespace Impl
{
	////////////////////////////////////
	// CLASS
	//	Action::Impl::FulltextBase --
	//
	// NOTES
	class FulltextBase
		: public Fulltext
	{
		typedef Fulltext Super;
		typedef FulltextBase This;
		
		enum State
		{
			TEXTING,
			EXTRACTING,
			COUNTING,
			EXPAND_EXTRACTING,
			EXPAND_COUNTING,
			FINALIZING
		};

	public:
		class ConditionParticleImpl
			:public Super::ConditionParticle
		{
		
		public:
			ConditionParticleImpl(FulltextBase* pOuter_,
								  Common::WordData* pWord)
				:m_pOuter(pOuter_),
				 m_pWord(pWord),
				 m_dWeight(1)
			{}
			
			ConditionParticleImpl()
				:m_pOuter(0),
				 m_pWord(0),
				 m_dWeight(1)
			{}

			virtual ~ConditionParticleImpl()
			{}

			

			virtual Common::WordData* getWordData() const
			{return m_pWord;}				

			
			virtual const STRING& getTerm() const
			{return m_pWord->getTerm();}
			
			virtual unsigned int getDocumentFrequency() const
			{return m_pWord->getDocumentFrequency();}

			virtual void setDocumentFrequency(unsigned int iDF_)
			{return m_pWord->setDocumentFrequency(iDF_);}
			
			virtual double getWeight() const {return m_dWeight;}

			virtual void setWeight(double dWeight_) {m_dWeight = dWeight_;}

			virtual void setOuter(Fulltext* pOuter_) {m_pOuter = pOuter_;}
			
		private:
			Fulltext* m_pOuter;
			double m_dWeight;
			Common::WordData* m_pWord;
		};

		// constructor
		FulltextBase(const LogicalFile::FileID cFileID)
			: Super(),
			  m_pTerm(0),
			  m_iDocNum(-1),
			  m_cFileID(cFileID),
			  m_vecText(),
			  m_vecLang(),
			  m_vecWord(),
			  m_vecSelection(),
			  m_eState(TEXTING),
			  m_cStream(),
			  m_cExtractor(),
			  m_iLimit(0),
			  m_iRelatedLimit(0),
			  m_iAvgLength(0)
		{}

		FulltextBase()
			: Super(),
			  m_pTerm(0),
			  m_iDocNum(-1),
			  m_cFileID(),
			  m_vecText(),
			  m_vecLang(),
			  m_vecWord(),
			  m_vecSelection(),
			  m_eState(TEXTING),
			  m_cStream(),
			  m_cExtractor(),
			  m_iLimit(0),
			  m_iRelatedLimit(0),
			  m_iAvgLength(0)
		{}
		
		// destructor
		~FulltextBase() {}

	/////////////////////
	// Action::Fulltext::
		virtual void initialize(Execution::Interface::IProgram& cProgram_);
		virtual void terminate(Execution::Interface::IProgram& cProgram_);
		virtual bool isEmpty();
		virtual void setCollectionSize(SIZE iSize_);
		virtual void setAvgLength(SIZE iSize_) {m_iAvgLength = iSize_;}
		virtual SIZE getCollectionSize() {return m_iDocNum;}
		virtual void setExtractor(const ModUnicodeString& cExtractor);

		virtual void setLimit(SIZE iLimit_);
		virtual void setRelatedLimit(SIZE iLimit_);


		virtual bool nextCandidate();
		bool hasNextCandidate();
		
		virtual STRING getCandidateTerm();
		
		virtual void setDocumentFrequency(unsigned int iDF_);
		virtual void setDocumentFrequency(const Common::WordData* pWordData);
		virtual void expandPool(const STRING&  cstrRelavance_, const ModLanguageSet& cLang_);

		virtual bool nextSelection();
		virtual ConditionParticle& getSelection();

		virtual STRING toStatement();
		
		virtual void clear();


		virtual void explain(Opt::Environment* pEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);


	protected:
		void serializeBase(ModArchive& archiver_);
		
		AUTOPOINTER<Utility::Term> m_pTerm;
		
	private:
		virtual void makeTerm() = 0;
		void expandRelatedCandidate();

		SIZE m_iDocNum;
		SIZE m_iAvgLength;		
		LogicalFile::FileID m_cFileID;
		ModVector<STRING> m_vecText;
		ModVector<ModLanguageSet> m_vecLang;
		ModVector<Common::WordData> m_vecWord;
		ModVector<Common::WordData>::Iterator m_iteWord;
		ModVector<ConditionParticleImpl> m_vecSelection;
		State m_eState;
		OSTRSTREAM m_cStream;
		STRING m_cExtractor;
		int m_iRelatedLimit;
		int m_iLimit;
	};

	class Freetext
		: public FulltextBase
	{
	
	public:
		typedef FulltextBase Super;
		typedef Freetext This;

		// constructor
		Freetext(const LogicalFile::FileID cFileID_,
				 const STRING& cstrFreetext_,
				 double dScaleParam_,
				 int iWordLimit)
			: Super(cFileID_),
			  m_cstrFreetext(cstrFreetext_),
			  m_dScaleParam(dScaleParam_),
			  m_iWordLimit(iWordLimit)
		{}

		Freetext()
			: Super(),
			  m_cstrFreetext()
		{}

		virtual ~Freetext(){}


		
		int getClassID() const;

		/////////////////////
		// ModSerializer::
		void serialize(ModArchive& archiver_)
		{
			serializeBase(archiver_);
			archiver_(m_cstrFreetext);
			archiver_(m_iWordLimit);
			archiver_(m_dScaleParam);
		}


	private:
		virtual void makeTerm();

		STRING m_cstrFreetext;
		double m_dScaleParam;
		int m_iWordLimit;
	};
	

	class WordList
		: public FulltextBase
	{
	
	public:
		typedef FulltextBase Super;
		typedef WordList This;

		// constructor
		WordList(const LogicalFile::FileID cFileID_,
				 const Plan::Interface::IScalar* pWordList_)
			: Super(cFileID_),
			  m_vecWordList()
		{
			for (unsigned int i = 0; i < pWordList_->getOperandSize(); i++) {
				const Common::Data* pData =  pWordList_->getOperandAt(i)->getData();
				; _SYDNEY_ASSERT(pData->getType() == Common::DataType::Word);
				const Common::WordData* pWordData =
					_SYDNEY_DYNAMIC_CAST(const Common::WordData*, pData);
				m_vecWordList.pushBack(*pWordData);
			}
		}
		

		WordList()
			: Super(),
			m_vecWordList()
		{}

		virtual ~WordList(){}

		
		int getClassID() const;


		/////////////////////
		// ModSerializer::

		void serialize(ModArchive& archiver_)
		{
			serializeBase(archiver_);
			if (archiver_.isStore()) {
				int n = m_vecWordList.getSize();
				archiver_ << n;
				if (n) {
					Common::OutputArchive& cOut = dynamic_cast<Common::OutputArchive&>(archiver_);
					for (int i = 0; i < n; ++i) {
						archiver_ << m_vecWordList[i];
					}
				}
			} else {
				int n;
				archiver_ >> n;
				m_vecWordList.erase(m_vecWordList.begin(), m_vecWordList.end());
				if (n) {
					m_vecWordList.reserve(n);
					Common::InputArchive& cIn = dynamic_cast<Common::InputArchive&>(archiver_);
					Common::WordData cValue;
					for (int i = 0; i < n; ++i) {
						archiver_ >> cValue;
						m_vecWordList.pushBack(cValue);
					}
				}
			}
		}

	private:
		ModVector<Common::WordData> m_vecWordList;
		virtual void makeTerm();
	};
		

	class SimpleOperand
		:public Fulltext
	{
	public:
		typedef Fulltext Super;
		
		class Node
			:public Execution::Interface::IObject,
			 public Common::ExecutableObject
		
		{
		public:
			typedef  Execution::Interface::IObject Super;
			
			virtual ~Node(){}
			virtual Node* getChild(int iOffset_) = 0;
			virtual int getChildCount() = 0;
			virtual bool isLeaf() = 0;
			virtual Fulltext::ConditionParticle* getLeaf() = 0;
			virtual void getLeafList(Common::LargeVector<ConditionParticle*>& cvecLeaves_) = 0;
			virtual void toStatement(OSTRSTREAM& cStream) = 0;

		protected:
			Node() : IObject(){}
		};

		class CombinatorImpl
			:public Node
		{
		public:
			typedef Node Super;
			typedef CombinatorImpl This;

			CombinatorImpl(Operator eOperator_)
				:Super(),
				 m_eOperator(eOperator_)
			{}

			CombinatorImpl()
				:Super()
			{}			

			virtual ~CombinatorImpl() {}
			virtual Node* getChild(int iOffset_)
			{return m_vecChildren.at(iOffset_).get();}
			
			virtual int getChildCount()
			{return m_vecChildren.getSize();}
			
			virtual bool isLeaf () {return false;}

			virtual void getLeafList(Common::LargeVector<ConditionParticle*>& cvecLeaves_)
			{
				Common::LargeVector<Common::ObjectPointer<Node> >::Iterator ite = m_vecChildren.begin();
				for (; ite < m_vecChildren.end(); ++ite)
					(*ite)->getLeafList(cvecLeaves_);
			}
			
			virtual Fulltext::ConditionParticle* getLeaf()
			{
				_SYDNEY_ASSERT(false);
				_SYDNEY_THROW0(Exception::NotSupported);
			}

			
			virtual void toStatement(OSTRSTREAM& cStream)
			{
				
				if (m_eOperator == OR) cStream << '(';
				char* ope = " ";
				Common::LargeVector<Common::ObjectPointer<Node> >::Iterator ite = m_vecChildren.begin();
				for (; ite < m_vecChildren.end(); ++ite) {
					cStream << ope;
					(*ite)->toStatement(cStream);
					ope = _pszStatementName[m_eOperator];
				}
				if (m_eOperator == OR) cStream << ')';
			}
				

			int getClassID() const
			{return Class::getClassID(Class::Category::FulltextCombinator);}

			void serialize(ModArchive& archiver_)
			{
				serializeID(archiver_);
				Execution::Utility::SerializeEnum(archiver_, m_eOperator);
				if (archiver_.isStore()) {
					int n = m_vecChildren.getSize();
					archiver_ << n;
					if (n) {
						Common::OutputArchive& cOut = dynamic_cast<Common::OutputArchive&>(archiver_);
						for (int i = 0; i < n; ++i) {
							cOut.writeObject(m_vecChildren[i].get());
						}
					}
				} else {
					int n;
					archiver_ >> n;
					m_vecChildren.erase(m_vecChildren.begin(), m_vecChildren.end());
					if (n) {
						m_vecChildren.reserve(n);
						Common::InputArchive& cIn = dynamic_cast<Common::InputArchive&>(archiver_);
						for (int i = 0; i < n; ++i) {
							Common::ObjectPointer<Node> p = dynamic_cast<Node*>(cIn.readObject());
							m_vecChildren.pushBack(p);
						}
					}
				}
			}

			void addOperand(Common::ObjectPointer<Node> pOperand_)
			{
				m_vecChildren.pushBack(pOperand_);
			}

			static This* getInstance(int iCategory_)
			{
				; _SYDNEY_ASSERT(iCategory_ == Class::Category::FulltextCombinator);
				return new CombinatorImpl;
			}

		protected:

			
		private:
			Operator m_eOperator;
			Common::LargeVector <Common::ObjectPointer<Node> > m_vecChildren;
		};

			
		class ConditionParticleImpl
			:public Node,
			 public Fulltext::ConditionParticle
		{
		
		public:
			ConditionParticleImpl(STRING& cstrStatement_)
				:Node(),
				 Fulltext::ConditionParticle(),
				 m_pOuter(0),
				 m_cstrStatement(cstrStatement_),
				 m_iDF(0),
				 m_dWeight(1)
			{}


			ConditionParticleImpl()
				:Node(),
				 Fulltext::ConditionParticle(),
				 m_pOuter(0),
				 m_cstrStatement(),
				 m_iDF(0),
				 m_dWeight(1)
			{}
			

			virtual ~ConditionParticleImpl()
			{}
			

			virtual Common::WordData* getWordData() const
			{_SYDNEY_THROW0(Exception::NotSupported);}
			
			virtual const STRING& getTerm() const
			{return m_cstrStatement;}
			
			virtual unsigned int getDocumentFrequency() const
			{return m_iDF;}

			virtual void setDocumentFrequency(unsigned int iDF_)
			{m_iDF = iDF_;}
			
			virtual double getWeight() const {return m_dWeight;}

			virtual void setWeight(double dWeight_) {m_dWeight = dWeight_;}

			virtual void setOuter(Fulltext* pOuter_) {m_pOuter = pOuter_;}

			int getClassID() const
			{return Class::getClassID(Class::Category::FulltextParticle);}
			
			void serialize(ModArchive& archiver_)
			{
				serializeID(archiver_);
				archiver_(m_cstrStatement);
				archiver_(m_iDF);
				archiver_(m_dWeight);
			}

			static ConditionParticleImpl* getInstance(int iCategory_)
			{
				; _SYDNEY_ASSERT(iCategory_ == Class::Category::FulltextParticle);
				return new ConditionParticleImpl;
			}			

			// Node
			virtual Node* getChild(int iOffset_) {_SYDNEY_THROW0(Exception::NotSupported);}
			virtual int getChildCount() {return 0;}
			virtual bool isLeaf() {return true;}
			virtual Fulltext::ConditionParticle* getLeaf() {return this;}
			virtual void getLeafList(Common::LargeVector<ConditionParticle*>& cvecLeaves_)
			{cvecLeaves_.pushBack(this);}
			virtual void toStatement(OSTRSTREAM& cStream)
			{

				cStream << " weight(" << getTerm() << " scale ";
				double weight = 1.0;
				if (getDocumentFrequency() != 0)
					weight =
						Os::Math::log(1.0 + 0.2 * m_pOuter->getCollectionSize() / getDocumentFrequency()) /
						Os::Math::log(1.0 + 0.2 * m_pOuter->getCollectionSize());
				cStream << weight * getWeight()<< ")";
			}

		private:
			Fulltext* m_pOuter;			
			STRING m_cstrStatement;
			SIZE m_iDF;
			double m_dWeight;
		};
		
		// constructor
		SimpleOperand(const LogicalFile::FileID cFileID)
			: Super(),
			  m_iDocNum(-1),
			  m_iAvgLength(0),
			  m_pRootNode()
		{}

		SimpleOperand()
			: Super(),
			  m_iDocNum(-1),
			  m_iAvgLength(0),
			  m_pRootNode()
		{}
		
		// destructor
		~SimpleOperand() {}


		void build(Opt::Environment& cEnvironment_, const Plan::Interface::IScalar* pOperand_) {
			Plan::Sql::QueryArgument cArgument;
			m_pRootNode = doBuild(cEnvironment_, pOperand_, cArgument);
		}

		Node* doBuild(Opt::Environment& cEnvironment_,
					  const Plan::Interface::IScalar* pOperand_,
					  Plan::Sql::QueryArgument& cArgument_);
		
	/////////////////////
	// Action::Fulltext::
		virtual void initialize(Execution::Interface::IProgram& cProgram_);
		virtual void terminate(Execution::Interface::IProgram& cProgram_);
		virtual bool isEmpty();
		virtual void setCollectionSize(SIZE iSize_) {m_iDocNum = iSize_;}
		virtual void setAvgLength(SIZE iSize_) {m_iAvgLength = iSize_;}		
		virtual SIZE getCollectionSize() {return m_iDocNum;}
		virtual void setExtractor(const ModUnicodeString& cExtractor);
		virtual void setLimit(SIZE iLimit_);
		virtual void setRelatedLimit(SIZE iLimit_);

		virtual bool nextCandidate();
		bool hasNextCandidate();


		virtual STRING getCandidateTerm();
		
		virtual void setDocumentFrequency(unsigned int iDF_);
		
		virtual void setDocumentFrequency(const Common::WordData* pWordData)
		{_SYDNEY_THROW0(Exception::NotSupported);}
		
		virtual void expandPool(const STRING&  cstrRelavance_, const ModLanguageSet& cLang_);

		virtual bool nextSelection();
		virtual ConditionParticle& getSelection();

		virtual STRING toStatement();
		
		virtual void clear();

		virtual void explain(Opt::Environment* pEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
	///////////////////////////////
	// Common::Externalizable
		int getClassID() const
		{return Class::getClassID(Class::Category::SimpleOperand);}
		
		static This* getInstance(int iCategory_)
		{
			; _SYDNEY_ASSERT(iCategory_ == Class::Category::SimpleOperand);
			return new SimpleOperand;
		}

	/////////////////////
	// ModSerializer::
		void serialize(ModArchive& archiver_);

	protected:

	private:
		SIZE m_iDocNum;
		SIZE m_iAvgLength;		
		Common::ObjectPointer<Node> m_pRootNode;
		Common::LargeVector<Fulltext::ConditionParticle*> m_vecLeaves;
		Common::LargeVector<Fulltext::ConditionParticle*>::ConstIterator m_iteCandidate;
		Common::LargeVector<Fulltext::ConditionParticle*>::ConstIterator m_iteSelection;
	};

} // namespace Impl

///////////////////////////////////////////////
// DExecution::Action::Impl::FulltextBase

// FUNCTION public
//	Action::Impl::FulltextBase::initialize -- initialize
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::FulltextBase::
initialize(Execution::Interface::IProgram& cProgram_)
{
	// do nothing
}

// FUNCTION public
//	Action::Impl::FulltextBase::terminate -- terminate
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::FulltextBase::
terminate(Execution::Interface::IProgram& cProgram_)
{
	clear();
}


// FUNCTION public
//	Action::Impl::FulltextBase::isEmpty --- isEmpty
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

bool
Impl::FulltextBase::
isEmpty()
{
	return (m_vecText.getSize() == 0);
}



// FUNCTION public
//	Action::Impl::FulltextBase::setCollectionSize -- 全文索引の登録件数をセットする
//
// NOTES
//	
//
// ARGUMENTS
//	SIZE iSize_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
//
// virtual

void
Impl::FulltextBase::
setCollectionSize(SIZE iSize_)
{
	; _SYDNEY_ASSERT(m_eState == TEXTING);
	m_pTerm = new Utility::Term(m_cFileID, iSize_);
	m_pTerm->setExtractor(m_cExtractor);
	m_pTerm->setLimit(m_iLimit);
	if (m_iRelatedLimit > 0) {
		m_pTerm->setRelatedLimit(m_iRelatedLimit);
	}
	makeTerm();
	m_iDocNum = iSize_;
	m_pTerm->getCandidate(m_vecWord);
	m_iteWord = m_vecWord.begin();
}




// FUNCTION public
//	Action::Impl::FulltextBase::setExtractor -- 質問処理機パラメータを設定する
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cExtractor_
//		質問処理機パラメーター
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
//
// virtual

void
Impl::FulltextBase::
setExtractor(const ModUnicodeString& cExtractor)
{
	m_cExtractor = cExtractor;
}


// FUNCTION public
//	Action::Impl::FulltextBase::setLimit -- リミットを設定する
//
// NOTES
//
// ARGUMENTS
//	SIZE iLimit_
//		リミットサイズ
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
//
// virtual

void
Impl::FulltextBase::
setLimit(SIZE iLimit_)
{
	m_iLimit = iLimit_;
}



// FUNCTION public
//	Action::Impl::FulltextBase::setRelatedLimit -- 関連文書のリミットを設定する
//
// NOTES
//
// ARGUMENTS
//	SIZE iLimit_
//		リミットサイズ
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
//
// virtual

void
Impl::FulltextBase::
setRelatedLimit(SIZE iRelatedLimit_)
{
	m_iRelatedLimit = iRelatedLimit_;
}


// FUNCTION public
//	Action::Impl::FulltextBase::nextCandidate 候補を得る
//
// NOTES
//
// ARGUMENTS
//	const ModVector<Common::WordData>& vecCandidate_
//		候補
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
//
// virtual

bool
Impl::FulltextBase::
nextCandidate()
{
	;_SYDNEY_ASSERT(m_pTerm != 0);
	switch (m_eState) {
	case  TEXTING: 
		if (m_iteWord == m_vecWord.end()) {
			expandRelatedCandidate();
		} else {
			m_eState = EXTRACTING;
		}
		break;
		
	case COUNTING:
		expandRelatedCandidate();
		break;
	case EXTRACTING:
	case EXPAND_EXTRACTING:
		m_iteWord++;
		break;
	case EXPAND_COUNTING:
		break;
	default:
		_SYDNEY_THROW0(Exception::Unexpected);
	}
		
	return m_eState != EXPAND_COUNTING;
}

bool
Impl::FulltextBase::
hasNextCandidate()
{
	;_SYDNEY_ASSERT(m_eState == EXPAND_EXTRACTING || m_eState == EXTRACTING);
	return (m_iteWord != m_vecWord.end() && (m_iteWord + 1 != m_vecWord.end()));
}

// FUNCTION public
//	Action::Impl::FulltextBase::getCandidateTerm 候補を得る
//
// NOTES
//
// ARGUMENTS
//	
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
//
// virtual

STRING
Impl::FulltextBase::
getCandidateTerm()
{
	;_SYDNEY_ASSERT(m_eState == EXTRACTING || m_eState == EXPAND_EXTRACTING);
	return m_iteWord->getTerm();
}



// FUNCTION public
//	Action::Impl::FulltextBase::setDocumentFrequency -- DF値を設定する
//
// NOTES
//
// ARGUMENTS
//	const ModVector<Common::WordData>& vecCandidate_
//		単語リスト
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
//
// virtual

void
Impl::FulltextBase::
setDocumentFrequency(unsigned int iDF_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Action::Impl::FulltextBase::setDocumentFrequency -- DF値を設定する
//
// NOTES
//
// ARGUMENTS
//	const ModVector<Common::WordData>& vecCandidate_
//		単語リスト
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
//
// virtual

void
Impl::FulltextBase::
setDocumentFrequency(const Common::WordData* pWordData)
{
	
	if (m_eState == EXTRACTING) {
		m_eState = COUNTING;
		m_vecWord.clear();
	} else if (m_eState == EXPAND_EXTRACTING) {
		m_eState = EXPAND_COUNTING;
		m_vecWord.clear();
	} else {
		;_SYDNEY_ASSERT(m_eState == COUNTING || m_eState == EXPAND_COUNTING);
	}

	m_vecWord.pushBack(*pWordData);
}


// FUNCTION public
//	Action::Impl::FulltextBase::expandPool 関連文書から単語を拡張する
//
// NOTES
//
// ARGUMENTS
//	const ModVector<Common::WordData>& vecCandidate_
//		単語リスト
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
//
// virtual

void
Impl::FulltextBase::
expandPool(const STRING& cstrRelavance_, const ModLanguageSet& cLang_)
{
	if (m_pTerm == 0) _SYDNEY_THROW0(Exception::Unexpected);
	; _SYDNEY_ASSERT(m_eState == TEXTING);
	m_vecText.pushBack(cstrRelavance_);
	m_vecLang.pushBack(cLang_);

}


// FUNCTION public
//	Action::Impl::FulltextBase::nextSelection
//
// NOTES
//
// ARGUMENTS
//	const Common::WordData& vecCandidate_
//		単語リスト
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
//
// virtual

bool
Impl::FulltextBase::
nextSelection()
{
	if (m_eState == FINALIZING) {
		m_iteWord++;
	} else if (m_eState == EXPAND_COUNTING) {
		m_pTerm->setRelatedDocumentFrequency(m_vecWord);
		m_vecWord.clear();
		m_pTerm->getSelection(m_vecWord);
		m_iteWord = m_vecWord.begin();
		m_eState = FINALIZING;
	}
	
	; _SYDNEY_ASSERT(m_eState == FINALIZING);

	if (m_iteWord != m_vecWord.end()) {
		Common::WordData* pResult = &(*m_iteWord);
		m_vecSelection.pushBack(ConditionParticleImpl(this, pResult));
		return true;
	} else {
		return false;
	}
}

// FUNCTION public
//	Action::Impl::FulltextBase::getSelection
//
// NOTES
//
// ARGUMENTS
//	const Common::WordData& vecCandidate_
//		単語リスト
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
//
// virtual

Fulltext::ConditionParticle&
Impl::FulltextBase::
getSelection()
{
	return m_vecSelection.GETBACK();
}

// FUNCTION public
//	Action::Impl::FulltextBase::toStatement -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	STRING&
//
// EXCEPTIONS

//virtual
STRING
Impl::FulltextBase::
toStatement()
{
	if (m_eState == EXPAND_COUNTING) {
		m_pTerm->setRelatedDocumentFrequency(m_vecWord);
		m_vecWord.clear();
		m_pTerm->getSelection(m_vecWord);
		m_iteWord = m_vecWord.begin();
		m_eState = FINALIZING;
	}
	
	OSTRSTREAM cStream;
	ModVector<Common::WordData>::Iterator iteWord = m_vecWord.begin();
	char* ope = " ";
	for (; iteWord != m_vecWord.end(); ++iteWord) {
		cStream << ope << " weight(" << m_pTerm->getFormula((*iteWord)) << " scale ";
		double weight = 1.0;
		if (iteWord->getDocumentFrequency() != 0) {
			weight = 
				Os::Math::log(1.0 + 0.2 * m_iDocNum / iteWord->getDocumentFrequency()) /
				Os::Math::log(1.0 + 0.2 * m_iDocNum);
		}
		if (iteWord->getScale() != 0) weight = weight * iteWord->getScale();
		cStream << weight << ")";
		ope = " |";
	}
	cStream << " calculator 'NormalizedOkapiTf' ";
	if (m_iAvgLength != 0) {
		cStream << " average length " << m_iAvgLength;
	}

	return cStream.getString();
}


// FUNCTION public
//	Action::Impl::FulltextBase::clear -- 
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
Impl::FulltextBase::
clear()
{
	m_pTerm = 0;
}




// FUNCTION public
//	Action::Impl::FulltextBase::serialize -- serialize
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
Impl::FulltextBase::
serializeBase(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cFileID.serialize(archiver_);
	archiver_(m_iLimit);
	Execution::Utility::SerializeEnum(archiver_, m_eState);
}


// FUNCTION public
//	Action::Impl::FulltextBase::explain -- explain
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Opt::Execution& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::FulltextBase::
explain(Opt::Environment* pEnvironment_,
		Execution::Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(" fulltext file id ");
}

// FUNCTION private
//	Action::Impl::FulltextBase::expandRelatedCandidate -- expandRelatedCandidate
//
// NOTES
//
// ARGUMENTS
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::FulltextBase::
expandRelatedCandidate()
{
	m_pTerm->setDocumentFrequency(m_vecWord);
	m_pTerm->expandPool(m_vecText, m_vecLang);
	m_vecWord.clear();
	m_pTerm->getRelatedCandidate(m_vecWord);
	m_iteWord = m_vecWord.begin();
	if (m_iteWord == m_vecWord.end()) {
		m_pTerm->setRelatedDocumentFrequency(m_vecWord);
		m_vecWord.clear();
		m_pTerm->getSelection(m_vecWord);
		m_iteWord = m_vecWord.begin();
		m_eState = EXPAND_COUNTING;
	} else {
		m_eState =EXPAND_EXTRACTING;
	}
}


////////////////////////////////////
// Execution::Action::Impl::Freetext::

// FUNCTION public
//	Action::Impl::Freetext::makePool 自然文からプールを作成する
//
// NOTES
//
// ARGUMENTS
//	const LogicalFile::FileID cFileID_
//	SIZE iSize_ 総文書数
//		
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
//
// virtual

void
Impl::Freetext::
makeTerm()
{
	m_pTerm->setWordLimit(m_iWordLimit);
	m_pTerm->setScaleParameter(m_dScaleParam);
	m_pTerm->makePool(m_cstrFreetext, _cLang);
}



// FUNCTION public
//	Action::Impl::Freetext::getClassID -- 
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
Impl::Freetext::
getClassID() const
{
	return Class::getClassID(Class::Category::Freetext);
}


////////////////////////////////////
// Execution::Action::Fulltext::Impl::WordList

// FUNCTION public
//	Action::Impl::WordList::makePool 単語リストからプールを作成する
//
// NOTES
//
// ARGUMENTS
//	const ModVector<Common::WordData>&
//		単語リスト
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
//
// virtual

void
Impl::WordList::
makeTerm()
{
	m_pTerm->makePool(m_vecWordList);
}


// FUNCTION public
//	Action::Impl::WordList::getClassID -- 
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
Impl::WordList::
getClassID() const
{
	return Class::getClassID(Class::Category::WordList);
}



///////////////////////////////////////////////
// DExecution::Action::Impl::SimpleOperand

// FUNCTION public
//	Action::Impl::SimpleOperand::initialize -- initialize
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::SimpleOperand::
initialize(Execution::Interface::IProgram& cProgram_)
{
	if (m_vecLeaves.isEmpty()) {
		m_pRootNode->getLeafList(m_vecLeaves);
		Common::LargeVector<ConditionParticle*>::ConstIterator ite = m_vecLeaves.begin();
		for (;ite < m_vecLeaves.end(); ++ite) {
			(*ite)->setOuter(this);
		}
	}
}

// FUNCTION public
//	Action::Impl::SimpleOperand::terminate -- terminate
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::SimpleOperand::
terminate(Execution::Interface::IProgram& cProgram_)
{
	
}


// FUNCTION public
//	Action::Impl::SimpleOperand::isEmpty --- isEmpty
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

bool
Impl::SimpleOperand::
isEmpty()
{
	return (m_pRootNode->getChildCount() == 0 && !m_pRootNode->isLeaf());
}


// FUNCTION public
//	Action::Impl::SimpleOperand::setExtractor -- 質問処理機パラメータを設定する
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cExtractor_
//		質問処理機パラメーター
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
//
// virtual

void
Impl::SimpleOperand::
setExtractor(const ModUnicodeString& cExtractor)
{
	
}


// FUNCTION public
//	Action::Impl::SimpleOperand::setLimit -- リミットを設定する
//
// NOTES
//	Wordに対するLimitではないため、設定の必要はない.
//
//
// ARGUMENTS
//	SIZE iLimit_
//		リミットサイズ
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
//
// virtual

void
Impl::SimpleOperand::
setLimit(SIZE iLimit_)
{
	; // noting todo
}



// FUNCTION public
//	Action::Impl::SimpleOperand::setRelatedLimit -- 関連文書のリミットを設定する
//
// NOTES
//
// ARGUMENTS
//	SIZE iLimit_
//		リミットサイズ
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
//
// virtual

void
Impl::SimpleOperand::
setRelatedLimit(SIZE iLimit_)
{
	;_SYDNEY_ASSERT(false);
}


// FUNCTION public
//	Action::Impl::SimpleOperand::nextCandidate 候補を得る
//
// NOTES
//
// ARGUMENTS
//	const ModVector<Common::WordData>& vecCandidate_
//		候補
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
//
// virtual

bool
Impl::SimpleOperand::
nextCandidate()
{
	;_SYDNEY_ASSERT(!m_vecLeaves.isEmpty());
	if (!m_iteCandidate.isValid()) {
		m_iteCandidate = m_vecLeaves.begin();
	} else {
		++m_iteCandidate;
	}

	return (m_iteCandidate != m_vecLeaves.end());
}


// FUNCTION public
//	Action::Impl::SimpleOperand::hasNextCandidate 次の候補が存在するかを確認する
//
// NOTES
//
// ARGUMENTS
//	
// RETURN
//	bool
//
// EXCEPTIONS
//
// virtual

bool
Impl::SimpleOperand::
hasNextCandidate()
{
	;_SYDNEY_ASSERT(!m_vecLeaves.isEmpty());
	if (!m_iteCandidate.isValid()) {
		m_iteCandidate = m_vecLeaves.begin();
	}

	return (m_iteCandidate != m_vecLeaves.end() && m_iteCandidate + 1 != m_vecLeaves.end());
}

// FUNCTION public
//	Action::Impl::SimpleOperand::getCandidateTerm 候補を得る
//
// NOTES
//
// ARGUMENTS
//	
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
//
// virtual

STRING
Impl::SimpleOperand::
getCandidateTerm()
{
	return (*m_iteCandidate)->getTerm();
}



// FUNCTION public
//	Action::Impl::SimpleOperand::setDocumentFrequency -- DF値を設定する
//
// NOTES
//
// ARGUMENTS
//	const ModVector<Common::WordData>& vecCandidate_
//		単語リスト
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
//
// virtual

void
Impl::SimpleOperand::
setDocumentFrequency(unsigned int iDF_)
{
	if (!m_iteSelection.isValid()) {
		m_iteSelection = m_vecLeaves.begin();
	}
	(*m_iteSelection)->setDocumentFrequency(iDF_);
	m_iteSelection++;
	if (m_iteSelection == m_vecLeaves.end()) {
		// getSelectonのための準備
		m_iteSelection = m_vecLeaves.begin();
	}
}


// FUNCTION public
//	Action::Impl::SimpleOperand::expandPool 関連文書から単語を拡張する
//
// NOTES
//
// ARGUMENTS
//	const ModVector<Common::WordData>& vecCandidate_
//		単語リスト
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
//
// virtual

void
Impl::SimpleOperand::
expandPool(const STRING& cstrRelavance_, const ModLanguageSet& cLang_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}


// FUNCTION public
//	Action::Impl::SimpleOperand::nextSelection
//
// NOTES
//
// ARGUMENTS
//	const Common::WordData& vecCandidate_
//		単語リスト
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
//
// virtual

bool
Impl::SimpleOperand::
nextSelection()
{
	bool bResult =  (m_iteSelection != m_vecLeaves.end());
	m_iteSelection++;
	return bResult;
}

// FUNCTION public
//	Action::Impl::SimpleOperand::getSelection
//
// NOTES
//
// ARGUMENTS
//	const Common::WordData& vecCandidate_
//		単語リスト
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
//
// virtual

Fulltext::ConditionParticle&
Impl::SimpleOperand::
getSelection()
{
	return **m_iteSelection;
}

// FUNCTION public
//	Action::Impl::SimpleOperand::toStatement -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	STRING&
//
// EXCEPTIONS

//virtual
STRING
Impl::SimpleOperand::
toStatement()
{
	OSTRSTREAM cStream;
	m_pRootNode->toStatement(cStream);
	cStream << " calculator 'NormalizedOkapiTf' ";
	if (m_iAvgLength != 0) {
		cStream << " average length " << m_iAvgLength;
	}
	return cStream.getString();
}


// FUNCTION public
//	Action::Impl::SimpleOperand::clear -- 
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
Impl::SimpleOperand::
clear()
{
	
}



// FUNCTION public
//	Action::Impl::SimpleOperand::doBuild
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
Impl::SimpleOperand::Node*
Impl::SimpleOperand::doBuild(Opt::Environment& cEnvironment_,
							 const Plan::Interface::IScalar* pOperand_,
							 Plan::Sql::QueryArgument& cArgument_) {

	switch (pOperand_->getType())
	{
	case Plan::Tree::Node::And:
	case Plan::Tree::Node::Or:
	case Plan::Tree::Node::AndNot:
	{
		AUTOPOINTER<CombinatorImpl> pResult =
			new CombinatorImpl(_getOperator(pOperand_->getType()));
		for (unsigned int i = 0; i < pOperand_->getOperandSize(); ++i) {
			pResult->addOperand(doBuild(cEnvironment_,
										_SYDNEY_DYNAMIC_CAST(const Plan::Interface::IScalar*,
															 pOperand_->getOperandAt(i)),
										cArgument_));
		}
		return pResult.release();
	}
			
	case Plan::Tree::Node::ExactWord:
	case Plan::Tree::Node::ExpandSynonym:
	case Plan::Tree::Node::Head:
	case Plan::Tree::Node::Pattern:
	case Plan::Tree::Node::SimpleWord:
	case Plan::Tree::Node::String:
	case Plan::Tree::Node::Synonym:		
	case Plan::Tree::Node::Tail:
	case Plan::Tree::Node::Within:				
	case Plan::Tree::Node::WordHead:
	case Plan::Tree::Node::WordTail:
	{
		STRING cstrStatement(pOperand_->toSQLStatement(cEnvironment_,cArgument_));
		return new ConditionParticleImpl(cstrStatement);
	}

	case Plan::Tree::Node::Weight:
	{
		;_SYDNEY_ASSERT(pOperand_->getOperandSize() == 1);
		AUTOPOINTER<Node> pNode = doBuild(cEnvironment_,
										  _SYDNEY_DYNAMIC_CAST(const Plan::Interface::IScalar*,
															   pOperand_->getOperandAt(0)),
										  cArgument_);
		if (pOperand_->getOptionSize() == 1) {
			;_SYDNEY_ASSERT(pNode->isLeaf());
			const Common::DoubleData* pScale =
				_SYDNEY_DYNAMIC_CAST(const Common::DoubleData*, pOperand_->getOptionAt(0)->getData());
			pNode->getLeaf()->setWeight(pScale->getValue());
		}
		return pNode.release();
	}
				
	default:
		; _SYDNEY_ASSERT(false);
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}


// FUNCTION public
//	Action::Impl::SimpleOperand::serialize -- serialize
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
Impl::SimpleOperand::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	if (archiver_.isStore()) {
		Common::OutputArchive& cOut = dynamic_cast<Common::OutputArchive&>(archiver_);
		cOut.writeObject(m_pRootNode.get());
	} else {
		Common::InputArchive& cIn = dynamic_cast<Common::InputArchive&>(archiver_);
		m_pRootNode = dynamic_cast<Node*>(cIn.readObject());
	}
}


// FUNCTION public
//	Action::Impl::SimpleOperand::serialize -- serialize
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
Impl::SimpleOperand::
explain(Opt::Environment* pEnvironment_,
		Execution::Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(" fulltext file id ");
}




////////////////////////////////////
// Execution::Action::Fulltext

// FUNCTION public
//	Action::Fulltext::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Fulltext*
//
// EXCEPTIONS

//static
Fulltext*
Fulltext::
create(Opt::Environment& cEnvironment_,
	   Execution::Interface::IProgram& cProgram_,
	   const LogicalFile::FileID& cFulltextID_,
	   const Plan::Interface::IScalar* pOperand_)
{
	AUTOPOINTER<This> pResult;
	Plan::Tree::Node::Type eType = pOperand_->getType();
	switch (pOperand_->getType()) {
	case Plan::Tree::Node::Freetext:
	{
		const ModUnicodeString& cstrFreetext =
			pOperand_->getOperandAt(0)->getData()->toString();

		int iWordLimit = 0;
		double dScaleParam = 0;
		for (int i = 0; i < static_cast<int> (pOperand_->getOptionSize()); ++i) {
			const LogicalFile::TreeNodeInterface* pOption = pOperand_->getOptionAt(i);
			if (pOption->getType() == LogicalFile::TreeNodeInterface::ScaleParameter) {
				dScaleParam = ModUnicodeCharTrait::toDouble(pOption->getValue());
			} else if (pOption->getType() == LogicalFile::TreeNodeInterface::WordLimit) {
				iWordLimit = ModUnicodeCharTrait::toInt(pOption->getValue());
			}
		}
		
		pResult = new Impl::Freetext(cFulltextID_, cstrFreetext, dScaleParam, iWordLimit);
		break;
	}
	case Plan::Tree::Node::WordList:
	{
		pResult = new Impl::WordList(cFulltextID_, pOperand_);
		break;
	}
	default:
	{
		AUTOPOINTER<Impl::SimpleOperand> pData = new Impl::SimpleOperand(cFulltextID_); 
		pData->build(cEnvironment_, pOperand_);
		pResult = pData.release();
	}
	}
	
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Action::Fulltext::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Fulltext*
//
// EXCEPTIONS

//static
Execution::Interface::IObject*
Fulltext::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::Freetext:
	{
		return new Impl::Freetext;
	}
	case Class::Category::WordList:
	{
		return new Impl::WordList;
	}
	case Class::Category::SimpleOperand:
	{
		return new Impl::SimpleOperand;
	}
	case Class::Category::FulltextParticle:
	{
		return Impl::SimpleOperand::ConditionParticleImpl::getInstance(iCategory_);
	}
	case Class::Category::FulltextCombinator:
	{
		return Impl::SimpleOperand::CombinatorImpl::getInstance(iCategory_);
	}
	default:
	{
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	}
}

// FUNCTION protected
//	Action::Fulltext::registerToProgram -- register to program
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Fulltext::
registerToProgram(Execution::Interface::IProgram& cProgram_)
{
	// Instance ID is obtained by registerFulltext method.
	setID(cProgram_.registerFulltext(this));
}

//////////////////////////////////////////
// Execution::Action::FulltextHolder

// FUNCTION public
//	Action::FulltextHolder::initialize -- initialize Fulltext instance
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FulltextHolder::
initialize(Execution::Interface::IProgram& cProgram_)
{
	if (!isInitialized()) {
		m_pFulltext =
			_SYDNEY_DYNAMIC_CAST(Fulltext*, cProgram_.getFulltext(m_iID));
		if (m_pFulltext == 0) {
			_SYDNEY_THROW0(Exception::Unexpected);
		}
		m_pFulltext->initialize(cProgram_);
	}
}

// FUNCTION public
//	Action::FulltextHolder::terminate -- terminate Fulltext instance
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FulltextHolder::
terminate(Execution::Interface::IProgram& cProgram_)
{
	if (m_pFulltext) {
		m_pFulltext->terminate(cProgram_);
		m_pFulltext = 0;
	}
}

// FUNCTION public
//	Action::FulltextHolder::clear -- clear Fulltext instance
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
FulltextHolder::
clear()
{
	if (m_pFulltext) {
		m_pFulltext->clear();
	}
}

// FUNCTION public
//	Action::FulltextHolder::serialize -- serializer
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
FulltextHolder::
serialize(ModArchive& archiver_)
{
	archiver_(m_iID);
}

// FUNCTION public
//	Action::FulltextHolder::explain -- explain
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FulltextHolder::
explain(Opt::Environment* pEnvironment_,
		Execution::Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	if (m_pFulltext) {
		m_pFulltext->explain(pEnvironment_, cProgram_, cExplain_);
	} else {
		cProgram_.getFulltext(m_iID)->explain(pEnvironment_, cProgram_, cExplain_);
	}
}

_SYDNEY_DEXECUTION_ACTION_END
_SYDNEY_DEXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
