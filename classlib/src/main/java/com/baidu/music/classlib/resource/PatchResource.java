package com.baidu.music.classlib.resource;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.util.Log;
import android.util.Xml;
import android.view.LayoutInflater;
import android.view.View;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;
import android.view.animation.AnimationSet;
import android.view.animation.RotateAnimation;
import android.view.animation.ScaleAnimation;
import android.view.animation.TranslateAnimation;

import com.baidu.music.classlib.utils.ApkUtils;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;

/**
 * 获取apk里面的资源文件
 * Created by Jarlene on 2015/11/23.
 */
public class PatchResource {

    public static final String TAG = PatchResource.class.getSimpleName();

    private Resources res;// 获取的资源apk里面的res
    private String apkPackageName;// 资源apk里面的包名
    private PatchContext mPatchContext;

    public PatchResource(Context context, String apkPatch) {
        mPatchContext = new PatchContext(context, apkPatch);
        res = mPatchContext.getResources();
        apkPackageName = ApkUtils.getPackageName(context, apkPatch);
        if (!apkPackageName.equals("com.baidu.music.classpatch")) {
            apkPackageName = "com.baidu.music.classpatch";
        }
    }


    public PatchResource(Resources res, String apkPackageName) {
        this.res = res;
        this.apkPackageName = apkPackageName;
    }

    /**
     * 获取layout文件中的id号
     *
     * @param layoutName
     *            layout名
     */
    public int getResApkLayoutId(String layoutName) {
        Log.d(TAG, "getResApkLayoutId");
        return res.getIdentifier(layoutName, "layout", apkPackageName);
    }

    /**
     * 获取布局layout文件
     *
     * @param context
     *            上下文
     * @params layoutName
     * @return view
     */
    public View getResApkLayoutView(Context context, String layoutName) {
        Log.d(TAG,"getResApkLayoutView");
        LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        return inflater.inflate(res.getLayout(getResApkLayoutId(layoutName)), null);
    }

    /**
     * 获取控件view的id号
     *
     * @param widgetName
     *            控件名
     */
    public int getResApkWidgetViewID(String widgetName) {
        Log.d(TAG,"getResApkWidgetViewID");
        return res.getIdentifier(widgetName, "id", apkPackageName);
    }

    /**
     * 获取布局文件中的控件
     *
     * @params layout,资源apk中的布局(view)
     * @params widgetName 控件名称
     * @return widgetView
     */
    public View getResApkWidgetView(View layout, String widgetName) {
        Log.d(TAG,"getResApkWidgetView");
        return layout.findViewById(getResApkWidgetViewID(widgetName));
    }

    /**
     * 获取drawable文件的id
     *
     * @param imgName
     *            图片名字
     */
    public int getDrawableId(String imgName) {
        Log.d(TAG,"getDrawableId");
        return res.getIdentifier(imgName, "drawable", apkPackageName);
    }

    /**
     * 获取图片资源
     *
     * @param imgName
     * @return drawable
     */
    public Drawable getResApkDrawable(String imgName) {
        Log.d(TAG,"getResApkDrawable");
        return res.getDrawable(getDrawableId(imgName));
    }

    /**
     * 获取string文件中的id号
     *
     * @param stringName
     *            字符串在String文件中的名字
     */
    public int getResApkStringId(String stringName) {
        Log.d(TAG,"getResApkStringId");
        return res.getIdentifier(stringName, "string", apkPackageName);
    }

    /**
     * 获取String字符串
     *
     * @param stringName
     * @return string
     */
    public String getResApkString(String stringName) {
        Log.d(TAG,"getResApkString");
        return res.getString(getResApkStringId(stringName));
    }

    /**
     * 获取anim文件中的id号
     *
     * @param animationName
     */
    public int getResApkAnimId(String animationName) {
        Log.d(TAG,"getResApkAnimId");
        return res.getIdentifier(animationName, "anim", apkPackageName);
    }

    /**
     * 获取anim文件 XmlPullParser
     *
     * @param animationName
     * @return XmlPullParser
     */
    public XmlPullParser getResApkAnimXml(String animationName) {
        Log.d(TAG,"getResApkAnimXml");
        return res.getAnimation(getResApkAnimId(animationName));
    }

    /**
     * 获取动画anim
     *
     * @params animationName
     * @param context
     */
    public Animation getResApkAnim(Context context, String animationName) {
        Log.d(TAG,"getResApkAnim");
        Animation animation = null;
        XmlPullParser parser = getResApkAnimXml(animationName);
        AttributeSet attrs = Xml.asAttributeSet(parser);
        try {
            animation = createAnimationFromXml(context, parser, null, attrs);
        } catch (XmlPullParserException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return animation;
    }

    /**
     * 获取anim动画
     */
    private Animation createAnimationFromXml(Context c, XmlPullParser parser,
                                             AnimationSet parent, AttributeSet attrs)
            throws XmlPullParserException, IOException {
        Log.d(TAG,"createAnimationFromXml");
        Animation anim = null;
        int type;
        int depth = parser.getDepth();
        while (((type = parser.next()) != XmlPullParser.END_TAG || parser.getDepth() > depth) && type != XmlPullParser.END_DOCUMENT) {

            if (type != XmlPullParser.START_TAG) {
                continue;
            }
            String name = parser.getName();
            if (name.equals("set")) {
                anim = new AnimationSet(c, attrs);
                createAnimationFromXml(c, parser, (AnimationSet) anim, attrs);
            } else if (name.equals("alpha")) {
                anim = new AlphaAnimation(c, attrs);
            } else if (name.equals("scale")) {
                anim = new ScaleAnimation(c, attrs);
            } else if (name.equals("rotate")) {
                anim = new RotateAnimation(c, attrs);
            } else if (name.equals("translate")) {
                anim = new TranslateAnimation(c, attrs);
            } else {
                throw new RuntimeException("Unknown animation name: "+ parser.getName());
            }
            if (parent != null) {
                parent.addAnimation(anim);
            }
        }
        return anim;
    }

    /**
     * 获取 color文件中的id号
     *
     * @param colorName
     */
    public int getResApkColorId(String colorName) {
        Log.d(TAG,"getResApkColorId");
        return res.getIdentifier(colorName, "color", apkPackageName);
    }

    /**
     * 获取color 值
     *
     * @param colorName
     * @return int
     */

    public int getResApkColor(String colorName) {
        Log.d(TAG,"getResApkColor");
        return res.getColor(getResApkColorId(colorName));
    }

    /**
     * 获取 dimens文件中的id号
     *
     * @param dimenName
     */
    public int getResApkDimensId(String dimenName) {
        Log.d(TAG,"getResApkDimensId");
        return res.getIdentifier(dimenName, "dimen", apkPackageName);
    }

    /**
     * 获取dimens文件中值
     *
     * @param dimenName
     * @return float
     */
    public float getResApkDimens(String dimenName) {
        Log.d(TAG,"getResApkDimens");
        return res.getDimension(getResApkDimensId(dimenName));
    }
}
