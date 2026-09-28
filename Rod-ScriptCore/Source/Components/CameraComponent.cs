using Rod.Internal;

namespace Rod;

public enum CameraProjectionType
{
    Perspective = 0,
    Orthographic = 1
}

public sealed class CameraComponent : Component
{
    public bool Primary
    {
        get => NativeApi.CameraGetPrimary(Entity.ID);
        set => NativeApi.CameraSetPrimary(Entity.ID, value);
    }

    public bool FixedAspectRatio
    {
        get => NativeApi.CameraGetFixedAspectRatio(Entity.ID);
        set => NativeApi.CameraSetFixedAspectRatio(Entity.ID, value);
    }

    public CameraProjectionType ProjectionType
    {
        get => (CameraProjectionType)NativeApi.CameraGetProjectionType(Entity.ID);
        set => NativeApi.CameraSetProjectionType(Entity.ID, (int)value);
    }

    public float PerspectiveVerticalFOV
    {
        get => NativeApi.CameraGetPerspectiveVerticalFOV(Entity.ID);
        set => NativeApi.CameraSetPerspectiveVerticalFOV(Entity.ID, value);
    }

    public float PerspectiveNearClip
    {
        get => NativeApi.CameraGetPerspectiveNearClip(Entity.ID);
        set => NativeApi.CameraSetPerspectiveNearClip(Entity.ID, value);
    }

    public float PerspectiveFarClip
    {
        get => NativeApi.CameraGetPerspectiveFarClip(Entity.ID);
        set => NativeApi.CameraSetPerspectiveFarClip(Entity.ID, value);
    }

    public float OrthographicSize
    {
        get => NativeApi.CameraGetOrthographicSize(Entity.ID);
        set => NativeApi.CameraSetOrthographicSize(Entity.ID, value);
    }

    public float OrthographicNearClip
    {
        get => NativeApi.CameraGetOrthographicNearClip(Entity.ID);
        set => NativeApi.CameraSetOrthographicNearClip(Entity.ID, value);
    }

    public float OrthographicFarClip
    {
        get => NativeApi.CameraGetOrthographicFarClip(Entity.ID);
        set => NativeApi.CameraSetOrthographicFarClip(Entity.ID, value);
    }
}
