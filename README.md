# MojoExec

MojoExec is a small library intended for porting desktop games to Android. It provides a single source of the game window and driver parameters set from the JNI/Android app side.

## Features

- Loading custom Vulkan drivers including Turnip (e.g. AdrenoTools, `qgl_config.txt` override is also supported);
- Loading provided EGL wrappers or drivers (such as Mesa) instead of the system libEGL - the app must discover the EGL handle through mojoexec in this case;
- Bypassing Android linker namespace restrictions - this, for example, allows using unmodified Mesa3D built with `android_stub`;
- Managing window surface sizes;
- Managing CPU core affinity.

## Usage

To use the library you need to do two things: include `jni-bindings` project as a dependency of your app and link the native part of the library to your application's native side.  
You may want to take a look at [MojoSDL](https://github.com/MojoLauncher/MojoSDL) and [glfw](https://github.com/MojoLauncher/glfw) - both forks already use mojoexec.

For usage examples see [MojoLauncher](https://github.com/MojoLauncher/MojoLauncher).
