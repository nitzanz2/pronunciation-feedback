package com.example.hadas.project_final;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.graphics.Typeface;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaPlayer;
import android.media.MediaRecorder;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.text.Html;
import android.text.method.SingleLineTransformationMethod;
import android.util.Base64;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import org.apache.commons.io.FileUtils;
import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.mime.MultipartEntity;
import org.apache.http.entity.mime.MultipartEntityBuilder;
import org.apache.http.entity.mime.content.FileBody;
import org.apache.http.entity.mime.content.StringBody;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.protocol.BasicHttpContext;
import org.apache.http.protocol.HttpContext;
import org.json.JSONArray;

/**
 * RecordActivity - recording words and send the recording to the
 * server for feedback
 */
public class RecordActivity extends BaseActivity {
    public static final double THRESHOLD1 = 0.6;
    public static final double THRESHOLD2 = 0.8;
    private static final int RECORDER_BPP = 16;
    private static final String AUDIO_RECORDER_FILE_EXT_WAV = ".wav";
    private static final String AUDIO_RECORDER_FOLDER = "AudioRecorder";
    private static final String AUDIO_RECORDER_TEMP_FILE = "record_temp.raw";
    private static final int RECORDER_SAMPLERATE = 16000;
    private static final int RECORDER_CHANNELS = AudioFormat.CHANNEL_IN_MONO;
    private static final int RECORDER_AUDIO_ENCODING = AudioFormat.ENCODING_PCM_16BIT;
    private static final String SERVER_IP="http://104.198.8.78";
    boolean first;
    String mode;
    String word,phones;
    String[] syl;
    int index;
    private AudioRecord recorder = null;
    private int bufferSize = 0;
    private Thread recordingThread = null;
    private boolean isRecording = false;
    MediaPlayer m;
    ImageView star;
    SharedPreferences.Editor editor;
    SharedPreferences sharedPref ;
    List<String> favorites;
    Globals g;
    List<Word>words;
    SingleLineTextView txt;
    ImageButton back,again;
    String encoded;
    MySpinnerDialog myInstance;

    /**
     * onCreate - create the recording screen
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_record);
        back=(ImageButton)findViewById(R.id.back);
        back.setVisibility(View.GONE);
        again=(ImageButton)findViewById(R.id.again1);
        again.setVisibility(View.GONE);

        first=true;
        setButtonHandlers();
        enableButtons(false);
        sharedPref = this.getSharedPreferences("pref",
                this.getApplicationContext().MODE_PRIVATE);
        editor = sharedPref.edit();
        mode="record";
        g=Globals.getInstance(this.getApplicationContext());
        g=Globals.getInstance(this.getApplicationContext());
        words=g.getData();
        bufferSize = AudioRecord.getMinBufferSize(8000,
                AudioFormat.CHANNEL_CONFIGURATION_MONO,
                AudioFormat.ENCODING_PCM_16BIT);
        m = new MediaPlayer();
        Bundle extras = getIntent().getExtras();
        if (extras != null) {
            word = extras.getString("word");
            phones = extras.getString("phones");
            syl=extras.getStringArray("syl");
            index=extras.getInt("index");
        }
        txt= (SingleLineTextView) findViewById(R.id.word);
        final Typeface font = Typeface.createFromAsset(this.getApplicationContext().getAssets(), "fonts/Bahamas-Plain.ttf");
        txt.setTypeface(font);
        txt.setHorizontallyScrolling(false);
        txt.setText(word);
        star=(ImageView)findViewById(R.id.favorite);

        if(words.get(index).getFvr()) {
            star.setImageResource(R.drawable.on);
        }
        //add or remove from favorites
        star.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (words.get(index).getFvr()) {
                    star.setImageResource(R.drawable.off);
                    Word temp=g.getData().get(index);
                    temp.setFvr(false);
                    g.setItem(index, temp);
                    remove(word);
                }
                else {
                    star.setImageResource(R.drawable.on);
                    Word temp=g.getData().get(index);
                    temp.setFvr(true);
                    g.setItem(index, temp);
                    add(word);
                }
            }
        });

    }

    /**
     * setButtonHandlers - set onclick listeners for buttons
     */
    private void setButtonHandlers() {
        ((ImageButton)findViewById(R.id.btnStart)).setOnClickListener(btnClick);
        ((ImageButton)findViewById(R.id.btnStop)).setOnClickListener(btnClick);
        ((ImageButton)findViewById(R.id.btnPlay)).setOnClickListener(btnClick);
        ((Button)findViewById(R.id.submit)).setOnClickListener(btnClick);
    }

    /**
     * enableButton - change buttons state
     * @param id - button id
     * @param isEnable - button state
     */
    private void enableButton(int id,boolean isEnable){
        ((ImageButton)findViewById(id)).setEnabled(isEnable);
        ((ImageButton)findViewById(id)).setClickable(isEnable);

    }

    /**
     * enableSubmit - change submit button state
     * @param id - button id
     * @param isEnable - button state
     */
    private void enableSubmit(int id,boolean isEnable){
        ((Button)findViewById(id)).setEnabled(isEnable);
        ((Button)findViewById(id)).setClickable(isEnable);

    }

    /**
     * enableButtons - change buttons state
     * @param isRecording - recording state
     */
    private void enableButtons(boolean isRecording) {
        enableButton(R.id.btnStart, !isRecording);
        enableButton(R.id.btnStop, isRecording);
        if(first) {
            enableButton(R.id.btnPlay, isRecording);
            enableSubmit(R.id.submit, isRecording);
            first=false;
        }
        else {
            enableButton(R.id.btnPlay, !isRecording);
            enableSubmit(R.id.submit, !isRecording);
        }
    }

    /**
     * getFilename - get the file path and name
     * @return - file path
     */
    private String getFilename(){
        String filepath = Environment.getExternalStorageDirectory().getPath();
        File file = new File(filepath,AUDIO_RECORDER_FOLDER);

        if(!file.exists()){
            file.mkdirs();
        }

        return (file.getAbsolutePath() + "/" + "tempFile" + AUDIO_RECORDER_FILE_EXT_WAV);
    }

    /**
     * getTempFilename - get temp file name
     * @return - file path
     */
    private String getTempFilename(){
        String filepath = Environment.getExternalStorageDirectory().getPath();
        File file = new File(filepath,AUDIO_RECORDER_FOLDER);

        if(!file.exists()){
            file.mkdirs();
        }

        File tempFile = new File(filepath,AUDIO_RECORDER_TEMP_FILE);

        if(tempFile.exists())
            tempFile.delete();

        return (file.getAbsolutePath() + "/" + AUDIO_RECORDER_TEMP_FILE);
    }

    /**
     * startRecording - start the record
     */
    private void startRecording(){
        recorder = new AudioRecord(MediaRecorder.AudioSource.MIC,
                RECORDER_SAMPLERATE, RECORDER_CHANNELS,RECORDER_AUDIO_ENCODING, bufferSize);

        int i = recorder.getState();
        if(i==1)
            recorder.startRecording();

        isRecording = true;

        recordingThread = new Thread(new Runnable() {

            @Override
            public void run() {
                writeAudioDataToFile();
            }
        },"AudioRecorder Thread");

        recordingThread.start();
    }

    /**
     * writeAudioDataToFile - write the recording to a file
     */
    private void writeAudioDataToFile(){
        byte data[] = new byte[bufferSize];
        String filename = getTempFilename();
        FileOutputStream os = null;

        try {
            os = new FileOutputStream(filename);
        } catch (FileNotFoundException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        int read = 0;

        if(null != os){
            while(isRecording){
                read = recorder.read(data, 0, bufferSize);

                if(AudioRecord.ERROR_INVALID_OPERATION != read){
                    try {
                        os.write(data);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }

            try {
                os.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * stopRecording - stop the record
     */
    private void stopRecording(){
        if(null != recorder){
            isRecording = false;

            int i = recorder.getState();
            if(i==1)
                recorder.stop();
            recorder.release();

            recorder = null;
            recordingThread = null;
        }

        copyWaveFile(getTempFilename(),getFilename());
        deleteTempFile();
    }

    /**
     * stopPlaying - stop playing the recording
     */
    private void stopPlaying() {
        mode="record";
        m.stop();
        m.release();
        m=null;
    }

    /**
     * deleteTempFile - delete the temp file
     */
    private void deleteTempFile() {
        File file = new File(getTempFilename());

        file.delete();
    }

    /**
     * copyWaveFile - copy the rwcording from temp file to new file
     * @param inFilename - temp file
     * @param outFilename - new file name
     */
    private void copyWaveFile(String inFilename,String outFilename){
        FileInputStream in = null;
        FileOutputStream out = null;
        long totalAudioLen = 0;
        long totalDataLen = totalAudioLen + 36;
        long longSampleRate = RECORDER_SAMPLERATE;
        int channels = 1;
        long byteRate = RECORDER_BPP * RECORDER_SAMPLERATE * channels/8;

        byte[] data = new byte[bufferSize];

        try {
            in = new FileInputStream(inFilename);
            out = new FileOutputStream(outFilename);
            totalAudioLen = in.getChannel().size();
            totalDataLen = totalAudioLen + 36;

            WriteWaveFileHeader(out, totalAudioLen, totalDataLen,
                    longSampleRate, channels, byteRate);

            while(in.read(data) != -1){
                out.write(data);
            }

            in.close();
            out.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * WriteWaveFileHeader - create wav file header
     * @params - wav features
     * @throws IOException
     */
    private void WriteWaveFileHeader(
            FileOutputStream out, long totalAudioLen,
            long totalDataLen, long longSampleRate, int channels,
            long byteRate) throws IOException {

        byte[] header = new byte[44];

        header[0] = 'R'; // RIFF/WAVE header
        header[1] = 'I';
        header[2] = 'F';
        header[3] = 'F';
        header[4] = (byte) (totalDataLen & 0xff);
        header[5] = (byte) ((totalDataLen >> 8) & 0xff);
        header[6] = (byte) ((totalDataLen >> 16) & 0xff);
        header[7] = (byte) ((totalDataLen >> 24) & 0xff);
        header[8] = 'W';
        header[9] = 'A';
        header[10] = 'V';
        header[11] = 'E';
        header[12] = 'f'; // 'fmt ' chunk
        header[13] = 'm';
        header[14] = 't';
        header[15] = ' ';
        header[16] = 16; // 4 bytes: size of 'fmt ' chunk
        header[17] = 0;
        header[18] = 0;
        header[19] = 0;
        header[20] = 1; // format = 1
        header[21] = 0;
        header[22] = (byte) channels;
        header[23] = 0;
        header[24] = (byte) (longSampleRate & 0xff);
        header[25] = (byte) ((longSampleRate >> 8) & 0xff);
        header[26] = (byte) ((longSampleRate >> 16) & 0xff);
        header[27] = (byte) ((longSampleRate >> 24) & 0xff);
        header[28] = (byte) (byteRate & 0xff);
        header[29] = (byte) ((byteRate >> 8) & 0xff);
        header[30] = (byte) ((byteRate >> 16) & 0xff);
        header[31] = (byte) ((byteRate >> 24) & 0xff);
        header[32] = (byte) (2 * 16 / 8); // block align
        header[33] = 0;
        header[34] = RECORDER_BPP; // bits per sample
        header[35] = 0;
        header[36] = 'd';
        header[37] = 'a';
        header[38] = 't';
        header[39] = 'a';
        header[40] = (byte) (totalAudioLen & 0xff);
        header[41] = (byte) ((totalAudioLen >> 8) & 0xff);
        header[42] = (byte) ((totalAudioLen >> 16) & 0xff);
        header[43] = (byte) ((totalAudioLen >> 24) & 0xff);

        out.write(header, 0, 44);
    }

    /**
     * btnClick - create buttons onclick function
     */
    private View.OnClickListener btnClick = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            switch(v.getId()){
                case R.id.btnStart:{
                    //AppLog.logString("Start Recording");
                    mode="record";
                    enableButtons(true);
                    startRecording();

                    break;
                }
                case R.id.btnStop:{
                    // AppLog.logString("Start Recording");

                    enableButtons(false);
                    if(mode.equals("record"))
                        stopRecording();
                    else
                        stopPlaying();

                    break;
                }
                case R.id.btnPlay:{
                    // AppLog.logString("Start Recording");
                    mode="play";
                    //enableButtons(false);
                    enableButtons(true);
                    play();

                    break;
                }
                case R.id.submit:{

                    new submit().execute(SERVER_IP);
                    //loading screen
                    FragmentManager fm = getSupportFragmentManager();
                    myInstance = new MySpinnerDialog();
                    myInstance.show(fm, "some_tag");

                    break;
                }
            }
        }
    };

    /**
     * play - play the recording
     */
    public void play() {

        m=new MediaPlayer();
        try {
            String filepath = Environment.getExternalStorageDirectory().getPath();
            File file = new File(filepath,"AudioRecorder");
            m.setDataSource(file.getAbsolutePath() + "/" + "tempFile" + ".wav");
        }


        catch (IOException e) {
            e.printStackTrace();
        }

        try {
            m.prepare();
        }

        catch (IOException e) {
            e.printStackTrace();
        }

        m.start();
        m.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
            @Override
            public void onCompletion(MediaPlayer mp) {
                enableButtons(false);
                mode="record";
                m.release();
                m=null;
            }
        });
    }

    /**
     * submit - send the recording to the server and get feedback
     */
    private class submit extends AsyncTask<String, String, String> {
        /**
         * doInBackground- send request to the server with the recording and phonems
         */
        @Override
        protected String doInBackground(String... params) {
            HttpClient httpClient = new DefaultHttpClient();
            HttpContext localContext = new BasicHttpContext();

            HttpPost httppost = new HttpPost(params[0]);
            String phonesWithSil="sil "+phones+"sil";
            String filepath = Environment.getExternalStorageDirectory().getPath();
            File file = new File(filepath,"AudioRecorder");
            file = new File(file.getAbsolutePath() + "/" + "tempFile" + ".wav");
            try {
                MultipartEntity mpEntity = new MultipartEntity();
                mpEntity.addPart("wav", new FileBody(file));
                mpEntity.addPart("phones", new StringBody(phones));
                httppost.setEntity(mpEntity);
            }
            catch (Exception e) {
                e.printStackTrace();
            }

            String text = null;
            //try to activate the function in the server
            try {
                HttpResponse response = httpClient.execute(httppost, localContext);
                HttpEntity entity = response.getEntity();
                text = getASCIIContentFromEntity(entity);
            } catch (Exception e) {
                e.printStackTrace();
            }
            return text;
        }

        /**
         * onPostExecute- display the feedback from server
         */
        protected void onPostExecute(String result) {
            myInstance.dismiss();
            if(result==null){
                Toast.makeText(getApplicationContext(), "try again", Toast.LENGTH_LONG).show();
            }
            else {
                enableSubmit(R.id.submit,false);
                scores(result);
            }
        }
    }

    /**
     * getASCIIContentFromEntity- convert the entity to string
     */
    protected String getASCIIContentFromEntity(HttpEntity entity)
            throws IllegalStateException, IOException {
        InputStream in = entity.getContent();
        StringBuffer out = new StringBuffer();
        int n = 1;
        while (n > 0) {
            byte[] b = new byte[4096];
            n = in.read(b);
            if (n > 0)
                out.append(new String(b, 0, n));
        }
        return out.toString();
    }

    /**
     * add - add word to favorites
     * @param name - the word
     */
    public void add(String name) {
        //import favorites list from the shared pref
        String my = sharedPref.getString("fvr", "");
        JSONArray fvrs;
        //add word to the list and update the shared pref
        try {
            if (my.compareTo("") != 0) {
                fvrs = new JSONArray(my);
            }
            else {
                fvrs=new JSONArray();
            }
            fvrs.put(name);
            editor.remove("fvr");
            editor.putString("fvr",fvrs.toString());
            editor.commit();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    /**
     * remove - remove word from favorites
     * @param name - word
     */
    public void remove(String name) {
        //import favorites list from the shared pref
        String my = sharedPref.getString("fvr", "");
        JSONArray fvrs;
        JSONArray newFvrs=new JSONArray();
        //try to remove and put it back to the shared pref
        try {
            if (my.compareTo("") != 0) {
                fvrs = new JSONArray(my);
                //put all the words in the list except for the specific word
                for (int i = 0; i < fvrs.length(); i++) {
                    String data = fvrs.getString(i);
                    //if the id doesn't matches, add this word
                    if(data.compareTo(name)!=0)
                        newFvrs.put(name);
                }
            }
            editor.remove("fvr");
            editor.putString("fvr", newFvrs.toString());
            editor.commit();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * scores - display the feedback
     * @param scores - scores from server
     */
    public void scores(String scores){
        //set buttons
        ((ImageButton)findViewById(R.id.btnPlay)).setVisibility(View.GONE);
        ((ImageButton)findViewById(R.id.btnStop)).setVisibility(View.INVISIBLE);
        ((ImageButton)findViewById(R.id.btnStart)).setVisibility(View.GONE);
        ((Button)findViewById(R.id.submit)).setVisibility(View.INVISIBLE);
        back.setVisibility(View.VISIBLE);
        again.setVisibility(View.VISIBLE);
        back.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
               onBackPressed();

            }
        });
        again.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                txt.setText(word);
                back.setVisibility(View.GONE);
                again.setVisibility(View.GONE);
                ((ImageButton)findViewById(R.id.btnPlay)).setVisibility(View.VISIBLE);
                ((ImageButton)findViewById(R.id.btnStop)).setVisibility(View.VISIBLE);
                ((ImageButton)findViewById(R.id.btnStart)).setVisibility(View.VISIBLE);
                ((Button)findViewById(R.id.submit)).setVisibility(View.VISIBLE);
            }
        });
        int goodColor = getResources().getColor(R.color.good);
        int badColor = getResources().getColor(R.color.bad);
        int medColor = getResources().getColor(R.color.med);
        String сolorStringGood = String.format("%X", goodColor).substring(2);
        String сolorStringBad = String.format("%X", badColor).substring(2);
        String colorStringMed = String.format("%X", medColor).substring(2);
        String good = String.format("<font color=\"#%s\">", сolorStringGood);
        String font = "</font>";
        String bad = String.format("<font color=\"#%s\">", сolorStringBad);
        String med = String.format("<font color=\"#%s\">", colorStringMed);
        String display = "";

        String[] sc=scores.split(" ");
        Boolean flag1=false;
        Boolean flag2=false;

        for (int i=0;i<syl.length;i++) {
            if (syl[i].equals("*")) {
                if (Double.valueOf(sc[i+1])<THRESHOLD1) {
                    flag1=true;
                }
                else if (Double.valueOf(sc[i+1])<THRESHOLD2) {
                    flag2=true;
                }
            }
            else {
                if (Double.valueOf(sc[i+1])<THRESHOLD1) {
                    display=display+bad+syl[i]+font;
                    flag1=false;
                }
                else if (Double.valueOf(sc[i+1])<THRESHOLD2) {
                    display=display+med+syl[i]+font;
                    flag2=false;
                }
                else {
                    if (flag1){
                        display=display+bad+syl[i]+font;
                        flag1=false;
                        flag2=false;
                    }
                    else if (flag2) {
                        display=display+med+syl[i]+font;
                        flag2=false;
                    }
                    else {
                        display=display+good+syl[i]+font;
                    }
                }
            }
        }

        txt.setText(Html.fromHtml(display), TextView.BufferType.SPANNABLE);
        Word temp=g.getData().get(index);
        if(!temp.getHistory()) {
            temp.setHistory(true);
            g.setItem(index, temp);
            addHistory(word);
        }


        String filepath = Environment.getExternalStorageDirectory().getPath();
        File file = new File(filepath,"AudioRecorder/" + "tempFile" + ".wav");
        try {
            byte[] bytes = FileUtils.readFileToByteArray(file);
            encoded = Base64.encodeToString(bytes, 0);

        } catch (IOException e) {
            e.printStackTrace();
        }
        addScore(word, display);
        addWav(word,encoded);
    }

    /**
     * addHistory - add word to history list
     * @param name - word
     */
    public void addHistory(String name) {
        //import channels list from the shared pref
        String my = sharedPref.getString("hislis", "");
        JSONArray his;
        //add word to the list and update the shared pref
        try {
            if (my.compareTo("") != 0) {
                his = new JSONArray(my);
            }
            else {
                his=new JSONArray();
            }
            his.put(name);
            editor.remove("hislis");
            editor.putString("hislis",his.toString());
            editor.commit();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    /**
     * addScore - add feedback to scores list
     * @param w - word
     * @param score feedback
     */
    public void addScore(String w,String score) {
        //import words list from the shared pref
        String my = sharedPref.getString(w, "");
        JSONArray his;
        //add score to the list and update the shared pref
        try {
            if (my.compareTo("") != 0) {
                his = new JSONArray(my);
            }
            else {
                his=new JSONArray();
            }
            JSONArray temp=new JSONArray();
            if(his.length()>=10) {
                for (int i=1;i<10;i++) {
                    temp.put(his.get(i));
                }
                his=new JSONArray();
                for (int i=0;i<9;i++) {
                    his.put(temp.get(i));
                }
            }
            his.put(score);
            editor.remove(w);
            editor.putString(w, his.toString());
            editor.commit();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    /**
     * addWav - add recording to scores list
     * @param w - word
     * @param score - recording
     */
    public void addWav(String w,String score) {
        //import recordings list from the shared pref
        String my = sharedPref.getString(w+"-wav", "");
        JSONArray his;
        //add recording to the list and update the shared pref
        try {
            if (my.compareTo("") != 0) {
                his = new JSONArray(my);
            }
            else {
                his=new JSONArray();
            }
            JSONArray temp=new JSONArray();
            if(his.length()>=10) {
                for (int i=1;i<10;i++) {
                    temp.put(his.get(i));
                }
                his=new JSONArray();
                for (int i=0;i<9;i++) {
                    his.put(temp.get(i));
                }
            }
            his.put(score);
            editor.remove(w+"-wav");
            editor.putString(w+"-wav", his.toString());
            editor.commit();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }
}