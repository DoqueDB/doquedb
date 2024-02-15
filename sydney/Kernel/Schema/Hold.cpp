// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Hold.cpp -- データベースオブジェクトのロック関連の関数定義
// 
// Copyright (c) 2001, 2002, 2005, 2007, 2008, 2011, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Schema/Hold.h"
#include "Schema/Message.h"
#ifdef DEBUG
#include "Schema/Debug.h"
#endif
#include "Common/Assert.h"
#include "Lock/Mode.h"
#include "Lock/Duration.h"
#include "Trans/Transaction.h"

#ifdef DEBUG

// ロックモードと持続期間をメッセージに出力するための関数の宣言

ModMessageStream& operator<<(ModMessageStream& cStream_,
							 _SYDNEY::Schema::Hold::Target::Value eValue_);
ModOstream& operator<<(ModOstream& cStream_,
					   _SYDNEY::Schema::Hold::Target::Value eValue_);
#endif

_SYDNEY_USING

namespace _AdequateLock
{
	// Trans::Transactionの実装をコピーして修正した

	//	STRUCT
	//	$$$::_Transaction::_Info --
	//		ある条件時にかけるべきロックに関する情報を表すクラス
	//
	//	NOTES

	struct _Info
	{
		// ロックモード
		Lock::Mode::Value		_mode;
		// ロックの持続期間
		Lock::Duration::Value	_duration;
	};

#define	VIS		Lock::Mode::VIS
#define VS		Lock::Mode::VS
#define	IS		Lock::Mode::IS
#define	IX		Lock::Mode::IX
#define	S		Lock::Mode::S
#define	VIX		Lock::Mode::VIX
#define	VSVIX	Lock::Mode::VSVIX
#define	SIX		Lock::Mode::SIX
#define	U		Lock::Mode::U
#define	SVIX	Lock::Mode::SVIX
#define	X		Lock::Mode::X
#define	VIXX	Lock::Mode::VIXX
#define	VX		Lock::Mode::VX
#define	N		Lock::Mode::N

#define	Instant	Lock::Duration::Instant
#define	Stmt	Lock::Duration::Statement
#define	Short	Lock::Duration::Short
#define	Middle	Lock::Duration::Middle
#define	Long	Lock::Duration::Long

const _Info infoForNoVersionTransaction
	[][Lock::Name::Category::ValueNum]
	[Trans::Transaction::IsolationLevel::ValueNum][Lock::Name::Category::ValueNum] =
{
// for Drop operation
{
// Unknown 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }}
															// Serializable
},

// Database 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ VX,Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ VX,Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ VX,Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ VX,Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }}
															// Serializable
},

// Table 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ VIX,Middle },{ VX,Middle  },{ N, Instant },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ VIX,Middle },{ VX,Middle  },{ N, Instant },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ VIX,Middle },{ VX,Middle  },{ N, Instant },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ VIX,Middle },{ VX,Middle  },{ N, Instant },{ IX,Stmt    }}
															// Serializable
},

// Tuple 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ VIX,Middle },{ VIX,Middle },{ VX,Middle  },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ VIX,Middle },{ VIX,Middle },{ VX,Middle  },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ VIX,Middle },{ VIX,Middle },{ VX,Middle  },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ VIX,Middle },{ VIX,Middle },{ VX,Middle  },{ IX,Stmt    }}
															// Serializable
},

// LogicalLog 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }},
															// ReadCommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }},
															// RepeatableRead
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }}
															// Serializable
}
},

// for Move Database
{
// Unknown 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }}
															// Serializable
},

// Database 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ VX,Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ VX,Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ VX,Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ VX,Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }}
															// Serializable
},

// Table 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ VIX,Middle },{ SVIX,Middle},{ N, Instant },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ VIX,Middle },{ SVIX,Middle},{ N, Instant },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ VIX,Middle },{ SVIX,Middle},{ N, Instant },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ VIX,Middle },{ SVIX,Middle},{ N, Instant },{ IX,Stmt    }}
															// Serializable
},

// Tuple 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ VIX,Middle },{ VIX,Middle },{ VX,Middle  },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ VIX,Middle },{ VIX,Middle },{ VX,Middle  },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ VIX,Middle },{ VIX,Middle },{ VX,Middle  },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ VIX,Middle },{ VIX,Middle },{ VX,Middle  },{ IX,Stmt    }}
															// Serializable
},

// LogicalLog 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }},
															// ReadCommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }},
															// RepeatableRead
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }}
															// Serializable
}
},

// for Read for Write operation
{
// Unknown 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }}
															// Serializable
},

// Database 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ X, Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ X, Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ X, Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ X, Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }}
															// Serializable
},

// Table 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ IX,Middle  },{ X, Middle  },{ N, Instant },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ IX,Middle  },{ X, Middle  },{ N, Instant },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ IX,Middle  },{ X, Middle  },{ N, Instant },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ IX,Middle  },{ X, Middle  },{ N, Instant },{ IX,Stmt    }}
															// Serializable
},

// Tuple 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ IX,Middle  },{ IX,Middle  },{ U, Middle  },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ IX,Middle  },{ IX,Middle  },{ U, Middle  },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ IX,Middle  },{ IX,Middle  },{ U, Middle  },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ IX,Middle  },{ IX,Middle  },{ U, Middle  },{ IX,Stmt    }}
															// Serializable
},

// LogicalLog 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }},
															// ReadCommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }},
															// RepeatableRead
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }}
															// Serializable
}
},

// for Read for Import operation
{
// Unknown 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IS,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IS,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IS,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IS,Stmt    }}
															// Serializable
},

// Database 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IS,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ S, Short   },{ N, Instant },{ N, Instant },{ IS,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ S, Short   },{ N, Instant },{ N, Instant },{ IS,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ S, Short   },{ N, Instant },{ N, Instant },{ IS,Stmt    }}
															// Serializable
},

// Table 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ IS,Middle  },{ N, Instant },{ N, Instant },{ IS,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ IS,Middle  },{ S, Short   },{ N, Instant },{ IS,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ IS,Middle  },{ S, Short   },{ N, Instant },{ IS,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ IS,Middle  },{ S, Short   },{ N, Instant },{ IS,Stmt    }}
															// Serializable
},

// Tuple 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ IS,Middle  },{ IS,Middle  },{ N, Instant },{ IS,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ IS,Middle  },{ IS,Middle  },{ S, Short   },{ IS,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ IS,Middle  },{ IS,Middle  },{ S, Short   },{ IS,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ IS,Middle  },{ IS,Middle  },{ S, Short   },{ IS,Stmt    }}
															// Serializable
},

// LogicalLog 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ S, Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ S, Stmt    }},
															// ReadCommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ S, Stmt    }},
															// RepeatableRead
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ S, Stmt    }}
															// Serializable
}
},

// for Read Only operation
// 版を使う場合のみ以下の表が使われる

// for Read only operation (with versioning)
{
// Unknown 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IS,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IS,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IS,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IS,Stmt    }}
															// Serializable
},

// Database 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IS,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ VS,Stmt    },{ N, Instant },{ N, Instant },{ IS,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ VS,Middle  },{ N, Instant },{ N, Instant },{ IS,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ VS,Middle  },{ N, Instant },{ N, Instant },{ IS,Stmt    }}
															// Serializable
},

// Table 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IS,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ VIS,Stmt   },{ VS,Stmt    },{ N, Instant },{ IS,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ VIS,Middle },{ VS,Middle  },{ N, Instant },{ IS,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ VIS,Middle },{ VS,Middle  },{ N, Instant },{ IS,Stmt    }}
															// Serializable
},

// Tuple 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IS,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ VIS,Stmt   },{ VIS,Stmt   },{ VS,Stmt    },{ IS,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ VIS,Middle },{ VIS,Middle },{ VS,Middle  },{ IS,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ VIS,Middle },{ VIS,Middle },{ VS,Middle  },{ IS,Stmt    }}
															// Serializable
},

// LogicalLog 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ S, Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ S, Stmt    }},
															// ReadCommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ S, Stmt    }},
															// RepeatableRead
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ S, Stmt    }}
															// Serializable
}
},

// for Read Write operation
{
// Unknown 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }}
															// Serializable
},

// Database 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ X, Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ X, Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ X, Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ X, Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }}
															// Serializable
},

// Table 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ IX,Middle  },{ X, Middle  },{ N, Instant },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ IX,Middle  },{ X, Middle  },{ N, Instant },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ IX,Middle  },{ X, Middle  },{ N, Instant },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ SIX,Middle },{ X, Middle  },{ N, Instant },{ IX,Stmt    }}
															// Serializable
},

// Tuple 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ IX,Middle  },{ IX,Middle  },{ X, Middle  },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ IX,Middle  },{ IX,Middle  },{ X, Middle  },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ IX,Middle  },{ IX,Middle  },{ X, Middle  },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ IX,Middle  },{ SIX,Middle },{ X, Middle  },{ IX,Stmt    }}
															// Serializable
},

// LogicalLog 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }},
															// ReadCommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }},
															// RepeatableRead
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }}
															// Serializable
}
}

};

#undef	VIS
#undef	VS
#undef	IS
#undef	IX
#undef	S
#undef	VIX
#undef	VSVIX
#undef	SIX	
#undef	U
#undef	SVIX
#undef	X
#undef	VIXX
#undef	VX
#undef	N

#undef	Instant
#undef	Stmt
#undef	Short
#undef	Middle
#undef	Long

} // namespace _AdequateLock

namespace _Lock
{
	// CONST
	// $$$::_Lock::_TargetTable -- ロック対象を得るための配列
	//
	// NOTES
	//	Hold::Target::ValueをLock::Name::Category::Valueに変換する

	Lock::Name::Category::Value _TargetTable[] =
	{
		Lock::Name::Category::Database,		// MetaDatabase
		Lock::Name::Category::Table,		// MetaTable
		Lock::Name::Category::Tuple,		// MetaTuple
		Lock::Name::Category::Database,		// Database
		Lock::Name::Category::Table,		// Table
		Lock::Name::Category::Tuple,		// Tuple
		Lock::Name::Category::LogicalLog,	// LogicalLog
		Lock::Name::Category::Unknown		// ValueNum
	};
} // namespace _Lock

#ifdef DEBUG
//---------------------------------------------
//メッセージ出力でenum型を文字列で見るための定義
//---------------------------------------------

ModMessageStream& operator<<(ModMessageStream& cStream,
							 Schema::Hold::Target::Value eValue)
{
	switch (eValue) {
	case Schema::Hold::Target::MetaDatabase:	return cStream << "MetaDatabase";
	case Schema::Hold::Target::MetaTable:		return cStream << "MetaTable";
	case Schema::Hold::Target::MetaTuple:		return cStream << "MetaTuple";
	case Schema::Hold::Target::Database:		return cStream << "Database";
	case Schema::Hold::Target::Table:			return cStream << "Table";
	case Schema::Hold::Target::Tuple:			return cStream << "Tuple";
	case Schema::Hold::Target::LogicalLog:		return cStream << "LogicalLog";
	default:
		break;
	}
	return cStream << "???";
}

ModOstream& operator<<(ModOstream& cStream,
					   Schema::Hold::Target::Value eValue)
{
	switch (eValue) {
	case Schema::Hold::Target::MetaDatabase:	return cStream << "MetaDatabase";
	case Schema::Hold::Target::MetaTable:		return cStream << "MetaTable";
	case Schema::Hold::Target::MetaTuple:		return cStream << "MetaTuple";
	case Schema::Hold::Target::Database:		return cStream << "Database";
	case Schema::Hold::Target::Table:			return cStream << "Table";
	case Schema::Hold::Target::Tuple:			return cStream << "Tuple";
	case Schema::Hold::Target::LogicalLog:		return cStream << "LogicalLog";
	default:
		break;
	}
	return cStream << "???";
}
#endif

_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::Hold::getAdequateLock -- 適切なロックモードを求める
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をしているトランザクション記述子
//		Lock::Name::Category::Value	eLocked_
//			ロックするオブジェクトの種類
//		Lock::Name::Category::Value	eManipulate_
//			操作するオブジェクトの種類
//		Schema::Hold::Operation::Value eOperation_
//			操作の種類
//		Lock::Mode::Value&	eReturnMode_
//			かけるべきロックのモードが設定され、
//			Lock::Mode::N が設定されたときは、ロックする必要はない
//		Lock::Duration::Value&	eReturnDuration_
//			かけるべきロックの持続期間が設定される
//
//	RETURN
//		true
//			ロックする必要がある
//		false
//			ロックする必要はない
//
//	EXCEPTIONS

bool
Hold::
getAdequateLock(Trans::Transaction& cTrans_,
				Lock::Name::Category::Value eLocked_,
				Lock::Name::Category::Value eManipulate_,
				Operation::Value eOperation_,
				Lock::Mode::Value& eReturnMode_,
				Lock::Duration::Value& eReturnDuration_)
{
	switch (eOperation_) {
	case Operation::ReadForImport:
		// 版を使う場合もMetaDatabaseのロックモードは独自の表を使う
		// 版を使う場合はReadOnlyに対応する独自の表にする
		if (!cTrans_.isNoVersion()) {
			eOperation_ = Operation::ReadOnly;
		}
		break;

	case Operation::ReadOnly:

		// トランザクションモジュールに決めてもらう

		return cTrans_.getAdequateLock(eLocked_, eManipulate_,
									   eOperation_ != Operation::ReadWrite,
									   eReturnMode_, eReturnDuration_);
	case Operation::ReadWrite:
		// ReadWriteもMetaDatabaseのロックモードは独自の表を使う
		break;
	}

	// スキーマが独自に決める

	; _SYDNEY_ASSERT(cTrans_.getStatus() ==
					 Trans::Transaction::Status::InProgress ||
					 cTrans_.getStatus() ==
					 Trans::Transaction::Status::Preparing ||
					 cTrans_.getStatus() ==
					 Trans::Transaction::Status::Committing ||
					 cTrans_.getStatus() ==
					 Trans::Transaction::Status::Rollbacking);

	if (cTrans_.getStatus() == Trans::Transaction::Status::Rollbacking) {

		// ロールバック中のときはロックする必要はない

		eReturnMode_ = Lock::Mode::N;
		eReturnDuration_ = Lock::Duration::Instant;

		return false;
	}

	// 適切なロックモードと持続期間を求める

	const _AdequateLock::_Info& info =
		_AdequateLock::infoForNoVersionTransaction
			[eOperation_][eManipulate_][cTrans_.getIsolationLevel()][eLocked_];
	eReturnMode_ = info._mode;
	eReturnDuration_ = info._duration;

	// 求めた適切なロックモードが N でなければ、ロックの必要がある

	return eReturnMode_ != Lock::Mode::N;
}

//	FUNCTION public
//	Schema::Hold::hold -- 適切なロックモードでロックする
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction& cTrans_
//		ロックをかけるトランザクション記述子
//	const Lock::Name& cLockName_
//		ロック名
//	Schema::Hold::Target::Value eTarget_
//		ロックする対象を表す値
//	Lock::Name::Category::Value eManipulate_
//		操作するオブジェクトのカテゴリー
//	Hold::Operation::Value eOperation_
//		操作対象に適用しようとしている操作
//	Lock::Timeout::Value iTimeout_ = Unlimited
//		ロック待ちタイムアウト
//
//	RETURN
//		true ... ロックできた
//		false... タイムアウトした
//
//	EXCEPTIONS

bool
Hold::
hold(Trans::Transaction& cTrans_,
	 const Lock::Name& cLockName_,
	 Hold::Target::Value eTarget_,
	 Lock::Name::Category::Value eManipulate_,
	 Hold::Operation::Value eOperation_,
	 Lock::Timeout::Value iTimeout_)
{
	Lock::Mode::Value eMode;
	Lock::Duration::Value eDuration;

	bool bResult = true;

	if (getAdequateLock(cTrans_, _Lock::_TargetTable[eTarget_],
						eManipulate_,
						eOperation_,
						eMode, eDuration)) {
#ifdef DEBUG
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Locking " << eTarget_ << " mode=" << eMode << " duration=" << eDuration
			<< "..."
			<< ModEndl;
#endif

		bResult = cTrans_.lock(cLockName_, eMode, eDuration, iTimeout_);

#ifdef DEBUG
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Locking " << eTarget_ << " mode=" << eMode << " duration=" << eDuration
			<< (bResult ? "...done" : "...failed")
			<< ModEndl;
#endif
	}
	return bResult;
}

//	FUNCTION public
//	Schema::Hold::convert -- 適切なロックモードでロックの変換を行う
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction& cTrans_
//		ロックをかけるトランザクション記述子
//	const Lock::Name& cLockName_
//		ロック名
//	Schema::Hold::Target::Value eTarget_
//		ロックする対象を表す値
//	Lock::Name::Category::Value eManipulateFrom_
//		変更前の操作するオブジェクトのカテゴリー
//	Hold::Operation::Value eOperationFrom_
//		変更前に読み込みのみを行おうとしているならtrue、
//		書き込みを行う可能性があるならfalse
//	Lock::Name::Category::Value eManipulateTo_
//		変更後の操作するオブジェクトのカテゴリー
//	Hold::Operation::Value eOperationTo_
//		操作対象に適用しようとしている操作
//	Lock::Timeout::Value iTimeout_ = Unlitimed
//		ロック待ちタイムアウト
//
//	RETURN
//		true ... ロックできた
//		false... タイムアウトした
//
//	EXCEPTIONS

bool
Hold::
convert(Trans::Transaction& cTrans_,
		const Lock::Name& cLockName_,
		Hold::Target::Value eTarget_,
		Lock::Name::Category::Value eManipulateFrom_,
		Hold::Operation::Value eOperationFrom_,
		Lock::Name::Category::Value eManipulateTo_,
		Hold::Operation::Value eOperationTo_,
		Lock::Timeout::Value iTimeout_)
{
	Lock::Mode::Value eModeFrom;
	Lock::Mode::Value eModeTo;
	Lock::Duration::Value eDurationFrom;
	Lock::Duration::Value eDurationTo;

	bool bResult = true;

	if (getAdequateLock(cTrans_, _Lock::_TargetTable[eTarget_],
							  eManipulateFrom_,
							  eOperationFrom_,
							  eModeFrom, eDurationFrom)) {
		if (getAdequateLock(cTrans_, _Lock::_TargetTable[eTarget_],
								  eManipulateTo_,
								  eOperationTo_,
								  eModeTo, eDurationTo)) {

			if (eModeFrom != eModeTo || eDurationFrom != eDurationTo) {
				// モードか持続期間が変化するときのみ変換を行う
#ifdef DEBUG
				SydSchemaParameterMessage(Message::ReportSystemTable)
					<< "Lock converting " << eTarget_
					<< " mode from=" << eModeFrom << "(" << eDurationFrom << ")"
					<< " to=" << eModeTo << "(" << eDurationTo << ")"
					<< " ..."
					<< ModEndl;
#endif

				bResult = cTrans_.convertLock(cLockName_, eModeFrom, eDurationFrom, eModeTo, eDurationTo, iTimeout_);

#ifdef DEBUG
				SydSchemaParameterMessage(Message::ReportSystemTable)
					<< "Lock converting " << eTarget_
					<< " mode from=" << eModeFrom << "(" << eDurationFrom << ")"
					<< " to=" << eModeTo << "(" << eDurationTo << ")"
					<< (bResult ? " ...done" : " ...failed")
					<< ModEndl;
#endif
			}
		}
	}
	return bResult;
}

//	FUNCTION public
//	Schema::Hold::release -- 必要ならアンロックする
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction& cTrans_
//		ロックをかけるトランザクション記述子
//	const Lock::Name& cLockName_
//		ロック名
//	Schema::Hold::Target::Value eTarget_
//		ロックする対象を表す値
//	Lock::Name::Category::Value eManipulate_
//		操作するオブジェクトのカテゴリー
//	Hold::Operation::Value eOperation_
//		操作対象に適用しようとしている操作
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Hold::
release(Trans::Transaction& cTrans_,
		const Lock::Name& cLockName_,
		Hold::Target::Value eTarget_,
		Lock::Name::Category::Value eManipulate_,
		Hold::Operation::Value eOperation_)
{
	Lock::Mode::Value eMode;
	Lock::Duration::Value eDuration;

	if (getAdequateLock(cTrans_, _Lock::_TargetTable[eTarget_],
						eManipulate_,
						eOperation_,
						eMode, eDuration)) {
#ifdef OBSOLETE
		/*
		 *【未実装】	持続期間が Statement のロックが実装されていないため
		 *				明示的に Statement のロックをはずしたいことがある
		 */
		// 持続期間がカーソルかユーザーのときに外す
		if (eDuration == Lock::Duration::Cursor
			|| eDuration == Lock::Duration::User) {
#endif
#ifdef DEBUG
			SydSchemaParameterMessage(Message::ReportSystemTable)
				<< "Unlocking " << eTarget_ << " mode=" << eMode << " duration=" << eDuration
				<< "..."
				<< ModEndl;
#endif

			cTrans_.unlock(cLockName_, eMode, eDuration);

#ifdef DEBUG
			SydSchemaParameterMessage(Message::ReportSystemTable)
				<< "Unlocking " << eTarget_ << " mode=" << eMode << " duration=" << eDuration
				<< "...done"
				<< ModEndl;
#endif
		}
#ifdef OBSOLETE
	}
#endif
}

//
// Copyright (c) 2001, 2002, 2005, 2007, 2008, 2011, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
