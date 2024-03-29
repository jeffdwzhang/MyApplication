package org.jeff.android.module.nav

import android.os.Bundle
import android.os.SystemClock
import com.google.android.material.snackbar.Snackbar
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.WindowCompat
import androidx.navigation.findNavController
import androidx.navigation.ui.AppBarConfiguration
import androidx.navigation.ui.navigateUp
import androidx.navigation.ui.setupActionBarWithNavController
import android.view.Menu
import android.view.MenuItem
import androidx.arch.core.executor.TaskExecutor
import org.jeff.android.R
import org.jeff.android.databinding.ActivityMainBinding
import org.jeff.android.log.JLog
import kotlin.random.Random

class MainActivity : AppCompatActivity() {

    companion object {
        private const val TAG = "MainActivity"
    }

    private lateinit var appBarConfiguration: AppBarConfiguration
    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        JLog.d(TAG, "onCreate begin.")
        WindowCompat.setDecorFitsSystemWindows(window, false)
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        setSupportActionBar(binding.toolbar)

        val navController = findNavController(R.id.nav_host_fragment_content_main)
        appBarConfiguration = AppBarConfiguration(navController.graph)
        setupActionBarWithNavController(navController, appBarConfiguration)

        binding.fab.setOnClickListener { view ->
            Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
                    .setAnchorView(R.id.fab)
                    .setAction("Action", null).show()
        }

        scheduleTestLog()

        JLog.d(TAG, "onCreate end.")
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        // Inflate the menu; this adds items to the action bar if it is present.
        menuInflater.inflate(R.menu.menu_main, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        return when (item.itemId) {
            R.id.action_settings -> true
            else -> super.onOptionsItemSelected(item)
        }
    }

    override fun onSupportNavigateUp(): Boolean {
        val navController = findNavController(R.id.nav_host_fragment_content_main)
        return navController.navigateUp(appBarConfiguration)
                || super.onSupportNavigateUp()
    }

    private fun scheduleTestLog() {
        var count = 0
        Thread {
            do {
                JLog.d(TAG, "scheduleTestLog -> count:$count, random string:${getRandomString()}")
                JLog.d(TAG, "scheduleTestLog -> stack trace:${getStackTrace()}")
                count++
                Thread.sleep(1000)
            } while (true)
        }.start()
    }

    fun getStackTrace(): String {
        var trace = Throwable().stackTrace
        var builder = java.lang.StringBuilder()
        for (temp in trace) {
            builder.append(temp).append('\n')
        }
        return builder.toString()
    }

    fun getRandomString() : String {
        val random = Random(System.currentTimeMillis())
        val builder = java.lang.StringBuilder()
        for (i in 0 until 200) {
            builder.append(random.nextLong())
        }
        return builder.toString()

    }
}