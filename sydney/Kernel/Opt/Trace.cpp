// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Opt/Trace.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Opt";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Opt/Trace.h"
#include "Opt/Configuration.h"

#include "Common/Assert.h"
#include "Common/BitSet.h"
#include "Common/DataArrayData.h"
#include "Common/DataInstance.h"
#include "Common/DecimalData.h"
#include "Common/ObjectIDData.h"
#include "Common/StringData.h"

#include "Exception/Unexpected.h"

#include "ModUnicodeOstrStream.h"

_SYDNEY_BEGIN
_SYDNEY_OPT_BEGIN

namespace
{
	// TEMPLATE CLASS local
	//	_DataFormatter --
	//
	// TEMPLATE ARGUMENTS
	//	class Stream_
	//
	// NOTES
	template <class Stream_>
	class _DataFormatter
	{
	public:
		_DataFormatter(Stream_& cStream_)
			: m_cStream(cStream_)
		{}
		void formatData(const Common::Data& cData_)
		{
			if (cData_.isNull()) {
				m_cStream << cData_.toString();
			} else {
				switch (cData_.getType()) {
				case Common::DataType::String:
					{
						formatString(cData_);
						break;
					}
				case Common::DataType::ObjectID:
					{
						formatObjectID(cData_);
						break;
					}
				case Common::DataType::Array:
					{
						if (cData_.getElementType() == Common::DataType::Data) {
							formatDataArray(_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&, cData_));
						} else {
							m_cStream << cData_.toString();
						}
						break;
					}
				case Common::DataType::BitSet:
					{
						formatBitSet(cData_);
						break;
					}
				default:
					{
						m_cStream << cData_.toString();
						break;
					}
				}
			}
		}
	protected:
	private:
		void formatString(const Common::Data& cData_)
		{
			; _SYDNEY_ASSERT(&cData_);
			const ModUnicodeString& cstrValue = cData_.toString();
			int iMaxString = Opt::Configuration::getTraceMaxStringSize();
			if (cstrValue.getLength() > static_cast<ModSize>(iMaxString)) {
				ModUnicodeString cstrTmp(cstrValue, iMaxString - 3);
				m_cStream << "'" << cstrTmp << "...'";
			} else {
				m_cStream << "'" << cstrValue << "'";
			}
		}
		void formatObjectID(const Common::Data& cData_)
		{
			; _SYDNEY_ASSERT(&cData_);
			m_cStream << "OID:" << cData_.toString();
		}
		void formatDataArray(const Common::DataArrayData& cData_)
		{
			; _SYDNEY_ASSERT(&cData_);
			m_cStream << '{';
			int n = cData_.getCount();
			for (int i = 0; i < n; ++i) {
				if (i > 0) m_cStream << ',';
				formatData(*(cData_.getElement(i)));
			}
			m_cStream << '}';
		}
		void formatBitSet(const Common::Data& cData_)
		{
			; _SYDNEY_ASSERT(&cData_);
			const Common::BitSet& cBitSet =
				_SYDNEY_DYNAMIC_CAST(const Common::BitSet&, cData_);
			ModUnicodeOstrStream stream;
			Common::BitSet::ConstIterator iterator = cBitSet.begin();
			const Common::BitSet::ConstIterator last = cBitSet.end();
			if (iterator != last) {
				stream << *iterator;
				++iterator;
			}
			for (; iterator != last; ++iterator) {
				stream << ",";
				stream << *iterator;
			}
			ModUnicodeString cstrValue = stream.getString();
			int iMaxString = Opt::Configuration::getTraceMaxStringSize();
			if (cstrValue.getLength() > static_cast<ModSize>(iMaxString)) {
				ModUnicodeString cstrTmp(cstrValue, iMaxString - 3);
				m_cStream << "{" << cstrTmp << "...}";
			} else {
				m_cStream << "{" << cstrValue << "}";
			}
		}

		Stream_& m_cStream;
	}; // class _DataFormatter
}

//////////////////////////////////////////
// Opt::Trace::Handler::
//////////////////////////////////////////

// FUNCTION public
//	Opt::Trace::Handler::Handler -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data& cData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Trace::Handler::
Handler(const Common::Data& cData_)
	: m_cData(cData_)
{}

// FUNCTION public
//	Opt::Trace::Handler::writeData -- 
//
// NOTES
//
// ARGUMENTS
//	ModMessageStream& cStream_
//	
// RETURN
//	ModMessageStream&
//
// EXCEPTIONS

ModMessageStream&
Trace::Handler::
writeData(ModMessageStream& cStream_) const
{
	_DataFormatter<ModMessageStream>(cStream_).formatData(m_cData);
	return cStream_;
}

// FUNCTION public
//	Opt::Trace::Handler::writeData -- 
//
// NOTES
//
// ARGUMENTS
//	ModOstream& cStream_
//	
// RETURN
//	ModOstream&
//
// EXCEPTIONS

ModOstream&
Trace::Handler::
writeData(ModOstream& cStream_) const
{
	_DataFormatter<ModOstream>(cStream_).formatData(m_cData);
	return cStream_;
}

//////////////////////////////////////////
// Opt::Trace::
//////////////////////////////////////////

// FUNCTION public
//	Opt::Trace::toString -- get string representation of data for tracing
//
// NOTES
//
// ARGUMENTS
//	const Common::Data& cData_
//	
// RETURN
//	Opt::Trace::Handler
//
// EXCEPTIONS

//static
Trace::Handler
Trace::
toString(const Common::Data& cData_)
{
	return Handler(cData_);
}

_SYDNEY_OPT_END
_SYDNEY_END

// FUNCTION public
//	ModMessageStream& operator<< -- for message stream
//
// NOTES
//
// ARGUMENTS
//	ModMessageStream& cStream_
//	const Opt::Trace::Handler& cHandler_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

ModMessageStream& operator<<(ModMessageStream& cStream_,
							 const _SYDNEY::Opt::Trace::Handler& cHandler_)
{
	return cHandler_.writeData(cStream_);
}

// FUNCTION public
//	ModOstream& operator<< -- 
//
// NOTES
//
// ARGUMENTS
//	ModOstream& cStream_
//	const Opt::Trace::Handler& cHandler_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

ModOstream& operator<<(ModOstream& cStream_,
					   const _SYDNEY::Opt::Trace::Handler& cHandler_)
{
	return cHandler_.writeData(cStream_);
}

//
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
