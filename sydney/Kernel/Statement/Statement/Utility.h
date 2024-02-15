// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Utility.h -- 便利関数
// 
// Copyright (c) 2005, 2006, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_UTILITY_H
#define __SYDNEY_STATEMENT_UTILITY_H

#ifndef __SY_DEFAULT_H
#error require #include "SyDefault.h"
#endif

#include "SyTypeName.h"

#include "Statement/Module.h"
#include "Common/InputArchive.h"
#include "Common/OutputArchive.h"
#include "Common/SQLData.h"
#include "Os/RWLock.h"
#include "ModArchive.h"
#include "ModUnicodeChar.h"
#include "ModVector.h"

class ModUnicodeString;

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

namespace Utility
{
	// 重複した文字を単一の文字に変換した文字列を作る
	void replaceDuplicateCharacter(const ModUnicodeChar* pHead_,
								   const ModUnicodeChar* pTail_,
								   ModUnicodeChar c_,
								   ModUnicodeString& cstrOutput_);

	// NAMESPACE
	// Statement::Utility::NameTable
	//
	// NOTES
	// アルファベットからなる文字列を高速に調べるマップのための定義

	namespace NameTable
	{
		// STRUCT
		// Statement::Utility::NameTable::Entry --
		//
		// NOTES

		struct Entry
		{
			char*	_key;
			int		_value;
		};

		// CLASS
		// Statement::Utility::NameTable::Map --
		//
		// NOTES
		// char型で構成される文字列からintを引く
		// 先頭2文字でハッシュ表を構成する

		class Map
		{
			typedef ModVector<Entry> Vector;
		public:
			Map() {m_bCreated = false;}
			~Map() {}

			// 与えられた文字列が対応するエントリーがあれば返す
			const Entry* getEntry(const Entry* p_,
								  const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_);

		private:
			// エントリーに含まれる文字か
			static bool isCandidate(ModUnicodeChar c_)
			{
				return (c_ >= (ModUnicodeChar)'A' && c_ <= (ModUnicodeChar)'Z');
			}

			// マップを作る
			void create(const Entry* p_);

			enum {
				Size = 'Z' - 'A' + 1,
			};

			// 先頭2文字をキーにしたハッシュ表
			Vector m_cMap[Size][Size];
			// 中身を保護するRWLock
			Os::RWLock m_cRWLock;
			// create済みかを示す
			bool m_bCreated;
		};
	}

	// serialize関連
	namespace Serialize
	{
		void SQLDataType(ModArchive& cArchiver_,
						 Common::SQLData& cTarget_);
		void CommonData(ModArchive& cArchiver_,
						Common::Data::Pointer& cTarget_);
		template <class Enum_>
		void EnumValue(ModArchive& cArchiver_,
					   Enum_& cTarget_);
		template <class Object_>
		void Object(ModArchive& cArchiver_,
					Object_*& cTarget_);
		template <class Object_>
		void ObjectVector(ModArchive& cArchiver_,
						  ModVector<Object_*>& vecTarget_);
	}
}

namespace Utility
{
namespace Serialize
{
// TEMPLATE FUNCTION public
//	Statement::Utility::Serialize::EnumValue -- 
//
// TEMPLATE ARGUMENTS
//	class Enum_
//	
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchiver_
//	Enum_& cTarget_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Enum_>
void
EnumValue(ModArchive& cArchiver_,
		  Enum_& cTarget_)
{
	int iValue;
	if (cArchiver_.isStore()) {
		iValue = static_cast<int>(cTarget_);
		cArchiver_ << iValue;
	} else {
		cArchiver_ >> iValue;
		cTarget_ = static_cast<Enum_>(iValue);
	}
}

// TEMPLATE FUNCTION public
//	Statement::Utility::Serialize::Object -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//	
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchiver_
//	Object_*& cTarget_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Object_>
void
Object(ModArchive& cArchiver_,
	   Object_*& cTarget_)
{
	if (cArchiver_.isStore()) {
		Common::OutputArchive& cOut = dynamic_cast<Common::OutputArchive&>(cArchiver_);
		cOut.writeObject(cTarget_);
	} else {
		Common::InputArchive& cIn = dynamic_cast<Common::InputArchive&>(cArchiver_);
		cTarget_ = dynamic_cast<Object_*>(cIn.readObject());
	}
}

// TEMPLATE FUNCTION public
//	Statement::Utility::Serialize::ObjectVector -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//	
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchiver_
//	ModVector<Object_*>& vecTarget_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Object_>
void
ObjectVector(ModArchive& cArchiver_,
			 ModVector<Object_*>& vecTarget_)
{
	ModSize n;
	if (cArchiver_.isStore()) {
		n = vecTarget_.getSize();
		cArchiver_ << n;
		if (n) {
			Common::OutputArchive& cOut = dynamic_cast<Common::OutputArchive&>(cArchiver_);
			for (ModSize i = 0; i < n; ++i) {
				cOut.writeObject(vecTarget_[i]);
			}
		}
	} else {
		cArchiver_ >> n;
		typename ModVector<Object_*>::Iterator iterator = vecTarget_.begin();
		const typename ModVector<Object_*>::Iterator last = vecTarget_.end();
		for (; iterator != last; ++iterator) {
			delete *iterator;
		}
		vecTarget_.erase(vecTarget_.begin(),
						 vecTarget_.end());
		if (n) {
			Common::InputArchive& cIn = dynamic_cast<Common::InputArchive&>(cArchiver_);
			for (ModSize i = 0; i < n; ++i) {
				Object_* p = dynamic_cast<Object_*>(cIn.readObject());
				vecTarget_.pushBack(p);
			}
		}
	}
}
}
}

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_UTILITY_H

//
//	Copyright (c) 2005, 2006, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
