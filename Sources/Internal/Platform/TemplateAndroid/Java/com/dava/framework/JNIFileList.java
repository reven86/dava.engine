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
		ArrayList<FileListDescriptor> listDescriptors = new ArrayList<FileListDescriptor>();
		
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
				if (path.length() > 0 && path.charAt(path.length() - 1) == '/') {
					path = path.substring(0, path.length() - 1);
				}
				String pathPrefix = !path.isEmpty() ? path + File.separator : "";
				
				String [] list = assetManager.list(path);
				
				
				for (String fileName : list) {
					String filePath = pathPrefix + fileName;
					
					try {
						InputStream inputStream = assetManager.open(filePath);
						listDescriptors.add(new FileListDescriptor(fileName, false, inputStream.available()));
						inputStream.close();
					} catch (IOException e) {
						listDescriptors.add(new FileListDescriptor(fileName, true, 0));
					}
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		
		return listDescriptors.toArray();
	}
}
