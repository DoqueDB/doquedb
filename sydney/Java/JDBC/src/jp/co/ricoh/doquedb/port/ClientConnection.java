// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ClientConnection.java -- DoqueDBとのクライアントコネクションの基底クラス
//
// Copyright (c) 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
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

import jp.co.ricoh.doquedb.common.*;

/**
 * DoqueDBとのクライアントコネクションの基底クラス。
 * すべてのクライアントコネクションクラスはこのクラスを継承する必要がある。
 *
 */
public abstract class ClientConnection extends Connection
{
	/**
	 * DoqueDBとのコネクションクラスを新しく作成する。
	 * このクラスは抽象クラスなので、直接このインスタンスが
	 * 確保されることはない。
	 *
	 * @param connectionType_	{@link ConnectionType コネクションタイプ}
	 * @param masterID_			マスターID
	 * @param slaveID_			スレーブID
	 */
	public ClientConnection(int connectionType_, int masterID_, int slaveID_)
	{
		super(connectionType_, masterID_, slaveID_);
	}

	/**
	 * サーバとの同期を取る
	 *
	 * @throws	java.io.IOException
	 *			通信関係のエラー
	 * @throws	java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 */
	public void sync()
		throws java.io.IOException, ClassNotFoundException
	{
		/* syncは遅いのでやめた
		//クライアント側なのでsyncのリクエストを受け取る
		while (true)
		{
			Serializable object = readObject();
			if (object instanceof Request)
			{
				Request request = (Request)object;
				if (request.getRequest() == Request.SYNC)
					break;
			}
		}
		//サーバにsyncのリクエストを送る
		writeObject(new Request(Request.SYNC));
		flush();*/
	}
}

//
// Copyright (c) 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
