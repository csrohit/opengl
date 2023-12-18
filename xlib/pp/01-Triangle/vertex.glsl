#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexColor;
out vec3 fragmentColor;
void main(){

    gl_Position = vec4(vertexPosition_modelspace, 1);
    fragmentColor = vertexColor;
}

