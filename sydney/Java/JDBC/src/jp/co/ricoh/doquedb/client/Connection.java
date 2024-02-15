// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Connection.java -- クライアントコネクション
//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.client;

import jp.co.ricoh.doquedb.common.ClassID;
import jp.co.ricoh.doquedb.common.IntegerArrayData;
import jp.co.ricoh.doquedb.common.IntegerData;
import jp.co.ricoh.doquedb.common.Request;
import jp.co.ricoh.doquedb.common.StringData;

/**
 * クライアントコネクションクラス
 * サーバとのコネクションを管理する。サーバ側にはこのクライアントコネクション
 * １つごとにスレッドが存在している。
 *
 */
final class Connection extends Object
{
	/** 自分の属するデータソース */
	private DataSource _dataSource;
	/** サーバ側のコネクションスレッドとの通信ポート */
	private Port _port;

	/**
	 * 新しくコネクションオブジェクトを作成する
	 *
	 * @param dataSource_	属するデータソースオブジェクト
	 * @param port_			サーバ側のコネクションスレッドとの通信ポート
	 */
	protected Connection(DataSource dataSource_, Port port_)
	{
		super(Object.CONNECTION);
		_dataSource = dataSource_;
		_port = port_;
	}

	/**
	 * クローズする。
	 *
	 * @throws	java.io.IOException
	 *			通信関係のエラー
	 */
	public void close()
		throws java.io.IOException
	{
		if (_port != null)
		{
			try
			{
				//[<-] リクエスト
				_port.writeObject(new Request(Request.END_CONNECTION));
				_port.flush();
				//[->] ステータス
				_port.readStatus();
			}
			catch (Exception e)	{}	// 例外を無視する

			try
			{
				//ポートをクローズする
				_port.close();
			}
			catch (Exception e) {} // 例外を無視する

			_port = null;
		}
	}

	/**
	 * ワーカを起動する。
	 *
	 * @return	ワーカとの通信ポート
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	protected Port beginWorker()
		throws java.io.IOException, ClassNotFoundException,
				java.sql.SQLException
	{
		//ポートプールからポートを得る
		Port port = _dataSource.popPort();

		int slaveID = jp.co.ricoh.doquedb.port.ConnectionSlaveID.ANY;
		IntegerData slaveData;
		IntegerData workerData;

		try
		{
			if (port != null) slaveID = port.getSlaveID();

			synchronized(_port)
			{
				//[<-] リクエスト
				_port.writeObject(new Request(Request.BEGIN_WORKER));
				//[<-] スレーブID
				_port.writeObject(new IntegerData(slaveID));
				_port.flush();

				//[->] スレーブID
				slaveData = _port.readIntegerData();
				//[->] ワーカID
				workerData = _port.readIntegerData();
				//[->] ステータス
				_port.readStatus();
			}
		}
		catch (java.io.IOException e)
		{
			if (port != null) _dataSource.pushPort(port);
			throw e;
		}
		catch (ClassNotFoundException e)
		{
			if (port != null) _dataSource.pushPort(port);
			throw e;
		}
		catch (java.sql.SQLException e)
		{
			if (port != null) _dataSource.pushPort(port);
			throw e;
		}
		if (slaveID == jp.co.ricoh.doquedb.port.ConnectionSlaveID.ANY)
		{
			//新しい通信ポート
			port = _dataSource.getNewPort(slaveData.getValue());
			port.open();
		}
		else
		{
			//プールされている通信ポート -> 同期を取る
			port.sync();
		}

		//ワーカIDを設定
		port.setWorkerID(workerData.getValue());

		return port;
	}

	/**
	 * ワーカをキャンセルする
	 *
	 * @param workerID_	キャンセルするワーカのID
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	protected void cancelWorker(int workerID_)
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException
	{
		synchronized(_port)
		{
			//[<-] リクエスト
			_port.writeObject(new Request(Request.CANCEL_WORKER));
			//[<-] ワーカID
			_port.writeObject(new IntegerData(workerID_));
			_port.flush();

			//[->] ステータス
			_port.readStatus();
		}
	}

	/**
	 * プリペアステートメントを削除する。
	 *
	 * @param databaseName_	データベース名
	 * @param prepareID_	プリペアステートメントID
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	protected void erasePrepareStatement(String databaseName_,
							   int prepareID_)
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException
	{
		synchronized(_port)
		{
			//[<-] リクエスト
			_port.writeObject(new Request(Request.ERASE_PREPARE_STATEMENT));
			//[<-] データベース名
			_port.writeObject(new StringData(databaseName_));
			//[<-] プリペアステートメントID
			_port.writeObject(new IntegerData(prepareID_));
			_port.flush();

			//[->] ステータス
			_port.readStatus();
		}
	}

	/**
	 * 使用しない通信ポートを切断する。
	 *
	 * @param slaveID_	切断する通信ポートのスレーブIDの配列
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	protected void disconnectPort(java.util.Vector slaveID_)
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException
	{
		synchronized(_port)
		{
			//[<-] リクエスト
			_port.writeObject(new Request(Request.NO_REUSE_CONNECTION));
			//[<-] スレーブIDの配列
			_port.writeObject(new IntegerArrayData(slaveID_));
			_port.flush();

			//[->] ステータス
			_port.readStatus();
		}
	}

	/**
	 * 新しいコネクションを得る。
	 *
	 * @return 新しいコネクション
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	protected Connection beginConnection()
		throws java.io.IOException, ClassNotFoundException,
				java.sql.SQLException
	{
		synchronized(_port)
		{
			//[<-] リクエスト
			_port.writeObject(new Request(Request.BEGIN_CONNECTION));
			_port.flush();

			//[->] スレーブIDを受け取る
			IntegerData slaveData = _port.readIntegerData();

			//新しい通信ポートを得る
			Port port = _dataSource.getNewPort(slaveData.getValue());
			port.open();

			//[->] ステータス
			_port.readStatus();

			//[->] 新しい通信ポートからステータスを得る
			port.readStatus();

			return new Connection(_dataSource, port);
		}
	}

	/**
	 * サーバの利用可能性を得る。
	 *
	 * @return	サーバが利用可能な場合は true 、利用不可能な場合は false
	 */
	protected boolean isServerAvailable()
		throws java.io.IOException, java.lang.ClassNotFoundException, java.sql.SQLException
	{
		synchronized(_port)
		{
			// [<-] リクエスト
			_port.writeObject(new Request(Request.CHECK_AVAILABILITY));
			// [<-] チェック対象
			_port.writeObject(new IntegerData(Request.AVAILABILITY_TARGET_SERVER));
			_port.flush();

			// [->] チェック結果
			IntegerData	result = _port.readIntegerData();
			// [->] ステータス
			_port.readStatus();

			return result.getValue() == 1;
		}
	}

	/**
	 * データベースの利用可能性を得る。
	 *
	 * @param	id_	データベース ID
	 * @return	データベースが利用可能な場合は true 、利用不可能な場合は false
	 */
	protected boolean isDatabaseAvailable(int	id_)
		throws java.io.IOException, java.lang.ClassNotFoundException, java.sql.SQLException
	{
		synchronized(_port)
		{
			// [<-] リクエスト
			_port.writeObject(new Request(Request.CHECK_AVAILABILITY));
			// [<-] チェック対象
			_port.writeObject(new IntegerData(Request.AVAILABILITY_TARGET_DATABASE));
			// [<-] データベース ID
			_port.writeObject(new IntegerData(id_));
			_port.flush();

			// [->] チェック結果
			IntegerData	result = _port.readIntegerData();
			// [->] ステータス
			_port.readStatus();

			return result.getValue() == 1;
		}
	}
}

//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
