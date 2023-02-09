package org.jeff.android.log;

import org.zy.alog.ALog;

/**
 * 对外部模块包装的Log调用接口
 *
 */
public class JLog {

    private static final int MAX_LEN = 8192;

    /**
     * 初始化
     *
     * @param level
     * @param mode
     * @param cacheDir
     * @param logDir
     * @param namePrefix
     * @param cacheDays
     * @param pubKey
     * @param maxSize
     */
    public static void initLog(int level, int mode, String cacheDir, String logDir, String namePrefix, int cacheDays, String pubKey, int maxSize) {
        // log实现采用Alog实例
        ALog alog = new ALog();
        // 开启log
        alog.appenderOpen(level, mode, cacheDir, logDir, namePrefix, pubKey, cacheDays);
        org.zy.alog.Log.setLogImp(alog);
        org.zy.alog.Log.setConsoleLogOpen(false);
        org.zy.alog.Log.setMaxSize(maxSize);
    }

    public static void release() {
        org.zy.alog.Log.appenderFlush();
        org.zy.alog.Log.appenderClose();
    }

    public static void v(String tag, String msg) {
        if (msg == null || msg.isEmpty() || msg.length() > MAX_LEN) {
            return;
        }
        org.zy.alog.Log.v(tag, msg);
    }

    public static void d(String tag, String msg) {
        if (msg == null || msg.isEmpty() || msg.length() > MAX_LEN) {
            return;
        }
        org.zy.alog.Log.d(tag, msg);
    }

    public static void i(String tag, String msg) {
        if (msg == null || msg.isEmpty() || msg.length() > MAX_LEN) {
            return;
        }
        org.zy.alog.Log.i(tag, msg);
    }

    public static void w(String tag, String msg) {
        if (msg == null || msg.isEmpty() || msg.length() > MAX_LEN) {
            return;
        }
        org.zy.alog.Log.w(tag, msg);
    }

    public static void e(String tag, String msg) {
        if (msg == null || msg.isEmpty() || msg.length() > MAX_LEN) {
            return;
        }
        org.zy.alog.Log.e(tag, msg);
    }

    public static void f(String tag, String msg) {
        if (msg == null || msg.isEmpty() || msg.length() > MAX_LEN) {
            return;
        }
        org.zy.alog.Log.f(tag, msg);
    }

}
