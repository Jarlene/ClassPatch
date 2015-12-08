# ClassPatch
1、原理介绍
   相关原理请参考
2、生成patch apk
    1、将改动的java文件编译生成class文件。
    2、将所有的class文件放进/ClassPatch/classes/目录下面，
    3、执行sample生成classes.dex文件在/ClassPatch/classes/
    4、随便找一个apk，重命名为.zip文件，将刚刚生成的clases.dex文件放进去，在重命名为.apk
    5、将apk放进data/data/app-packageName/patch目录下。（具体参考sample中的复制按钮选项）
    6、重启app后，patch才能生效

