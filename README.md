### 前言：
增量更新已经出来几年了，而一些大的公司早就实现了增量更新。增量更新相较于全量更新的好处不言而喻，利用差分算法获得1.0版本到2.0版本的差分包，这样在安装了1.0的设备上只要下载这个差分包就能够完成由1.0-2.0的更新。
###### 存在一个1.0版本的apk：
![image.png](https://upload-images.jianshu.io/upload_images/2406435-23ca802e805c60b7.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

###### 存在一个2.0版本的apk：
![image.png](https://upload-images.jianshu.io/upload_images/2406435-ba56fff6e52621b0.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

这样如果进行全量更新则需要下载完整的25.9大小的apk文件，进行安装。而如果使用增量更新则只需要下载如下 25.3的差分包。

![image.png](https://upload-images.jianshu.io/upload_images/2406435-d7c7247248b988c0.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)


下载数据减少了0.6M（这里只是为了做了功能，patch包和最新的安装包大小差不多的情况：建议做全量更新）。这样做的好处不仅仅在于对于流量的节省。对于用户来说现在流量可能并不值钱，或者使用wifi再进行更新，但是从下载时间能够得到一个良好的优化，同时也减小了服务器的压力。

###### 先来看效果图：

![bianyi.gif](https://upload-images.jianshu.io/upload_images/2406435-cac055b0168968e3.gif?imageMogr2/auto-orient/strip)

![2323.gif](https://upload-images.jianshu.io/upload_images/2406435-29c6450f1c18a129.gif?imageMogr2/auto-orient/strip)




###### Step1：实现增量更新流程：
![image.png](https://upload-images.jianshu.io/upload_images/2406435-3173882cf4873d75.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

###### Step2：实现bsdiff、bspatch源文件的编译
需要实现增量更新，现在有各种开源的制作与合并差分包的开源库，比如：bsdiff、hdiff等等。因此我们只需要获得源码来使用即可。

bsdiff 下载地址： [http://www.daemonology.net/bsdiff/](http://www.daemonology.net/bsdiff/) 
bsdiff 依赖bzip2(zip压缩库)
 [https://nchc.dl.sourceforge.net/project/gnuwin32/bzip2/1.0.5/bzip2-1.0.5-src.zip](https://nchc.dl.sourceforge.net/project/gnuwin32/bzip2/1.0.5/bzip2-1.0.5-src.zip)

下载完成后解压：
![image.png](https://upload-images.jianshu.io/upload_images/2406435-4ebcca08d535cacd.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)
###### 其中:
Makefile(编译bsdiff、bapatch)
bsdiff:		比较两个文件的二进制数据，生成差分包
bspatch:	合并旧的文件与差分包，生成新文件

很显然，bspatch我们需要在Android环境下来执行，而bsdiff 一般会在你的存储服务器当中执行即电脑环境下执行(win或linux)
###### 编译:
对于windows，可以直接从 [https://github.com/cnSchwarzer/bsdiff-win/releases](https://github.com/cnSchwarzer/bsdiff-win/releases) 下载。
而Linux/Mac则可以自行编译：
在Linux中的解压目录直接执行：`make` 会产生错误。需要修改：
```
install:
	${INSTALL_PROGRAM} bsdiff bspatch ${PREFIX}/bin
.ifndef WITHOUT_MAN
	${INSTALL_MAN} bsdiff.1 bspatch.1 ${PREFIX}/man/man1
.endif
#上面这段makefile片段显然有问题(lsn9资料，指令必须以tab开头)
#因此需要修改为：
install:
	${INSTALL_PROGRAM} bsdiff bspatch ${PREFIX}/bin
	.ifndef WITHOUT_MAN
	${INSTALL_MAN} bsdiff.1 bspatch.1 ${PREFIX}/man/man1
	.endif
#也就是在 `.if` 和 `.endif` 前加一个 tab
```
修改后再来执行`make ` 如果出现找不到bzip2 `no file found bzlib.h`之类的错误，则需要先安装bzip2：

Ubuntu:

>apt install libbz2-dev 

Centos:

>yum -y install bzip2-devel.x86_64

Mac:
>brew install bzip2
如果已经安装bzip2，执行make还是出现：

```
bsdiff.c:(.text.startup+0x2aa): undefined reference to `BZ2_bzWriteOpen'
bsdiff.c:(.text.startup+0xcfa): undefined reference to `BZ2_bzWrite'
bsdiff.c:(.text.startup+0xe37): undefined reference to `BZ2_bzWrite'
bsdiff.c:(.text.startup+0xf80): undefined reference to `BZ2_bzWrite'
bsdiff.c:(.text.startup+0xfe1): undefined reference to `BZ2_bzWriteClose'
bsdiff.c:(.text.startup+0x1034): undefined reference to `BZ2_bzWriteOpen'
bsdiff.c:(.text.startup+0x105c): undefined reference to `BZ2_bzWrite'
bsdiff.c:(.text.startup+0x1082): undefined reference to `BZ2_bzWriteClose'
bsdiff.c:(.text.startup+0x10d5): undefined reference to `BZ2_bzWriteOpen'
bsdiff.c:(.text.startup+0x1100): undefined reference to `BZ2_bzWrite'
bsdiff.c:(.text.startup+0x1126): undefined reference to `BZ2_bzWriteClose'
```
则修改Makefile为：
```
CFLAGS          +=      -O3 -lbz2

PREFIX          ?=      /usr/local
INSTALL_PROGRAM ?=      ${INSTALL} -c -s -m 555
INSTALL_MAN     ?=      ${INSTALL} -c -m 444

all:            bsdiff bspatch
bsdiff:         bsdiff.c
        cc bsdiff.c ${CFLAGS} -o bsdiff  #增加
bspatch:        bspatch.c
        cc bspatch.c ${CFLAGS} -o bspatch #增加

install:
        ${INSTALL_PROGRAM} bsdiff bspatch ${PREFIX}/bin
        .ifndef WITHOUT_MAN
        ${INSTALL_MAN} bsdiff.1 bspatch.1 ${PREFIX}/man/man1
        .endif
```
>cc bsdiff.c \${CFLAGS} -o bsdiff  #增加
cc bspatch.c ${CFLAGS} -o bspatch #增加



猜测可能是编译器的原因。
######Step3:AS工程中去配置
1、创建一个AS（NDK）工程
2、在app目录下的grade文件中增加编译架构类型
```
externalNativeBuild {
            cmake {
                cppFlags ""
                abiFilters 'armeabi-v7a'//加入
            }
        }
```
3、将下载好的bsdiff 解压之后，将bspatch.c复制进cpp目录下。同时也将bspatch.c依赖的bzip2库的源文件和头文件复制进cpp目录下。
![pe](https://upload-images.jianshu.io/upload_images/2406435-83dee9cb8a48bd1a.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

4、配置CMakeLists.txt
```
 cmake_minimum_required(VERSION 3.4.1)


 file(GLOB bzip_source src/main/cpp/bzip/*.c)

 add_library(
              native-lib

              SHARED

              src/main/cpp/native-lib.cpp
              src/main/cpp/bspatch.c
              ${bzip_source}
              
               )
 include_directories(src/main/cpp/bzip)


 find_library( 
               log-lib
               log )


 target_link_libraries( 
                        native-lib
                        ${log-lib} )
```
5、配置好之后，写一个native方法
```
    /**
     *
     * @param oldApk 当前运行的APK
     * @param patch 差分包
     * @param output 合成后新的APK输出到
     */
    native void bspatch(String oldApk,String patch,String output);
```
```
extern "C" {
//bspatch.c中在执行合成方法就是其中main(int argc,char * argv[])
//所以在这里需要引入该方法
extern int main(int argc,char * argv[]);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ly_chen_incrementalupdates_MainActivity_bspatch(JNIEnv *env, jobject instance,
                                                         jstring oldApk_, jstring patch_,
                                                         jstring output_) {
    const char *oldApk = env->GetStringUTFChars(oldApk_, 0);
    const char *patch = env->GetStringUTFChars(patch_, 0);
    const char *output = env->GetStringUTFChars(output_, 0);


    char * argv[4] = {"", const_cast<char *>(oldApk), const_cast<char *>(output),
                      const_cast<char *>(patch)};
    main(4,argv);

    env->ReleaseStringUTFChars(oldApk_, oldApk);
    env->ReleaseStringUTFChars(patch_, patch);
    env->ReleaseStringUTFChars(output_, output);
}
```



6、接下我们做合成了安装包了，不过的注意这个操作是一个耗时操作
```
//1、合成 apk
        //先从服务器下载到差分包
        new AsyncTask<Void,Void,File>(){
            @Override
            protected File doInBackground(Void... voids) {
                //拿到已经安装版本路径
                String old = getApplication().getApplicationInfo().sourceDir;
                //合成新的安装包（备注：记得路径 等会放置 patch包时候需要）
                bspatch(old,"/sdcard/patch","/sdcard/new.apk");
                //返回新的APK文件
                return new File("/sdcard/new.apk");
            }

            @Override
            protected void onPostExecute(File file) {
                Intent intent = new Intent(Intent.ACTION_VIEW);
                //兼容7.0 
                if (Build.VERSION.SDK_INT < Build.VERSION_CODES.N) {
                    intent.setDataAndType(Uri.fromFile(file), "application/vnd.android.package-archive");
                } else {
                    // 声明需要的临时权限
                    intent.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                    // 第二个参数，即第一步中配置的authorities
                    String packageName = getApplication().getPackageName();
                    Uri contentUri = FileProvider.getUriForFile(MainActivity.this, packageName + ".fileprovider", file);
                    intent.setDataAndType(contentUri, "application/vnd.android.package-archive");
                }
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(intent);
            }
        }.execute();
```

7、兼容7.0和8.0
兼容7.0：在AndroidManifest.xml配置
```
 <!--四大组件之一 内容提供者 可以跨进程使用，7.0安装必须使用它-->
        <provider
            android:authorities="com.ly.chen.incrementalupdates.fileprovider"
            android:name="android.support.v4.content.FileProvider"
            android:exported="false"
            android:grantUriPermissions="true">

            <meta-data
                android:name="android.support.FILE_PROVIDER_PATHS"
                android:resource="@xml/file_paths"/>
        </provider>

```
新建一个xml文件
![image.png](https://upload-images.jianshu.io/upload_images/2406435-6ae0b8a94009d855.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

暴露手机SD卡的路径
```
<?xml version="1.0" encoding="utf-8"?>
<resource>
    <paths>
        <external-path name="haah" path="" />
    </paths>
</resource>
```
适配8.0：加入未知来源权限。备注：只是做Demo演示，没有动态申请权限，需手动开启相关权限
```
<!--存储-->
<uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE"/>
<!--位未知来源权限,适配8.0-->
<uses-permission android:name="android.permission.REQUEST_INSTALL_PACKAGES"/>
```
分别打包 V1.0.0版本 V2.0.0版本

Step3:生成差分包 这里我直接用的是Windows编译的bsdiff.exe生成的差分包（对于windows，可以直接从 [https://github.com/cnSchwarzer/bsdiff-win/releases](https://github.com/cnSchwarzer/bsdiff-win/releases) 下载。）

![image.png](https://upload-images.jianshu.io/upload_images/2406435-bb424a5c89070fff.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)
上图：
其中app1.apk:V1.0.0
其中app2.apk:V2.0.0
其中patch:差分包
下载好windows 版 解压出现：
bsdiff.exe:用于生产差分包
bspatch.exe:用于合成差分包玉低版本APK
在当前文件夹下输入：cmd
>bsdiff.exe app1.apk app2.apk patch 
生成patch

将patch包放置进 手机SD卡
>adb push patch /sdcard/

安装低版本APK
>adb install app1.apk

好了，安装完成之后（注意手动开启相关权限，不然会闪退），点击更新按钮，就开始更新了。。。不出意外的话，你会看到版本已经变了。
