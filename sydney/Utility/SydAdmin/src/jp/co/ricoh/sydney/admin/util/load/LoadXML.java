// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LoadXML.java -- 
// 
// Copyright (c) 2005, 2006, 2023 Ricoh Company, Ltd.
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
import java.util.*;

import javax.xml.parsers.*;

import jp.co.ricoh.sydney.admin.util.*;
import jp.co.ricoh.sydney.admin.util.exception.AdminException;
import jp.co.ricoh.sydney.admin.util.log.Log;

import org.xml.sax.SAXException;

public class LoadXML implements Runnable
{
	private SAXParserFactory	_parserFactory = null;

	private LoadXMLHandler		_xmlHandler = null;
	private LoadXMLHandler		_threadHandler = null;

	private LoadInsert			_load = null;

	private int					_recordCount = 0;
	private int					_insertRecordCount = 0;
	private int					_separateIndexCount = 0;

	private int					_commitCount = 0;

	private String				_loadTable = null; 
	private String				_externColumns = null;
	private boolean			_isSeparatedFile = false;
	private String				_insertColumns = null;		// ロードするカラムを指定←v16.4対応

	private String				_loadCode = null;
	private final static String	_defaultLoadCode = "UTF-8"; // default charset

	private ArrayList<String>			_dataList = new ArrayList<String>();
	private ArrayList<LoadItem>		_errTagList = new ArrayList<LoadItem>();

	private String				_loadDir = null;
	private boolean				_forceStatus = false;
	private String				_tagName = null;

	private File				_threadF = null;

	public LoadXML(	java.sql.Connection	conn_,
					java.sql.Statement	state_)
		throws ParserConfigurationException, SAXException
	{
		// SAXパーサーを生成
		this._parserFactory = SAXParserFactory.newInstance();

		// SAX用ハンドルの生成
		this._xmlHandler = new LoadXMLHandler();
		this._threadHandler = new LoadXMLHandler();

		// INSERT用クラスの生成
		this._load = new LoadInsert(conn_, state_);

		this._loadCode = _defaultLoadCode;
	}

	public LoadXML(	java.sql.Connection	conn_,
					java.sql.Statement	state_,
					String				loadCode_)
		throws ParserConfigurationException, SAXException
	{
		this(conn_, state_);

		this._loadCode = loadCode_;
	}

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
		return (String[])this._dataList.toArray(new String[this._dataList.size()]);
	}

	public int getErrTagSize()
	{
		return this._errTagList.size();
	}

	public LoadItem getErrTag(int	i_)
	{
		return (LoadItem)this._errTagList.get(i_);
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

	public String getTagName()
	{
		return this._tagName;
	}

	public void ForceEndRecord()
		throws SQLException, IOException, InterruptedException, AdminException
	{
		this._forceStatus = true;
		recordTable(false);
	}

	public void getFileInfo(String	loadFilePath_)
		throws IOException, ParserConfigurationException, SAXException, AdminException
	{
		Log.trace(Res.get("TRACE_START"));

		// 全変数の初期化
		this._commitCount = 0;
		this._recordCount = 0;
		this._insertRecordCount = 0;
		this._separateIndexCount = 0;
		this._loadTable = null; 
		this._externColumns = null;
		this._isSeparatedFile = false;
		this._insertColumns = null;
		this._loadDir = null;
		this._dataList.clear();

		this._forceStatus = false;

		// 指定ファイルからヘッダー情報を取得する。
		readHeader(loadFilePath_);

		Log.trace(Res.get("TRACE_END"));
	}

	/**
	 * 初期化処理及びヘッダー情報取得
	 * @param loadFilePath_ ロードファイル名
	 * @throws SQLException
	 * @throws IOException
	 * @throws SAXException
	 * @throws AdminException
	 * @throws ParserConfigurationException
	 */
	public void open(String	loadFilePath_)
		throws SQLException, IOException, ParserConfigurationException, SAXException, AdminException
	{
		Log.trace(Res.get("TRACE_START"));

		// 初期化
		this.getFileInfo(loadFilePath_);

		// 列情報の初期設定
		this._load.loadOpen(loadFilePath_, this._loadTable, null, null, this._externColumns, this._loadCode, this._insertColumns);

		if (this._isSeparatedFile == false) {
			this._threadF = new File(loadFilePath_);
			this._threadHandler.open("/sydney/record");
			this._threadHandler.setThreadStatus(true);
			
			Thread	th = new Thread(this);
			th.start();
		}

		Log.trace(Res.get("TRACE_END"));
	}

	public void close()
		throws IOException, SQLException
	{
		// エラー時にスレッドが残るので終了するように設定
		this._threadHandler.setForceStatus(true);

		this._load.loadClose();
	}

	/**
	 * ロードファイルを読み込みテーブルに反映する。
	 * @return true:データが存在 false:データ終了
	 * @throws InterruptedException
	 * @throws IOException
	 * @throws SQLException
	 * @throws IOException
	 * @throws SQLException
	 * @throws Exception
	 * @throws AdminException
	 * @throws Exception
	 */
	public boolean recordTable(boolean skip_)
		throws InterruptedException, SQLException, IOException, AdminException
	{
		// 中断イベントが発生した
		if (this._forceStatus) {
			// スレッド強制終了
			this._threadHandler.setForceStatus(true);
			// 終了フラグを返す
			return false;
		}

		// １行読み込み
		if (this._isSeparatedFile == false) {

			// 1要素ブロックを読み込む(実際はハンドラーの停止処理を設定(loop()の脱出))
			// falseで終了
			if (this._threadHandler.read() == false) return false;

		} else {

			if (getSeparateData() == false) return false;
		}

		// 読み飛ばし指定
		if(skip_==true)return true;


		boolean	status = false;
		try {

			this._load.insertXMLRecord(this._threadHandler.getItemList(), this._loadCode);
			this._insertRecordCount++;
			status = true;

		} finally {

			if (status != true) {
				this._tagName = this._threadHandler.getTagName();
				this._errTagList.clear();
				for (int i = 0; i < this._threadHandler.getItemSize(); i++) {
					LoadItem	item = this._threadHandler.getItem(i);
					this._errTagList.add(item);
				}
			}
		}

		return true;
	}

	private boolean getSeparateData()
		throws InterruptedException
	{
		if (this._separateIndexCount != 0 && this._threadHandler.read()) return true;
		if (this._separateIndexCount >= this._dataList.size()) return false;

		String	file = (String)this._dataList.get(this._separateIndexCount);
		this._threadF = FileUtil.getFileHandler(this._loadDir, file);
		this._threadHandler.open("/*/*");
		this._threadHandler.setThreadStatus(true);

		Thread	th = new Thread(this);
		th.start();

		this._separateIndexCount++;

		// １データを読み込む。
		return this._threadHandler.read();
	}

	private void readHeader(String	loadFilePath_)
		throws IOException, AdminException, ParserConfigurationException, SAXException
	{
		SAXParser	parser = null;
		String[]	st = null;

		Log.trace(Res.get("TRACE_START"));

		// パーサーの構築
		parser = this._parserFactory.newSAXParser();

		// ロードディレクトリの取得。
		java.io.File	f = new File(loadFilePath_);
		this._loadDir = f.getParent();

		// ヘッダー情報の取得。
		try {
			this._xmlHandler.open("/sydney/header");
			this._xmlHandler.setForceStatus(true);

			// 2006.07.20
			// UTF-8ファイルの先頭に EF BB BF が入ってる場合の対応
//			parser.parse(f,this._xmlHandler);
			FileInputStream isr = new FileInputStream(f);
			org.xml.sax.InputSource ins = new org.xml.sax.InputSource(isr);
			parser.parse(ins,this._xmlHandler);
		} catch (SAXException	se_) {

			// 強制終了エラーかどうか検証する。
			if (se_.getMessage().equals("RICOH!SYDNEY!") != true){
				// 2006.07.20
				// UTF-8ファイルの先頭に EF BB BF が入ってる場合の対応
//				throw new SAXException(se_);
				try{
					FileInputStream isr = new FileInputStream(f);
					org.xml.sax.InputSource ins = new org.xml.sax.InputSource(isr);
					isr.skip(3);
					ins = new org.xml.sax.InputSource(isr);
					parser.parse(ins,this._xmlHandler);
				}catch(SAXException se_2){
					// 強制終了エラーかどうか検証する。
					if (se_2.getMessage().equals("RICOH!SYDNEY!") != true){
						throw new SAXException(se_2);
					}
				}
			}
		}

		// テーブル名の取得
		st = getHeader("table");
		if (st == null || st.length == 0) throw new AdminException(ErrorReason.NO_LOAD_TABLE_NAME);
		this._loadTable = st[0];

		// 別列情報
		this._externColumns = "";
		st = getHeader("extern");
		if (st != null && st.length > 0) {
			for (int i = 0; i < st.length; i++) {
				this._externColumns = this._externColumns + st[i] + ",";
			}
			this._externColumns = this._externColumns.substring(0, this._externColumns.length() - 1);
		}

		// 別ファイル情報
		st = getHeader("separated_file");
		if (st != null) {
			if (st.length > 0) {
				String separatedFile = st[0];
				this._isSeparatedFile = separatedFile.equalsIgnoreCase("true");
			}
		}

		// コミットタイミング
		st = getHeader("commit_count");
		if (st != null && st.length > 0) {
			try {
				this._commitCount = Integer.parseInt(st[0]);
			} catch (Exception	e_) {
				throw new AdminException(ErrorReason.ILLEGAL_COMMIT_COUNT);
			}
		}

		// insertするカラム名を取得(v16.4)
		st = getHeader("insert_columns");
		if (st != null) {
			if (st.length > 0) {
				this._insertColumns = st[0];
			}
		}
		
		// レコード数の取得
		this._xmlHandler.open("/sydney/record");
		if (this._isSeparatedFile == false) {

			this._xmlHandler.setCountStatus(true);
//			parser.parse(f, this._xmlHandler);
			// 2006.07.20
			// UTF-8ファイルの先頭に EF BB BF が入ってる場合の対応
			try{
				FileInputStream isr = new FileInputStream(f);
				org.xml.sax.InputSource ins = new org.xml.sax.InputSource(isr);
				ins = new org.xml.sax.InputSource(isr);
				parser.parse(ins,this._xmlHandler);
			}catch(SAXException se_){
				// 強制終了エラーかどうか検証する。
				if (se_.getMessage().equals("RICOH!SYDNEY!") != true){
					try{
						FileInputStream isr = new FileInputStream(f);
						org.xml.sax.InputSource ins = new org.xml.sax.InputSource(isr);
						isr.skip(3);
						ins = new org.xml.sax.InputSource(isr);
						parser.parse(ins,this._xmlHandler);
					}catch(SAXException se_2){
						// 強制終了エラーかどうか検証する。
						if (se_2.getMessage().equals("RICOH!SYDNEY!") != true){
							throw new SAXException(se_2);
						}
					}
				}
			}
			this._recordCount = this._xmlHandler.getRecordCount();

		} else {

			// 別ファイル情報の取得
			this._xmlHandler.setCountStatus(false);
			this._xmlHandler.setStock(true);
//			parser.parse(f, this._xmlHandler);
			// 2006.07.20
			// UTF-8ファイルの先頭に EF BB BF が入ってる場合の対応
			try{
				FileInputStream isr = new FileInputStream(f);
				org.xml.sax.InputSource ins = new org.xml.sax.InputSource(isr);
				ins = new org.xml.sax.InputSource(isr);
				parser.parse(ins,this._xmlHandler);
			}catch(SAXException se_){
				// 強制終了エラーかどうか検証する。
				if(se_.getMessage().equals("RICOH!SYDNEY!") != true){
					try{
						FileInputStream isr = new FileInputStream(f);
						org.xml.sax.InputSource ins = new org.xml.sax.InputSource(isr);
						isr.skip(3);
						ins = new org.xml.sax.InputSource(isr);
						parser.parse(ins,this._xmlHandler);
					}catch(SAXException se_2){
						// 強制終了エラーかどうか検証する。
						if (se_2.getMessage().equals("RICOH!SYDNEY!") != true){
							throw new SAXException(se_2);
						}
					}
				}
			}
			
			// 別ファイル情報を解析してレコード数を取得する
			LoadXMLHandler	xmlHandler = null;
			LoadItem		item = null;

			for (int i = 0; i < this._xmlHandler.getItemSize(); i++) {

				item = this._xmlHandler.getItem(i);
				String	fileName = item.getValue();
				if (fileName != null && fileName.length() > 0) {
					this._dataList.add(fileName);
					f = FileUtil.getFileHandler(this._loadDir, fileName);
					xmlHandler = new LoadXMLHandler();
					xmlHandler.open("/*/*");				// タグフィルターは深さが２以上なら何でも構わない。
					xmlHandler.setCountStatus(true);		// レコード数のみ取得する。反映処理は行わない。
//					parser.parse(f, xmlHandler);			// SAX処理。
					// 2006.07.20
					// UTF-8ファイルの先頭に EF BB BF が入ってる場合の対応
					try{
						FileInputStream isr = new FileInputStream(f);
						org.xml.sax.InputSource ins = new org.xml.sax.InputSource(isr);
						ins = new org.xml.sax.InputSource(isr);
						parser.parse(ins, xmlHandler);
					}catch(SAXException se_){
						// 強制終了エラーかどうか検証する。
						if (se_.getMessage().equals("RICOH!SYDNEY!") != true){
							try{
								FileInputStream isr = new FileInputStream(f);
								org.xml.sax.InputSource ins = new org.xml.sax.InputSource(isr);
								isr.skip(3);
								ins = new org.xml.sax.InputSource(isr);
								parser.parse(ins, xmlHandler);
							}catch(SAXException se_2){
								// 強制終了エラーかどうか検証する。
								if (se_2.getMessage().equals("RICOH!SYDNEY!") != true){
									throw new SAXException(se_2);
								}
							}
						}
					}

					this._recordCount = this._recordCount + xmlHandler.getRecordCount();
				}
			}
		}

		Log.trace(Res.get("TRACE_END"));
	}

	private String[] getHeader(String propName_)
	{
		LoadItem	item = null;
		List<String>		list = new ArrayList<String>();

		for (int i = 0; i < this._xmlHandler.getItemSize(); i++) {
			item = this._xmlHandler.getItem(i);
			String	name = item.getName();
			if (name.equalsIgnoreCase(propName_)) {
				list.add(item.getValue());
			}
		}

		return (String[])list.toArray(new String[list.size()]);
	}

	public void run()
	{
		Log.trace(Res.get("TRACE_START"));

		try {

			SAXParser	threadParser = this._parserFactory.newSAXParser();
//			threadParser.parse(this._threadF, this._threadHandler);
			// 2006.07.20
			// UTF-8ファイルの先頭に EF BB BF が入ってる場合の対応
			FileInputStream isr = new FileInputStream(this._threadF);
			org.xml.sax.InputSource ins = new org.xml.sax.InputSource(isr);
			ins = new org.xml.sax.InputSource(isr);
			threadParser.parse(ins, this._threadHandler);

		} catch (Exception e_) {

			// 強制終了エラーかどうか検証する。
//			if (e_.getMessage().equals("RICOH!SYDNEY!") != true) Log.exception(e_.toString());
			if (e_.getMessage().equals("RICOH!SYDNEY!") != true){
				try{
					SAXParser	threadParser = this._parserFactory.newSAXParser();
					FileInputStream isr = new FileInputStream(this._threadF);
					org.xml.sax.InputSource ins = new org.xml.sax.InputSource(isr);
					isr.skip(3);
					ins = new org.xml.sax.InputSource(isr);
					threadParser.parse(ins, this._threadHandler);
				}catch(Exception e_2){
					// 強制終了エラーかどうか検証する。
					if (e_2.getMessage().equals("RICOH!SYDNEY!") != true) Log.exception(e_.toString());
				}
			}
		}

		Log.trace(Res.get("TRACE_END"));
	}
}

//
// Copyright (c) 2005, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
