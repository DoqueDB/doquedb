// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedIterator.h -- 
// 
// Copyright (c) 1997, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedIterator_H__
#define __ModInvertedIterator_H__

#include "ModOs.h"
#include "ModInvertedTypes.h"
#include "ModInvertedManager.h"

class ModInvertedLocationListIterator;

//
// CLASS
// ModInvertedIterator --- 反復子
//
// NOTES
// 反復子のインタフェースを規定する抽象クラス。
// 
class
ModInvertedIterator : public ModInvertedObject {
public:
	typedef ModInvertedDocumentID DocumentID;

    virtual ~ModInvertedIterator() {}
    void operator++();

    virtual void next() = 0;
    virtual void reset() = 0;
    virtual ModBoolean find(const DocumentID documentId) = 0;
    virtual ModBoolean lowerBound(const DocumentID documentId) = 0;
    virtual ModBoolean isEnd() const = 0;
    virtual DocumentID getDocumentId() = 0;
    virtual ModSize getInDocumentFrequency() = 0;
    virtual ModInvertedLocationListIterator* getLocationListIterator() = 0;
private:
};


//
// FUNCTION
// ModInvertedIterator::operator++ -- インクリメントオペレータ
//
// NOTES
// 転置リストの反復子の現在位置を前に進める。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void 
ModInvertedIterator::operator++()
{
	next();
}

#endif	__ModInvertedIterator_H__

//
// Copyright (c) 1997, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

