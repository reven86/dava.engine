package com.dava.framework;

import java.util.concurrent.Callable;
import java.util.concurrent.FutureTask;

import android.text.InputType;
import android.view.inputmethod.EditorInfo;
import android.widget.EditText;

public class JNITextField {
	final static String TAG = "JNITextField";
	
	public static void ShowField(
			final float x,
			final float y,
			final float dx,
			final float dy,
			final String defaultText,
			final boolean isPassword,
			final int autoCapitalizationType,	//eAutoCapitalizationType autoCapitalizationType;
			final int autoCorrectionType,		//eAutoCorrectionType autoCorrectionType;
			final int spellCheckingType,		//eSpellCheckingType spellCheckingType;
			final int keyboardAppearanceType,	//eKeyboardAppearanceType keyboardAppearanceType;
			final int keyboardType,				//eKeyboardType keyboardType;
			final int returnKeyType)			//eReturnKeyType returnKeyType
	{
		final FutureTask<Void> task = new FutureTask<Void> (new Callable<Void>() {
			@Override
			public Void call() throws Exception {
				EditText editText = JNIActivity.GetActivity().ShowEditText(x, y, dx, dy, defaultText, isPassword);
				if (isPassword)
				{
					editText.setInputType(EditorInfo.TYPE_CLASS_TEXT | EditorInfo.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD);
				}
				else
				{
					SetStyle(editText,
						autoCapitalizationType,
						autoCorrectionType,
						spellCheckingType,
						keyboardAppearanceType,
						keyboardType,
						returnKeyType);
				}
				return null;
			}
		});
		
		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
			@Override
			public void run() {
				task.run();
			}
		});
		
		while (!task.isDone())
			Thread.yield();
	}
	
	static String text;
	public static void HideField()
	{
		final FutureTask<Void> task = new FutureTask<Void> (new Callable<Void>() {
			@Override
			public Void call() throws Exception {
				JNIActivity activity = JNIActivity.GetActivity();
				text = activity.GetEditText();
				activity.HideEditText();
				return null;
			}
		});
		
		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
			@Override
			public void run() {
				task.run();
			}
		});
		
		while (!task.isDone())
			Thread.yield();
		
		FieldHiddenWithText(text);
	}
	
	private static void SetStyle(
			EditText editText,
			final int autoCapitalizationType,	//eAutoCapitalizationType autoCapitalizationType;
			final int autoCorrectionType,		//eAutoCorrectionType autoCorrectionType;
			final int spellCheckingType,		//eSpellCheckingType spellCheckingType;
			final int keyboardAppearanceType,	//eKeyboardAppearanceType keyboardAppearanceType;
			final int keyboardType,				//eKeyboardType keyboardType;
			final int returnKeyType)			//eReturnKeyType returnKeyType
	{
		int inputFlags = 0;
		
		switch (keyboardType) {
		case 2:		//KEYBOARD_TYPE_NUMBERS_AND_PUNCTUATION
		case 4:		//KEYBOARD_TYPE_NUMBER_PAD
		case 5:		//KEYBOARD_TYPE_PHONE_PAD
		case 6:		//KEYBOARD_TYPE_NAME_PHONE_PAD
		case 8:		//KEYBOARD_TYPE_DECIMAL_PAD
			inputFlags |= InputType.TYPE_CLASS_NUMBER;
			break;
			
		case 3:		//KEYBOARD_TYPE_URL
		case 7:		//KEYBOARD_TYPE_EMAIL_ADDRESS
		case 9:		//KEYBOARD_TYPE_TWITTER
			inputFlags |= InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_URI;
			break;

		case 0:		//KEYBOARD_TYPE_DEFAULT
		case 1:		//KEYBOARD_TYPE_ASCII_CAPABLE
		default:
			inputFlags |= EditorInfo.TYPE_CLASS_TEXT;// | EditorInfo.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD;
			break;
		};
		
		switch (autoCapitalizationType)
		{
		case 1:		//AUTO_CAPITALIZATION_TYPE_WORDS
			inputFlags |= InputType.TYPE_TEXT_FLAG_CAP_WORDS;
			break;
		case 2:		//AUTO_CAPITALIZATION_TYPE_SENTENCES
			inputFlags |= InputType.TYPE_TEXT_FLAG_CAP_SENTENCES;
			break;
		case 3:		//AUTO_CAPITALIZATION_TYPE_ALL_CHARS
			inputFlags |= InputType.TYPE_TEXT_FLAG_CAP_CHARACTERS;
			break;
		case 0:		//AUTO_CAPITALIZATION_TYPE_NONE
		default:
			break;
		};
		
		switch (autoCorrectionType)
		{
		case 0:		//AUTO_CORRECTION_TYPE_DEFAULT
		case 2:		//AUTO_CORRECTION_TYPE_YES
			inputFlags |= InputType.TYPE_TEXT_FLAG_AUTO_CORRECT;
			break;
		case 1: 	//AUTO_CORRECTION_TYPE_NO
		default:
			break;
		};
		
		switch (spellCheckingType)
		{
		case 0:		//SPELL_CHECKING_TYPE_DEFAULT
		case 2:		//SPELL_CHECKING_TYPE_YES
			inputFlags |= InputType.TYPE_TEXT_FLAG_AUTO_COMPLETE;
			break;
		case 1:		//SPELL_CHECKING_TYPE_NO
		default:
			break;
		};

		int imeOptions = 0;//EditorInfo.IME_ACTION_DONE;
		switch (returnKeyType) {
		case 1:		//RETURN_KEY_GO,
			imeOptions = EditorInfo.IME_ACTION_GO;
			break;
		case 2:		//RETURN_KEY_GOOGLE,
		case 6:		//RETURN_KEY_SEARCH,
		case 8:		//RETURN_KEY_YAHOO,
			imeOptions = EditorInfo.IME_ACTION_SEARCH;
			break;
		case 4:		//RETURN_KEY_NEXT,
			imeOptions = EditorInfo.IME_ACTION_NEXT;
			break;
		case 7:		//RETURN_KEY_SEND,
			imeOptions = EditorInfo.IME_ACTION_SEND;
			break;
		case 0:		//RETURN_KEY_DEFAULT = 0,
		case 3:		//RETURN_KEY_JOIN,
		case 5:		//RETURN_KEY_ROUTE,
		case 9:		//RETURN_KEY_DONE,
		case 10:	//RETURN_KEY_EMERGENCY_CALL
		default:
			imeOptions = EditorInfo.IME_ACTION_DONE;
			break;
		}
		
		editText.setInputType(inputFlags);
		editText.setImeOptions(imeOptions);
	}
	
	public static native void FieldHiddenWithText(String text);
	public static native void TextFieldShouldReturn();
	public static native boolean TextFieldKeyPressed(int replacementLocation, int replacementLength, String replacementString);
}
