#include "Model.h"
#include <stb_image.h>

// Function to load texture from file
unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma)
{
    std::string filename = std::string(path);
    // If the path is not absolute and doesn't start with resources/, prepend the directory
    if (filename.find('/') == std::string::npos && filename.find('\\') == std::string::npos)
    {
        // First ensure directory starts with resources/
        std::string dir = directory;
        if (dir.substr(0, 10) != "resources/") {
            dir = "resources/" + dir;
        }
        filename = dir + '/' + filename;
    }
    else if (filename.substr(0, 10) != "resources/") {
        filename = "resources/" + filename;
    }

    std::cout << "Loading texture from: " << filename << std::endl;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << filename << std::endl;
        stbi_image_free(data);
        return 0;
    }

    return textureID;
}

// Constructor for the Model class
Model::Model(std::string const& path)
{
    position = glm::vec3(0.0f);
    rotation = glm::vec3(0.0f);
    scaleFactor = glm::vec3(1.0f);

    // Prepend "resources/" to the path if it doesn't already start with it
    std::string fullPath;
    if (path.substr(0, 10) != "resources/") {
        fullPath = "resources/" + path;
    }
    else {
        fullPath = path;
    }

    this->path = fullPath;
    loadModel(fullPath);
}

// Function to draw the model with the given shader
void Model::Draw(Shader& shader)
{
    for (unsigned int i = 0; i < meshes.size(); i++)
    {
        // Set material properties for the current mesh
        shader.setBool("useTextures", meshes[i].material.hasTexture);
        shader.setVec3("materialColor", meshes[i].material.diffuseColor);
        shader.setVec3("materialSpecular", meshes[i].material.specularColor);
        shader.setFloat("materialShininess", meshes[i].material.shininess);

        // Draw the current mesh
        meshes[i].Draw(shader);
    }
}

// Load the model from the given file path
void Model::loadModel(std::string const& path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    size_t lastSlash = path.find_last_of("/\\");
    directory = (lastSlash != std::string::npos) ? path.substr(0, lastSlash) : ".";

    processNode(scene->mRootNode, scene);
}

// Recursively process each node in the model and extract meshes
void Model::processNode(aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

// Process the mesh data and extract vertex, index, and texture information
Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // Process vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector;

        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }
        else
        {
            vertex.Normal = glm::vec3(0.0f);
        }

        if (mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        }
        else
        {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    // Process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    Material material;

    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];

        aiColor3D diffuse(0.0f, 0.0f, 0.0f);
        aiColor3D specular(0.0f, 0.0f, 0.0f);
        float shininess = 0.0f;

        mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
        mat->Get(AI_MATKEY_COLOR_SPECULAR, specular);
        mat->Get(AI_MATKEY_SHININESS, shininess);

        material.diffuseColor = glm::vec3(diffuse.r, diffuse.g, diffuse.b);
        material.specularColor = glm::vec3(specular.r, specular.g, specular.b);
        material.shininess = shininess;
        material.hasTexture = mat->GetTextureCount(aiTextureType_DIFFUSE) > 0;

        if (material.hasTexture)
        {
            std::vector<Texture> diffuseMaps = LoadMaterialTextures(mat, aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        }
    }

    return Mesh(vertices, indices, textures, material);  // Pass material to mesh constructor
}

// Load material textures for the mesh
std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        // Check if texture was loaded before
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if (!skip)
        {
            Texture texture;
            // Handle path for textures
            std::string texturePath = std::string(str.C_Str());
            if (texturePath.substr(0, 10) != "resources/") {
                texturePath = "resources/" + texturePath;
            }

            std::cout << "Attempting to load texture from path: " << texturePath << std::endl;
            texture.id = TextureFromFile(str.C_Str(), directory);
            if (texture.id == 0)
            {
                std::cout << "Failed to load texture at path: " << texturePath << std::endl;
                continue;
            }
            texture.type = typeName;
            texture.path = texturePath;
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }
    return textures;
}
