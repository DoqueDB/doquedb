// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Log.java -- 
// 
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.sydney.admin.util.log;

import java.io.IOException;
import java.util.logging.*;

public class Log
{
	private static Logger	log = Logger.getLogger(Log.class.getName());
	private static Handler	logHandler = null;

	private static boolean	silent = false;	// Throwable.printStackTrace() を出力するかどうか（ false で出力する）

	public Log() {}

	public static void open(String	logFile_)
		throws SecurityException, IOException
	{
		// 最大100MByteで2つまでのファイル

		// ↓ なぜ append mode でないのかわからない…？？？ 上のコメントで『2つまで』と書いているのになぜ第 3 引数が 1 なのか？？？
		//logHandler = new FileHandler(logFile+".log",100000000,1,false);

		// 最大 1 [MB] を 10 個まで
		logHandler = new FileHandler(logFile_ + ".log", 1024 * 1024, 10, true);
		logHandler.setFormatter(new LoggingFormat());
		log.addHandler(Log.logHandler);
		log.setLevel(Level.INFO);
	}

	public static void close()
	{
		if (logHandler != null) logHandler.close();
	}

	/**
	 * 全てのレベルのログを出力できるようにする。
	 */
	public static void debug()
	{
		log.setLevel(Level.ALL);
	}

	/**
	 * Loggerインスタンスを返す。任意にLoggerの設定値を変更したいとき等に使用する。
	 * @return Loggerインスタンス
	 */
	public static Logger getLoggerClass()
	{
		return log;
	}

	/**
	 * 任意レベルを指定してログを出力する。
	 * @param level ログレベル
	 * @param msg メッセージ
	 */
	public static void log(	Level	level_,
							String	msg_)
	{
		log.log(level_, msg_);
	}

	/**
	 * トレースメッセージを出力する。
	 */
	public static void trace(String	msg_)
	{
		log.log(Level.FINE, msg_);
	}

	/**
	 * 情報メッセージを出力する。
	 */
	public static void info(String	msg_)
	{
		log.log(Level.INFO, msg_);
	}

	/**
	 * 警告メッセージを出力する。
	 */
	public static void warning(String	msg_)
	{
		log.log(Level.WARNING, msg_);
	}

	/**
	 * 例外メッセージを出力する。
	 */
	public static void exception(String	msg_)
	{
		log.log(Level.SEVERE, msg_);
	}

	public static void setSilent(boolean	silent_)
	{
		Log.silent = silent_;
	}

	public static boolean getSilent()
	{
		return Log.silent;
	}

	public static void printStackTrace(Throwable	t_)
	{
		if (Log.silent == false) t_.printStackTrace();
	}
}

//
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
