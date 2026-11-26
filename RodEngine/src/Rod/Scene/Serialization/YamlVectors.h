#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <yaml-cpp/yaml.h> // IWYU pragma: keep

namespace YAML {

	template <>
	struct convert<glm::vec3>
	{
		static auto encode(const glm::vec3& rhs) -> Node
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static auto decode(const Node& node, glm::vec3& rhs) -> bool
		{
			if (!node.IsSequence() || node.size() != static_cast<size_t>(rhs.length()))
				return false;

			for (decltype(rhs.length()) component = 0; component < rhs.length(); ++component)
				rhs[component] = node[static_cast<size_t>(component)].as<float>();
			return true;
		}
	};

	template <>
	struct convert<glm::vec4>
	{
		static auto encode(const glm::vec4& rhs) -> Node
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static auto decode(const Node& node, glm::vec4& rhs) -> bool
		{
			if (!node.IsSequence() || node.size() != static_cast<size_t>(rhs.length()))
				return false;

			for (decltype(rhs.length()) component = 0; component < rhs.length(); ++component)
				rhs[component] = node[static_cast<size_t>(component)].as<float>();
			return true;
		}
	};

}

namespace Rod::SceneSerialization {

	inline auto operator<<(YAML::Emitter& out, const glm::vec3& v) -> YAML::Emitter&
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	inline auto operator<<(YAML::Emitter& out, const glm::vec4& v) -> YAML::Emitter&
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

}
