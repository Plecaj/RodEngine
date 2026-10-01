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
    private static readonly Dictionary<Type, ComponentType> s_ComponentTypes = new()
    {
        [typeof(TagComponent)] = ComponentType.Tag,
        [typeof(TransformComponent)] = ComponentType.Transform,
        [typeof(CameraComponent)] = ComponentType.Camera,
        [typeof(SpriteRendererComponent)] = ComponentType.SpriteRenderer,
        [typeof(MeshComponent)] = ComponentType.Mesh,
        [typeof(DirectionalLightComponent)] = ComponentType.DirectionalLight,
        [typeof(ScriptComponent)] = ComponentType.Script
    };

    public static int GetComponentType<T>() where T : Component
    {
        Type type = typeof(T);
        if (s_ComponentTypes.TryGetValue(type, out ComponentType componentType))
            return (int)componentType;

        throw new NotSupportedException($"Component '{type.FullName}' is not exposed to Rod scripts.");
    }
}
