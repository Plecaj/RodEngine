#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace YAML {
	class Emitter;
	class Node;
}

namespace Rod {
	class RenderDevice;
	struct TagComponent;
	struct TransformComponent;
	struct CameraComponent;
	struct SpriteRendererComponent;
	struct MeshComponent;
	struct DirectionalLightComponent;
	struct ScriptComponent;
}

namespace Rod::SceneSerialization {
	class AssetPaths
	{
	public:
		explicit AssetPaths(std::filesystem::path projectDirectory = {});
		[[nodiscard]] auto ForSave(const std::string& path) const -> std::string;
		[[nodiscard]] auto ForLoad(const std::string& path) const -> std::string;
		[[nodiscard]] static auto ReadReference(const YAML::Node& node) -> std::optional<std::string>;

	private:
		std::filesystem::path m_ProjectDirectory;
	};

	struct TagSerializer
	{
		using Component = TagComponent;
		static constexpr auto s_Key = "TagComponent";

		static void Write(YAML::Emitter& out, const Component& tag);
		static void Read(const YAML::Node& node, Component& tag);
	};

	struct TransformSerializer
	{
		using Component = TransformComponent;
		static constexpr auto s_Key = "TransformComponent";

		static void Write(YAML::Emitter& out, const Component& transform);
		static void Read(const YAML::Node& node, Component& transform);
	};

	struct CameraSerializer
	{
		using Component = CameraComponent;
		static constexpr auto s_Key = "CameraComponent";

		static void Write(YAML::Emitter& out, const Component& cameraComponent);
		static void Read(const YAML::Node& node, Component& camera);
	};

	struct SpriteRendererSerializer
	{
		using Component = SpriteRendererComponent;
		static constexpr auto s_Key = "SpriteRendererComponent";
		static constexpr auto s_AssetKey = "Texture";
		const AssetPaths& Paths;

		void Write(YAML::Emitter& out, const Component& sprite) const;
		void Read(const YAML::Node& node, Component& sprite, const RenderDevice& device) const;
		[[nodiscard]] static auto ReadAssetPath(const YAML::Node& node) -> std::optional<std::string>;
	};

	struct MeshSerializer
	{
		using Component = MeshComponent;
		static constexpr auto s_Key = "MeshComponent";
		static constexpr auto s_AssetKey = "Path";
		const AssetPaths& Paths;

		void Write(YAML::Emitter& out, const Component& meshComponent) const;
		void Read(const YAML::Node& node, Component& mesh, const RenderDevice& device) const;
		[[nodiscard]] static auto ReadAssetPath(const YAML::Node& node) -> std::optional<std::string>;
	};

	struct DirectionalLightSerializer
	{
		using Component = DirectionalLightComponent;
		static constexpr auto s_Key = "DirectionalLightComponent";

		static void Write(YAML::Emitter& out, const Component& light);
		static void Read(const YAML::Node& node, Component& light);
	};

	struct ScriptSerializer
	{
		using Component = ScriptComponent;
		static constexpr auto s_Key = "ScriptComponent";

		static void Write(YAML::Emitter& out, const Component& script);
		static void Read(const YAML::Node& node, Component& script);
	};
}
