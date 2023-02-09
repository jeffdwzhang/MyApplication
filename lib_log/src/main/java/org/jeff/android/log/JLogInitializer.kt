package org.jeff.android.log

import android.app.Application
import android.content.Context
import android.os.Environment
import android.util.Log
import org.zy.alog.LogConstants.Companion.APPENDER_MODE_ASYNC
import org.zy.alog.LogConstants.Companion.APPENDER_MODE_SYNC
import org.zy.alog.LogLevel
import java.io.File


class JLogInitializer private constructor() {

    companion object {

        private val TAG = JLogInitializer::class.java.simpleName
        private const val LOG_DIR = "jlog_v1"
        private const val CACHE_DIR = "log_cache"
        private const val LOG_PREFIX = "app.log"

        val instance by lazy(LazyThreadSafetyMode.SYNCHRONIZED) {
            JLogInitializer()
        }
    }

    private var mLogMaxSize = 50 * 1024 * 1024 // 默认最大50M

    private var mLogLevel: Int = LogLevel.LEVEL_DEBUG
    private var mLogMode: Int = APPENDER_MODE_SYNC
    private var mLogDir: File? = null
    private var mLogCacheDir: File? = null
    private val mAliveDays = 10

    private var packageName: String? = null
    private var mApplication: Application? = null
    private var mContext: Context? = null

    private lateinit var mLogKey: String

    //
    fun builder(
        context: Context,
        logLevel: Int,
        logKey: String,
        appVersion: String
    ): JLogInitializer {
        this.mContext = context
        this.mLogLevel = logLevel
        this.mLogDir = getLogDir(context)
        this.mLogCacheDir = getLogCacheDir(context)
        this.packageName = context.packageName
        this.mLogKey = logKey
        return this
    }


    fun setApplication(app: Application): JLogInitializer {
        this.mApplication = app
        return this
    }

    // 设置log最大size
    fun setLogMaxSize(maxSize: Int): JLogInitializer {
        this.mLogMaxSize = maxSize
        return this
    }


    fun setLogMode(mode: Int): JLogInitializer {
        this.mLogMode = mode
        return this
    }

    fun init(): JLogInitializer {
        Log.i(TAG, "init -> log dir:" + mLogDir?.absolutePath)
        Log.i(TAG, "init -> cache dir:" + mLogCacheDir?.absolutePath)
        JLog.initLog(
            mLogLevel,
            mLogMode,
            mLogCacheDir?.absolutePath,
            mLogDir?.absolutePath,
            LOG_PREFIX,
            mAliveDays,
            mLogKey,
            mLogMaxSize
        )
        return this
    }

    /**
     * 获取log输出目录，优先使用外部存储
     * @param context
     * @return
     */
    private fun getLogDir(context: Context): File {
        var root: File? = null
        if (Environment.MEDIA_MOUNTED == Environment.getExternalStorageState() || !Environment.isExternalStorageRemovable()) {
            root = context.getExternalFilesDir(null)
            val logDir = File(root, LOG_DIR)
            if (!logDir.exists()) {
                val ret = logDir.mkdir()
                if (ret) {
                    return logDir
                }
            } else {
                return logDir
            }
        }
        root = context.filesDir
        val logDir = File(root, LOG_DIR)
        if (!logDir.exists()) {
            val ret = logDir.mkdir()
            if (!ret) {
                Log.w(TAG, "can not create dir:" + logDir.absolutePath)
            }
        }
        return logDir
    }

    /**
     * log缓存目录，使用
     * @param context
     * @return
     */
    private fun getLogCacheDir(context: Context): File {
        val root: File = context.getFilesDir()
        val logDir = File(root, LOG_DIR)
        if (!logDir.exists()) {
            val ret: Boolean = logDir.mkdir()
            if (!ret) {
                Log.w(TAG, "can not create dir:" + logDir.getAbsolutePath())
            }
        }
        val cache = File(logDir, CACHE_DIR)
        if (!cache.exists()) {
            val ret: Boolean = cache.mkdir()
            if (!ret) {
                Log.w(TAG, "can not create dir:" + cache.getAbsolutePath())
            }
        }
        return cache
    }
}