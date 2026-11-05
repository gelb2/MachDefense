package com.gelb2.machdefense
import android.content.Context
import android.media.AudioAttributes
import android.media.MediaPlayer
import android.os.Handler
import android.os.HandlerThread
import android.util.Log

/** Android compressed-music streaming. Effects remain in the shared OpenAL module. */
internal object Music {
    private val thread=HandlerThread("MachDefense music").apply{start()}
    private val queue=Handler(thread.looper)
    private var context:Context?=null
    private var player:MediaPlayer?=null
    private var track=""
    private var ready=false
    private var active=false
    private var volume=1f
    fun initialize(context:Context){queue.post{this.context=context}}
    fun active(on:Boolean){queue.post{active=on;update()}}
    private fun update(){val p=player?:return;if(!ready)return;try{p.setVolume(volume,volume);if(active&&!p.isPlaying)p.start() else if(!active&&p.isPlaying)p.pause()}catch(e:IllegalStateException){Log.e("MachDefense","Music state",e)}}
    fun play(name:String,gain:Float){queue.post{
        if(!gain.isFinite())return@post
        volume=gain.coerceIn(0f,1f)
        if(track==name&&player!=null){update();return@post}
        player?.release();player=null;track=name;ready=false
        if(name.isEmpty())return@post
        try{
            val p=MediaPlayer();player=p
            p.setAudioAttributes(AudioAttributes.Builder().setUsage(AudioAttributes.USAGE_GAME).setContentType(AudioAttributes.CONTENT_TYPE_MUSIC).build())
            context!!.assets.openFd("Audio/$name").use{p.setDataSource(it.fileDescriptor,it.startOffset,it.length)}
            p.isLooping=true
            p.setOnPreparedListener{if(player===it){ready=true;update();Log.i("MachDefense","Music ready: $track")}}
            p.setOnErrorListener{_,what,extra->Log.e("MachDefense","Music error $what/$extra: $track");true}
            p.prepareAsync()
        }catch(e:Exception){Log.e("MachDefense","Music open: $name",e);player?.release();player=null;ready=false;track=""}
    }}
}
