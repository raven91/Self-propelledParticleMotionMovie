//
// Created by Nikita Kruk on 17.02.20.
//

#include "Renderer.hpp"

#include <iostream>
#include <sstream>
#include <list>
#include <numeric> // std::iota
#include <iomanip> // std::setprecision
#include <fstream>
#include <complex>

void TakeScreenshotPpm(int width, int height, int image_index);
void FreePpm();
void TakeScreenshotPng(unsigned int width, unsigned int height, int image_index);
void FreePng();
void Jet2Rgb(float j, float &r, float &g, float &b);
void Hsv2Rgb(float h, float s, float v, float &r, float &g, float &b);
void DoubleColoring(float c, float &r, float &g, float &b);

bool Renderer::stop_flag_ = true;
bool Renderer::pause_flag_ = true;
bool Renderer::take_screenshot_flag_ = false;
int Renderer::screenshot_count_ = 0;
int Renderer::frame_speed_ = 1; // 1 - the basic frame rate
GLfloat Renderer::x_shift_ = 0.0f;
GLfloat Renderer::y_shift_ = 0.0f;
GLfloat Renderer::z_scale_ = 1.0f;
int Renderer::t_start_ = 0;
bool Renderer::show_time_ = true;
Real Renderer::time_stamp_to_show_ = 0.0f;
int Renderer::number_of_points_per_particle_ = 1; // should be consistent with ParticleRepresentationType
int Renderer::number_of_color_components_ = 3;
ParticleRepresentationType Renderer::particle_representation_type_ = ParticleRepresentationType::kPointMass;

Renderer::Renderer(Model *model) :
    model_(model),
    system_state_over_time_interval_(),
    coloring_over_time_interval_(),
    ft_(),
    face_()
{
  std::cout << "Renderer Created" << std::endl;
  model_->SkipTimeUnits(t_start_, 1.0);
}

Renderer::~Renderer()
{
  std::cout << "Renderer Deleted" << std::endl;
}

void Renderer::Start()
{
  GLFWwindow *window;

  glfwSetErrorCallback(Renderer::ErrorCallback);

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
  glfwWindowHint(GLFW_SAMPLES, 4); // MSAA

  window = glfwCreateWindow(1280 / 2, 1280 / 2, "Particle Dynamics", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    std::cerr << "Window opening failure" << std::endl;
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glfwSetKeyCallback(window, Renderer::KeyCallback);

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
  glGenVertexArrays(3, &vao[0]);
  glGenBuffers(3, &vbo[0]);

  GLuint shader_program[3] = {0};
  InitShaders(shader_program);

  InitializeSystemState();
  while (!glfwWindowShouldClose(window))
  {
    glUseProgram(shader_program[1]); // shader parameters must be initialized when the respective shader is active
    SetShaderParameter(shader_program[1], x_shift_, "x_shift");
    SetShaderParameter(shader_program[1], y_shift_, "y_shift");
    SetShaderParameter(shader_program[1], z_scale_, "z_scale");
    DisplayFunc(window, vao, vbo, shader_program);

//		if (!stop_flag)
    if (take_screenshot_flag_)
    {
      int width, height;
      glfwGetFramebufferSize(window, &width, &height);
//			glfwGetWindowSize(window, &width, &height);
//			glReadBuffer(GL_BACK);
//			GLubyte *pixels = NULL;
//			TakeScreenshotPpm(width, height, screenshot_count++);
      TakeScreenshotPng(width, height, screenshot_count_++);
//			free(pixels);
      take_screenshot_flag_ = false;
    }
//    if (time_stamp_to_show_ >= 100.0f)
//    {
//      glfwSetWindowShouldClose(window, GL_TRUE);
//    }

    ReadNewState();
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  FinFunc();
  glfwDestroyWindow(window);
  glfwTerminate();
}

void Renderer::ErrorCallback(int error, const char *description)
{
  std::cerr << description << std::endl;
}

void Renderer::KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  if (GLFW_PRESS == action)
  {
    switch (key)
    {
      case GLFW_KEY_ESCAPE:
//			case GLFW_PRESS:
        glfwSetWindowShouldClose(window, GL_TRUE);
        break;

      case GLFW_KEY_S:stop_flag_ = !stop_flag_;
        break;

      case GLFW_KEY_P:
        if (stop_flag_)
        {
          pause_flag_ = !pause_flag_;
        }
        break;

      case GLFW_KEY_I:take_screenshot_flag_ = true;
        break;

      case GLFW_KEY_O:++frame_speed_;
        break;

      case GLFW_KEY_L:
        if (frame_speed_ > 1)
        {
          --frame_speed_;
        }
        break;

      case GLFW_KEY_T: show_time_ = !show_time_;
        break;

        /*case GLFW_KEY_LEFT:red_blue_boundary -= 0.01f;
          std::cout << "red_blue_boundary = " << red_blue_boundary << std::endl;
          break;

        case GLFW_KEY_RIGHT:red_blue_boundary += 0.01f;
          std::cout << "red_blue_boundary = " << red_blue_boundary << std::endl;
          break;*/

      case GLFW_KEY_1:x_shift_ += 0.1f;
        break;

      case GLFW_KEY_2:x_shift_ -= 0.1f;
        break;

      case GLFW_KEY_3:y_shift_ += 0.1f;
        break;

      case GLFW_KEY_4:y_shift_ -= 0.1f;
        break;

      case GLFW_KEY_5: z_scale_ /= 1.25f;
        break;

      case GLFW_KEY_6: z_scale_ *= 1.25f;
        break;

      default: break;
    }
  }
}

void Renderer::InitShaders(GLuint *shader_program)
{
  shader_program[0]
      = CreateProgramFromShader
      (std::string("/Users/nikita/CLionProjects/sppLangevinIntegrationMovie/Shaders/border_vertex_shader.shader"),
       std::string("/Users/nikita/CLionProjects/sppLangevinIntegrationMovie/Shaders/border_fragment_shader.shader"));
  if (particle_representation_type_ == ParticleRepresentationType::kPointMass)
  {
    shader_program[1]
        = CreateProgramFromShader
        (std::string("/Users/nikita/CLionProjects/sppLangevinIntegrationMovie/Shaders/particle_vertex_shader.shader"),
         std::string(
             "/Users/nikita/CLionProjects/sppLangevinIntegrationMovie/Shaders/particle_geometry_shader_point_mass.shader"),
         std::string(
             "/Users/nikita/CLionProjects/sppLangevinIntegrationMovie/Shaders/particle_fragment_shader_point_mass.shader"));
  } else if (particle_representation_type_ == ParticleRepresentationType::kStripe)
  {
    shader_program[1]
        = CreateProgramFromShader
        (std::string("/Users/nikita/CLionProjects/sppLangevinIntegrationMovie/Shaders/particle_vertex_shader.shader"),
         std::string("/Users/nikita/CLionProjects/sppLangevinIntegrationMovie/Shaders/particle_geometry_shader.shader"),
         std::string("/Users/nikita/CLionProjects/sppLangevinIntegrationMovie/Shaders/particle_fragment_shader.shader"));
  } else
  {
    std::cerr << "wrong particle representation type" << std::endl;
  }

  // shader parameters must be initialized when the respective shader is active
  SetShaderParameter(shader_program[1], (GLint) number_of_points_per_particle_, "kPointsPerParticle");
  SetShaderParameter(shader_program[1], (GLfloat) model_->GetXSize(), "x_size");
  SetShaderParameter(shader_program[1], (GLfloat) model_->GetYSize(), "y_size");
  SetShaderParameter(shader_program[1], (GLfloat) z_scale_, "z_scale");

  shader_program[2]
      = CreateProgramFromShader
      (std::string("/Users/nikita/CLionProjects/sppLangevinIntegrationMovie/Shaders/text_vertex_shader.shader"),
       std::string("/Users/nikita/CLionProjects/sppLangevinIntegrationMovie/Shaders/text_fragment_shader.shader"));
  if (FT_Init_FreeType(&ft_))
  {
    std::cerr << "Could not init freetype library" << std::endl;
  }
  if (FT_New_Face(ft_, "/System/Library/Fonts/HelveticaNeue.ttc", 0, &face_))
  {
    std::cerr << "Could not open font" << std::endl;
  }
  FT_Set_Pixel_Sizes(face_, 0, 20);//48
  GLuint texture;
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  GLint tex_location = glGetUniformLocation(shader_program[2], "tex");
  if (tex_location != -1)
  {
    glUniform1i(tex_location, 0);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void Renderer::SetShaderParameter(GLuint shader_program,
                                  GLint parameter_value,
                                  const std::string &parameter_name_in_shader)
{
  GLint location = glGetUniformLocation(shader_program, parameter_name_in_shader.c_str());
  if (-1 != location)
  {
    glUniform1i(location, parameter_value);
  }
}

void Renderer::SetShaderParameter(GLuint shader_program,
                                  GLfloat parameter_value,
                                  const std::string &parameter_name_in_shader)
{
  GLint location = glGetUniformLocation(shader_program, parameter_name_in_shader.c_str());
  if (-1 != location)
  {
    glUniform1f(location, parameter_value);
  }
}

void Renderer::FinFunc()
{
//  FreePpm();
  FreePng();
}

void Renderer::InitializeSystemState()
{
  /*std::vector<float> average_angular_velocity(model_->GetNumberOfParticles(), 0.0f);
  int counter = 0;
  std::vector<Real> prev, cur;
  model_->ReadNewState(time_stamp_to_show_);
  cur = model_->GetCurrentState();
  for (int n = 0; n < 1000 * 1; ++n)
  {
    prev = cur;
    model_->ReadNewState(time_stamp_to_show_);
    cur = model_->GetCurrentState();
    for (int i = 0; i < model_->GetNumberOfParticles(); ++i)
    {
      float phi1 = prev[model_->GetNumberOfStateVariables() * i + 2];
      float phi2 = cur[model_->GetNumberOfStateVariables() * i + 2];
      float diff = phi2 - phi1;
      diff -= (2.0 * M_PI) * std::nearbyint(diff / (2.0 * M_PI));
      average_angular_velocity[i] += diff / 0.005;
    } // i
    ++counter;
  } // n
  std::for_each(average_angular_velocity.begin(), average_angular_velocity.end(), [&](float &f) { f /= counter; });
  auto minmax = std::minmax_element(average_angular_velocity.begin(), average_angular_velocity.end());
  Real min = *minmax.first, max = *minmax.second;

  std::cout << "min angular velocity:" << *minmax.first << ", max angular velocity:" << *minmax.second << std::endl;
  std::for_each(average_angular_velocity.begin(),
                average_angular_velocity.end(),
                [&](float &f) { f = 1.0f - (f - min) / (max - min); });
  model_->ReopenFile();
  model_->SkipTimeUnits(t_start_, 1.0);*/

  for (int n = 0; n < number_of_points_per_particle_; ++n)
  {
    model_->ReadNewState(time_stamp_to_show_);
    model_->ApplyPeriodicBoundaryConditions();
    system_state_over_time_interval_.push_back(model_->GetCurrentState());

    /*std::vector<std::vector<int>> linked_list;
    ConstructAngularLinkedList(linked_list);
    std::sort(linked_list.begin(),
              linked_list.end(),
              [](const std::vector<int> &cell1, const std::vector<int> &cell2) { return cell1.size() > cell2.size(); });
    std::vector<float> fraction_per_cell(linked_list.size(), 0.0f);
    std::transform(linked_list.begin(),
                   linked_list.end(),
                   fraction_per_cell.begin(),
                   [](const std::vector<int> &cell) { return cell.size(); });
    auto minmax = std::minmax_element(fraction_per_cell.begin(), fraction_per_cell.end());
    float min = *minmax.first, max = *minmax.second;
    std::for_each(fraction_per_cell.begin(), fraction_per_cell.end(), [&](float &f) { f = (f - min) / (max - min); });*/
    /*float min_angular_velocity = MAXFLOAT, max_angular_velocity = -MAXFLOAT;
    for (int i = 0; i < model_->GetNumberOfParticles(); ++i)
    {
      float angular_velocity = system_state_over_time_interval_.back()[model_->GetNumberOfStateVariables() * i + 2];
      if (min_angular_velocity > angular_velocity)
      {
        min_angular_velocity = angular_velocity;
      }
      if (max_angular_velocity < angular_velocity)
      {
        max_angular_velocity = angular_velocity;
      }
    } // i
    for (int i = 0; i < model_->GetNumberOfParticles(); ++i)
    {
      system_state_over_time_interval_.back()[model_->GetNumberOfStateVariables() * i + 2] =
          (system_state_over_time_interval_.back()[model_->GetNumberOfStateVariables() * i + 2] - min_angular_velocity)
              / (max_angular_velocity - min_angular_velocity);
    } // i*/

    const std::vector<Real> &system_state = model_->GetCurrentState();
    std::vector<GLfloat> particle_coloring(number_of_color_components_ * model_->GetNumberOfParticles(), 0.0f);
    for (int i = 0; i < model_->GetNumberOfParticles(); ++i)
//    for (int i_cell = 0; i_cell < linked_list.size(); ++i_cell)
//      for (int i : linked_list[i_cell])
    {
      static const float two_pi = 2.0f * M_PI;
      Hsv2Rgb(ModWrap0Denom(system_state[model_->GetNumberOfStateVariables() * i + 2], two_pi) / two_pi,
              1.0f,
              1.0f,
              particle_coloring[number_of_color_components_ * i],
              particle_coloring[number_of_color_components_ * i + 1],
              particle_coloring[number_of_color_components_ * i + 2]);
      /*DoubleColoring(average_angular_velocity[i],
                     particle_coloring[number_of_color_components_ * i],
                     particle_coloring[number_of_color_components_ * i + 1],
                     particle_coloring[number_of_color_components_ * i + 2]);*/
      /*Jet2Rgb(fraction_per_cell[i_cell],
//            i_cell / float(linked_list.size()),
          particle_coloring[number_of_color_components_ * i],
          particle_coloring[number_of_color_components_ * i + 1],
          particle_coloring[number_of_color_components_ * i + 2]);*/
      /*Jet2Rgb(system_state_over_time_interval_.back()[model_->GetNumberOfStateVariables() * i + 2],
              particle_coloring[number_of_color_components_ * i],
              particle_coloring[number_of_color_components_ * i + 1],
              particle_coloring[number_of_color_components_ * i + 2]);*/
    } // i
    coloring_over_time_interval_.push_back(particle_coloring);
  } // n
}

void Renderer::ReadNewState()
{
  if (!stop_flag_ || !pause_flag_)
  {
    model_->ReadNewState(time_stamp_to_show_);
    model_->ApplyPeriodicBoundaryConditions();
    system_state_over_time_interval_.push_back(model_->GetCurrentState());
    system_state_over_time_interval_.pop_front();

    /*std::vector<std::vector<int>> linked_list;
    ConstructAngularLinkedList(linked_list);
    std::sort(linked_list.begin(),
              linked_list.end(),
              [](const std::vector<int> &cell1, const std::vector<int> &cell2) { return cell1.size() > cell2.size(); });
    std::vector<float> fraction_per_cell(linked_list.size(), 0.0f);
    std::transform(linked_list.begin(),
                   linked_list.end(),
                   fraction_per_cell.begin(),
                   [](const std::vector<int> &cell) { return cell.size(); });
    auto minmax = std::minmax_element(fraction_per_cell.begin(), fraction_per_cell.end());
    float min = *minmax.first, max = *minmax.second;
    std::for_each(fraction_per_cell.begin(), fraction_per_cell.end(), [&](float &f) { f = (f - min) / (max - min); });*/
    /*for (int i = 0; i < model_->GetNumberOfParticles(); ++i)
    {
      system_state_over_time_interval_.back()[model_->GetNumberOfStateVariables() * i + 2] = ClampWithinRange(
          system_state_over_time_interval_.back()[model_->GetNumberOfStateVariables() * i + 2],
          -7.0,
          0.0);
    } // i*/
    /*float min_angular_velocity = MAXFLOAT, max_angular_velocity = -MAXFLOAT;
    for (int i = 0; i < model_->GetNumberOfParticles(); ++i)
    {
      float angular_velocity = system_state_over_time_interval_.back()[model_->GetNumberOfStateVariables() * i + 2];
      if (min_angular_velocity > angular_velocity)
      {
        min_angular_velocity = angular_velocity;
      }
      if (max_angular_velocity < angular_velocity)
      {
        max_angular_velocity = angular_velocity;
      }
    } // i
    for (int i = 0; i < model_->GetNumberOfParticles(); ++i)
    {
      system_state_over_time_interval_.back()[model_->GetNumberOfStateVariables() * i + 2] =
          (system_state_over_time_interval_.back()[model_->GetNumberOfStateVariables() * i + 2] - min_angular_velocity)
              / (max_angular_velocity - min_angular_velocity);
    } // i*/

    const std::vector<Real> &system_state = model_->GetCurrentState();
    std::vector<GLfloat> particle_coloring(number_of_color_components_ * model_->GetNumberOfParticles(), 0.0f);
    for (int i = 0; i < model_->GetNumberOfParticles(); ++i)
//    for (int i_cell = 0; i_cell < linked_list.size(); ++i_cell)
//      for (int i : linked_list[i_cell])
    {
      static const float two_pi = 2.0f * M_PI;
      Hsv2Rgb(ModWrap0Denom(system_state[model_->GetNumberOfStateVariables() * i + 2], two_pi) / two_pi,
              1.0f,
              1.0f,
              particle_coloring[number_of_color_components_ * i],
              particle_coloring[number_of_color_components_ * i + 1],
              particle_coloring[number_of_color_components_ * i + 2]);
      /*Jet2Rgb(fraction_per_cell[i_cell],
//            i_cell / float(linked_list.size()),
          particle_coloring[number_of_color_components_ * i],
          particle_coloring[number_of_color_components_ * i + 1],
          particle_coloring[number_of_color_components_ * i + 2]);*/
      /*Jet2Rgb(system_state_over_time_interval_.back()[model_->GetNumberOfStateVariables() * i + 2],
              particle_coloring[number_of_color_components_ * i],
              particle_coloring[number_of_color_components_ * i + 1],
              particle_coloring[number_of_color_components_ * i + 2]);*/
    } // i
    coloring_over_time_interval_.push_back(particle_coloring);
    coloring_over_time_interval_.pop_front();

    // speed up or down the simulation output
    model_->SkipTimeUnits(frame_speed_ - 1, 1);
    pause_flag_ = true;
  }
}

void Renderer::DetermineAverageAngularVelocity()
{
  std::vector<float> average_angular_velocity(model_->GetNumberOfParticles(), 0.0f);
  for (int i = 0; i < model_->GetNumberOfParticles(); ++i)
  {
    std::vector<float> angular_velocity;
    for (auto it = system_state_over_time_interval_.begin(); it != std::prev(system_state_over_time_interval_.end());
         ++it)
    {
      float phi1 = (*it)[model_->GetNumberOfStateVariables() * i + 2];
      float phi2 = (*std::next(it))[model_->GetNumberOfStateVariables() * i + 2];
      float diff = phi2 - phi1;
      diff -= (2.0 * M_PI) * std::nearbyint(diff / (2.0 * M_PI));
      angular_velocity.push_back(diff / 0.005);
    } // it
    average_angular_velocity[i] =
        std::accumulate(angular_velocity.begin(), angular_velocity.end(), 0.0f) / angular_velocity.size();
  } // i
  auto minmax = std::minmax_element(average_angular_velocity.begin(), average_angular_velocity.end());
  std::for_each(average_angular_velocity.begin(),
                average_angular_velocity.end(),
                [&](float &f) { f = (f - *minmax.first) / (*minmax.second - *minmax.first); });
  for (int n = 0; n < number_of_points_per_particle_; ++n)
  {
    std::vector<GLfloat> particle_coloring(number_of_color_components_ * model_->GetNumberOfParticles(), 0.0f);
    for (int i = 0; i < model_->GetNumberOfParticles(); ++i)
    {
      Jet2Rgb(average_angular_velocity[i],
//              1.0f,
//              1.0f,
              particle_coloring[number_of_color_components_ * i],
              particle_coloring[number_of_color_components_ * i + 1],
              particle_coloring[number_of_color_components_ * i + 2]);
    } // i
    coloring_over_time_interval_.pop_front();
    coloring_over_time_interval_.push_back(particle_coloring);
  } // n
}

void Renderer::DisplayFunc(GLFWwindow *window, GLuint *vao, GLuint *vbo, GLuint *shader_program)
{
  glEnable(GL_BLEND);
//	glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
//	glBlendEquation(GL_FUNC_ADD);
//	glBlendEquation(GL_MAX);

  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_MULTISAMPLE); // MSAA

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glShadeModel(GL_SMOOTH);

  glUseProgram(shader_program[0]);
  RenderBorder(vao[0], vbo[0], shader_program[0]);

  glUseProgram(shader_program[1]);
  RenderParticles(vao[1], vbo[1], shader_program[1]);

  if (show_time_)
  {
    glUseProgram(shader_program[2]);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    GLfloat sx = 2.0f / GLfloat(width);
    GLfloat sy = 2.0f / GLfloat(height);

    std::ostringstream buffer;
    buffer << std::fixed << std::setprecision(2) << "t = " << time_stamp_to_show_;
    RenderText(buffer.str(), -1.0f + 8.0f * sx, 1.0f - 25.0f * sy, sx, sy, vao[2], vbo[2], shader_program[2]);//50.0f
  }
}

void Renderer::RenderBorder(GLuint vao, GLuint vbo, GLuint shader_program)
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

void Renderer::ConstructAngularLinkedList(std::vector<std::vector<int>> &linked_list)
{
  int linked_list_size = 100;
  linked_list = std::vector<std::vector<int>>(linked_list_size, std::vector<int>());
  for (int i = 0; i < model_->GetNumberOfParticles(); ++i)
  {
    float phi = system_state_over_time_interval_.back()[model_->GetNumberOfStateVariables() * i + 2];
    phi = ModWrap0Denom(phi, 2.0 * M_PI);
    int i_cell = int(phi / (2.0 * M_PI) * linked_list_size);
    linked_list[i_cell].push_back(i);
  } // i
}

void Renderer::RenderParticles(GLuint vao, GLuint vbo, GLuint shader_program)
{
  int number_of_state_variables = model_->GetNumberOfStateVariables();
  int number_of_particles = model_->GetNumberOfParticles();
  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER,
               number_of_points_per_particle_ * number_of_state_variables * number_of_particles * sizeof(GLfloat)
                   + number_of_points_per_particle_ * number_of_color_components_ * number_of_particles
                       * sizeof(GLfloat)
                   + number_of_points_per_particle_ * number_of_particles * sizeof(GLint),
               NULL,
               GL_DYNAMIC_DRAW);

  // send position data
  for (size_t i = 0; i < number_of_particles; ++i)
  {
    size_t n = 0;
//    for (std::list<std::vector<GLfloat>>::iterator it_x = system_state_over_time_interval_.begin();
//         it_x != system_state_over_time_interval_.end(); ++it_x)
    for (const std::vector<GLfloat> &system_state_at_time_point : system_state_over_time_interval_)
    {
      glBufferSubData(GL_ARRAY_BUFFER,
                      i * number_of_points_per_particle_ * number_of_state_variables * sizeof(GLfloat)
                          + n * number_of_state_variables * sizeof(GLfloat),
                      number_of_state_variables * sizeof(GLfloat),
                      &(system_state_at_time_point[number_of_state_variables * i]));
      ++n;
    } // system_state_at_time_point
  } // i

  // send color data
  for (size_t i = 0; i < number_of_particles; ++i)
  {
    size_t n = 0;
//    for (std::list<std::vector<GLfloat>>::iterator it_color = coloring_over_time_interval_.begin();
//         it_color != coloring_over_time_interval_.end(); ++it_color)
    for (const std::vector<GLfloat> &coloring_at_time_point : coloring_over_time_interval_)
    {
      glBufferSubData(GL_ARRAY_BUFFER,
                      number_of_points_per_particle_ * number_of_state_variables * number_of_particles * sizeof(GLfloat)
                          + i * number_of_points_per_particle_ * number_of_color_components_ * sizeof(GLfloat)
                          + n * number_of_color_components_ * sizeof(GLfloat),
                      number_of_color_components_ * sizeof(GLfloat),
                      &(coloring_at_time_point[number_of_color_components_ * i]));
      ++n;
    } // it_color
  } // i

  // send trajectorial data (used for transparency)
  std::vector<GLint> trajectory_indexes(number_of_points_per_particle_, 0);
  std::iota(trajectory_indexes.begin(), trajectory_indexes.end(), 0);
  for (size_t i = 0; i < number_of_particles; ++i)
  {
    glBufferSubData(GL_ARRAY_BUFFER,
                    number_of_points_per_particle_ * number_of_state_variables * number_of_particles * sizeof(GLfloat)
                        + number_of_points_per_particle_ * number_of_color_components_ * number_of_particles
                            * sizeof(GLfloat)
                        + i * number_of_points_per_particle_ * sizeof(GLint),
                    number_of_points_per_particle_ * sizeof(GLint),
                    &trajectory_indexes[0]);
  } // i

  GLint position_attribute = glGetAttribLocation(shader_program, "position");
  glVertexAttribPointer(position_attribute, 2, GL_FLOAT, GL_FALSE, number_of_state_variables * sizeof(GLfloat), 0);
  glEnableVertexAttribArray(position_attribute);

  GLint color_attribute = glGetAttribLocation(shader_program, "color_into_vertex_shader");
  glVertexAttribPointer(color_attribute,
                        number_of_color_components_,
                        GL_FLOAT,
                        GL_FALSE,
                        number_of_color_components_ * sizeof(GLfloat),
                        (GLvoid *) (number_of_points_per_particle_ * number_of_state_variables
                            * number_of_particles * sizeof(GLfloat)));
  glEnableVertexAttribArray(color_attribute);

  GLint trajectory_index_attribute = glGetAttribLocation(shader_program, "trajectory_index");
  glVertexAttribIPointer(trajectory_index_attribute,
                         1,
                         GL_INT,
                         sizeof(GLint),
                         (GLvoid *) (number_of_points_per_particle_ * number_of_state_variables * number_of_particles
                             * sizeof(GLfloat)
                             + number_of_points_per_particle_ * number_of_color_components_ * number_of_particles
                                 * sizeof(GLfloat)));
  glEnableVertexAttribArray(trajectory_index_attribute);

  /*std::vector<std::vector<int>> linked_list;
  ConstructAngularLinkedList(linked_list);
  std::sort(linked_list.begin(),
            linked_list.end(),
            [](const std::vector<int> &cell1, const std::vector<int> &cell2) { return cell1.size() > cell2.size(); });
  std::vector<int> sorted_indices;
  for (const std::vector<int> &cell : linked_list)
  {
    sorted_indices.insert(sorted_indices.end(), cell.begin(), cell.end());
  } // cell*/
  std::vector<int> sorted_indices(number_of_particles, 0);
  std::iota(sorted_indices.begin(), sorted_indices.end(), 0);
  std::sort(sorted_indices.begin(),
            sorted_indices.end(),
            [&](int i, int j)
            {
              return system_state_over_time_interval_.back()[number_of_state_variables * i + 2]
                  < system_state_over_time_interval_.back()[number_of_state_variables * j + 2];
            });

  glBindVertexArray(vao);
  for (int i = 0; i < number_of_particles; ++i)
//  for (int i : sorted_indices)
  {
    if (particle_representation_type_ == ParticleRepresentationType::kPointMass)
    {
      glDrawArrays(GL_POINTS, i * number_of_points_per_particle_, number_of_points_per_particle_);
    } else if (particle_representation_type_ == ParticleRepresentationType::kStripe)
    {
      glDrawArrays(GL_LINE_STRIP, i * number_of_points_per_particle_, number_of_points_per_particle_);
    }
  } // i

  glDisableVertexAttribArray(position_attribute);
  glDisableVertexAttribArray(color_attribute);
  glDisableVertexAttribArray(trajectory_index_attribute);

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
   */
}

void Renderer::RenderText(const std::string &text,
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

  FT_GlyphSlot glyph_slot = face_->glyph;

  for (char letter : text)
  {
    if (FT_Load_Char(face_, letter, FT_LOAD_RENDER))
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
  } // i
  glDisableVertexAttribArray(coord_location);
}

void Renderer::ReadShaderSource(const std::string &fname, std::vector<char> &buffer)
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

GLuint Renderer::LoadAndCompileShader(const std::string &fname, GLenum shader_type)
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

GLuint Renderer::CreateProgramFromShader(const std::string &vertex_shader_path, const std::string &fragment_shader_path)
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

GLuint Renderer::CreateProgramFromShader(const std::string &vertex_shader_path,
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