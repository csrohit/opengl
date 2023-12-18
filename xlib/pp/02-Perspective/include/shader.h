#ifndef SHADER_H
#define SHADER_H
/**
 * @file      shader.h
 * @brief     shader compilation
 * @author    Rohit Nimkar
 * @version   1.0
 * @date      2023-12-19
 * @copyright Copyright 2023 Rohit Nimkar
 *
 * @attention
 *  Use of this source code is governed by a BSD-style
 *  license that can be found in the LICENSE file or at
 *  opensource.org/licenses/BSD-3-Clause
 */

#include <GL/glew.h>

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path, GLuint* program);

GLint loadShader(GLuint shaderId, const char* pFilename);
#endif
