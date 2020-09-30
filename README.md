# ClassPatch
# 第一、原理介绍
    相关原理请参考：http://blog.csdn.net/xwl198937/article/details/49801975
# 第二、生成patch apk
     1、将改动的java文件编译生成class文件。
     2、将所有的class文件放进/ClassPatch/classes/目录下面，
     3、执行sample生成classes.dex文件在/ClassPatch/classes/
     4、随便找一个apk，重命名为.zip文件，将刚刚生成的clases.dex文件放进去，在重命名为.apk
     5、将apk放进data/data/app-packageName/patch目录下。（具体参考sample中的复制按钮选项）
     6、重启app后，patch才能生效
     或者你使用tool目录下面的工具生成patch apk（这个是阿里巴巴andfix开源中的patch生成器）（这个还是有点问题）之后我会自己弄一个出来。包括资源，混淆，dex等等
# 第三、说明
    1、由于现在还在完善中，后期考虑加入patch实时生效的能力，还有替换布局的能力。具体参考我的另一个博客:http://blog.csdn.net/xwl198937/article/details/50134861
    2、目录下面的patch是已经生成的patch apk，可以直接用，将其放进sdcard中，并重命名为ClassPatch
    3、so hook 是正在研究的项目，有兴趣的可以一起讨论一下。sample中so替换功能是不能实现的。
    4、现在已经基本实现dalvik系统中patch实时生效的能力。原理是在proxyDvmResolveClass增加标识，不选择从dvmDexGetResolvedClass获取class。
    5、实现so hook功能，现在已经实现了

Explaining it in English :
    Update the PATH Environment Variable (Microsoft Windows)

You can run Java applications just fine without setting the PATH environment variable. Or, you can optionally set it as a convenience.

Set the PATH environment variable if you want to be able to conveniently run the executables (javac.exe, java.exe, javadoc.exe, and so on) from any directory without having to type the full path of the command. If you do not set the PATH variable, you need to specify the full path to the executable every time you run it, such as:

C:\Java\jdk1.7.0\bin\javac MyClass.java

The PATH environment variable is a series of directories separated by semicolons (;). Microsoft Windows looks for programs in the PATH directories in order, from left to right. You should have only one bin directory for the JDK in the path at a time (those following the first are ignored), so if one is already present, you can update that particular entry.

The following is an example of a PATH environment variable:

C:\Java\jdk1.7.0\bin;C:\Windows\System32\;C:\Windows\;C:\Windows\System32\Wbem

It is useful to set the PATH environment variable permanently so it will persist after rebooting. To make a permanent change to the PATH variable, use the System icon in the Control Panel. The precise procedure varies depending on the version of Windows:
Windows XP

    Select Start, select Control Panel. double click System, and select the Advanced tab.
    Click Environment Variables. In the section System Variables, find the PATH environment variable and select it. Click Edit. If the PATH environment variable does not exist, click New.
    In the Edit System Variable (or New System Variable) window, specify the value of the PATH environment variable. Click OK. Close all remaining windows by clicking OK.

Windows Vista:

    From the desktop, right click the My Computer icon.
    Choose Properties from the context menu.
    Click the Advanced tab (Advanced system settings link in Vista).
    Click Environment Variables. In the section System Variables, find the PATH environment variable and select it. Click Edit. If the PATH environment variable does not exist, click New.
    In the Edit System Variable (or New System Variable) window, specify the value of the PATH environment variable. Click OK. Close all remaining windows by clicking OK.

Windows 7:

    From the desktop, right click the Computer icon.
    Choose Properties from the context menu.
    Click the Advanced system settings link.
    Click Environment Variables. In the section System Variables, find the PATH environment variable and select it. Click Edit. If the PATH environment variable does not exist, click New.
    In the Edit System Variable (or New System Variable) window, specify the value of the PATH environment variable. Click OK. Close all remaining windows by clicking OK.

Note: You may see a PATH environment variable similar to the following when editing it from the Control Panel:

%JAVA_HOME%\bin;%SystemRoot%\system32;%SystemRoot%;%SystemRoot%\System32\Wbem

Variables enclosed in percentage signs (%) are existing environment variables. If one of these variables is listed in the Environment Variables window from the Control Panel (such as JAVA_HOME), then you can edit its value. If it does not appear, then it is a special environment variable that the operating system has defined. For example, SystemRoot is the location of the Microsoft Windows system folder. To obtain the value of a environment variable, enter the following at a command prompt. (This example obtains the value of the SystemRoot environment variable):

echo %SystemRoot%

Update the PATH Variable (Solaris and Linux)

You can run the JDK just fine without setting the PATH variable, or you can optionally set it as a convenience. However, you should set the path variable if you want to be able to run the executables (javac, java, javadoc, and so on) from any directory without having to type the full path of the command. If you do not set the PATH variable, you need to specify the full path to the executable every time you run it, such as:

% /usr/local/jdk1.7.0/bin/javac MyClass.java

To find out if the path is properly set, execute:

% java -version

This will print the version of the java tool, if it can find it. If the version is old or you get the error java: Command not found, then the path is not properly set.

To set the path permanently, set the path in your startup file.

For C shell (csh), edit the startup file (~/.cshrc):

set path=(/usr/local/jdk1.7.0/bin $path)

For bash, edit the startup file (~/.bashrc):

PATH=/usr/local/jdk1.7.0/bin:$PATH
export PATH

For ksh, the startup file is named by the environment variable, ENV. To set the path:

PATH=/usr/local/jdk1.7.0/bin:$PATH
export PATH

For sh, edit the profile file (~/.profile):

PATH=/usr/local/jdk1.7.0/bin:$PATH
export PATH

Then load the startup file and verify that the path is set by repeating the java command:

For C shell (csh):

% source ~/.cshrc
% java -version

For ksh, bash, or sh:

% . /.profile
% java -version

Checking the CLASSPATH variable (All platforms)

The CLASSPATH variable is one way to tell applications, including the JDK tools, where to look for user classes. (Classes that are part of the JRE, JDK platform, and extensions should be defined through other means, such as the bootstrap class path or the extensions directory.)

The preferred way to specify the class path is by using the -cp command line switch. This allows the CLASSPATH to be set individually for each application without affecting other applications. Setting the CLASSPATH can be tricky and should be performed with care.

The default value of the class path is ".", meaning that only the current directory is searched. Specifying either the CLASSPATH variable or the -cp command line switch overrides this value.

To check whether CLASSPATH is set on Microsoft Windows NT/2000/XP, execute the following:

C:> echo %CLASSPATH%

On Solaris or Linux, execute the following:

% echo $CLASSPATH

If CLASSPATH is not set you will get a CLASSPATH: Undefined variable error (Solaris or Linux) or simply %CLASSPATH% (Microsoft Windows NT/2000/XP).

To modify the CLASSPATH, use the same procedure you used for the PATH variable.

Class path wildcards allow you to include an entire directory of .jar files in the class path without explicitly naming them individually. For more information, including an explanation of class path wildcards, and a detailed description on how to clean up the CLASSPATH environment variable, see the Setting the Class Path technical note.
