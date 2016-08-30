package com.example.hadas.project_final;

        import android.support.v7.app.ActionBar;
        import android.support.v7.app.ActionBarActivity;
        import android.os.Bundle;
        import android.view.Menu;
        import android.view.MenuItem;

/**
 * class BaseActivity- display screen without action bar
 */
public class BaseActivity extends ActionBarActivity {
    /**
     * onCreate- calls setActionBar function
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_base);
        setActionBar();
    }

    /**
     * setActionBar- hide action bar
     */
    public void setActionBar() {
        ActionBar bar = getSupportActionBar();
        bar.hide();

    }
}