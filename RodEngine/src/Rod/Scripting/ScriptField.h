#pragma once

#include <string>
#include <unordered_map>

namespace Rod {

	enum class ScriptFieldType
	{
		None = 0,
		Float,
		Double,
		Bool,
		Char,
		Byte,
		Short,
		Int,
		Long,
		UByte,
		UShort,
		UInt,
		ULong,
		Vector2,
		Vector3,
		Vector4,
		Entity,
		String
	};

	struct ScriptField
	{
		std::string Name;
		std::string TypeName;
		ScriptFieldType Type = ScriptFieldType::None;
	};

	struct ScriptFieldInstance
	{
		ScriptField Field;
		std::string Value;
	};

	using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;
}
