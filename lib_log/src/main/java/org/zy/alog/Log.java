package org.zy.alog;

import static org.zy.alog.LogLevel.*;

import android.content.Context;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.os.Process;
import android.widget.Toast;

import androidx.annotation.NonNull;

public class Log {

    private static final String TAG = "zy.log";

    // 全局toast使用的上下文
    public static Context toastSupportContext = null;

    private static final int PID = Process.myPid();
    private static final long MAIN_ID = Looper.getMainLooper().getThread().getId();

    // log基本接口
    public interface LogImp {

        void logV(long logInstancePtr, String tag, int pid, long tid, long mainTid, String log);
        void logI(long logInstancePtr, String tag, int pid, long tid, long mainTid, String log);
        void logD(long logInstancePtr, String tag, int pid, long tid, long mainTid, String log);
        void logW(long logInstancePtr, String tag, int pid, long tid, long mainTid, String log);
        void logE(long logInstancePtr, String tag, int pid, long tid, long mainTid, String log);
        void logF(long logInstancePtr, String tag, int pid, long tid, long mainTid, String log);

        /**
         * 获取当前实例所对应的log输出级别
         * 实例指输出该log级别以上的log，低于该级别的直接忽略
         * @param logInstancePtr
         * @return
         */
        int getLogLevel(long logInstancePtr);

        /**
         * 创建Log输出实例
         * @param level   实例对应的最高log输出级别，后面只输出不大于这个级别的log
         * @param mode
         * @param cacheDir
         * @param logDir
         * @param namePrefix
         * @param cacheDays
         * @return   log实例的句柄，用于后续调用访问log实例
         */
        long openLogInstance(int level, int mode, String cacheDir, String logDir, String namePrefix, int cacheDays);

        /**
         *
         * @param namePrefix
         * @return
         */
        long getLogInstance(String namePrefix);

        /**
         *
         * @param namePrefix
         */
        void releaseLogInstance(String namePrefix);


        void appenderOpen(int level, int mode, String cacheDir, String logDir, String namePrefix, int compressMode, String pubKey, int cacheDays);

        void appenderClose();

        void appenderFlush(long logInstancePtr, boolean isSync);

        void setConsoleLogOpen(long logInstancePtr, boolean isOpen);

        void setMaxFileSize(long logInstancePtr, long maxSize);

        void setMaxAliveTime(long logInstancePtr, long aliveSeconds);
    }


    private static final LogImp debugLog = new LogImp() {

        // default to LEVEL_NONE
        private int level = LEVEL_VERBOSE;

        private Handler handler = new Handler(Looper.getMainLooper());

        @Override
        public void logV(long logInstancePtr, String tag, int pid, long tid, long mainTid, String log) {
            if (level <= LEVEL_VERBOSE) {
                android.util.Log.v(tag, log);
            }
        }

        @Override
        public void logI(long logInstancePtr, String tag, int pid, long tid, long mainTid, String log) {
            if (level <= LEVEL_INFO) {
                android.util.Log.i(tag, log);
            }
        }

        @Override
        public void logD(long logInstancePtr, String tag, int pid, long tid, long mainTid, String log) {
            if (level <= LEVEL_DEBUG) {
                android.util.Log.d(tag, log);
            }
        }

        @Override
        public void logW(long logInstancePtr, String tag, int pid, long tid, long mainTid, String log) {
            if (level <= LEVEL_WARNING) {
                android.util.Log.w(tag, log);
            }
        }

        @Override
        public void logE(long logInstancePtr, String tag, int pid, long tid, long mainTid, String log) {
            if (level <= LEVEL_ERROR) {
                android.util.Log.e(tag, log);
            }
        }

        @Override
        public void logF(long logInstancePtr, String tag, int pid, long tid, long mainTid, String log) {
            if (level > LEVEL_FATAL) {
                return;
            }
            android.util.Log.e(tag, log);
            if (toastSupportContext != null) {
                handler.post(() -> {
                    Toast.makeText(toastSupportContext, log, Toast.LENGTH_LONG).show();
                });
            }
        }

        @Override
        public int getLogLevel(long logInstancePtr) {
            return level;
        }

        @Override
        public long openLogInstance(int level, int mode, String cacheDir, String logDir, String namePrefix, int cacheDays) {
            this.level = level;
            return 0;
        }

        @Override
        public long getLogInstance(String namePrefix) {
            return 0;
        }

        @Override
        public void releaseLogInstance(String namePrefix) {

        }

        @Override
        public void appenderOpen(int level, int mode, String cacheDir, String logDir, String namePrefix, int compressMode, String pubKey, int cacheDays) {
            this.level = level;
        }

        @Override
        public void appenderClose() {

        }

        @Override
        public void appenderFlush(long logInstancePtr, boolean isSync) {

        }

        @Override
        public void setConsoleLogOpen(long logInstancePtr, boolean isOpen) {

        }

        @Override
        public void setMaxFileSize(long logInstancePtr, long maxSize) {

        }

        @Override
        public void setMaxAliveTime(long logInstancePtr, long aliveSeconds) {

        }
    };

    private static volatile LogImp logImp = debugLog;

    /**
     * 设置对应的log输出实现
     * @param imp
     */
    public static void setLogImp(@NonNull LogImp imp) {
        logImp = imp;
    }

    /**
     * 获取当前的log实现
     * @return
     */
    public static LogImp getImpl() {
        return logImp;
    }


    public static void appenderOpen(int level, int mode, String cacheDir, String logDir, String namePrefix, int compressMode, String pubKey, int cacheDays) {
        if (logImp != null) {
            logImp.appenderOpen(level, mode, cacheDir, logDir, namePrefix, compressMode, pubKey, cacheDays);
        }
    }

    public static void appenderClose() {
        if (logImp != null) {
            logImp.appenderClose();

//            for (Map.Entry<String, LogInstance> entry: sLogInstanceMap.entrySet()) {
//                closeLogInstance(entry.getKey());
//            }
        }
    }

    public static void appenderFlush() {
        if (logImp != null) {
            logImp.appenderFlush(0, false);

//            for (Map.Entry<String, LogInstance> entry: sLogInstanceMap.entrySet()) {
//                entry.getValue().appenderFlush();
//            }
        }
    }

    public static void appenderFlushSync(boolean isSync) {
        if (logImp != null) {
            logImp.appenderFlush(0, isSync);
        }
    }

    public static void setConsoleLogOpen(boolean isOpen) {
        if (logImp != null) {
            logImp.setConsoleLogOpen(0, isOpen);
        }
    }

    public static void setMaxSize(int maxSize) {
        if (logImp != null) {
            logImp.setMaxFileSize(0, maxSize);
        }
    }

    public static void f(String tag, final String format, final Object... obj) {
        if (logImp != null && logImp.getLogLevel(0) <= LEVEL_FATAL) {
            final String log = obj == null ? format : String.format(format, obj);
            logImp.logF(0, tag, PID, Thread.currentThread().getId(), MAIN_ID, log);
        }
    }

    public static void e(String tag, final String format, final Object... obj) {
        if (logImp != null && logImp.getLogLevel(0) <= LEVEL_ERROR) {
            String log = obj == null ? format : String.format(format, obj);
            if (log == null) {
                log = "";
            }
            logImp.logE(0, tag, PID, Thread.currentThread().getId(), MAIN_ID, log);
        }
    }

    public static void w(String tag, final String format, final Object... obj) {
        if (logImp != null && logImp.getLogLevel(0) <= LEVEL_WARNING) {
            String log = obj == null ? format : String.format(format, obj);
            if (log == null) {
                log = "";
            }
            logImp.logW(0, tag, PID, Thread.currentThread().getId(), MAIN_ID, log);
        }
    }

    public static void i(String tag, final String format, final Object... obj) {
        if (logImp != null && logImp.getLogLevel(0) <= LEVEL_INFO) {
            String log = obj == null ? format : String.format(format, obj);
            if (log == null) {
                log = "";
            }
            logImp.logI(0, tag, PID, Thread.currentThread().getId(), MAIN_ID, log);
        }
    }

    public static void d(String tag, final String format, final Object... obj) {
        if (logImp != null && logImp.getLogLevel(0) <= LEVEL_DEBUG) {
            String log = obj == null ? format : String.format(format, obj);
            if (log == null) {
                log = "";
            }
            logImp.logD(0, tag, PID, Thread.currentThread().getId(), MAIN_ID, log);
        }
    }


    public static void v(String tag, final String format, final Object... obj) {
        if (logImp != null && logImp.getLogLevel(0) <= LEVEL_VERBOSE) {
            String log = obj == null ? format : String.format(format, obj);
            if (log == null) {
                log = "";
            }
            logImp.logV(0, tag, PID, Thread.currentThread().getId(), MAIN_ID, log);
        }
    }

    public static void printErrStackTrace(String tag, Throwable tr, final String format, final Object... obj) {
        if (logImp != null && logImp.getLogLevel(0) <= LEVEL_ERROR) {
            String log = obj == null ? format : String.format(format, obj);
            if (log == null) {
                log = "";
            }
            log += "  " + android.util.Log.getStackTraceString(tr);
            logImp.logE(0, tag, PID, Thread.currentThread().getId(), MAIN_ID, log);
        }
    }

    //---------------------------------------------------------------------------
    private static final String SYS_INFO;

    static {
        final StringBuilder sb = new StringBuilder();
        try {
            sb.append("VERSION.RELEASE:[" + Build.VERSION.RELEASE);
            sb.append("] VERSION.CODENAME:[" + Build.VERSION.CODENAME);
            sb.append("] VERSION.INCREMENTAL:[" + Build.VERSION.INCREMENTAL);
            sb.append("] BOARD:[" + Build.BOARD);
            sb.append("] DEVICE:[" + Build.DEVICE);
            sb.append("] DISPLAY:[" + Build.DISPLAY);
            sb.append("] FINGERPRINT:[" + Build.FINGERPRINT);
            sb.append("] HOST:[" + Build.HOST);
            sb.append("] MANUFACTURER:[" + Build.MANUFACTURER);
            sb.append("] MODEL:[" + Build.MODEL);
            sb.append("] PRODUCT:[" + Build.PRODUCT);
            sb.append("] TAGS:[" + Build.TAGS);
            sb.append("] TYPE:[" + Build.TYPE);
            sb.append("] USER:[" + Build.USER + "]");
        } catch (Throwable e) {
            e.printStackTrace();
        }

        SYS_INFO = sb.toString();
    }

    /**
     * 获取基本的系统信息
     * @return
     */
    public static String getSysInfo() {
        return SYS_INFO;
    }
    //---------------------------------------------------------------------------


}
