// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ConnectionSlaveID.java -- スレーブIDを管理するクラス
//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
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
 * スレーブIDを管理するクラス
 *
 */
public final class ConnectionSlaveID
{
	/** スレーブIDの最小値 */
	public final static int MINIMUM		= 0;
	/** スレーブIDの最大値 */
	public final static int MAXIMUM		= 0x7fffffff;
	/** 任意のスレーブIDをあらわす数 */
	public final static int ANY			= 0x80000000;
	/** 未定義をあらわす数 */
	public final static int UNDEFINED	= 0xffffffff;

	/**
	 * スレーブIDが正しいものかどうかチェックする
	 *
	 * @return	正しいものの場合はtrue、それ以外の場合はfalse
	 */
	public static boolean isNormal(int slaveID_)
	{
		return slaveID_ >= MINIMUM && slaveID_ < MAXIMUM;
	}
}

//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
