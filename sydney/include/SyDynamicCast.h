// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SyDynamicCast.h -- Sydney のダイナミックキャストを定義する
// 
// Copyright (c) 2001, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_DYNAMICCAST_H
#define __SYDNEY_DYNAMICCAST_H

//	MACRO
//	_SYDNEY_DYNAMIC_CAST -- Sydney 固有のダイナミックキャストを表すマクロ
//
//	NOTES
//		dynamic_cast は遅いので、dynamic_cast でなければ実現できない
//
//		* 多重継承時の通常のキャストでは無理なキャスト
//		* キャストエラーの検出
//
//		などのコードを書くには dynamic_cast を使い、
//		それ以外ではこのマクロを使う

#ifdef DEBUG
#define	_SYDNEY_DYNAMIC_CAST(t, d)	dynamic_cast<t>(d)
#else
#define	_SYDNEY_DYNAMIC_CAST(t, d)	static_cast<t>(d)
#endif

#endif //__SYDNEY_DYNAMICCAST_H

//
// Copyright (c) 2001, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
