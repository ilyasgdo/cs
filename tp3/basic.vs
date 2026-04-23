#version 330 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texcoords;

// Variables de sortie vers le Fragment Shader
out vec3 v_Position;
out vec3 v_Normal;
out vec2 v_TexCoords; 

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main() {
    // Calcul de la position dans le monde
    vec4 worldPos = u_Model * vec4(a_position, 1.0);
    v_Position = worldPos.xyz;
    
    // Transmission de la normale et des UVs
    v_Normal = mat3(u_Model) * a_normal;
    v_TexCoords = a_texcoords; 
    
    // Position finale à l'écran
    gl_Position = u_Projection * u_View * worldPos;
}