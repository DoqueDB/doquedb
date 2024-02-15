// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Iterator/OperandElement.h --
// 
// Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_ITERATOR_OPERANDELEMENT_H
#define __SYDNEY_EXECUTION_ITERATOR_OPERANDELEMENT_H

#include "Execution/Iterator/Module.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Action/IteratorHolder.h"
#include "Execution/Action/Status.h"
#include "Execution/Declaration.h"

#include "Common/Object.h"

#include "Opt/Declaration.h"

_SYDNEY_BEGIN

namespace Common
{
	class DataArrayData;
}

_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ITERATOR_BEGIN

/////////////////////////////////////////////
// CLASS
//	Execution::Iterator::OperandElement -- 
//
// NOTES
class OperandElement
	: public Common::Object
{
public:
	OperandElement()
		: m_cIterator(),
		  m_cData(),
		  m_bHasData(false)
	{}
	OperandElement(int iIteratorID_,
				   int iDataID_)
		: m_cIterator(iIteratorID_),
		  m_cData(iDataID_),
		  m_bHasData(true)
	{}
	~OperandElement() {}

	void setIteratorID(int iID_) {m_cIterator.setID(iID_);}
	void setDataID(int iID_) {m_cData.setDataID(iID_);}

	bool isValid() {return m_cIterator.isValid();}

	void explain(Opt::Environment* pEnvironment_,
				 Interface::IProgram& cProgram_,
				 Opt::Explain& cExplain_);
	void initialize(Interface::IProgram& cProgram_);
	void terminate(Interface::IProgram& cProgram_);
	Action::Status::Value startUp(Interface::IProgram& cProgram_);
	void finish(Interface::IProgram& cProgram_);

	bool next(Interface::IProgram& cProgram_);
	void reset(Interface::IProgram& cProgram_);

	bool isHasData() const {return m_bHasData;}
	const Common::DataArrayData* getData() const;

	void serialize(ModArchive& archiver_)
	{
		m_cIterator.serialize(archiver_);
		m_cData.serialize(archiver_);
	}
protected:
private:
	Action::IteratorHolder m_cIterator;
	Action::ArrayDataHolder m_cData;
	bool m_bHasData;
};

_SYDNEY_EXECUTION_ITERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ITERATOR_OPERANDELEMENT_H

//
//	Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
