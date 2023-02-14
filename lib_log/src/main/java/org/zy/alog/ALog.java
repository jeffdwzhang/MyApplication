package org.zy.alog;

import static org.zy.alog.LogConstants.APPENDER_MODE_ASYNC;
import static org.zy.alog.LogConstants.ZLIB_MODE;
import static org.zy.alog.LogLevel.LEVEL_DEBUG;
import static org.zy.alog.LogLevel.LEVEL_ERROR;
import static org.zy.alog.LogLevel.LEVEL_FATAL;
import static org.zy.alog.LogLevel.LEVEL_INFO;
import static org.zy.alog.LogLevel.LEVEL_VERBOSE;
import static org.zy.alog.LogLevel.LEVEL_WARNING;

public class ALog implements Log.LogImp {

    /**
     * log基本配置
     */
    public static class ALogConfig {
        public int level = LEVEL_INFO;
        public int mode = APPENDER_MODE_ASYNC;
        public String logDir;
        public String namePrefix;
        public String pubKey = "";
        public int compressMode = ZLIB_MODE;
        public int compressLevel = 0;
        public String cacheDir;
        public int cacheDays = 0;
    }

    private volatile static boolean sIsLoaded;
    static {
        // 加载对应的so
        try {
            System.loadLibrary("alog");
            sIsLoaded = true;
        } catch (UnsatisfiedLinkError e) {
            // so加载失败
        }
    }

    public void open(int level, int mode, String cacheDir, String logDir, String namePrefix, int compressMode, String pubKey, int cacheDays) {
        if (!sIsLoaded) {
            // 重试一次
            try {
                System.loadLibrary("alog");
            } catch (UnsatisfiedLinkError e) {
                // so加载失败
                return;
            }
        }

        // 创建构造参数
        ALogConfig logConfig = new ALogConfig();
        logConfig.level = level;
        logConfig.mode = mode;
        logConfig.logDir = logDir;
        logConfig.namePrefix = namePrefix;
        logConfig.pubKey = pubKey;
        logConfig.compressMode = compressMode;
        logConfig.compressLevel = 0;
        logConfig.cacheDir = cacheDir;
        logConfig.cacheDays = cacheDays;

        appenderOpen(logConfig);
    }

    private static String decryptTag(String tag) {
        return tag;
    }

    @Override
    public void logV(long logInstancePtr, String tag, int pid, long tid, long mainTid, String log) {
        logWrite(logInstancePtr, LEVEL_VERBOSE, decryptTag(tag), pid, tid, mainTid, log);
    }

    @Override
    public void logI(long logInstancePtr, String tag, int pid, long tid, long mainTid, String log) {
        logWrite(logInstancePtr, LEVEL_INFO, decryptTag(tag), pid, tid, mainTid, log);
    }

    @Override
    public void logD(long logInstancePtr, String tag, int pid, long tid, long mainTid, String log) {
        logWrite(logInstancePtr, LEVEL_DEBUG, decryptTag(tag), pid, tid, mainTid, log);
    }

    @Override
    public void logW(long logInstancePtr, String tag, int pid, long tid, long mainTid, String log) {
        logWrite(logInstancePtr, LEVEL_WARNING, decryptTag(tag), pid, tid, mainTid, log);
    }

    @Override
    public void logE(long logInstancePtr, String tag, int pid, long tid, long mainTid, String log) {
        logWrite(logInstancePtr, LEVEL_ERROR, decryptTag(tag), pid, tid, mainTid, log);
    }

    @Override
    public void logF(long logInstancePtr, String tag, int pid, long tid, long mainTid, String log) {
        logWrite(logInstancePtr, LEVEL_FATAL, decryptTag(tag), pid, tid, mainTid, log);
    }

    @Override
    public long openLogInstance(int level, int mode, String cacheDir, String logDir, String namePrefix, int cacheDays) {
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

        ALogConfig logConfig = new ALogConfig();
        logConfig.level = level;
        logConfig.mode = mode;
        logConfig.logDir = logDir;
        logConfig.namePrefix = namePrefix;
        logConfig.compressMode = compressMode;
        logConfig.pubKey = pubKey;
        logConfig.cacheDir = cacheDir;
        logConfig.cacheDays = cacheDays;

        appenderOpen(logConfig);
    }

    private native void appenderOpen(ALogConfig logConfig);

    @Override
    public native void appenderClose();

    @Override
    public native void appenderFlush(long logInstancePtr, boolean isSync);

    @Override
    public native void setConsoleLogOpen(long logInstancePtr, boolean isOpen);

    @Override
    public native void setMaxFileSize(long logInstancePtr, long maxSize);

    @Override
    public native void setMaxAliveTime(long logInstancePtr, long aliveSeconds);

    @Override
    public native int getLogLevel(long logInstancePtr);

    private native void logWrite(long logInstancePtr, int level, String tag, int pid, long tid, long mainTid, String log);

}
