// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SyReinterpretCast.h -- Sydney の reinterpret_cast を定義する
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

#ifndef __SYDNEY_REINTERPRETCAST_H
#define __SYDNEY_REINTERPRETCAST_H

//	NOTES
//		reinterpret_cast は UNIX 系 OS では遅いので、
//		C言語スタイルのキャストに変換する

#ifdef SYD_OS_WINDOWS
#define syd_reinterpret_cast	reinterpret_cast
#else
template <class D, class S> D syd_reinterpret_cast(S s) { return (D)s; }
#endif

#endif //__SYDNEY_REINTERPRETCAST_H

//
// Copyright (c) 2008, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
