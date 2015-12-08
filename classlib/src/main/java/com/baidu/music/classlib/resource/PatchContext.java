package com.baidu.music.classlib.resource;

import android.content.Context;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.view.ContextThemeWrapper;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * 主要为patch apk实现资源提取（伪Context）
 * Created by Jarlene on 2015/12/1.
 */
public class PatchContext extends ContextThemeWrapper {

    private AssetManager mAssetManager;
    private Resources mResources;
    private Resources      mProxyResource;
    private Context mContext;
    private String mPatchPath;

    public PatchContext(Context base, String apkPath) {
        super(base, 0);
        this.mContext = base;
        this.mProxyResource = base.getResources();
        this.mPatchPath = apkPath;

    }

    @Override
    public Resources getResources() {
        if (mResources == null) {
            mResources = new Resources(getAssets(), mProxyResource.getDisplayMetrics(),
                    mProxyResource.getConfiguration());
        }
        return mResources;
    }

    @Override
    public AssetManager getAssets() {
        if (mAssetManager == null) {
            mAssetManager = (AssetManager) newInstanceObject(AssetManager.class);
            invokeMethod(mAssetManager, "addAssetPath", new Class[]{String.class}, new Object[]{mPatchPath});
        }
        return mAssetManager;
    }

    private Object invokeMethod(Object obj, String methodName, Class[] valueType, Object[] values) {
        try {
            Class<?> clazz = obj.getClass();
            Method method = clazz.getDeclaredMethod(methodName, valueType);
            method.setAccessible(true);
            return method.invoke(obj, values);
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        }
        return null;
    }

    private Object newInstanceObject(Class<?> clazz){
        try {
            return clazz.getConstructor().newInstance();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }
}
