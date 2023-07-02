#version 330 core

// Ouput data
out vec4 color;
in vec2 fragmentColor;
uniform sampler2D texture1;
void main()
{

	// Output color = red 
	color = texture(texture1, fragmentColor);

}
