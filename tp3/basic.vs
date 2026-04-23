#version 330 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texcoords;

out vec3 v_Position;
out vec3 v_Normal;
out vec2 v_TexCoords;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main() {
    // Passage en espace Monde pour les calculs d'illumination [cite: 2462]
    vec4 worldPos = u_Model * vec4(a_position, 1.0);
    v_Position = worldPos.xyz;
    
    // Transformation de la normale [cite: 2412]
    v_Normal = mat3(u_Model) * a_normal;
    v_TexCoords = a_texcoords;
    
    gl_Position = u_Projection * u_View * worldPos;
}