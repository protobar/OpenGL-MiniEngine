#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // Holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include "Shader.h"
#include "Texture.h" // Include Texture.h to use Texture struct

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    // Add more attributes if needed (e.g., Tangents, Bitangents)
};

struct Material {
    glm::vec3 diffuseColor;
    glm::vec3 specularColor;
    float shininess;
    bool hasTexture;
    // You can add more material properties here (ambient, emissive, etc.)
};

class Mesh {
public:
    // Mesh Data
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    Material material;  // Material properties for the mesh

    // Constructor
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, Material material);

    // Render the mesh
    void Draw(Shader& shader);

private:
    // Render data
    unsigned int VAO, VBO, EBO;

    // Initializes all the buffer objects/arrays
    void setupMesh();
};

#endif // MESH_H
