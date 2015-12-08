package com.baidu.music.classlib.utils;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.math.BigDecimal;
import java.security.MessageDigest;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;

import org.apache.http.util.ByteArrayBuffer;

import android.content.Context;
import android.os.Environment;
import android.os.StatFs;
import android.provider.MediaStore.Audio;
import android.provider.MediaStore.Video;
import android.text.TextUtils;
import android.util.Log;

/**
 * 文件相关的操作
 * @modify by Jarlene on 2015/11/23.
 */
public class FileUtils {
    /**
     * KB
     */
    public static final long ONE_KB = 1024;
    /**
     * MB
     */
    public static final long ONE_MB = ONE_KB * ONE_KB;
    /**
     * GB
     */
    public static final long ONE_GB = ONE_KB * ONE_MB;
    /**
     * 后缀名分隔符
     */
    public static final char EXTENSION_SEPARATOR = '.';

    public static char[] hexChar = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    private static final String TAG = "FileUtils";
    /**
     * Unix路径分隔符
     */
    private static final char UNIX_SEPARATOR = '/';
    /**
     * Windows路径分隔符
     */
    private static final char WINDOWS_SEPARATOR = '\\';
    private static final int BUF_SIZE = 1024;

    /**
     * write a string To a File
     *
     * @param file
     * @param string
     * @param isAppend
     *
     * @return
     */
    public static boolean writeStringToFile(File file, String string, boolean isAppend) {
        boolean isWriteOk = false;

        if (null == file || null == string) {
            return isWriteOk;
        }

        FileWriter fw = null;
        try {
            fw = new FileWriter(file, isAppend);

            fw.write(string, 0, string.length());
            fw.flush();
            isWriteOk = true;
        } catch (Exception e) {
            isWriteOk = false;
            e.printStackTrace();
        } finally {
            if (fw != null) {
                try {
                    fw.close();
                } catch (IOException e) {
                    isWriteOk = false;
                    e.printStackTrace();
                }
            }
        }
        return isWriteOk;
    }

    public static void clearFile(Context context, String filename) {
        if (context == null || TextUtils.isEmpty(filename)) {
            return;
        }
        File file = getFile(context, filename);
        Log.d(TAG, "clearFile path : " + file.getAbsolutePath());
        File dir = file.getParentFile();
        if (!dir.exists()) {
            Log.d(TAG, "dir not exists");
            dir.mkdirs();
        }
        if (file.exists()) {
            file.delete();
        }
        try {
            file.createNewFile();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static boolean isExist (String filePath) {
        return !TextUtils.isEmpty(filePath) && new File(filePath).exists();
    }

    public static void touchFile(Context context, String filename) {
        if (context == null || TextUtils.isEmpty(filename)) {
            return;
        }
        File file = getFile(context, filename);
        Log.d(TAG, "touchFile path : " + file.getAbsolutePath());
        File dir = file.getParentFile();
        if (!dir.exists()) {
            Log.d(TAG, "dir not exists");
            dir.mkdirs();
        }
        if (file.exists()) {
            return;
        }
        try {
            file.createNewFile();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static File getFile(Context context, String filename) {
        if (context == null || TextUtils.isEmpty(filename)) {
            return null;
        }
        return new File(context.getFilesDir().getAbsoluteFile() + filename);

    }

    /**
     * 将文件大小的long值转换为可读的文字
     *
     * @param size
     *
     * @return 10KB或10MB或1GB
     */
    public static String byteCountToDisplaySize(long size) {
        String displaySize;

        if (size / ONE_GB > 0) {
            displaySize = String.valueOf(size / ONE_GB) + " GB";
        } else if (size / ONE_MB > 0) {
            displaySize = String.valueOf(size / ONE_MB) + " MB";
        } else if (size / ONE_KB > 0) {
            displaySize = String.valueOf(size / ONE_KB) + " KB";
        } else {
            displaySize = String.valueOf(size) + " bytes";
        }
        return displaySize;
    }

    /**
     * 将文件大小的long值转换为可读的文字
     *
     * @param size
     * @param scale 保留几位小数
     *
     * @return 10KB或10MB或1GB
     */
    public static String byteCountToDisplaySize(long size, int scale) {
        String displaySize;
        if (size / ONE_GB > 0) {
            float d = (float) size / ONE_GB;
            displaySize = getOffsetDecimal(d, scale) + " GB";
        } else if (size / ONE_MB > 0) {
            float d = (float) size / ONE_MB;
            displaySize = getOffsetDecimal(d, scale) + " MB";
        } else if (size / ONE_KB > 0) {
            float d = (float) size / ONE_KB;
            displaySize = getOffsetDecimal(d, scale) + " KB";
        } else {
            displaySize = String.valueOf(size) + " bytes";
        }
        return displaySize;
    }

    public static String getOffsetDecimal(float ft, int scale) {
        int roundingMode = 4; // 表示四舍五入，可以选择其他舍值方式，例如去尾，等等.
        BigDecimal bd = new BigDecimal(ft);
        bd = bd.setScale(scale, roundingMode);
        ft = bd.floatValue();
        return "" + ft;
    }

    public static boolean delAllFileWithoutDir(String path) {
        boolean flag = false;
        File file = new File(path);
        if (!file.exists()) {
            return flag;
        }
        if (!file.isDirectory()) {
            return flag;
        }
        String[] tempList = file.list();
        File temp = null;
        for (int i = 0; i < tempList.length; i++) {
            if (path.endsWith(File.separator)) {
                temp = new File(path + tempList[i]);
            } else {
                temp = new File(path + File.separator + tempList[i]);
            }
            if (temp.isFile()) {
                temp.delete();
            }
            if (temp.isDirectory()) {
                delAllFileWithoutDir(path + "/" + tempList[i]); // 先删除文件夹里面的文件
                flag = true;
            }
        }
        return flag;
    }

    public static String loadString(Context context, String filename) {
        if (context == null || TextUtils.isEmpty(filename)) {
            return null;
        }
        File file = getFile(context, filename);
        return loadString(file);
    }

    /**
     * read file to a string
     *
     * @param file
     *
     * @return
     */
    public static String loadString(File file) {
        if (null == file || !file.exists()) {
            return "";
        }
        FileInputStream fis = null;

        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        try {
            fis = new FileInputStream(file);
            int restSize = fis.available();
            int bufSize = restSize > BUF_SIZE ? BUF_SIZE : restSize;
            byte[] buf = new byte[bufSize];
            while (fis.read(buf) != -1) {
                baos.write(buf);
                restSize -= bufSize;

                if (restSize <= 0) {
                    break;
                }
                if (restSize < bufSize) {
                    bufSize = restSize;
                }
                buf = new byte[bufSize];
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (SecurityException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (fis != null) {
                try {
                    fis.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }

        return baos.toString();
    }

    public static long getFolderSize(File folder) throws IllegalArgumentException {
        // Validate
        if (folder == null || !folder.isDirectory()) {
            throw new IllegalArgumentException("Invalid   folder ");
        }
        String[] list = folder.list();
        if (list == null || list.length < 1) {
            return 0;
        }

        // Get size
        File object = null;
        long folderSize = 0;
        for (int i = 0; i < list.length; i++) {
            object = new File(folder, list[i]);
            if (object.isDirectory()) {
                folderSize += getFolderSize(object);
            } else if (object.isFile()) {
                folderSize += object.length();
            }
        }
        return folderSize;
    }

    /**
     * 获取文件编码
     *
     * @param sourceFile
     *
     * @return
     */
    public static String getFileCharset(String sourceFile) {
        Log.d(TAG, "getFileCharset, sourceFile=" + sourceFile);
        String charset = "UTF-8";
        byte[] first3Bytes = new byte[3];
        BufferedInputStream bis = null;
        try {
            boolean checked = false;
            bis = new BufferedInputStream(new FileInputStream(sourceFile));
            bis.mark(3);
            int read = bis.read(first3Bytes, 0, 3);
            if (read == -1) {
                return charset; // 文件编码为 ANSI
            } else if (first3Bytes[0] == (byte) 0xFF && first3Bytes[1] == (byte) 0xFE) {
                charset = "UTF-16LE"; // 文件编码为 Unicode
                checked = true;
            } else if (first3Bytes[0] == (byte) 0xFE && first3Bytes[1] == (byte) 0xFF) {
                charset = "UTF-16BE"; // 文件编码为 Unicode big endian
                checked = true;
            } else if (first3Bytes[0] == (byte) 0xEF && first3Bytes[1] == (byte) 0xBB
                    && first3Bytes[2] == (byte) 0xBF) {
                charset = "UTF-8"; // 文件编码为 UTF-8
                checked = true;
            }
            bis.reset();
            if (!checked) {
                while ((read = bis.read()) != -1) {
                    if (read >= 0xF0) {
                        charset = "GBK";
                        break;
                    }
                    // 单独出现BF以下的，也算是GBK
                    if (0x80 <= read && read <= 0xBF) {
                        charset = "GBK";
                        break;
                    }
                    if (0xC0 <= read && read <= 0xDF) {
                        read = bis.read();
                        // 双字节 (0xC0 - 0xDF) (0x80 - 0xBF),也可能在GB编码内
                        if (0x80 <= read && read <= 0xBF) {
                            continue;
                        } else {
                            charset = "GBK";
                            break;
                        }
                    } else if (0xE0 <= read && read <= 0xEF) { // 也有可能出错，但是几率较小
                        read = bis.read();
                        if (0x80 <= read && read <= 0xBF) {
                            read = bis.read();
                            if (0x80 <= read && read <= 0xBF) {
                                charset = "UTF-8";
                                break;
                            } else {
                                charset = "GBK";
                                break;
                            }
                        } else {
                            charset = "GBK";
                            break;
                        }
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (bis != null) {
                try {
                    bis.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        Log.d(TAG, "getFileCharset, charset=" + charset);
        return charset;
    }

    public static String getName(String filename) {
        if (filename == null) {
            return null;
        }
        int index = indexOfLastSeparator(filename);
        return filename.substring(index + 1);
    }

    public static int indexOfLastSeparator(String filename) {
        if (filename == null) {
            return -1;
        }
        int lastUnixPos = filename.lastIndexOf(UNIX_SEPARATOR);
        int lastWindowsPos = filename.lastIndexOf(WINDOWS_SEPARATOR);
        return Math.max(lastUnixPos, lastWindowsPos);
    }

    /**
     * 流拷贝
     *
     * @param is
     * @param os
     *
     * @throws IOException
     */
    public static void copyStream(InputStream is, OutputStream os) throws IOException {
        byte[] buffer = new byte[1024];
        int len = -1;
        while ((len = is.read(buffer, 0, 1024)) != -1) {
            os.write(buffer, 0, len);
        }
    }

    /**
     * 拷贝流到文件中
     *
     * @param is
     * @param desc
     *
     * @throws IOException
     */
    public static void copyStream2File(InputStream is, File desc) throws IOException {
        OutputStream os = null;
        try {
            os = new FileOutputStream(desc);
            copyStream(is, os);
        } finally {
            if (os != null) {
                os.close();
            }
        }
    }

    /**
     * 拷贝文件
     *
     * @param src
     * @param desc
     *
     * @throws IOException
     */
    public static void copyFile(File src, File desc) throws IOException {
        InputStream is = null;
        OutputStream os = null;
        try {
            is = new FileInputStream(src);
            os = new FileOutputStream(desc);
            copyStream(is, os);
        } finally {
            if (is != null) {
                is.close();
            }
            if (os != null) {
                os.close();
            }
        }
    }

    /**
     * 获得文件简单名称
     *
     * @param file
     *
     * @return
     */
    public static String getSimpleName(String file) {
        return getSimpleName(new File(file));
    }

    /**
     * 获得文件简单名称
     *
     * @param file
     *
     * @return
     */
    public static String getSimpleName(File file) {
        if (file == null) {
            return "";
        }

        if (file.isDirectory()) {
            return file.getName();
        }

        String filePath = file.getName();
        int index = filePath.lastIndexOf(".");

        if (index != -1) {
            return filePath.substring(0, index);
        }

        return filePath;
    }

    /**
     * 根据文件URI判断是否为媒体文件
     *
     * @param uri
     *
     * @return
     */
    public static boolean isMediaUri(String uri) {
        if (uri.startsWith(Audio.Media.INTERNAL_CONTENT_URI.toString())
                || uri.startsWith(Audio.Media.EXTERNAL_CONTENT_URI.toString())
                || uri.startsWith(Video.Media.INTERNAL_CONTENT_URI.toString())
                || uri.startsWith(Video.Media.EXTERNAL_CONTENT_URI.toString())) {
            return true;
        } else {
            return false;
        }
    }

    public static boolean copyfile(File src, File desc) throws Exception {
        if (src == null || desc == null) {
            return false;
        }
        InputStream is = null;
        OutputStream os = null;
        try {
            if (!src.isFile() || !src.exists()) {
                return false;
            }

            if (!checkFile(desc)) {
                return false;
            }
            is = new FileInputStream(src);
            os = new FileOutputStream(desc);

            byte[] buf = new byte[1024];
            int len;
            while ((len = is.read(buf)) > 0) {
                os.write(buf, 0, len);
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            throw e;
        } catch (IOException e) {
            e.printStackTrace();
            throw e;
        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        } finally {
            if (is != null) {
                is.close();
            }
            if (os != null) {
                os.close();
            }
        }
        return true;
    }


    /**
     * 文件copy
     *
     * @param src
     * @param dest
     * @return
     * @throws java.io.IOException
     */
    public static File copyFile(String src, String dest) throws IOException {
        InputStream is = null;
        OutputStream os = null;
        try {
            File destFile = new File(dest);
            if (!destFile.getParentFile().exists())
                destFile.getParentFile().mkdirs();
            if (destFile.exists())
                destFile.delete();
            is = new FileInputStream(src);
            os = new FileOutputStream(destFile);
            FileUtils.copyStream(is, os);
            return destFile;
        } finally {
            if (is != null)
                try {
                    is.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            if (os != null)
                try {
                    os.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
        }
    }

    /**
     * 获取文件名。不好含后缀名
     * @param filePath
     * @return
     */
    public static String getFileNameWithOutPostfix(String filePath) {
        if (TextUtils.isEmpty(filePath)) {
            return "";
        }
        File file = new File(filePath);
        String fileName = file.getName();
        return fileName.substring(0, fileName.lastIndexOf("."));
    }

    public static String getFileName(String path) {
        if (path == null) {
            return null;
        }
        String retStr = "";
        if (path.indexOf(File.separator) > 0) {
            retStr = path.substring(path.lastIndexOf(File.separator) + 1);
        } else {
            retStr = path;
        }
        return retStr;
    }

    public static String getFileNameNoPostfix(String path) {
        if (path == null) {
            return null;
        }
        return path.substring(path.lastIndexOf(File.separator) + 1);
    }

    /**
     * 根据文件URI得到文件扩展名
     *
     * @param uri 文件路径标识
     *
     * @return
     */
    public static String getExtension(String uri) {
        if (uri == null) {
            return null;
        }

        int extensionIndex = uri.lastIndexOf(EXTENSION_SEPARATOR);
        int lastUnixIndex = uri.lastIndexOf(UNIX_SEPARATOR);
        int lastWindowsIndex = uri.lastIndexOf(WINDOWS_SEPARATOR);
        int index = Math.max(lastUnixIndex, lastWindowsIndex);
        if (index > extensionIndex || extensionIndex < 0) {
            return null;
        }
        return uri.substring(extensionIndex + 1);
    }

    /**
     * 判断是否为本地文件
     *
     * @param uri
     *
     * @return
     */
    public static boolean isLocal(String uri) {
        if (uri != null && !uri.startsWith("http://")) {
            return true;
        }
        return false;
    }

    /**
     * 判断文件是否为视频文件
     *
     * @param filename
     *
     * @return
     */
    public static boolean isVideo(String filename) {
        String mimeType = getMimeType(filename);
        if (mimeType != null && mimeType.startsWith("video/")) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * 判断文件是否为音频文件
     *
     * @param filename
     *
     * @return
     */
    public static boolean isAudio(String filename) {
        String mimeType = getMimeType(filename);
        if (mimeType != null && mimeType.startsWith("audio/")) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * 根据文件名得到文件的mimetype 简单判断,考虑改为xml文件配置关联
     *
     * @param filename
     *
     * @return
     */
    public static String getMimeType(String filename) {
        String mimeType = null;

        if (filename == null) {
            return mimeType;
        }
        if (filename.endsWith(".3gp")) {
            mimeType = "video/3gpp";
        } else if (filename.endsWith(".mid")) {
            mimeType = "audio/mid";
        } else if (filename.endsWith(".mp3")) {
            mimeType = "audio/mpeg";
        } else if (filename.endsWith(".xml")) {
            mimeType = "text/xml";
        } else {
            mimeType = "";
        }
        return mimeType;
    }

    public static boolean isDirectory(File file) {
        return file.exists() && file.isDirectory();
    }

    public static boolean isFile(File file) {
        return file.exists() && file.isFile();
    }

    public static boolean createNewDirectory(File file) {
        if (file.exists() && file.isDirectory()) {
            return false;
        }
        return file.mkdirs();
    }

    public static boolean deleteFile(String filePath) {
        if (filePath == null || filePath.length() < 1) {
            return true;
        }
        File file = new File(filePath);
        return deleteFile(file);
    }

    public static boolean deleteFile(File file) {
        if (!file.exists()) {
            return true;
        }
        boolean flag = false;
        if (file.isFile()) {
            flag = file.delete();
        }
        return flag;
    }

    public static void delDirectory(String directoryPath) {
        try {
            delAllFile(directoryPath); // 删除完里面所有内容
            File myFilePath = new File(directoryPath);
            myFilePath.delete(); // 删除空文件夹
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static boolean delAllFile(String path) {
        boolean flag = false;
        File file = new File(path);
        if (!file.exists()) {
            return flag;
        }
        if (!file.isDirectory()) {
            return flag;
        }
        String[] tempList = file.list();
        File temp = null;
        for (int i = 0; i < tempList.length; i++) {
            if (path.endsWith(File.separator)) {
                temp = new File(path + tempList[i]);
            } else {
                temp = new File(path + File.separator + tempList[i]);
            }
            if (temp.isFile()) {
                temp.delete();
            }
            if (temp.isDirectory()) {
                delAllFile(path + "/" + tempList[i]); // 先删除文件夹里面的文件
                delDirectory(path + "/" + tempList[i]); // 再删除空文件夹
                flag = true;
            }
        }
        return flag;
    }

    public static String getHash(String fileName, String hashType) throws Exception {
        InputStream fis;
        fis = new FileInputStream(fileName);
        byte[] buffer = new byte[1024];
        MessageDigest md5 = MessageDigest.getInstance(hashType);
        int numRead = 0;
        while ((numRead = fis.read(buffer)) > 0) {
            md5.update(buffer, 0, numRead);
        }
        fis.close();
        return toHexString(md5.digest());
    }

    public static String toHexString(byte[] b) {
        StringBuilder sb = new StringBuilder(b.length * 2);
        for (int i = 0; i < b.length; i++) {
            sb.append(hexChar[(b[i] & 0xf0) >>> 4]);
            sb.append(hexChar[b[i] & 0x0f]);
        }
        return sb.toString();
    }

    // true :lossless
    // false:loss
    public static boolean isLosslessSupported(File f) {
        String s = f.toString();
        if (s.endsWith(".flac") || s.endsWith(".FLAC")) {
            return true;
        } else if (s.endsWith(".ape") || s.endsWith(".APE")) {
            return true;
        } else if (s.endsWith(".wav") || s.endsWith(".WAV")) {
            return true;
        } else if (s.endsWith(".wv") || s.endsWith(".WV")) {
            return true;
        } else if (s.endsWith(".mpc") || s.endsWith(".MPC")) {
            return true;
        } else if (s.endsWith(".m4a") || s.endsWith(".M4A")) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * 清除目录dirPath下以prefix为前缀的文件
     *
     * @param dirPath
     * @param keyWord
     * @return 文件存在并删除了返回true,否则false
     */
    public static boolean clearFilesWithKeyWord(String dirPath, String keyWord) {
        if (TextUtils.isEmpty(dirPath) || TextUtils.isEmpty(keyWord)) {
            return false;
        }
        File dir = new File(dirPath);
        if (!dir.exists() || !dir.isDirectory()) {
            return false;
        }
        for (File file : dir.listFiles()) {
            String filename = file.getName();
            if (filename.contains(keyWord)) {
                file.delete();
                return true;
            }
        }
        return false;
    }

    /**
     * 清除目录dirPath下后缀名为suffix的文件
     *
     * @param dirPath
     * @param suffix
     */
    public static void clearFiles(String dirPath, String suffix) {
        if (TextUtils.isEmpty(dirPath) || TextUtils.isEmpty(suffix)) {
            return;
        }
        File dir = new File(dirPath);
        if (!dir.exists() || !dir.isDirectory()) {
            return;
        }
        String filename = null;
        for (File file : dir.listFiles()) {
            filename = file.getName();
            if (filename.endsWith(suffix)) {
                file.delete();
            }
        }
    }

    /**
     * 清理路径列表list中后缀名为suffix的文件
     *
     * @param list
     * @param suffix
     */
    public static void clearFiles(ArrayList<String> list, String suffix) {
        if (list == null || list.isEmpty()) {
            return;
        }
        File file = null;
        for (String path : list) {
            if (TextUtils.isEmpty(path)) {
                continue;
            }
            String tmp = getExtension(path);
            if (TextUtils.isEmpty(tmp) || !tmp.equals(suffix)) {
                continue;
            }
            file = new File(path);
            if (file.exists() && file.isFile()) {
                file.delete();
            }
        }
    }

    public static String writeToFile(Context context, byte[] bytes, String filename) {
        return writeToFile(context, bytes, filename, false);
    }

    public static String writeToFile(Context context, byte[] bytes, String filename, boolean isAppend) {
        if (bytes == null || context == null || TextUtils.isEmpty(filename)) {
            return null;
        }
        BufferedOutputStream out = null;
        try {
            out = new BufferedOutputStream(new FileOutputStream(getFile(context, filename), isAppend));
            out.write(bytes);
        } catch (IOException ioe) {
            ioe.printStackTrace();
            Log.e(TAG, "IOException : " + ioe.getMessage());
        } finally {
            try {
                if (out != null) {
                    out.flush();
                    out.close();
                }
            } catch (IOException ioe) {
                ioe.printStackTrace();
            }
        }
        return context.getFilesDir() + File.separator + filename;
    }

    /**
     * 将is输入流指定的数据写入到context对应的/data/data/<package name>/files目录下，filename作为文件名
     *
     * @param context
     * @param is
     * @param filename
     *
     * @return
     */
    public static String writeToFile(Context context, InputStream is, String filename) {
        if (is == null || context == null || TextUtils.isEmpty(filename)) {
            return null;
        }
        BufferedInputStream in = null;
        BufferedOutputStream out = null;
        touchFile(context, filename);
        Log.d(TAG, "writeToFile : " + filename);
        try {
            in = new BufferedInputStream(is);
            out = new BufferedOutputStream(new FileOutputStream(getFile(context, filename)));
            byte[] buffer = new byte[BUF_SIZE];
            int l;
            while ((l = in.read(buffer)) != -1) {
                out.write(buffer, 0, l);
            }
        } catch (IOException ioe) {
            ioe.printStackTrace();
        } finally {
            try {
                if (in != null) {
                    in.close();
                }
                is.close();
                if (out != null) {
                    out.flush();
                    out.close();
                }
            } catch (IOException ioe) {
                ioe.printStackTrace();
            }
        }
        return context.getFilesDir() + File.separator + filename;
    }

    /**
     * 将输入流is指定的数据写入filepath指定的文件中
     *
     * @param is
     * @param filepath
     *
     * @return
     *
     * @throws IOException
     */
    public static String writeToFile(InputStream is, String filepath) throws IOException {
        if (is == null || TextUtils.isEmpty(filepath)) {
            return null;
        }
        BufferedInputStream in = null;
        BufferedOutputStream out = null;
        try {
            Log.d(TAG, "write to file : " + filepath);

            File file = new File(filepath);
            checkDir(file.getParent());
            in = new BufferedInputStream(is);
            out = new BufferedOutputStream(new FileOutputStream(filepath));
            byte[] buffer = new byte[BUF_SIZE];
            int l;
            while ((l = in.read(buffer)) != -1) {
                out.write(buffer, 0, l);
            }
        } catch (IOException e) {
            e.printStackTrace();
            throw e;
        } finally {
            try {
                if (in != null) {
                    in.close();
                }
                is.close();
                if (out != null) {
                    out.flush();
                    out.close();
                }
            } catch (IOException ioe) {
                ioe.printStackTrace();
            }
        }
        return filepath;
    }

    /**
     * 将输入流is指定数据同步写入filepath指定位置
     *
     * @param is
     * @param filepath
     *
     * @return
     *
     * @throws IOException
     */
    public static synchronized  String writeToFileSync(InputStream is, String filepath) throws IOException {
        return writeToFile(is, filepath);
    }

    /**
     * read file to a string
     *
     * @param file
     *
     * @return
     */
    public static String readFileToString(File file) {
        if (null == file) {
            return "";
        }
        FileInputStream fileInput = null;
        StringBuffer strBuf = new StringBuffer();

        try {
            fileInput = new FileInputStream(file);
            byte[] buf = new byte[BUF_SIZE];
            while (fileInput.read(buf) != -1) {
                strBuf.append(new String(buf));
                buf = new byte[BUF_SIZE];
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (SecurityException e) {
            e.printStackTrace();

        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (fileInput != null) {
                try {
                    fileInput.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }

        return strBuf.toString();
    }


    /**
     * read file to byte
     *
     * @param file
     *
     * @return
     */
    public static byte [] readFileToBytes(File file) {
        if (null == file) {
            return null;
        }
        FileInputStream fileInput = null;
        ByteArrayBuffer content = new ByteArrayBuffer(BUF_SIZE);

        try {
            fileInput = new FileInputStream(file);
            byte[] buf = new byte[BUF_SIZE];
            while (fileInput.read(buf) != -1) {
                content.append(buf, 0, BUF_SIZE);
                buf = new byte[BUF_SIZE];
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (SecurityException e) {
            e.printStackTrace();

        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (fileInput != null) {
                try {
                    fileInput.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return content.toByteArray();
    }

    /**
     * list all files from current path
     *
     * @param path
     *
     * @return
     */
    public static File[] listFiles(String path) {
        File dir = new File(path);
        if (!dir.exists()) {
            dir.mkdir();
        }
        try {
            return dir.listFiles();
        } catch (SecurityException ex) {
            return null;
        }
    }

    /**
     * get a file lines
     *
     * @return
     */
    public static int getFileLines(File file) {
        if (null == file) {
            return 0;
        }
        BufferedReader bufReader = null;
        int count = 0;
        try {
            bufReader = new BufferedReader(new FileReader(file));

            while ((bufReader.readLine()) != null) {
                count++;
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            count = 0;
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (bufReader != null) {
                try {
                    bufReader.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return count;
    }

    /**
     * 检查目录是否存在，如果不存在创建之
     *
     * @return 目录是否存在
     */
    public static boolean checkDir(String dirPath) {
        if (TextUtils.isEmpty(dirPath)) {
            return false;
        }
        File dir = new File(dirPath);
        if (dir.exists() && dir.isDirectory()) {
            return true;
        }
        if (dir.exists()) {
            dir.delete();
        }
        return dir.mkdirs();
    }

    /**
     * 获取目录所占空间大小(包括子目录文件)
     *
     * @param dirPath 目录路径
     *
     * @return
     */
    public static long getDirLength(String dirPath) {
        if (TextUtils.isEmpty(dirPath)) {
            return 0;
        }
        File dir = new File(dirPath);
        if (!dir.exists() || !dir.isDirectory()) {
            return 0;
        }
        long length = 0;
        for (File file : dir.listFiles()) {
            if (file.isFile()) {
                length += file.length();
            } else if (file.isDirectory()) {
                length += getDirLength(file.getAbsolutePath());
            }
        }
        return length;
    }

    /**
     * 从指定目录清除最久未使用的文件，被清除的文件大小总和需要大于length
     *
     * @param dirPath 目录路径
     * @param length  要清除掉的文件大小总和
     *
     * @return 被清除的文件数
     */
    public static int removeOldFiles(String dirPath, long length) {
        if (TextUtils.isEmpty(dirPath) || length <= 0) {
            return 0;
        }
        File dir = new File(dirPath);
        if (!dir.exists() || !dir.isDirectory()) {
            return 0;
        }
        File[] files = getFilesByLastModified(dir);
        long l = 0;
        int count = 0;
        for (File file : files) {
            l = file.length();
            if (file.delete()) {
                count++;
                length -= l;
                if (length <= 0) {
                    break;
                }
            }
        }
        return count;
    }

    /**
     * 按照最后修改时间排序，获取目录dir下文件列表
     *
     * @param dir
     *
     * @return
     */
    public static File[] getFilesByLastModified(File dir) {
        if (dir == null || !dir.exists() || !dir.isDirectory()) {
            return null;
        }
        File[] files = dir.listFiles();

        try {
            // jdk7 此算法有修改，有可能报错，设置属性，使其使用旧版本排序
            System.setProperty("java.util.Arrays.useLegacyMergeSort", "true");
            Arrays.sort(files, new Comparator<File>() {
                @Override
                public int compare(File lhs, File rhs) {
                    return (int) (lhs.lastModified() - rhs.lastModified());
                }
            });
        } catch (Exception e) {
            e.printStackTrace();
        }

        return files;
    }

    /**
     * SDCard是否可用
     */
    public static boolean isSDCardAvailable() {
        String status = Environment.getExternalStorageState();
        return status.equals(Environment.MEDIA_MOUNTED);
    }

    /**
     * 清除目录下的所以文件
     *
     * @param dirPath 目录路径
     *
     * @return 是否清除成功
     */
    public static boolean clearDir(String dirPath) {
        File dir = new File(dirPath);
        if (!dir.exists() || !dir.isDirectory()) {
            return false;
        }
        for (File file : dir.listFiles()) {
            if (!file.exists()) {
                continue;
            }
            if (file.isFile()) {
                file.delete();
            }
            if (file.isDirectory()) {
                clearDir(file.getAbsolutePath());
            }
        }
        File[] files = dir.listFiles();
        return files == null || files.length == 0;
    }

    /**
     * 检查文件是否存在于某目录下, 3.3.0
     *
     * @param filePathName
     *
     * @return
     */
    public static String checkFile(String filePathName) {
        // 添加入口参数检查
        if (filePathName == null) {
            return null;
        }
        // 在图片存储目录里检查
        File file = new File(filePathName);
        if (file.exists() || file.isFile()) {
            try {
                return file.getCanonicalPath();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return null;
    }

    /**
     * 检查文件是否存在，不存在时创建相应文件及所在目录
     *
     * @param file
     *
     * @return
     *
     * @throws IOException
     */
    public static boolean checkFile(File file) throws IOException {
        if (file == null) {
            return false;
        }
        if (file.isFile() && file.exists()) {
            return true;
        }
        if (file.exists() && !file.isFile()) {
            file.delete();
        }
        file.getParentFile().mkdirs();
        return file.createNewFile();
    }

    /**
     * 过滤文件名，保证过滤后的文件名为合法文件名<br/>
     * 非法字符将被替换成下划线_
     *
     * @param filename 需要过滤的文件名(不包括父目录路径)
     *
     * @return 过滤后合法的文件名
     */
    public static String filterFileName(String filename) {
        if (TextUtils.isEmpty(filename)) {
            return filename;
        }
        filename = filename.replace(' ', '_');
        filename = filename.replace('"', '_');
        filename = filename.replace('\'', '_');
        filename = filename.replace('\\', '_');
        filename = filename.replace('/', '_');
        filename = filename.replace('<', '_');
        filename = filename.replace('>', '_');
        filename = filename.replace('|', '_');
        filename = filename.replace('?', '_');
        filename = filename.replace(':', '_');
        filename = filename.replace(',', '_');
        filename = filename.replace('*', '_');
        return filename;
    }

    /**
     * 获取SD卡可用大小
     *
     * @return
     */
    @SuppressWarnings("deprecation")
    public static long getSDCardAvailableSpace() {
        File file = Environment.getExternalStorageDirectory(); // 取得sdcard文件路径
        StatFs stat = new StatFs(file.getPath());

        long blockSize = stat.getBlockSize();

        long availableBlocks = stat.getAvailableBlocks();

        return availableBlocks * blockSize;
    }

    /**
     * 检查SD卡可用空间是否满足需要
     *
     * @return
     */
    public static boolean checkSDCardHasEnoughSpace(long size) {
        return getSDCardAvailableSpace() > size;
    }

}