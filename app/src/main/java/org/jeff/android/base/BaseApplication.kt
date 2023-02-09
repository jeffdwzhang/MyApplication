package org.jeff.android.base

import android.app.Application
import android.content.Context
import org.jeff.android.BuildConfig
import org.jeff.android.log.JLog
import org.jeff.android.log.JLogInitializer
import org.zy.alog.LogLevel

class BaseApplication : Application() {

    companion object {
        private const val TAG = "BaseApplication"

        private const val LOG_KEY = "572d1e2710ae5fbca54c76a382fdd44050b3a675cb2bf39feebe85ef63d947aff0fa4943f1112e8b6af34bebebbaefa1a0aae055d9259b89a1858f7cc9af9df1"
    }

    override fun onCreate() {
        JLog.d(TAG, "onCreate begin.")
        super.onCreate()

        initLog(this)

        JLog.d(TAG, "onCreate end.")
    }

    override fun attachBaseContext(base: Context?) {
        JLog.d(TAG, "attachBaseContext.")
        super.attachBaseContext(base)
    }

    override fun onLowMemory() {
        super.onLowMemory()
    }

    override fun onTerminate() {
        super.onTerminate()
    }

    private fun initLog(application: Application) {
        var logLevel = if (BuildConfig.DEBUG)  LogLevel.LEVEL_DEBUG else LogLevel.LEVEL_INFO
        var version = application.packageManager.getPackageInfo(application.packageName, 0).versionName
        JLogInitializer.instance.builder(application, logLevel, LOG_KEY, version).init()
    }

}