/* 8921104
 204053268 Hadas Cohen
 203571435 Einav Saad
 204208714 Nitzan Zeira
 203532064 Adar Kendelker
 */
package com.example.hadas.project_final;

import android.content.DialogInterface;
import android.media.MediaPlayer;
import android.os.Environment;
import android.util.Base64;
import android.view.View;
import android.widget.Toast;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.reflect.Array;
import java.util.ArrayList;

/*
 * score and recording
 */
public class Score {
    private String score;
    private String wav;
    View.OnClickListener listener;
    /*************************************************************************
     * function name: Score
     * The Input: score,encoded wav file
     * The output: no output
     * The Function operation: constructor
     **************************************************************************/
    public Score(String score, final String wav){
        this.score = score;
        this.wav=wav;
        this.setListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                String filepath = Environment.getExternalStorageDirectory().getPath();
                File file = new File(filepath, "AudioRecorder" + "/" + "tempFile" + ".wav");
                if (file.exists())
                    file.delete();
                file = new File(filepath, "AudioRecorder" + "/" + "tempFile" + ".wav");
                MediaPlayer m = new MediaPlayer();
                try {
                    FileOutputStream os = new FileOutputStream(file, true);
                    byte[] decoded = Base64.decode(wav, 0);
                    os.write(decoded);
                    os.close();
                    m.setDataSource(file.getPath());
                } catch (Exception e) {
                    e.printStackTrace();
                }

                try {
                    m.prepare();
                } catch (IOException e) {
                    e.printStackTrace();
                }

                m.start();

            }

        });

    }
    public String getScore() {
        return this.score;
    }
    public void setListener(View.OnClickListener l){
        this.listener=l;
    }

    public View.OnClickListener getListener() {
        return listener;
    }
}
