// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModWideChar.h -- ModWideChar のクラス定義
// 
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModWideChar_H__
#define __ModWideChar_H__

//
// TYPEDEF
// ModWideChar -- マルチバイト文字を表す型
//
// NOTES
// この型は ASCII とマルチバイト文字を同じ型で扱うために用いる。
// ModString などで大きな配列にする可能性があるため、class にはしない。
// char* との相互変換などの ModWideChar への操作は ModWideCharTrait という
// クラスが提供する。
//
typedef int  ModWideChar;

#endif // __ModWideChar_H__

//
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
