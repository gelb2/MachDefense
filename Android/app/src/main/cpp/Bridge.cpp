#include "Runtime.hpp"
#include <jni.h>
#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>
#include <android/log.h>
namespace {
using md::android::Runtime;
std::string text(JNIEnv* e,jstring s){if(!s)return {};auto* p=e->GetStringUTFChars(s,nullptr);if(!p)return {};std::string result(p);e->ReleaseStringUTFChars(s,p);return result;}
template<class F>void run(jlong p,F f){if(!p)return;auto& r=*reinterpret_cast<Runtime*>(p);try{f(r);}catch(const std::exception& e){r.error=e.what();__android_log_print(ANDROID_LOG_ERROR,"MachDefense","%s",e.what());try{r.active(false);}catch(...){/* Preserve the first failure; never unwind across JNI. */}}}
}
extern "C" JNIEXPORT jlong JNICALL Java_com_gelb2_machdefense_Native_create(JNIEnv* e,jobject,jobject assets,jstring files){
    try{
        auto engine=static_cast<jclass>(e->NewGlobalRef(e->FindClass("com/gelb2/machdefense/GameEngine")));
        auto music=e->GetStaticMethodID(engine,"music","(Ljava/lang/String;F)V");
        auto active=e->GetStaticMethodID(engine,"musicActive","(Z)V");
        auto name=e->GetStaticMethodID(engine,"requestName","(Ljava/lang/String;IJ)V");
        auto transferOpen=e->GetStaticMethodID(engine,"openTransferField","(Ljava/lang/String;ZFFFFJ)V");
        auto transferLayout=e->GetStaticMethodID(engine,"layoutTransferField","(FFFFJ)V");
        auto transferClose=e->GetStaticMethodID(engine,"closeTransferField","(J)V");
        auto transferCopy=e->GetStaticMethodID(engine,"copyTransferCode","(Ljava/lang/String;)V");
        if(e->ExceptionCheck()||!music||!active||!name||!transferOpen||!transferLayout||!transferClose||!transferCopy)return 0;
        // All native methods and callbacks run on the process-lifetime render thread.
        md::android::Callbacks callbacks;
        callbacks.music=[=](std::string value,float gain){auto s=e->NewStringUTF(value.c_str());e->CallStaticVoidMethod(engine,music,s,gain);e->DeleteLocalRef(s);};
        callbacks.musicActive=[=](bool value){e->CallStaticVoidMethod(engine,active,jboolean(value));};
        callbacks.name=[=](std::string value,unsigned maximum,std::uint64_t generation){auto s=e->NewStringUTF(value.c_str());e->CallStaticVoidMethod(engine,name,s,jint(maximum),jlong(generation));e->DeleteLocalRef(s);};
        callbacks.transferOpen=[=](std::string value,bool readOnly,md::Rect r,std::uint64_t generation){auto s=e->NewStringUTF(value.c_str());e->CallStaticVoidMethod(engine,transferOpen,s,jboolean(readOnly),r.x,r.y,r.width,r.height,jlong(generation));e->DeleteLocalRef(s);};
        callbacks.transferLayout=[=](md::Rect r,std::uint64_t generation){e->CallStaticVoidMethod(engine,transferLayout,r.x,r.y,r.width,r.height,jlong(generation));};
        callbacks.transferClose=[=](std::uint64_t generation){e->CallStaticVoidMethod(engine,transferClose,jlong(generation));};
        callbacks.transferCopy=[=](std::string value){auto s=e->NewStringUTF(value.c_str());e->CallStaticVoidMethod(engine,transferCopy,s);e->DeleteLocalRef(s);};
        return reinterpret_cast<jlong>(new Runtime(AAssetManager_fromJava(e,assets),text(e,files),std::move(callbacks)));
    }catch(const std::exception& x){e->ThrowNew(e->FindClass("java/lang/IllegalStateException"),x.what());return 0;}
}
extern "C" JNIEXPORT void JNICALL Java_com_gelb2_machdefense_Native_surface(JNIEnv*e,jobject,jlong p,jobject surface){auto* w=surface?ANativeWindow_fromSurface(e,surface):nullptr;run(p,[&](auto&r){r.surface(w);});if(w)ANativeWindow_release(w);}
extern "C" JNIEXPORT void JNICALL Java_com_gelb2_machdefense_Native_active(JNIEnv*,jobject,jlong p,jboolean on){run(p,[&](auto&r){r.active(on);});}
extern "C" JNIEXPORT void JNICALL Java_com_gelb2_machdefense_Native_focus(JNIEnv*,jobject,jlong p,jboolean on){run(p,[&](auto&r){r.focus(on);});}
extern "C" JNIEXPORT void JNICALL Java_com_gelb2_machdefense_Native_insets(JNIEnv*,jobject,jlong p,jint t,jint b,jfloat d){run(p,[&](auto&r){r.insets(t,b,d);});}
extern "C" JNIEXPORT void JNICALL Java_com_gelb2_machdefense_Native_frame(JNIEnv*,jobject,jlong p,jdouble time){run(p,[&](auto&r){r.frame(time);});}
extern "C" JNIEXPORT void JNICALL Java_com_gelb2_machdefense_Native_touch(JNIEnv*,jobject,jlong p,jint id,jint phase,jfloat x,jfloat y){run(p,[&](auto&r){r.touch(id,phase,x,y);});}
extern "C" JNIEXPORT void JNICALL Java_com_gelb2_machdefense_Native_nameResult(JNIEnv*e,jobject,jlong p,jlong generation,jstring value){run(p,[&](auto&r){r.nameResult(generation,value?std::optional(text(e,value)):std::nullopt);});}
extern "C" JNIEXPORT void JNICALL Java_com_gelb2_machdefense_Native_command(JNIEnv*e,jobject,jlong p,jstring command,jint a,jint b){auto name=text(e,command);run(p,[&](auto&r){r.command(name,a,b);});}
extern "C" JNIEXPORT jstring JNICALL Java_com_gelb2_machdefense_Native_status(JNIEnv*e,jobject,jlong p){std::string result="{}";run(p,[&](auto&r){result=r.status();});return e->NewStringUTF(result.c_str());}

extern "C" JNIEXPORT void JNICALL Java_com_gelb2_machdefense_Native_transferTextChanged(JNIEnv*e,jobject,jlong p,jlong generation,jstring value){run(p,[&](auto&r){r.transferTextChanged(generation,text(e,value));});}
