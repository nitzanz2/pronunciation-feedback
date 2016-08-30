/* 8921104
 204053268 Hadas Cohen
 203571435 Einav Saad
 204208714 Nitzan Zeira
 203532064 Adar Kendelker
 */

package com.example.hadas.project_final;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Typeface;
import android.media.MediaPlayer;
import android.os.Environment;
import android.support.v4.app.FragmentActivity;
import android.text.Html;
import android.util.Base64;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Filter;
import android.widget.Filterable;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import org.json.JSONArray;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * ScoreAdapter - connect between the scores list and the listView
 */
public class ScoreAdapter extends BaseAdapter  {
    private Activity activity;
    private LayoutInflater inflater;
    private List<Score> items;
    SharedPreferences.Editor editor;
    SharedPreferences sharedPref ;
    boolean flag;
    ImageView fvr;
    Score item;

    /**
     * ScoreAdapter constructor - initialize the adapter
     * input - the activity that display the list.
     * a list of scores
     */
    public ScoreAdapter(Activity activity, List<Score> items) {
        this.activity = activity;
        this.items = items;
        sharedPref = activity.getSharedPreferences("pref",
                activity.getApplicationContext().MODE_PRIVATE);
        editor = sharedPref.edit();
    }

    /**
     * getCount - return the size of the list
     */
    @Override
    public int getCount() {
        return items.size();
    }

    /**
     * getItem - return item in given location.
     * input - location
     */
    @Override
    public Object getItem(int location) {
        return items.get(location);
    }

    /**
     * getItemId - return the item position
     * input - position
     */
    @Override
    public long getItemId(int position) {
        return position;
    }


    /**
     * getView - making a View for each item in the list.
     */
    @Override
    public View getView(int position, View convertView, ViewGroup parent) {

        if (inflater == null)
            inflater = (LayoutInflater) activity
                    .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        if (convertView == null)
            convertView = inflater.inflate(R.layout.score_item, null);
        item = items.get(position);

        TextView txtTitle = (TextView) convertView.findViewById(R.id.word);
        RelativeLayout layout = (RelativeLayout) convertView.findViewById(R.id.word_item);

        final Typeface font = Typeface.createFromAsset(activity.getApplicationContext().getAssets(), "fonts/Bahamas-Plain.ttf");
        txtTitle.setTypeface(font);

        txtTitle.setText(Html.fromHtml(item.getScore()), TextView.BufferType.SPANNABLE);

        layout.setOnClickListener(item.getListener());
        return convertView;
    }


}