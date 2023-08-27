#version 430

in vec3 position;
in vec3 color;

in vec2 texcoord;

out vec3 Color;
out vec2 Texcoord;
out vec4 Vertex;

uniform float time;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    Color = color;
    Texcoord = texcoord;
    gl_Position = proj * view * model * vec4(position, 1.0);

    Vertex = model * vec4(position, 1.0);
}
