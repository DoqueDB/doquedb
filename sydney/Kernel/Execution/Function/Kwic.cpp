// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Kwic.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Execution::Function";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Function/Kwic.h"
#include "Execution/Function/Class.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/Serialize.h"

#include "Common/Assert.h"
#include "Common/IntegerData.h"
#include "Common/StringData.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

#include "Utility/Kwic.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_FUNCTION_BEGIN

namespace Impl
{
	// CLASS local
	//	Execution::Function::Impl::KwicImpl -- implementation classes of Kwic
	//
	// NOTES
	class KwicImpl
		: public Function::Kwic
	{
	public:
		typedef KwicImpl This;
		typedef Function::Kwic Super;

		struct Property
		{
			// property key and value is arraydata created at kwicoption in plan
			Action::ArrayDataHolder m_cKey;
			Action::ArrayDataHolder m_cValue;

			// constructor
			Property() : m_cKey(), m_cValue() {}
			Property(int iKeyID_) : m_cKey(iKeyID_), m_cValue() {}

			void initialize(Interface::IProgram& cProgram_)
			{
				m_cKey.initialize(cProgram_);
				m_cValue.initialize(cProgram_);
			}
			void terminate(Interface::IProgram& cProgram_)
			{
				m_cKey.terminate(cProgram_);
				m_cValue.terminate(cProgram_);
			}
			void serialize(ModArchive& archiver_)
			{
				m_cKey.serialize(archiver_);
				m_cValue.serialize(archiver_);
			}
		};

		KwicImpl()
			: Super(),
			  m_cSourceData(),
			  m_cPosition(),
			  m_cData(),
			  m_iFieldID(-1),
			  m_cSize(),
			  m_cStartTag(),
			  m_cEndTag(),
			  m_cEscape(),
			  m_cEllipsis(),
			  m_cLanguage(),
			  m_vecProperty(),
			  m_cKwic(),
			  m_bFirstCall(true)
		{}
		KwicImpl(int iSourceID_,
				 int iPositionID_,
				 int iDataID_)
			: Super(),
			  m_cSourceData(iSourceID_),
			  m_cPosition(iPositionID_),
			  m_cData(iDataID_),
			  m_iFieldID(-1),
			  m_cSize(),
			  m_cStartTag(),
			  m_cEndTag(),
			  m_cEscape(),
			  m_cEllipsis(),
			  m_cLanguage(),
			  m_vecProperty(),
			  m_cKwic(),
			  m_bFirstCall(true)
		{}

		~KwicImpl() {}

	///////////////////////////
	// Function::Kwic::
		virtual void setFieldID(int iID_) {m_iFieldID = iID_;}
		virtual void setSize(int iID_) {m_cSize.setDataID(iID_);}
		virtual void setStartTag(int iID_) {m_cStartTag.setDataID(iID_);}
		virtual void setEndTag(int iID_) {m_cEndTag.setDataID(iID_);}
		virtual void setEscape(int iID_) {m_cEscape.setDataID(iID_);}
		virtual void setEllipsis(int iID_) {m_cEllipsis.setDataID(iID_);}

		virtual void setLanguage(int iID_) {m_cLanguage.setDataID(iID_);}
		virtual void setPropertyKey(int iID_);
		virtual void setPropertyValue(int iID_);

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
	//	virtual void accumulate(Interface::IProgram& cProgram_,
	//							Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
		// accessor
		const Common::Data* getSourceData() {return m_cSourceData.getData();}
		const Common::IntegerData* getPosition() {return m_cPosition.isValid() ? m_cPosition.getData() : 0;}
		const Common::IntegerData* getSize() {return m_cSize.isValid() ? m_cSize.getData() : 0;}
		const Common::StringData* getStartTag() {return m_cStartTag.isValid() ? m_cStartTag.getData() : 0;}
		const Common::StringData* getEndTag() {return m_cEndTag.isValid() ? m_cEndTag.getData() : 0;}
		const Common::StringData* getEscape() {return m_cEscape.isValid() ? m_cEscape.getData() : 0;}
		const Common::StringData* getEllipsis() {return m_cEllipsis.isValid() ? m_cEllipsis.getData() : 0;}
		const Common::LanguageData* getLanguage() {return m_cLanguage.isValid() ? m_cLanguage.getData() : 0;}

		Common::StringData* getData() {return m_cData.get();}

	private:
		void cutData(const Common::Data* pData_,
					 int iStart_,
					 int iSize_,
					 ModUnicodeString& cstrResult_);
		bool cutString(const Common::Data* pData_,
					   int& iStart_,
					   int& iSize_,
					   ModUnicodeString& cstrResult_);
		int getLength(const Common::Data* pData_);

		// data type of source data is guaranteed by createDataType in plan
		Action::DataHolder m_cSourceData;
		// data type of position is specified by kwic design
		Action::IntegerDataHolder m_cPosition;

		// data type of result is guaranteed by createDataType in plan
		Action::StringDataHolder m_cData;

		// field id to get kwic
		int m_iFieldID;

		// data type of options are guaranteed by setExpectedType in analyzer
		Action::IntegerDataHolder m_cSize;
		Action::StringDataHolder m_cStartTag;
		Action::StringDataHolder m_cEndTag;
		Action::StringDataHolder m_cEscape;
		Action::StringDataHolder m_cEllipsis;

		// language data type is guaranteed because it is language column
		Action::LanguageDataHolder m_cLanguage;
		VECTOR<Property> m_vecProperty;

		_SYDNEY::Utility::Kwic m_cKwic;
		bool m_bFirstCall;
	};
}

///////////////////////////////////////////////
// Execution::Function::Impl::KwicImpl

// FUNCTION public
//	Function::Impl::KwicImpl::setPropertyKey -- 
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
Impl::KwicImpl::
setPropertyKey(int iID_)
{
	// new property holder added
	m_vecProperty.PUSHBACK(Property(iID_));
}

// FUNCTION public
//	Function::Impl::KwicImpl::setPropertyValue -- 
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
Impl::KwicImpl::
setPropertyValue(int iID_)
{
	if (m_vecProperty.ISEMPTY()
		|| m_vecProperty.GETBACK().m_cValue.isValid()) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	m_vecProperty.GETBACK().m_cValue.setDataID(iID_);
}

// FUNCTION public
//	Function::Impl::KwicImpl::explain -- 
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
Impl::KwicImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put("kwic");
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.pushNoNewLine();
		cExplain_.put("(");
		m_cSourceData.explain(cProgram_, cExplain_);
		cExplain_.put(" for ");
		m_cSize.explain(cProgram_, cExplain_);
		if (m_cStartTag.isValid()) {
			cExplain_.put(" enclose with ");
			m_cStartTag.explain(cProgram_, cExplain_);
			if (m_cEndTag.isValid()) {
				cExplain_.put(" and ");
				m_cEndTag.explain(cProgram_, cExplain_);
			}
		}
		if (m_cEscape.isValid()) {
			cExplain_.put(" escape ");
			m_cEscape.explain(cProgram_, cExplain_);
		}
		if (m_cEllipsis.isValid()) {
			cExplain_.put(" ellipsis ");
			m_cEllipsis.explain(cProgram_, cExplain_);
		}
		cExplain_.put(")");
		cExplain_.put(" to ");
		m_cData.explain(cProgram_, cExplain_);
		cExplain_.popNoNewLine();
	}
}

// FUNCTION public
//	Function::Impl::KwicImpl::initialize -- 
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
Impl::KwicImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized() == false) {
		if (m_cData.isValid() == false
			|| m_cSourceData.isValid() == false
			|| m_cSize.isValid() == false
			|| m_vecProperty.ISEMPTY()) {
			_SYDNEY_THROW0(Exception::Unexpected);
		}
		m_cData.initialize(cProgram_);
		m_cSourceData.initialize(cProgram_);
		m_cPosition.initialize(cProgram_);
		m_cSize.initialize(cProgram_);
		m_cStartTag.initialize(cProgram_);
		m_cEndTag.initialize(cProgram_);
		m_cEscape.initialize(cProgram_);
		m_cEllipsis.initialize(cProgram_);
		m_cLanguage.initialize(cProgram_);
		Opt::ForEach(m_vecProperty,
					 boost::bind(&Property::initialize,
								 _1,
								 boost::ref(cProgram_)));
		m_bFirstCall = true;
	}
}

// FUNCTION public
//	Function::Impl::KwicImpl::terminate -- 
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
Impl::KwicImpl::
terminate(Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized()) {
		m_cData.terminate(cProgram_);
		m_cSourceData.terminate(cProgram_);
		m_cPosition.terminate(cProgram_);
		m_cSize.terminate(cProgram_);
		m_cStartTag.terminate(cProgram_);
		m_cEndTag.terminate(cProgram_);
		m_cEscape.terminate(cProgram_);
		m_cEllipsis.terminate(cProgram_);
		m_cLanguage.terminate(cProgram_);
		Opt::ForEach(m_vecProperty,
					 boost::bind(&Property::terminate,
								 _1,
								 boost::ref(cProgram_)));
	}
}

// FUNCTION public
//	Function::Impl::KwicImpl::execute -- 
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
Impl::KwicImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		if (m_bFirstCall) {
			m_bFirstCall = false;

			VECTOR<Property>::ITERATOR iterator = m_vecProperty.begin();
			const VECTOR<Property>::ITERATOR last = m_vecProperty.end();
			; _SYDNEY_ASSERT(iterator != last);

			m_cKwic.set((*iterator).m_cKey.getData(),
						(*iterator).m_cValue.getData(),
						(getStartTag() ? getStartTag()->getValue() : ModUnicodeString()),
						(getEndTag() ? getEndTag()->getValue() : ModUnicodeString()),
						(getEllipsis() ? getEllipsis()->getValue() : ModUnicodeString()),
						(getEscape() ? getEscape()->getValue() : ModUnicodeString()),
						(getSize() ? getSize()->getValue() : 0));
			
			for (++iterator; iterator != last; ++iterator) {
				m_cKwic.set((*iterator).m_cKey.getData(),
							(*iterator).m_cValue.getData(),
							ModUnicodeString(),
							ModUnicodeString(),
							ModUnicodeString(),
							ModUnicodeString(),
							0);
			}
		}
		if (getSourceData()->isNull()) {
			m_cData->setNull();
		} else {
			ModUnicodeString cstrSource;
			ModUnicodeString cstrResult;

			int iPosition =
				getPosition() ? (getPosition()->isNull() ? -1 : getPosition()->getValue()) : 0;

			if (iPosition < 0) {
				// position is null -> use first <size> characters
				cutData(getSourceData(),
						0,
						getSize()->getValue(),
						cstrResult);
			} else {
				ModSize iRoughSize = m_cKwic.getRoughSize(m_iFieldID);
				cutData(getSourceData(),
						iPosition,
						iRoughSize,
						cstrSource);
				if (cstrSource.getLength() > 0) {
					if ((m_cKwic.generate(cstrSource,
										  iRoughSize,
										  (m_cLanguage.isValid()
										   ? m_cLanguage->getValue()
										   : ModLanguageSet()),
										  cstrResult)) == false) {
						// Retry. Find in wider range.
						cutData(getSourceData(),
								0,
								getLength(getSourceData()),
								cstrSource);
						if (cstrSource.getLength() > 0) {
							m_cKwic.generate(cstrSource,
											 getSize()->getValue(),
											 (m_cLanguage.isValid()
											  ? m_cLanguage->getValue()
											  : ModLanguageSet()),
											 cstrResult, true);
						} else {
							cutData(getSourceData(),
									0,
									getSize()->getValue(),
									cstrResult);
						}
					}
				} else {
					// position is outside of string -> use first <size> characters
					cutData(getSourceData(),
							0,
							getSize()->getValue(),
							cstrResult);
				}
			}
			getData()->setValue(cstrResult);
		}
		done();
	}
	return Action::Status::Success;
#undef EMPTY_IF_NULL
#undef ZERO_IF_NULL
}

// FUNCTION public
//	Function::Impl::KwicImpl::finish -- 
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
Impl::KwicImpl::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Function::Impl::KwicImpl::reset -- 
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
Impl::KwicImpl::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Function::Impl::KwicImpl::getClassID -- 
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
Impl::KwicImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Kwic);
}

// FUNCTION public
//	Function::Impl::KwicImpl::serialize -- 
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
Impl::KwicImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cSourceData.serialize(archiver_);
	m_cPosition.serialize(archiver_);
	m_cData.serialize(archiver_);
	m_cSize.serialize(archiver_);
	m_cStartTag.serialize(archiver_);
	m_cEndTag.serialize(archiver_);
	m_cEscape.serialize(archiver_);
	m_cEllipsis.serialize(archiver_);
	m_cLanguage.serialize(archiver_);
	archiver_(m_iFieldID);
	Utility::SerializeObject(archiver_,
							 m_vecProperty);
}

// FUNCTION private
//	Function::Impl::KwicImpl::cutData -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data* pData_
//	int iStart_
//	int iSize_
//	ModUnicodeString& cstrResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::KwicImpl::
cutData(const Common::Data* pData_,
		int iStart_,
		int iSize_,
		ModUnicodeString& cstrResult_)
{
	switch (pData_->getType()) {
	case Common::DataType::String:
		{
			cutString(pData_,
					  iStart_,
					  iSize_,
					  cstrResult_);
			break;
		}
	case Common::DataType::Array:
		{
			if (pData_->getElementType() == Common::DataType::Data) {
				const Common::DataArrayData* pArray =
					_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, pData_);
				int n = pArray->getCount();
				ModUnicodeString cstrTmp;
				for (int i = 0; i < n; ++i) {
					if (cutString(pArray->getElement(i).get(),
								  iStart_,
								  iSize_,
								  cstrTmp)) {
						cstrResult_.append(cstrTmp);
						if (iSize_ <= 0) break;
					}
				}
				break;		
			}
			; // thru.
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

// FUNCTION private
//	Function::Impl::KwicImpl::cutString -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data* pData_
//	int& iStart_
//	int& iSize_
//	ModUnicodeString& cstrResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Impl::KwicImpl::
cutString(const Common::Data* pData_,
		  int& iStart_,
		  int& iSize_,
		  ModUnicodeString& cstrResult_)
{
	if (pData_->getType() != Common::DataType::String) {
		// illegal type
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	const Common::StringData* pString =
		_SYDNEY_DYNAMIC_CAST(const Common::StringData*, pData_);
	int iLength = pString->getLength();

	if (iStart_ < iLength) {
		pString->getValue().copy(cstrResult_,
								 iStart_,
								 iSize_);
		iStart_ = 0;
		iSize_ -= iLength;
		return true;
	}
	iStart_ -= iLength;
	return false;
}

int
Impl::KwicImpl::
getLength(const Common::Data* pData_)
{
	switch (pData_->getType()) {
	case Common::DataType::String:
		{
			const Common::StringData* pString =
				_SYDNEY_DYNAMIC_CAST(const Common::StringData*, pData_);
			return pString->getLength();
		}
	case Common::DataType::Array:
		{
			if (pData_->getElementType() == Common::DataType::Data) {
				const Common::DataArrayData* pArray =
					_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, pData_);
				int n = pArray->getCount();
				int len = 0;
				for (int i = 0; i < n; ++i) {
					const Common::StringData* tmpString =
						_SYDNEY_DYNAMIC_CAST(const Common::StringData*, pArray->getElement(i).get());
					len += tmpString->getLength();
				}
				return len;
			}
			; // thru.
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}


/////////////////////////////////
// Function::Kwic::

// FUNCTION public
//	Function::Kwic::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iSourceID_
//	int iPositionID_
//	int iDataID_
//	
// RETURN
//	Kwic*
//
// EXCEPTIONS

//static
Kwic*
Kwic::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iSourceID_,
	   int iPositionID_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult = new Impl::KwicImpl(iSourceID_,
												   iPositionID_,
												   iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Function::Kwic::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Kwic*
//
// EXCEPTIONS

//static
Kwic*
Kwic::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::Kwic);
	return new Impl::KwicImpl;
}

_SYDNEY_EXECUTION_FUNCTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
