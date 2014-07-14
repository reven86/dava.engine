package com.dava.framework;

import java.util.HashMap;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;

import android.content.Context;
import android.graphics.Color;
import android.os.Looper;
import android.text.InputFilter;
import android.text.InputType;
import android.text.Spanned;
import android.text.method.PasswordTransformationMethod;
import android.util.Log;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.TextView;

public class JNITextField {
	static class NativeEditText {
		public EditText editText;
		public int id;
	}
	static Map<Integer, NativeEditText> controls = new HashMap<Integer, NativeEditText>();
	
	static final int NoActiveTextField = -1;
	static int activeTextField = NoActiveTextField; 

	final static String TAG = "JNITextField";
	
	private static NativeEditText GetNativeEditText(int id) {
		if (!controls.containsKey(id)) {
			Log.d(TAG, String.format("Unknown control id:%d", id));
			return null;
		}
		return controls.get(id);
	}
	
	private static EditText GetEditText(int id) {
		NativeEditText nativeEditText = GetNativeEditText(id);
		if (nativeEditText != null)
			return nativeEditText.editText;
		return null;
	}
	
	private static class InternalTask<V>{
		EditText text = null;
		Callable<V> task = null;
		public InternalTask(EditText text, Callable<V> task) {
			this.text = text;
			this.task = task;
		}
		
		public V Run() {
			FutureTask<V> inTask = new FutureTask<V>(new Callable<V>() {
				@Override
				public V call() throws Exception {
					InputFilter[] filters = null;
					if (text != null) {
						filters = text.getFilters();
						text.setFilters(new InputFilter[]{});
					}
					V res = task.call();
					if (text != null)
						text.setFilters(filters);
					return res;
				}
			});
			JNIActivity.GetActivity().runOnUiThread(inTask);
			try {
				return inTask.get();
			} catch (InterruptedException e) {
				e.printStackTrace();
			} catch (ExecutionException e) {
				e.printStackTrace();
			}
			return null;
		}
		
		public void AsyncRun() {
			Runnable inTask = new Runnable() {
				@Override
				public void run() {
					InputFilter[] filters = null;
					if (text != null) {
						filters = text.getFilters();
						text.setFilters(new InputFilter[]{});
					}
					try {
						task.call();
					} catch (Exception e) {
						e.printStackTrace();
					}
					if (text != null)
						text.setFilters(filters);
				}
			};
			JNIActivity.GetActivity().runOnUiThread(inTask);
		}
	}

	public static void Create(final int id, final float x, final float y,
			final float dx, final float dy) {
		if (controls.containsKey(id)) {
			Log.d(TAG, String.format("Control with id:%d already created", id));
			return;
		}

		InternalTask<Void> task = new InternalTask<Void>(null, new Callable<Void>() {
			@Override
			public Void call() throws Exception {
				JNIActivity activity = JNIActivity.GetActivity();
				EditText text = new EditText(activity) {
					@Override
					public boolean onTouchEvent(MotionEvent event) {
						MotionEvent newEvent = MotionEvent.obtain(event);
						newEvent.setLocation(getLeft() + event.getX(), getTop() + event.getY());
						JNIActivity.GetActivity().glView.dispatchTouchEvent(newEvent);
						return super.onTouchEvent(event);
					}
				};
				
				FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
						Math.round(dx), Math.round(dy));
				params.leftMargin = Math.round(x);
				params.topMargin = Math.round(y);
				params.gravity = Gravity.LEFT | Gravity.TOP;
				text.setPadding(0, 0, 0, 0);
				text.setSingleLine(true);
				int fontSize = (int) (20);
				text.setTextSize(TypedValue.COMPLEX_UNIT_PX, fontSize);
				text.setBackgroundColor(Color.TRANSPARENT);
				text.setTextColor(Color.WHITE);
				text.setVisibility(View.GONE);

				activity.addContentView(text, params);
				NativeEditText nativeEditText = new NativeEditText();
				nativeEditText.editText = text;
				nativeEditText.id = id;
				
				InputFilter inputFilter = new InputFilter() {
					private final int _id = id;
					
					@Override
					public CharSequence filter(final CharSequence source, final int start, final int end,
							Spanned dest, final int dstart, final int dend) {
						
						FutureTask<Boolean> t = new FutureTask<Boolean>(new Callable<Boolean>() {
							@Override
							public Boolean call() throws Exception {
								byte []bytes = source.toString().getBytes("UTF-8");
								return TextFieldKeyPressed(_id, dstart, dend - dstart, bytes);
							}
						});
						JNIActivity.GetActivity().PostEventToGL(t);
						try {
							if (t.get())
								return source;
						} catch (InterruptedException e) {
							e.printStackTrace();
						} catch (ExecutionException e) {
							e.printStackTrace();
						}
						
						return "";
					}
				};
				text.setFilters(new InputFilter[]{inputFilter});
				
				text.setOnEditorActionListener(new TextView.OnEditorActionListener() {
					private final int _id = id;
					
					@Override
					public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
						JNIActivity.GetActivity().PostEventToGL(new Runnable() {
							@Override
							public void run() {
								JNITextField.TextFieldShouldReturn(_id);
							}
						});
						return true;
					}
				});
				
				controls.put(id, nativeEditText);
				return null;
			}
		});
		task.Run();
	}

	static void Destroy(final int id) {
		final EditText editText = GetEditText(id);
		if (editText == null)
			return;

		InternalTask<Void> task = new InternalTask<Void>(editText, new Callable<Void>() {
			@Override
			public Void call() throws Exception {
				ViewGroup parent = (ViewGroup) editText.getParent();
				if (parent != null)
					parent.removeView(editText);
				controls.remove(id);
				return null;
			}
		});
		task.Run();
	}

	public static void UpdateRect(final int id, final float x, final float y,
			final float dx, final float dy) {
		final EditText editText = GetEditText(id);
		if (editText == null)
			return;

		InternalTask<Void> task = new InternalTask<Void>(editText, new Callable<Void>() {
			@Override
			public Void call() throws Exception {
				FrameLayout.LayoutParams params = (FrameLayout.LayoutParams) editText
						.getLayoutParams();
				params.leftMargin = Math.round(x);
				params.topMargin = Math.round(y);
				params.width = Math.round(dx);
				params.height = Math.round(dy);
				editText.setLayoutParams(params);
				return null;
			}
		});
		task.AsyncRun();
	}

	public static void SetText(int id, final String string) {
		final EditText text = GetEditText(id);
		if (text == null)
			return;

		InternalTask<Void> task = new InternalTask<Void>(text, new Callable<Void>() {
			@Override
			public Void call() throws Exception {
				text.setText(string);
				return null;
			}
		});
		task.AsyncRun();
	}

	public static void SetTextColor(int id, final float r, final float g,
			final float b, final float a) {
		final EditText text = GetEditText(id);
		if (text == null)
			return;

		InternalTask<Void> task = new InternalTask<Void>(text, new Callable<Void>() {
			@Override
			public Void call() throws Exception {
				text.setTextColor(Color.argb((int) (255 * a), (int) (255 * r),
						(int) (255 * g), (int) (255 * b)));
				return null;
			}
		});
		task.AsyncRun();
	}

	public static void SetFontSize(int id, final float size) {
		final EditText text = GetEditText(id);
		if (text == null)
			return;

		InternalTask<Void> task = new InternalTask<Void>(text, new Callable<Void>() {
			@Override
			public Void call() throws Exception {
				text.setTextSize(TypedValue.COMPLEX_UNIT_PX, size);
				return null;
			}
		});
		task.Run();
	}

	public static void SetIsPassword(int id, final boolean isPassword) {
		final EditText text = GetEditText(id);
		if (text == null)
			return;

		InternalTask<Void> task = new InternalTask<Void>(text, new Callable<Void>() {
			@Override
			public Void call() throws Exception {
				class PswTransformationMethod extends
						PasswordTransformationMethod {
					@Override
					public CharSequence getTransformation(CharSequence source,
							View view) {
						return new PasswordCharSequence(source);
					}

					class PasswordCharSequence implements CharSequence {
						private CharSequence source;

						public PasswordCharSequence(CharSequence source) {
							this.source = source;
						}

						public char charAt(int index) {
							return '*';
						}

						public int length() {
							return source.length();
						}

						public CharSequence subSequence(int start, int end) {
							return source.subSequence(start, end);
						}
					}
				};
				
				if (isPassword) {
					text.setTransformationMethod(new PswTransformationMethod());
					text.setInputType(EditorInfo.TYPE_CLASS_TEXT | EditorInfo.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD);
				}
				else
				{
					text.setInputType(text.getInputType() & ~(EditorInfo.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD));
				}

				return null;
			}
		});
		task.Run();
	}

	public static void SetTextAlign(int id, final int align) {
		final EditText text = GetEditText(id);
		if (text == null)
			return;

		InternalTask<Void> task = new InternalTask<Void>(text, new Callable<Void>() {
			@Override
			public Void call() throws Exception {

				int gravityH = Gravity.LEFT;
				int gravityV = Gravity.CENTER_VERTICAL;
				if ((align & 0x01) > 0) // ALIGN_LEFT
					gravityH = Gravity.LEFT;
				if ((align & 0x02) > 0) // ALIGN_HCENTER
					gravityH = Gravity.CENTER_HORIZONTAL;
				if ((align & 0x04) > 0) // ALIGN_RIGHT
					gravityH = Gravity.RIGHT;
				if ((align & 0x08) > 0) // ALIGN_TOP
					gravityV = Gravity.TOP;
				if ((align & 0x10) > 0) // ALIGN_VCENTER
					gravityV = Gravity.CENTER_VERTICAL;
				if ((align & 0x20) > 0) // ALIGN_BOTTOM
					gravityV = Gravity.BOTTOM;

				text.setGravity(gravityH | gravityV);
				return null;
			}
		});
		task.Run();
	}

	public static void SetInputEnabled(int id, final boolean value) {
		final EditText text = GetEditText(id);
		if (text == null)
			return;

		InternalTask<Void> task = new InternalTask<Void>(text, new Callable<Void>() {
			@Override
			public Void call() throws Exception {
				text.setEnabled(value);
				return null;
			}
		});
		task.Run();
	}
	
	public static void SetAutoCapitalizationType(int id, final int autoCapitalizationType) {
		final EditText text = GetEditText(id);
		if (text == null)
			return;
		
		InternalTask<Void> task = new InternalTask<Void>(text, new Callable<Void>() {
			@Override
			public Void call() throws Exception {
				int autoCapitalizationFlag = text.getInputType();
				autoCapitalizationFlag &= ~(
						InputType.TYPE_TEXT_FLAG_CAP_WORDS |
						InputType.TYPE_TEXT_FLAG_CAP_SENTENCES |
						InputType.TYPE_TEXT_FLAG_CAP_CHARACTERS);
				switch (autoCapitalizationType) {
				case 1: // AUTO_CAPITALIZATION_TYPE_WORDS
					autoCapitalizationFlag |= InputType.TYPE_TEXT_FLAG_CAP_WORDS;
					break;
				case 2: // AUTO_CAPITALIZATION_TYPE_SENTENCES
					autoCapitalizationFlag |= InputType.TYPE_TEXT_FLAG_CAP_SENTENCES;
					break;
				case 3: // AUTO_CAPITALIZATION_TYPE_ALL_CHARS
					autoCapitalizationFlag |= InputType.TYPE_TEXT_FLAG_CAP_CHARACTERS;
					break;
				case 0: // AUTO_CAPITALIZATION_TYPE_NONE
				default:
					break;
				};
				text.setInputType(autoCapitalizationFlag);
				return null;
			}
		});
		task.Run();
	}
	
	public static void SetAutoCorrectionType(int id, final int autoCorrectionType) {
		final EditText text = GetEditText(id);
		if (text == null)
			return;
		
		InternalTask<Void> task = new InternalTask<Void>(text, new Callable<Void>() {
			@Override
			public Void call() throws Exception {
				int autoCorrectionFlag = text.getInputType();
				switch (autoCorrectionType) {
				case 0: // AUTO_CORRECTION_TYPE_DEFAULT
				case 2: // AUTO_CORRECTION_TYPE_YES
					autoCorrectionFlag |= InputType.TYPE_TEXT_FLAG_AUTO_CORRECT;
					break;
				case 1: // AUTO_CORRECTION_TYPE_NO
				default:
					autoCorrectionFlag &= ~(InputType.TYPE_TEXT_FLAG_AUTO_CORRECT);
					break;
				};
				text.setInputType(autoCorrectionFlag);
				return null;
			}
		});
		task.Run();
	}
	
	public static void SetSpellCheckingType(int id, final int spellCheckingType) {
		final EditText text = GetEditText(id);
		if (text == null)
			return;
		
		InternalTask<Void> task = new InternalTask<Void>(text, new Callable<Void>() {
			@Override
			public Void call() throws Exception {
				int spellCheckingFlag = text.getInputType();
				switch (spellCheckingType) {
				case 0: // SPELL_CHECKING_TYPE_DEFAULT
				case 2: // SPELL_CHECKING_TYPE_YES
					spellCheckingFlag |= InputType.TYPE_TEXT_FLAG_AUTO_COMPLETE;
					break;
				case 1: // SPELL_CHECKING_TYPE_NO
				default:
					spellCheckingFlag &= ~(InputType.TYPE_TEXT_FLAG_AUTO_COMPLETE);
					break;
				};
				text.setInputType(spellCheckingFlag);
				return null;
			}
		});
		task.Run();
	}
		
	public static void SetKeyboardType(int id, final int keyboardType) {
		final EditText text = GetEditText(id);
		if (text == null)
			return;
		
		InternalTask<Void> task = new InternalTask<Void>(text, new Callable<Void>() {
			@Override
			public Void call() throws Exception {

				int inputFlags = text.getInputType();
				inputFlags &= ~(InputType.TYPE_CLASS_NUMBER |
						InputType.TYPE_CLASS_TEXT |
						InputType.TYPE_TEXT_VARIATION_URI |
						EditorInfo.TYPE_CLASS_TEXT);
				
				switch (keyboardType) {
				case 2: // KEYBOARD_TYPE_NUMBERS_AND_PUNCTUATION
				case 4: // KEYBOARD_TYPE_NUMBER_PAD
				case 5: // KEYBOARD_TYPE_PHONE_PAD
				case 6: // KEYBOARD_TYPE_NAME_PHONE_PAD
				case 8: // KEYBOARD_TYPE_DECIMAL_PAD
					inputFlags |= InputType.TYPE_CLASS_NUMBER;
					break;

				case 3: // KEYBOARD_TYPE_URL
				case 7: // KEYBOARD_TYPE_EMAIL_ADDRESS
				case 9: // KEYBOARD_TYPE_TWITTER
					inputFlags |= InputType.TYPE_CLASS_TEXT
							| InputType.TYPE_TEXT_VARIATION_URI;
					break;

				case 0: // KEYBOARD_TYPE_DEFAULT
				case 1: // KEYBOARD_TYPE_ASCII_CAPABLE
				default:
					inputFlags |= EditorInfo.TYPE_CLASS_TEXT;
					break;
				};
				text.setInputType(inputFlags);
				return null;
			}
		});
		task.Run();
	}
	
	public static void SetReturnKeyType(int id, final int returnKeyType) {
		final EditText text = GetEditText(id);
		if (text == null)
			return;
		
		InternalTask<Void> task = new InternalTask<Void>(text, new Callable<Void>() {
			@Override
			public Void call() throws Exception {
				int imeOptions = 0;// EditorInfo.IME_ACTION_DONE;
				switch (returnKeyType) {
				case 1: // RETURN_KEY_GO,
					imeOptions = EditorInfo.IME_ACTION_GO;
					break;
				case 2: // RETURN_KEY_GOOGLE,
				case 6: // RETURN_KEY_SEARCH,
				case 8: // RETURN_KEY_YAHOO,
					imeOptions = EditorInfo.IME_ACTION_SEARCH;
					break;
				case 4: // RETURN_KEY_NEXT,
					imeOptions = EditorInfo.IME_ACTION_NEXT;
					break;
				case 7: // RETURN_KEY_SEND,
					imeOptions = EditorInfo.IME_ACTION_SEND;
					break;
				case 0: // RETURN_KEY_DEFAULT = 0,
				case 3: // RETURN_KEY_JOIN,
				case 5: // RETURN_KEY_ROUTE,
				case 9: // RETURN_KEY_DONE,
				case 10: // RETURN_KEY_EMERGENCY_CALL
				default:
					imeOptions = EditorInfo.IME_ACTION_DONE;
					break;
				}

				text.setImeOptions(imeOptions);
				return null;
			}
		});
		task.Run();
	}
	
	public static void ShowField(int id) {
		final EditText text = GetEditText(id);
		if (text == null)
			return;
		
		JNIActivity.GetActivity().PostEventToGL(new Runnable() {
			
			@Override
			public void run() {
				InternalTask<Void> task = new InternalTask<Void>(text, new Callable<Void>() {
					@Override
					public Void call() throws Exception {
						text.setVisibility(View.VISIBLE);
						return null;
					}
				});
				task.AsyncRun();
			}
		});
	}
	
	public static void HideField(int id) {
		final EditText text = GetEditText(id);
		if (text == null)
			return;
		
		if (id == activeTextField)
			CloseKeyboard(id);
		
		JNIActivity.GetActivity().PostEventToGL(new Runnable() {
			
			@Override
			public void run() {
				InternalTask<Void> task = new InternalTask<Void>(text, new Callable<Void>() {
					@Override
					public Void call() throws Exception {
						text.setVisibility(View.GONE);
						return null;
					}
				});
				task.AsyncRun();
			}
		});
	}
	
	public static void OpenKeyboard(final int id) {
		final EditText text = GetEditText(id);
		if (text == null)
			return;
		
		JNIActivity.GetActivity().PostEventToGL(new Runnable() {
			
			@Override
			public void run() {
				InternalTask<Void> task = new InternalTask<Void>(text, new Callable<Void>() {
					@Override
					public Void call() throws Exception {
						text.setVisibility(EditText.VISIBLE);
						text.requestFocus();
						
						InputMethodManager imm = (InputMethodManager) JNIActivity.GetActivity().getSystemService(Context.INPUT_METHOD_SERVICE);
						imm.showSoftInput(text, InputMethodManager.SHOW_FORCED);
						
						activeTextField = id;
						
						return null;
					}
				});
				task.AsyncRun();
			}
		});
	}
	
	public static void CloseKeyboard(int id) {
		final EditText text = GetEditText(id);
		if (text == null)
			return;
		
		final Runnable runnable = new Runnable() {
			
			@Override
			public void run() {
				InputMethodManager imm = (InputMethodManager) JNIActivity.GetActivity().getSystemService(Context.INPUT_METHOD_SERVICE);
				imm.hideSoftInputFromWindow(text.getWindowToken(), 0);
				text.clearFocus();
				activeTextField = NoActiveTextField;
			}
		};
		
		if (Thread.currentThread() == Looper.getMainLooper().getThread())
			runnable.run();
		else
		{
			InternalTask<Void> task = new InternalTask<Void>(text, new Callable<Void>() {
				@Override
				public Void call() throws Exception {
					runnable.run();
					return null;
				}
			});
			task.AsyncRun();
		}
	}
	
	public static void SetEnableReturnKeyAutomatically(int id, boolean value) {
		final EditText text = GetEditText(id);
		if (text == null)
			return;
	}
	
	public static void SetKeyboardAppearanceType(int id, int value) {
		final EditText text = GetEditText(id);
		if (text == null)
			return;
	}
	
	public static int GetCursorPos(int id) {
		final EditText text = GetEditText(id);
		if (text == null)
			return 0;
		
		int pos = text.getSelectionStart();
		return pos;
	}
	
	public static void SetCursorPos(int id, final int pos) {
		final EditText text = GetEditText(id);
		if (text == null)
			return;
		
		InternalTask<Void> task = new InternalTask<Void>(text, new Callable<Void>() {
			@Override
			public Void call() throws Exception {
				text.setSelection(pos);
				return null;
			}
		});
		task.AsyncRun();
	}
	
	static protected void RelinkNativeControls() {
		for (NativeEditText control: controls.values()) {
			View view = control.editText;
			ViewGroup viewGroup = (ViewGroup) view.getParent();
			viewGroup.removeView(view);
			JNIActivity.GetActivity().addContentView(view, view.getLayoutParams());
		}
	}
	
	static protected void Invalidate() {
		Timer t = new Timer();
		t.schedule(new TimerTask() {
			
			@Override
			public void run() {
				JNIActivity.GetActivity().runOnUiThread(new Runnable() {
					
					@Override
					public void run() {
						for (NativeEditText control: controls.values()) {
							if (control.editText.isShown())
							{
								control.editText.bringToFront();
								control.editText.setVisibility(View.VISIBLE);
								control.editText.invalidate();
							}
						}
					}
				});
			}
		}, 200);
	}

	public static native void TextFieldShouldReturn(int id);
	public static native boolean TextFieldKeyPressed(
			int id,
			int replacementLocation,
			int replacementLength,
			byte[] byteArray);
}
