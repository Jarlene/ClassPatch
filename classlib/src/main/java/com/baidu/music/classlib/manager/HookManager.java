package com.baidu.music.classlib.manager;

import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;
import android.text.TextUtils;

import com.baidu.music.classlib.build.DexClient;
import com.baidu.music.classlib.listener.FileOperatorListener;
import com.baidu.music.classlib.utils.ApkUtils;
import com.baidu.music.classlib.utils.FileUtils;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

/**
 * 如果想对UI界面进行调整，要将相关的资源文件打包进patch中，
 * 我已经为其提供了PatchResource工具，在主apk中，
 * 在用到资源文件的时候，使用就行了。
 * Created by Jarlene on 2015/11/23.
 */
public class HookManager extends BaseManager<String, File>{


    private static volatile HookManager instance;
    private final IOHandlerProvider ioHandlerProvider;

    private HookManager() {
        ioHandlerProvider = new IOHandlerProvider();
    }

    public static HookManager getInstance() {
        if (instance == null) {
            synchronized (HookManager.class) {
                if (instance == null) {
                    instance = new HookManager();
                }
            }
        }
        return instance;
    }

    /**
     * 添加patch
     * @param patchName
     * @param file
     */
    public void addPatch(String patchName, File file) {
        if ((TextUtils.isEmpty(patchName)) || file == null || !file.exists()) {
            return;
        }
        addItem(patchName, file);
    }

    /**
     * 移除patch
     * @param patchName
     */
    public void removePatch(String patchName) {
        if ((TextUtils.isEmpty(patchName))) {
            return;
        }
        removeItem(patchName);
    }

    /**
     * 获取Patch
     * @param patchName
     * @return
     */
    public File getPatch(String patchName) {
        if ((TextUtils.isEmpty(patchName))) {
            return null;
        }
        return getItem(patchName);
    }

    /**
     * 验证patch的有效性
     * 这个验证有点草率，以后会采用更好的验证
     * @param context
     * @param apkPath
     * @return
     */
    public boolean isValidate(Context context, String apkPath) {
        boolean validate = false;
        if (!TextUtils.isEmpty(apkPath)) {
            File apkFile = new File(apkPath);
            if (apkFile.getAbsolutePath().endsWith(".apk")) {
                if (ApkUtils.getPackageInfo(context, apkPath) != null) {
                    validate = true;
                }
            }
        }
        return validate;
    }

    /**
     * 取得patch的版本
     * 这个有点草率，以后修改
     * @param context
     * @param apkPath
     * @return
     */
    public int getVersion(Context context, String apkPath) {
        return ApkUtils.getApkVersio(context, apkPath);
    }

    /**
     * 去掉无效的patch
     * @param context
     * @param files
     * @return
     */
    public List<File> patchFileFilter(Context context, List<File> files) {
        if (context == null || files == null) {
            throw new NullPointerException("Context is null or files is Empty");
        }
        List<File> fileList = new ArrayList<File>();
        for (File file : files) {
            if (isValidate(context, file.getAbsolutePath())) {
                fileList.add(file);
            }
        }
        return fileList;
    }

    /**
     * 将文件拷贝到相关的目录下。
     * @param context
     * @param apkPath
     * @param listener
     */
    public void copyFile(final Context context, final String apkPath,
                         final FileOperatorListener listener) {
        if (TextUtils.isEmpty(apkPath) || context == null || listener == null) {
            throw new NullPointerException("apkPath is null or context is null or callback is null");
        }
        ioHandlerProvider.getIOHandler().post(new Runnable() {
            @Override
            public void run() {
                try {
                    if (!isValidate(context, apkPath)) {
                        listener.notifyError(FileOperatorListener.ERROR_CODE_INVALIDATE);
                    } else {
                        File srcFile = new File(apkPath);
                        File destFile = new File(context.getDir("patch", Context.MODE_PRIVATE),
                                srcFile.getName());
                        if (destFile.exists()) {
                            if (getVersion(context, apkPath) > getVersion(context,
                                    destFile.getAbsolutePath())){
                                destFile.delete();
                                FileUtils.copyFile(srcFile, destFile);
                                listener.notifyCompleted();
                            } else {
                                listener.notifyError(FileOperatorListener.ERROR_VERSION_CODE);
                            }
                        } else {
                            FileUtils.copyFile(srcFile, destFile);
                            listener.notifyCompleted();
                        }
                    }
                } catch (Exception e) {
                    listener.notifyError(FileOperatorListener.ERROR_CODE_UNKNOW);
                    e.printStackTrace();
                }
            }
        });
    }

    /**
     * 生成dex文件 ，主要是在线程中做，通知回调
     *   主要是将.class文件和。jar文件生成dex
     * @param classDir
     * @param dexPath
     * @param listener
     */
    public void makeDex(final String classDir, final String dexPath,
                        final FileOperatorListener listener) {
        if (TextUtils.isEmpty(classDir) || TextUtils.isEmpty(dexPath) || listener == null) {
            throw new NullPointerException("classDir is null or dexPath is null or listener is null");
        }
        ioHandlerProvider.getIOHandler().post(new Runnable() {
            @Override
            public void run() {
                File classPathFileDir = new File(classDir);
                if (!classPathFileDir.exists()) {
                    return;
                }
                File[] classPathF = classPathFileDir.listFiles();
                if (classPathF != null && classPathF.length > 0) {
                    int size = classPathF.length;
                    List<String> classList = new ArrayList<String>();
                    for (int i = 0; i < size; i++) {
                        String path = classPathF[i].getAbsolutePath();
                        if (path.endsWith(".class") || path.endsWith(".jar")) {
                            classList.add(classPathF[i].getAbsolutePath());
                        }
                    }
                    DexClient client = new DexClient();
                    client.makeDex(classList, dexPath);
                }
                if (FileUtils.isExist(dexPath)) {
                    listener.notifyCompleted();
                } else {
                    listener.notifyError(FileOperatorListener.ERROR_CODE_UNKNOW);
                }
            }
        });
    }

    /**
     * 获取patch存放的位置目录
     *    data/data/packageName/aap_patch/
     * @param context
     * @return
     */
    public File getPatchDir(Context context) {
        if (context == null) {
            throw new NullPointerException("context is null");
        }
        return context.getDir("patch", Context.MODE_PRIVATE);
    }

    /**
     * 获取patch opt后存放的位置目录
     *    data/data/packageName/aap_patchOpt/
     * @param context
     * @return
     */
    public File getPatchOptDir(Context context) {
        if (context == null) {
            throw new NullPointerException("context is null");
        }
        return context.getDir("patchOpt", Context.MODE_PRIVATE);
    }

    /**
     * IO操作线程
     */
    private static class IOHandlerProvider {

        private Handler mIOHandler;
        private IOHandlerProvider(){
            HandlerThread thread = new HandlerThread("patch");
            thread.start();
            mIOHandler = new Handler(thread.getLooper());
        }
        public Handler getIOHandler(){
            return mIOHandler;
        }
    }

}
