#include "rdpch.h"
#include "Mesh.h"

namespace Rod {

	Ref<Mesh> Mesh::Create(const std::string& glbPath)
	{
		return std::make_shared<Mesh>(glbPath);
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

		bool ok = loader.LoadBinaryFromFile(&model, &err, &warn, path);
		if (!warn.empty()) RD_CORE_WARN(warn);
		if (!err.empty())  RD_CORE_ERROR(err);
		RD_CORE_ASSERT(ok, "Failed to load GLB");

		RD_CORE_ASSERT(!model.meshes.empty(), "Meshes are empty");

		m_Material = Material::Create();
		m_VAO = VertexArray::Create();

		const auto& mesh = model.meshes[0];
		for (const auto& primitive : mesh.primitives) {
			ProcessPrimitive(model, primitive);
			ProcessMaterial(model, primitive);
			break;
		}
	}

	void Mesh::ProcessPrimitive(const tinygltf::Model& model, const tinygltf::Primitive& primitive)
	{
		RD_PROFILE_FUNCTION();

		auto posIt = primitive.attributes.find("POSITION");
		RD_CORE_ASSERT(posIt != primitive.attributes.end(), "Couldnt find position in mesh primitive");

		const auto& posAccessor = model.accessors[posIt->second];
		const auto& posView = model.bufferViews[posAccessor.bufferView];
		const auto& posBuffer = model.buffers[posView.buffer];

		const float* positions = reinterpret_cast<const float*>(
			&posBuffer.data[posView.byteOffset + posAccessor.byteOffset]);

		size_t vertexCount = posAccessor.count;
		const float* normals = nullptr;

		auto normIt = primitive.attributes.find("NORMAL");
		if (normIt != primitive.attributes.end())
		{
			const auto& normAccessor = model.accessors[normIt->second];
			const auto& normView = model.bufferViews[normAccessor.bufferView];
			const auto& normBuffer = model.buffers[normView.buffer];

			normals = reinterpret_cast<const float*>(
				&normBuffer.data[normView.byteOffset + normAccessor.byteOffset]);
		}
		else
		{
			RD_CORE_WARN("Mesh primitive has no NORMAL attribute");
		}

		std::vector<float> vertexData;
		vertexData.reserve(vertexCount * 6);

		for (size_t i = 0; i < vertexCount; i++) {
			vertexData.push_back(positions[i * 3 + 0]);
			vertexData.push_back(positions[i * 3 + 1]);
			vertexData.push_back(positions[i * 3 + 2]);

			if (normals)
			{
				vertexData.push_back(normals[i * 3 + 0]);
				vertexData.push_back(normals[i * 3 + 1]);
				vertexData.push_back(normals[i * 3 + 2]);
			}
			else
			{
				vertexData.push_back(0.0f);
				vertexData.push_back(0.0f);
				vertexData.push_back(0.0f);
			}
		}

		auto vertexBuffer = VertexBuffer::Create(
			vertexData.data(),
			vertexData.size() * sizeof(float)
		);
		BufferLayout layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" }
		};
		vertexBuffer->SetBufferLayout(layout);

		RD_CORE_ASSERT(primitive.indices >= 0, "Mesh indicies < 0");

		const auto& idxAccessor = model.accessors[primitive.indices];
		const auto& idxView = model.bufferViews[idxAccessor.bufferView];
		const auto& idxBuffer = model.buffers[idxView.buffer];

		std::vector<uint32_t> indices;
		indices.reserve(idxAccessor.count);

		const uint8_t* data = &idxBuffer.data[idxView.byteOffset + idxAccessor.byteOffset];
		size_t stride = idxView.byteStride;
		if (stride == 0)
			stride = idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT ? 2 : 4;

		for (size_t i = 0; i < idxAccessor.count; i++)
		{
			const uint8_t* element = data + i * stride;

			switch (idxAccessor.componentType)
			{
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
				indices.push_back(*reinterpret_cast<const uint16_t*>(element));
				break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
				indices.push_back(*reinterpret_cast<const uint32_t*>(element));
				break;
			default:
				RD_CORE_ASSERT(false, "Unsupported index type");
			}
		}

		auto indexBuffer = IndexBuffer::Create(indices.data(), indices.size());
		m_VAO->AddVertexBuffer(vertexBuffer);
		m_VAO->SetIndexBuffer(indexBuffer);
	}

	void Mesh::ProcessMaterial(const tinygltf::Model& model, const tinygltf::Primitive& primitive)
	{
		if (primitive.material == -1)
			return;

		RD_CORE_ASSERT(primitive.material < static_cast<int>(model.materials.size()), "Invalid material index");
		const auto& gltfMaterial = model.materials[primitive.material];
		const auto& pbr = gltfMaterial.pbrMetallicRoughness;

		if (pbr.baseColorFactor.size() == 4)
		{
			m_Material->SetAlbedo(glm::vec4(
				static_cast<float>(pbr.baseColorFactor[0]),
				static_cast<float>(pbr.baseColorFactor[1]),
				static_cast<float>(pbr.baseColorFactor[2]),
				static_cast<float>(pbr.baseColorFactor[3])
			));
		}

		m_Material->SetRoughness(static_cast<float>(pbr.roughnessFactor));
		m_Material->SetMetallic(static_cast<float>(pbr.metallicFactor));

		if (gltfMaterial.emissiveFactor.size() == 3)
		{
			m_Material->SetEmissive(glm::vec3(
				static_cast<float>(gltfMaterial.emissiveFactor[0]),
				static_cast<float>(gltfMaterial.emissiveFactor[1]),
				static_cast<float>(gltfMaterial.emissiveFactor[2])
			));
		}
	}

}
