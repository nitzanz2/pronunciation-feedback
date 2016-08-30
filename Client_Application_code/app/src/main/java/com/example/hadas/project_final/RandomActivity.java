package com.example.hadas.project_final;

import android.content.Context;
import android.content.Intent;
import android.content.res.AssetManager;
import android.graphics.Color;
import android.graphics.Typeface;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.RandomAccess;

/**
 * RandomActivity - test mode, the words are chosen randomly
 */
public class RandomActivity extends BaseActivity {

    List<Word> list;
    SingleLineTextView word;
    Word w;
    Thread t;
    int index=0;
    Globals g;
    boolean infoOn;
    TextView information;
    RelativeLayout lay;
    private SensorManager mSensorManager;
    private ShakeEventListener mSensorListener;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_random);
        Button bNew=(Button)findViewById(R.id.newWord);
        Button bRecord=(Button)findViewById(R.id.record);
        g=Globals.getInstance(this.getApplicationContext());
        list=g.getData();
        word=(SingleLineTextView)findViewById(R.id.word);
        final Typeface font = Typeface.createFromAsset(this.getApplicationContext().getAssets(), "fonts/Bahamas-Plain.ttf");
        word.setTypeface(font);
        word.setHorizontallyScrolling(false);
        w=list.get(0);
        index=0;
        word.setText("");
        w=random();
        word.setText(w.getTitle());
        bNew.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                t = new Thread(new Runnable() {
                    @Override
                    public void run() {
                        while (index < 15) {
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    w=random();
                                    word.setText(w.getTitle());
                                }
                            });
                            try {
                                Thread.sleep(100);
                                index +=1;
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                            }
                        }
                    }
                });
                t.start();
                index=0;

            }
        });
        bRecord.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(RandomActivity.this, RecordActivity.class);
                String title = w.getTitle();
                String phones = w.getPhones();
                String[] syl = w.getSyl();
                int i=0;
                int index1=0;
                for (Word word:list) {
                    if(word.getTitle().equals(title)) {
                        phones=word.getPhones();
                        syl=word.getSyl();
                        index1=i;
                    }
                    i++;
                }
                intent.putExtra("word", title);
                intent.putExtra("phones", phones);
                intent.putExtra("syl",syl);
                intent.putExtra("index",index1);
                startActivity(intent);
            }
        });

        mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        mSensorListener = new ShakeEventListener();

        mSensorListener.setOnShakeListener(new ShakeEventListener.OnShakeListener() {

            public void onShake() {
                t = new Thread(new Runnable() {
                    @Override
                    public void run() {
                        while (index < 15) {
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    w=random();
                                    word.setText(w.getTitle());
                                }
                            });
                            try {
                                Thread.sleep(100);
                                index +=1;
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                            }
                        }
                    }
                });
                t.start();
                index=0;
            }
        });
        infoOn=false;
        ImageButton info= (ImageButton) findViewById(R.id.btnInfo);
        information=(TextView)findViewById(R.id.infoText);
        lay=(RelativeLayout)findViewById(R.id.infoSpace);
        info.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(infoOn){
                    information.setText("");
                    lay.setBackgroundDrawable(null);
                    infoOn=false;
                }
                else {
                    information.setText("press SHUFFLE or shake the device to pick a new word");
                    //lay.setBackgroundColor(Color.parseColor("#d6d7d7"));
                    lay.setBackgroundResource(R.drawable.info_bg);
                    infoOn=true;
                }
            }
        });
    }

    /**
     * random - select a random word
     * @return - random word
     */
    public Word random() {
        Random rand=new Random();
        int i= rand.nextInt(list.size());
        return list.get(i);
    }

    @Override
    protected void onResume() {
        super.onResume();
        mSensorManager.registerListener(mSensorListener,
                mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER),
                SensorManager.SENSOR_DELAY_UI);
    }

    @Override
    protected void onPause() {
        mSensorManager.unregisterListener(mSensorListener);
        super.onPause();
    }
}
