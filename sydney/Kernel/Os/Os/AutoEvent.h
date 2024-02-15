// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoEvent.h --
//		自動イベント関連のクラス定義、関数宣言
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

#ifndef	__TRMEISTER_OS_AUTOEVENT_H
#define	__TRMEISTER_OS_AUTOEVENT_H

#include "Os/Module.h"
#include "Os/Event.h"

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

//	CLASS
//	Os::AutoEvent --
//		オブジェクト生成時に自動的にイベントをシグナルかまたは非シグナル化し、
//		破棄時に自動的にイベントを非シグナルかまたはシグナル化するクラス
//
//	NOTES

class AutoEvent
{
public:
	// コンストラクター
	AutoEvent(Event& event, bool signaled = true);
	// デストラクター
	~AutoEvent();

private:
	// 操作するイベント
	Event&					_event;
	// イベントの状態
	bool					_signaled;
};

//	CLASS
//	Os::AutoEvent::AutoEvent --
//		オブジェクト生成時に自動的にイベントをシグナルかまたは非シグナル化し、
//		破棄時に自動的にイベントを非シグナルかまたはシグナル化するクラスの
//		コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Os::Event&			event
//			操作するイベントのリファレンス
//		bool				signaled
//			true または指定されないとき
//				イベントをシグナル化する
//			false
//				イベントを非シグナル化する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
AutoEvent::AutoEvent(Event& event, bool signaled)
	: _event(event),
	  _signaled(signaled)
{
	(_signaled) ? _event.set() : _event.reset();
}

//	CLASS
//	Os::AutoEvent::~AutoEvent --
//		オブジェクト生成時に自動的にイベントをシグナルかまたは非シグナル化し、
//		破棄時に自動的にイベントを非シグナルかまたはシグナル化するクラスの
//		デストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
AutoEvent::~AutoEvent()
{
	(_signaled) ? _event.reset() : _event.set();
}

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_AUTOEVENT_H

//
// Copyright (c) 2001, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
