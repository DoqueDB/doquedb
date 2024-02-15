// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Module.h --	運用管理モジュールのソースコードを
//				書くために使用するマクロ定義
// 
// Copyright (c) 2001, 2004, 2007, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_ADMIN_MODULE_H
#define __SYDNEY_ADMIN_MODULE_H

#include "Common/Common.h"
#include "Common/Internal.h"

#define	_SYDNEY_ADMIN_BEGIN				namespace Admin {
#define	_SYDNEY_ADMIN_END				}
#define _SYDNEY_ADMIN_USING				using namespace Admin;

#define _SYDNEY_ADMIN_RECOVERY_BEGIN	namespace Recovery {
#define _SYDNEY_ADMIN_RECOVERY_END		}
#define _SYDNEY_ADMIN_RECOVERY_USING	using namespace Recovery;

#define _SYDNEY_ADMIN_REORGANIZATION_BEGIN	namespace Reorganization {
#define _SYDNEY_ADMIN_REORGANIZATION_END	}
#define _SYDNEY_ADMIN_REORGANIZATION_USING	using namespace Reorganization;

#define _SYDNEY_ADMIN_STARTBACKUP_BEGIN	namespace StartBackup {
#define _SYDNEY_ADMIN_STARTBACKUP_END	}
#define _SYDNEY_ADMIN_STARTBACKUP_USING	using namespace StartBackup;

#define _SYDNEY_ADMIN_ENDBACKUP_BEGIN	namespace EndBackup {
#define _SYDNEY_ADMIN_ENDBACKUP_END		}
#define _SYDNEY_ADMIN_ENDBACKUP_USING	using namespace EndBackup;

#define _SYDNEY_ADMIN_MOUNT_BEGIN		namespace Mount {
#define _SYDNEY_ADMIN_MOUNT_END			}
#define _SYDNEY_ADMIN_MOUNT_USING		using namespace Mount;

#define _SYDNEY_ADMIN_UNMOUNT_BEGIN		namespace Unmount {
#define _SYDNEY_ADMIN_UNMOUNT_END		}
#define _SYDNEY_ADMIN_UNMOUNT_USING		using namespace Unmount;

#define _SYDNEY_ADMIN_UTILITY_BEGIN		namespace Utility {
#define _SYDNEY_ADMIN_UTILITY_END		}
#define _SYDNEY_ADMIN_UTILITY_USING		using namespace Utility;

#define	_SYDNEY_ADMIN_LOG_BEGIN			namespace Log {
#define	_SYDNEY_ADMIN_LOG_END			}
#define	_SYDNEY_ADMIN_LOG_USING			using namespace Log;

#ifdef SYD_DLL
#ifdef SYD_ADMIN_EXPORT_FUNCTION
#define SYD_ADMIN_FUNCTION	SYD_EXPORT
#else
#define SYD_ADMIN_FUNCTION	SYD_IMPORT
#endif
#else
#define	SYD_ADMIN_FUNCTION
#endif

#endif //__SYDNEY_ADMIN_MODULE_H

//
//	Copyright (c) 2001, 2004, 2007, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
