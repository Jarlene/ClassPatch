package com.baidu.music.classpatch;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;

/**
 * Created by Jarlene on 2015/11/25.
 */
public class SecondActivity extends Activity {


    private static Context context;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        context = this;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        Log.d("SecondActivity", "destroy");
    }

}
