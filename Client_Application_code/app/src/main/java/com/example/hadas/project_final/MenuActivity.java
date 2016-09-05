package com.example.hadas.project_final;

import android.os.Bundle;


import android.content.Intent;
import android.view.View;
import android.widget.Button;

/**
 * MenuActivity - the menu
 */
public class MenuActivity extends BaseActivity {
    Button btnTrain, btnTest, btnScores;

    /**
     * onCreate - create and display the menu
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_menu);
        btnTrain=(Button)findViewById(R.id.btnTrain);
        btnTest=(Button)findViewById(R.id.btnTest);
        btnScores=(Button)findViewById(R.id.btnScores);

        btnTrain.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent i=new Intent(MenuActivity.this,WordsActivity.class);
                startActivity(i);
            }
        });
        btnTest.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent i=new Intent(MenuActivity.this,RandomActivity.class);
                startActivity(i);
            }
        });
        btnScores.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent i=new Intent(MenuActivity.this,ReviewActivity.class);
                startActivity(i);
            }
        });
        
    }
}