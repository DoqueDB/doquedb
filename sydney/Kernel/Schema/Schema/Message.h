// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Message.h -- スキーマモジュール内部で使うメッセージに関する定義
// 
// Copyright (c) 2000, 2006, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_MESSAGE_H
#define	__SYDNEY_SCHEMA_MESSAGE_H

#include "Schema/Module.h"

#include "Common/Message.h"
#include "Common/SystemParameter.h"
#include "Common/UnicodeString.h"

#include "Exception/Message.h"

#include "ModCharString.h"
#include "ModUnicodeChar.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

namespace Message {
	const ModCharString Debug("Schema_MessageOutputDebug");
	const ModCharString ReportSystemTable("Schema_ReportSystemTable");
	const ModCharString ReportReorganization("Schema_ReportReorganization");
	const ModCharString Verification("Schema_Verification_MessageOutputInfo");

	const ModUnicodeChar Zero[] = {Common::UnicodeChar::usZero, Common::UnicodeChar::usNull};
}

// ErrorとInfoレベルのメッセージは共通のものを使う

#define SydSchemaDebugMessage	_SYDNEY_MESSAGE( \
									Message::Debug, \
									Common::MessageStreamBuffer::LEVEL_DEBUG)
#define SydSchemaVerifyMessage	_SYDNEY_MESSAGE( \
									Message::Verification, \
									Common::MessageStreamBuffer::LEVEL_INFO)
#define SydSchemaParameterMessage(_param_) \
								if (Common::SystemParameter::getString(_param_) == Schema::Message::Zero) \
									; \
								else \
									_SYDNEY_MESSAGE( \
										(_param_), \
										Common::MessageStreamBuffer::LEVEL_DEBUG)

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_MESSAGE_H

//
// Copyright (c) 2000, 2006, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
