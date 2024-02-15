// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UserID.java -- createUserで指定するUserIDに関する定数定義
//
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.port;

/**
 * 認証方式
 *
 */
public final class UserID
{
	/** 自動割当 */
	public final static int AUTO			= -1;

	/** 最小値 */
	public final static int MIN				= 1;

	/** 最大値 */
	public final static int MAX				= 0x7fffffff;
}

//
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
