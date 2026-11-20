#pragma once

#include "Rod/Core/Core.h"
#include "Rod/Core/UUID.h"
#include "Rod/Renderer/Material.h"
#include "Rod/Renderer/Mesh.h"
#include "Rod/Renderer/Texture.h"
#include "Rod/Scene/SceneCamera.h"
#include "Rod/Scripting/ScriptField.h"
#include "ScriptableEntity.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <utility>

namespace Rod {

	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		constexpr explicit IDComponent(UUID id) noexcept : ID(id) {}
	};

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		constexpr explicit TagComponent(std::string tag) noexcept(noexcept(std::string(std::move(tag))))
		    : Tag(std::move(tag))
		{}
	};

	struct TransformComponent
	{
		glm::vec3 Translation = glm::vec3(0.0f);
		glm::vec3 Rotation = glm::vec3(0.0f);
		glm::vec3 Scale = glm::vec3(1.0f);

		TransformComponent() = default;
		constexpr explicit TransformComponent(const glm::vec3& translation) noexcept : Translation(translation) {}

		[[nodiscard]] auto GetTransform() const noexcept -> glm::mat4
		{
			const glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), Rotation.x, {1, 0, 0}) *
			                           glm::rotate(glm::mat4(1.0f), Rotation.y, {0, 1, 0}) *
			                           glm::rotate(glm::mat4(1.0f), Rotation.z, {0, 0, 1});

			return glm::translate(glm::mat4(1.0f), Translation) * rotation * glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color = glm::vec4(1.0f);
		Ref<Texture2D> Texture;
		float TilingFactor = 1.0f;

		SpriteRendererComponent() = default;
		explicit SpriteRendererComponent(const glm::vec4& color) noexcept : Color(color) {}
	};

	struct MeshComponent
	{
		Ref<Rod::Mesh> Mesh;
		Rod::Material Material;

		void SetMesh(Ref<Rod::Mesh> mesh) noexcept
		{
			Mesh = std::move(mesh);
			Material = Mesh ? Mesh->GetMaterial() : Rod::Material{};
		}
	};

	struct DirectionalLightComponent
	{
		static constexpr glm::vec3 s_DefaultDirection{0.667f, -0.667f, -0.333f};
		glm::vec3 Direction = s_DefaultDirection;
		glm::vec3 Color = glm::vec3(1.0f);
		float Intensity = 1.0f;

		DirectionalLightComponent() = default;
		// NOLINTNEXTLINE(bugprone-easily-swappable-parameters): Direction and color use the same vector type.
		DirectionalLightComponent(const glm::vec3& direction, const glm::vec3& color, float intensity = 1.0f) noexcept
		    : Direction(direction), Color(color), Intensity(intensity)
		{}
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = true;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
	};

	struct NativeScriptComponent
	{
		Scope<ScriptableEntity> Instance;

		Scope<ScriptableEntity> (*InstantiateScript)() = nullptr;

		template <typename T>
		void Bind() noexcept
		{
			InstantiateScript = []() -> Scope<ScriptableEntity> {
				return CreateScope<T>();
			};
		}
	};

	struct ScriptComponent
	{
		std::string ClassName;
		ScriptFieldMap Fields;

		ScriptComponent() = default;
		explicit ScriptComponent(std::string className) : ClassName(std::move(className)) {}
	};

}
