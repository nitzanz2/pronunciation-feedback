/*
 204053268 Hadas Cohen
 203571435 Einav Saad
 204208714 Nitzan Zeira
 */
package com.example.hadas.project_final;

import android.view.View;
/*
 * word item
 */
public class Word {
    private String title;
    private String phones;
    private String[] syl;
    View.OnClickListener listener;
    private boolean fvr;
    private boolean history;
    /*************************************************************************
     * function name: Word
     * The Input: title,phones, OnClickListener and syllables
     * The output: no output
     * The Function operation: constructor
     **************************************************************************/
    public Word(String title,String phones,View.OnClickListener listener,String[] syl){
        this.title = title;
        this.phones=phones;
        this.listener=listener;
        this.fvr=false;
        this.history=false;
        this.syl=syl;
    }

    /**
     * construct word with title, phones and syllables
     * @param title- title
     * @param phones- phones
     * @param syl- syllable
     */
    public Word(String title,String phones,String[] syl){
        this.title = title;
        this.phones=phones;
        this.syl=syl;
        this.listener=null;
    }
    /*************************************************************************
     * function name: getTitle
     * The Input: no input
     * The output: title
     * The Function operation: return the word title
     **************************************************************************/
    public String getTitle(){ return this.title; }
    /**
     * getPhones- return the words' phonemes
     * @return- the words' phonemes
     */
    public String getPhones(){ return this.phones; }

    /**
     * getPhones- return the words' syllables
     * @return- the words' syllables
     */
    public String[] getSyl(){ return this.syl; }
    /*************************************************************************
     * function name: getListener
     * The Input: no input
     * The output: listener
     * The Function operation: return the listener - what to do when pressing on
     * this word
     **************************************************************************/
    public View.OnClickListener getListener(){
        return this.listener;
    }

    /**
     * setFvr- set the words' favorite status
     * @param f- true/false
     */
    public void setFvr(boolean f){
        this.fvr=f;
    }

    /**
     * getFvr- return the true if the word's in the favorites
     * @return- true/false
     */
    public boolean getFvr(){
        return this.fvr;
    }

    /**
     * getHistory- return true if this word's in the history list
     * @return- true/false
     */
    public boolean getHistory() { return this.history;}

    /**
     * setListener- set the words' listener
     * @param listener
     */
    public void setListener(View.OnClickListener listener) {
        this.listener=listener;
    }

    /**
     * setHistory- set the words' history status
     * @param b- true/false
     */
    public void setHistory(boolean b) {
        this.history=b;
    }
}
