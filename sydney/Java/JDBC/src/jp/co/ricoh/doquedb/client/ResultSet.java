// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ResultSet.java -- 結果集合クラス
//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2016, 2023 Ricoh Company, Ltd.
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
import jp.co.ricoh.doquedb.common.DataArrayData;
import jp.co.ricoh.doquedb.common.ResultSetMetaData;
import jp.co.ricoh.doquedb.common.Serializable;
import jp.co.ricoh.doquedb.common.Status;

/**
 * 結果集合クラス
 *
 */
public final class ResultSet extends Object
{
	/** データソース */
	private DataSource _dataSource;
	/** 通信ポート */
	private Port _port;
	/** 結果集合メタデータ */
	private ResultSetMetaData _metaData;
	/** 一行のデータ */
	private DataArrayData _tupleData;

	/** 不明なステータス */
	public final static int UNDEFINED		= 0;
	/** データである */
	public final static int DATA			= 1;
	/** データ終了である */
	public final static int END_OF_DATA		= 2;
	/** 正常終了 */
	public final static int SUCCESS			= 3;
	/** キャンセルされた */
	public final static int CANCELED		= 4;
	/** エラーが発生した */
	public final static int ERROR			= 5;
	/** 結果集合メタデータ */
	public final static int META_DATA		= 6;
	/** 続きのデータがある */
	public final static int HAS_MORE_DATA	= 7;

	/** ステータス */
	private int _status;

	/**
	 * 新しく結果集合オブジェクトを作成する。
	 *
	 * @param dataSource_	データソース
	 * @param port_			通信ポート
	 */
	protected ResultSet(DataSource dataSource_,
						Port port_)
	{
		super(Object.RESULT_SET);
		_dataSource = dataSource_;
		_port = port_;
		_status = DATA;
		_metaData = null;
		_tupleData = null;
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
				switch (_status)
				{
				case DATA:
				case META_DATA:
				case HAS_MORE_DATA:
				case END_OF_DATA:
					//実行ステータスを得ていないので、得る
					getStatus(true /* skip until success or canceled is obtained */);
					break;
				}
			}
			catch (Exception e) {}	//例外は無視する
		}
	}

	/**
	 * 実行ステータスを得る。
	 * 実行ステータスのみを返す。サーバからデータを受け取っても読み捨てる。
	 *
	 * @param	skipAll_	trueのとき複文のすべての結果を読み捨てる
	 *						falseのとき複文なら1文の結果のみ読み捨てる
	 * @return 	実行ステータス
	 * @throws	java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws	java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	public int getStatus(boolean skipAll_)
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException
	{
		while (_status == META_DATA
			   || _status == DATA
			   || _status == END_OF_DATA
			   || (skipAll_ && _status == HAS_MORE_DATA))
		{
			getNextTuple(null);
		}

		return _status;
	}

	/**
	 * 実行ステータスを得る。
	 * 引数なし版(互換性のため)。
	 *
	 * @return 	実行ステータス
	 * @throws	java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws	java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	public int getStatus()
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException
	{
		return getStatus(true);
	}

	/**
	 * 最後のResultSetステータスを得る
	 *
	 * @return ResultSetステータス
	 */
	public int getLastStatus()
	{
		return _status;
	}

	/**
	 * 次のタプルデータを読む。
	 *
	 * @param tuple_	読み込んだタプルデータ。
	 *					呼び出し側は空のDataArrayDataを設定する必要がある。
	 * @return 	実行ステータス
	 * @throws	java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws	java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	public int getNextTuple(DataArrayData tuple_)
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException
	{
		if (_port == null) return _status;

		int status = UNDEFINED;
		Serializable object;

		if (_tupleData != null)
		{
			// 中身をassignする
			if (tuple_ != null) tuple_.assign(_tupleData);
		}
		else
		{
			//中身を解放する
			if (tuple_ != null) tuple_.clear();
		}

		try
		{
			try
			{
				//通信ポートから1つ読み込む
				object = _port.readObject(tuple_);
			}
			catch (java.io.IOException e)
			{
				_status = ERROR;
				throw e;
			}
			catch (java.lang.ClassNotFoundException e)
			{
				_status = ERROR;
				throw e;
			}
			catch (java.sql.SQLException e)
			{
				_status = ERROR;
				throw e;
			}

			if (object == null)
			{
				//データ終了
				status = END_OF_DATA;
				_metaData = null;
				_tupleData = null;
			}
			else if (object.getClassID() == ClassID.RESULTSET_META_DATA)
			{
				// メタデータ
				status = META_DATA;
				_metaData = (ResultSetMetaData)object;
				_tupleData = _metaData.createTupleData();
			}
			else if (object.getClassID() == ClassID.STATUS)
			{
				Status s = (Status)object;
				//実行ステータス
				switch (s.getStatus())
				{
				case Status.SUCCESS:
					status = SUCCESS;
					break;
				case Status.CANCELED:
					status = CANCELED;
					break;
				case Status.HAS_MORE_DATA:
					status = HAS_MORE_DATA;
					break;
				}
			}
			else if (object.getClassID() == ClassID.DATA_ARRAY_DATA)
			{
				//タプルデータ
				status = DATA;
			}

			if (status == UNDEFINED)
			{
				_status = ERROR;
				//予期せぬエラー
				throw new jp.co.ricoh.doquedb.exception.Unexpected();
			}

			//現在の状態をセーブ
			_status = status;
		}
		finally
		{
			switch (_status) {
			case DATA:
			case END_OF_DATA:
			case META_DATA:
			case HAS_MORE_DATA:
				{
					break;
				}
			case SUCCESS:
				{
					_dataSource.pushPort(_port);
					_port = null;
					break;
				}
			case CANCELED:
				{
					if (_port.getMasterID() >= jp.co.ricoh.doquedb.jdbc.Driver.PROTOCOL_VERSION3)
					{
						_dataSource.pushPort(_port);
						_port = null;
						break;
					}
					// thru.
				}
			case ERROR:
			case UNDEFINED:
			default:
				{
					if (_port.isReuse() == true)
						_dataSource.pushPort(_port);
					else
						_port.close();
					_port = null;
					break;
				}
			}
		}

		return status;
	}

	/**
	 * 実行をキャンセルする。
	 *
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	public void cancel()
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException
	{
		//クライアントコネクションを得る
		Connection clientConnection = _dataSource.getClientConnection();

		//中断を要求する
		clientConnection.cancelWorker(_port.getWorkerID());
	}

	/**
	 * ResultSetMetaDataを得る
	 */
	public ResultSetMetaData getMetaData()
	{
		return _metaData;
	}

	/**
	 * ※ <code>jdbc.ResultSet</code> の
	 * <code>finalize</code> メソッドから呼び出されます。
	 *
	 * @throws	java.lang.Throwable
	 *			このメソッドで生じた例外。
	 */
	public void finalize() throws java.lang.Throwable
	{
		if (_port != null) {
			_dataSource.expungePort(_port);
		}
	}

}

//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
