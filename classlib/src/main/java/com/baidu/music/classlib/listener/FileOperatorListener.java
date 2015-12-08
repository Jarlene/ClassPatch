package com.baidu.music.classlib.listener;

/**
 * 文件操作回调
 * Created by Jarlenr on 2015/10/15.
 */
public interface FileOperatorListener {

    public static final int ERROR_CODE_NOSPACE = 0; // 空间不足
    public static final int ERROR_CODE_UNKNOW = 1; // 未知错误
    public static final int ERROR_CODE_INVALIDATE = 2; // 文件无效
    public static final int ERROR_VERSION_CODE = 3; // 版本错误

    void notifyCompleted();
    void notifyError(int errorCode);
}
