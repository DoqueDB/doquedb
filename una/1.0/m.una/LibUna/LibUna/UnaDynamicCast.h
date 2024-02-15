// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SyDynamicCast.h -- define UNA's dynamic_cast
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

#ifndef __UNA_DYNAMICCAST_H
#define __UNA_DYNAMICCAST_H

//	MACRO
//	_UNA_DYNAMIC_CAST -- Macro of UNA's dynamic_cast
//
//	NOTES
//		Use this macro because dynamic_cast is slow.
//		But you should use normal 'dynamic_cast<>' at following case
//		* cast for multiple inheritance
//		* detect cast error
//

#ifdef DEBUG
#define	_UNA_DYNAMIC_CAST(t, d)	dynamic_cast<t>(d)
#else
#define	_UNA_DYNAMIC_CAST(t, d)	static_cast<t>(d)
#endif

#endif //__UNA_DYNAMICCAST_H

//
// Copyright (c) 2001, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
