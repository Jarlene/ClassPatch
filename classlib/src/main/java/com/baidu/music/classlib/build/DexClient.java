package com.baidu.music.classlib.build;

import android.text.TextUtils;

import com.android.dx.cf.iface.ParseException;
import com.android.dx.dex.DexOptions;
import com.android.dx.dex.cf.CfOptions;
import com.android.dx.dex.cf.CfTranslator;
import com.android.dx.dex.code.PositionList;
import com.android.dx.dex.file.ClassDefItem;
import com.android.dx.dex.file.DexFile;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.List;

import javassist.CannotCompileException;
import javassist.ClassPool;
import javassist.CtClass;
import javassist.CtConstructor;
import javassist.CtField;
import javassist.CtMethod;
import javassist.Modifier;
import javassist.NotFoundException;


/**
 * 生成dex文件。每一个DexClient只能使用一次。
 * 这里包含了自动写的代码生成class文件，再转为换dex文件逻辑
 * Created by Jarlene on 2015/11/23.
 */
public class DexClient {

    private static DexFile outputDex;

    private final CfOptions cfOptions;

    private final ClassPool classPool;

    private final DexOptions options;

    public DexClient() {
        options = new DexOptions();
        outputDex = new DexFile(options);
        cfOptions = new CfOptions();
        classPool = ClassPool.getDefault();

        cfOptions.positionInfo = PositionList.LINES;
        cfOptions.localInfo = true;
        cfOptions.strictNameCheck = false;
        cfOptions.optimize = false;
        cfOptions.optimizeListFile = null;
        cfOptions.dontOptimizeListFile = null;
        cfOptions.statistics = false;
    }

    /**
     * 将多个class文件生成dex文件
     * @param srcPath
     *          classes文件夹路径
     * @param savePath
     *          dex文件路径
     * @return
     */
    public File makeDex(List<String> srcPath, String savePath) {
        int i = 0;
        String[] pathArray = new String[srcPath.size()];
        for (String path : srcPath) {
            pathArray[i++] = path;
        }
        return makeDex(pathArray, savePath);
    }


    /**
     * 将多个class文件生成dex文件
     * @param srcPath
     *          classes文件夹路径
     * @param savePath
     *          dex文件路径
     * @return
     */
    public File makeDex(String[] srcPath, String savePath) {
        try {
            if (srcPath == null || srcPath.length <= 0 || TextUtils.isEmpty(savePath)) {
                return null;
            }

            byte[][] content = new byte[srcPath.length][];

            for (int i = 0; i < srcPath.length; i++) {
                content[i] = processFile(srcPath[i]);
            }
            byte[] dexByte = classesToDex(srcPath, content);
            return makeDex(savePath, dexByte);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;

    }


    /**
     * 将一个class文件生成dex文件
     * @param srcPath
     *         class文件路径
     * @param savePath
     *         dex文件路径
     * @return
     */
    public File makeDex(String srcPath, String savePath) {
        if (TextUtils.isEmpty(srcPath) || TextUtils.isEmpty(savePath)) {
            return null;
        }
        File srcFile = new File(srcPath);
        if (!srcFile.exists()) {
            return null;
        }
        byte[] buffer = processFile(srcPath);
        byte[] dexByte = classToDex(savePath, buffer);
        return makeDex(savePath, dexByte);

    }

    /**
     * 将文件转化为byt[]数组
     * @param path
     * @return
     */
    private byte[] processFile(String path) {
        if (TextUtils.isEmpty(path)) {
            return null;
        }
        File srcFile = new File(path);
        if (!srcFile.exists()) {
            return null;
        }
        FileInputStream inputStream = null;
        ByteArrayOutputStream byteArrayOutputStream = null;
        try {
            inputStream = new FileInputStream(srcFile);
            byteArrayOutputStream = new ByteArrayOutputStream(1024);
            byte[] b = new byte[1024];
            int n;
            while ((n = inputStream.read(b)) != -1) {
                byteArrayOutputStream.write(b, 0, n);
            }
            return byteArrayOutputStream.toByteArray();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } if (inputStream != null) {
            try {
                inputStream.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        if (byteArrayOutputStream != null) {
            try {
                byteArrayOutputStream.flush();
                byteArrayOutputStream.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return null;
    }

    /**
     * 将dex字节码 转换为Dex文件
     * @param savePath 保存dex文件的路径
     * @param content dex字节
     * @return
     */
    public File makeDex(String savePath, byte[] content) {
        if (TextUtils.isEmpty(savePath) || content == null) {
            return null;
        }
        File file = new File(savePath);
        if (file.exists()) {
            file.delete();
        }
        FileOutputStream fileOutputStream = null;

        try {
            fileOutputStream = new FileOutputStream(file);
            fileOutputStream.write(content);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (fileOutputStream != null) {
                try {
                    fileOutputStream.flush();
                    fileOutputStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return file;
    }

    /**
     * 单个class文件转为dex
     * @param name  class文件路径
     * @param content class读取的内容
     * @return
     */
    public byte[] classToDex(String name, byte[] content) {
        if (TextUtils.isEmpty(name) || content == null) {
            return null;
        }
        if (processClass(name, content)) {
            return writeDex();
        }
        return null;
    }


    /**
     * 将classes文件转换为dex文件。这里面的是一堆的class文件
     * @param names
     * @param byteArrays
     * @return
     */
    public byte[] classesToDex(String[] names, byte[][] byteArrays) {
      for (int i = 0; i < names.length; i++) {
        String name = names[i];
        byte[] byteArray = byteArrays[i];
        processClass(name, byteArray);
      }

      byte[] outputArray = writeDex();

      return outputArray;
    }

    /**
     * Processes one classfile.
     *
     * @param name {@code non-null;} name of the file, clipped such that it
     * <i>should</i> correspond to the name of the class it contains
     * @param bytes {@code non-null;} contents of the file
     * @return whether processing was successful
     */
    private boolean processClass(String name, byte[] bytes) {
        try {
            ClassDefItem clazz = CfTranslator.translate(name, bytes, cfOptions, options);
            outputDex.add(clazz);
            return true;
        } catch (ParseException ex) {
            ex.printStackTrace();
        }
        return false;
    }

    /**
     * Converts {@link #outputDex} into a {@code byte[]}, write
     * it out to the proper file (if any), and also do whatever human-oriented
     * dumping is required.
     *
     * @return {@code null-ok;} the converted {@code byte[]} or {@code null}
     * if there was a problem
     */
    private byte[] writeDex() {
        byte[] outArray = null;

        OutputStreamWriter out = new OutputStreamWriter(new ByteArrayOutputStream());
        try {
            outArray = outputDex.toDex(out, false);
        } catch (Exception ex) {
          ex.printStackTrace();
        } finally {
            if (out != null) {
                try {
                    out.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }

        return outArray;
    }


    /**
     * 生成一个class文件。并设置器父类。
     * @param className 生成的类名
     * @param superClassName 设置父类， 可以为空
     * @return
     */
    public CtClass makeClass(String className, String superClassName) {
        CtClass ctClass = null;
        try {
            ctClass = classPool.makeClass(className);
            if (!TextUtils.isEmpty(superClassName)) {
                classPool.importPackage(superClassName);
                CtClass supClass = classPool.get(superClassName);
                ctClass.setSuperclass(supClass);
            }
        } catch (NotFoundException e) {
            e.printStackTrace();
        } catch (CannotCompileException e) {
            e.printStackTrace();
        }
        return ctClass;
    }

    /**
     * 为Class添加构造函数，并设置其访问类型
     * @param targetClass 目标class
     * @param content 构造函数的内容。
     * @param modifier 构造函数的类型 {@link Modifier}
     * @param parameters 构造函数的参数
     * @return
     */
    public void addConstructor(CtClass targetClass, CtClass[] parameters, String content, int modifier) {
        if (targetClass == null) {
            throw new NullPointerException("CtClass must not be null!");
        }
        CtConstructor constructor = new CtConstructor(parameters, targetClass);
        constructor.setModifiers(modifier);
        try {
            constructor.setBody(content);
            targetClass.addConstructor(constructor);
        } catch (CannotCompileException e) {
            e.printStackTrace();
        }
    }


    /**
     * 为类添加方法。
     * @param targetClass 目标class
     * @param returnType 返回类型
     * @param parameters 参数
     * @param content 内容
     * @param declaring
     * @param modifier 访问类型 {@link Modifier}
     * @return
     */
    public void addMethod(CtClass targetClass, CtClass returnType, CtClass[] parameters,
                          String content, CtClass declaring, int modifier) {
        if (targetClass == null) {
            throw new NullPointerException("CtClass must not be null!");
        }
        CtMethod method = new CtMethod(returnType, content, parameters, declaring);
        try {
            method.setBody(content);
            method.setModifiers(modifier);
            targetClass.addMethod(method);
        } catch (CannotCompileException e) {
            e.printStackTrace();
        }
    }

    /**
     * 添加成员变量
     * @param targetClass 目标class
     * @param type 类型
     * @param name 名字
     * @param declaring
     */
    public void addField(CtClass targetClass, CtClass type, String name, CtClass declaring) {
        if (targetClass == null) {
            throw new NullPointerException("CtClass must not be null!");
        }
        try {
            CtField field = new CtField(type, name, declaring);
            targetClass.addField(field);
        } catch (CannotCompileException e) {
            e.printStackTrace();
        }
    }

}



