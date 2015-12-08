package com.baidu.music.classlib.manager;

import android.content.Context;
import android.text.TextUtils;

import com.baidu.music.classlib.resource.PatchContext;

/**
 * patch apk的伪Context管理类，主要是用于获取patch中的资源文件
 * Created by Jarlene on 2015/12/2.
 */
public class ContextManager extends BaseManager<String, PatchContext>{

    private static ContextManager instance = null;

    /**
     * 获得patch上下文管理实例
     *
     * @return 管理器实例
     */
    public static ContextManager getInstance() {
        if (instance == null) {
            synchronized (ContextManager.class) {
                if (instance == null) {
                    instance = new ContextManager();
                }
            }
        }
        return instance;
    }

    /**
     * 获取Context
     * @param context
     * @param apkPath
     * @return
     */
    public PatchContext getContext(Context context, String apkPath) {
        PatchContext patchContext = getItem(apkPath);
        if (patchContext == null) {
            if (context == null) {
                return null;
            }
            patchContext = new PatchContext(context, apkPath);
            if (patchContext != null) {
                addItem(apkPath, patchContext);
            }

        }
        return patchContext;
    }

    /**
     * 移除Context
     * @param apkPath
     */
    public void removeContext(String apkPath) {
        if (TextUtils.isEmpty(apkPath)) {
            throw new NullPointerException("Context is null");
        }
        removeItem(apkPath);
    }

}
