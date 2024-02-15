// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// load.java -- ロードコマンドのメインクラス
// 
// Copyright (c) 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.sydney.admin.load;

import java.io.*;
import java.sql.*;
import java.util.ArrayList;

import javax.xml.parsers.ParserConfigurationException;

import org.xml.sax.SAXException;

import jp.co.ricoh.sydney.admin.util.Res;
import jp.co.ricoh.sydney.admin.util.exception.AdminException;
import jp.co.ricoh.sydney.admin.util.load.*;
import jp.co.ricoh.sydney.admin.util.log.Log;

public class load
{
	private static String	RESOURCE_PACKAGE = "jp.co.ricoh.sydney.admin";

	private Connection		conn = null;
	private Statement		state = null;

	private String			host = null;
	private String			port = null;
	private String			dataBaseName = null;

	private boolean			xmlFlag = false;
	private String			_loadDirPath = null;

	private String			loadCode = null;

	private ArrayList<String>		allFiles = new ArrayList<String>();

	/**
	 * @param args_
	 */
	public static void main(String[]	args_)
	{
		// リソースファイルの初期化
		Res.locale(RESOURCE_PACKAGE+".load.resource");

		String[]	usage = {
			Res.get("USAGE_0"),
			Res.get("USAGE_1"),
			Res.get("USAGE_2"),
			Res.get("USAGE_3"),
			Res.get("USAGE_4"),
			Res.get("USAGE_5"),
			Res.get("USAGE_6"),
			Res.get("USAGE_7"),
		};
		boolean	isDebugMode = false;
		boolean	loadStatus = false;
		load	load = null;
		String	user = null;
		String	password = null;

		// パラメータ数チェック
		if (args_.length < 1) {
			for (int i = 0; i < usage.length; i++) {
				System.out.println(usage[i]);
			}
			System.exit(1);
		
		}
		if (args_.length >= 2){
			// デバッグモードは第2パラメータにdebugと入力する。
			// デバッグモードは全てのログを出力します。
			
			for (int i = 0; i < args_.length; i++) {
				if (args_[i].equals("debug")) {
					isDebugMode = true;
					System.out.println(Res.get("DEBUG_MODE"));
				}else if(args_[i].equalsIgnoreCase("english")) {
					Res.locale(RESOURCE_PACKAGE + ".manager.resource", "en", "EN");
				}
			}
			
		} 
		if( args_.length >= 3){
			user = args_[1];
			password = args_[2];
		}

		try {
			// ログ情報の初期設定
			Log.open("load");
			if (isDebugMode) {
				Log.debug();
			}
			Log.trace(Res.get("TRACE_START"));
			Log.info("Version " + Res.getVersion());
			Log.info(Res.get("LOAD_START"));
			// メイン処理
			load = new load(args_[0]);
			boolean	ret = load.checkFile();
			if (ret) {
				load.info();
				load.open(user, password);
				loadStatus = load.execute();
			}
		} catch (SQLException	exSql) {
			Log.warning(exSql.toString());
		} catch (Exception	ex) {
			Log.exception(ex.toString());
		} catch (AdminException	ea) {
			Log.exception(ea.toString());
		} finally {
			if (load != null) {
				try {
					load.close();
				} catch (SQLException	exSql) {
					Log.warning(exSql.toString());
				}
			}
			if (loadStatus) {
				Log.info(Res.get("LOAD_END"));
			} else {
				Log.warning(Res.get("LOAD_ERR_END"));
			}
			Log.trace(Res.get("TRACE_END"));
			Log.close();
		}
	}

	/**
	 * loadコンストラクタ
	 * @param scriptFileName_ loadスクリプトファイル名
	 * @throws SQLException
	 * @throws ClassNotFoundException
	 */
	public load(String	scriptFileName_)
	{
		BufferedReader	reader = null;
		String			line = null;
		String			keyName = null;
		String			keyValue = null;
		boolean			tableFlag = false;
		
		Log.trace(Res.get("TRACE_START")); //$NON-NLS-1$
		
		// カレントディレクトリの取得
		this._loadDirPath = System.getProperty("user.dir") + System.getProperty("file.separator");

		try {

			reader = new BufferedReader(new FileReader(scriptFileName_));
			while ((line = reader.readLine()) != null) {
				// 空白を削除して何も無ければ次の行へ移動。
				line = line.trim();
				if (line.length() < 1) continue;
				// 先頭に#があればコメント
				if (line.startsWith("#")) {
					Log.trace("com="+line);
					continue;
				}
				if (tableFlag == false) {
					int	index = line.indexOf("=");
					if (index < 0) continue;
					keyName = line.substring(0, index);
					keyValue = line.substring(index + 1);
					Log.trace("keyName(" + keyName + "):keyValue(" + keyValue + ")");
					
					// ホスト名
					if (keyName.equalsIgnoreCase("HOSTNAME")) {
						this.host = keyValue;
					}
					// ポート番号
					if (keyName.equalsIgnoreCase("PORT")) {
						this.port = keyValue;
					}
					// データベース名
					if (keyName.equalsIgnoreCase("DATABASE")) {
						this.dataBaseName = keyValue;
					}
					// xmlファイルでアンロードするか？
					// ディフォルトはtextファイル
					if (keyName.equalsIgnoreCase("FORMAT")) {
						if (keyValue.equalsIgnoreCase("XML")) {
							this.xmlFlag = true;
						}
					}
					// load文字コード
					if (keyName.equalsIgnoreCase("LOAD_CODE")) {
						this.loadCode = keyValue;
					}
					// アンロード先ディレクトリ
					// ディフォルトはカレント
					if (keyName.equalsIgnoreCase("DIRECTORY")) {
						String	s = System.getProperty("file.separator");
						if (keyValue.endsWith(s) != true) {
							this._loadDirPath = keyValue + s;
						} else {
							this._loadDirPath = keyValue;
						}
					}
					// FILESキーワードが無ければ何もしない。
					if (keyName.equalsIgnoreCase("FILES")) {
						if (keyValue.length() > 0) {
							// 2006.02.21
//							this.allFiles.add(line);
							this.allFiles.add(keyValue);
						}
						tableFlag = true;
					}
				} else {
					this.allFiles.add(line);
				}
			}

		} catch (Exception e) {
			Log.exception(e.toString());
		} finally {
			if (reader != null) {
				try {
					reader.close();
				} catch (Exception e) {
					Log.exception(e.toString());
				}
			}
			Log.trace(Res.get("TRACE_END")); //$NON-NLS-1$
		}
	}

	/**
	 * ファイル存在チェックを行う。
	 * @return true:XMLファイル false:textファイル
	 */
	public boolean checkFile()
	{
		boolean ret = false;

		Log.trace(Res.get("TRACE_START"));

		// ディレクトリ存在チェック
		java.io.File	loadDir = new java.io.File(this._loadDirPath);
		if (loadDir.exists() != true ) {
			if (loadDir.isDirectory() != true) {
				Log.warning(Res.get("LOAD_ERR_DIRECTORY", new String[] { this._loadDirPath }));
				return ret;
			}
		}

		// ファイル存在チェック
		for (int i = 0; i < this.allFiles.size(); i++) {
			String	loadFilePath = this._loadDirPath + this.allFiles.get(i);
			java.io.File	loadFile = new File(loadFilePath);
			if (loadFile.exists() != true ){
				if (loadFile.isDirectory()) {
					Log.warning(Res.get("LOAD_ERR_FILE", new String[] { loadFilePath }));
					return ret;
				}
			}
		}

		// FILESキーワードが無い
		// 全ファイルを操作する。
		if( this.allFiles.size() == 0 ){
			String loadDirPath = new String();
			recursive(loadDirPath, loadDir, 0);
		}

		ret = true;

		Log.trace(Res.get("TRACE_END"));

		return ret;
	}
	
	private void recursive(	String	loadDirPath_,
							File	dir_,
							int		indent_)
	{
		//ディレクトリの内容の取得
		String[]	contents = dir_.list();

		//その出力
		for (int i = 0; i < contents.length; i++) {

			//サブパスのオブジェクト化
			java.io.File	subDir = new java.io.File(dir_, contents[i]);
			//ディレクトリならば再帰的にリスト
			if (subDir.isDirectory()) {
				String	subPath = loadDirPath_ + contents[i] + System.getProperty("file.separator");
				recursive(subPath, subDir, indent_ + 1);
			}

			String	name = contents[i].toLowerCase();
			if (this.xmlFlag) {
				if (name.endsWith(".xml")) {
					this.allFiles.add(loadDirPath_ + contents[i]);
				}
			}else{
				if (name.endsWith(".dat")) {
					this.allFiles.add(loadDirPath_ + contents[i]);
				}
			}
		}
	}
	
	public void info() throws AdminException {
		StringBuffer msg = new StringBuffer();
		boolean status = true;
		
		msg.append(Res.get("LOAD_ERR_SCRIPT_FILE")+"\n");
		// ホスト名
		if( this.host == null ){
			msg.append(Res.get("LOAD_HISSU",new String[]{"HOSTNAME"})+"\n");
			status = false;
		}else{
			Log.info(Res.get("LOAD_HOSTNAME",new String[]{this.host}));
		}
		// データベース名
		if( this.dataBaseName == null ){
			msg.append(Res.get("LOAD_HISSU",new String[]{"DATABASE"})+"\n");
			status = false;
		}else{
			Log.info(Res.get("LOAD_DATABASE_NAME",new String[]{this.dataBaseName}));
		}
		// ポート番号
		if( this.port == null ){
			msg.append(Res.get("LOAD_HISSU",new String[]{"PORT"})+"\n");
			status = false;
		}else{
			Log.info(Res.get("LOAD_PORT_NUMBER",new String[]{this.port}));
		}
		if( status != true ){
			String errmsg = msg.toString();
			errmsg = errmsg.substring(0,errmsg.length()-1);
			throw new AdminException(errmsg);
		}
		// 出力フォーマット
		String format = "TEXT";
		if (this.xmlFlag) {
			format = "XML";
		}
		Log.info(Res.get("LOAD_FORMAT",new String[]{format}));
		// 文字コード
		if( this.loadCode == null ){
			Log.info(Res.get("LOAD_LOADCODE",new String[]{Res.get("LOAD_STRING_SYSTEM")}));
		}else{
			Log.info(Res.get("LOAD_LOADCODE",new String[]{this.loadCode}));
		}
		Log.info(Res.get("LOAD_DIRECTORY",new String[]{this._loadDirPath}));
		StringBuffer loadFilePaths = new StringBuffer();
		loadFilePaths.append(Res.get("LOAD_FILES")+"\n");
		for(int i=0;i<this.allFiles.size();i++){
			loadFilePaths.append(this._loadDirPath+(String)this.allFiles.get(i)+"\n");
		}
		String loadFiles = loadFilePaths.toString();
		loadFiles = loadFiles.substring(0,loadFiles.length()-1);
		Log.info(loadFiles);
	}
	
	/**
	 * データベースオープン
	 * @throws SQLException
	 * @throws ClassNotFoundException
	 */
	public void open(String user, String password) throws SQLException, ClassNotFoundException {
		boolean ret = false;

		Log.trace(Res.get("TRACE_START"));
		
		String url = "jdbc:ricoh:doquedb://"+this.host+":"+this.port+"/"+this.dataBaseName;
		Log.info("DATABASE URL="+url);
		
		try{
			Class.forName(Res.get("DATABASE_DRIVER")); //$NON-NLS-1$
			this.conn = DriverManager.getConnection(url, user, password);

			// v14未対応
			if(this.conn.getMetaData().getDatabaseProductVersion().equals("14.0")==true){
				throw new jp.co.ricoh.doquedb.exception.NotSupported();
			}
			this.conn.setAutoCommit(false);
			this.state = this.conn.createStatement();
			
			ret = true;
		} finally {
			if( ret != true ){
				if( state != null){
					state.close();
				}
				if( conn != null){
					conn.close();
				}
			}
			Log.trace(Res.get("TRACE_END"));
		}
	}

	/**
	 * データベースクローズ
	 * @throws SQLException
	 */
	public void close() throws SQLException {
		Log.trace(Res.get("TRACE_START")); //$NON-NLS-1$

		if (this.state != null) {
			this.state.close();
		}
		if (this.conn != null) {
			this.conn.close();
		}

		Log.trace(Res.get("TRACE_END")); //$NON-NLS-1$
	}

	/**
	 * ロード用クラスを呼び出す
	 * @throws SAXException
	 * @throws ParserConfigurationException
	 * 
	 */
	public boolean execute() throws ParserConfigurationException, SAXException {
		LoadTEXT text = null;
		LoadXML xml = null;
		boolean executeStatus = true;

		// 総レコードを管理する為にコミットカウントに対応したレコードブロック数を管理
		int total_block_count = 0;

		Log.trace(Res.get("TRACE_START"));

		// loadユーティリティーの初期化
		if (this.xmlFlag) {
			xml = new LoadXML(this.conn,this.state,this.loadCode);
		}else{
			text = new LoadTEXT(this.conn,this.state,this.loadCode);
		}

		// スクリプトファイルで指定された全loadファイルを処理する。
		for(int i=0;i<this.allFiles.size();i++){
			// loadファイルの絶対パス
			String	loadFilePath = this._loadDirPath + this.allFiles.get(i);
			String	loadFileName = (String)this.allFiles.get(i);
			Log.info(Res.get("LOAD_START_TABLENAME"));
			try {
				// レコード読み込み
				boolean ret = true;
				boolean commitStatus = true;
				int count = 0;
				int recordCount = 0;
				int commitCount = 0;
				String tableName = null;
				boolean separate = false;
				// loadファイルをオープンする。
				if (this.xmlFlag) {
					xml.open(loadFilePath);
					commitCount = xml.getCommitCount();
					tableName = xml.getLoadTable();
					separate = xml.isSeparatedFile();
				}else{
					text.open(loadFilePath);
					commitCount = text.getCommitCount();
					tableName = text.getLoadTable();
					separate = text.isSeparatedFile();
				}
				// commit範囲が指定された場合
				String sepaBuff = "false";
				if (separate) {
					sepaBuff = "true";
				}
				Log.info(Res.get("LOAD_SPEC",new String[] { loadFileName, tableName, sepaBuff, String.valueOf(commitCount) }));
				for( ;; ){
					try{
						if (this.xmlFlag) {
							ret = xml.recordTable(false);
						}else{
							ret = text.recordTable(false);
						}
						if( ret == false ){
							break;
						}
						count++;
						// commitの範囲が指定されていた場合。
						if( commitCount != 0 ){
							if( count >= commitCount ){
								if (commitStatus) {
									// この時点で全て正常にデータベースに反映されている。
									if( this.xmlFlag == false ){text.commit();}else{xml.commit();}
									Log.info(Res.get("LOAD_COMMIT",new String[]{String.valueOf(commitCount)}));

									// 現時点までのコミットカウントに対応したブロック数
									total_block_count++;

//								}else{
//									// この時点で１件でもデータベースの反映に失敗している場合。
//									if( this.xmlFlag == false ){text.rollback();}else{xml.rollback();}
//									Log.warning(Res.get("LOAD_ROLLBACK"));
								}
								count = 0;
								commitStatus = true;
							}
						}
						recordCount++;
					}catch (Exception e) {
						commitStatus = false;
						executeStatus = false;
						Log.exception(e.toString());						
						if (this.xmlFlag) {
							StringBuffer tagBuff = new StringBuffer();
							tagBuff.append("RECORD DATA\n<" + xml.getTagName() + ">\n");
							for(int k=0;k<xml.getErrTagSize();k++){
								LoadItem item = xml.getErrTag(k);
								String name = item.getName();
								String value = item.getValue();
								tagBuff.append("    <"+name+">"+value+"</"+name+">\n");
							}
							tagBuff.append("</" + xml.getTagName() + ">");
							Log.exception(tagBuff.toString());
						}else{
							Log.exception(text.getRecordData());
						}

						// 2006.01.17
						// エラーの時点でロールバックするように修正
						// この時点で１件でもデータベースの反映に失敗している場合。
						if( this.xmlFlag == false ){text.rollback();}else{xml.rollback();}
						Log.warning(Res.get("LOAD_ROLLBACK"));

						// ロールバック前の成功してしまった分を総成功カウント(recordCount)から減らす
						recordCount = recordCount - count;

						count++;// 失敗したレコード自身をカウント
						commitStatus = true;
						int nSkip = commitCount-count;
						for(int nLoop=0;nLoop<nSkip;nLoop++){
							if (this.xmlFlag) {
								ret = xml.recordTable(true);
							}else{
								ret = text.recordTable(true);
							}
							if( ret == false ){
								commitStatus = false;
								break;
							}
							count++;
						}
						Log.warning("rollback #" + (total_block_count*commitCount+1) + "-#" + (total_block_count*commitCount+count));

						count = 0;
						// 現時点までのコミットカウントに対応したブロック数(失敗したブロック含む)
						total_block_count++;
						if(ret==false)break;
						// コミットカウント0の場合は1件でもエラーで終了
						if(commitCount==0){
							commitStatus = false;
							break;
						}
					}
				}
				if (commitStatus) {
					if( this.xmlFlag == false ){text.commit();}else{xml.commit();}
					Log.info(Res.get("LOAD_INFO_SUCCEED",new String[]{tableName,String.valueOf(recordCount)}));
				}else{
					if( this.xmlFlag == false ){text.rollback();}else{xml.rollback();}
					Log.warning(Res.get("LOAD_ERR_FAILED",new String[]{tableName}));
				}
				Log.info(Res.get("LOAD_END_TABLENAME"));
			} catch (Exception e) {
				Log.exception(e.toString());
				executeStatus = false;
			} catch (AdminException e) {
				String	reason = load.getLoadErrorReason(e);
				if (reason == null) {
					reason = "";
				} else if (reason.length() > 0) {
					reason = " " + reason;
				}
				Log.warning(Res.get("LOAD_ERR_LOADFILE", new String[] { loadFileName }) + reason);
				executeStatus = false;
			}finally{
				try {
					if( xml != null ){xml.close();}
					if( text != null ){text.close();}
				} catch (Exception e1) {
					Log.exception(e1.toString());
				}
			}
		}

		Log.trace(Res.get("TRACE_END"));

		return executeStatus;

	}

	private static String getLoadErrorReason(AdminException	exception_)
	{
		String	reason = null;
		switch (exception_.getCode()) {
		case jp.co.ricoh.sydney.admin.util.load.ErrorReason.NO_LOAD_TABLE_NAME:
			reason = Res.get("LOAD_ERR_NO_LOAD_TABLE_NAME");
			break;
		case jp.co.ricoh.sydney.admin.util.load.ErrorReason.NO_COLUMN_SEPARATOR:
			reason = Res.get("LOAD_ERR_NO_COLUMN_SEPARATOR");
			break;
		case jp.co.ricoh.sydney.admin.util.load.ErrorReason.DUPLICATE_SEPARATOR:
			reason = Res.get("LOAD_ERR_DUPLICATE_SEPARATOR");
			break;
		case jp.co.ricoh.sydney.admin.util.load.ErrorReason.NO_ARRAY_ELEMENT_SEPARATOR:
			reason = Res.get("LOAD_ERR_NO_ARRAY_ELEMENT_SEPARATOR");
			break;
		case jp.co.ricoh.sydney.admin.util.load.ErrorReason.NOT_SUPPORTED_ELEMENT_DATA_TYPE:
			reason = Res.get("LOAD_ERR_NOT_SUPPORTED_ELEMENT_DATA_TYPE", new String[] { exception_.getCauseObject() });
			break;
		case jp.co.ricoh.sydney.admin.util.load.ErrorReason.ILLEGAL_COMMIT_COUNT:
			reason = Res.get("LOAD_ERR_ILLEGAL_COMMIT_COUNT");
			break;
		case jp.co.ricoh.sydney.admin.util.load.ErrorReason.NOT_ARRAY_COLUMN:
			reason = Res.get("LOAD_ERR_NOT_ARRAY_COLUMN", new String[] { exception_.getCauseObject() });
			break;
		case jp.co.ricoh.sydney.admin.util.load.ErrorReason.NOT_SET_ELEMENT:
			reason = Res.get("LOAD_ERR_NOT_SET_ELEMENT", new String[] { exception_.getCauseObject() });
			break;
		default:
			reason = "";
			break;
		}
		return reason;
	}
}

//
// Copyright (c) 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
