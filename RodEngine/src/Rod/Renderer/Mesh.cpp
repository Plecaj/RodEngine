#include "rdpch.h"

#include "Mesh.h"

#include "RenderDevice.h"

#include <cstring>
#include <limits>
#include <stdexcept>
#include <tiny_gltf.h>
#include <vector>

namespace Rod {

	namespace {
		constexpr size_t g_VectorComponentCount = 3;
		constexpr size_t g_ColorComponentCount = 4;
		constexpr size_t g_VertexComponentCount = g_VectorComponentCount * 2;

		template <typename T>
		[[nodiscard]] auto ReadScalar(std::span<const uint8_t> data, size_t offset) noexcept -> T
		{
			T value{};
			std::memcpy(&value, data.subspan(offset, sizeof(value)).data(), sizeof(value));
			return value;
		}

		[[nodiscard]] auto
		AccessorData(const tinygltf::Model& model, const tinygltf::Accessor& accessor, size_t elementSize)
		    -> std::span<const uint8_t>
		{
			if (accessor.sparse.isSparse || accessor.bufferView < 0 ||
			    !std::cmp_less(accessor.bufferView, model.bufferViews.size()))
				throw std::runtime_error("Unsupported mesh accessor");
			const auto& view = model.bufferViews[static_cast<size_t>(accessor.bufferView)];
			if (view.buffer < 0 || !std::cmp_less(view.buffer, model.buffers.size()))
				throw std::runtime_error("Mesh buffer missing");
			const auto& buffer = model.buffers[static_cast<size_t>(view.buffer)];
			if (view.byteOffset > buffer.data.size() || view.byteLength > buffer.data.size() - view.byteOffset ||
			    accessor.byteOffset > view.byteLength)
				throw std::runtime_error("Mesh buffer bounds invalid");
			const size_t available = view.byteLength - accessor.byteOffset;
			const size_t stride = view.byteStride ? view.byteStride : elementSize;
			if (accessor.count == 0 || stride < elementSize || available < elementSize ||
			    accessor.count - 1 > (available - elementSize) / stride)
				throw std::runtime_error("Mesh accessor bounds invalid");
			return std::span{buffer.data}.subspan(view.byteOffset + accessor.byteOffset, available);
		}

		[[nodiscard]] auto ReadVertices(const tinygltf::Model& model, const tinygltf::Primitive& primitive)
		    -> std::vector<float>
		{
			const auto position = primitive.attributes.find("POSITION");
			if (position == primitive.attributes.end() || position->second < 0 ||
			    !std::cmp_less(position->second, model.accessors.size()))
				throw std::runtime_error("Mesh POSITION missing");

			const auto& accessor = model.accessors[static_cast<size_t>(position->second)];
			if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || accessor.type != TINYGLTF_TYPE_VEC3)
				throw std::runtime_error("Unsupported POSITION format");
			if (accessor.count > std::numeric_limits<uint32_t>::max() / (g_VertexComponentCount * sizeof(float)))
				throw std::runtime_error("Mesh vertex buffer too large");
			const auto positions = AccessorData(model, accessor, g_VectorComponentCount * sizeof(float));
			const size_t packedStride = g_VectorComponentCount * sizeof(float);
			const auto positionByteStride = model.bufferViews[static_cast<size_t>(accessor.bufferView)].byteStride;
			const size_t positionStride = positionByteStride ? positionByteStride : packedStride;
			std::span<const uint8_t> normals;
			size_t normalStride = 0;
			const auto normal = primitive.attributes.find("NORMAL");
			if (normal != primitive.attributes.end())
			{
				if (normal->second < 0 || !std::cmp_less(normal->second, model.accessors.size()))
					throw std::runtime_error("Mesh NORMAL accessor invalid");
				const auto& normalAccessor = model.accessors[static_cast<size_t>(normal->second)];
				if (normalAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT ||
				    normalAccessor.type != TINYGLTF_TYPE_VEC3 || normalAccessor.count != accessor.count)
					throw std::runtime_error("Unsupported NORMAL format");
				normals = AccessorData(model, normalAccessor, g_VectorComponentCount * sizeof(float));
				const auto byteStride = model.bufferViews[static_cast<size_t>(normalAccessor.bufferView)].byteStride;
				normalStride = byteStride ? byteStride : packedStride;
			}

			std::vector<float> vertices;
			vertices.reserve(accessor.count * g_VertexComponentCount);
			for (size_t vertex = 0; vertex < accessor.count; ++vertex)
			{
				for (size_t component = 0; component < g_VectorComponentCount; ++component)
					vertices.push_back(
					    ReadScalar<float>(positions, vertex * positionStride + component * sizeof(float)));
				for (size_t component = 0; component < g_VectorComponentCount; ++component)
					vertices.push_back(
					    !normals.empty() ? ReadScalar<float>(normals, vertex * normalStride + component * sizeof(float))
						                 : 0.0f);
			}
			return vertices;
		}

		[[nodiscard]] auto ReadIndices(const tinygltf::Model& model, const tinygltf::Primitive& primitive)
		    -> std::vector<uint32_t>
		{
			if (primitive.indices < 0 || !std::cmp_less(primitive.indices, model.accessors.size()))
				throw std::runtime_error("Mesh indices missing");

			const auto& accessor = model.accessors[static_cast<size_t>(primitive.indices)];
			if (accessor.type != TINYGLTF_TYPE_SCALAR ||
			    (accessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT &&
			     accessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT))
				throw std::runtime_error("Unsupported mesh index format");
			if (accessor.count > std::numeric_limits<uint32_t>::max() / sizeof(uint32_t))
				throw std::runtime_error("Mesh index buffer too large");
			const size_t elementSize =
			    accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT ? sizeof(uint16_t) : sizeof(uint32_t);
			const auto data = AccessorData(model, accessor, elementSize);
			const auto& view = model.bufferViews[static_cast<size_t>(accessor.bufferView)];
			const size_t stride = view.byteStride ? view.byteStride : elementSize;

			std::vector<uint32_t> indices;
			indices.reserve(accessor.count);
			for (size_t index = 0; index < accessor.count; ++index)
			{
				const size_t offset = index * stride;
				if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
					indices.push_back(ReadScalar<uint16_t>(data, offset));
				else
					indices.push_back(ReadScalar<uint32_t>(data, offset));
			}
			return indices;
		}

		template <typename Vector>
		[[nodiscard]] auto ToFloatVector(std::span<const double> values) -> Vector
		{
			Vector result(0.0f);
			for (typename Vector::length_type component = 0; component < result.length(); ++component)
				result[component] = static_cast<float>(values[static_cast<size_t>(component)]);
			return result;
		}
	}

	Mesh::Mesh(const RenderDevice& device, const std::string& path)
	try : m_SourcePath(path)
	{
		tinygltf::TinyGLTF loader;
		tinygltf::Model model;
		std::string err, warn;

		const bool loaded = loader.LoadBinaryFromFile(&model, &err, &warn, path);
		if (!loaded)
			throw std::runtime_error(err.empty() ? "Invalid GLB" : err);

		if (model.meshes.empty())
			throw std::runtime_error("Meshes missing");

		const auto& mesh = model.meshes[0];
		if (mesh.primitives.empty())
			throw std::runtime_error("Mesh primitives missing");

		const auto& primitive = mesh.primitives.front();
		if (primitive.mode != TINYGLTF_MODE_TRIANGLES && primitive.mode != -1)
			throw std::runtime_error("Unsupported primitive mode");
		if (primitive.material < -1 ||
		    (primitive.material >= 0 && !std::cmp_less(primitive.material, model.materials.size())))
			throw std::runtime_error("Material index invalid");
		auto vertices = ReadVertices(model, primitive);
		auto indices = ReadIndices(model, primitive);
		for (const auto index : indices)
		{
			if (index >= vertices.size() / g_VertexComponentCount)
				throw std::runtime_error("Vertex index invalid");
		}

		ProcessMaterial(model, primitive);
		Upload(device, vertices, indices);
	}
	catch (const std::runtime_error& exception)
	{
		throw std::runtime_error("Mesh load failed '" + path + "': " + exception.what());
	}

	void Mesh::Upload(const RenderDevice& device, std::span<const float> vertices, std::span<const uint32_t> indices)
	{
		m_VAO = device.CreateVertexArray();
		auto vertexBuffer = device.CreateVertexBuffer(std::as_bytes(vertices));
		const BufferLayout layout = {{ShaderDataType::Float3, "a_Position"}, {ShaderDataType::Float3, "a_Normal"}};
		vertexBuffer->SetBufferLayout(layout);

		auto indexBuffer = device.CreateIndexBuffer(indices);
		m_VAO->AddVertexBuffer(std::move(vertexBuffer));
		m_VAO->SetIndexBuffer(std::move(indexBuffer));
	}

	void Mesh::ProcessMaterial(const tinygltf::Model& model, const tinygltf::Primitive& primitive)
	{
		if (primitive.material == -1)
			return;

		const auto& gltfMaterial = model.materials[static_cast<size_t>(primitive.material)];
		const auto& pbr = gltfMaterial.pbrMetallicRoughness;

		if (pbr.baseColorFactor.size() == g_ColorComponentCount)
			m_Material.SetAlbedo(ToFloatVector<glm::vec4>(pbr.baseColorFactor));

		m_Material.SetRoughness(static_cast<float>(pbr.roughnessFactor));
		m_Material.SetMetallic(static_cast<float>(pbr.metallicFactor));

		if (gltfMaterial.emissiveFactor.size() == g_VectorComponentCount)
			m_Material.SetEmissive(ToFloatVector<glm::vec3>(gltfMaterial.emissiveFactor));
	}

}
