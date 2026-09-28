using System.Reflection;
using System.Globalization;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using System.Text.Json;

namespace Rod.Internal;

[StructLayout(LayoutKind.Sequential)]
public readonly struct NativeCall
{
    public readonly IntPtr Name;
    public readonly IntPtr Function;
}

public static unsafe class ScriptHost
{
    private static AssemblyLoadContext? s_LoadContext;
    private static AssemblyDependencyResolver? s_AssemblyResolver;
    private static Assembly? s_GameAssembly;
    private static readonly Dictionary<ulong, ScriptBehaviour> s_Instances = new();

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    public static int Initialize(NativeCall* calls, int count)
    {
        for (int i = 0; i < count; i++)
        {
            string? name = Marshal.PtrToStringUTF8(calls[i].Name);
            if (!string.IsNullOrEmpty(name))
                NativeApi.Register(name, calls[i].Function);
        }

        return 0;
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    public static void Shutdown()
    {
        UnloadAssemblyInternal();
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    public static IntPtr LoadAssembly(byte* assemblyPath)
    {
        string path = Marshal.PtrToStringUTF8((IntPtr)assemblyPath) ?? string.Empty;

        UnloadAssemblyInternal();
        s_LoadContext = new AssemblyLoadContext("RodGameScripts", isCollectible: true);
        s_AssemblyResolver = new AssemblyDependencyResolver(path);
        s_LoadContext.Resolving += ResolveScriptAssembly;
        s_GameAssembly = s_LoadContext.LoadFromAssemblyPath(path);

        return Marshal.StringToCoTaskMemUTF8(BuildMetadataJson());
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    public static void UnloadAssembly()
    {
        UnloadAssemblyInternal();
    }

    private static void UnloadAssemblyInternal()
    {
        foreach (ScriptBehaviour instance in s_Instances.Values)
            instance.InvokeOnDestroy();

        s_Instances.Clear();
        s_GameAssembly = null;
        s_AssemblyResolver = null;

        if (s_LoadContext != null)
        {
            s_LoadContext.Resolving -= ResolveScriptAssembly;
            s_LoadContext.Unload();
            s_LoadContext = null;
        }
    }

    private static Assembly? ResolveScriptAssembly(AssemblyLoadContext context, AssemblyName assemblyName)
    {
        Assembly scriptCoreAssembly = typeof(ScriptBehaviour).Assembly;
        if (AssemblyName.ReferenceMatchesDefinition(scriptCoreAssembly.GetName(), assemblyName))
            return scriptCoreAssembly;

        string? assemblyPath = s_AssemblyResolver?.ResolveAssemblyToPath(assemblyName);
        if (assemblyPath != null)
            return context.LoadFromAssemblyPath(assemblyPath);

        return null;
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    public static int CreateScript(ulong entityID, byte* classNamePtr)
    {
        if (s_GameAssembly == null)
            return 0;

        string className = Marshal.PtrToStringUTF8((IntPtr)classNamePtr) ?? string.Empty;
        Type? type = s_GameAssembly.GetType(className);
        if (type == null || !typeof(ScriptBehaviour).IsAssignableFrom(type))
            return 0;

        if (Activator.CreateInstance(type) is not ScriptBehaviour instance)
            return 0;

        instance.Entity = new Entity(entityID);
        s_Instances[entityID] = instance;
        return 1;
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    public static void StartScript(ulong entityID)
    {
        if (s_Instances.TryGetValue(entityID, out ScriptBehaviour? instance))
            instance.InvokeOnCreate();
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    public static void DestroyScript(ulong entityID)
    {
        if (!s_Instances.TryGetValue(entityID, out ScriptBehaviour? instance))
            return;

        instance.InvokeOnDestroy();
        s_Instances.Remove(entityID);
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    public static void UpdateScript(ulong entityID, float timestep)
    {
        if (s_Instances.TryGetValue(entityID, out ScriptBehaviour? instance))
            instance.InvokeOnUpdate(timestep);
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    public static void SetFieldValue(ulong entityID, byte* fieldNamePtr, int fieldType, byte* valuePtr)
    {
        if (!s_Instances.TryGetValue(entityID, out ScriptBehaviour? instance))
            return;

        string fieldName = Marshal.PtrToStringUTF8((IntPtr)fieldNamePtr) ?? string.Empty;
        string value = Marshal.PtrToStringUTF8((IntPtr)valuePtr) ?? string.Empty;
        FieldInfo? field = instance.GetType().GetField(fieldName, BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
        if (field == null)
            return;

        object? convertedValue = ConvertStringToFieldValue(field.FieldType, value);
        if (convertedValue != null)
            field.SetValue(instance, convertedValue);
    }

    private static string BuildMetadataJson()
    {
        if (s_GameAssembly == null)
            return "{\"classes\":[]}";

        var classes = GetLoadableTypes(s_GameAssembly)
            .Where(type => type.IsClass && !type.IsAbstract && typeof(ScriptBehaviour).IsAssignableFrom(type))
            .Select(type => new
            {
                fullName = type.FullName ?? type.Name,
                @namespace = type.Namespace ?? string.Empty,
                name = type.Name,
                fields = type.GetFields(BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic)
                    .Where(IsSerializableField)
                    .Select(field => new
                    {
                        name = field.Name,
                        type = GetFieldTypeName(field.FieldType)
                    })
            });

        return JsonSerializer.Serialize(new { classes });
    }

    private static IEnumerable<Type> GetLoadableTypes(Assembly assembly)
    {
        try
        {
            return assembly.GetTypes();
        }
        catch (ReflectionTypeLoadException e)
        {
            return e.Types.Where(type => type != null)!;
        }
    }

    private static bool IsSerializableField(FieldInfo field)
    {
        if (field.IsStatic || field.IsInitOnly)
            return false;
        if (field.GetCustomAttribute<HideInInspectorAttribute>() != null)
            return false;
        return field.IsPublic || field.GetCustomAttribute<SerializeFieldAttribute>() != null;
    }

    private static string GetFieldTypeName(Type type)
    {
        if (type == typeof(float)) return "Single";
        if (type == typeof(double)) return "Double";
        if (type == typeof(bool)) return "Boolean";
        if (type == typeof(char)) return "Char";
        if (type == typeof(byte)) return "Byte";
        if (type == typeof(short)) return "Int16";
        if (type == typeof(int)) return "Int32";
        if (type == typeof(long)) return "Int64";
        if (type == typeof(sbyte)) return "SByte";
        if (type == typeof(ushort)) return "UInt16";
        if (type == typeof(uint)) return "UInt32";
        if (type == typeof(ulong)) return "UInt64";
        if (type == typeof(Vector2)) return "Vector2";
        if (type == typeof(Vector3)) return "Vector3";
        if (type == typeof(Vector4)) return "Vector4";
        if (type == typeof(Entity)) return "Entity";
        if (type == typeof(string)) return "String";
        return type.Name;
    }

    private static object? ConvertStringToFieldValue(Type type, string value)
    {
        try
        {
            if (type == typeof(float)) return float.Parse(value, CultureInfo.InvariantCulture);
            if (type == typeof(double)) return double.Parse(value, CultureInfo.InvariantCulture);
            if (type == typeof(bool)) return bool.Parse(value);
            if (type == typeof(char)) return string.IsNullOrEmpty(value) ? '\0' : value[0];
            if (type == typeof(byte)) return byte.Parse(value, CultureInfo.InvariantCulture);
            if (type == typeof(short)) return short.Parse(value, CultureInfo.InvariantCulture);
            if (type == typeof(int)) return int.Parse(value, CultureInfo.InvariantCulture);
            if (type == typeof(long)) return long.Parse(value, CultureInfo.InvariantCulture);
            if (type == typeof(sbyte)) return sbyte.Parse(value, CultureInfo.InvariantCulture);
            if (type == typeof(ushort)) return ushort.Parse(value, CultureInfo.InvariantCulture);
            if (type == typeof(uint)) return uint.Parse(value, CultureInfo.InvariantCulture);
            if (type == typeof(ulong)) return ulong.Parse(value, CultureInfo.InvariantCulture);
            if (type == typeof(string)) return value;
            if (type == typeof(Entity)) return new Entity(ulong.Parse(value, CultureInfo.InvariantCulture));
            if (type == typeof(Vector2)) return ParseVector2(value);
            if (type == typeof(Vector3)) return ParseVector3(value);
            if (type == typeof(Vector4)) return ParseVector4(value);
        }
        catch
        {
            return null;
        }

        return null;
    }

    private static Vector2 ParseVector2(string value)
    {
        string[] parts = value.Split(',');
        return new Vector2(float.Parse(parts[0], CultureInfo.InvariantCulture), float.Parse(parts[1], CultureInfo.InvariantCulture));
    }

    private static Vector3 ParseVector3(string value)
    {
        string[] parts = value.Split(',');
        return new Vector3(float.Parse(parts[0], CultureInfo.InvariantCulture), float.Parse(parts[1], CultureInfo.InvariantCulture), float.Parse(parts[2], CultureInfo.InvariantCulture));
    }

    private static Vector4 ParseVector4(string value)
    {
        string[] parts = value.Split(',');
        return new Vector4(float.Parse(parts[0], CultureInfo.InvariantCulture), float.Parse(parts[1], CultureInfo.InvariantCulture), float.Parse(parts[2], CultureInfo.InvariantCulture), float.Parse(parts[3], CultureInfo.InvariantCulture));
    }
}
