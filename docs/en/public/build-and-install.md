# Build and installation

<!-- language-nav:start -->
[한국어](../../public/build-and-install.md) | [English](build-and-install.md)
<!-- language-nav:end -->

## Web / PWA

[Play or install MachDefense](https://gelb2.github.io/MachDefense/)

Open the address, finish downloading the resources, and select **PLAY IN BROWSER**. Installing the PWA gives it a separate app window and allows offline play.

- iPhone / iPad: open the address in Safari and choose Share → Add to Home Screen.
- Android: press **INSTALL GAME** in Chrome. If it says **HOW TO INSTALL**, choose Install app or Add to Home screen from the browser menu.
- Mac Chrome: **INSTALL GAME** opens the installation prompt. If it says **HOW TO INSTALL**, use Chrome menu → Cast, save, and share → Install page as app. If already installed, open the existing app icon.
- Mac Safari: choose File → Add to Dock. **HOW TO INSTALL** explains this route.

To play without installing, press **PLAY IN BROWSER**. Canceling the installation prompt does not prevent browser play.

Open the installed app and check that the download has finished. Progress is stored on the device. To move it elsewhere, use **GET CODE** on the slot screen and enter it through **ENTER CODE** on the other device. This replaces all 5 slots. Keep a code before deleting site data.

[PWA guide and shared recovery code](quick-play.md)

## Tauri Mac

[Download the Mac app](https://github.com/gelb2/MachDefense/releases/download/v0.6/MachDefense-Desktop-v0.6-arm64.zip). Extract it, move MachDefense Desktop.app to Applications, and launch it. It requires macOS 14 or later on Apple Silicon. Game resources are included for offline play from the first launch.

The app is ad-hoc signed and has not been notarized by Apple. If macOS blocks it, use System Settings → Privacy & Security → Open Anyway. Use the keyboard for text entry and the mouse or trackpad to play. Drag to move the battlefield; wheel and trackpad scrolling work in the production list. Keyboard battlefield movement is not implemented yet.

Use GET CODE and ENTER CODE on the slot screen to transfer progress between platforms. [Build, saves and updates](../../../Desktop/Tauri/README.en.md).

## iPhone / iPad

On an Apple Silicon Mac, install Xcode, iOS Simulator, Python 3, and CMake 3.20 or later. Homebrew users can run `brew install cmake`. The first build compiles the included OpenAL Soft source using CMake.

Open `Apple/MachDefense.xcworkspace`. Select the `MachDefense` scheme for the regular app or `MachDefenseCheat` for the cheat app. Choose an iPhone or iPad simulator and press Run. For a physical device, sign with your own development team. The minimum deployment target is iOS/iPadOS 15.0.

The workspace and project files are included. XcodeGen is only needed to regenerate them.

## Android

Install the regular or cheat APK from [Releases](https://github.com/gelb2/MachDefense/releases/latest), or open `Android/` in Android Studio. The build uses SDK 36, NDK 26.3.11579264, CMake 3.22.1, and JDK 17. Select `normalDebug` or `cheatDebug` and your connected device. The target is arm64 on Android 8 (API 26) or later.

## Build from source

```sh
git clone https://github.com/gelb2/MachDefense.git
cd MachDefense
```

The clone includes game code, images, audio, maps, the font atlas, and third-party library sources. These are regular Git files; there is no Git LFS download step. Build from the checked-out files without an original IPA or running `prepare_resources.py`. To build the web version, follow the [web development guide](../../../Web/README.en.md).

[Game overview](../../../README.en.md) · [Documentation](README.md)
