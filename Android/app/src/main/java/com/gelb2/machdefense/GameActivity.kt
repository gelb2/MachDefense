package com.gelb2.machdefense
import android.app.Activity
import android.app.AlertDialog
import android.content.Context
import android.media.AudioAttributes
import android.media.AudioFocusRequest
import android.media.AudioManager
import android.os.*
import android.text.InputFilter
import android.text.InputType
import android.text.Editable
import android.text.TextWatcher
import android.graphics.Color
import android.graphics.Typeface
import android.view.inputmethod.InputMethodManager
import android.view.inputmethod.EditorInfo
import android.widget.FrameLayout
import kotlin.math.roundToInt
import android.view.*
import android.widget.EditText

class GameActivity:Activity(), SurfaceHolder.Callback {
    private lateinit var surface:SurfaceView
    private lateinit var root:FrameLayout
    private var transferInput:EditText?=null
    private var transferGeneration=0L
    private lateinit var audio:AudioManager
    private lateinit var focus:AudioFocusRequest
    private var resumed=false
    private var nameDialog:AlertDialog?=null
    @Suppress("DEPRECATION")
    override fun onCreate(state:Bundle?){
        super.onCreate(state)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        window.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_NOTHING)
        if(Build.VERSION.SDK_INT>=28)window.attributes=window.attributes.apply{layoutInDisplayCutoutMode=WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES}
        if(Build.VERSION.SDK_INT>=30){window.setDecorFitsSystemWindows(false);window.decorView.post { window.insetsController?.let{it.hide(WindowInsets.Type.systemBars());it.systemBarsBehavior=WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE} }}
        else window.decorView.systemUiVisibility=View.SYSTEM_UI_FLAG_FULLSCREEN or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION or View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION or View.SYSTEM_UI_FLAG_LAYOUT_STABLE
        audio=getSystemService(Context.AUDIO_SERVICE) as AudioManager
        focus=AudioFocusRequest.Builder(AudioManager.AUDIOFOCUS_GAIN)
            .setAudioAttributes(AudioAttributes.Builder().setUsage(AudioAttributes.USAGE_GAME).setContentType(AudioAttributes.CONTENT_TYPE_MUSIC).build())
            .setOnAudioFocusChangeListener({change->GameEngine.focus(resumed&&change==AudioManager.AUDIOFOCUS_GAIN)},Handler(Looper.getMainLooper())).build()
        surface=SurfaceView(this);surface.holder.addCallback(this)
        surface.setOnApplyWindowInsetsListener{_,insets->
            val top=if(Build.VERSION.SDK_INT>=28)insets.displayCutout?.safeInsetTop?:0 else 0
            val bottom=if(Build.VERSION.SDK_INT>=30)insets.getInsets(WindowInsets.Type.navigationBars()).bottom else @Suppress("DEPRECATION") insets.systemWindowInsetBottom
            GameEngine.insets(top,bottom,resources.displayMetrics.density);insets
        }
        surface.setOnTouchListener{_,event->
            when(event.actionMasked){
                MotionEvent.ACTION_DOWN,MotionEvent.ACTION_POINTER_DOWN->{val i=event.actionIndex;GameEngine.touch(event.getPointerId(i),0,event.getX(i),event.getY(i))}
                MotionEvent.ACTION_MOVE->for(i in 0 until event.pointerCount)GameEngine.touch(event.getPointerId(i),1,event.getX(i),event.getY(i))
                MotionEvent.ACTION_UP,MotionEvent.ACTION_POINTER_UP->{val i=event.actionIndex;GameEngine.touch(event.getPointerId(i),2,event.getX(i),event.getY(i))}
                MotionEvent.ACTION_CANCEL->for(i in 0 until event.pointerCount)GameEngine.touch(event.getPointerId(i),3,event.getX(i),event.getY(i))
            };true
        }
        root=FrameLayout(this);root.addView(surface,FrameLayout.LayoutParams(-1,-1))
        setContentView(root);GameEngine.bind(this);surface.requestApplyInsets()
    }
    override fun onWindowFocusChanged(hasFocus:Boolean){
        super.onWindowFocusChanged(hasFocus)
        if(hasFocus&&nameDialog==null&&Build.VERSION.SDK_INT>=30)window.insetsController?.hide(WindowInsets.Type.systemBars())
    }
    override fun onResume(){super.onResume();resumed=true;GameEngine.active(true);GameEngine.focus(audio.requestAudioFocus(focus)==AudioManager.AUDIOFOCUS_REQUEST_GRANTED)}
    override fun onPause(){resumed=false;GameEngine.active(false);GameEngine.focus(false);audio.abandonAudioFocusRequest(focus);super.onPause()}
    override fun surfaceCreated(holder:SurfaceHolder){if(Build.VERSION.SDK_INT>=30)holder.surface.setFrameRate(60f,Surface.FRAME_RATE_COMPATIBILITY_FIXED_SOURCE)}
    override fun surfaceChanged(holder:SurfaceHolder,format:Int,width:Int,height:Int){GameEngine.surface(holder.surface);surface.requestApplyInsets()}
    override fun surfaceDestroyed(holder:SurfaceHolder){GameEngine.surface(null)}
    override fun onDestroy(){nameDialog?.dismiss();nameDialog=null;closeTransfer(transferGeneration);super.onDestroy()}
    internal fun showTransfer(request:GameEngine.TransferRequest){
        if(isFinishing||isDestroyed)return
        closeTransfer(transferGeneration)
        transferGeneration=request.generation
        val input=EditText(this).apply{
            contentDescription="Recovery code"
            setTextColor(Color.rgb(230,230,220));setBackgroundColor(Color.rgb(20,27,30))
            typeface=Typeface.MONOSPACE;textSize=13f;gravity=Gravity.TOP or Gravity.START
            setPadding(8,8,8,8);setSingleLine(false)
            inputType=InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_FLAG_MULTI_LINE or InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS
            imeOptions=EditorInfo.IME_FLAG_NO_EXTRACT_UI or EditorInfo.IME_FLAG_NO_ENTER_ACTION
            filters=arrayOf(InputFilter.LengthFilter(120000))
            setText(request.text)
            if(request.readOnly){keyListener=null;setTextIsSelectable(true)}
            addTextChangedListener(object:TextWatcher{
                override fun beforeTextChanged(s:CharSequence?,start:Int,count:Int,after:Int){}
                override fun onTextChanged(s:CharSequence?,start:Int,before:Int,count:Int){
                    if(transferInput===this@apply&&transferGeneration==request.generation)
                        GameEngine.transferTextChanged(request.generation,s?.toString()?:"")
                }
                override fun afterTextChanged(s:Editable?){}
            })
        }
        transferInput=input;root.addView(input);layoutTransfer(request)
        if(!request.readOnly)input.post{
            if(transferInput===input&&!isDestroyed){input.requestFocus();(getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager).showSoftInput(input,InputMethodManager.SHOW_IMPLICIT)}
        }
    }
    internal fun layoutTransfer(request:GameEngine.TransferRequest){
        if(request.generation!=transferGeneration)return
        transferInput?.layoutParams=FrameLayout.LayoutParams(request.width.roundToInt().coerceAtLeast(1),request.height.roundToInt().coerceAtLeast(1)).apply{
            leftMargin=request.x.roundToInt();topMargin=request.y.roundToInt()
        }
    }
    internal fun validationTransferText(value:String){
        if(BuildConfig.CHEAT&&transferInput?.keyListener!=null)transferInput?.setText(value)
    }
    internal fun closeTransfer(generation:Long){
        if(generation!=transferGeneration)return
        val input=transferInput?:return
        transferInput=null;transferGeneration=0L
        (getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager).hideSoftInputFromWindow(input.windowToken,0)
        root.removeView(input);surface.requestFocus()
    }
    internal fun showName(request:GameEngine.NameRequest){
        if(isFinishing||isDestroyed||nameDialog!=null)return
        val input=EditText(this).apply{setSingleLine(true);inputType=InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;setText(request.text);selectAll()}
        input.filters=arrayOf(InputFilter{source,start,end,dest,dstart,dend->
            val before=dest.subSequence(0,dstart).toString();val after=dest.subSequence(dend,dest.length).toString()
            var part=source.subSequence(start,end).toString()
            while((before+part+after).toByteArray(Charsets.UTF_8).size>request.maximum&&part.isNotEmpty())part=part.dropLast(Character.charCount(part.codePointBefore(part.length)))
            if(part==source.subSequence(start,end).toString())null else part
        })
        val dialog=AlertDialog.Builder(this).setTitle("Profile name").setView(input)
            .setPositiveButton("OK"){_,_->GameEngine.nameResult(request.generation,input.text.toString())}
            .setNegativeButton("Cancel"){_,_->GameEngine.nameResult(request.generation,null)}
            .setOnCancelListener{GameEngine.nameResult(request.generation,null)}.create()
        nameDialog=dialog;dialog.setOnDismissListener{nameDialog=null;if(Build.VERSION.SDK_INT>=30)window.insetsController?.hide(WindowInsets.Type.systemBars())};dialog.setOnShowListener{input.requestFocus();dialog.window?.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE)};dialog.show()
    }
}
