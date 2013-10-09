package com.dava.framework;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Callable;
import java.util.concurrent.FutureTask;

import android.app.Activity;
import android.media.MediaPlayer;
import android.view.SurfaceHolder;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.VideoView;

public class JNIMovieViewControl {

	private static class MovieControl {
		public MovieControl(VideoView view, MediaPlayer player) {
			this.view = view;
			this.player = player;
		}

		public VideoView view = null;
		public MediaPlayer player = null;
		public boolean isReady = false;
		public boolean isResetedBeforeHide = false;
		public String path = null;
	}

	private static Map<Integer, MovieControl> controls = new HashMap<Integer, MovieControl>();

	public static void Initialize(final int id, final float x, final float y,
			final float dx, final float dy) {
		FutureTask<Void> task = new FutureTask<Void>(new Callable<Void>() {
			
			@Override
			public Void call() throws Exception {
				Activity activity = JNIActivity.GetActivity();
				FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
						Math.round(dx), Math.round(dy));
				params.leftMargin = (int) x;
				params.topMargin = (int) y;
				
				VideoView view = null;
				if (controls.containsKey(id)) {
					view = (VideoView) controls.get(id).view;
					view.setLayoutParams(params);
				} else {
					view = new VideoView(activity);
					MovieControl control = new MovieControl(view, new MediaPlayer());
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
							control.isReady = false;
							control.isResetedBeforeHide = false;
						}

						@Override
						public void surfaceDestroyed(SurfaceHolder holder) {
							control.player.reset();
							control.isResetedBeforeHide = true;
						}
						
					}

					view.getHolder().addCallback(new MovieHolder(control));
					activity.addContentView(view, params);
					controls.put(id, control);
				}
				return null;
			}
		});
		
		JNIActivity.GetActivity().runOnUiThread(task);
		while (!task.isDone())
			Thread.yield();
	}

	public static void Uninitialize(final int id) {
		if (!controls.containsKey(id))
			return;
		
		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
			
			@Override
			public void run() {
				MovieControl control = controls.remove(id);
				control.player.release();
				((ViewGroup)control.view.getParent()).removeView(control.view);
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
				VideoView view = controls.get(id).view;
				FrameLayout.LayoutParams params = (FrameLayout.LayoutParams) view.getLayoutParams();
				params.leftMargin = (int) x;
				params.topMargin = (int) y;
				params.width = Math.round(dx);
				params.height = Math.round(dy);
				view.setLayoutParams(params);
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

	public static void OpenMovie(final int id, final String path) {
		if (!controls.containsKey(id))
			return;
		
		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
		
			@Override
			public void run() {
				MovieControl control= controls.get(id);
				control.path = path;
				try {
					LocalFileDescriptor descriptor = new LocalFileDescriptor(path);
					control.player.setDataSource(descriptor.GetDescriptor(), descriptor.GetStartOffset(), descriptor.GetLength());
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
		});
	}

	public static void Play(final int id) {
		if (!controls.containsKey(id))
			return;
		
		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
		
			@Override
			public void run() {
				MovieControl control = controls.get(id);
				
				if (!control.isReady) {
					try {
						control.player.prepare();
						control.player.seekTo(0);
						control.isReady = true;
					} catch (IllegalStateException e) {
						e.printStackTrace();
						return;
					} catch (IOException e) {
						e.printStackTrace();
						return;
					}
				}
				control.player.start();
			}
		});
	}

	public static void Stop(final int id) {
		if (!controls.containsKey(id))
			return;
		
		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
		
			@Override
			public void run() {
				MovieControl control = controls.get(id);
				control.player.stop();
				control.isReady = false;
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
		
		MediaPlayer player = controls.get(id).player;
		return player.isPlaying();
	}
}
