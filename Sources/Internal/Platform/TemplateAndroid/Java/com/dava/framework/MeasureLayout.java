package com.dava.framework;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.FrameLayout;

public class MeasureLayout extends FrameLayout {
	
	boolean isOnMeasure = false;

	public MeasureLayout(Context context) {
		super(context);
	}
	
	public MeasureLayout(Context context, AttributeSet attrs) {
		super(context, attrs);
	}
	
	public MeasureLayout(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
	}

	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		super.onMeasure(widthMeasureSpec, heightMeasureSpec);
		
		if (!isOnMeasure) {
			isOnMeasure = true;
			JNITextField.Invalidate();
			isOnMeasure = false;
		}
	}
	
}
