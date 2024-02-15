// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataOperation.h --
// 
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOB_DATAOPERATION_H
#define __SYDNEY_LOB_DATAOPERATION_H

// #include <limits.h>
// #include <float.h>

#include "Lob/Module.h"
#include "Lob/LobFile.h"

#include "Os/Limits.h"

#include "ModUnicodeChar.h"

_SYDNEY_BEGIN
_SYDNEY_LOB_BEGIN

//
//	TEMPLATE CLASS
//	Lob::DataOperation --
//
//	NOTES
//
template <class TYPE>
class DataOperation
{
public:
	//コンストラクタ
	DataOperation(LobFile* pLobFile_) : m_pLobFile(pLobFile_) {}
	//デストラクタ
	virtual ~DataOperation() {}

	// 各型の最大長
	static ModSize getMaxSize()
	{
		return Os::Limits<int>::getMax() / sizeof(TYPE);
	}

	//
	//	FUNCTION public
	//	Lob::DataOperation<TYPE>::getSize -- データ長を得る
	//
	ModSize getSize(const ObjectID& cBlock_)
	{
		return m_pLobFile->getDataSize(cBlock_) / sizeof(TYPE);
	}
	
	//
	//	FUNCTION public
	//	Lob::DataOperation<TYPE>::get -- 内容を取り出す
	//
	AutoPointer<TYPE> get(const ObjectID& cBlock_,
						  ModSize uiPosition_, ModSize& uiLength_,
						  bool& isNull_)
	{
		AutoPointer<TYPE> p;
		if (uiLength_ > getMaxSize()) uiLength_ = getMaxSize();
		ModSize uiLength = uiLength_ * sizeof(TYPE);
		p = m_pLobFile->get(cBlock_, uiPosition_ * sizeof(TYPE),
							uiLength, isNull_);
		uiLength_ = uiLength / sizeof(TYPE);
		return p;
	}
	
	//
	//	FUNCTION public
	//	Lob::DataOperation<TYPE>::insert -- 挿入する
	//
	ObjectID insert(const TYPE* pBuffer_, ModSize uiLength_)
	{
		return m_pLobFile->insert(pBuffer_, uiLength_ * sizeof(TYPE));
	}

	//
	//	FUNCTION public
	//	Lob::DataOperation<TYPE>::update -- 更新する
	void update(const ObjectID& cBlock_,
				const TYPE* pBuffer_, ModSize uiLength_)
	{
		m_pLobFile->update(cBlock_, pBuffer_, uiLength_ * sizeof(TYPE));
	}
	
	//
	//	FUNCTION public
	//	Lob::DataOperation<TYPE>::append -- 追加する
	//
	void append(const ObjectID& cBlock_,
				const TYPE* pBuffer_, ModSize uiLength_)
	{
		m_pLobFile->append(cBlock_, pBuffer_, uiLength_ * sizeof(TYPE));
	}
	
	//
	//	FUNCTION public
	//	Lob::DataOperation<TYPE>::truncate -- 縮める
	//
	void truncate(const ObjectID& cBlock_, ModSize uiLength_)
	{
		m_pLobFile->truncate(cBlock_, uiLength_ * sizeof(TYPE));
	}
	
	//
	//	FUNCTION public
	//	Lob::LobFiLe<TYPE>::replace -- 内容を取り替える
	//
	void replace(const ObjectID& cBlock_, ModSize uiPosition_,
				 const TYPE* pBuffer_, ModSize uiLength_)
	{
		m_pLobFile->replace(cBlock_, uiPosition_ * sizeof(TYPE),
							pBuffer_, uiLength_ * sizeof(TYPE));
	}

private:
	// ファイル
	LobFile* m_pLobFile;
};

_SYDNEY_LOB_END
_SYDNEY_END

#endif //__SYDNEY_LOB_DATAOPERATION_H

//
//	Copyright (c) 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
