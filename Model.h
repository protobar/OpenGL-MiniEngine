#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h" // Include Texture.h to use Texture struct
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model
{
public:
    // Model data
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<Texture> textures_loaded; // To avoid loading duplicate textures

    // Transformations
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scaleFactor;

    // Model path
    std::string path;

    // Constructor, expects a filepath to a 3D model.
    Model(std::string const& path);

    // Draws the model, and thus all its meshes
    void Draw(Shader& shader);

private:
    // Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(std::string const& path);

    // Processes a node in a recursive fashion.
    void processNode(aiNode* node, const aiScene* scene);

    // Processes a mesh and returns a Mesh object.
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);

    // Loads textures from file
    std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};

// Utility function for loading a 2D texture from file
unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);

#endif // MODEL_H
