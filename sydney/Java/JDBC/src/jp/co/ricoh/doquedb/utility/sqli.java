// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// sqli.java -- SQL文を実行するクライアントコマンド
// 
// Copyright (c) 2002, 2004, 2005, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
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

import jp.co.ricoh.doquedb.client.*;
import jp.co.ricoh.doquedb.common.*;

/**
 * オプション
 */
class Option
{
	//実行対象
	public final static int TARGET_STDIN	= 0;
	public final static int TARGET_FILE		= 1;
	public final static int TARGET_LINE		= 2;

	//データベース名
	public String _databaseName;

	//ターゲット
	public int _target;
	//ファイル名
	public String _fileName;
	//コマンドライン
	public String _commandLine;

	//サーバホスト名
	public String _hostName;
	//ポート番号
	public int _portNumber;

	//shutdown?
	public boolean _shutdown;
	//time?
	public boolean _time;
	// skip?
	public boolean _skip;

	//code?
	public String _code;

	//crypt?
	public int _protocolVersion = jp.co.ricoh.doquedb.jdbc.Driver.PROTOCOL_VERSION5;

	// user name
	public String _userName;
	// password
	public String _password;

	//コンストラクタ
	public Option()
	{
		_shutdown = false;
		_target = TARGET_STDIN;
		_databaseName = "";
		_time = false;
		_skip = false;
		_code = "";
		_userName = null;
		_password = null;
	}

	//引数を解析する
	public boolean set(String[] args)
	{
		int i = 0;
		while (i < args.length)
		{
			char p = args[i].charAt(0);
			if (p == '-')
			{
				//オプション
				if (args[i].equals("-database"))
				{
					i++;
					if (i >= args.length) return false;
					_databaseName = args[i++];
				}
				else if (args[i].equals("-remote"))
				{
					i++;
					if (i >= args.length) return false;
					_hostName = args[i++];
					if (i >= args.length) return false;
					_portNumber = new Integer(args[i++]).intValue();
				}
				else if (args[i].equals("-sql"))
				{
					i++;
					if (i >= args.length) return false;
					_commandLine = args[i++];
					_target = TARGET_LINE;
				}
				else if (args[i].equals("-shutdown"))
				{
					i++;
					_shutdown = true;
				}
				else if (args[i].equals("-time"))
				{
					i++;
					_time = true;
				}
				else if (args[i].equals("-skip"))
				{
					i++;
					_skip = true;
				}
				else if (args[i].equals("-code"))
				{
					i++;
					if (i >= args.length) return false;
					_code = args[i++];
				}
				else if (args[i].equals("-user"))
				{
					i++;
					if (i >= args.length) return false;
					_userName = args[i++];
				}
				else if (args[i].equals("-password"))
				{
					i++;
					if (i >= args.length) return false;
					_password = args[i++];
				}
				else
				{
					System.err.println("Unknown option: '" + args[i] + "'");
					return false;
				}
			}
			else
			{
				if (_target == TARGET_LINE)
				{
					_commandLine += " ";
					_commandLine += args[i++];
					continue;
				}

				if (_target != TARGET_STDIN)
					return false;

				_fileName = args[i++];
				_target = TARGET_FILE;
			}
		}
		return true;
	}
}

/**
 * 実行する
 */
abstract class Exec
{
	private DataSource _dataSource;
	private Option _option;
	
	public Exec(DataSource dataSource_, Option option_)
	{
		_dataSource = dataSource_;
		_option = option_;
	}

	public void initialize()
		throws java.io.IOException {}
	public void terminate() {}

	//実行する
	public void execute()
		throws java.io.IOException, ClassNotFoundException,
				java.sql.SQLException,
				java.security.NoSuchAlgorithmException,
				java.security.spec.InvalidKeySpecException,
				java.security.InvalidKeyException,
				java.security.NoSuchProviderException, 
				javax.crypto.NoSuchPaddingException,
				javax.crypto.BadPaddingException,
				javax.crypto.IllegalBlockSizeException
	{
		//セッションを作成する
		Session session = _dataSource.createSession(_option._databaseName,
													_option._userName,
													_option._password);

		try
		{
			while (true)
			{
				String sql = getNext();
				if (sql == null || "exit".equals(sql)) return;

				long s = System.currentTimeMillis();

				//SQL文を実行する
				ResultSet result = session.executeStatement(sql, null);

				try
				{
					while (true)
					{
						DataArrayData tuple = new DataArrayData();
						int status;
						switch (status = result.getNextTuple(tuple))
						{
						case ResultSet.META_DATA:
							if (_option._skip == false)
								System.out.println(result.getMetaData());
							break;
						case ResultSet.DATA:
							if (_option._skip == false)
								System.out.println(tuple);
							break;
						case ResultSet.END_OF_DATA:
						case ResultSet.HAS_MORE_DATA:
							break;
						case ResultSet.SUCCESS:
//							System.out.println("OK");
							break;
						case ResultSet.CANCELED:
							System.out.println("CANCEL");
							break;
						default:
							throw new jp.co.ricoh.doquedb.exception.Unexpected();
						}

						if (status == ResultSet.SUCCESS
							|| status == ResultSet.CANCELED)
						{
							break;
						}
					}
				}
				catch (java.sql.SQLException e)
				{
					System.out.println("ERROR " + e.getMessage());
				}
				finally
				{
					result.close();

					if (_option._time == true)
					{
						long interval = System.currentTimeMillis() - s;
						System.out.println("TIME: " + interval + " ms");
					}
				}
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
		finally
		{
			session.close();
		}
	}

	//SQL文を１つ取り出す
	public abstract String getNext()
		throws java.io.IOException;
}

/**
 * 実行する
 */
class ExecStdin extends Exec
{
	private String _code;

	protected java.io.InputStream in;
	protected java.io.InputStreamReader stream;
	
	public ExecStdin(DataSource dataSource_, Option option_)
	{
		super(dataSource_, option_);
		_code = option_._code;
		in = System.in;
	}

	public void initialize()
		throws java.io.IOException
	{
		//ストリームを確保する
		if ("".equals(_code))
			stream = new java.io.InputStreamReader(in);
		else
			stream = new java.io.InputStreamReader(in, _code);
	}

	public void printSQL()
	{
		System.out.print("SQL>");
		System.out.flush();
	}

	public String getNext()
		throws java.io.IOException
	{
		boolean literal = false;
		printSQL();
		StringBuilder buf = new StringBuilder();
		int c;
		while ((c = stream.read()) != -1)
		{
			if (c == '\n')
			{
				if (buf.length() == 0)
				{
					printSQL();
					continue;
				}
			}
			else if (c == ' ' || c == '\t' || c == '\r')
			{
				if (buf.length() == 0) continue;
			}
			else if (c == '\'')
			{
				if (literal == false) literal = true;
				else literal = false;
			}

			if (literal == false && c == ';')
			{
				for (;;)
				{
					c = stream.read(); //改行を読む
					if (c == '\n') break;
				}
				if (buf.length() != 0) break;
				else printSQL();
			}
			else
			{
				buf.append((char)c);
			}
		}

		if (buf.length() != 0)
		{
			return buf.toString();
		}

		return null;
	}
}

/**
 * 実行する
 */
class ExecFile extends ExecStdin
{
	private String _fileName;
	
	public ExecFile(DataSource dataSource_, Option option_)
	{
		super(dataSource_, option_);
		_fileName = option_._fileName;
	}

	public void initialize()
		throws java.io.IOException
	{
		//ストリームを確保する
		in = new java.io.FileInputStream(_fileName);
		super.initialize();
	}

	public void printSQL()
	{
	}
}

/**
 * 実行する
 */
class ExecLine extends Exec
{
	private String _statement;
	
	public ExecLine(DataSource dataSource_, Option option_)
	{
		super(dataSource_, option_);
		_statement = option_._commandLine;
	}

	public String getNext()
		throws java.io.IOException
	{
		String s = _statement;
		_statement = null;
		return s;
	}
}

/**
 * SQL文を実行するクライアントコマンド
 *
 * @author Takuya Hiraoka
 * @version 1.0
 */
public class sqli
{
	//USAGE
	private static void USAGE()
	{
		System.out.println("");
		System.out.println("Usage: sqli -remote host_name port_number [options] [filename]");
		System.out.println("       (execute sql statement)");
		System.out.println("   or");
		System.out.println("       sqli -remote host_name port_number -shutdown");
		System.out.println("       (shutdown server)");
		System.out.println("");
		System.out.println("where options include:");
		System.out.println("    -database database_name             set a database name.");
		System.out.println("    -user user_name             		set a user name.");
		System.out.println("    -password password 		            set password.");
		System.out.println("    -sql statement                      execute a command line sql.");
		System.out.println("    -time                               print exeution time.");
		System.out.println("    -skip                               skip exeution output.");
		System.out.println("    -help                               print this message.");
		System.out.println("");
	}
	
	/**
	 * メイン
	 */
	public static void main(String[] args)
		throws java.io.IOException, ClassNotFoundException,
				java.sql.SQLException,
				java.security.NoSuchAlgorithmException, java.security.spec.InvalidKeySpecException, java.security.spec.InvalidKeySpecException,
				java.security.InvalidKeyException, java.security.NoSuchProviderException, 
				javax.crypto.NoSuchPaddingException, javax.crypto.BadPaddingException, javax.crypto.IllegalBlockSizeException
	{
		Option op = new Option();
		if (op.set(args) == false)
		{
			USAGE();
			return;
		}

		if (op._hostName == null || op._portNumber == 0)
		{
			USAGE();
			return;
		}

		DataSource dataSource = new DataSource(op._hostName, op._portNumber);

		if (op._shutdown == true)
		{
			if (op._userName != null)
				dataSource.shutdown(op._userName, op._password);
			else
				dataSource.shutdown();
		}
		else
		{
			dataSource.open(op._protocolVersion);
				
			try
			{
				Exec exec = null;
				switch (op._target)
				{
				case Option.TARGET_STDIN:
					exec = new ExecStdin(dataSource, op);
					break;
				case Option.TARGET_FILE:
					exec = new ExecFile(dataSource, op);
					break;
				case Option.TARGET_LINE:
					exec = new ExecLine(dataSource, op);
				}

				exec.initialize();
				exec.execute();
				exec.terminate();
			}
			catch (Exception e)
			{
				e.printStackTrace();
				return;
			}
			finally
			{
				dataSource.close();
			}
		}
	}
}

//
// Copyright (c) 2002, 2004, 2005, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

