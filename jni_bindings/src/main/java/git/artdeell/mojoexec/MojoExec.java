package git.artdeell.mojoexec;

public class MojoExec {
    public static native boolean prepareEgl(String eglPath, boolean useBypass, boolean useGles, int glesVersion);
    public static native void setDisplayParams(int width, int height, float hz);
    public static native void overrideVulkanDriver(boolean enable);
    public static native void overrideVulkanDriverPath(String vulkanLibrary);
    public static native void preloadVulkan();
    public static native void setNativeLibraryDir(String dir);
}
