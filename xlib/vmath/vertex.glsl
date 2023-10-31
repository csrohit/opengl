#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexColor;
uniform mat4 MVP;
out vec3 fragmentColor;
out vec2 TexCoord;
void main(){

    gl_Position = MVP * vec4(vertexPosition_modelspace, 1);
    //fragmentColor = vertexColor;
	TexCoord = vec2(vertexColor.x, vertexColor.y);
}

