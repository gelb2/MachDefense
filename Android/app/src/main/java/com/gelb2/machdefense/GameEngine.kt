package com.gelb2.machdefense
import android.content.Context
import android.os.*
import android.view.Choreographer
import android.view.Surface
import android.util.Log
import java.lang.ref.WeakReference
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit

/** A single game and render thread survive Activity and Surface replacement. */
internal object GameEngine {
    private val thread=HandlerThread("MachDefense GL").apply { start() }
    private val queue=Handler(thread.looper)
    private val main=Handler(Looper.getMainLooper())
    private var handle=0L
    private var initialized=false
    private var running=false
    private var hasSurface=false
    private var framePending=false
    private var lastFrame=0L
    private var failure=""
    private var activity=WeakReference<GameActivity>(null)
    private var app:Context?=null
    data class NameRequest(val text:String,val maximum:Int,val generation:Long)
    private var pendingName:NameRequest?=null // main-thread ownership
    fun bind(owner:GameActivity) {
        activity=WeakReference(owner)
        pendingName?.let { owner.showName(it) }
        val context=owner.applicationContext
        queue.post {
            if(!initialized){initialized=true;app=context;Music.initialize(context)
                try {handle=Native.create(context.assets,context.filesDir.absolutePath)}catch(e:Exception){failure=e.toString();Log.e("MachDefense","Initialization failed",e)}
            }
            report()
        }
    }
    private fun report(){if(failure.isNotEmpty())Log.e("MachDefense",failure) else if(handle!=0L)Log.i("MachDefense",Native.status(handle))}
    private val callback:Choreographer.FrameCallback=Choreographer.FrameCallback { time ->
        framePending=false
        if(running&&hasSurface&&handle!=0L){
            // At most one core tick per callback; never replay missed frames.
            if(lastFrame==0L||time-lastFrame>=16_000_000L){Native.frame(handle,time/1_000_000_000.0);lastFrame=time}
            schedule()
        }
    }
    private fun schedule(){if(!framePending&&running&&hasSurface&&handle!=0L){framePending=true;Choreographer.getInstance().postFrameCallback(callback)}}
    private fun unschedule(){Choreographer.getInstance().removeFrameCallback(callback);framePending=false;lastFrame=0L}
    fun surface(surface:Surface?){
        val done=CountDownLatch(1)
        queue.post{try{unschedule();hasSurface=surface!=null;if(handle!=0L)Native.surface(handle,surface);schedule();report()}finally{done.countDown()}}
        if(surface==null&&!done.await(5,TimeUnit.SECONDS))Log.e("MachDefense","Surface release timed out")
    }
    fun active(on:Boolean){queue.post{running=on;unschedule();if(handle!=0L)Native.active(handle,on);schedule();report()}}
    fun focus(on:Boolean){queue.post{if(handle!=0L)Native.focus(handle,on)}}
    fun insets(top:Int,bottom:Int,density:Float){queue.post{if(handle!=0L)Native.insets(handle,top,bottom,density)}}
    fun touch(id:Int,phase:Int,x:Float,y:Float){queue.post{if(handle!=0L)Native.touch(handle,id,phase,x,y)}}
    @JvmStatic fun music(name:String,gain:Float){Music.play(name,gain)}
    @JvmStatic fun musicActive(on:Boolean){Music.active(on)}
    @JvmStatic fun requestName(text:String,maximum:Int,generation:Long){main.post{val request=NameRequest(text,maximum,generation);pendingName=request;activity.get()?.showName(request)}}
    fun nameResult(generation:Long,value:String?){if(pendingName?.generation!=generation)return;pendingName=null;queue.post{if(handle!=0L)Native.nameResult(handle,generation,value)}}
    fun command(name:String,a:Int=0,b:Int=0,done:()->Unit={}){
        if(BuildConfig.CHEAT&&name=="recreate-activity"){main.post{try{activity.get()?.recreate()}finally{done()}};return}
        queue.post{try{if(BuildConfig.CHEAT&&handle!=0L){Native.command(handle,name,a,b);report()}}finally{done()}}}
}
