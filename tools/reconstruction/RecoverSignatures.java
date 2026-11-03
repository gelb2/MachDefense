// Local Ghidra analysis only; exports repaired decompiler views, never patches bytes.
// The ARMv7 ObjC float argument storage inferred for these methods overlaps.
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.data.*;
import ghidra.program.model.symbol.SourceType;
import java.nio.file.*;
import java.nio.charset.StandardCharsets;

public class RecoverSignatures extends GhidraScript {
    public void run() throws Exception {
        Path out=Paths.get(getScriptArgs()[0]);Files.createDirectories(out);
        long[] addresses={0x8bbc,0x41dc4};int[] counts={8,6};
        if(getScriptArgs().length>1) {
            addresses=new long[getScriptArgs().length-1];counts=new int[addresses.length];
            for(int i=0;i<addresses.length;i++) {
                String[] spec=getScriptArgs()[i+1].split(":");
                addresses[i]=Long.decode(spec[0]);counts[i]=Integer.parseInt(spec[1]);
            }
        }
        DecompInterface decompiler=new DecompInterface();decompiler.openProgram(currentProgram);
        for(int i=0;i<addresses.length;i++) {
            Function function=getFunctionAt(toAddr(addresses[i]));
            if(function==null) throw new IllegalStateException("Function missing");
            String before=function.getPrototypeString(false,true);
            for(Variable variable:function.getLocalVariables()) function.removeVariable(variable);
            Parameter[] parameters=new Parameter[counts[i]];
            for(int p=0;p<parameters.length;p++) parameters[p]=new ParameterImpl("word"+p,UnsignedIntegerDataType.dataType,currentProgram);
            function.updateFunction(currentProgram.getCompilerSpec().getDefaultCallingConvention().getName(),
                new ReturnParameterImpl(VoidDataType.dataType,currentProgram),java.util.Arrays.asList(parameters),
                Function.FunctionUpdateType.DYNAMIC_STORAGE_ALL_PARAMS,true,SourceType.USER_DEFINED);
            decompiler.flushCache();
            DecompileResults result=decompiler.decompileFunction(function,120,monitor);
            String text="// Before: "+before+"\n// ABI-word signature: "+function.getPrototypeString(false,true)+"\n";
            if(result.decompileCompleted()) text+=result.getDecompiledFunction().getC();
            else text+="// ERROR: "+result.getErrorMessage();
            Files.writeString(out.resolve("0x"+Long.toHexString(addresses[i])+".abi-words.c"),text,StandardCharsets.UTF_8);
            println("Recovered "+function.getName()+": "+result.decompileCompleted());
        }
        decompiler.dispose();
    }
}
