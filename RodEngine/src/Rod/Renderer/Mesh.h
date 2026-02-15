#pragma once
#include <string>
#include <vector>
#include <memory>

#include <tiny_gltf.h>

#include "Buffer.h"
#include "VertexArray.h"

namespace Rod {
    class Mesh {
    public:
        static Ref<Mesh> Create(const std::string& glbPath);
        Mesh(const std::string& glbPath);

        Ref<VertexArray> GetVAO() const { return m_VAO; }

    private:
        void LoadGLB(const std::string& path);
        void ProcessPrimitive(const tinygltf::Model& model, const tinygltf::Primitive& primitive);

    private:
        Ref<VertexArray> m_VAO;
    };

}
