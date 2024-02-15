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

package jp.co.ricoh.sydney.admin.util.xml;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.TransformerFactoryConfigurationError;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.DOMException;
import org.w3c.dom.Document;
import org.w3c.dom.Element;

public class XMLUtil {
	private Document document = null;
	private Element root = null;
	
	/**
	 * @param rootName
	 * @throws DOMException
	 * @throws ParserConfigurationException
	 * @throws FactoryConfigurationError
	 */
	public XMLUtil(String rootName) throws DOMException, ParserConfigurationException, FactoryConfigurationError{
        this.document = DocumentBuilderFactory.newInstance()
        .newDocumentBuilder()
        .getDOMImplementation()
        .createDocument("", rootName, null);
        
        if( this.document != null ){
        	this.root = this.document.getDocumentElement();
        }
        
	}
	
	/**
	 * ルートのElementを返します。
	 * @return ルートのElement
	 */
	public Element getRootElement() {
		return this.root;
	}
	
	/**
	 * 子ノードのElementを返します。
	 * @param name ノード名
	 * @return Element 子ノードのElementを返す。
	 */
	public Element getElementNode(String name) {
		return this.document.createElement(name);
	}
	
	/**
	 * 指定ノードの属性を与えます。
	 * @param ele 属性を与えるElement
	 * @param name 属性名
	 * @param value 属性値
	 */
	public void setAttribute(Element ele,String name,String value) {
		ele.setAttribute(name,value);
	}
	
	/**
	 * ノードに内容を設定します。
	 * @param ele 指定ノード
	 * @param value 内容
	 */
	public void setTextNode(Element ele,String value) {
		ele.appendChild(this.document.createTextNode(value));
	}
	
	/**
	 * 指定ノード以下にコメントノードを作成します。
	 * @param ele
	 * @param comment
	 */
	public void setComment(Element ele,String comment) {
		ele.appendChild(this.document.createComment(comment));
	}

	/**
	 * 子ノードを追加します。
	 * @param pre 親ノード
	 * @param sub 設定ノード
	 */
	public void addElement(Element pre,Element sub) {
		pre.appendChild(sub);
	}
	
	/**
	 * 指定ファイル名にXMLドキュメントを書き込みます。
	 * @param fileName 書き込みファイル名
	 * @throws TransformerFactoryConfigurationError
	 * @throws FileNotFoundException
	 * @throws TransformerException
	 */
	public void write(String fileName,String code) throws TransformerFactoryConfigurationError, FileNotFoundException, TransformerException, java.io.IOException {
		if( this.document == null ){return;}	// what's throws ??
		
        Transformer transformer = TransformerFactory.newInstance().newTransformer();
        transformer.setOutputProperty("indent","yes");
        if( code != null ){
        	transformer.setOutputProperty("encoding", code);
        }
        DOMSource source = new DOMSource(this.document);
        FileOutputStream os = new FileOutputStream(new File(fileName));
        StreamResult result = new StreamResult(os);
        transformer.transform(source, result);
		os.close();
	}
}

//
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
