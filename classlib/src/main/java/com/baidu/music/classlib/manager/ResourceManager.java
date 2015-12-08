package com.baidu.music.classlib.manager;

import android.content.Context;
import android.text.TextUtils;

import com.baidu.music.classlib.resource.PatchContext;
import com.baidu.music.classlib.resource.PatchResource;

/**
 * patch Apk的资源文件管理
 * Created by Jarlene on 2015/12/2.
 */
public class ResourceManager extends BaseManager<String, PatchResource>{

    private static ResourceManager instance = null;

    /**
     * 获得Patch单例实例
     *
     * @return 管理器实例
     */
    public static ResourceManager getInstance() {
        if (instance == null) {
            synchronized (ResourceManager.class) {
                if (instance == null) {
                    instance = new ResourceManager();
                }
            }
        }
        return instance;
    }

    /**
     * 获取PatchResource
     * @param context
     * @param apkPath
     * @return
     */
    public PatchResource getPatchResource(Context context, String apkPath) {
        PatchResource patchResource = getItem(apkPath);
        if (patchResource == null) {
            if (context == null) {
                return null;
            }
            PatchContext patchContext = ContextManager.getInstance().getContext(context, apkPath);
            if (patchContext != null) {
                patchResource = new PatchResource(patchContext, apkPath);
            }
            if (patchResource != null) {
                addItem(apkPath, patchResource);
            }
        }
        return patchResource;
    }

    /**
     * 移除Context
     * @param apkPath
     */
    public void removePatchResource(String apkPath) {
        if (TextUtils.isEmpty(apkPath)) {
            throw new NullPointerException("Context is null");
        }
        removeItem(apkPath);
    }
}
