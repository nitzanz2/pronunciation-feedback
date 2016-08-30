/*
 204053268 Hadas Cohen
 203571435 Einav Saad
 204208714 Nitzan Zeira
 */
package com.example.hadas.project_final;

import android.content.Intent;
import android.graphics.Typeface;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * class WordsActivity- display screen with words list
 */
public class WordsActivity extends BaseActivity implements View.OnClickListener {
    public static List<Word> words;
    WordAdapter wordAdapter;
    Map<String, Integer> mapIndex;
    ListView lst;
    List<Word> copy;
    String[] syl;
    Globals g;
    boolean favOn;
    Button favs;
    EditText inputSearch;
    /**
     * onCreate- create activity of words list.
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_words);
        g=Globals.getInstance(this.getApplicationContext());
        words=g.getData();
        addListener();
        copy=new ArrayList<Word>(words);
        lst = (ListView)findViewById(R.id.lstWords);
        //set the adapter
        wordAdapter = new WordAdapter(WordsActivity.this, copy);
        lst.setAdapter(wordAdapter);
        inputSearch = (EditText) findViewById(R.id.search);
        /**
         * Enabling Search Filter
         * */
        inputSearch.addTextChangedListener(new TextWatcher() {
            @Override
            public void onTextChanged(CharSequence cs, int arg1, int arg2, int arg3) {
                // When user changed the Text
                copy.clear();
                String search = cs.toString();
                for (Word o : words) {
                    if (o.getTitle().startsWith(search.toLowerCase()))
                        if((favOn&&o.getFvr()) ||!favOn)
                            copy.add(o);
                    wordAdapter.notifyDataSetChanged();
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
        getIndexList();
        displayIndex();
        favs=(Button)findViewById(R.id.btnFav);
        favOn=false;
        favs.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(!favOn) {
                    copy.clear();
                    for (Word o : words) {
                        if (o.getFvr())
                            copy.add(o);
                        wordAdapter.notifyDataSetChanged();
                    }
                    favOn=true;
                    favs.setText("all");
                    inputSearch.setText("");
                }
                else {
                    copy.clear();
                    copy.addAll(words);
                    wordAdapter.notifyDataSetChanged();
                    favOn=false;
                    favs.setText("favorites");
                    inputSearch.setText("");
                }
            }
        });
    }

    /**
     * onResume- clear the search bar and update the favorites
     */
    @Override
    public void onResume() {
        super.onResume();
        copy.clear();
        if(favOn) {
            for (Word o : words) {
                if (o.getFvr())
                    copy.add(o);
            }
        }
        else
            copy.addAll(g.getData());
        wordAdapter.notifyDataSetChanged();
        inputSearch.setText("");
    }

    /**
     * addListener- for each word, add an onClickListener to its record activity with its data:
     * (phones, syllable,word and lists' index)
     */
    private void addListener() {
        copy=new ArrayList<Word>();
        for (Word w:words) {
            View.OnClickListener listener=new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    Intent intent = new Intent(WordsActivity.this, RecordActivity.class);
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
     * getIndexList- set the words list indexes
     */
    private void getIndexList() {
        mapIndex = new LinkedHashMap<String, Integer>();
        for (int i = 0; i < words.size(); i++) {
            String word = words.get(i).getTitle();
            String index = word.substring(0, 1);

            if (mapIndex.get(index) == null)
                mapIndex.put(index, i);
        }
    }

    /**
     * displayIndex- display the words indexes
     */
    private void displayIndex() {
        LinearLayout indexLayout = (LinearLayout) findViewById(R.id.side_index);
        TextView textView;
        List<String> indexList = new ArrayList<String>(mapIndex.keySet());
        for (String index : indexList) {
            textView = (TextView) getLayoutInflater().inflate(
                    R.layout.side_index_item, null);
            textView.setText(index);
            textView.setOnClickListener(this);
            indexLayout.addView(textView);
            final Typeface font = Typeface.createFromAsset(this.getApplicationContext().getAssets(), "fonts/Bahamas-Light.ttf");
            textView.setTypeface(font);
        }
    }

    /**
     * onClick- the scroll bar listener.
     */
    public void onClick(View view) {
        TextView selectedIndex = (TextView) view;
        inputSearch.setText("");
        lst.setSelection(mapIndex.get(selectedIndex.getText()));
    }
}

