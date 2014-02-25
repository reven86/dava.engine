package com.dava.framework;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import android.content.res.AssetManager;

public class JNIFileList {
	
	public static class FileListDescriptor {
		public String name;
		public boolean isDirectory;
		public long size;
		
		public FileListDescriptor(String name, boolean isDirectory, long size) {
			this.name = name;
			this.isDirectory = isDirectory;
			this.size = size;
		}
	}
	
	public static Object[] GetFileList(String path) {
		ArrayList<Object> listDescriptors = new ArrayList<Object>();
		
		if (LocalFileDescriptor.IsLocal(path)) {
			File filePath = new File(path);
			String [] list = filePath.list();
			if (list != null) {
				for (String fileName : list) {
					File file = new File(path + File.separator + fileName);
					
					FileListDescriptor descriptor;
					descriptor = new FileListDescriptor(fileName, file.isDirectory(), file.length());
					listDescriptors.add(descriptor);
				}
			}
		} else {
			AssetManager assetManager = JNIActivity.GetActivity().getAssets();
			try {
				if (path.length() > 0 && path.charAt(path.length() - 1) == '/')
					path = path.substring(0, path.length() - 1);
				
				String [] list = assetManager.list(path);
				for (String fileName : list) {
					String filePath = !path.isEmpty() ? (path + File.separator + fileName) : fileName;
					
					boolean isDir = true;
					long size = 0;
					try {
						InputStream inputStream = (AssetManager.AssetInputStream) assetManager.open(filePath);
						if (inputStream != null) {
							isDir = false;
							size = inputStream.available();
							inputStream.close();
						}
					} catch (IOException e) {
					}
					
					FileListDescriptor descriptor;
					descriptor = new FileListDescriptor(fileName, isDir, size);
					listDescriptors.add(descriptor);
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		
		return listDescriptors.toArray();
	}
}
