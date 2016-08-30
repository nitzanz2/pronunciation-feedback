package com.example.hadas.project_final;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.view.View;
import android.widget.TextView;

import org.json.JSONArray;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

/**
 * class Globals- holds the word list
 */
public class Globals {
    private static Globals instance;

    // Global variable
    private List<Word> data;
    private  List<String> favorites;
    private  List<String> history;
    SharedPreferences sharedPref ;
    SharedPreferences.Editor editor;
    // Restrict the constructor from being instantiated
    private Globals(Context c){
        readWords(c);
    }

    public void setData(List<Word> d){
        this.data=new ArrayList<Word>(d);
    }
    public List<Word> getData(){
        return this.data;
    }
    /**
     * setItem- set word in index i
     */
    public void setItem(int i,Word w) {
        this.data.set(i,w);
    }
    /**
     * getInstance- if the list was already initialized return the list,
     * else, create the list.
     */
    public static synchronized Globals getInstance(Context c){
        if(instance==null){
            instance=new Globals(c);
        }
        return instance;
    }
    /**
     * readWords- read the words from file to list
     */
    public void readWords(Context c) {

        AssetManager am = c.getAssets();
        String sCurrentLine;

        sharedPref = c.getSharedPreferences("pref",
                c.getApplicationContext().MODE_PRIVATE);
        editor = sharedPref.edit();
        InputStream is;
        BufferedReader br = null;
        this.data = new ArrayList<Word>();
        try {
            is = am.open("words_list.txt");
            br = new BufferedReader(new InputStreamReader(is));

            while ((sCurrentLine = br.readLine()) != null) {
                String[] temp = sCurrentLine.split("/");
                String title=temp[0].replace(" ", "");
                String phones=temp[1];
                String[] syl = temp[2].split(" ");
                Word t=new Word(title, phones,syl);
                getFav();
                getHistory();
                if(favorites.contains(t.getTitle())) {
                    t.setFvr(true);
                }
                if(history.contains(t.getTitle())) {
                    t.setHistory(true);
                }
                this.data.add(t);

            }

        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                if (br != null) br.close();
            } catch (IOException ex) {
                ex.printStackTrace();
            }
        }

    }

    /**
     * getFav - read favorites list from shared preferences and update the list
     */
    public void getFav() {
        favorites=new ArrayList<String>();
        //import favorites list from the shared pref
        String my = sharedPref.getString("fvr", "");
        //try to remove and put it back to the shared pref
        try {
            if (my.compareTo("") != 0) {
                JSONArray fvrs = new JSONArray(my);
                //add the word to the list
                for (int i = 0; i < fvrs.length(); i++) {
                    String data = fvrs.getString(i);
                    favorites.add(data);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    /**
     * getHistory - read history list from shared preferences and update the list
     */
    public void getHistory() {
        history=new ArrayList<String>();
        //import history list from the shared pref
        String my = sharedPref.getString("hislis", "");
        //try to remove and put it back to the shared pref
        try {
            if (my.compareTo("") != 0) {
                JSONArray his = new JSONArray(my);
                //add the word to my list
                for (int i = 0; i < his.length(); i++) {
                    String data = his.getString(i);
                    history.add(data);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
