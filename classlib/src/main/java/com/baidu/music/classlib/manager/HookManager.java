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
import java.io.IOException;
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

    private String signture = null;

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
     * 注册签名信息，所以有的patch apk签名都要一样。
     * @param signture
     */
    public void registerSignture(String signture) {
        if (TextUtils.isEmpty(signture)) {
            throw  new NullPointerException("the signture is null");
        }
        this.signture = signture;
    }

    /**
     * 检查签名是否有效
     */
    private void checkSignture() {
        if (TextUtils.isEmpty(signture)) {
            throw  new NullPointerException("the signture is null");
        }
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
     * 采用签名进行验证
     * @param context
     * @param apkPath
     * @return
     */
    public boolean isValidate(Context context, String apkPath) {
        boolean validate = false;
        if (!TextUtils.isEmpty(apkPath)) {
            File apkFile = new File(apkPath);
            if (apkFile.getAbsolutePath().endsWith(".apk")) {
                if (ApkUtils.getPackageInfo(context, apkPath) != null
                        /*&& signture.equals(ApkUtils.getApkSignatures(context,apkPath)[0])*/) {
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
//        checkSignture();
        List<File> fileList = new ArrayList<File>();
        for (File file : files) {
            if (isValidate(context, file.getAbsolutePath())) {
                fileList.add(file);
            }
        }
        return fileList;
    }

    /**
     * 将Apk文件拷贝到相关的目录下。
     * @param context
     * @param apkPath
     * @param listener
     */
    public void copyFile(final Context context, final String apkPath,
                         final FileOperatorListener listener) {
        if (TextUtils.isEmpty(apkPath) || context == null || listener == null) {
            throw new NullPointerException("apkPath is null or context is null or callback is null");
        }
        final File srcFile = new File(apkPath);
        final File destFile = new File(getPatchDir(context), srcFile.getName());
        if (destFile.exists()) {
            if (getVersion(context, apkPath) > getVersion(context, destFile.getAbsolutePath())) {
                gotoCopy(srcFile, destFile, listener);
            } else {
                listener.notifyError(FileOperatorListener.ERROR_VERSION_CODE);
            }
        } else {
            gotoCopy(srcFile, destFile, listener);
        }
    }

    /**
     * 执行文件复制操作
     * @param srcFile
     * @param destFile
     * @param listener
     */
    private void gotoCopy(final File srcFile, final File destFile, final FileOperatorListener listener) {
        ioHandlerProvider.getIOHandler().post(new Runnable() {
            @Override
            public void run() {
                try {
                    FileUtils.copyFile(srcFile, destFile);
                    listener.notifyCompleted();
                } catch (IOException e) {
                    listener.notifyError(FileOperatorListener.ERROR_CODE_UNKNOW);
                    e.printStackTrace();
                }
            }
        });
    }

    /**
     * 将文件复制到相关目录下面
     * @param context
     * @param savePath
     * @param srcPath
     * @param listener
     */
    public void copyFile(final Context context, final String savePath, final String srcPath,
                         final FileOperatorListener listener) {
        if (TextUtils.isEmpty(srcPath) || TextUtils.isEmpty(savePath)
                || context == null || listener == null) {
            throw new NullPointerException("savePath or srcPath is null or context is null or callback is null");
        }
        File srcFile = new File(srcPath);
        File destFile = new File(savePath);
        gotoCopy(srcFile, destFile, listener);
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
     * so patch保存的路径
     * @param context
     * @return
     */
    public File getSoPatchDir(Context context) {
        if (context == null) {
            throw new NullPointerException("context is null");
        }
        return context.getDir("soPatch", Context.MODE_PRIVATE);
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
