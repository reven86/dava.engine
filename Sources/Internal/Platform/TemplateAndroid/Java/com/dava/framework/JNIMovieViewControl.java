package com.dava.framework;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Callable;
import java.util.concurrent.FutureTask;

import android.graphics.Color;
import android.media.MediaPlayer;
import android.view.SurfaceHolder;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.RelativeLayout;
import android.widget.VideoView;
import android.util.Log;

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
		public MovieControl(int id) {
			this.id = id;
		}

		public MovieControl(int id, RelativeLayout layout, VideoView view, MediaPlayer player) {
			this.id = id;
			this.view = view;
			this.player = player;
			this.layout = layout;
		}

		void setPlayerState(int newState) {
			Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::MovieControl::setPlayerState " + newState);
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

	private static class MovieHolder implements SurfaceHolder.Callback {
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
			Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Initialize::MovieHolder::surfaceCreated in");

			control.player.setDisplay(holder);
			if (control.isResetedBeforeHide) {
				control.setPlayerState(playerStateNoReady);
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
			control.isResetedBeforeHide = false;

			Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Initialize::MovieHolder::surfaceCreated out");
		}

		@Override
		public void surfaceDestroyed(SurfaceHolder holder) {
			Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Initialize::MovieHolder::surfaceDestroyed in");
			control.player.reset();
			control.isResetedBeforeHide = true;
			Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Initialize::MovieHolder::surfaceDestroyed out");
		}
	}

	private static Map<Integer, MovieControl> controls = new HashMap<Integer, MovieControl>();

	public static void Initialize(final int id, final float x, final float y,
			final float dx, final float dy) {

		Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Initialize in");

		final JNIActivity activity = JNIActivity.GetActivity();

		if(controls.containsKey(id)) {
			final MovieControl control = controls.get(id);
			activity.runOnUiThread(new Runnable(){

				@Override
				public void run() {
					Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Initialize::run(update) in");

					FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
						Math.round(dx), Math.round(dy));
					params.leftMargin = (int) x;
					params.topMargin = (int) y;
					RelativeLayout layout = control.layout;
					layout.setLayoutParams(params);

					Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Initialize::run(update) out");
				}
			});

		} else {
			final MovieControl control = new MovieControl(id);
			controls.put(id, control);
			activity.runOnUiThread(new Runnable() {

				@Override
				public void run() {
					Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Initialize::run(create) in");

					FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
						Math.round(dx), Math.round(dy));
					params.leftMargin = (int) x;
					params.topMargin = (int) y;

					RelativeLayout layout = new RelativeLayout(activity);
					VideoView view = new VideoView(layout.getContext());
					view.setBackgroundColor(Color.BLACK);
					RelativeLayout.LayoutParams params2 = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.MATCH_PARENT);
					layout.addView(view, params2);
					
					control.layout = layout;
					control.view = view;
					control.player = new MediaPlayer();

					view.clearAnimation();
					layout.clearAnimation();
					view.setZOrderOnTop(true);
					view.getHolder().addCallback(new MovieHolder(control));
					activity.addContentView(layout, params);

					Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Initialize::run(create) out");
				}
			});
		}

		Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Initialize out");
	}

	public static void Uninitialize(final int id) {
		if (!controls.containsKey(id))
			return;

		Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Uninitialize in");

		final MovieControl control = controls.remove(id);

		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
			
			@Override
			public void run() {
				Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Uninitialize::run in");
				((ViewGroup)control.view.getParent()).removeView(control.view);
				control.player.release();
				Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Uninitialize::run out");
			}
		});

		Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Uninitialize out");
	}

	public static void SetRect(final int id, final float x, final float y,
			final float dx, final float dy) {
		if (!controls.containsKey(id))
			return;
		
		Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::SetRect in");

		final MovieControl control = controls.get(id);
		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
		
			@Override
			public void run() {
				Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::SetRect::run in");

				RelativeLayout layout = control.layout;
				FrameLayout.LayoutParams params = (FrameLayout.LayoutParams) layout.getLayoutParams();
				params.leftMargin = (int) x;
				params.topMargin = (int) y;
				params.width = Math.round(dx);
				params.height = Math.round(dy);
				layout.setLayoutParams(params);

				Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::SetRect::run out");
			}
		});

		Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::SetRect out");
	}

	public static void SetVisible(final int id, final boolean isVisible) {
		if (!controls.containsKey(id))
			return;

		Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::SetVisible in");

		final MovieControl control = controls.get(id);
		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
		
			@Override
			public void run() {
				Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::SetVisible::run in");

				VideoView view = control.view;
				view.setVisibility(isVisible ? VideoView.VISIBLE : VideoView.GONE);

				Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::SetVisible::run out");
			}
		});

		Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::SetVisible out");
	}
	
	private static void PrepareVideo(final MovieControl control) {
		Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::PrepareVideo in");

		control.setPlayerState(playerStateInprogress);
		JNIActivity.GetActivity().runOnUiThread(new Runnable() {

			@Override
			public void run() {

				Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::PrepareVideo::run in");

				control.player.setOnPreparedListener(new MediaPlayer.OnPreparedListener() {
			
					@Override
					public void onPrepared(MediaPlayer mp) {

						Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::PrepareVideo::onPrepared in");

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
								control.setPlayerState(playerStatePlaying);
								control.player.start();
								control.player.setOnSeekCompleteListener(null);
							}
						});
						control.player.seekTo(0);

						Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::PrepareVideo::onPrepared out");
					}
				});
				control.player.prepareAsync();

				Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::PrepareVideo::run out");
			}
		});

		Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::PrepareVideo out");
	}

	public static void OpenMovie(final int id, final String path, final int scalingMode) {
		if(!controls.containsKey(id))
			return;

		Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::OpenMovie in");
		
		final MovieControl control = controls.get(id);
		control.scalingMode = scalingMode;
		control.path = path;

		JNIActivity.GetActivity().runOnUiThread(new Runnable(){

			@Override
			public void run() {
				Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::OpenMovie::run in");
				try {
					LocalFileDescriptor descriptor = new LocalFileDescriptor(path);
					control.player.setDataSource(descriptor.GetDescriptor(), descriptor.GetStartOffset(), descriptor.GetLength());
				} catch (Exception e) {
					control.setPlayerState(playerStateErrorData);
				}
				Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::OpenMovie::run out");
			}
		});
		
		Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::OpenMovie out");
	}
	
	public static void Play(final int id) {
		if(!controls.containsKey(id))
			return;

		Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Play in");
		
		final MovieControl control = controls.get(id);

		if (control.getPlayerState() == playerStateNoReady) {
			PrepareVideo(control);
		} else if (control.getPlayerState() == playerStateReady 
				|| control.getPlayerState() == playerStatePlaying) {
			
			JNIActivity.GetActivity().runOnUiThread(new Runnable() {
				@Override
				public void run() {
					Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Play::run in");
					control.setPlayerState(playerStatePlaying);
					control.player.start();
					Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Play::run out");
				}	
			});
		}

		Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Play out");
	}

	public static void Stop(final int id) {
		Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Stop in");

		if (!controls.containsKey(id))
			return;
		
		final MovieControl control = controls.get(id);
		control.setPlayerState(playerStateNoReady);

		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
			@Override
			public void run() {
				Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Stop::run in");
				control.player.stop();
				Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Stop::run out");
			}
		});

		Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Stop out");
	}

	public static void Pause(final int id) {
		if (!controls.containsKey(id))
			return;

		Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Pause in");
		
		final MovieControl control = controls.get(id);

		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
			@Override
			public void run() {
				Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Pause::run in");
				control.player.pause();
				Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Pause::run out");
			}
		});

		Log.d(JNIConst.LOG_TAG, "JNIMovieViewControl::Pause out");
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
