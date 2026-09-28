using Rod.Internal;

namespace Rod;

public static class Input
{
    public static bool IsKeyPressed(int keycode)
    {
        return NativeApi.InputIsKeyPressed(keycode);
    }

    public static bool IsMouseButtonPressed(int button)
    {
        return NativeApi.InputIsMouseButtonPressed(button);
    }

    public static Vector2 MousePosition => NativeApi.InputGetMousePosition();
}
