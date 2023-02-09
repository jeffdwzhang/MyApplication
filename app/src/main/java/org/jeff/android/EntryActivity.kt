package org.jeff.android

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.WindowCompat
import org.jeff.android.log.JLog
import org.zy.alog.ALog

class EntryActivity : AppCompatActivity() {

    companion object {
        private const val TAG = "EntryActivity"
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        JLog.d(TAG, "onCreate begin.")

        WindowCompat.setDecorFitsSystemWindows(window, true)
        super.onCreate(savedInstanceState)



    }
}