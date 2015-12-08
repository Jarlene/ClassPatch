package com.baidu.music.classlib.utils;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.Signature;
import android.text.TextUtils;

/**
 * 对Patch Apk进行处理
 * Created by Jarlene on 2015/11/23.
 */
public class ApkUtils {


    /**
     * 获取Apk的版本
     * @param context
     * @param apkPath
     * @return
     */
    public static int getApkVersio(Context context, String apkPath){
        if (context == null || TextUtils.isEmpty(apkPath)) {
            throw new NullPointerException("Context is null or apk path is null");
        }
        PackageInfo info = getPackageInfo(context, apkPath);
        if (info != null) {
            return info.versionCode;
        }
        return -1;

    }


    /**
     * 获取pakageName
     * @param context
     * @param apkPath
     * @return
     */
    public static String getPackageName(Context context, String apkPath) {
        if (context == null || TextUtils.isEmpty(apkPath)) {
            throw new NullPointerException("Context is null or apk path is null");
        }
        PackageInfo info = getPackageInfo(context, apkPath);
        if (info != null) {
            return info.packageName;
        }
        return "";
    }

    /**
     * 获取Apk相关的信息
     * @param context
     * @param apkPath
     * @return
     */
    public static PackageInfo getPackageInfo(Context context, String apkPath){
        if (context == null || TextUtils.isEmpty(apkPath)) {
            throw new NullPointerException("Context is null or apk path is null");
        }
        return  context.getPackageManager().getPackageArchiveInfo(apkPath, 1);
    }

    /**
     * 获取Apk的签名证书。
     * @param context
     * @param apkPath
     * @return
     */
    public static Signature[] getApkSignatures(Context context, String apkPath) {
        if (context == null || TextUtils.isEmpty(apkPath)) {
            throw new NullPointerException("Context is null or apk path is null");
        }
        return  context.getPackageManager().getPackageArchiveInfo(apkPath,
                PackageManager.GET_SIGNATURES).signatures;
    }

}
