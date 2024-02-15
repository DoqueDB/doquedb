// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Mode.cpp -- ロックモード関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2011, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Lock";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Lock/Mode.h"

#include "ModMessage.h"
#include "ModOstream.h"

_SYDNEY_USING
_SYDNEY_LOCK_USING

namespace {

// 2 つのロックモード値の最小上界を求めるための配列

#define VIS		Mode::VIS
#define	VS		Mode::VS
#define	IS		Mode::IS
#define VSIS	Mode::VSIS
#define	IX		Mode::IX
#define	S		Mode::S
#define VSIX	Mode::VSIX
#define VIX		Mode::VIX
#define VSVIX	Mode::VSVIX
#define	SIX		Mode::SIX
#define	U		Mode::U
#define SVIX	Mode::SVIX
#define	X		Mode::X
#define VIXX	Mode::VIXX
#define	VX		Mode::VX
#define	N		Mode::N

const Mode::Value	leastUpperBound[Mode::ValueNum][Mode::ValueNum] =
{
// 既存
//	VIS		VS		IS		VSIS	IX		S		VSIX	VIX		VSVIX	SIX		U		SVIX	X		VIXX	VX		N

	VIS,	VS,		IS,		VSIS,	IX,		S,		VSIX,	VIX,	VSVIX,	SIX,	U,		SVIX,	X,		VIXX,	VX,		VIS,	// VIS    要
	VS,		VS,		VSIS,	VSIS,	VSIX,	S,		VSIX,	VSVIX,	VSVIX,	SIX,	U,		SVIX,	X,		VIXX,	VX,		VS,		// VS     求
	IS,		VSIS,	IS,		VSIS,	IX,		S,		VSIX,	VIX,	VSVIX,	SIX,	U,		SVIX,	X,		VIXX,	VX,		IS,		// IS
	VSIS,	VSIS,	VSIS,	VSIS,	VSIX,	S,		VSIX,	VSVIX,	VSVIX,	SIX,	U,		SVIX,	X,		VIXX,	VX,		VSIS,	// VSIS
	IX,		VSIX,	IX,		VSIX,	IX,		SIX,	VSIX,	VIX,	VSVIX,	SIX,	SIX,	SVIX,	X,		VIXX,	VX,		IX,		// IX
	S,		S,		S,		S,		SIX,	S,		SIX,	SVIX,	SVIX,	SIX,	U,		SVIX,	X,		VIXX,	VX,		S,		// S
	VSIX,	VSIX,	VSIX,	VSIX,	VSIX,	SIX,	VSIX,	VSVIX,	VSVIX,	SIX,	SIX,	SVIX,	X,		VIXX,	VX,		VSIX,	// VSIX
	VIX,	VSVIX,	VIX,	VSVIX,	VIX,	SVIX,	VSVIX,	VIX,	VSVIX,	SVIX,	SVIX,	SVIX,	VIXX,	VIXX,	VX,		VIX,	// VIX
	VSVIX,	VSVIX,	VSVIX,	VSVIX,	VSVIX,	SVIX,	VSVIX,	VSVIX,	VSVIX,	SVIX,	SVIX,	SVIX,	VIXX,	VIXX,	VX,		VSVIX,	// VSVIX
	SIX,	SIX,	SIX,	SIX,	SIX,	SIX,	SIX,	SVIX,	SVIX,	SIX,	SIX,	SVIX,	X,		VIXX,	VX,		SIX,	// SIX
	U,		U,		U,		U,		SIX,	U,		SIX,	SVIX,	SVIX,	SIX,	U,		SVIX,	X,		VIXX,	VX,		U,		// U
	SVIX,	SVIX,	SVIX,	SVIX,	SVIX,	SVIX,	SVIX,	SVIX,	SVIX,	SVIX,	SVIX,	SVIX,	VIXX,	VIXX,	VX,		SVIX,	// SVIX
	X,		X,		X,		X,		X,		X,		X,		VIXX,	VIXX,	X,		X,		VIXX,	X,		VIXX,	VX,		X,		// X
	VIXX,	VIXX,	VIXX,	VIXX,	VIXX,	VIXX,	VIXX,	VIXX,	VIXX,	VIXX,	VIXX,	VIXX,	VIXX,	VIXX,	VX,		VIXX,	// VIXX
	VX,		VX,		VX,		VX,		VX,		VX,		VX,		VX,		VX,		VX,		VX,		VX,		VX,		VX,		VX,		VX,		// VX
	VIS,	VS,		IS,		VSIS,	IX,		S,		VSIX,	VIX,	VSVIX,	SIX,	U,		SVIX,	X,		VIXX,	VX,		N		// N
};

#undef	VIS
#undef	VS
#undef	IS
#undef	VSIS
#undef	IX
#undef	S
#undef	VSIX
#undef	VIX
#undef	VSVIX
#undef	SIX
#undef	U
#undef	SVIX
#undef	X
#undef	VIXX
#undef	VX
#undef	N

// 2 つのロックモード値が両立するかを調べるための配列

const bool	compatibility[Mode::ValueNum][Mode::ValueNum] =
{
// 要求 
//	VIS		VS		IS		VSIS	IX		S		VSIX	VIX		VSVIX	SIX		U		SVIX	X		VIXX	VX		N

	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	false,	true,	// VIS   既
	true,	true,	true,	true,	true,	true,	true,	false,	false,	true,	true,	false,	true,	false,	false,	true,	// VS    存
	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	false,	false,	false,	true,	// IS
	true,	true,	true,	true,	true,	true,	true,	false,	false,	true,	true,	false,	false,	false,	false,	true,	// VSIS
	true,	true,	true,	true,	true,	false,	true,	true,	true,	false,	false,	false,	false,	false,	false,	true,	// IX
	true,	true,	true,	true,	false,	true,	false,	false,	false,	false,	true,	false,	false,	false,	false,	true,	// S
	true,	true,	true,	true,	true,	false,	true,	false,	false,	false,	false,	false,	false,	false,	false,	true,	// VSIX
	true,	false,	true,	false,	true,	false,	false,	true,	false,	false,	false,	false,	false,	false,	false,	true,	// VIX
	true,	false,	true,	false,	true,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true,	// VSVIX
	true,	true,	true,	true,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true,	// SIX
	true,	true,	true,	true,	false,	true,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true,	// U
	true,	false,	true,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true,	// SVIX
	true,	true,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true,	// X
	true,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true,	// VIXX
	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true,	// VX
	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true	// N
};

// 親のロックモード値に対して、子のロックモード値が可能か調べるための配列

#define	IM		Mode::Possibility::Impossible	// ×
#define	PO		Mode::Possibility::Possible		// △
#define	UN		Mode::Possibility::Unnecessary	// ○

const Mode::Possibility::Value	possibility[Mode::ValueNum][Mode::ValueNum] =
{
//		下	位
//	VIS       VS        IS        VSIS      IX        S         VSIX      VIX       VSVIX     SIX       U         SVIX      X         VIXX      VX        N

	UN/*○*/, PO/*△*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/,	// VIS
	UN/*○*/, UN/*○*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/,	// VS
	UN/*○*/, PO/*△*/, UN/*○*/, PO/*△*/, IM/*×*/, PO/*△*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, PO/*△*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/,	// IS
	UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, IM/*×*/, PO/*△*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, PO/*△*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/,	// VSIS
	UN/*○*/, PO/*△*/, UN/*○*/, PO/*△*/, UN/*○*/, PO/*△*/, PO/*△*/, PO/*△*/, PO/*△*/, PO/*△*/, PO/*△*/, PO/*△*/, PO/*△*/, PO/*△*/, PO/*△*/, IM/*×*/,	// IX
	UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, IM/*×*/, UN/*○*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, UN/*○*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/,	// S		上
	UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, PO/*△*/, UN/*○*/, PO/*△*/, PO/*△*/, PO/*△*/, PO/*△*/, PO/*△*/, PO/*△*/, PO/*△*/, PO/*△*/, IM/*×*/,	// VSIX
	UN/*○*/, PO/*△*/, UN/*○*/, PO/*△*/, UN/*○*/, PO/*△*/, PO/*△*/, UN/*○*/, PO/*△*/, PO/*△*/, PO/*△*/, PO/*△*/, PO/*△*/, PO/*△*/, PO/*△*/, IM/*×*/,	// VIX
	UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, PO/*△*/, UN/*○*/, UN/*○*/, UN/*○*/, PO/*△*/, PO/*△*/, PO/*△*/, PO/*△*/, PO/*△*/, PO/*△*/, IM/*×*/,	// VSVIX
	UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, PO/*△*/, PO/*△*/, UN/*○*/, UN/*○*/, PO/*△*/, PO/*△*/, PO/*△*/, PO/*△*/, IM/*×*/,	// SIX
	UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, IM/*×*/, UN/*○*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, UN/*○*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/,	// U		位
	UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, PO/*△*/, PO/*△*/, PO/*△*/, IM/*×*/,	// SVIX
	UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, PO/*△*/, PO/*△*/, UN/*○*/, UN/*○*/, PO/*△*/, UN/*○*/, PO/*△*/, PO/*△*/, IM/*×*/,	// X
	UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, PO/*△*/, IM/*×*/,	// VIXX
	UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, UN/*○*/, IM/*×*/,	// VX
	IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/, IM/*×*/	// N
};

#undef	IM
#undef	PO
#undef	UN

} // end of global namespace

//	FUNCTION public
//	Lock::Mode::getLeastUpperBound --
//		あるロック対象のロックモードと
//		要求されたロックモードとの最小上界となるロックモードを得る
//
//	NOTES
//		すでにロックされているロック対象を、
//		あるモードでさらにロックしたときに、そのロック対象は、
//		結果的にどういうモードでロックされるかを得るために使用する
//
//	ARGUMENTS
//		Lock::Mode::Value	granted
//			調べるロック対象の現在のロックモード値
//		Lock::Mode::Value	requested
//			調べるロック対象に要求されたロックのロックモード値
//
//	RETURN
//		現在のロックモード値と要求されたロックモード値の最小上界
//
//	EXCEPTIONS
//		なし

// static
Mode::Value
Mode::getLeastUpperBound(Mode::Value	granted,
						 Mode::Value	requested)
{
	return leastUpperBound[granted][requested];
}

//	FUNCTION public
//	Lock::Mode::isCompatible --
//		あるロック対象のロックモードと
//		要求されたロックモードの両立性があるか調べる
//
//	NOTES
//		すでにロックされているロック対象を
//		あるモードでロック可能か調べるために使用する
//
//	ARGUMENTS
//		Lock::Mode::Value		granted
//			調べるロック対象の現在のロックモード値
//		Lock::Mode::Value		requested
//			調べるロック対象に要求されたロックのロックモード値
//
//	RETURN
//		true
//			要求されたモードは現在のロックと両立する
//		false
//			両立しない
//
//	EXCEPTIONS
//		なし

// static
bool
Mode::isCompatible(Mode::Value	granted,
				   Mode::Value	requested)
{
	return compatibility[granted][requested];
}

//	FUNCTION public
//	Lock::Mode::isPossible --
//		あるロック対象があるモードでロックされているとき、
//		その子を指定されたモードでロック可能か調べる
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Mode::Value		parent
//			あるロック対象の現在のロックのロックモード値
//		Lock::Mode::Value		child
//			子に要求するロックのロックモード値
//
//	RETURN
//		Lock::Mode::Impossible
//			子に対して要求されたモードでロックできない
//		Lock::Mode::Possible
//			子に対して要求されたモードでロック可能であり、必要である
//		Lock::Mode::Unnecessary
//			子に対して要求されたモードでロック可能であるが、必要ない
//
//	EXCEPTIONS
//		なし

// static
Mode::Possibility::Value
Mode::isPossible(Mode::Value	parent,
				 Mode::Value	child)
{
	return possibility[parent][child];
}

namespace
{
	const char* const _pszModeName[Lock::Mode::ValueNum+1] =
	{
		"VIS",
		"VS",
		"IS",
		"VSIS",
		"IX",
		"S",
		"VSIX",
		"VIX",
		"VSVIX",
		"SIX",
		"U",
		"SVIX",
		"X",
		"VIXX",
		"VX",
		"N",
		"--",
	};
}

// ロックモードをメッセージに出力する
ModMessageStream& operator<<(ModMessageStream& cStream_, Lock::Mode::Value eValue_)
{
	return cStream_ << _pszModeName[eValue_];
}

ModOstream& operator<<(ModOstream& cStream_, Lock::Mode::Value eValue_)
{
	return cStream_ << _pszModeName[eValue_];
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
