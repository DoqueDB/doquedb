// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// copy.java -- テーブルの内容をコピーする
//
// Copyright (c) 2009, 2012, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.utility;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.sql.Statement;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * SQL文を実行するクライアントコマンド
 *
 */
public class copy
{
	/**
	* オプション
	*/
	private static class Option
	{
		// ソース側

		// ユーザ名
		public String _srcUser = null;
		// パスワード
		public String _srcPassword = null;
		// URL
		public String _srcUrl = null;
		// テーブル名
		public String _srcTable = null;

		// ディスト側

		// ユーザ名
		public String _dstUser = null;
		// パスワード
		public String _dstPassword = null;
		// URL
		public String _dstUrl = null;
		// テーブル名
		public String _dstTable = null;

		public boolean _verbose = false;

		// limit
		public int _limit = -1;
		// offset
		public int _offset = -1;

		//コンストラクタ
		public Option()	{}

		//引数を解析する
		public boolean set(String[] args)
		{
			int i = 0;
			while (i < args.length)
			{
				if (args[i].equals("-srcurl"))
				{
					i++;
					if (i >= args.length) return false;
					_srcUrl = args[i++];
				}
				else if (args[i].equals("-dsturl"))
				{
					i++;
					if (i >= args.length) return false;
					_dstUrl = args[i++];
				}
				else if (args[i].equals("-srctable"))
				{
					i++;
					if (i >= args.length) return false;
					_srcTable = args[i++];
				}
				else if (args[i].equals("-dsttable"))
				{
					i++;
					if (i >= args.length) return false;
					_dstTable = args[i++];
				}
				else if (args[i].equals("-srcuser"))
				{
					i++;
					if (i >= args.length) return false;
					_srcUser = args[i++];
				}
				else if (args[i].equals("-dstuser"))
				{
					i++;
					if (i >= args.length) return false;
					_dstUser = args[i++];
				}
				else if (args[i].equals("-srcpassword"))
				{
					i++;
					if (i >= args.length) return false;
					_srcPassword = args[i++];
				}
				else if (args[i].equals("-dstpassword"))
				{
					i++;
					if (i >= args.length) return false;
					_dstPassword = args[i++];
				}
				else if (args[i].equals("-verbose"))
				{
					i++;
					_verbose = true;
				}
				else if (args[i].equals("-count") ||
						 args[i].equals("-limit"))
				{
					i++;
					if (i >= args.length) return false;
					_limit = Integer.parseInt(args[i++]);
				}
				else if (args[i].equals("-offset"))
				{
					i++;
					if (i >= args.length) return false;
					_offset = Integer.parseInt(args[i++]);
				}
				else
				{
					System.err.println("Unknown option: '" + args[i] + "'");
					return false;
				}
			}

			// 引数チェック
			if (_srcUrl == null ||
				_srcTable == null ||
				_dstTable == null)
				return false;

			if (_dstUrl == null)
			{
				_dstUrl = _srcUrl;
				if (_dstUser == null)
					_dstUser = _srcUser;
				if (_dstPassword == null)
					_dstPassword = _dstPassword;
			}

			return true;
		}
	}

	//USAGE
	private static void USAGE()
	{
		System.out.println("");
		System.out.println("Usage: import -srcurl url -srctable table_name");
		System.out.println("             [-srcuser user_name] [-srcpassword password]");
		System.out.println("             [-dsturl url] -dsttable table_name");
		System.out.println("             [-dstuser user_name] [-dstpassword password]");
		System.out.println("             [-verbose]");
		System.out.println("             [-limit count [-offset offset]]");
		System.out.println("");
	}

	/**
	 * メイン
	 */
	public static void main(String[] args)
		throws SQLException, ClassNotFoundException
	{
		Option op = new Option();
		if (op.set(args) == false)
		{
			USAGE();
			return;
		}

		Connection sc = null;
		Statement ss = null;
		ResultSet sr = null;

		Connection dc = null;
		PreparedStatement dp = null;

		try
		{
			// JDBCドライバーのロード
			Class.forName("jp.co.ricoh.doquedb.jdbc.Driver");

			// Connection を得る
			sc = DriverManager.getConnection(op._srcUrl,
											 op._srcUser,
											 op._srcPassword);
			dc = DriverManager.getConnection(op._dstUrl,
											 op._dstUser,
											 op._dstPassword);

			// ソーステープルを select する
			ss = sc.createStatement();
			int count = 0;
			int pout = 0;
			SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss.SSS");
			if (op._verbose == true)
			{
				if (op._limit == -1)
				{
					// 冗長メッセージを表示するので、件数を取得する
					sr = ss.executeQuery("select count(*) from " + op._srcTable);
					sr.next();
					count = sr.getInt(1);
					sr.close();
				}
				else
				{
					// 指定されたカウントを利用する
					count = op._limit;
				}

				System.out.println("TABLE COUNT: " + count);
				System.out.println("START: " + dateFormat.format(new Date()));

				pout = count / 100;
			}

			String sql = "select * from " + op._srcTable;
			if (op._limit != -1)
			{
				sql = sql + " limit " + Integer.toString(op._limit);
			}
			if (op._offset != -1)
			{
				sql = sql + " offset " + Integer.toString(op._offset);
			}
			sr = ss.executeQuery(sql);

			ResultSetMetaData meta = null;
			int n = 0;
			int p = 0;

			while (sr.next())
			{
				if (meta == null)
				{
					// ソースのResultSetMetaDataから
					// ディストのPreparedStatementを得る

					meta = sr.getMetaData();
					dp = createPreparedStatement(dc, meta, op._dstTable);
					n = meta.getColumnCount();
				}

				for (int i = 1; i <= n; ++i)
				{
					// ソースのデータをディストへ設定する
					dp.setObject(i, sr.getObject(i));
				}

				// 挿入を発行

				dp.executeUpdate();

				++p;

				if (op._verbose == true && pout != 0)
				{
					if ((p % pout) == 0)
					{
						System.out.println(Integer.toString(p / pout) + "%: " + dateFormat.format(new Date()));
					}
				}
			}

			if (op._verbose == true)
				System.out.println("END: " + dateFormat.format(new Date()));

		}
		catch (SQLException e)
		{
			System.out.println("ERROR " + e.getMessage());
		}
		finally
		{
			if (sr != null) sr.close();
			if (ss != null) ss.close();
			if (sc != null) sc.close();
			if (dp != null) dp.close();
			if (dc != null) dc.close();
		}
	}

	private static PreparedStatement
	createPreparedStatement(Connection dc,
							ResultSetMetaData meta,
							String tableName)
		throws SQLException
	{
		int n = meta.getColumnCount();
		StringBuilder sql = new StringBuilder();
		sql.append("insert into ").append(tableName).append("(");
		for (int i = 0; i < n; ++i)
		{
			if (i != 0)
				sql.append(", ");
			sql.append(meta.getColumnName(i + 1));
		}
		sql.append(") values (");
		for (int i = 0; i < n; ++i)
		{
			if (i != 0)
				sql.append(", ");
			sql.append("?");
		}
		sql.append(")");

		return dc.prepareStatement(sql.toString());
	}
}

//
// Copyright (c) 2009, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
