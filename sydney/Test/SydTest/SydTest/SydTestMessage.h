// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SydTestMessage.h -- Messageマクロ定義用
// 
// Copyright (c) 2000, 2001, 2007, 2023 Ricoh Company, Ltd.
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
#include "Common/Message.h"
#include "Common/Internal.h"

#define SydTestInfoMessage _SYDNEY_MESSAGE ("SydTest_MessageOutputInfo", Common::MessageStreamBuffer::LEVEL_INFO).offThread()
#define SydTestDebugMessage _SYDNEY_MESSAGE("SydTest_MessageOutputDebug", Common::MessageStreamBuffer::LEVEL_DEBUG).offThread()
#define SydTestErrorMessage _SYDNEY_MESSAGE("SydTest_MessageOutputError", Common::MessageStreamBuffer::LEVEL_ERROR).offThread()
#define SydTestTimeMessage _SYDNEY_MESSAGE("SydTest_MessageOutputTime", Common::MessageStreamBuffer::LEVEL_INFO).offThread()

bool isSydTestInfoMessage();
bool isSydTestDebugMessage();
bool isSydTestTimeMessage();

//
//	Copyright (c) 2000, 2001, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
