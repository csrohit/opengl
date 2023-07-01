#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path, GLuint* program);

GLint loadShader(GLuint shaderId, const char* pFilename);
#endif
