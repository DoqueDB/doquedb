// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// unload.java -- 
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

package jp.co.ricoh.sydney.admin.unload;

import java.io.*;
import java.sql.*;
import java.util.ArrayList;

import jp.co.ricoh.sydney.admin.util.Res;
import jp.co.ricoh.sydney.admin.util.log.Log;
import jp.co.ricoh.sydney.admin.util.unload.UnloadResult;

public class unload {
	private static String RESOURCE_PACKAGE = "jp.co.ricoh.sydney.admin";
	
    private Connection conn = null;
    private Statement state = null;
    
	private String host = null;
	private String port = null;
	private String dataBaseName = null;
	
	private boolean xmlFlag = false;
	private String unloadDir = null;
	private ArrayList<String> allTables = new ArrayList<String>();
	
	private String unloadCode = null;
	
	/**
	 * @param args
	 */
	public static void main(String[] args) {
	    // リソースファイルの初期化
	    Res.locale(RESOURCE_PACKAGE+".unload.resource");
	    
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
		boolean DEBUG_STATUS = false;
		unload unload = null;
		String user = null;
		String password = null;
		
		// パラメータ数チェック
		if( args.length < 1 ){
			for(int i=0;i<usage.length;i++){
				System.out.println(usage[i]);
			}
			System.exit(1);
		}
		// デバッグモードは第２パラメータにdebugと入力する。
		// デバッグモードは全てのログを出力します。
		if( args.length >= 2 ){
			for (int i = 0; i < args.length; i++) {
				if( args[i].equals("debug") == true ){
					DEBUG_STATUS = true;
					System.out.println(Res.get("DEBUG_MODE"));
				}else if(args[i].equalsIgnoreCase("english")) {
					Res.locale(RESOURCE_PACKAGE + ".manager.resource", "en", "EN");
				}
			}
		}
		if( args.length >= 3){
			user = args[1];
			password = args[2];
		}

		boolean	succeeded = false;

		try {
			// ログ情報の初期設定
		    Log.open("unload");
			if( DEBUG_STATUS == true ){
			    Log.debug();
			}
			Log.trace(Res.get("TRACE_START")); //$NON-NLS-1$
			Log.info("Version " + Res.getVersion());
	    	Log.info(Res.get("UNLOAD_START"));
			// メイン処理
			unload = new unload(args[0]);
			unload.open(user, password);		// DBオープン
			unload.write();		// アンロード処理

			succeeded = true;
		} catch (SQLException exSql) {
			exSql.printStackTrace();
		    Log.warning(exSql.toString());
		} catch (Exception ex) {
			ex.printStackTrace();
			Log.exception(ex.toString());
		} finally {
			if( unload != null ){
				try {
				    unload.close();	// DBクローズ
					Log.info(Res.get(succeeded ? "UNLOAD_SUCCEED" : "UNLOAD_FAILED"));
				} catch (SQLException exSql) {
				    Log.warning(exSql.toString());
				}
			}
	    	Log.info(Res.get("UNLOAD_END"));
			Log.trace(Res.get("TRACE_END")); //$NON-NLS-1$
		}
	}
	
	/**
	 * unloadコンストラクタ
	 * @param file unloadスクリプトファイル
	 * @throws SQLException
	 * @throws ClassNotFoundException
	 */
	public unload(String file)
		throws Exception
	{
		BufferedReader reader = null;
		String line = null;
		String keyName = null;
		String keyValue = null;
		boolean tableFlag = false;
		
		Log.trace(Res.get("TRACE_START")); //$NON-NLS-1$

	    try {
            reader = new BufferedReader(new FileReader(file));
            while ((line = reader.readLine()) != null) {
                // 空白を削除して何も無ければ次の行へ移動。
                line = line.trim();
                if( line.length() < 1 ){continue;}
                // 先頭に#があればコメント
                if( line.startsWith("#") == true ){
                    Log.trace("com="+line);
                    continue;
                }
                // テーブル名の追加
                if( tableFlag == true ){
                    this.allTables.add(line);
                    continue;
                }
                int index = line.indexOf("=");
                if( index < 0 ){continue;}
                keyName = line.substring(0,index);
                keyValue = line.substring(index+1);
                Log.trace("keyName("+keyName+"):keyValue("+keyValue+")");
        		
        		// ホスト名
        		if( keyName.equalsIgnoreCase("HOSTNAME") == true ){
        		    this.host = keyValue;
        		}
        		// ポート番号
        		if( keyName.equalsIgnoreCase("PORT") == true ){
        		    this.port = keyValue;
        		}
        		// データベース名
        		if( keyName.equalsIgnoreCase("DATABASE") == true ){
        		    this.dataBaseName = keyValue;
        		}
        		// xmlファイルでアンロードするか？
        		// ディフォルトはtextファイル
        		if( keyName.equalsIgnoreCase("FORMAT") == true ){
        		    if( keyValue.equalsIgnoreCase("XML") == true ){
        		        this.xmlFlag = true;
        		    }
        		}
        		// アンロード先ディレクトリ
        		// ディフォルトはカレント
        		if( keyName.equalsIgnoreCase("DIRECTORY") == true ){
	        		String s = System.getProperty("file.separator");
	        		if( keyValue.endsWith(s) != true ){
	        		    this.unloadDir = keyValue + s;
	        		}else{
	        			this.unloadDir = keyValue;
	        		}
        		    
        		}
        		// load文字コード
        		if( keyName.equalsIgnoreCase("UNLOAD_CODE") == true ){
    		        this.unloadCode = keyValue;
        		}
        		// TABLEキーワードが無ければ何もしない。
        		if( keyName.equalsIgnoreCase("TABLE") == true ){
        		    if( keyValue.length() > 0 ){
        		        this.allTables.add(keyValue);
        		    }
        		    tableFlag = true;
        		}
            }
            // 2006.02.21
            if(tableFlag==false || this.allTables.size()<=0){
            	Log.warning(Res.get("UNLOAD_WAR_TABLE_IS_NOT_SPECIFIED"));
            }

			if (this.unloadDir == null) {

				// 出力先未指定
				// デフォルト → カレント

				this.unloadDir = System.getProperty("user.dir") + System.getProperty("file.separator");

			} else {

				// ある？

				java.io.File	outDir = new java.io.File(this.unloadDir);
				if (outDir.exists()) {

					// あったけど、それはディレクトリ？

					if (outDir.isDirectory() == false) {

						// ファイルでした…
						// 作れない → アンロードできない
						throw new java.io.FileNotFoundException(Res.get("UNLOAD_ERR_IS_NOT_DIR"));

					} else {

						// ちゃんとディレクトリでした（スルー）

					}

				} else {

					// なかったので mkdir

					outDir.mkdirs();
				}
			}
//        } catch (Exception e) {
//          Log.exception(e.toString());
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
	 * データベースオープン
	 * @throws SQLException
	 * @throws ClassNotFoundException
	 */
	public void open(String user, String password) throws SQLException, ClassNotFoundException {
		boolean ret = false;

		Log.trace(Res.get("TRACE_START")); //$NON-NLS-1$
		
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
				if( this.state != null){
				    this.state.close();
				}
				if( this.conn != null){
				    this.conn.close();
				}
			}
			Log.trace(Res.get("TRACE_END")); //$NON-NLS-1$
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
	 * スクリプトファイルの指定どおりにアンロードを行う。
	 * 
	 */
	public void write()
		throws Exception
	{
	    Log.trace(Res.get("TRACE_START")); //$NON-NLS-1$

	    UnloadResult unloadResult = new UnloadResult(this.state,this.unloadDir,this.xmlFlag,this.unloadCode);
	    
        for(int i=0;i<this.allTables.size();i++){
		    try {
				int count = unloadResult.open((String)this.allTables.get(i));
		    	String table = unloadResult.getTableName();
		    	Log.info(Res.get("UNLOAD_TABLE_RECORD",new String[]{table,String.valueOf(count)}));
			    for(int j=0;j<count;j++){
			    	if( unloadResult.writeRecord() != true ){
			    		break;
			    	}
			    }
		    	Log.info(Res.get("UNLOAD_TABLE_SUCCEED",new String[]{table}));
//			} catch (Exception e) {
//				Log.exception(e.toString());
			}finally{
			    try {
			    	unloadResult.close();
				} catch (Exception e) {
					Log.exception(e.toString());
				}
			}
		}

        Log.trace(Res.get("TRACE_END")); //$NON-NLS-1$
	}
	
}

//
// Copyright (c) 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
