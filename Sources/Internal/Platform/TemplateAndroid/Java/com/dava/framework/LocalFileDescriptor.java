package com.dava.framework;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.IOException;

import android.app.Activity;
import android.content.res.AssetFileDescriptor;

public class LocalFileDescriptor {

	private FileDescriptor descriptor = null;
	private FileInputStream inputStream = null;
	private long startOffset = 0;
	private long length = 0;

	public LocalFileDescriptor(String path) throws IOException {
		if (IsLocal(path)) { // path to global file system
			File file = new File(path);
			inputStream = new FileInputStream(file);
			descriptor = inputStream.getFD();
			startOffset = 0;
			length = file.length();
		} else {
			Activity activity = JNIActivity.GetActivity();
			AssetFileDescriptor assetFileDescriptor = activity.getAssets().openFd(path);
			descriptor = assetFileDescriptor.getFileDescriptor();
			startOffset = assetFileDescriptor.getStartOffset();
			length = assetFileDescriptor.getLength();
		}
	}
	
	public FileDescriptor GetDescriptor() {
		return descriptor;
	}
	
	public long GetStartOffset() {
		return startOffset;
	}
	
	public long GetLength() {
		return length;
	}
	
	private static boolean IsLocal(String path) {
		return path.startsWith("/");
	}
}
