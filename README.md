# ClassPatch
第一、原理介绍
   相关原理请参考
第二、生成patch apk
    1、将改动的java文件编译生成class文件。
    2、将所有的class文件放进/ClassPatch/classes/目录下面，
    3、执行sample生成classes.dex文件在/ClassPatch/classes/
    4、随便找一个apk，重命名为.zip文件，将刚刚生成的clases.dex文件放进去，在重命名为.apk
    5、将apk放进data/data/app-packageName/patch目录下。（具体参考sample中的复制按钮选项）
    6、重启app后，patch才能生效
第三、说明
   由于现在还在完善中，后期考虑加入patch实时生效的能力，还有替换布局的能力。具体参考我的另一个博客:http://blog.csdn.net/xwl198937/article/details/50134861

