#version 330 core

in vec3 a_position;
in vec3 a_normal;
in vec2 a_uv; 

out vec3 v_Normal;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main() {
    // Calcul de la position finale
    gl_Position = u_Projection * u_View * u_Model * vec4(a_position, 1.0);
    
    // On convertit la normale en repère monde (matrice 3x3 pour ignorer la translation)
    v_Normal = mat3(u_Model) * a_normal; 
}