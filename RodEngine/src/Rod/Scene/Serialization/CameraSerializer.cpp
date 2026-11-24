#include "rdpch.h"

#include "ComponentSerializers.h"
#include "Rod/Scene/Components.h"

#include <yaml-cpp/yaml.h> // IWYU pragma: keep

namespace Rod::SceneSerialization {

	void CameraSerializer::Write(YAML::Emitter& out, const CameraComponent& cameraComponent)
	{
		const auto& camera = cameraComponent.Camera;

		out << YAML::BeginMap;
		out << YAML::Key << "Camera" << YAML::Value;
		out << YAML::BeginMap;
		out << YAML::Key << "ProjectionType" << YAML::Value << static_cast<int>(camera.GetProjectionType());
		out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
		out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
		out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
		out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
		out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
		out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
		out << YAML::EndMap;

		out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
		out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;
		out << YAML::EndMap;
	}

	void CameraSerializer::Read(const YAML::Node& node, CameraComponent& camera)
	{
		auto cameraProps = node["Camera"];

		camera.Camera.SetProjectionType(
		    static_cast<SceneCamera::ProjectionType>(cameraProps["ProjectionType"].as<int>()));
		camera.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
		camera.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
		camera.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());
		camera.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
		camera.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
		camera.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

		camera.Primary = node["Primary"].as<bool>();
		camera.FixedAspectRatio = node["FixedAspectRatio"].as<bool>();
	}
}
