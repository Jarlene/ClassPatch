package com.baidu.music.classpatch;

import android.content.Context;
import android.os.Bundle;
import android.os.Environment;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import com.baidu.music.classlib.jni.HookBridge;
import com.baidu.music.classlib.listener.FileOperatorListener;
import com.baidu.music.classlib.manager.HookManager;
import com.baidu.music.classlib.manager.ResourceManager;
import com.baidu.music.classlib.resource.PatchContext;
import com.baidu.music.classlib.resource.PatchResource;
import com.baidu.music.classlib.utils.ApkUtils;

import java.io.File;
import java.lang.reflect.Method;


public class MainActivity extends AppCompatActivity implements View.OnClickListener{

    private Button classToDexBtn, showToastBtn, changeMethodBtn, changeSoBtn, copyFileBtn;

    private static Context mContext;

    private FileOperatorListener listener = new FileOperatorListener() {
        @Override
        public void notifyCompleted() {
            Toast.makeText(mContext, "复制成功",Toast.LENGTH_SHORT).show();
        }

        @Override
        public void notifyError(int errorCode) {
            Toast.makeText(mContext, "复制失败",Toast.LENGTH_SHORT).show();
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext = this;
        String apkPath = HookManager.getInstance().getPatchDir(mContext).getAbsolutePath() + File.separator + "DexTest.apk";
        PatchResource patchResource = ResourceManager.getInstance().getPatchResource(mContext, apkPath);
        int resId = patchResource.getResApkLayoutId("activity_main");
        if (resId <= 0) {
            setContentView(R.layout.activity_main);
        } else {
            setContentView(resId);
        }
        classToDexBtn = (Button) findViewById(R.id.classesToDex);
        showToastBtn = (Button) findViewById(R.id.showToast);
        changeMethodBtn = (Button) findViewById(R.id.changeMethod);
        changeSoBtn = (Button) findViewById(R.id.changeSo);
        copyFileBtn = (Button) findViewById(R.id.copyFile);

        classToDexBtn.setOnClickListener(this);
        showToastBtn.setOnClickListener(this);
        changeMethodBtn.setOnClickListener(this);
        changeSoBtn.setOnClickListener(this);
        copyFileBtn.setOnClickListener(this);

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
        fab.setOnClickListener(this);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        android.os.Process.killProcess(android.os.Process.myPid());
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }


    // native 执行该方法
    public static void toastSoString(String from) {
        Log.d("MainActivity", from);
        Toast.makeText(mContext, from, Toast.LENGTH_LONG).show();
    }


    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.classesToDex:
                classToDexButtonClicked();
                break;
            case R.id.showToast:
                showToastButtonClicked();
                break;
            case R.id.changeMethod:
                javaHookButtonClicked();
                break;
            case R.id.changeSo:
                nativeHookButtonClicked();
                break;
            case R.id.fab:
                Snackbar.make(v, "the class come from Main Dex", Snackbar.LENGTH_LONG)
                        .setAction("Action", null).show();
                break;
            case R.id.copyFile:
                String patch = Environment.getExternalStorageDirectory().getAbsolutePath()
                        + File.separator + "ClassPatch"
                        + File.separator + "DexTest.apk";
                HookManager.getInstance().copyFile(mContext, patch, listener);
                break;
            default:
                break;
        }
    }

    private void  classToDexButtonClicked() {
        String classPathDir = Environment.getExternalStorageDirectory().getAbsolutePath()
                + File.separator + "ClassPatch" + File.separator + "classes";
        String dexPath = new File(classPathDir, "classes.dex").getAbsolutePath();
        HookManager.getInstance().makeDex(classPathDir, dexPath, new FileOperatorListener() {
            @Override
            public void notifyCompleted() {
                Toast.makeText(mContext, "生成dex Ok", Toast.LENGTH_LONG).show();
            }

            @Override
            public void notifyError(int errorCode) {
                Toast.makeText(mContext, "生成dex Error", Toast.LENGTH_LONG).show();
            }
        });
    }

    private void showToastButtonClicked() {
        DexTest test = new DexTest();
        test.showToast(MainActivity.this);
    }

    private void nativeHookButtonClicked() {
        final String soPath = Environment.getExternalStorageDirectory().getAbsolutePath()
                + File.separator + "ClassPatch"
                + File.separator + "libpatch.so";
        final String destFile =  new File(HookManager.getInstance().getSoPatchDir(mContext),
                "libpatch.so").getAbsolutePath();
        HookManager.getInstance().copyFile(mContext, destFile, soPath, new FileOperatorListener() {
            @Override
            public void notifyCompleted() {
                try {
                    System.load(destFile);
                } catch (Exception e) {
                    e.printStackTrace();
                }
                HookBridge.hookNativeMethod("libcommonHook.so", soPath, "testString", "testString");
            }

            @Override
            public void notifyError(int errorCode) {
                Toast.makeText(mContext, "copy so error",Toast.LENGTH_SHORT).show();
            }
        });
    }

    private void javaHookButtonClicked() {
        try {
            Method src = DexTest.class.getDeclaredMethod("getStringStatic", int.class, String.class);
            Method target = DexTest.class.getDeclaredMethod("getTestString", int.class, String.class);
            HookBridge.hookJavaMethod(src, target);
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        }
        Toast.makeText(MainActivity.this, DexTest.getStringStatic(1024, " from aa "), Toast.LENGTH_SHORT).show();
    }
}
