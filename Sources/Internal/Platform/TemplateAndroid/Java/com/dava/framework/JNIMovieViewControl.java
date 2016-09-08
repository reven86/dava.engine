package com.dava.framework;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.HashMap;
import java.util.Map;

import android.content.res.AssetManager;
import android.graphics.Color;
import android.media.MediaPlayer;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.RelativeLayout;
import android.widget.VideoView;
import android.util.Log;

// if you interesting why all so complicated:
// http://stackoverflow.com/questions/8269353/how-to-load-videos-from-assets-folder-to-play-them-with-videoview
// so we just copy file from assets to doc directory and play it from here
// only if needed

public class JNIMovieViewControl {
	
	private final static int scalingModeNone = 0;
	private final static int scalingModeAspectFit = 1;
	private final static int scalingModeAspectFill = 2;
	private final static int scalingModeFill = 3;

	private static class MovieControl {
		public MovieControl() {
		}
		
		public RelativeLayout layout = null;
		public VideoView videoView = null;
		public MediaPlayer player = null;
		public String extPath = null;
		public int scalingMode = scalingModeNone;
		public int pausePosition = 0;
		public boolean logicIsPlaying = false;
		
		// HACK!!! If client call several times SetRect form Game thread
		// every call set runnable on UI thread with its own view params
		// I need store params here to faster update
		// view params, because on some android devices such lenovo k900
		// after first view.setLayoutParams next call to video view does nothing
		public int leftMargin = 0;
		public int topMargin = 0;
		public int width = 0;
		public int height = 0;
	}

	private static Map<Integer, MovieControl> controls = new HashMap<Integer, MovieControl>();

	public static void Initialize(final int id, final float x, final float y,
			final float dx, final float dy) {

		//Log.e(JNIConst.LOG_TAG, "LQ initialize " + x + " " + y + " " + dx + " " + dy);
		final JNIActivity activity = JNIActivity.GetActivity();

		if(controls.containsKey(id)) {
			throw new RuntimeException("second time initialize movie control with id: " + id);
		} else {
			final MovieControl control = new MovieControl();
			
			controls.put(id, control);
			activity.runOnUiThread(new Runnable() {
				@Override
				public void run() {
					VideoView videoView = new VideoView(activity);
					
					videoView.setBackgroundColor(Color.BLACK);
					videoView.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
						@Override
						public void onCompletion(MediaPlayer mp) {
							control.logicIsPlaying = false;
						}
					});
					videoView.setOnPreparedListener(new MediaPlayer.OnPreparedListener() {
						@Override
						public void onPrepared(MediaPlayer mediaPlayer) {
							
							control.player = mediaPlayer; // save player every time
							int layoutWidth = control.width; //control.layout.getWidth();
							int layoutHeight = control.height; //control.layout.getHeight();
							int videoWidth = mediaPlayer.getVideoWidth();
							int videoHeight = mediaPlayer.getVideoHeight();

							//Log.e(JNIConst.LOG_TAG, "LQ on_prepared ui thread start control size: " + layoutWidth +
							//		" " + layoutHeight + " video size: " + videoWidth + " " + videoHeight);
							
							if (videoHeight == 0 || videoWidth == 0) {
								return;
							}

							// update scaling
							RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(0, 0);
							switch (control.scalingMode) {
					/*0*/	case scalingModeNone: {
								params.width = Math.max(layoutWidth, videoWidth);
								params.height = Math.max(layoutHeight, videoHeight);
								params.leftMargin = (layoutWidth - params.width) / 2;
								params.topMargin = (layoutHeight - params.height) / 2;
							}
								break;
					/*1*/   case scalingModeAspectFit: {
								float xFactor = videoWidth / (float) layoutWidth;
								float yFactor = videoHeight / (float) layoutHeight;
								if (xFactor > yFactor) {
									params.width = layoutWidth;
									params.height = videoHeight * layoutWidth / videoWidth;
									params.topMargin = (layoutHeight - params.height) / 2;
									params.leftMargin = 0;
								} else {
									params.height = layoutHeight;
									params.width = videoWidth * layoutHeight / videoHeight;
									params.leftMargin = (layoutWidth - params.width) / 2;
									params.topMargin = 0;
								}
							}
								break;
					/*2*/	case scalingModeAspectFill: {
								float xFactor = videoWidth / (float) layoutWidth;
								float yFactor = videoHeight / (float) layoutHeight;
								if (xFactor > yFactor) {
									params.height = layoutHeight;
									params.width = videoWidth * layoutHeight / videoHeight;
									params.leftMargin = params.rightMargin = (layoutWidth - params.width) / 2;
									params.topMargin = params.bottomMargin = 0;
								} else {
									params.width = layoutWidth;
									params.height = videoHeight * layoutWidth / videoWidth;
									params.leftMargin = params.rightMargin = 0;
									params.topMargin = params.bottomMargin = (layoutHeight - params.height) / 2;
								}
							}
								break;
					/*3*/	case scalingModeFill: {
								params.rightMargin = 0;
								params.bottomMargin = 0;
								params.width = layoutWidth;
								params.height = layoutHeight;
							}
								break;
							}
							
							//Log.e(JNIConst.LOG_TAG, "LQ on_prepared params " + params.leftMargin + " " + params.topMargin 
							//		+ " " + params.width + " " + params.height + " scaling mode: " + control.scalingMode);
							control.videoView.setLayoutParams(params);
							// HACK we have to change parent layout params here to Lenovo k900 show fullscreen video
							FrameLayout.LayoutParams frameLayoutParams = (FrameLayout.LayoutParams)control.layout.getLayoutParams();
							frameLayoutParams.leftMargin = control.leftMargin;
							frameLayoutParams.topMargin = control.topMargin;
							frameLayoutParams.width = params.width;
							frameLayoutParams.height = params.height;
							control.layout.setLayoutParams(frameLayoutParams);
						}
					});
					RelativeLayout.LayoutParams relLayoutParams = new RelativeLayout.LayoutParams(
							RelativeLayout.LayoutParams.WRAP_CONTENT, 
							RelativeLayout.LayoutParams.WRAP_CONTENT);
					relLayoutParams.alignWithParent = true;
					
					RelativeLayout relativeLayout = new RelativeLayout(activity);
					relativeLayout.setId(1984); // to find layout in DDMS
					relativeLayout.addView(videoView, relLayoutParams);
					
					control.layout = relativeLayout;
					control.videoView = videoView;

					videoView.clearAnimation();
					relativeLayout.clearAnimation();
					videoView.setZOrderOnTop(true);
							
					FrameLayout.LayoutParams frameLayoutParams = new FrameLayout.LayoutParams(
							FrameLayout.LayoutParams.WRAP_CONTENT,
							FrameLayout.LayoutParams.WRAP_CONTENT);
					
					//Log.e(JNIConst.LOG_TAG, "LQ initialize ui thread " + params.leftMargin + " " + 
					//		params.topMargin + " " + params.width + " " + params.height);
					activity.addContentView(relativeLayout, frameLayoutParams);
				}
			});
		}
	}

	public static void Uninitialize(final int id) {
		if (!controls.containsKey(id))
			return;

		final MovieControl control = controls.remove(id);

		JNIActivity.GetActivity().runOnUiThread(new Runnable() {		
			@Override
			public void run() {
				((ViewGroup)control.videoView.getParent()).removeView(control.videoView);
				control.videoView = null;
				control.player = null;
				control.pausePosition = 0;
				control.logicIsPlaying = false;
				try
				{
					if (control.extPath != null)
					{
						File file = new File(control.extPath);
						file.delete();
						control.extPath = null;
					}
				} catch (Exception e)
				{
					Log.e(JNIConst.LOG_TAG, "can't remove file video file cause: " + e);
				}
			}
		});
	}

	public static void SetRect(final int id, final float x, final float y,
			final float dx, final float dy) {
		
		//Log.e(JNIConst.LOG_TAG, "LQ set_rect " + x + " " + y + " " + dx + " " + dy);
		
		if (!controls.containsKey(id))
			return;

		final MovieControl control = controls.get(id);
		
		control.leftMargin = Math.round(x);
		control.topMargin = Math.round(y);
		control.width = Math.round(dx);
		control.height = Math.round(dy);
	}

	public static void SetVisible(final int id, final boolean isVisible) {
		if (!controls.containsKey(id))
			return;

		final MovieControl control = controls.get(id);
		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
			@Override
			public void run() {
				VideoView view = control.videoView;
				view.setVisibility(isVisible ? VideoView.VISIBLE : VideoView.GONE);
			}
		});
	}

	public static void OpenMovie(final int id, final String path, final int scalingMode) {
		if(!controls.containsKey(id))
			return;
		
		//Log.e(JNIConst.LOG_TAG, "LQ open_movie " + scalingMode);
		
		final MovieControl control = controls.get(id);
		control.scalingMode = scalingMode;
		
		String extPath = null;
		File f = new File(path);
		if (!f.exists()) {
			AssetManager assetManager = JNIActivity.GetActivity().getAssets();
			try {
				// HACK we need add Data to path
				File targetFile = File.createTempFile("movie", null, null);
                targetFile.deleteOnExit();
 
                extPath = targetFile.getAbsolutePath();
 
                InputStream is = assetManager.open("Data/" + path);
                OutputStream os = new FileOutputStream(targetFile);
 
                int nread = 0;
                byte[] buffer = new byte[4096];
                while ((nread = is.read(buffer)) != -1)
                {
                    os.write(buffer, 0, nread);
                }
                os.close();
			} catch (IOException e) {
				Log.e(JNIConst.LOG_TAG, e.getMessage());
			}
		} else
		{
			extPath = path;
		}

		control.extPath = extPath;
		
		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
			@Override
			public void run() {
				try
				{
					//Log.e(JNIConst.LOG_TAG, "LQ open_movie setVideoPath ui thread " + control.extPath);
				    control.videoView.setVideoPath(control.extPath);	
				} catch(Exception e)
				{
					Log.e(JNIConst.LOG_TAG, "can't setVideoPath cause: " + e.toString());
				}
			}
		});
	}
	
	public static void Play(final int id) {
		if(!controls.containsKey(id))
			return;

		final MovieControl control = controls.get(id);
		control.logicIsPlaying = true; // in current thread
		
		//Log.e(JNIConst.LOG_TAG, "LQ play");
		
		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
			@Override
			public void run() {
				if (control.player == null)
				{
					//Log.e(JNIConst.LOG_TAG, "LQ play control.player == null set_video_path ui thread " + control.extPath);
					control.videoView.setVideoPath(control.extPath);
				}
				//Log.e(JNIConst.LOG_TAG, "LQ play ui thread ");
				control.videoView.start();
			}
		});
	}

	public static void Stop(final int id) {
		if (!controls.containsKey(id))
			return;
		
		final MovieControl control = controls.get(id);
		control.logicIsPlaying = false; // in current thread

		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
			@Override
			public void run() {
				control.player = null;
				control.videoView.stopPlayback();
			}
		});
	}

	public static void Pause(final int id) {
		if (!controls.containsKey(id))
			return;
		
		final MovieControl control = controls.get(id);
		control.logicIsPlaying = false;

		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
			@Override
			public void run() {
				if (control.player != null)
				{
					try
					{
						control.player.pause();
					} catch(Exception e)
					{
						control.pausePosition = control.videoView.getCurrentPosition();
						control.videoView.pause();
					}
				}
			}
		});
	}

	public static void Resume(final int id) {
		if (!controls.containsKey(id))
			return;
		
		final MovieControl control = controls.get(id);
		control.logicIsPlaying = true;
		
		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
			@Override
			public void run() {
				if (control.player != null)
				{
					try
					{
						control.player.start();
					} catch(Exception e)
					{
						control.videoView.seekTo(control.pausePosition);
						control.videoView.start();
					}
				}
			}
		});
	}

	public static boolean IsPlaying(final int id) {
		boolean result = false;
		if (!controls.containsKey(id))
		{
			result = false;
		} else
		{
			MovieControl control = controls.get(id);
			result = control.logicIsPlaying;
		}
		return result;
	}
}
