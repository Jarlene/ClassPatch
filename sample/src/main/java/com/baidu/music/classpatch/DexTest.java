package com.baidu.music.classpatch;

import android.content.Context;
import android.widget.Toast;

/**
 * Created by Jarlene on 2015/11/27.
 */
public class DexTest {

    private static String from = "来自Patch Dex";

    public void showToast(Context context) {
        Toast.makeText(context, "Toast from " + from, Toast.LENGTH_SHORT ).show();
    }


    public static String getStringStatic(String a, String b){
        b = b + from;
        return String.format("%s %s from getStringStatic  ", a, b);
    }

    public static String getTestString(String a, String b) {
        b = b + from;
        return "hook ok,  get string from getTestString" + a + b;
    }
}
