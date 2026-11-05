package com.gelb2.machdefense
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
/** Explicit ADB commands exist only in the separate validation package. */
class TestReceiver:BroadcastReceiver(){
    override fun onReceive(context:Context,intent:Intent){
        val pending=goAsync()
        GameEngine.command(intent.getStringExtra("command")?:"snapshot",intent.getIntExtra("a",0),intent.getIntExtra("b",0)){pending.finish()}
    }
}
