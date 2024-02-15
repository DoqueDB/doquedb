// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CClientImpl.h -- 
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

#ifndef __TRMEISTER_CLIENT2_CCLIENTIMPL_H
#define __TRMEISTER_CLIENT2_CCLIENTIMPL_H

#include "Client2/Module.h"

#include "Exception/Object.h"

_TRMEISTER_BEGIN

namespace Common
{
	class DataArrayData;
	class IntegerArrayData;
}

_TRMEISTER_CLIENT2_BEGIN

class DataSource;
class Session;
class PrepareStatement;
class ResultSet;

// DataSource
struct tr_ds_t_impl
{
	tr_ds_t_impl() : m_pDataSource(0) {}
	
	DataSource* m_pDataSource;
	Exception::Object m_cException;
};

// Session
struct tr_ss_t_impl
{
	tr_ss_t_impl() : m_pSession(0), m_pTrDsImpl(0) {}
	
	Session* m_pSession;
	tr_ds_t_impl* m_pTrDsImpl;
};

// ResultSet
struct tr_rs_t_impl
{
	tr_rs_t_impl() : m_pResultSet(0), m_pTrDsImpl(0) {}
	
	ResultSet* m_pResultSet;
	tr_ds_t_impl* m_pTrDsImpl;
};

// PrepareStatement
struct tr_ps_t_impl
{
	tr_ps_t_impl() : m_pPrepareStatement(0), m_pTrDsImpl(0) {}
	
	PrepareStatement* m_pPrepareStatement;
	tr_ds_t_impl* m_pTrDsImpl;
};

// Tuple
struct tr_tp_t_impl
{
	tr_tp_t_impl() : m_pArrayData(0), m_bOwner(false), m_pTrDsImpl(0) {}
	
	Common::DataArrayData* m_pArrayData;
	bool m_bOwner;
	tr_ds_t_impl* m_pTrDsImpl;
};

_TRMEISTER_CLIENT2_END
_TRMEISTER_END

#endif //__TRMEISTER_CLIENT2_CCLIENTIMPL_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
