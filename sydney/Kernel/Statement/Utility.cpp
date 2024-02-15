// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Utility.cpp -- 便利関数
// 
// Copyright (c) 2005, 2006, 2008, 2009, 2011, 2012, 2023 Ricoh Company, Ltd.
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
	const char moduleName[] = "Statement";
	const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"
#include "Statement/Utility.h"
#include "Common/Assert.h"
#include "Common/StringData.h"
#include "Os/AutoRWLock.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

// FUNCTION public
//	Statement::Utility::replaceDuplicateCharacter -- 重複した文字を単一の文字に変換した文字列を作る
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeChar* pHead_
//	const ModUnicodeChar* pTail_
//	ModUnicodeChar c_
//	ModUnicodeString& cstrOutput_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Utility::
replaceDuplicateCharacter(const ModUnicodeChar* pHead_,
						  const ModUnicodeChar* pTail_,
						  ModUnicodeChar c_,
						  ModUnicodeString& cstrOutput_)
{
	// 空文字列に対しては呼ばれない
	; _SYDNEY_ASSERT(pHead_ < pTail_);

	cstrOutput_.reallocate(static_cast<ModSize>(pTail_ - pHead_));
	const ModUnicodeChar* p = pHead_;
	const ModUnicodeChar* q = p;
	for(; q < pTail_; ++q) {
		if (*q == c_ && *(q+1) == c_) {
			if (p < q) {
				// qの直前までコピーする
				cstrOutput_.append(p, static_cast<ModSize>(q - p));
			}
			p = ++q; // 次のコピーでc_がひとつコピーされる
		}
	}
	// 残りをコピーする
	; _SYDNEY_ASSERT(p < q);
	cstrOutput_.append(p, static_cast<ModSize>(q - p));
}

// FUNCTION public
//	Statement::Utility::NameTable::Map::getEntry -- 与えられた文字列が対応するエントリーがあれば返す
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeChar* pHead_
//	const ModUnicodeChar* pTail_
//	
// RETURN
//	const Utility::NameTable::Entry*
//
// EXCEPTIONS

const Utility::NameTable::Entry*
Utility::NameTable::Map::
getEntry(const Entry* p_,
		 const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_)
{
	// 2文字以上のものしか扱わない
	if (pTail_ - pHead_ >= 2) {
		// 1文字目と2文字目が英字である
		ModUnicodeChar c1 = ModUnicodeCharTrait::toUpper(*pHead_);
		ModUnicodeChar c2 = ModUnicodeCharTrait::toUpper(*(pHead_ + 1));
		if (isCandidate(c1) && isCandidate(c2)) {

			Os::AutoTryRWLock	lock(m_cRWLock);
			lock.lock(Os::RWLock::Mode::Read);

			if (!m_bCreated) {
				// Readのロックをはずし、Writeでロックしなおしてからマップを作る
				lock.unlock();
				lock.lock(Os::RWLock::Mode::Write);

				if (!m_bCreated) {
					// ここで再度チェック
					create(p_);
					m_bCreated = true;		
				}

				// Readでロックしなおす
				lock.unlock();
				lock.lock(Os::RWLock::Mode::Read);
			}

			const Vector& cVector = m_cMap[c1 - (ModUnicodeChar)'A'][c2 - (ModUnicodeChar)'A'];
			if (!cVector.isEmpty()) {
				Vector::ConstIterator iterator = cVector.begin();
				const Vector::ConstIterator& last = cVector.end();
				do {
					const Entry& cEntry = *iterator;
					// 3文字目以降について調べる
					const char* p = cEntry._key + 2;
					const ModUnicodeChar* q = pHead_ + 2;

					while (*p && q < pTail_
						   && (ModUnicodeChar)*p == ModUnicodeCharTrait::toUpper(*q))
						++p, ++q;

					if (!*p && q == pTail_) return &cEntry;

				} while (++iterator != last);
			}
		}
	}
	return 0;
}

// FUNCTION public
//	Statement::Utility::NameTable::Map::create -- マップを作る
//
// NOTES
//
// ARGUMENTS
//	const Element* p_
//		マップの元になるElementの配列。最終要素のキーを0にすること。
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Utility::NameTable::Map::
create(const Entry* p_)
{
	for (int i = 0; p_[i]._key; ++i) {
		; _SYDNEY_ASSERT(isCandidate((ModUnicodeChar)p_[i]._key[0]));
		; _SYDNEY_ASSERT(isCandidate((ModUnicodeChar)p_[i]._key[1]));

		m_cMap[(ModUnicodeChar)p_[i]._key[0] - (ModUnicodeChar)'A']
			  [(ModUnicodeChar)p_[i]._key[1] - (ModUnicodeChar)'A'].pushBack(p_[i]);
	}
}

// FUNCTION public
//	Statement::Utility::Serialize::SQLDataType -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchiver_
//	Common::SQLData& cTarget_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Utility::Serialize::
SQLDataType(ModArchive& cArchiver_,
			Common::SQLData& cTarget_)
{
	if (cArchiver_.isStore()) {
		cTarget_.serialize(cArchiver_);
		int iValue = static_cast<int>(cTarget_.getCollation());
		cArchiver_ << iValue;
	} else {
		cTarget_.serialize(cArchiver_);
		int iValue;
		cArchiver_ >> iValue;
		cTarget_.setCollation(static_cast<Common::Collation::Type::Value>(iValue));
	}
}

// FUNCTION public
//	Statement::Utility::Serialize::CommonData -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchiver_
//	Common::Data::Pointer& cTarget_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Utility::Serialize::
CommonData(ModArchive& cArchiver_,
		   Common::Data::Pointer& cTarget_)
{
	if (cArchiver_.isStore()) {
		Common::OutputArchive& cOut = dynamic_cast<Common::OutputArchive&>(cArchiver_);
		cOut.writeObject(cTarget_.get());
		if (cTarget_->getType() == Common::DataType::String) {
			// serialize collation and encoding form (for compatibility)
			Common::StringData* pStringData =
				_SYDNEY_DYNAMIC_CAST(Common::StringData*, cTarget_.get());
			; _SYDNEY_ASSERT(pStringData);
			int iCollation = static_cast<int>(pStringData->getCollation());
			cArchiver_ << iCollation;
			int iEncodingForm = static_cast<int>(pStringData->getEncodingForm());
			cArchiver_ << iEncodingForm;
		}
	} else {
		Common::InputArchive& cIn = dynamic_cast<Common::InputArchive&>(cArchiver_);
		cTarget_ = dynamic_cast<Common::Data*>(cIn.readObject());
		if (cTarget_->getType() == Common::DataType::String) {
			// serialize collation and encoding form (for compatibility)
			Common::StringData* pStringData =
				_SYDNEY_DYNAMIC_CAST(Common::StringData*, cTarget_.get());
			; _SYDNEY_ASSERT(pStringData);
			int iValue;
			cArchiver_ >> iValue;
			pStringData->setCollation(static_cast<Common::Collation::Type::Value>(iValue));
			cArchiver_ >> iValue;
			pStringData->setEncodingForm(static_cast<Common::StringData::EncodingForm::Value>(iValue));
		}
	}
}

//
//	Copyright (c) 2005, 2006, 2008, 2009, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
