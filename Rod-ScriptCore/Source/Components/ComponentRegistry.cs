namespace Rod;

internal enum ComponentType
{
    ID = 0,
    Tag = 1,
    Transform = 2,
    Camera = 3,
    SpriteRenderer = 4,
    Mesh = 5,
    DirectionalLight = 6,
    Script = 7
}

internal static class ComponentRegistry
{
    public static int GetComponentType<T>() where T : Component
    {
        Type type = typeof(T);

        if (type == typeof(TagComponent)) return (int)ComponentType.Tag;
        if (type == typeof(TransformComponent)) return (int)ComponentType.Transform;
        if (type == typeof(CameraComponent)) return (int)ComponentType.Camera;
        if (type == typeof(SpriteRendererComponent)) return (int)ComponentType.SpriteRenderer;
        if (type == typeof(MeshComponent)) return (int)ComponentType.Mesh;
        if (type == typeof(DirectionalLightComponent)) return (int)ComponentType.DirectionalLight;
        if (type == typeof(ScriptComponent)) return (int)ComponentType.Script;

        throw new NotSupportedException($"Component '{type.FullName}' is not exposed to Rod scripts.");
    }
}
