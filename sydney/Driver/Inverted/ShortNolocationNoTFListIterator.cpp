// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortNolocationNoTFListIterator.cpp --
// 
// Copyright (c) 2008, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Inverted/FakeError.h"
#include "Inverted/InvertedList.h"
#include "Inverted/InvertedUnit.h"
#include "Inverted/LeafPage.h"
#include "Inverted/ShortNolocationNoTFListIterator.h"
#include "Inverted/Types.h"

#include "Exception/Unexpected.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public
//	Inverted::ShortNolocationNoTFListIterator::ShortNolocationNoTFListIterator -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::InvertedList& cInvertedList_
//		転置リスト(Short or Batch)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ShortNolocationNoTFListIterator::ShortNolocationNoTFListIterator(InvertedList& cInvertedList_)
	: ShortBaseListIterator(cInvertedList_)
{
}

//
//	FUNCTION public
//	Inverted::ShortNolocationNoTFListIterator::~ShortNolocationNoTFListIterator -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ShortNolocationNoTFListIterator::~ShortNolocationNoTFListIterator()
{
}


#ifdef DEBUG
//
//	FUNCTION public
//	Inverted::ShortNolocationNoTFListIterator::expunge -- DEBUG用
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
ShortNolocationNoTFListIterator::expunge()
{
	ShortBaseListIterator::expunge();
	; _INVERTED_FAKE_ERROR(ShortNolocationNoTFListIterator::expunge);
}

//
//	FUNCTION public
//	Inverted::ShortNolocationNoTFListIterator::undoExpunge -- DEBUG用
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
ShortNolocationNoTFListIterator::undoExpunge(ModUInt32 uiDocumentID_, const ModInvertedSmartLocationList& cLocationList_)
{
	ShortBaseListIterator::undoExpunge(uiDocumentID_, cLocationList_);
	; _INVERTED_FAKE_ERROR(ShortNolocationNoTFListIterator::undoExpunge);
}
#endif

//
//	FUNCTION public
//	Inverted::ShortNolocationNoTFListIterator::getHeadAddress --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
ModUInt32*
ShortNolocationNoTFListIterator::getHeadAddress()
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
