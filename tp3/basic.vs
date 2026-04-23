#version 330 core

in vec3 a_position;
in vec3 a_normal;

// On transmet la position et la normale (espace Monde) au Fragment Shader
out vec3 v_Position;
out vec3 v_Normal;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main() {
    // Calcul de la position dans l'espace Monde
    vec4 worldPos = u_Model * vec4(a_position, 1.0);
    v_Position = worldPos.xyz;
    
    // Position finale projetée sur l'écran
    gl_Position = u_Projection * u_View * worldPos;
    
    // Normale dans l'espace Monde. 
    // Note du cours : comme nos transformations (rotations/translations) créent 
    // une matrice orthogonale, mat3(u_Model) est égal à la transposée de l'inverse.
    v_Normal = mat3(u_Model) * a_normal; 
}