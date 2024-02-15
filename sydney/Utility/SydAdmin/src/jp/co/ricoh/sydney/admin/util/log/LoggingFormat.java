// 
// Copyright (c) 2004, 2023 Ricoh Company, Ltd.
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

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.StringTokenizer;
import java.util.logging.Formatter;
import java.util.logging.LogRecord;

public class LoggingFormat extends Formatter {
	private final String KAIGYO = System.getProperty("line.separator");
	
	public synchronized String format(LogRecord record) {
		StringBuffer buf = new StringBuffer();
		// 日時を設定
		Date date = new Date();
		date.setTime(record.getMillis());
		SimpleDateFormat formatter = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");
		buf.append(formatter.format(date)+",");

		// ソース情報の取得
		int i=0;
		StackTraceElement[] ste = new Throwable().getStackTrace();
		for(i=0;i<ste.length;i++){
		    String name = getClassName(ste[i].getClassName());
		    //System.out.println("class name="+name);
	        if( name.equalsIgnoreCase("Log") == true ){
	            i++;
	            break;
	        }
		}

		// クラス情報を設定
		String cname = getClassName(ste[i].getClassName());
		//System.out.println("class name="+cname);
		buf.append(cname+"::"+ste[i].getMethodName());
/*		buf.append(record.getSourceClassName());
		buf.append(" ");
		buf.append(record.getSourceMethodName());
		buf.append(" : ");*/

		// ソースの行数を取得
		buf.append("("+ste[i].getLineNumber()+"),");
		
//		System.out.println("class="+ste[i].getClassName()+"::"+ste[i].getMethodName()+"("+ste[i].getLineNumber()+")");

		// レベルを設定
//		buf.append(record.getLevel().getLocalizedName()+",");
		buf.append(record.getLevel().getName()+",");

		// メッセージを設定（改行つき）
		buf.append(record.getMessage());
		buf.append(KAIGYO);

		return buf.toString();
	}
	
	private String getClassName(String str) {
	    String ret = null;
	    
	    StringTokenizer st = new StringTokenizer(str, ".");
	    while (st.hasMoreTokens()) {
	        ret = st.nextToken();
	    }

	    return ret;
	}
}

//
// Copyright (c) 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
