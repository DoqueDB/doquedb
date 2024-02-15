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

package jp.co.ricoh.sydney.admin.util.zip;

import java.io.*;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.zip.*;

public class Zip {
    private final int EOF = -1;
    private final String SEPA = System.getProperty("file.separator");

    // 2006.03.31	zipファイルの中身が空だった場合にエラーが発生していた件に対応
    private FileOutputStream fos = null;

    private ZipOutputStream zos = null;
    private ArrayList<ZipFileEntry> allFiles = null;

    private String rootPath = null;

    private long fileSize = 0;

    private int	numEntries = 0;

    public Zip() {
        this.allFiles = new ArrayList<ZipFileEntry>();
        this.fileSize = 0;
        this.numEntries = 0;
    }

	public void open(String zipFileName) throws FileNotFoundException {
		// ZIPファイル

	    // 2006.03.31	zipファイルの中身が空だった場合にエラーが発生していた件に対応
		this.fos = new FileOutputStream(new File(zipFileName));
//		this.zos = new ZipOutputStream(new FileOutputStream(new File(zipFileName)));
		this.zos = new ZipOutputStream(this.fos);
		this.fileSize = 0;
		this.numEntries = 0;
	}

    public void close() throws IOException {
        if( this.zos != null ){
		    if (this.numEntries > 0){
		    	this.zos.close();
		        // 2006.03.31	zipファイルの中身が空だった場合にエラーが発生していた件に対応
		    	this.fos.close();
		    }else{
		        // 2006.03.31	zipファイルの中身が空だった場合にエラーが発生していた件に対応
		    	this.fos.close();
		    }
		    this.zos = null;
        }
    }

    /**
     * 指定ファイルを圧縮する。
     * @param zipFile  圧縮ファイル名
     * @param compFile 圧縮対象ファイルまたはディレクトリ
     * @return true:成功 false:失敗
     * @throws FileNotFoundException
     * @throws ZipException
     * @throws IOException
     */
    public boolean compress(String zipFile,String compFile) throws FileNotFoundException, ZipException, IOException {
        boolean ret = false;

        try {
            this.fileSize = 0;
	        open(zipFile);
	        ret = compress(compFile);
        } finally {
            close();
        }
        return ret;
    }

	public boolean compress(String zipFile,String compFile, int nCase_) throws FileNotFoundException, ZipException, IOException {
		boolean ret = false;

		try {
			this.fileSize = 0;
			open(zipFile);
			ret = compress(compFile, nCase_);
		} finally {
			close();
		}
		return ret;
	}

    /**
     * 指定ファイルを圧縮する。
     * @param zipFile  圧縮ファイル名
     * @param list     圧縮対象ファイル集
     * @return true:成功 false:失敗
     * @throws FileNotFoundException
     * @throws ZipException
     * @throws IOException
     */
	public boolean compress(String zipFile,ArrayList<String> list) throws FileNotFoundException, ZipException, IOException {
		boolean ret = false;
		File file = null;

        try {
            this.fileSize = 0;
	        open(zipFile);

            for(int i=0;i<list.size();i++){
                file = new File((String)list.get(i));
                this.rootPath = file.getPath();
        		if( this.rootPath.endsWith(this.SEPA) != true ){
        		    this.rootPath = this.rootPath + this.SEPA;
        		}
                if(!file.isFile()) {
					continue;
				}else{
					//System.out.println("file="+file);
					addTargetFile(file,file.getName());
				}
            }
            ret = true;
        } finally {
            close();
        }
        return ret;
    }

    public boolean compress(String compFile) throws FileNotFoundException, ZipException, IOException {
        boolean ret = false;
        File file = null;

        try {
            this.fileSize = 0;
            if( compFile == null ){
                return ret;
            }
            // 圧縮されるファイル
	        file = new File(compFile);
            this.rootPath = file.getPath();
    		if( this.rootPath.endsWith(this.SEPA) != true ){
    		    this.rootPath = this.rootPath + this.SEPA;
    		}
	        if(file.isFile()) {
	            addTargetFile(file,file.getName());
	        }else{
		        recursive("",file,0);
		        ZipFileEntry entry = null;
	            for(int i=0;i<this.allFiles.size();i++){
	                entry = (ZipFileEntry)this.allFiles.get(i);
	                file = new File(entry.getAbsoluteFile());
	                addTargetFile(file,entry.getPathFile());
	            }
	        }
	        ret = true;
        } finally {
            // 取得ファイルリストのクリア
            this.allFiles.clear();
        }
        return ret;
    }

    public boolean compress(String compFile, int nCase_) throws FileNotFoundException, ZipException, IOException {
        boolean ret = false;
        File file = null;

        try {
            this.fileSize = 0;
            if( compFile == null ){
                return ret;
            }
            // 圧縮されるファイル
	        file = new File(compFile);
            this.rootPath = file.getPath();
    		if( this.rootPath.endsWith(this.SEPA) != true ){
    		    this.rootPath = this.rootPath + this.SEPA;
    		}
	        if(file.isFile()) {
	            addTargetFile(file,file.getName());
	        }else{
		        recursive("",file, 0, nCase_);
		        ZipFileEntry entry = null;
	            for(int i=0;i<this.allFiles.size();i++){
	                entry = (ZipFileEntry)this.allFiles.get(i);
	                file = new File(entry.getAbsoluteFile());
	                addTargetFile(file,entry.getPathFile());
	            }
	        }
	        ret = true;
        } finally {
            // 取得ファイルリストのクリア
            this.allFiles.clear();
        }
        return ret;
    }


    public ArrayList<String> extractEntry(String compFile) throws IOException {
        ArrayList<String> list = new ArrayList<String>();
        ZipInputStream zis = null;
        ZipEntry zent = null;

        try {
	        zis = new ZipInputStream(new FileInputStream(compFile));
	        while ((zent=zis.getNextEntry()) != null) {
	            list.add(zent.getName());
	        }
        } finally {
            if( zis != null ){
                zis.closeEntry();
                zis.close();
            }
        }
        return list;

    }

    public void extract(String compFile,String outfile,String entryFile) throws IOException {
        String workDir = outfile;
		String s = System.getProperty("file.separator");
		if( outfile.endsWith(s) != true ){
		    workDir = workDir + s;
		}

        ZipFile zf = new ZipFile(compFile);
        Enumeration<? extends ZipEntry> i = zf.entries();
        while( i.hasMoreElements() ) {
            ZipEntry target = (ZipEntry)i.nextElement();
            if( entryFile == null ){
                extractFile(zf,target,workDir);
                continue;
            }
            if( entryFile.equals(target.getName()) == true ){
                extractFile(zf,target,workDir);
            }
        }
    }

    public void extractFile(ZipFile zf,ZipEntry target,String outfile) throws ZipException,IOException {
        FileOutputStream fos = null;
        BufferedOutputStream bos = null;

        try {
            File file = new File(outfile+target.getName());
            if( target.isDirectory() ) {
                file.mkdirs();
            }else {
                InputStream is = zf.getInputStream( target );
                BufferedInputStream bis = new BufferedInputStream( is );
                if( file.getParent() != null ){
	                File dir = new File(file.getParent());
	                dir.mkdirs();
                }
                fos = new FileOutputStream( file );
                bos = new BufferedOutputStream( fos );

				//int c;
				//while( (c = bis.read()) != EOF ) {
				//	bos.write( (byte)c );
				//}
				byte[]	buff = new byte[4096];
				int	len;
				while ((len = bis.read(buff, 0, 4096)) > 0) {
					bos.write(buff, 0, len);
				}
            }
        } finally {
            if( bos != null ){
                bos.close();
            }
            if( fos != null ){
                fos.close();
            }
        }
    }

	private void recursive(String path, File dir, int indent) {
		String[] contents = dir.list();

		if (contents == null) return;

		for (int i=0; i < contents.length; i++) {
			//サブパスのオブジェクト化
			File sdir = new File(dir, contents[i]);
			//ディレクトリならば再帰的にリスト
			if (sdir.isDirectory()) {
			    String subPath = path + contents[i] + this.SEPA;
				recursive(subPath, sdir, indent+1);
				continue;
			}

			String absolutePath = this.rootPath + path;
			ZipFileEntry entry = new ZipFileEntry();
			entry.setPathLevel(indent);
			entry.setFile(contents[i]);
			entry.setPath(path);
			entry.setPathFile(path + contents[i]);
			entry.setAbsolutePath(absolutePath);
			entry.setAbsoluteFile(absolutePath + contents[i]);
	        this.allFiles.add(entry);
		}
	}

	private void recursive(String path, File dir, int indent, int nCase_) {
		// nCase	0:DataPath
		//			1:LogPath
		//			2:SystemPath
		String[] contents = dir.list();

		if (contents == null) return;

		for (int i=0; i < contents.length; i++) {
			//サブパスのオブジェクト化
			File sdir = new File(dir, contents[i]);
			//ディレクトリならば再帰的にリスト
			String sWorkPath = sdir.getName().toUpperCase();
			if (sdir.isDirectory()) {

				// データパス時にシステムパスをスルーする
				if(indent==0 && nCase_==0 && (sWorkPath.indexOf("OBJECTID")!=-1 || sWorkPath.indexOf("SCHEMA")!=-1))continue;

				// ログパス時にデータ＆システムパスをスルーする
				if(indent==0 && nCase_==1)continue;

				// システムパス時にデータパスをスルーする
				if(indent==0 && nCase_==2 && (sWorkPath.indexOf("OBJECTID")==-1 && sWorkPath.indexOf("SCHEMA")==-1))continue;

			    String subPath = path + contents[i] + this.SEPA;
				recursive(subPath, sdir, indent+1, nCase_);
				continue;
			}else{

				// データパス時にログファイルをスルーする
				if(indent==0 && nCase_==0 && (sWorkPath.indexOf("LOGICALLOG.SYD")!=-1 || sWorkPath.indexOf("TIMESTAMP.SYD")!=-1))continue;

				// ログパス時に他のファイルをスルーする
				if(indent==0 && nCase_==1 && (sWorkPath.indexOf("LOGICALLOG.SYD")==-1 && sWorkPath.indexOf("TIMESTAMP.SYD")==-1))continue;

				// システムパス時に直下ファイルをスルーする
				if(indent==0 && nCase_==2)continue;
			}

			String absolutePath = this.rootPath + path;
			ZipFileEntry entry = new ZipFileEntry();
			entry.setPathLevel(indent);
			entry.setFile(contents[i]);
			entry.setPath(path);
			entry.setPathFile(path + contents[i]);
			entry.setAbsolutePath(absolutePath);
			entry.setAbsoluteFile(absolutePath + contents[i]);
	        this.allFiles.add(entry);
		}
	}

    private void addTargetFile(File file,String name) throws FileNotFoundException,ZipException,IOException {
        BufferedInputStream bis = null;
        ZipEntry target = null;
        int count;

        try {
            this.fileSize = this.fileSize + file.length();
            bis = new BufferedInputStream(new FileInputStream(file));
            target = new ZipEntry(name);
            this.zos.putNextEntry(target);
            this.numEntries++;

            byte buf[] = new byte[1024];
            while( (count=bis.read(buf,0,1024)) != EOF ) {
                this.zos.write(buf,0,count);
            }

            this.zos.closeEntry();
        } finally {
    	    if( bis != null ){
    	        bis.close();
    	    }
		}
    }

    public long getFileSize() {
        return this.fileSize;
    }

}

//
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
