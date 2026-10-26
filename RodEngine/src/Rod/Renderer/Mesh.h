#pragma once
#include <string>
#include <vector>
#include <memory>

#include <tiny_gltf.h>

#include "Buffer.h"
#include "Material.h"
#include "VertexArray.h"

namespace Rod {
    class Mesh {
    public:
        explicit Mesh(const std::string& glbPath);

        const VertexArray* GetVAO() const { return m_VAO.get(); }
        const Material& GetMaterial() const { return m_Material; }
        const std::string& GetPath() const { return m_SourcePath; }

    private:
        void LoadGLB(const std::string& path);
        void ProcessPrimitive(const tinygltf::Model& model, const tinygltf::Primitive& primitive);
        void ProcessMaterial(const tinygltf::Model& model, const tinygltf::Primitive& primitive);

    private:
        Scope<VertexArray> m_VAO;
        Material m_Material;
        std::string m_SourcePath;
    };

}
