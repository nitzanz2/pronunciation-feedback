package com.example.hadas.project_final;

import android.content.SharedPreferences;
import android.graphics.Typeface;
import android.media.MediaPlayer;
import android.os.Environment;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.util.Base64;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;

import org.json.JSONArray;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * class HistoryActivity - 10 last scores for each word
 */
public class HistoryActivity extends BaseActivity {
    ScoreAdapter adapter;
    List<Score> hist;
    ListView lst;
    String word;
    String wav;
    SharedPreferences sharedPref ;
    SharedPreferences.Editor editor;
    String wavFile;
    @Override
    /**
     * onCreate - display the 10 last scores for the word
     */
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        hist=new ArrayList<Score>();
        Bundle extras = getIntent().getExtras();
        if (extras != null) {
            word = extras.getString("word");
            wav = extras.getString("wav");

        }
        sharedPref = this.getSharedPreferences("pref",
                this.getApplicationContext().MODE_PRIVATE);
        editor = sharedPref.edit();

        String my = sharedPref.getString(word, "");
        JSONArray his;
        String wa = sharedPref.getString(word+"-wav", "");
        JSONArray wavs;
        //add channel to the list and update the shared pref
        try {
            if (my.compareTo("") != 0) {
                his = new JSONArray(my);
                wavs=new JSONArray(wa);
            }
            else {
                his=new JSONArray();
                wavs=new JSONArray();
            }
            for(int i=0;i<his.length();i++) {
                Score s=new Score(his.getString(i),wavs.getString(i));
                wavFile=wavs.getString(i);
                hist.add(s);
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        setContentView(R.layout.activity_history);
        adapter = new ScoreAdapter(HistoryActivity.this, hist);
        lst = (ListView)findViewById(R.id.lstWords);
        lst.setAdapter(adapter);


        TextView txt= (TextView) findViewById(R.id.search);
        final Typeface font = Typeface.createFromAsset(this.getApplicationContext().getAssets(), "fonts/Bahamas-Bold.ttf");
        txt.setTypeface(font);
    }

}
