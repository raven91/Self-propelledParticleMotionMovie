#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <fstream>
#include <cmath>
#include <sstream>
#include <vector>
#include <string>
#include <list>
#include <algorithm> // std::copy std::max_element
#include <iomanip>   // std::set_precision
#include <numeric>   // std::iota
#include <cassert>

#include <ft2build.h>
#include FT_FREETYPE_H

const size_t kS = 3;
const size_t kN = 50000;
const size_t kL = 1;
const size_t kPointsPerParticle = 20;
const size_t kDtRecip = 1;
//const GLfloat kDt = 1.0f / GLfloat(kDtRecip);
const size_t kTStart = 0;

//const std::string kSolutionFileName = std::string("/Users/nikita/Documents/Projects/spc2/spc2OdeIntegration/")
//const std::string kSolutionFileName = std::string("/Volumes/Kruk/spc2/spc2OdeIntegration/continued/")
const std::string
    kSolutionFileName = std::string("/Volumes/Kruk/spc2/spc2OdeIntegration/continued/")
    + std::string("v0_0.01_sigma_1_rho_0.01_alpha_1_Dphi_0.0575_N_") + std::to_string(kN)
    + std::string("_0_0.bin");
std::ifstream solution_file;
const std::string kColorFileName
    ("/Users/nikita/Documents/spc2/spc2OdeIntegration/Rk4Localized/color_by_frequency5_sigma_4_rho_0.3_alpha_1.54_Dphi_0_N_1000_0.bin");
std::ifstream color_file;
//#define USE_PRECALCULATED_COLOR

std::list<std::vector<GLfloat>> x;//(kPointsPerParticle, std::vector<GLfloat>(kS * kN, 0.0f));
GLfloat t = 0.0f;
const size_t kColorComponents = 3;//RGB | Alpha is implemented in the shaders
std::list<std::vector<GLfloat>> color;//(kColorComponents * kN, 0.0f);
GLfloat min_color = 0.0f;
GLfloat max_color = 0.0f;

GLboolean stop_flag = true;
GLboolean pause_flag = true;
GLboolean take_screenshot_flag = false;
GLfloat red_blue_boundary = 2.5f; // nlc:0.4 lc1:0.67 lc4:2.5
int spc2FrameSpeed = 1; // 1 - the basic frame rate
GLfloat kZScale = 1.0f;

FT_Library ft; // FreeType library object
FT_Face face; // FreeType face object
bool show_time = true;

int screenshot_count = 0;

void ErrorCallback(int error, const char *description);
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
void DisplayFunc(GLFWwindow *window, GLuint vao[], GLuint vbo[], GLuint shader_program[]);
void RenderBorder(GLuint vao, GLuint vbo, GLuint shader_program);
void RenderParticles(GLuint vao, GLuint vbo, GLuint shader_program);
void RenderText(const std::string &text,
                GLfloat x,
                GLfloat y,
                GLfloat sx,
                GLfloat sy,
                GLuint vao,
                GLuint vbo,
                GLuint shader_program);
void InitFunc(GLFWwindow *window, GLuint vao[], GLuint vbo[], GLuint shader_program[]);
void InitShaders(GLuint shader_program[]);
void SetShaderParameter(GLuint shader_program, GLfloat parameter_value, const std::string &parameter_name_in_shader);
void SetShaderParameter(GLuint shader_program, GLint parameter_value, const std::string &parameter_name_in_shader);
void FinFunc();
void ReadNewState();
void ReadShaderSource(const std::string &fname, std::vector<char> &buffer);
GLuint LoadAndCompileShader(const std::string &fname, GLenum shader_type);
GLuint CreateProgramFromShader(const std::string &vertex_shader_path, const std::string &fragment_shader_path);
GLuint CreateProgramFromShader(const std::string &vertex_shader_path,
                               const std::string &geometry_shader_path,
                               const std::string &fragment_shader_path);
void RedBlueColors(GLfloat c, GLfloat &r, GLfloat &g, GLfloat &b);
GLfloat ModWrap0Denom(GLfloat numerator, GLfloat denominator);
void ApplyPeriodicBoundaryConditions(std::vector<float> &x);
void FindColorMinMax(const std::string &color_file_name);

void TakeScreenshotPpm(int width, int height, int image_index);
void FreePpm();
void TakeScreenshotPng(unsigned int width, unsigned int height, int image_index);
void FreePng();
void Jet2Rgb(float j, float &r, float &g, float &b);
void Hsv2Rgb(float h, float s, float v, float &r, float &g, float &b);

int main(int argc, char *argv[])
{
  GLFWwindow *window;

  glfwSetErrorCallback(ErrorCallback);

  if (!glfwInit())
  {
    std::cerr << "Initialization of GLFW failure" << std::endl;
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_SAMPLES, 4);//MSAA

  window = glfwCreateWindow(1280, 1280, "Particle Dynamics", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    std::cerr << "Window opening failure" << std::endl;
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glfwSetKeyCallback(window, KeyCallback);

  int major, minor, rev;
  major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
  minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
  rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
  std::cout << "OpenGL - " << major << "." << minor << "." << rev << std::endl;

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK)
  {
    std::cerr << "GLEW initialization failure" << std::endl;
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  // contexts for border, particles, text
  GLuint vao[3] = {0};
  GLuint vbo[3] = {0};
  GLuint shader_program[3] = {0};
  glGenVertexArrays(3, &vao[0]);
  glGenBuffers(3, &vbo[0]);

  InitFunc(window, vao, vbo, shader_program);

  while (!glfwWindowShouldClose(window))
  {
    DisplayFunc(window, vao, vbo, shader_program);

//		if (!stop_flag)
    if (take_screenshot_flag)
    {
      int width, height;
      glfwGetFramebufferSize(window, &width, &height);
//			glfwGetWindowSize(window, &width, &height);

//			glReadBuffer(GL_BACK);

//			GLubyte *pixels = NULL;
//			TakeScreenshotPpm(width, height, screenshot_count++);
      TakeScreenshotPng(width, height, screenshot_count++);
//			free(pixels);

      take_screenshot_flag = false;
    }

    glfwSwapBuffers(window);
    glfwPollEvents();

//    if (t >= 100.0f)
//    {
//      glfwSetWindowShouldClose(window, GL_TRUE);
//    }
  }

  FinFunc();
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}

void ErrorCallback(int error, const char *description)
{
  std::cerr << description << std::endl;
}

void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  if (GLFW_PRESS == action)
  {
    switch (key)
    {
      case GLFW_KEY_ESCAPE:
//			case GLFW_PRESS:
        glfwSetWindowShouldClose(window, GL_TRUE);
        break;

      case GLFW_KEY_S:stop_flag = !stop_flag;
        break;

      case GLFW_KEY_P:
        if (stop_flag)
        {
          pause_flag = !pause_flag;
        }
        break;

      case GLFW_KEY_I:take_screenshot_flag = true;
        break;

      case GLFW_KEY_O:++spc2FrameSpeed;
        break;

      case GLFW_KEY_L:
        if (spc2FrameSpeed > 1)
        {
          --spc2FrameSpeed;
        }
        break;

      case GLFW_KEY_T: show_time = !show_time;
        break;

      case GLFW_KEY_LEFT:red_blue_boundary -= 0.01f;
        std::cout << "red_blue_boundary = " << red_blue_boundary << std::endl;
        break;

      case GLFW_KEY_RIGHT:red_blue_boundary += 0.01f;
        std::cout << "red_blue_boundary = " << red_blue_boundary << std::endl;
        break;

      case GLFW_KEY_5: kZScale /= 1.25f;
        break;

      case GLFW_KEY_6: kZScale *= 1.25f;
        break;

      default:break;
    }
  }
}

void InitFunc(GLFWwindow *window, GLuint vao[], GLuint vbo[], GLuint shader_program[])
{
#if defined(USE_PRECALCULATED_COLOR)
  FindColorMinMax(kColorFileName);
#endif

  solution_file.open(kSolutionFileName, std::ios::binary | std::ios::in);
  assert(solution_file.is_open());
  solution_file.seekg(kTStart * kDtRecip * (1l + kS * kN) * sizeof(float), std::ios::beg);

#if defined(USE_PRECALCULATED_COLOR)
  color_file.open(kColorFileName, std::ios::binary | std::ios::in);
  assert(color_file.is_open());
  color_file.seekg(kTStart * kDtRecip * (1 + kN) * sizeof(float), std::ios::beg);
#endif

  float temp = 0.0f;
  std::vector<GLfloat> x_temp(kS * kN, 0.0f);
//	std::vector<GLfloat> x_temp_reordered(kS * kN, 0.0f);
  std::vector<GLfloat> color_temp(kColorComponents * kN, 0.0f);
  std::vector<GLfloat> color_base(kN, 0.0f);

  for (size_t n = 0; n < kPointsPerParticle; ++n)
  {
    solution_file.read((char *) &temp, sizeof(float));
    solution_file.read((char *) &x_temp[0], kS * kN * sizeof(GLfloat));
//		solution_file.read((char *)&x_temp_reordered[0], kS * kN * sizeof(GLfloat));
//		for (int i = 0; i < kN; ++i)
//		{
//			x_temp[kS * i    ] = x_temp_reordered[i            ];
//			x_temp[kS * i + 1] = x_temp_reordered[i +     kN];
//			x_temp[kS * i + 2] = x_temp_reordered[i + 2 * kN];
//		}
    ApplyPeriodicBoundaryConditions(x_temp);
    x.push_back(x_temp);
//    std::cout << x_temp[0] << '\t' << x_temp[1] << std::endl;

#if defined(USE_PRECALCULATED_COLOR)
    color_file.read((char *)&temp, sizeof(float));
    color_file.read((char *)&color_base[0], kN * sizeof(float));
    std::for_each(color_base.begin(), color_base.end(),
                  [](GLfloat &c)
                  {
                    c = std::fabs(c);
                  });
#endif

    for (size_t i = 0; i < kN; ++i)
    {
#if defined(USE_PRECALCULATED_COLOR)
      RedBlueColors(color_base[i], color_temp[kColorComponents * i], color_temp[kColorComponents * i + 1], color_temp[kColorComponents * i + 2]);
#else
      //			Jet2Rgb(ModWrap0Denom(x_temp[kS * i + 2], 2.0f * M_PI) / (2.0f * M_PI), color_temp[kColorComponents * i], color_temp[kColorComponents * i + 1], color_temp[kColorComponents * i + 2]);
      Hsv2Rgb(ModWrap0Denom(x_temp[kS * i + 2], 2.0f * M_PI) / (2.0f * M_PI),
              1.0f,
              1.0f,
              color_temp[kColorComponents * i],
              color_temp[kColorComponents * i + 1],
              color_temp[kColorComponents * i + 2]);
//			Jet2Rgb(color_base[i], color_temp[kColorComponents * i], color_temp[kColorComponents * i + 1], color_temp[kColorComponents * i + 2]);
//			Hsv2Rgb(color_base[i], 1.0f, 1.0f, color_temp[kColorComponents * i], color_temp[kColorComponents * i + 1], color_temp[kColorComponents * i + 2]);
#endif
    } // i
    color.push_back(color_temp);
  } // n
  // averaged colors
  /*std::vector<GLfloat> phis(kN, 0.0f);
  for (const std::vector<GLfloat> &vec : x)
  {
    for (int i = 0; i < kN; ++i)
    {
      phis[i] += vec[kS * i + 2] / kPointsPerParticle;
    } // i
  } // vec
  for (int i = 0; i < kN; ++i)
  {
    Hsv2Rgb(ModWrap0Denom(phis[i], 2.0f * M_PI) / (2.0f * M_PI),
            1.0f,
            1.0f,
            color_temp[kColorComponents * i],
            color_temp[kColorComponents * i + 1],
            color_temp[kColorComponents * i + 2]);
  } // i
  for (std::list<std::vector<GLfloat>>::iterator it = color.begin(); it != color.end(); ++it)
  {
    *it = color_temp;
  } // it*/

//	t += kPointsPerParticle * kDt;
  t = temp;

  glEnable(GL_BLEND);
//	glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR);
//	glBlendEquation(GL_FUNC_ADD);
//	glBlendEquation(GL_MIN);

  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_MULTISAMPLE); // MSAA

  InitShaders(shader_program);
  SetShaderParameter(shader_program[1], (GLint) kPointsPerParticle, "kPointsPerParticle");
  SetShaderParameter(shader_program[1], (GLint) kL, "kSystemSize");
  SetShaderParameter(shader_program[1], (GLfloat) kZScale, "kZScale");
}

void InitShaders(GLuint shader_program[])
{
  shader_program[0] = CreateProgramFromShader(std::string(
      "/Users/nikita/CLionProjects/sppKuramotoWithInertiaMovie/Shaders/border_vertex_shader.shader"),
                                              std::string(
                                                  "/Users/nikita/CLionProjects/sppKuramotoWithInertiaMovie/Shaders/border_fragment_shader.shader"));
  shader_program[1] = CreateProgramFromShader(std::string(
      "/Users/nikita/CLionProjects/sppKuramotoWithInertiaMovie/Shaders/particle_vertex_shader.shader"),
                                              std::string(
                                                  "/Users/nikita/CLionProjects/sppKuramotoWithInertiaMovie/Shaders/particle_geometry_shader.shader"),
                                              std::string(
                                                  "/Users/nikita/CLionProjects/sppKuramotoWithInertiaMovie/Shaders/particle_fragment_shader.shader"));
  shader_program[2] = CreateProgramFromShader(std::string(
      "/Users/nikita/CLionProjects/sppKuramotoWithInertiaMovie/Shaders/text_vertex_shader.shader"),
                                              std::string(
                                                  "/Users/nikita/CLionProjects/sppKuramotoWithInertiaMovie/Shaders/text_fragment_shader.shader"));
  if (FT_Init_FreeType(&ft))
  {
    std::cerr << "Could not init freetype library" << std::endl;
  }
  if (FT_New_Face(ft, "/System/Library/Fonts/HelveticaNeueDeskInterface.ttc", 0, &face))
  {
    std::cerr << "Could not open font" << std::endl;
  }
  FT_Set_Pixel_Sizes(face, 0, 48);
  GLuint texture;
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  GLint tex_location = glGetUniformLocation(shader_program[2], "tex");
  glUniform1i(tex_location, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void SetShaderParameter(GLuint shader_program, GLfloat parameter_value, const std::string &parameter_name_in_shader)
{
  GLint location = glGetUniformLocation(shader_program, parameter_name_in_shader.c_str());
  if (-1 != location)
  {
    glUniform1i(location, parameter_value);
  }
}

void SetShaderParameter(GLuint shader_program, GLint parameter_value, const std::string &parameter_name_in_shader)
{
  GLint location = glGetUniformLocation(shader_program, parameter_name_in_shader.c_str());
  if (-1 != location)
  {
    glUniform1f(location, parameter_value);
  }
}

void FinFunc()
{
  if (solution_file.is_open())
  {
    solution_file.close();
  }

  if (color_file.is_open())
  {
    color_file.close();
  }

//	FreePpm();
  FreePng();
}

void ReadNewState()
{
  if (!stop_flag || !pause_flag)
  {
    if (solution_file.is_open())
    {
      float temp;
      std::vector<GLfloat> x_temp(kS * kN, 0.0f);
//			std::vector<GLfloat> x_temp_reordered(kS * kN, 0.0f);
      std::vector<GLfloat> color_temp(kColorComponents * kN, 0.0f);
      std::vector<GLfloat> color_base(kN, 0.0f);

      solution_file.read((char *) &temp, sizeof(float));
      solution_file.read((char *) &x_temp[0], kS * kN * sizeof(float));
//			solution_file.read((char *)&x_temp_reordered[0], kS * kN * sizeof(float));
//			for (int i = 0; i < kN; ++i)
//			{
//				x_temp[kS * i] = x_temp_reordered[i];
//				x_temp[kS * i + 1] = x_temp_reordered[i + kN];
//				x_temp[kS * i + 2] = x_temp_reordered[i + 2 * kN];
//			}
      ApplyPeriodicBoundaryConditions(x_temp);

//			t += kDt;
      t = temp;
      std::cout << temp << std::endl;

#if defined(USE_PRECALCULATED_COLOR)
      color_file.read((char *)&temp, sizeof(float));
      color_file.read((char *)&color_base[0], kN * sizeof(GLfloat));
      std::for_each(color_base.begin(), color_base.end(),
                    [](GLfloat &c)
                    {
                      c = std::fabs(c);
                    });
#endif

      x.push_back(x_temp);
      x.pop_front();

      for (size_t i = 0; i < kN; ++i)
      {
#if defined(USE_PRECALCULATED_COLOR)
        RedBlueColors(color_base[i], color_temp[kColorComponents * i], color_temp[kColorComponents * i + 1], color_temp[kColorComponents * i + 2]);
#else
        //				Jet2Rgb(ModWrap0Denom(x_temp[kS * i + 2], 2.0f * M_PI) / (2.0f * M_PI), color_temp[kColorComponents * i], color_temp[kColorComponents * i + 1], color_temp[kColorComponents * i + 2]);
        Hsv2Rgb(ModWrap0Denom(x_temp[kS * i + 2], 2.0f * M_PI) / (2.0f * M_PI),
                1.0f,
                1.0f,
                color_temp[kColorComponents * i],
                color_temp[kColorComponents * i + 1],
                color_temp[kColorComponents * i + 2]);
//				Jet2Rgb(color_base[i], color_temp[kColorComponents * i], color_temp[kColorComponents * i + 1], color_temp[kColorComponents * i + 2]);
//				Hsv2Rgb(color_base[i], 1.0f, 1.0f, color_temp[kColorComponents * i], color_temp[kColorComponents * i + 1], color_temp[kColorComponents * i + 2]);
#endif
//        color_temp[kColorComponents * i] = 1.0;
//        color_temp[kColorComponents * i + 1] = 1.0;
//        color_temp[kColorComponents * i + 2] = 1.0;
      }
      color.push_back(color_temp);
      color.pop_front();
      // averaged colors
      /*std::vector<GLfloat> phis(kN, 0.0f);
      for (const std::vector<GLfloat> &vec : x)
      {
        for (int i = 0; i < kN; ++i)
        {
          phis[i] += vec[kS * i + 2] / kPointsPerParticle;
        } // i
      } // vec
      for (int i = 0; i < kN; ++i)
      {
        Hsv2Rgb(ModWrap0Denom(phis[i], 2.0f * M_PI) / (2.0f * M_PI),
                1.0f,
                1.0f,
                color_temp[kColorComponents * i],
                color_temp[kColorComponents * i + 1],
                color_temp[kColorComponents * i + 2]);
      } // i
      *(std::prev(color.end())) = color_temp;*/

      //speed up or down the silumation output
      for (int n = 0; n < spc2FrameSpeed - 1; ++n)
      {
        solution_file.seekg((1 + kS * kN) * sizeof(GLfloat), std::ios::cur);
      }
    }
    pause_flag = true;
  }
}

void DisplayFunc(GLFWwindow *window, GLuint vao[], GLuint vbo[], GLuint shader_program[])
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glShadeModel(GL_SMOOTH);

  glUseProgram(shader_program[0]);
  RenderBorder(vao[0], vbo[0], shader_program[0]);

  glUseProgram(shader_program[1]);
  RenderParticles(vao[1], vbo[1], shader_program[1]);

  if (show_time)
  {
    glUseProgram(shader_program[2]);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    GLfloat sx = 2.0f / width;
    GLfloat sy = 2.0f / height;

    std::ostringstream buffer;
    buffer << std::fixed << std::setprecision(2) << "t = " << t;
    RenderText(buffer.str(), -1.0f + 8.0f * sx, 1.0f - 50.0f * sy, sx, sy, vao[2], vbo[2], shader_program[2]);
  }
}

void RenderBorder(GLuint vao, GLuint vbo, GLuint shader_program)
{
  glBindVertexArray(vao);

  GLfloat border_vertices[8] =
      {
          0.0f, 0.0f,
          1.0f, 0.0f,
          1.0f, 1.0f,
          0.0f, 1.0f
      };

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), border_vertices, GL_STATIC_DRAW);

  GLint position_attribute = glGetAttribLocation(shader_program, "position");
  glVertexAttribPointer(position_attribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(position_attribute);

  glBindVertexArray(vao);
  glDrawArrays(GL_LINE_LOOP, 0, 4);

  glDisableVertexAttribArray(position_attribute);
}

void RenderParticles(GLuint vao, GLuint vbo, GLuint shader_program)
{
  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER,
               kPointsPerParticle * kS * kN * sizeof(GLfloat)
                   + kPointsPerParticle * kColorComponents * kN * sizeof(GLfloat)
                   + kPointsPerParticle * kN * sizeof(GLint),
               NULL,
               GL_DYNAMIC_DRAW);

  for (size_t i = 0; i < kN; ++i)
  {
    size_t n = 0;
    for (std::list<std::vector<GLfloat>>::iterator it_x = x.begin(); it_x != x.end(); ++it_x)
    {
      glBufferSubData(GL_ARRAY_BUFFER,
                      i * kPointsPerParticle * kS * sizeof(GLfloat) + n * kS * sizeof(GLfloat),
                      kS * sizeof(GLfloat),
                      &((*it_x)[kS * i]));
      ++n;
    }
  }

  for (size_t i = 0; i < kN; ++i)
  {
    size_t n = 0;
    for (std::list<std::vector<GLfloat>>::iterator it_color = color.begin(); it_color != color.end(); ++it_color)
    {
      glBufferSubData(GL_ARRAY_BUFFER,
                      kPointsPerParticle * kS * kN * sizeof(GLfloat)
                          + i * kPointsPerParticle * kColorComponents * sizeof(GLfloat)
                          + n * kColorComponents * sizeof(GLfloat),
                      kColorComponents * sizeof(GLfloat),
                      &((*it_color)[kColorComponents * i]));
      ++n;
    }
  }

  std::vector<GLint> trajectory_indexes(kPointsPerParticle, 0);
  std::iota(trajectory_indexes.begin(), trajectory_indexes.end(), 0);
  for (int i = 0; i < kN; ++i)
  {
    glBufferSubData(GL_ARRAY_BUFFER,
                    kPointsPerParticle * kS * kN * sizeof(GLfloat)
                        + kPointsPerParticle * kColorComponents * kN * sizeof(GLfloat)
                        + i * kPointsPerParticle * sizeof(GLint),
                    kPointsPerParticle * sizeof(GLint),
                    &trajectory_indexes[0]);
  }

  GLint position_attribute = glGetAttribLocation(shader_program, "position");
  glVertexAttribPointer(position_attribute, 2, GL_FLOAT, GL_FALSE, kS * sizeof(GLfloat), 0);
  glEnableVertexAttribArray(position_attribute);

  GLint color_attribute = glGetAttribLocation(shader_program, "color_into_vertex_shader");
  glVertexAttribPointer(color_attribute,
                        kColorComponents,
                        GL_FLOAT,
                        GL_FALSE,
                        kColorComponents * sizeof(GLfloat),
                        (GLvoid *) (kPointsPerParticle * kS * kN * sizeof(GLfloat)));
  glEnableVertexAttribArray(color_attribute);

  GLint trajectory_index_attribute = glGetAttribLocation(shader_program, "trajectory_index");
  glVertexAttribIPointer(trajectory_index_attribute,
                         1,
                         GL_INT,
                         sizeof(GLint),
                         (GLvoid *) (kPointsPerParticle * kS * kN * sizeof(GLfloat)
                             + kPointsPerParticle * kColorComponents * kN * sizeof(GLfloat)));
  glEnableVertexAttribArray(trajectory_index_attribute);

  glBindVertexArray(vao);
  for (int i = 0; i < kN; ++i)
  {
    glDrawArrays(GL_LINE_STRIP, i * kPointsPerParticle, kPointsPerParticle);
//    glDrawArrays(GL_POINTS, i * kPointsPerParticle, kPointsPerParticle);
  }

  glDisableVertexAttribArray(position_attribute);
  glDisableVertexAttribArray(color_attribute);
  glDisableVertexAttribArray(trajectory_index_attribute);

  ReadNewState();

  /*glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glBufferData(GL_ARRAY_BUFFER, kPointsPerParticle * kS * kN * sizeof(GLfloat) + kPointsPerParticle * kColorComponents * kN * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
  size_t n = 0;
  for (std::list<std::vector<GLfloat>>::iterator it_x = x.begin(), it_color = color.begin(); it_x != x.end(); ++it_x, ++it_color)
  {
      glBufferSubData(GL_ARRAY_BUFFER, n * kS * kN * sizeof(GLfloat), kS * kN * sizeof(GLfloat), &((*it_x)[0]));

      //reassign alpha channels
      for (size_t i = 0; i < kN; ++i)
      {
          (*it_color)[kColorComponents * i + 3] = ModifiedAlphaChannel(GLfloat(n) * 1.0f / GLfloat(kPointsPerParticle - 1));
//			(*it_color)[kColorComponents * i + 3] = 1.0f;
      }
      glBufferSubData(GL_ARRAY_BUFFER, kPointsPerParticle * kS * kN * sizeof(GLfloat) + n * kColorComponents * kN * sizeof(GLfloat), kColorComponents * kN * sizeof(GLfloat), &((*it_color)[0]));

      ++n;
  }

  GLuint shader_program = CreateProgramFromShader(std::string("/Users/nikita/Projects/spc2Movie/spc2Movie/Shaders/particle_vertex_shader.shader"), std::string("/Users/nikita/Projects/spc2Movie/spc2Movie/Shaders/particle_fragment_shader.shader"), std::string("/Users/nikita/Projects/spc2Movie/spc2Movie/Shaders/particle_geometry_shader.shader"));

  GLint position_attribute = glGetAttribLocation(shader_program, "position");
  glVertexAttribPointer(position_attribute, 2, GL_FLOAT, GL_FALSE, kS * sizeof(GLfloat), 0);
  glEnableVertexAttribArray(position_attribute);

  GLint color_attribute = glGetAttribLocation(shader_program, "color_into_vertex_shader");
  glVertexAttribPointer(color_attribute, kColorComponents, GL_FLOAT, GL_FALSE, kColorComponents * sizeof(GLfloat), (GLvoid *)(kPointsPerParticle * kS * kN * sizeof(GLfloat)));
  glEnableVertexAttribArray(color_attribute);

//	GLint position_0_attribute = glGetAttribLocation(shader_program, "position_0");
//	glVertexAttribPointer(position_0_attribute, 2, GL_FLOAT, GL_FALSE, kS * sizeof(GLfloat), 0);
//	glEnableVertexAttribArray(position_0_attribute);
//
//	GLint position_1_attribute = glGetAttribLocation(shader_program, "position_1");
//	glVertexAttribPointer(position_1_attribute, 2, GL_FLOAT, GL_FALSE, kS * sizeof(GLfloat), (GLvoid *)(kS * kN * sizeof(GLfloat)));

  glBindVertexArray(vao);
  glDrawArrays(GL_LINES, 0, (kPointsPerParticle) * kN);

  ReadNewState();
   */
}

void RenderText(const std::string &text,
                GLfloat x,
                GLfloat y,
                GLfloat sx,
                GLfloat sy,
                GLuint vao,
                GLuint vbo,
                GLuint shader_program)
{
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  GLint coord_location = glGetAttribLocation(shader_program, "generic_coord");
  glVertexAttribPointer(coord_location, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(coord_location);

  FT_GlyphSlot glyph_slot = face->glyph;

  for (size_t i = 0; i < text.length(); ++i)
  {
    if (FT_Load_Char(face, text[i], FT_LOAD_RENDER))
    {
      continue;
    }

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RED,
                 glyph_slot->bitmap.width,
                 glyph_slot->bitmap.rows,
                 0,
                 GL_RED,
                 GL_UNSIGNED_BYTE,
                 glyph_slot->bitmap.buffer);

    GLfloat x2 = x + glyph_slot->bitmap_left * sx;
    GLfloat y2 = -y - glyph_slot->bitmap_top * sy;
    GLfloat w = glyph_slot->bitmap.width * sx;
    GLfloat h = glyph_slot->bitmap.rows * sy;

    GLfloat box[4][4] =
        {
            {x2, -y2, 0.0f, 0.0f},
            {x2 + w, -y2, 1.0f, 0.0f},
            {x2, -y2 - h, 0.0f, 1.0f},
            {x2 + w, -y2 - h, 1.0f, 1.0f}
        };

    glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    x += (glyph_slot->advance.x / 64) * sx;
    y += (glyph_slot->advance.y / 64) * sy;
  }
}

void ReadShaderSource(const std::string &fname, std::vector<char> &buffer)
{
  std::ifstream in;
  in.open(fname, std::ios::binary | std::ios::in);

  if (in.is_open())
  {
    in.seekg(0, std::ios::end);
    size_t length = (size_t) in.tellg();

    in.seekg(0, std::ios::beg);

    buffer.resize(length + 1);
    in.read((char *) &buffer[0], length);
    buffer[length] = '\0';

    in.close();
  } else
  {
    std::cerr << "Unable to read the shader file \"" << fname << "\"" << std::endl;
    exit(EXIT_FAILURE);
  }
}

GLuint LoadAndCompileShader(const std::string &fname, GLenum shader_type)
{
  std::vector<char> buffer;
  ReadShaderSource(fname, buffer);
  const char *src = &buffer[0];

  GLuint shader = glCreateShader(shader_type);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);

  GLint compilation_test;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compilation_test);
  if (!compilation_test)
  {
    std::cerr << "Shader compilation failed with the following message: " << std::endl;
    std::vector<char> compilation_log(512, '\0');
    glGetShaderInfoLog(shader, (GLsizei) compilation_log.size(), NULL, &compilation_log[0]);
    std::cerr << &compilation_log[0] << std::endl;
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  return shader;
}

GLuint CreateProgramFromShader(const std::string &vertex_shader_path, const std::string &fragment_shader_path)
{
  GLuint vertex_shader = LoadAndCompileShader(vertex_shader_path, GL_VERTEX_SHADER);
  GLuint fragment_shader = LoadAndCompileShader(fragment_shader_path, GL_FRAGMENT_SHADER);

  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  glLinkProgram(shader_program);
  glUseProgram(shader_program);

  return shader_program;
}

GLuint CreateProgramFromShader(const std::string &vertex_shader_path,
                               const std::string &geometry_shader_path,
                               const std::string &fragment_shader_path)
{
  GLuint vertex_shader = LoadAndCompileShader(vertex_shader_path, GL_VERTEX_SHADER);
  GLuint geometry_shader = LoadAndCompileShader(geometry_shader_path, GL_GEOMETRY_SHADER);
  GLuint fragment_shader = LoadAndCompileShader(fragment_shader_path, GL_FRAGMENT_SHADER);

  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, geometry_shader);
  glAttachShader(shader_program, fragment_shader);

  glDeleteShader(vertex_shader);
  glDeleteShader(geometry_shader);
  glDeleteShader(fragment_shader);

  glLinkProgram(shader_program);
  glUseProgram(shader_program);

  return shader_program;
}

void RedBlueColors(GLfloat c, GLfloat &r, GLfloat &g, GLfloat &b)
{
  if (c >= red_blue_boundary)
  {
    r = 0.99f;//0.75f;
    g = 0.13f;//0.13f;
    b = 0.13f;//0.13f;
  } else
  {
    r = 0.0f;//0.13f;
    g = 0.5f;//0.13f;
    b = 0.99f;//0.75f;
  }
}

GLfloat ModWrap0Denom(GLfloat numerator, GLfloat denominator)
{
  return numerator - denominator * floorf(numerator / denominator);
}

// Restrict particle coordinates to the simulation box
void ApplyPeriodicBoundaryConditions(std::vector<float> &x)
{
  float x_size = kL, y_size = kL;
  float x_rsize = 1.0f / x_size, y_rsize = 1.0f / y_size;

#pragma unroll
  for (int i = 0; i < kN; ++i)
  {
    x[kS * i] -= std::floor(x[kS * i] * x_rsize) * x_size;
    x[kS * i + 1] -= std::floor(x[kS * i + 1] * y_rsize) * y_size;
  } // i
}

void FindColorMinMax(const std::string &color_file_name)
{
  std::vector<GLfloat> color_base(kN, 0.0f);
  std::pair<std::vector<GLfloat>::iterator, std::vector<GLfloat>::iterator> min_max;
  max_color = std::numeric_limits<GLfloat>::lowest();
  min_color = std::numeric_limits<GLfloat>::max();

  color_file.open(color_file_name, std::ios::binary | std::ios::in | std::ios::ate);
  assert(color_file.is_open());
  std::streampos size = color_file.tellg();
  color_file.seekg(0, std::ios::beg);
  GLfloat t = 0.0f;

  while (size > 0)
  {
    color_file.read((char *) &t, sizeof(GLfloat));
    color_file.read((char *) &color_base[0], kN * sizeof(GLfloat));
    size -= (1 + kN) * sizeof(GLfloat);

    std::for_each(color_base.begin(), color_base.end(),
                  [](GLfloat &c)
                  {
                    c = std::fabs(c);
                  });
    min_max = std::minmax_element(color_base.begin(), color_base.end());
    min_color = std::min(min_color, *min_max.first);
    max_color = std::max(max_color, *min_max.second);
  }
  color_file.close();

  std::cout << "min color = " << min_color << std::endl;
  std::cout << "max color = " << max_color << std::endl;
}