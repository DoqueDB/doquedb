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
package jp.co.ricoh.sydney.admin.util;

import java.io.File;

public class FileUtil {
	public FileUtil() {
		
	}
	
	/**
	 * 指定パスと相対パスからファイルハンドルを取得する。
	 * @param rootPath 指定パス 
	 * @param file 相対パス
	 * @return ファイルハンドル
	 */
	public static File getFileHandler(String rootPath,String file) {
	    File fp = null;

	    fp = new File(file);						// 絶対パスであることが前提
	    if( fp.exists() == false ){				// でも相対パスで指定された場合
	        fp = new File(rootPath,file);			// 指定パス+相対パス
	        if( fp.exists() == false ){			// 実は言うと絶対パスだった場合。
	        	String name = fp.getName();			// ファイル名のみ取得する。
	            fp = new File(rootPath,name);		// 指定パス+相対パス（これ以上はエラーとする）
	        }
	    }
	    
	    return fp;
	}

}

//
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
