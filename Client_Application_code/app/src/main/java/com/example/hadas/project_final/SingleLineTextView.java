package com.example.hadas.project_final;
/*
 204053268 Hadas Cohen
 203571435 Einav Saad
 204208714 Nitzan Zeira
 */
import android.content.Context;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.util.TypedValue;
import android.widget.TextView;

/**
 * class SingleLineTextView- display text in a single line
 */
public class SingleLineTextView extends TextView {
    // Attributes
    private Paint mTestPaint;
    private float defaultTextSize;

    /**
     * construct SingleLineTextView
     * @param context- context
     */
    public SingleLineTextView(Context context) {
        super(context);
        initialize();
    }

    /**
     * construct SingleLineTextView with attributeSet
     * @param context- context
     * @param attrs- attribiutes
     */
    public SingleLineTextView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initialize();
    }

    /**
     * initialize- set the paint and the text size
     */
    private void initialize() {
        mTestPaint = new Paint();
        mTestPaint.set(this.getPaint());
        defaultTextSize = getTextSize();
    }

    /* Re size the font so the specified text fits in the text box
     * assuming the text box is the specified width.
     */
    private void refitText(String text, int textWidth) {
        if (textWidth <= 0 || text.isEmpty())
            return;
        int targetWidth = textWidth - this.getPaddingLeft() - this.getPaddingRight();
        // this is most likely a non-relevant call
        if( targetWidth<=2 )
            return;
        // text already fits with the xml-defined font size?
        mTestPaint.set(this.getPaint());
        mTestPaint.setTextSize(defaultTextSize);
        if(mTestPaint.measureText(text) <= targetWidth) {
            this.setTextSize(TypedValue.COMPLEX_UNIT_PX, defaultTextSize);
            return;
        }
        // adjust text size using binary search for efficiency
        float hi = defaultTextSize;
        float lo = 2;
        final float threshold = 0.5f; // How close we have to be
        while (hi - lo > threshold) {
            float size = (hi + lo) / 2;
            mTestPaint.setTextSize(size);
            if(mTestPaint.measureText(text) >= targetWidth )
                hi = size; // too big
            else
                lo = size; // too small
        }
        // Use lo so that we undershoot rather than overshoot
        this.setTextSize(TypedValue.COMPLEX_UNIT_PX, lo);
    }

    /**
     * onMeasure- measure the dimensions according to width and heighgt
     * @param widthMeasureSpec
     * @param heightMeasureSpec
     */
    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        int parentWidth = MeasureSpec.getSize(widthMeasureSpec);
        int height = getMeasuredHeight();
        refitText(this.getText().toString(), parentWidth);
        this.setMeasuredDimension(parentWidth, height);
    }

    /**
     * onTextChanged- refit the text size when it's changed
     * @param text- the text
     * @param start- start
     * @param before- size before
     * @param after- size after
     */
    @Override
    protected void onTextChanged(final CharSequence text, final int start,
                                 final int before, final int after) {
        refitText(text.toString(), this.getWidth());
    }

    /**
     * onSizeChanged- refit the text when size changed
     * @param w- width
     * @param h- height
     * @param oldw- old width
     * @param oldh- old weight
     */
    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        if (w != oldw || h != oldh) {
            refitText(this.getText().toString(), w);
        }
    }
}