// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LoadXMLHander.java -- 
// 
// Copyright (c) 2005, 2008, 2023 Ricoh Company, Ltd.
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

import jp.co.ricoh.sydney.admin.util.Res;
import jp.co.ricoh.sydney.admin.util.log.Log;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

public class LoadXMLHandler extends DefaultHandler
{
	private int					_recordCount = 0;			// XML要素数

	private java.util.ArrayList<LoadItem>	_itemList = null;			// 指定タグ要素数

	private java.util.Stack<String>		_tagStack = null;			// 要素名のスタックバッファー
	private String[]			_urlList = null;			// 指定フィルターの解析用バッファー（/sydney/headerならsydneyとheaderと分ける）

	private boolean				_filterStatus = false;		// 指定要素フラグ
	private boolean				_countStatus = false;		// カウントフラグ（指定タグ要素情報を取得しないかどうか）
	private boolean				_forceStatus = false;		// 中止フラグ（trueにするとSAXExceptionで強制終了させる）

	private boolean				_threadStatus = false;		// スレッド対応にするかどうか。(trueにすると中止処理が可能になる)
	private boolean				_threadStopStatus = true;	// loop処理をさせる為のフラグ　

	private boolean				_endStatus = false;			// 終了フラグ
	private boolean				_startStatus = false;		// 開始フラグ
	private boolean				_itemSetStatus = false;		// 指定タグ要素情報を取得したどうか

	private String				_tagName = null; 			// 現在のフィルターからのタグ名
	private String				_textValue = null;			// 要素値

	private boolean				_stock = false;

	public LoadXMLHandler()
	{
		this._itemList = new java.util.ArrayList<LoadItem>();
	}

	// 要素数を返す（フィルターによる）
	public int getRecordCount()
	{
		return this._recordCount;
	}

	// XML要素リスト

	public int getItemSize()
	{
		return this._itemList.size();
	}

	public LoadItem getItem(int i_)
	{
		return (LoadItem)this._itemList.get(i_);
	}

	public java.util.ArrayList<LoadItem> getItemList()
	{
		return this._itemList;
	}

	// スレッドとして解析処理を行うか（1要素ブロック単位で処理する場合に有効。大量データなど）
	public void setThreadStatus(boolean	threadStatus_)
	{
		this._threadStatus = threadStatus_;
	}

	// カウンターのみのフラグ
	public void setCountStatus(boolean	countStatus_)
	{
		this._countStatus = countStatus_;
	}

	// 強制終了させるかどうかのフラグ
	public void setForceStatus(boolean	forceStatus_)
	{
		this._forceStatus = forceStatus_;
		this._threadStopStatus = false;
	}

	public void setStock(boolean	stock_)
	{
		this._stock = stock_;
	}

	// 要素名
	public String getTagName()
	{
		return this._tagName;
	}

	public void open(String	url_)
	{
		// URLの解析
		url_ = url_.substring(1);
		this._urlList = url_.split("/");

		this._countStatus = false;
		this._forceStatus = false;
		this._threadStatus = false;
		this._threadStopStatus = true;

		this._tagStack = new java.util.Stack<String>();
		this._filterStatus = false;

		this._startStatus = false;
		this._endStatus = false;
		this._itemSetStatus = false;

		this._stock = false;
	}

	public boolean read()
		throws InterruptedException
	{
		// 終了中かどうか
		if (this._endStatus) return false;

		// 開始されていない
		if (this._startStatus == false) {
			for (;;) {
				Thread.sleep(5);
				if (this._startStatus) break;

				// 終了中かどうか(念の為)
				if (this._endStatus) return false;
			}
		}

		// ループをクリア
		this._threadStopStatus = false;

		// アイテム導入まで待機
		for (;;) {
			Thread.sleep(5);
			if (this._itemSetStatus) break;

			// 終了中かどうか(タイミングにより)
			if (this._endStatus) return false;
		}
		this._itemSetStatus = false;

		return true;
	}

	/**
	 * ドキュメント開始時
	 */
    public void startDocument()
	{
		this._textValue = null;
		this._recordCount = 0;
		// スペックファイルからのファイルリストをクリアする。
		this._itemList.clear();

		this._startStatus = true;
	}

	// qName_ は使っていない？？？
	private boolean checkUrl(String	qName_)
	{
		// 指定フィルターとタグ名が一致するかどうかのチェック
		int count = this._tagStack.size();
		if (this._urlList.length != count) {
			return false;
		}

		for(int i=0;i<this._tagStack.size();i++){
			String value = (String)this._tagStack.get(i);
			String checkvalue = this._urlList[i];
			if( checkvalue.equals("*") != true ){			// *の場合は任意。tagのレベルが一致していればいい
				if( value.equals(checkvalue) != true ){		// タグ名チェック
					return false;
				}
			}
		}

		return true;
	}

    /**
     * 要素の開始タグ読み込み時
     * @param uri
     * @param localName
     * @param qName
     * @param attributes
     * @throws SAXException
     */
    public void startElement(String uri,
            	String localName,
            	String qName,
            	Attributes attributes) {

    	// スタックにタグ名をプッシュする。
    	this._tagStack.push(qName);

    	// フィルタースタート
    	if( this._filterStatus == false ){
	    	if( checkUrl(qName) == true ){
    			// スレッド処理の場合のみ強制停止
		    	// 呼び出し側から停止処理を解除する
	    		loop();
	    		// タグ名取得
	    		this._tagName = qName;
	    		// 指定要素の開始フラグ
	    		this._filterStatus = true;
	    		// 項目データのクリア
	    		if( this._countStatus == false ){
		    		if (this._stock == false) this._itemList.clear();
	    		}
	    		// 要素取得フラグ
	    		this._itemSetStatus = false;
	    		// スレッド処理の場合強制停止フラグをオン
//	    		if( this._threadStatus == true ){
//	    			this._threadStopStatus = true;
//	    		}
	    	}
    	}
    	this._textValue = null;
    }

    private void loop() {
    	// スレッド処理の場合
    	if( this._threadStatus == true ){
    		for(;;){
				try {
					if( this._threadStopStatus == false ){
						break;
					}
					Thread.sleep(10);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
    		}
    	}
    }

    /**
     * テキストデータ読み込み時
     * @param ch
     * @param offset
     * @param length
     */
    public void characters(
            	char[] ch,
            	int offset,
            	int length) {

    	if( this._filterStatus != true ){return;}		// 要素外である
    	if( this._countStatus == true ){return;}		// カウンターを取得するのみ

 		//String body = new String(ch,offset,length).trim();
		String	body = new String(ch,offset,length);
		if( body.length() > 0 ){
//			System.out.println("データ"+body);

//		    Log.trace("データ長:"+length+",OFFSET:"+offset+",データ"+body);
			if (this._textValue == null) {
				this._textValue = body;
			} else {
				this._textValue = this._textValue + body;
			}
		}
    }

    /**
     * 要素の終了タグ読み込み時
     * @param uri
     * @param localName
     * @param qName
     * @throws SAXException
     * @throws SAXException
     */
    public void endElement(
            String uri,
            String localName,
            String qName) throws SAXException {

    	if( this._filterStatus == true ){
    		if( checkUrl(qName) == true ){
    			if( this._forceStatus == true ){
    				this._endStatus = true;
    				throw new SAXException("RICOH!SYDNEY!");
    			}
		    	// スレッド停止処理の設定
		    	this._threadStopStatus = true;
    			// 指定要素のステータスを初期化
		    	this._filterStatus = false;
		    	// 項目データをセットした。
		    	this._itemSetStatus = true;
	    		// カウンタ-加算
		        this._recordCount++;
		        if (this._countStatus == false) {
		        	LoadItem	item = new LoadItem();
		        	item.setName(qName);
		        	item.setValue((this._textValue == null) ? "" : this._textValue.trim());
		        	this._itemList.add(item);
		        	this._textValue = null;
		        }
    		}else{
    			if( this._countStatus == false ){
			    	LoadItem item = new LoadItem();
					item.setName(qName);
					if( this._textValue == null ){
						item.setValue("");
					}else{
						item.setValue(this._textValue);
					}
			    	this._itemList.add(item);
					this._textValue = null;
    			}
    		}
    	}

    	// 現在のタグ名をスタックから破棄
    	this._tagStack.pop();

    }

    /**
     * ドキュメント終了時
     */
    public void endDocument() {
        Log.trace(Res.get("TRACE_START"));

//        System.out.println("endDocument");

        this._endStatus = true;

        Log.trace(Res.get("TRACE_END"));
    }

}

//
// Copyright (c) 2005, 2008, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
