// fragment shader
#version 330 core
struct Light {
    vec3 position;
    vec3 rotation;
    vec3 scale;
    vec3 color;
    float intensity;
};

#define MAX_LIGHTS 10
uniform int numLights;
uniform Light lights[MAX_LIGHTS];
uniform vec3 viewPos;
uniform vec3 materialColor;    // Add this uniform for BSDF base color
uniform bool useTextures;      // Add this to toggle between textured and solid color

// Material textures
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;

out vec4 FragColor;

void main()
{
    // Ambient
    vec3 ambient = vec3(0.1);
    
    // Initialize result with ambient
    vec3 result = ambient;
    
    // Normal vector
    vec3 norm = normalize(Normal);
    
    // View direction
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Iterate through all lights
    for(int i = 0; i < numLights; i++)
    {
        // Light direction
        vec3 lightDir = normalize(lights[i].position - FragPos);
        
        // Diffuse shading
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = lights[i].color * diff * lights[i].intensity;
        
        // Specular shading
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = lights[i].color * spec * lights[i].intensity * 0.5;  // Reduced specular intensity
        
        // Accumulate results
        result += diffuse + specular;
    }

    // Use either texture or material color
    vec3 color;
    if (useTextures) {
        color = texture(texture_diffuse1, TexCoords).rgb;
        vec3 specColor = texture(texture_specular1, TexCoords).rgb;
        result *= color;
        result += specColor;
    } else {
        color = materialColor;
        result *= color;
    }

    // Tone mapping and gamma correction
    result = result / (result + vec3(1.0));
    result = pow(result, vec3(1.0/2.2));
    
    FragColor = vec4(result, 1.0);
}