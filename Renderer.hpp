//
// Created by Nikita Kruk on 17.02.20.
//

#ifndef SPPKURAMOTOWITHINERTIAMOVIE_RENDERER_HPP
#define SPPKURAMOTOWITHINERTIAMOVIE_RENDERER_HPP

#include "Definitions.hpp"
#include "Model.hpp"

#include <string>
#include <list>
#include <vector>

#include <GLEW/glew.h>
#include <GLFW/glfw3.h>

#include <ft2build.h>
#include FT_FREETYPE_H

class Renderer
{
 public:

  Renderer(Model *model);
  ~Renderer();

  void Start();

 private:

  Model *model_;
  std::list<std::vector<GLfloat>> system_state_over_time_interval_;
  std::list<std::vector<GLfloat>> coloring_over_time_interval_;

  static bool stop_flag_;
  static bool pause_flag_;
  static bool take_screenshot_flag_;
  static int screenshot_count_;
  static int frame_speed_; // 1 - the basic frame rate
  static GLfloat x_shift_;
  static GLfloat y_shift_;
  static GLfloat z_scale_;
  static int t_start_;
  static bool show_time_;
  static Real time_stamp_to_show_;
  static int number_of_points_per_particle_;
  static int system_size_;
  static int number_of_color_components_; // RGB | Alpha is implemented in the shaders
  static ParticleRepresentationType particle_representation_type_;
  FT_Library ft_; // FreeType library object
  FT_Face face_; // FreeType face object

  static void ErrorCallback(int error, const char *description);
  static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
  void InitShaders(GLuint shader_program[]);
  static void SetShaderParameter(GLuint shader_program,
                                 GLfloat parameter_value,
                                 const std::string &parameter_name_in_shader);
  static void SetShaderParameter(GLuint shader_program,
                                 GLint parameter_value,
                                 const std::string &parameter_name_in_shader);
  static void FinFunc();
  void InitializeSystemState();
  void ReadNewState();
  void DisplayFunc(GLFWwindow *window, GLuint vao[], GLuint vbo[], GLuint shader_program[]);
  static void RenderBorder(GLuint vao, GLuint vbo, GLuint shader_program);
  void RenderParticles(GLuint vao, GLuint vbo, GLuint shader_program);
  void RenderText(const std::string &text,
                  GLfloat x,
                  GLfloat y,
                  GLfloat sx,
                  GLfloat sy,
                  GLuint vao,
                  GLuint vbo,
                  GLuint shader_program);
  static void ReadShaderSource(const std::string &fname, std::vector<char> &buffer);
  static GLuint LoadAndCompileShader(const std::string &fname, GLenum shader_type);
  static GLuint CreateProgramFromShader(const std::string &vertex_shader_path, const std::string &fragment_shader_path);
  static GLuint CreateProgramFromShader(const std::string &vertex_shader_path,
                                        const std::string &geometry_shader_path,
                                        const std::string &fragment_shader_path);
  void DetermineAverageAngularVelocity();
  void ConstructAngularLinkedList(std::vector<std::vector<int>> &linked_list);

};

#endif //SPPKURAMOTOWITHINERTIAMOVIE_RENDERER_HPP
