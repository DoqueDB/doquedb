// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ErrorRecovery.h -- エラー処理関連
// 
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_ERROR_RECOVERY_H
#define	__SYDNEY_SCHEMA_ERROR_RECOVERY_H

#include "Schema/Database.h"
#include "Common/Message.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

#define	_BEGIN_REORGANIZE_RECOVERY(_id_)	\
								/* UNDO中の場合エラー処理をしない */ \
								if (!bUndo_ && Schema::Database::isAvailable(_id_)) { \
									try {
#define _END_REORGANIZE_RECOVERY(_id_) \
									} catch (Exception::Object& e) { \
										SydErrorMessage << "Error recovery failed. FATAL. " << e << ModEndl; \
										/* データベースを使用不可能にする*/ \
										Schema::Database::setAvailability((_id_), false); \
										/* エラー処理中に発生した例外は再送しない */ \
										/* thru. */ \
									} catch (...) { \
										SydErrorMessage << "Error recovery failed. FATAL." << ModEndl; \
										/* データベースを使用不可能にする*/ \
										Schema::Database::setAvailability((_id_), false); \
										/* エラー処理中に発生した例外は再送しない */ \
										/* thru. */ \
									} \
								}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_ERROR_RECOVERY_H

//
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
