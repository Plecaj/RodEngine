using Rod.Internal;

namespace Rod;

public static class Log
{
    public static void Trace(string message) => NativeApi.LogMessage(0, message);
    public static void Info(string message) => NativeApi.LogMessage(1, message);
    public static void Warn(string message) => NativeApi.LogMessage(2, message);
    public static void Error(string message) => NativeApi.LogMessage(3, message);
}
