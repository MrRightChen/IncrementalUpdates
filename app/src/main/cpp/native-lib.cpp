#include <jni.h>
#include <string>


extern "C" {
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