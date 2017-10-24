#include "Mesh.hpp"
#include "StringHelpers.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <Windows.h>

void Mesh::loadFromFile(const char* fileName)
{
    std::ifstream file(fileName);

    if (!file)
    {
        OutputDebugString((std::string("#### Failed to load file: ") + std::string(fileName) + " ####").c_str());
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::vector<std::string> data = StringHelpers::split(line, ' ');

        if (data[0] == "v")
        {
            vertices.push_back({
                DirectX::XMFLOAT3(std::atof(data[1].c_str()), std::atof(data[2].c_str()), std::atof(data[3].c_str())),
                DirectX::XMFLOAT4(1.f, 0.f, 0.f, 1.f)
            });
        }
    }

    file.close();
}
