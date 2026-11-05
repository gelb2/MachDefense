#!/usr/bin/env python3
"""Build the port's preserved OpenAL Soft; no original game executable is used."""
import argparse,fcntl,os,shutil,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--sdk',choices=['iphonesimulator','iphoneos','macosx'],default='iphonesimulator');a=p.parse_args()
cmake=shutil.which('cmake') or '/opt/homebrew/bin/cmake'
out=root/'.local/openal'/a.sdk;out.mkdir(parents=True,exist_ok=True)
with (out/'build.lock').open('w') as lock:
 fcntl.flock(lock,fcntl.LOCK_EX)
 env=os.environ.copy()
 # Xcode's SDKROOT otherwise leaks into CMake's compiler detection.
 env.pop('SDKROOT',None)
 subprocess.run([cmake,'-S',str(root/'Modules/ThirdParty/openal-soft'),'-B',str(out),
  '-DCMAKE_POLICY_VERSION_MINIMUM=3.5','-DCMAKE_SYSTEM_NAME='+('Darwin' if a.sdk=='macosx' else 'iOS'),
  '-DCMAKE_OSX_SYSROOT='+a.sdk,'-DCMAKE_OSX_ARCHITECTURES=arm64',
  '-DCMAKE_OSX_DEPLOYMENT_TARGET='+('12.0' if a.sdk=='macosx' else '15.0'),'-DCMAKE_BUILD_TYPE=Release',
  '-DLIBTYPE=STATIC','-DALSOFT_UTILS=OFF','-DALSOFT_EXAMPLES=OFF','-DALSOFT_NO_CONFIG_UTIL=ON',
  '-DALSOFT_BACKEND_WAVE=OFF','-DALSOFT_REQUIRE_COREAUDIO=ON','-DALSOFT_INSTALL=OFF'],check=True,env=env)
 subprocess.run([cmake,'--build',str(out),'--target','OpenAL','--parallel','4'],check=True,env=env)
print(out/'libopenal.a')
