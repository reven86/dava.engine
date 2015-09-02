package com.dava.framework;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Callable;
import java.util.concurrent.FutureTask;

import android.app.Activity;
import android.graphics.Color;
import android.media.MediaPlayer;
import android.view.SurfaceHolder;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.RelativeLayout;
import android.widget.VideoView;

public class JNIMovieViewControl {
	
	private final static int scalingModeNone = 0;
	private final static int scalingModeAspectFit = 1;
	private final static int scalingModeAspectFill = 2;
	private final static int scalingModeFill = 3;
	
	private final static int playerStateNoReady = 0;
	private final static int playerStateInprogress = 1;
	private final static int playerStateReady = 2;
	private final static int playerStatePlaying = 3;
	private final static int playerStateErrorData = 4;

	private static class MovieControl {
		public MovieControl(int id, RelativeLayout layout, VideoView view, MediaPlayer player) {
			this.id = id;
			this.view = view;
			this.player = player;
			this.layout = layout;
		}

		void setPlayerState(int newState) {
			if (_playerState != playerStateErrorData)
				_playerState = newState;
		}
		
		int getPlayerState() {
			return _playerState;
		}
		
		public int id = 0;
		public RelativeLayout layout = null;
		public VideoView view = null;
		public MediaPlayer player = null;
		private int _playerState = playerStateNoReady;
		public boolean isResetedBeforeHide = false;
		public String path = null;
		public int scalingMode = scalingModeNone;
	}

	private static Map<Integer, MovieControl> controls = new HashMap<Integer, MovieControl>();

	public static void Initialize(final int id, final float x, final float y,
			final float dx, final float dy) {
		final JNIActivity activity = JNIActivity.GetActivity();
        FutureTask<Void> task = new FutureTask<Void>(new Callable<Void>() {
			
			@Override
			public Void call() throws Exception {
				FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
						Math.round(dx), Math.round(dy));
				params.leftMargin = (int) x;
				params.topMargin = (int) y;
				
				MovieControl control = null;
				VideoView view = null;
				RelativeLayout layout = null;
				if (controls.containsKey(id)) {
					control = controls.get(id);
					view = control.view;
					layout = control.layout;
					layout.setLayoutParams(params);
				} else {
					layout = new RelativeLayout(activity);
					view = new VideoView(layout.getContext());
					view.setBackgroundColor(Color.BLACK);
					RelativeLayout.LayoutParams params2 = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.MATCH_PARENT);
					layout.addView(view, params2);
					control = new MovieControl(id, layout, view, new MediaPlayer());
					class MovieHolder implements SurfaceHolder.Callback {
						private MovieControl control = null;
						
						public MovieHolder(MovieControl control) {
							this.control = control;
						}

						@Override
						public void surfaceChanged(SurfaceHolder holder,
								int format, int width, int height) {
						}

						@Override
						public void surfaceCreated(SurfaceHolder holder) {
							control.player.setDisplay(holder);
							if (control.isResetedBeforeHide) {
								try {
									control.player.setDataSource(control.path);
								} catch (IllegalArgumentException e) {
									e.printStackTrace();
								} catch (SecurityException e) {
									e.printStackTrace();
								} catch (IllegalStateException e) {
									e.printStackTrace();
								} catch (IOException e) {
									e.printStackTrace();
								}
							}
							control.setPlayerState(playerStateNoReady);
							control.isResetedBeforeHide = false;
						}

						@Override
						public void surfaceDestroyed(SurfaceHolder holder) {
							control.player.reset();
							control.isResetedBeforeHide = true;
						}
						
					}

					view.clearAnimation();
					layout.clearAnimation();
					view.setZOrderOnTop(true);
					view.getHolder().addCallback(new MovieHolder(control));
					activity.addContentView(layout, params);
					controls.put(id, control);
				}
				
				return null;
			}
		});
		
		activity.runOnUiThread(task);
		while (!task.isDone() && !activity.GetIsPausing())
		{
			Thread.yield();
		}
	}

	public static void Uninitialize(final int id) {
		if (!controls.containsKey(id))
			return;
		
		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
			
			@Override
			public void run() {
				MovieControl control = controls.remove(id);
				((ViewGroup)control.view.getParent()).removeView(control.view);
				control.player.release();
			}
		});
	}

	public static void SetRect(final int id, final float x, final float y,
			final float dx, final float dy) {
		if (!controls.containsKey(id))
			return;
		
		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
		
			@Override
			public void run() {
				RelativeLayout layout = controls.get(id).layout;
				FrameLayout.LayoutParams params = (FrameLayout.LayoutParams) layout.getLayoutParams();
				params.leftMargin = (int) x;
				params.topMargin = (int) y;
				params.width = Math.round(dx);
				params.height = Math.round(dy);
				layout.setLayoutParams(params);
			}
		});
	}

	public static void SetVisible(final int id, final boolean isVisible) {
		if (!controls.containsKey(id))
			return;
		
		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
		
			@Override
			public void run() {
				VideoView view = controls.get(id).view;
				view.setVisibility(isVisible ? VideoView.VISIBLE : VideoView.GONE);
			}
		});
	}
	
	private static void PrepareVideo(final MovieControl control) {
		control.setPlayerState(playerStateInprogress);
		control.player.setOnPreparedListener(new MediaPlayer.OnPreparedListener() {
			
			@Override
			public void onPrepared(MediaPlayer mp) {
				int layoutWidth = control.layout.getWidth();
				int layoutHeight = control.layout.getHeight();
				int videoWidth = control.player.getVideoWidth();
				int videoHeight = control.player.getVideoHeight();
				
				if (videoHeight == 0 || videoWidth == 0)
				{
					control.setPlayerState(playerStateErrorData);
					return;
				}
				
				//update scaling
				RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(0, 0);
				switch (control.scalingMode) {
				case scalingModeNone: {
					params.width = Math.min(layoutWidth, videoWidth);
					params.height = Math.min(layoutHeight, videoHeight);
					params.leftMargin = (layoutWidth - params.width) / 2;
					params.topMargin = (layoutHeight - params.height) / 2;
				} break;
				case scalingModeAspectFit: {
					float xFactor = videoWidth / (float)layoutWidth;
					float yFactor = videoHeight / (float)layoutHeight;
					if (xFactor > yFactor) {
						params.width = layoutWidth;
						params.height = videoHeight * layoutWidth / videoWidth;
						params.topMargin = (layoutHeight - params.height) / 2;
					} else {
						params.height = layoutHeight;
						params.width = videoWidth * layoutHeight/ videoHeight;
						params.leftMargin = (layoutWidth - params.width) / 2;
					}
				} break;
				case scalingModeAspectFill: {
					float xFactor = videoWidth / (float)layoutWidth;
					float yFactor = videoHeight / (float)layoutHeight;
					if (xFactor > yFactor) {
						params.height = layoutHeight;
						params.width = videoWidth * layoutHeight/ videoHeight;
						params.leftMargin = params.rightMargin = (layoutWidth - params.width) / 2;
						params.topMargin = params.bottomMargin = 1;
					} else {
						params.width = layoutWidth;
						params.height = videoHeight * layoutWidth / videoWidth;
						params.leftMargin = params.rightMargin = 1;
						params.topMargin = params.bottomMargin = (layoutHeight - params.height) / 2;
					}
				} break;
				case scalingModeFill: {
					params.rightMargin = params.bottomMargin = 0;
					params.width = layoutWidth;
					params.height = layoutHeight;
				} break;
				}
				control.view.setLayoutParams(params);
				
				control.player.setOnSeekCompleteListener(new MediaPlayer.OnSeekCompleteListener() {
					
					@Override
					public void onSeekComplete(MediaPlayer mp) {
						control.setPlayerState(playerStateReady);
						Play(control.id);
						control.player.setOnSeekCompleteListener(null);
					}
				});
				control.player.seekTo(0);
			}
		});
		
		
		control.player.prepareAsync();
	}

	public static void OpenMovie(final int id, final String path, final int scalingMode) {
		if (!controls.containsKey(id))
			return;
		
		FutureTask<Void> task = new FutureTask<Void>(new Callable<Void>() {
			@Override
			public Void call() throws Exception {
				MovieControl control= controls.get(id);
				control.scalingMode = scalingMode;
				control.path = path;
				try {
					LocalFileDescriptor descriptor = new LocalFileDescriptor(path);
					control.player.setDataSource(descriptor.GetDescriptor(), descriptor.GetStartOffset(), descriptor.GetLength());
				} catch (Exception e) {
					control.setPlayerState(playerStateErrorData);
				}
				return null;
			}
		});
		
		JNIActivity.GetActivity().runOnUiThread(task);
		while (!task.isDone())
			Thread.yield();
	}
	
	public static void Play(final int id) {
		if (!controls.containsKey(id))
			return;
		
		FutureTask<Void> task = new FutureTask<Void>(new Callable<Void>() {
			@Override
			public Void call() throws Exception {
				MovieControl control = controls.get(id);
				
				if (control.getPlayerState() == playerStateNoReady)
					PrepareVideo(control);
				else if (control.getPlayerState() == playerStateReady) {
					control.player.start();
					control.setPlayerState(playerStatePlaying);
				}
				return null;
			}
		});
		
		JNIActivity.GetActivity().runOnUiThread(task);
		while (!task.isDone())
			Thread.yield();
	}

	public static void Stop(final int id) {
		if (!controls.containsKey(id))
			return;
		
		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
		
			@Override
			public void run() {
				MovieControl control = controls.get(id);
				control.player.stop();
				control.setPlayerState(playerStateNoReady);
			}
		});
	}

	public static void Pause(final int id) {
		if (!controls.containsKey(id))
			return;
		
		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
		
			@Override
			public void run() {
				MediaPlayer player = controls.get(id).player;
				player.pause();
			}
		});
	}

	public static void Resume(final int id) {
		Play(id);
	}

	public static boolean IsPlaying(final int id) {
		if (!controls.containsKey(id))
			return false;
		
		MovieControl control = controls.get(id);
		if (control.getPlayerState() == playerStateErrorData)
		{
			return false;
		}
		
		if (control.getPlayerState() == playerStateInprogress ||
			control.getPlayerState() == playerStateReady)
			return true;
		
		MediaPlayer player = control.player;
		return player.isPlaying();
	}
}
