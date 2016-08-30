package com.example.hadas.project_final;

import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;


import android.content.Intent;
import android.graphics.Typeface;
import android.os.Bundle;
import android.renderscript.Sampler;
import android.widget.TextView;

import java.util.List;


/**
 * class mainActivity- display home screen
 */
public class MainActivity extends BaseActivity {
    private int index = 0;
    static List<Word> words;
    /**
     * onCreate- display the activity with start screen
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Thread t = new Thread(new Runnable() {
            @Override
            public void run() {
                while (index < 3) {

                    try {
                        Thread.sleep(1000);
                        index +=1;
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
                //after 5 seconds, get to the menu screen
                Intent i = new Intent(MainActivity.this, MenuActivity.class);
                startActivity(i);
                finish();
            }
        });
        t.start();
    }
}
