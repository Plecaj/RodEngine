using Rod.Internal;

namespace Rod;

public readonly struct Entity
{
    public readonly ulong ID;

    public Entity(ulong id)
    {
        ID = id;
    }

    public string Name
    {
        get => NativeApi.EntityGetName(ID);
        set => NativeApi.EntitySetName(ID, value);
    }

    public void SetName(string name)
    {
        NativeApi.EntitySetName(ID, name);
    }

    public static Entity Create(string name = "Entity")
    {
        return new Entity(NativeApi.EntityCreate(name));
    }

    public static Entity FindByName(string name)
    {
        return new Entity(NativeApi.EntityFindByName(name));
    }

    public bool HasComponent<T>() where T : Component, new()
    {
        return NativeApi.EntityHasComponent(ID, ComponentRegistry.GetComponentType<T>());
    }

    public T AddComponent<T>() where T : Component, new()
    {
        NativeApi.EntityAddComponent(ID, ComponentRegistry.GetComponentType<T>());
        return GetComponent<T>();
    }

    public T GetComponent<T>() where T : Component, new()
    {
        return new T { Entity = this };
    }

    public void RemoveComponent<T>() where T : Component, new()
    {
        NativeApi.EntityRemoveComponent(ID, ComponentRegistry.GetComponentType<T>());
    }

    public bool IsValid => ID != 0;
}
