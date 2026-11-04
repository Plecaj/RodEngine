#include "rdpch.h"

#include "Mesh.h"

#include <cstring>
#include <limits>

namespace Rod {

	template <typename T>
	T Mesh::ReadScalar(const uint8_t* data)
	{
		T value{};
		std::memcpy(&value, data, sizeof(value));
		return value;
	}

	const uint8_t* Mesh::AccessorData(const tinygltf::Model& model, const tinygltf::Accessor& accessor)
	{
		const auto& view = model.bufferViews[static_cast<size_t>(accessor.bufferView)];
		const auto& buffer = model.buffers[static_cast<size_t>(view.buffer)];
		return buffer.data.data() + view.byteOffset + accessor.byteOffset;
	}

	std::vector<float> Mesh::ReadVertices(const tinygltf::Model& model, const tinygltf::Primitive& primitive)
	{
		const auto position = primitive.attributes.find("POSITION");
		RD_CORE_ASSERT(position != primitive.attributes.end(), "Could not find position in mesh primitive");
		if (position == primitive.attributes.end())
			return {};

		const auto& accessor = model.accessors[static_cast<size_t>(position->second)];
		const uint8_t* positions = AccessorData(model, accessor);
		const uint8_t* normals = nullptr;
		const auto normal = primitive.attributes.find("NORMAL");
		if (normal != primitive.attributes.end())
			normals = AccessorData(model, model.accessors[static_cast<size_t>(normal->second)]);
		else
			RD_CORE_WARN("Mesh primitive has no NORMAL attribute");

		std::vector<float> vertices;
		vertices.reserve(accessor.count * VERTEX_COMPONENT_COUNT);
		for (size_t vertex = 0; vertex < accessor.count; ++vertex)
		{
			for (size_t component = 0; component < VECTOR_COMPONENT_COUNT; ++component)
				vertices.push_back(
				    ReadScalar<float>(positions + (vertex * VECTOR_COMPONENT_COUNT + component) * sizeof(float)));
			for (size_t component = 0; component < VECTOR_COMPONENT_COUNT; ++component)
				vertices.push_back(
				    normals ? ReadScalar<float>(normals + (vertex * VECTOR_COMPONENT_COUNT + component) * sizeof(float))
				            : 0.0f);
		}
		return vertices;
	}

	std::vector<uint32_t> Mesh::ReadIndices(const tinygltf::Model& model, const tinygltf::Primitive& primitive)
	{
		RD_CORE_ASSERT(primitive.indices >= 0, "Mesh indices < 0");
		if (primitive.indices < 0)
			return {};

		const auto& accessor = model.accessors[static_cast<size_t>(primitive.indices)];
		if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT &&
		    accessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
		{
			RD_CORE_ASSERT(false, "Unsupported index type");
			return {};
		}
		const auto& view = model.bufferViews[static_cast<size_t>(accessor.bufferView)];
		const uint8_t* data = AccessorData(model, accessor);
		size_t stride = view.byteStride;
		if (stride == 0)
			stride =
			    accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT ? sizeof(uint16_t) : sizeof(uint32_t);

		std::vector<uint32_t> indices;
		indices.reserve(accessor.count);
		for (size_t index = 0; index < accessor.count; ++index)
		{
			const uint8_t* element = data + index * stride;
			if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
				indices.push_back(ReadScalar<uint16_t>(element));
			else
				indices.push_back(ReadScalar<uint32_t>(element));
		}
		return indices;
	}

	template <typename Vector>
	Vector Mesh::ToFloatVector(const std::vector<double>& values)
	{
		Vector result(0.0f);
		for (typename Vector::length_type component = 0; component < result.length(); ++component)
			result[component] = static_cast<float>(values[static_cast<size_t>(component)]);
		return result;
	}

	Mesh::Mesh(const std::string& glbPath)
	{
		LoadGLB(glbPath);
	}

	void Mesh::LoadGLB(const std::string& path)
	{
		RD_PROFILE_FUNCTION();
		m_SourcePath = path;

		tinygltf::TinyGLTF loader;
		tinygltf::Model model;
		std::string err, warn;

		const bool loaded = loader.LoadBinaryFromFile(&model, &err, &warn, path);
		if (!warn.empty())
			RD_CORE_WARN(warn);
		if (!err.empty())
			RD_CORE_ERROR(err);
		RD_CORE_ASSERT(loaded, "Failed to load GLB");
		if (!loaded)
			return;

		RD_CORE_ASSERT(!model.meshes.empty(), "Meshes are empty");
		if (model.meshes.empty())
			return;

		m_VAO = VertexArray::Create();

		const auto& mesh = model.meshes[0];
		RD_CORE_ASSERT(!mesh.primitives.empty(), "Mesh has no primitives");
		if (mesh.primitives.empty())
			return;

		const auto& primitive = mesh.primitives.front();
		ProcessPrimitive(model, primitive);
		ProcessMaterial(model, primitive);
	}

	void Mesh::ProcessPrimitive(const tinygltf::Model& model, const tinygltf::Primitive& primitive)
	{
		RD_PROFILE_FUNCTION();

		auto vertexData = ReadVertices(model, primitive);
		if (vertexData.empty())
			return;

		const size_t vertexBufferSize = vertexData.size() * sizeof(float);
		const bool vertexBufferFits = vertexBufferSize <= std::numeric_limits<uint32_t>::max();
		RD_CORE_ASSERT(vertexBufferFits, "Mesh vertex buffer is too large");
		if (!vertexBufferFits)
		{
			RD_CORE_ERROR("Mesh vertex buffer is too large: {0} bytes.", vertexBufferSize);
			return;
		}

		auto vertexBuffer = VertexBuffer::Create(vertexData.data(), static_cast<uint32_t>(vertexBufferSize));
		const BufferLayout layout = {{ShaderDataType::Float3, "a_Position"}, {ShaderDataType::Float3, "a_Normal"}};
		vertexBuffer->SetBufferLayout(layout);

		auto indices = ReadIndices(model, primitive);
		if (indices.empty())
			return;

		const bool indexBufferFits = indices.size() <= std::numeric_limits<uint32_t>::max();
		RD_CORE_ASSERT(indexBufferFits, "Mesh index buffer is too large");
		if (!indexBufferFits)
		{
			RD_CORE_ERROR("Mesh index buffer is too large: {0} indices.", indices.size());
			return;
		}

		auto indexBuffer = IndexBuffer::Create(indices.data(), static_cast<uint32_t>(indices.size()));
		m_VAO->AddVertexBuffer(std::move(vertexBuffer));
		m_VAO->SetIndexBuffer(std::move(indexBuffer));
	}

	void Mesh::ProcessMaterial(const tinygltf::Model& model, const tinygltf::Primitive& primitive)
	{
		if (primitive.material == -1)
			return;

		RD_CORE_ASSERT(std::cmp_less(primitive.material, model.materials.size()), "Invalid material index");
		const auto& gltfMaterial = model.materials[static_cast<size_t>(primitive.material)];
		const auto& pbr = gltfMaterial.pbrMetallicRoughness;

		if (pbr.baseColorFactor.size() == COLOR_COMPONENT_COUNT)
			m_Material.SetAlbedo(ToFloatVector<glm::vec4>(pbr.baseColorFactor));

		m_Material.SetRoughness(static_cast<float>(pbr.roughnessFactor));
		m_Material.SetMetallic(static_cast<float>(pbr.metallicFactor));

		if (gltfMaterial.emissiveFactor.size() == VECTOR_COMPONENT_COUNT)
			m_Material.SetEmissive(ToFloatVector<glm::vec3>(gltfMaterial.emissiveFactor));
	}

}
