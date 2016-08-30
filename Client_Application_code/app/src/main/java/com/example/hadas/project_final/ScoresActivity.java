package com.example.hadas.project_final;

import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.graphics.Typeface;
import android.provider.UserDictionary;
import android.support.v4.app.FragmentActivity;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;

import org.json.JSONArray;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * ScoresActivity - list of words that had been recorded
 */
public class ScoresActivity extends BaseActivity {
    public static List<Word> words;
    HistoryAdapter historyAdapter;
    ListView lst;
    List<Word> copy;
    String[] syl;
    Globals g;
    boolean favOn;
    Button favs;
    SharedPreferences sharedPref ;
    SharedPreferences.Editor editor;
    EditText inputSearch;
    @Override
    /**
     * onCreate - create the words list and display it
     */
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_scores);
        g=Globals.getInstance(this.getApplicationContext());
        words=g.getData();
        addListener();
        sharedPref = this.getSharedPreferences("pref",
                this.getApplicationContext().MODE_PRIVATE);
        editor = sharedPref.edit();
        copy=new ArrayList<Word>();
        lst = (ListView)findViewById(R.id.lstWords);

        historyAdapter = new HistoryAdapter(ScoresActivity.this, copy);
        lst.setAdapter(historyAdapter);
        inputSearch = (EditText) findViewById(R.id.search);

        copy.clear();
        for (Word o : words) {
            if (o.getHistory()==true)
                copy.add(o);
        }
        historyAdapter.notifyDataSetChanged();

        inputSearch.addTextChangedListener(new TextWatcher() {

            @Override
            public void onTextChanged(CharSequence cs, int arg1, int arg2, int arg3) {
                // When user changed the Text
                copy.clear();
                String search = cs.toString();
                for (Word o : words) {
                    if (o.getTitle().startsWith(search.toLowerCase()))
                        if ((favOn && o.getFvr()) || !favOn)
                            copy.add(o);
                    historyAdapter.notifyDataSetChanged();
                }
            }


            @Override
            public void beforeTextChanged(CharSequence arg0, int arg1, int arg2,
                                          int arg3) {
                // TODO Auto-generated method stub

            }

            @Override
            public void afterTextChanged(Editable s) {

            }
        });
        inputSearch.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                v.setFocusable(true);
                v.setFocusableInTouchMode(true);
                return false;
            }
        });
        favs=(Button)findViewById(R.id.btnFav);
        favs.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                clear();
                for (int i=0;i<words.size();i++) {
                    Word temp = g.getData().get(i);
                    if (temp.getHistory()) {
                        temp.setHistory(false);
                        g.setItem(i, temp);
                    }
                }
                words=g.getData();
                copy.clear();
                for (Word o : words) {
                    if (o.getHistory()==true)
                        copy.add(o);
                }
                historyAdapter.notifyDataSetChanged();
            }
        });
    }
    @Override
    public void onResume() {
        super.onResume();

    }

    /**
     * addListener - add onclick listener for every word
     */
    private void addListener() {
        copy=new ArrayList<Word>();
        for (Word w:words) {
            View.OnClickListener listener=new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    Intent intent = new Intent(ScoresActivity.this, HistoryActivity.class);
                    String title=((TextView) v.findViewById(R.id.word)).getText().toString();
                    String phones="";
                    int i=0;
                    int index=0;
                    for (Word w:words) {
                        if(w.getTitle().equals(title)) {
                            phones=w.getPhones();
                            syl=w.getSyl();
                            index=i;
                        }
                        i++;
                    }
                    intent.putExtra("word",title);
                    intent.putExtra("phones",phones);
                    intent.putExtra("index",index);
                    intent.putExtra("syl",syl);
                    startActivity(intent);
                }
            };
            w.setListener(listener);
            copy.add(w);
        }

    }

    /**
     * clear - clear all scores and history
     */
    public void clear() {
        //import words list from the shared pref
        String my = sharedPref.getString("hislis", "");
        JSONArray his;
        //try to remove and put it back to the shared pref
        try {
            if (my.compareTo("") != 0) {
                his = new JSONArray(my);
                //put all the words in the list except for the specific word
                for (int i = 0; i < his.length(); i++) {
                    String data = his.getString(i);
                    editor.remove(data);
                    editor.remove(data+"-wav");
                }
                editor.remove("hislis");
                editor.commit();
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}

