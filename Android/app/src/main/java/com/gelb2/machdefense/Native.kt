package com.gelb2.machdefense
import android.content.res.AssetManager
import android.view.Surface
internal object Native {
    init { System.loadLibrary("machdefense") }
    external fun create(assets:AssetManager, files:String):Long
    external fun surface(handle:Long,surface:Surface?)
    external fun active(handle:Long,on:Boolean)
    external fun focus(handle:Long,on:Boolean)
    external fun insets(handle:Long,top:Int,bottom:Int,density:Float)
    external fun frame(handle:Long,time:Double)
    external fun touch(handle:Long,id:Int,phase:Int,x:Float,y:Float)
    external fun nameResult(handle:Long,generation:Long,value:String?)
    external fun command(handle:Long,name:String,a:Int,b:Int)
    external fun status(handle:Long):String
}
