/* Music output only; effects use Emscripten's OpenAL implementation. */
window.MDAudio={context:null,buffers:new Map(),node:null,name:'',gain:1,active:true,
 async prepare(module){this.context=new AudioContext();for(const name of module.FS.readdir('/assets/Audio').filter(n=>n.endsWith('.mp3'))){const data=module.FS.readFile('/assets/Audio/'+name);this.buffers.set(name,await this.context.decodeAudioData(data.buffer.slice(data.byteOffset,data.byteOffset+data.byteLength)));}},
 music(name,gain){if(name===this.name&&this.node)return;this.stop();this.name=name;this.gain=gain;this.start();},
 stop(){if(this.node){this.node.stop();this.node.disconnect();this.node=null;}},
 start(){if(!this.active||!this.name||!this.buffers.has(this.name))return;const source=this.context.createBufferSource(),gain=this.context.createGain();source.buffer=this.buffers.get(this.name);source.loop=true;gain.gain=this.gain;source.connect(gain).connect(this.context.destination);source.start();this.node=source;},
 async setActive(active){this.active=active;if(active){await this.context.resume();if(!this.node)this.start();}else{this.stop();await this.context.suspend();}}
};
