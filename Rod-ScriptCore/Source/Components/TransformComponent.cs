using Rod.Internal;

namespace Rod;

public sealed class TransformComponent : Component
{
    public Vector3 Translation
    {
        get => NativeApi.TransformGetTranslation(Entity.ID);
        set => NativeApi.TransformSetTranslation(Entity.ID, value);
    }

    public Vector3 Rotation
    {
        get => NativeApi.TransformGetRotation(Entity.ID);
        set => NativeApi.TransformSetRotation(Entity.ID, value);
    }

    public Vector3 Scale
    {
        get => NativeApi.TransformGetScale(Entity.ID);
        set => NativeApi.TransformSetScale(Entity.ID, value);
    }
}
