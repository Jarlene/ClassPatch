package com.baidu.music.classlib.jni;

import android.os.Build;
import android.text.TextUtils;

import java.lang.reflect.Member;
import java.lang.reflect.Method;
import java.util.List;

/**
 * Jni接口，只对dalvik有效，art模式不需要，原因是在art模式下，可以原生实现class补丁。
 *  但是method替换在art模式下不能使用，之后会将这部分代码单独移除。
 * Created by Jarlene on 2015/11/23.
 */
public final class HookBridge {

    private static final String vmVersion = System.getProperty("java.vm.version");
    private static final boolean isArt = (vmVersion != null && vmVersion.startsWith("2"));

    static{
        try {
            System.loadLibrary("substrate");
            System.loadLibrary("substrate-dvm");
            System.loadLibrary("commonHook");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * 初始化
     * @return
     */
    public static boolean initJNIEnv() {
        if (isArt) {
            return false;
        }
        try {
            int apiLevel = Build.VERSION.SDK_INT;
            return initHookEnv(isArt, apiLevel);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }

    /**
     * hook Java层Method
     * @param src
     * @param target
     */
    public static void hookJavaMethod(Method src, Method target) {
        if (src == null || target == null) {
            return;
        }
        if (isArt) {
            return;
        }
        try {
            replaceJavaMethod(src, target);
        } catch (Exception e) {
            e.printStackTrace();
        }

    }

    /**
     * hook Native层 Method
     * @param oldSoName
     * @param newSoName
     * @param methodName
     * @param sym
     */
    public static void hookNativeMethod(String oldSoName, String newSoName,
                                        String methodName, String sym) {
        if (TextUtils.isEmpty(oldSoName) || TextUtils.isEmpty(newSoName)
                || TextUtils.isEmpty(sym) || TextUtils.isEmpty(methodName)) {
            return;
        }
        if (isArt) {
            return;
        }
        try {
            replaceNativeMethod(oldSoName, newSoName, methodName, sym);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * 添加过滤的class名，用于实时生效。
     * @param clazzNames
     */
    public static void classesFilter(List<String> clazzNames) {
        if (clazzNames != null && !clazzNames.isEmpty()) {
            String[] clazz = new String[clazzNames.size()];
            int i = 0;
            for (String cl : clazzNames) {
                clazz[i++] = cl;
            }
            classesResolvedFilter(clazz);
        }
    }

    private static native boolean initHookEnv(boolean isArt, int apiLevel);

    private static native void replaceJavaMethod(Member src, Member target);

    private static native void replaceNativeMethod(String oldSoName, String newSoName,
                                                   String oldSymbol, String newSymbol);

    private static native void classesResolvedFilter(String[] classNames);

}
