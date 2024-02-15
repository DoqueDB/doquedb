// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LoadTEXT.java -- 
// 
// Copyright (c) 2005, 2006, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.sydney.admin.util.load;

import java.io.*;
import java.sql.*;
import java.util.ArrayList;
import jp.co.ricoh.sydney.admin.util.*;
import jp.co.ricoh.sydney.admin.util.exception.AdminException;
import jp.co.ricoh.sydney.admin.util.log.Log;
import jp.co.ricoh.sydney.admin.util.load.Separator;

public class LoadTEXT
{
	private static final String	START_COMMENT = "<!--->";
	private static final String	END_COMMENT   = "<---!>";

	private LoadInsert			_load = null;

	private int					_recordCount = 0;			// ?
	private int					_insertRecordCount = 0;
	private int					_separateIndexCount = 0;

	private int					_commitCount = 0;

	private String				_loadTable = null;			// ロード先のテーブル名
	private String				_columnSeparator = null;	// ロードファイル内の列の区切り文字
	private String				_elementSeparator = null;	// ロードファイル内の配列型の列値の要素の区切り文字
	private String				_recordSeparator = null;	// ロードファイル内のレコードの区切り文字
	private String				_externColumns = null;
	private boolean			_isSeparatedFile = false;
	private String				_insertColumns = null;		// ロードするカラムを指定←v16.4対応

	private ArrayList<String>	dataList = new ArrayList<String>();

	private BufferedReader		_loadFileReader = null;	// ロードファイルのリーダ
	private BufferedReader		_sepFileReader = null;	// 外部ファイルのリーダ
	private String				_loadDir = null;	// ロードディレクトリ

	private ArrayList<String>			_recordData = null;

	private String				_loadCode = null;	// ロードファイルの文字コード
	private final static String	_defaultLoadCode = "UTF-8"; // default charset
	//private final static String	_defaultLoadCode = System.getProperty("file.encoding"); // default charset



	/**
	 * @param conn_ 接続先Connection値
	 * @param state_ 接続先Statement値
	 */
	public LoadTEXT(Connection	conn_,
					Statement	state_)
	{
		this._load = new LoadInsert(conn_, state_);
		this._loadCode = _defaultLoadCode;
	}

	/**
	 * @param conn_ 接続先Connection値
	 * @param state_ 接続先Statement値
	 * @param loadCode_ loadファイルの文字コード
	 */
	public LoadTEXT(Connection	conn_,
					Statement	state_,
					String		loadCode_)
	{
		this._load = new LoadInsert(conn_,state_);

		this._loadCode = loadCode_;
	}

	// ロードファイル内の列の区切り文字を返す
	public String getColumnSeparator()
	{
		return this._columnSeparator;
	}

	// ロードファイル内の配列型の列値の要素の区切り文字を返す
	public String getArrayElementSeparator()
	{
		return this._elementSeparator;
	}

	// ロードファイル内のレコードの区切り文字を返す
	public String getRecordSeparator()
	{
		return this._recordSeparator;
	}

	// ロード先のテーブル名を返す
	public String getLoadTable()
	{
		return this._loadTable;
	}

	public int getRecordCount()
	{
		return this._recordCount;
	}

	public int getInsertRecordCount()
	{
		return this._insertRecordCount;
	}

	public int getCommitCount()
	{
		return this._commitCount;
	}

	public boolean isSeparatedFile()
	{
		return this._isSeparatedFile;
	}

	public String[] getDataFileList()
	{
		return (String[])this.dataList.toArray(new String[this.dataList.size()]);
	}

	public String getRecordData()
	{
		return this._recordData.toString();
	}

	public void commit()
		throws SQLException
	{
		this._load.commit();
	}

	public void rollback()
		throws SQLException
	{
		this._load.rollback();
	}

	public void getFileInfo(String	loadFilePath_)
		throws IOException, SQLException, AdminException
	{
		Log.trace(Res.get("TRACE_START"));

		// 全変数の初期化
		this._commitCount = 0;
		this._recordCount = 0;
		this._insertRecordCount = 0;
		this._separateIndexCount = 0;
		this._loadTable = null;
		this._columnSeparator = null;
		this._externColumns = null;
		this._isSeparatedFile = false;
		this._loadFileReader = null;
		this._sepFileReader = null;
		this._insertColumns = null;
		this._loadDir = null;
		this.dataList.clear();

		// 指定ファイルからヘッダー情報を取得する。
		readHeader(loadFilePath_);

		Log.trace(Res.get("TRACE_END"));
	}

	/**
	 * 初期化処理及びヘッダー情報取得
	 * @param loadFilePath_ ロードファイルパス
	 * @throws IOException
	 * @throws SQLException
	 * @throws AdminException
	 */
	public void open(String	loadFilePath_)
		throws IOException, SQLException, AdminException
	{
		String line = null;

		Log.trace(Res.get("TRACE_START"));

		// 初期化
		getFileInfo(loadFilePath_);

		// 列情報の初期設定
		this._load.loadOpen(loadFilePath_, this._loadTable, this._columnSeparator, this._elementSeparator, this._externColumns, this._loadCode, this._insertColumns);
		// 内データ方式の場合ロードファイルをオープンする。
		if (this._isSeparatedFile == false) {
			this._loadFileReader = new BufferedReader(new InputStreamReader(new FileInputStream(loadFilePath_), this._loadCode));
			// ヘッダー情報を全て読みファイルポインタをレコード部へ移動する。
			while ((line = this._loadFileReader.readLine()) != null) {
				if (line.indexOf("</header>") >= 0) break;
			}
		}

		Log.trace(Res.get("TRACE_END"));
	}

	public void close()
		throws IOException, SQLException
	{
		try {
			if (this._loadFileReader != null) this._loadFileReader.close();
			if (this._sepFileReader != null) this._sepFileReader.close();
		} finally {
			this._load.loadClose();
		}
	}

	// ロードファイルのヘッダー情報を取得する。
	private void readHeader(String	loadFilePath_)
		throws AdminException, IOException, SQLException
	{
		Log.trace(Res.get("TRACE_START"));

		// ロードディレクトリの取得。
		java.io.File	f = new java.io.File(loadFilePath_);
		this._loadDir = f.getParent();

		// テーブル名
		String	loadTable = getHeader(loadFilePath_, "table");
		if (loadTable == null || loadTable.length() == 0) throw new AdminException(ErrorReason.NO_LOAD_TABLE_NAME);	// 省略不可
		this._loadTable = loadTable;

		// 列の区切り文字
		String	columnSep = getHeader(loadFilePath_, "separate");
		if (columnSep == null || columnSep.length() == 0) {
			columnSep = getHeader(loadFilePath_, "column_separator");
			if (columnSep == null || columnSep.length() == 0) throw new AdminException(ErrorReason.NO_COLUMN_SEPARATOR);	// 省略不可
		}
		//columnSep = columnSep.substring(0, 1);

		// 配列の要素の区切り文字
		String	elementSep = this.getHeader(loadFilePath_, "arrayelement_separator");

		// 列の区切り文字 column_separator と配列の要素の区切り文字 arrayelement_separator が
		// 衝突していればそれはエラー
		if (elementSep != null && elementSep.length() > 0) {
			//elementSep = elementSep.substring(0, 1);
			if (columnSep.equals(elementSep)) throw new AdminException(ErrorReason.DUPLICATE_SEPARATOR);
		}


		// レコードの区切り文字
		String	sysLineSep = java.lang.System.getProperty("line.separator");
		String	recordSep = this.getHeader(loadFilePath_, "record_separator");
		if(recordSep!=null && recordSep.length()>0){
			recordSep = recordSep.replaceAll("\\\\t", "\t");
			recordSep = recordSep.replaceAll("\\\\r", "\r");
			recordSep = recordSep.replaceAll("\\\\n", sysLineSep);
			this._recordSeparator = recordSep;
		}else{
			this._recordSeparator = sysLineSep;
		}

		this._columnSeparator = columnSep;
		this._elementSeparator = elementSep;

		// 別列情報
		this._externColumns = getHeader(loadFilePath_, "extern");

		// 別ファイル情報
		String	separatedFile = getHeader(loadFilePath_, "separated_file");
		if (separatedFile != null) {
			this._isSeparatedFile = separatedFile.equalsIgnoreCase("true");
		}

		// コミットタイミング
		String	commit = getHeader(loadFilePath_, "commit_count");
		if (commit != null) {
			try {
				this._commitCount = Integer.parseInt(commit);
			} catch (Exception	e_) {
				throw new AdminException(ErrorReason.ILLEGAL_COMMIT_COUNT);
			}
		}

		// insertするカラム名を取得(v16.4)
		this._insertColumns = getHeader(loadFilePath_, "insert_columns");

		// レコード数
		this._recordCount = Integer.parseInt(getHeader(loadFilePath_, "recordCount"));

		Log.trace(Res.get("TRACE_END"));
	}

	/**
	 * ロードファイルのヘッダー情報を取得する。
	 * @param loadFilePath_ ロードファイル名
	 * @param property_ ヘッダープロパティー名
	 * @return　ヘッダープロパティー値
	 * @throws IOException
	 * @throws Exception
	 */
	private String getHeader(	String	loadFilePath_,
								String	property_)
		throws IOException, AdminException
	{
		Log.trace(Res.get("TRACE_START"));

		boolean			commentFlag = false;
		boolean			recordFlag = false;
		boolean			propFlag = false;
		BufferedReader	reader = null;
		String			line;
		String			propValue = null;
		int				count = 0;

		property_ = property_ + "=";

		try {

			reader = new BufferedReader(new InputStreamReader(new FileInputStream(loadFilePath_)));
			while ((line = reader.readLine()) != null) {
				// コメント中
				if (commentFlag) {
					if( line.indexOf(END_COMMENT) >= 0 ){	// コメント終了調査
						commentFlag = false;		// コメント終了
					}
					continue;
				}
				// コメント開始
				if( line.indexOf(START_COMMENT) >= 0 ){		// コメント開始
					if( line.indexOf(END_COMMENT) < 0 ){	// コメントが終了しない
						commentFlag = true;			// コメントフラグをオン
					}
					continue;
				}
				if( propFlag == false ){
					int ret = line.indexOf(property_);
					if( ret == 0 ){
						propValue = line.substring(property_.length());
						propFlag = true;
						if( property_.equals("recordCount=") == false ){
							break;
						}
					}
				}
				if (recordFlag == false) {

					if (line.indexOf("</header>") >= 0) recordFlag = true;

				} else {

					// 外データの場合のチェック。
					if (this._isSeparatedFile) {
						java.io.File	file_check = new java.io.File(line);
						if( file_check.exists() != true ){
							throw new AdminException(ErrorReason.BAD_SPECFILE);
						}

						// 2006.02.21 何回も呼ばれて同じファイルが内部リストに積み上がるのを防ぐ
						if(property_.equals("recordCount=")){
							count = count + separateRecordCount(line);
						}
					} else {
						Separator.getData(line);
						if ( Separator._endFlg ) {
							count++;
							Separator._endFlg = false;
						}
					}
				}
			}

			if (property_.equals("recordCount=")) propValue = String.valueOf(count);

		} finally {

			if (reader != null) reader.close();
		}

		Log.trace(Res.get("TRACE_END"));

		return propValue;
	}

	/**
	 * @param loadFilePath_
	 * @return
	 * @throws IOException
	 */
	private int separateRecordCount(String	separateFilePath_)
		throws IOException
	{
		BufferedReader	reader = null;
		int				count = 0;

		try {

			// カウントチェック
			java.io.File	f = FileUtil.getFileHandler(this._loadDir, separateFilePath_);
			reader = new BufferedReader(new InputStreamReader(new FileInputStream(f)));
			String line = null;
			while ((line = reader.readLine()) != null) {
				Separator.getData(line);
				if ( Separator._endFlg ) {
					count++;
					Separator._endFlg = false;
				}
			}

			// 別データファイル名の保存
			this.dataList.add(separateFilePath_);

		} finally {

			if (reader != null) reader.close();

			Log.trace(Res.get("TRACE_END"));
		}

		return count;
	}

	/**
	 * ロードファイルを読み込みテーブルに反映する。
	 * @return null以外:成功 null:失敗
	 * @throws SQLException
	 * @throws IOException
	 * @throws IOException
	 * @throws SQLException
	 * @throws AdminException
	 * @throws Exception
	 */
	public boolean recordTable(boolean skip_)
		throws SQLException, IOException, AdminException
	{
		this._recordData = null;

		Log.trace(Res.get("TRACE_START"));

		// １レコード読み込み
		if (this._isSeparatedFile == false) {

			// 内データの場合
			this._recordData = getRecordData(this._loadFileReader);
			if (this._recordData == null || this._recordData.size() == 0) return false;

		// 外部ファイルの場合
		} else {
			if ((this._recordData = getSeparateLineData()) == null ||
				this._recordData.size() == 0) {
				this._separateIndexCount++;
				this._sepFileReader.close();
				this._sepFileReader = null;
				return false;
			}
		}

		// 読み飛ばし指定
		if(skip_==true)return true;

		// INSERT処理
		this._insertRecordCount++;
		this._load.insertTextRecord(this._recordData);

		Log.trace(Res.get("TRACE_END"));

		return true;
	}
	private ArrayList<String> getRecordData(BufferedReader loadFileReader_) throws IOException
	{
		Separator record = new Separator(this._recordSeparator, this._columnSeparator, loadFileReader_);
		return record.getData();
	}

	private ArrayList<String> getSeparateLineData()
		throws IOException
	{

		if (this._sepFileReader != null) {
			return getRecordData(this._sepFileReader);
		}

		// 外データファイルを全て読み込んだ。
		if (this._separateIndexCount >= this.dataList.size()) return null;

		// 残存する場合はオープンする。
		String	separateFilePath = (String)this.dataList.get(this._separateIndexCount);
		java.io.File	f = FileUtil.getFileHandler(this._loadDir, separateFilePath);
		this._sepFileReader = new BufferedReader(new InputStreamReader(new FileInputStream(f), this._loadCode));
		return getRecordData(this._sepFileReader);

	}
}


//
// Copyright (c) 2005, 2006, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
