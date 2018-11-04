package com.github.vatbub.hearingaid;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;

import java.io.IOException;

public class FeedbackPrivacyActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_feedback_privacy);
        if (getSupportActionBar() != null)
            getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        try {
            MainActivity.displayMarkdown(this, R.raw.privacy, R.id.activity_feedback_privacy_markdown_view);
        } catch (IOException e) {
            e.printStackTrace();
            // TODO: Implement Bugsnag
        }
    }

    @Override
    public boolean onSupportNavigateUp() {
        finish();
        return true;
    }
}
