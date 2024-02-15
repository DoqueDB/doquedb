// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CClientImpl.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Client2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Client2/CClient.h"
#include "Client2/CClientImpl.h"
#include "Client2/DataSource.h"
#include "Client2/PrepareStatement.h"
#include "Client2/ResultSet.h"
#include "Client2/Singleton.h"
#include "Client2/Session.h"

#include "Common/BinaryData.h"
#include "Common/DataArrayData.h"
#include "Common/DateTimeData.h"
#include "Common/DecimalData.h"
#include "Common/DoubleData.h"
#include "Common/IntegerData.h"
#include "Common/Integer64Data.h"
#include "Common/LanguageData.h"
#include "Common/NullData.h"
#include "Common/StringData.h"
#include "Common/UnsignedIntegerData.h"

#include "Exception/Message.h"
#include "Exception/ModLibraryError.h"
#include "Exception/Unexpected.h"

#include "Os/Memory.h"

#include "ModException.h"
//#include "ModKanjiCode.h"
#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"

_TRMEISTER_USING
_TRMEISTER_CLIENT2_USING

//	FUNCTION public
//	tr_errno --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" int
tr_errno(const tr_ds_t* ds)
{
	int result = -1;
	
	if (ds != 0)
	{
		const tr_ds_t_impl* dsi = syd_reinterpret_cast<const tr_ds_t_impl*>(ds);
		result = dsi->m_cException.getErrorNumber();
	}

	return result;
}

//	FUNCTION public
//	tr_sqlstate --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" const char*
tr_sqlstate(const tr_ds_t* ds)
{
	const char* result = 0;
	
	if (ds != 0)
	{
		const tr_ds_t_impl* dsi = syd_reinterpret_cast<const tr_ds_t_impl*>(ds);
		result = dsi->m_cException.getStateCode();
	}

	return result;
}

//	FUNCTION public
//	tr_error_message_size --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" unsigned int
tr_error_message_size(const tr_ds_t* ds)
{
	unsigned int result = 0;
	
	if (ds != 0)
	{
		const tr_ds_t_impl* dsi = syd_reinterpret_cast<const tr_ds_t_impl*>(ds);
		ModUnicodeOstrStream stream;
		stream << dsi->m_cException;
		ModUnicodeString s = stream.getString();
		const char* p = s.getString();
		result = s.getStringBufferSize();
	}

	return result;
}

//	FUNCTION public
//	tr_error_message --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" int
tr_error_message(tr_ds_t* ds,
				 char* buf,
				 unsigned int bufsize)
{
	// bad argument
	int result = -1;

	if (ds != 0 && buf != 0)
	{
		if (bufsize == 0)
		{
			// string size
			result = 0;
		}
		else
		{
			tr_ds_t_impl* dsi = syd_reinterpret_cast<tr_ds_t_impl*>(ds);
			
			ModUnicodeOstrStream stream;
			stream << dsi->m_cException;
			ModUnicodeString s = stream.getString();
			
			const char* p = s.getString();
			unsigned int size = s.getStringBufferSize();
			
			unsigned int copysize = size;
			if (bufsize < size)
			{
				copysize = bufsize - 1;
			}
			try
			{
				// something error occurred
				result = -3;
				Os::Memory::copy(buf, p, copysize);
				if (bufsize < size)
				{
					buf[bufsize - 1] = '\0';
					// short of bufsize
					result = -2;
				}
				else
				{
					// string size
					result = copysize;
				}
			}
			catch (Exception::Object& e)
			{
				dsi->m_cException = e;
			}
			catch (ModException& e)
			{
				dsi->m_cException =	Exception::ModLibraryError(
					moduleName, srcFile, __LINE__, e);
			}
			catch (...)
			{
				dsi->m_cException =
					Exception::Unexpected(moduleName, srcFile, __LINE__);
			}
		}
	}
	
	return result;
}

//	FUNCTION public
//	tr_reset_error --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_ok_t
tr_reset_error(tr_ds_t* ds)
{
	tr_ok_t result = tr_ng;
	
	if (ds != 0)
	{
		tr_ds_t_impl* dsi = syd_reinterpret_cast<tr_ds_t_impl*>(ds);
		dsi->m_cException = Exception::Object();
		result = tr_ok;
	}

	return result;
}

//	FUNCTION public
//	tr_initialize --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_ds_t*
tr_initialize()
{
	tr_ds_t_impl* dsi = 0;
	try
	{
		Singleton::RemoteServer::initialize();
		dsi = new tr_ds_t_impl();
	}
	catch (...)
	{
	}
	return syd_reinterpret_cast<tr_ds_t*>(dsi);
}

//	FUNCTION public
//	tr_terminate --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_ok_t
tr_terminate(tr_ds_t* ds)
{
	tr_ok_t result = tr_ng;
	try
	{
		Singleton::RemoteServer::terminate();
		if (ds != 0)
		{
			tr_ds_t_impl* dsi = syd_reinterpret_cast<tr_ds_t_impl*>(ds);
			delete dsi;
			ds = 0;
		}
		result = tr_ok;
	}
	catch (...)
	{
	}
	return result;
}

//	FUNCTION public
//	tr_open --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_ok_t
tr_open(tr_ds_t* ds,
		const char* host_name,
		int port_number)
{
	ModUnicodeString cstrHostName(host_name);

	tr_ok_t result = tr_ng;
	
	if (ds != 0)
	{
		tr_ds_t_impl* dsi = syd_reinterpret_cast<tr_ds_t_impl*>(ds);
		try
		{
			dsi->m_pDataSource =
				DataSource::createDataSource(cstrHostName, port_number);
			dsi->m_pDataSource->open();
			dsi->m_cException = Exception::Object();
			result = tr_ok;
		}
		catch (Exception::Object& e)
		{
			dsi->m_cException = e;
		}
		catch (ModException& e)
		{
			dsi->m_cException =
				Exception::ModLibraryError(moduleName, srcFile, __LINE__, e);
		}
		catch (...)
		{
			dsi->m_cException =
				Exception::Unexpected(moduleName, srcFile, __LINE__);
		}
	}
	
	return result;
}

//	FUNCTION public
//	tr_close --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_ok_t
tr_close(tr_ds_t* ds)
{
	tr_ok_t result = tr_ng;

	if (ds != 0)
	{
		tr_ds_t_impl* dsi = syd_reinterpret_cast<tr_ds_t_impl*>(ds);
		if (dsi->m_pDataSource != 0)
		{
			try
			{
				dsi->m_pDataSource->close();
				dsi->m_pDataSource->release();
				result = tr_ok;
			}
			catch (Exception::Object& e)
			{
				dsi->m_cException = e;
			}
			catch (ModException& e)
			{
				dsi->m_cException = Exception::ModLibraryError(
					moduleName, srcFile, __LINE__, e);
			}
			catch (...)
			{
				dsi->m_cException =
					Exception::Unexpected(moduleName, srcFile, __LINE__);
			}
			dsi->m_pDataSource = 0;
		}
	}
	
	return result;
}

//	FUNCTION public
//	tr_create_session --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_ss_t*
tr_create_session(tr_ds_t* ds,
				  const char* database_name,
				  const char* user_name,
				  const char* password)
{
	// check argument
	if (ds == 0)
	{
		return 0;
	}
	tr_ds_t_impl* dsi = syd_reinterpret_cast<tr_ds_t_impl*>(ds);
	if (dsi->m_pDataSource == 0)
	{
		return 0;
	}
	ModUnicodeString cstrDatabaseName(database_name);
	ModUnicodeString cstrUserName(user_name);
	ModUnicodeString cstrPassword(password);

	tr_ss_t_impl* ssi = 0;
	try
	{
		ssi = new tr_ss_t_impl();
		ssi->m_pTrDsImpl = dsi;
		
		ssi->m_pSession = dsi->m_pDataSource->createSession(
			cstrDatabaseName, cstrUserName, cstrPassword);
	}
	catch (Exception::Object& e)
	{
		dsi->m_cException = e;
		delete ssi, ssi = 0;
	}
	catch (ModException& e)
	{
		dsi->m_cException =
			Exception::ModLibraryError(moduleName, srcFile, __LINE__, e);
		delete ssi, ssi = 0;
	}
	catch (...)
	{
		dsi->m_cException =
			Exception::Unexpected(moduleName, srcFile, __LINE__);
		delete ssi, ssi = 0;
	}
	
	return syd_reinterpret_cast<tr_ss_t*>(ssi);
}

//	FUNCTION public
//	tr_execute_statement --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_rs_t*
tr_execute_statement(tr_ss_t* ss,
					 const char* statement)
//					 tr_kc_t kanji)
{
	// check argument
	if (ss == 0)
	{
		return 0;
	}
	tr_ss_t_impl* ssi = syd_reinterpret_cast<tr_ss_t_impl*>(ss);
	if (ssi->m_pSession == 0 || ssi->m_pTrDsImpl == 0)
	{
		return 0;
	}

	tr_rs_t_impl* rsi = 0;
	tr_ds_t_impl* dsi = ssi->m_pTrDsImpl;
	try
	{
		rsi = new tr_rs_t_impl();
		rsi->m_pTrDsImpl = dsi;
		
//		ModUnicodeString cstrStatement(
//			statement, static_cast<ModKanjiCode::KanjiCodeType>(kanji));
		ModUnicodeString cstrStatement(statement);
		rsi->m_pResultSet = ssi->m_pSession->executeStatement(cstrStatement);
	}
	catch (Exception::Object& e)
	{
		dsi->m_cException = e;
		delete rsi, rsi = 0;
	}
	catch (ModException& e)
	{
		dsi->m_cException =
			Exception::ModLibraryError(moduleName, srcFile, __LINE__, e);
		delete rsi, rsi = 0;
	}
	catch (...)
	{
		dsi->m_cException =
			Exception::Unexpected(moduleName, srcFile, __LINE__);
		delete rsi, rsi = 0;
	}
	
	return syd_reinterpret_cast<tr_rs_t*>(rsi);
}

//	FUNCTION public
//	tr_create_preparestatement --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_ps_t*
tr_create_preparestatement(tr_ss_t* ss,
						   const char* statement)
{
	// check argument
	if (ss == 0 || statement == 0)
	{
		return 0;
	}
	tr_ss_t_impl* ssi = syd_reinterpret_cast<tr_ss_t_impl*>(ss);
	if (ssi->m_pSession == 0)
	{
		return 0;
	}
	
	// new prepare statement
	tr_ps_t_impl* psi = 0;
	try
	{
		psi = new tr_ps_t_impl();
	}
	catch (...)
	{
		ssi->m_pTrDsImpl->m_cException =
			Exception::Unexpected(moduleName, srcFile, __LINE__);
		return 0;
	}

	// create prepare statement
	ModUnicodeString cstrStatement(statement);
	try
	{
		psi->m_pPrepareStatement =
			ssi->m_pSession->createPrepareStatement(cstrStatement);
	}
	catch (Exception::Object& e)
	{
		ssi->m_pTrDsImpl->m_cException = e;
	}
	catch (ModException& e)
	{
		ssi->m_pTrDsImpl->m_cException =
			Exception::ModLibraryError(moduleName, srcFile, __LINE__, e);
	}
	catch (...)
	{
		ssi->m_pTrDsImpl->m_cException =
			Exception::Unexpected(moduleName, srcFile, __LINE__);
	}
	if (psi->m_pPrepareStatement == 0)
	{
		delete psi;
		return 0;
	}

	return syd_reinterpret_cast<tr_ps_t*>(psi);
}

//	FUNCTION public
//	tr_execute_preparestatement --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_rs_t*
tr_execute_preparestatement(tr_ss_t* ss,
							tr_ps_t* ps,
							tr_tp_t* array)
{
	// check argument
	if (ss == 0 || ps == 0)
	{
		return 0;
	}
	tr_ss_t_impl* ssi = syd_reinterpret_cast<tr_ss_t_impl*>(ss);
	if (ssi->m_pSession == 0)
	{
		return 0;
	}
	tr_ps_t_impl* psi = syd_reinterpret_cast<tr_ps_t_impl*>(ps);
	if (psi->m_pPrepareStatement == 0)
	{
		return 0;
	}
	tr_tp_t_impl* tpi = syd_reinterpret_cast<tr_tp_t_impl*>(array);
	if (tpi && tpi->m_pArrayData == 0)
	{
		return 0;
	}

	tr_rs_t_impl* rsi = 0;
	tr_ds_t_impl* dsi = ssi->m_pTrDsImpl;
	try
	{
		rsi = new tr_rs_t_impl();
		rsi->m_pTrDsImpl = dsi;
		if (tpi)
		{
			rsi->m_pResultSet = ssi->m_pSession->executePrepareStatement(
				*(psi->m_pPrepareStatement),
				*(tpi->m_pArrayData));
		}
		else
		{
			rsi->m_pResultSet = ssi->m_pSession->executePrepareStatement(
				*(psi->m_pPrepareStatement),
				Common::DataArrayData());
		}
	}
	catch (Exception::Object& e)
	{
		dsi->m_cException = e;
		delete rsi, rsi = 0;
	}
	catch (ModException& e)
	{
		dsi->m_cException =
			Exception::ModLibraryError(moduleName, srcFile, __LINE__, e);
		delete rsi, rsi = 0;
	}
	catch (...)
	{
		dsi->m_cException =
			Exception::Unexpected(moduleName, srcFile, __LINE__);
		delete rsi, rsi = 0;
	}
	
	return syd_reinterpret_cast<tr_rs_t*>(rsi);
}

//	FUNCTION public
//	tr_erase_preparestatement --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_ok_t
tr_erase_preparestatement(tr_ps_t* ps)
{
	tr_ok_t result = tr_ng;

	// check argument
	if (ps != 0)
	{
		tr_ps_t_impl* psi = syd_reinterpret_cast<tr_ps_t_impl*>(ps);
		if (psi->m_pPrepareStatement != 0 && psi->m_pTrDsImpl != 0)
		{
			tr_ds_t_impl* dsi = psi->m_pTrDsImpl;
			try
			{
				psi->m_pPrepareStatement->close();
				psi->m_pPrepareStatement->release();
				result = tr_ok;
			}
			catch (Exception::Object& e)
			{
				dsi->m_cException = e;
			}
			catch (ModException& e)
			{
				dsi->m_cException = Exception::ModLibraryError(
					moduleName, srcFile, __LINE__, e);
			}
			catch (...)
			{
				dsi->m_cException =
					Exception::Unexpected(moduleName, srcFile, __LINE__);
			}
	
			try
			{
				delete psi;
			}
			catch (...) {}
		}
	}

	return result;
}
	
//	FUNCTION public
//	tr_close_session --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_ok_t
tr_close_session(tr_ss_t* ss)
{
	tr_ok_t result = tr_ng;
	
	if (ss != 0)
	{
		tr_ss_t_impl* ssi = syd_reinterpret_cast<tr_ss_t_impl*>(ss);
		if (ssi->m_pSession != 0 && ssi->m_pTrDsImpl != 0)
		{
			tr_ds_t_impl* dsi = ssi->m_pTrDsImpl;
			try
			{
				ssi->m_pSession->close();
				ssi->m_pSession->release();
				result = tr_ok;
			}
			catch (Exception::Object& e)
			{
				dsi->m_cException = e;
			}
			catch (ModException& e)
			{
				dsi->m_cException =	Exception::ModLibraryError(
					moduleName, srcFile, __LINE__, e);
			}
			catch (...)
			{
				dsi->m_cException =
					Exception::Unexpected(moduleName, srcFile, __LINE__);
			}

			try
			{
				delete ssi;
			}
			catch (...) {}
		}
	}

	return result;
}

//	FUNCTION public
//	tr_get_next_tuple --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_tp_t*
tr_get_next_tuple(tr_rs_t* rs,
				  tr_st_t* status)
{
	tr_tp_t_impl* tpi = 0;

	if (rs != 0)
	{
		tr_rs_t_impl* rsi = syd_reinterpret_cast<tr_rs_t_impl*>(rs);
		if (rsi->m_pTrDsImpl != 0)
		{
			tr_ds_t_impl* dsi = rsi->m_pTrDsImpl;

			try
			{
				tpi = new tr_tp_t_impl();
				tpi->m_pArrayData = new Common::DataArrayData();
				tpi->m_bOwner = true;
				tpi->m_pTrDsImpl = dsi;
				
				*status = static_cast<tr_st_t>(
					rsi->m_pResultSet->getNextTuple(tpi->m_pArrayData));
			}
			catch (Exception::Object& e)
			{
				dsi->m_cException = e;
				delete tpi, tpi = 0;
				*status = tr_st_error;
			}
			catch (ModException& e)
			{
				dsi->m_cException = Exception::ModLibraryError(
					moduleName, srcFile, __LINE__, e);
				delete tpi, tpi = 0;
				*status = tr_st_error;
			}
			catch (...)
			{
				dsi->m_cException =
					Exception::Unexpected(moduleName, srcFile, __LINE__);
				delete tpi, tpi = 0;
				*status = tr_st_error;
			}
		}
	}
	
	return syd_reinterpret_cast<tr_tp_t*>(tpi);
}
	
//	FUNCTION public
//	tr_get_metadata_size --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" unsigned int
tr_get_metadata_size(const tr_rs_t* rs)
{
	unsigned int result = 0;
	
	if (rs != 0)
	{
		const tr_rs_t_impl* rsi = syd_reinterpret_cast<const tr_rs_t_impl*>(rs);
		if (rsi->m_pResultSet != 0)
		{
			ModUnicodeString s = rsi->m_pResultSet->getMetaData()->getString();
			const char* p = s.getString();
			result = s.getStringBufferSize();
		}
	}

	return result;
}
	
//	FUNCTION public
//	tr_get_metadata --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" int
tr_get_metadata(tr_rs_t* rs,
				char* buf,
				unsigned int bufsize)
{
	// bad argument
	int result = -1;

	if (rs != 0 && buf != 0)
	{
		if (bufsize == 0)
		{
			// string size
			result = 0;
		}
		else
		{
			tr_rs_t_impl* rsi = syd_reinterpret_cast<tr_rs_t_impl*>(rs);
			if (rsi->m_pResultSet != 0 && rsi->m_pTrDsImpl != 0)
			{
				tr_ds_t_impl* dsi = rsi->m_pTrDsImpl;
				
				ModUnicodeString s =
					rsi->m_pResultSet->getMetaData()->getString();
			
				const char* p = s.getString();
				unsigned int size = s.getStringBufferSize();

				unsigned int copysize = size;
				if (bufsize < size)
				{
					copysize = bufsize - 1;
				}
				try
				{
					// something error occurred
					result = -3;
					Os::Memory::copy(buf, p, copysize);
					if (bufsize < size)
					{
						buf[bufsize - 1] = '\0';
						// short of bufsize
						result = -2;
					}
					else
					{
						// string size
						result = copysize;
					}
				}
				catch (Exception::Object& e)
				{
					dsi->m_cException = e;
				}
				catch (ModException& e)
				{
					dsi->m_cException =	Exception::ModLibraryError(
						moduleName, srcFile, __LINE__, e);
				}
				catch (...)
				{
					dsi->m_cException =
						Exception::Unexpected(moduleName, srcFile, __LINE__);
				}
			}
		}
	}	
	return result;
}

//	FUNCTION public
//	tr_close_resultset --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_ok_t
tr_close_resultset(tr_rs_t* rs)
{
	tr_ok_t result = tr_ng;

	if (rs != 0)
	{
		tr_rs_t_impl* rsi = syd_reinterpret_cast<tr_rs_t_impl*>(rs);
		if (rsi->m_pResultSet != 0 && rsi->m_pTrDsImpl != 0)
		{
			tr_ds_t_impl* dsi = rsi->m_pTrDsImpl;
			try
			{
				rsi->m_pResultSet->close();
				rsi->m_pResultSet->release();
				result = tr_ok;
			}
			catch (Exception::Object& e)
			{
				dsi->m_cException = e;
			}
			catch (ModException& e)
			{
				dsi->m_cException = Exception::ModLibraryError(
					moduleName, srcFile, __LINE__, e);
			}
			catch (...)
			{
				dsi->m_cException =
					Exception::Unexpected(moduleName, srcFile, __LINE__);
			}

			try
			{
				delete rsi;
			}
			catch (...) {}
		}
	}
	
	return result;
}
	
//	FUNCTION public
//	tr_get_tuple_size --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" int
tr_get_tuple_size(const tr_tp_t* tuple)
{
	int result = 0;
	if (tuple != 0)
	{
		const tr_tp_t_impl* tpi =
			syd_reinterpret_cast<const tr_tp_t_impl*>(tuple);
		if (tpi->m_pArrayData != 0)
		{
			result = tpi->m_pArrayData->getCount();
		}
	}
	return result;
}

//	FUNCTION public
//	tr_get_tuple_type --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_type_t
tr_get_tuple_type(const tr_tp_t* tuple,
				  int i)
{
	tr_type_t result = tr_type_undefined;
	
	if (tuple != 0 && i >= 0)
	{
		const tr_tp_t_impl* tpi =
			syd_reinterpret_cast<const tr_tp_t_impl*>(tuple);
		if (tpi->m_pArrayData != 0 && i < tpi->m_pArrayData->getCount())
		{
			const Common::Data* pData = tpi->m_pArrayData->getElement(i).get();
			result = static_cast<tr_type_t>(pData->getType());
		}
	}
	
	return result;
}

//	FUNCTION public
//	tr_release_tuple --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_ok_t
tr_release_tuple(tr_tp_t* tuple)
{
	if (tuple != 0)
	{
		tr_tp_t_impl* tpi = syd_reinterpret_cast<tr_tp_t_impl*>(tuple);
		try
		{
			if (tpi->m_bOwner && tpi->m_pArrayData != 0)
			{
				delete tpi->m_pArrayData;
			}
			delete tpi;
		}
		catch (...)
		{
			return tr_ng;
		}
	}
	return tr_ok;
}
	
//	FUNCTION public
//	tr_get_string_size --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" unsigned int
tr_get_string_size(const tr_tp_t* tuple,
				   int i)
{
	unsigned int result = 0;
	
	if (tuple != 0 && i >= 0)
	{
		const tr_tp_t_impl* tpi =
			syd_reinterpret_cast<const tr_tp_t_impl*>(tuple);
		if (tpi->m_pArrayData != 0 && i < tpi->m_pArrayData->getCount())
		{
			const Common::Data* pData = tpi->m_pArrayData->getElement(i).get();
//			const Common::StringData* p =
//				_SYDNEY_DYNAMIC_CAST(const Common::StringData*, pData);
//			ModUnicodeString s = p->getValue();
			ModUnicodeString s = pData->getString();
			const char* q = s.getString();
			result = s.getStringBufferSize();
		}
	}

	return result;
}

//	FUNCTION public
//	tr_get_string --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" int
tr_get_string(const tr_tp_t* tuple,
			  int i,
			  char* buf,
			  unsigned int bufsize)
{
	// bad argument
	int result = -1;
	
	if (tuple != 0 && i >= 0 && buf != 0)
	{
		const tr_tp_t_impl* tpi =
			syd_reinterpret_cast<const tr_tp_t_impl*>(tuple);
		if (tpi->m_pArrayData != 0 &&
			i < tpi->m_pArrayData->getCount())
		{
			if (bufsize == 0)
			{
				// string size
				result = 0;
			}
			else
			{
				tr_ds_t_impl* dsi = tpi->m_pTrDsImpl;
				
				const Common::Data* pData =
					tpi->m_pArrayData->getElement(i).get();
//				const Common::StringData* p =
//					_SYDNEY_DYNAMIC_CAST(const Common::StringData*, pData);
//				ModUnicodeString s = p->getString();
				ModUnicodeString s = pData->getString();
				
				const char* q = s.getString();
				unsigned int size = s.getStringBufferSize();
				
				unsigned int copysize = size;
				if (bufsize < size)
				{
					copysize = bufsize - 1;
				}
				try
				{
					// something error occurred
					result = -3;
					Os::Memory::copy(buf, q, copysize);
					if (bufsize < size)
					{
						buf[bufsize - 1] = '\0';
						// short of bufsize
						result = -2;
					}
					else
					{
						// string size
						result = copysize;
					}
				}
				catch (Exception::Object& e)
				{
					if (dsi) dsi->m_cException = e;
				}
				catch (ModException& e)
				{
					if (dsi) dsi->m_cException = Exception::ModLibraryError(
						moduleName, srcFile, __LINE__, e);
				}
				catch (...)
				{
					if (dsi) dsi->m_cException = Exception::Unexpected(
						moduleName, srcFile, __LINE__);
				}
			}
		}
	}

	return result;
}

//	FUNCTION public
//	tr_get_int --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" int
tr_get_int(const tr_tp_t* tuple,
		   int i)
{
	const tr_tp_t_impl* tpi = syd_reinterpret_cast<const tr_tp_t_impl*>(tuple);
	const Common::Data* pData = tpi->m_pArrayData->getElement(i).get();
	const Common::IntegerData* p =
		_SYDNEY_DYNAMIC_CAST(const Common::IntegerData*, pData);
	return p->getValue();
}

//	FUNCTION public
//	tr_get_unsignedint --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" unsigned int
tr_get_unsignedint(const tr_tp_t* tuple,
				   int i)
{
	const tr_tp_t_impl* tpi = syd_reinterpret_cast<const tr_tp_t_impl*>(tuple);
	const Common::Data* pData = tpi->m_pArrayData->getElement(i).get();
	const Common::UnsignedIntegerData* p =
		_SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData*, pData);
	return p->getValue();
}

//	FUNCTION public
//	tr_get_bigint --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" long long
tr_get_bigint(const tr_tp_t* tuple,
			  int i)
{
	const tr_tp_t_impl* tpi = syd_reinterpret_cast<const tr_tp_t_impl*>(tuple);
	const Common::Data* pData = tpi->m_pArrayData->getElement(i).get();
	const Common::Integer64Data* p =
		_SYDNEY_DYNAMIC_CAST(const Common::Integer64Data*, pData);
	return p->getValue();
}

//	FUNCTION public
//	tr_get_float --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" double
tr_get_float(const tr_tp_t* tuple,
			 int i)
{
	const tr_tp_t_impl* tpi = syd_reinterpret_cast<const tr_tp_t_impl*>(tuple);
	const Common::Data* pData = tpi->m_pArrayData->getElement(i).get();
	const Common::DoubleData* p =
		_SYDNEY_DYNAMIC_CAST(const Common::DoubleData*, pData);
	return p->getValue();
}

//	FUNCTION public
//	tr_get_decimal_size --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" unsigned int
tr_get_decimal_size(const tr_tp_t* tuple,
					int i)
{
	const tr_tp_t_impl* tpi = syd_reinterpret_cast<const tr_tp_t_impl*>(tuple);
	const Common::Data* pData = tpi->m_pArrayData->getElement(i).get();
	const Common::DecimalData* p =
		_SYDNEY_DYNAMIC_CAST(const Common::DecimalData*, pData);
	ModUnicodeString s = p->getString();
	const char* q = s.getString();
	return s.getBufferSize();
}

//	FUNCTION public
//	tr_get_decimal --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" void
tr_get_decimal(const tr_tp_t* tuple,
			   int i,
			   char* decimal)
{
	const tr_tp_t_impl* tpi = syd_reinterpret_cast<const tr_tp_t_impl*>(tuple);
	const Common::Data* pData = tpi->m_pArrayData->getElement(i).get();
	const Common::DecimalData* p =
		_SYDNEY_DYNAMIC_CAST(const Common::DecimalData*, pData);
	ModUnicodeString s = p->getString();
	const char* q = s.getString();
	Os::Memory::copy(decimal, q, s.getBufferSize());
}

//	FUNCTION public
//	tr_getLanguage_size --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" unsigned int
tr_get_language_size(const tr_tp_t* tuple,
					 int i)
{
	const tr_tp_t_impl* tpi = syd_reinterpret_cast<const tr_tp_t_impl*>(tuple);
	const Common::Data* pData = tpi->m_pArrayData->getElement(i).get();
	const Common::LanguageData* p =
		_SYDNEY_DYNAMIC_CAST(const Common::LanguageData*, pData);
	const ModLanguageSet& c = p->getValue();
	ModUnicodeString s = c.getName();
	const char* q = s.getString();
	return s.getStringBufferSize();
}

//	FUNCTION public
//	tr_get_language --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" void
tr_get_language(const tr_tp_t* tuple,
				int i,
				char* language)
{
	const tr_tp_t_impl* tpi = syd_reinterpret_cast<const tr_tp_t_impl*>(tuple);
	const Common::Data* pData = tpi->m_pArrayData->getElement(i).get();
	const Common::LanguageData* p =
		_SYDNEY_DYNAMIC_CAST(const Common::LanguageData*, pData);
	const ModLanguageSet& c = p->getValue();
	ModUnicodeString s = c.getName();
	const char* q = s.getString();
	Os::Memory::copy(language, q, s.getStringBufferSize());
}

//	FUNCTION public
//	tr_get_binary_size --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" unsigned int
tr_get_binary_size(const tr_tp_t* tuple,
				   int i)
{
	const tr_tp_t_impl* tpi = syd_reinterpret_cast<const tr_tp_t_impl*>(tuple);
	const Common::Data* pData = tpi->m_pArrayData->getElement(i).get();
	const Common::BinaryData* p =
		_SYDNEY_DYNAMIC_CAST(const Common::BinaryData*, pData);
	return p->getSize();
}

//	FUNCTION public
//	tr_get_binary --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" void
tr_get_binary(const tr_tp_t* tuple,
			  int i,
			  void* buf,
			  unsigned int bufsize)
{
	int result = -1;

	if (tuple != 0 && i >= 0 && buf != 0)
	{
		const tr_tp_t_impl* tpi =
			syd_reinterpret_cast<const tr_tp_t_impl*>(tuple);
		if (tpi->m_pArrayData != 0 &&
			i < tpi->m_pArrayData->getCount())
		{
			if (bufsize == 0)
			{
				// string size
				result = 0;
			}
			else
			{
				tr_ds_t_impl* dsi = tpi->m_pTrDsImpl;
				
				const Common::Data* pData =
					tpi->m_pArrayData->getElement(i).get();
				const Common::BinaryData* p =
					_SYDNEY_DYNAMIC_CAST(const Common::BinaryData*, pData);
				
				unsigned int size = p->getSize();
				
				unsigned int copysize = size;
				if (bufsize < size)
				{
					copysize = bufsize;
				}
				try
				{
					// something error occurred
					result = -3;
					Os::Memory::copy(buf, p->getValue(), copysize);
					if (bufsize < size)
					{
						// short of bufsize
						result = -2;
					}
					else
					{
						// string size
						result = copysize;
					}
				}
				catch (Exception::Object& e)
				{
					if (dsi) dsi->m_cException = e;
				}
				catch (ModException& e)
				{
					if (dsi) dsi->m_cException = Exception::ModLibraryError(
						moduleName, srcFile, __LINE__, e);
				}
				catch (...)
				{
					if (dsi) dsi->m_cException =
						Exception::Unexpected(moduleName, srcFile, __LINE__);
				}
			}
		}
	}
}

//	FUNCTION public
//	tr_get_datetime_size --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" unsigned int
tr_get_datetime_size(const tr_tp_t* tuple,
					 int i)
{
	const tr_tp_t_impl* tpi = syd_reinterpret_cast<const tr_tp_t_impl*>(tuple);
	const Common::Data* pData = tpi->m_pArrayData->getElement(i).get();
	const Common::DateTimeData* p =
		_SYDNEY_DYNAMIC_CAST(const Common::DateTimeData*, pData);
	ModUnicodeString s = p->getString();
	const char* q = s.getString();
	return s.getStringBufferSize();
}

//	FUNCTION public
//	tr_get_datetime --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" void
tr_get_datetime(const tr_tp_t* tuple,
				int i,
				char* datetime)
{
	const tr_tp_t_impl* tpi = syd_reinterpret_cast<const tr_tp_t_impl*>(tuple);
	const Common::Data* pData = tpi->m_pArrayData->getElement(i).get();
	const Common::DateTimeData* p =
		_SYDNEY_DYNAMIC_CAST(const Common::DateTimeData*, pData);
	ModUnicodeString s = p->getString();
	const char* q = s.getString();
	Os::Memory::copy(datetime, q, s.getStringBufferSize());
}

//	FUNCTION public
//	tr_get_array --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_tp_t*
tr_get_array(tr_tp_t* tuple,
			 int i)
{
	tr_tp_t_impl* subtuple = 0;
	
	if (tuple != 0 && i >= 0)
	{
		tr_tp_t_impl* tpi = syd_reinterpret_cast<tr_tp_t_impl*>(tuple);
		if (tpi->m_pArrayData != 0 &&
			i < tpi->m_pArrayData->getCount())
		{
			tr_ds_t_impl* dsi = tpi->m_pTrDsImpl;
			
			Common::Data* pData = tpi->m_pArrayData->getElement(i).get();
			Common::DataArrayData* p =
				_SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pData);
			
			try
			{
				subtuple = new tr_tp_t_impl();
				subtuple->m_pArrayData = p;
				subtuple->m_bOwner = false;
				subtuple->m_pTrDsImpl = dsi;
			}
			catch (Exception::Object& e)
			{
				if (dsi) dsi->m_cException = e;
				delete subtuple;
			}
			catch (ModException& e)
			{
				if (dsi) dsi->m_cException = Exception::ModLibraryError(
					moduleName, srcFile, __LINE__, e);
				delete subtuple;
			}
			catch (...)
			{
				if (dsi) dsi->m_cException = Exception::Unexpected(
					moduleName, srcFile, __LINE__);
				delete subtuple;
			}
		}
	}
	
	return syd_reinterpret_cast<tr_tp_t*>(subtuple);
}

//	FUNCTION public
//	tr_release_array --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_ok_t
tr_release_array(tr_tp_t* array)
{
	return tr_release_tuple(array);
}

//	FUNCTION public
//	tr_get_array_size --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" int
tr_get_array_size(const tr_tp_t* array)
{
	return tr_get_tuple_size(array);
}

//	FUNCTION public
//	tr_create_array --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_tp_t*
tr_create_array()
{
	tr_tp_t_impl* tpi = 0;

	try
	{
		tpi = new tr_tp_t_impl();
		tpi->m_pArrayData = new Common::DataArrayData();
		tpi->m_bOwner = true;
		tpi->m_pTrDsImpl = 0;
	}
	catch (...) {}

	return syd_reinterpret_cast<tr_tp_t*>(tpi);
}
	
//	FUNCTION public
//	tr_set_string --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_ok_t
tr_set_string(tr_tp_t* array,
			  int i,
			  char* buf,
			  unsigned int size)
{
	tr_ok_t result = tr_ng;

	if (array != 0 && i >= 0 && buf != 0)
	{
		tr_tp_t_impl* tpi = syd_reinterpret_cast<tr_tp_t_impl*>(array);

		if (tpi->m_pArrayData)
		{
			try
			{
				Common::StringData* pStringData = new Common::StringData();
				Common::Data::Pointer p = pStringData;
				
				// ModUnicodeString に渡すサイズは文字数なので、
				// 新しいバッファを確保し、終端を null にする

				char* b = new char[size + 1];
				try {
					Os::Memory::copy(b, buf, size);
					b[size] = 0;
					pStringData->setValue(ModUnicodeString(b));
					delete[] b;
				} catch (...) {
					delete[] b;
					throw;
				}
				
				tpi->m_pArrayData->setElement(i, p);

				result = tr_ok;
			}
			catch (...) {}
		}
	}

	return result;
}

//	FUNCTION public
//	tr_set_int --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_ok_t
tr_set_int(tr_tp_t* array,
		   int i,
		   int v)
{
	tr_ok_t result = tr_ng;

	if (array != 0 && i >= 0)
	{
		tr_tp_t_impl* tpi = syd_reinterpret_cast<tr_tp_t_impl*>(array);

		if (tpi->m_pArrayData)
		{
			try
			{
				Common::IntegerData* pIntegerData = new Common::IntegerData();
				Common::Data::Pointer p = pIntegerData;
				pIntegerData->setValue(v);
				
				tpi->m_pArrayData->setElement(i, p);

				result = tr_ok;
			}
			catch (...) {}
		}
	}

	return result;
}

//	FUNCTION public
//	tr_set_unsignedint --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_ok_t
tr_set_unsignedint(tr_tp_t* array,
				   int i,
				   unsigned int v)
{
	tr_ok_t result = tr_ng;

	if (array != 0 && i >= 0)
	{
		tr_tp_t_impl* tpi = syd_reinterpret_cast<tr_tp_t_impl*>(array);

		if (tpi->m_pArrayData)
		{
			try
			{
				Common::UnsignedIntegerData*
					pUnsignedIntegerData = new Common::UnsignedIntegerData();
				Common::Data::Pointer p = pUnsignedIntegerData;
				pUnsignedIntegerData->setValue(v);
				
				tpi->m_pArrayData->setElement(i, p);

				result = tr_ok;
			}
			catch (...) {}
		}
	}

	return result;
}

//	FUNCTION public
//	tr_set_bigint --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_ok_t
tr_set_bigint(tr_tp_t* array,
		   int i,
		   long long v)
{
	tr_ok_t result = tr_ng;

	if (array != 0 && i >= 0)
	{
		tr_tp_t_impl* tpi = syd_reinterpret_cast<tr_tp_t_impl*>(array);

		if (tpi->m_pArrayData)
		{
			try
			{
				Common::Integer64Data*
					pInteger64Data = new Common::Integer64Data();
				Common::Data::Pointer p = pInteger64Data;
				pInteger64Data->setValue(v);
				
				tpi->m_pArrayData->setElement(i, p);

				result = tr_ok;
			}
			catch (...) {}
		}
	}

	return result;
}

//	FUNCTION public
//	tr_set_float --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_ok_t
tr_set_float(tr_tp_t* array,
			 int i,
			 double v)
{
	tr_ok_t result = tr_ng;

	if (array != 0 && i >= 0)
	{
		tr_tp_t_impl* tpi = syd_reinterpret_cast<tr_tp_t_impl*>(array);

		if (tpi->m_pArrayData)
		{
			try
			{
				Common::DoubleData* pDoubleData = new Common::DoubleData();
				Common::Data::Pointer p = pDoubleData;
				pDoubleData->setValue(v);
				
				tpi->m_pArrayData->setElement(i, p);

				result = tr_ok;
			}
			catch (...) {}
		}
	}

	return result;
}

//	FUNCTION public
//	tr_set_decimal --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_ok_t
tr_set_decimal(tr_tp_t* array,
			   int i,
			   char* decimal,
			   unsigned int size)
{
	// 面倒なのでとりあえず文字列で設定しちゃう
	// エラー検出をサーバ側にゆだねるという意味もある
	
	return tr_set_string(array, i, decimal, size);
}

//	FUNCTION public
//	tr_set_language --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_ok_t
tr_set_language(tr_tp_t* array,
				int i,
				char* lang,
				unsigned int size)
{
	// 面倒なのでとりあえず文字列で設定しちゃう
	// エラー検出をサーバ側にゆだねるという意味もある
	
	return tr_set_string(array, i, lang, size);
}

//	FUNCTION public
//	tr_set_datetime --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_ok_t
tr_set_datetime(tr_tp_t* array,
				int i,
				char* datetime,
				unsigned int size)
{
	// 面倒なのでとりあえず文字列で設定しちゃう
	// エラー検出をサーバ側にゆだねるという意味もある
	
	return tr_set_string(array, i, datetime, size);
}

//	FUNCTION public
//	tr_set_binary --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_ok_t
tr_set_binary(tr_tp_t* array,
			  int i,
			  void* buf,
			  unsigned int size)
{
	tr_ok_t result = tr_ng;

	if (array != 0 && i >= 0 && buf != 0)
	{
		tr_tp_t_impl* tpi = syd_reinterpret_cast<tr_tp_t_impl*>(array);

		if (tpi->m_pArrayData)
		{
			try
			{
				Common::BinaryData* pBinaryData = new Common::BinaryData();
				Common::Data::Pointer p = pBinaryData;
				pBinaryData->setValue(buf, size);
				
				tpi->m_pArrayData->setElement(i, p);

				result = tr_ok;
			}
			catch (...) {}
		}
	}

	return result;
}

//	FUNCTION public
//	tr_set_array --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_ok_t
tr_set_array(tr_tp_t* array,
			 int i,
			 tr_tp_t* v)
{
	tr_ok_t result = tr_ng;

	if (array != 0 && i >= 0)
	{
		tr_tp_t_impl* tpi = syd_reinterpret_cast<tr_tp_t_impl*>(array);
		tr_tp_t_impl* vi = syd_reinterpret_cast<tr_tp_t_impl*>(v);

		if (tpi->m_pArrayData && vi->m_pArrayData)
		{
			try
			{
				tpi->m_pArrayData->setElement(i, vi->m_pArrayData->copy());
				result = tr_ok;
			}
			catch (...) {}
		}
	}

	return result;
}

//	FUNCTION public
//	tr_set_null --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
extern "C" tr_ok_t
tr_set_null(tr_tp_t* array,
			int i)
{
	tr_ok_t result = tr_ng;

	if (array != 0 && i >= 0)
	{
		tr_tp_t_impl* tpi = syd_reinterpret_cast<tr_tp_t_impl*>(array);

		if (tpi->m_pArrayData)
		{
			try
			{
				Common::Data::Pointer p(Common::NullData::getInstance());
				tpi->m_pArrayData->setElement(i, p);

				result = tr_ok;
			}
			catch (...) {}
		}
	}

	return result;
}

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
