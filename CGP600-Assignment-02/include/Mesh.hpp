#pragma once

#include "Vertex.hpp"
#include <vector>

class Mesh
{
    private:
        std::vector<Vertex> vertices;
    public:
        void loadFromFile(const char* fileName);
};
