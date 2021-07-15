//
// Created by Nikita Kruk on 17.02.20.
//

#include "SecondOrderModel.hpp"

#include <string>
#include <cassert>
#include <iostream>

SecondOrderModel::SecondOrderModel() :
    Model(),
    number_of_particles_(1000),
    number_of_state_variables_(4),
    system_state_(number_of_particles_ * number_of_state_variables_),
    number_of_reduced_state_variables_(3),
    reduced_system_state_for_renderer_(number_of_particles_ * number_of_reduced_state_variables_),
    x_size_(1.0),
    y_size_(1.0)
{
  folder_ = "/Volumes/Kruk/spss/spssLangevinIntegration/parameter_scan_1000_sigma1/";
//  folder_ = "/Users/nikita/Documents/Projects/spss/spssLangevinIntegration/";
  file_name_ = folder_ + "v0_1_xi_0.1_sigma_1_rho_0.6_alpha_0.3_Dphi_0_N_1000_0_0.bin";
  data_file_.open(file_name_, std::ios::binary | std::ios::in);
  assert(data_file_.is_open());
}

SecondOrderModel::~SecondOrderModel()
{
  system_state_.clear();
  if (data_file_.is_open())
  {
    data_file_.close();
  }
}

void SecondOrderModel::ReadNewState(Real &t)
{
  std::vector<Real> vec(number_of_particles_ * number_of_state_variables_, 0.0f);
  data_file_.read((char *) &t, sizeof(Real));
  data_file_.read((char *) &vec[0], number_of_particles_ * number_of_state_variables_ * sizeof(Real));
  std::copy(vec.begin(), vec.end(), system_state_.begin());
  for (int i = 0; i < number_of_particles_; ++i)
  {
    int ii = i * number_of_state_variables_;
    reduced_system_state_for_renderer_[number_of_reduced_state_variables_ * i] = system_state_[ii];
    reduced_system_state_for_renderer_[number_of_reduced_state_variables_ * i + 1] = system_state_[ii + 1];
    reduced_system_state_for_renderer_[number_of_reduced_state_variables_ * i + 2] = system_state_[ii + 2];
//    reduced_system_state_for_renderer_[number_of_reduced_state_variables_ * i + 2] = (system_state_[ii + 3]);
  } // i
  std::cout << "t:" << t << std::endl;
}

void SecondOrderModel::SkipTimeUnits(int t, Real delta_t)
{
  data_file_.seekg(int(t / delta_t) * (1l + number_of_particles_ * number_of_state_variables_) * sizeof(Real),
                   std::ios::cur);
}

void SecondOrderModel::ReopenFile()
{
  data_file_.close();
  data_file_.clear();
  data_file_.open(file_name_, std::ios::binary | std::ios::in);
  assert(data_file_.is_open());
}

const std::vector<float> &SecondOrderModel::GetCurrentState()
{
//  return system_state_;
  return reduced_system_state_for_renderer_;
}

int SecondOrderModel::GetNumberOfParticles() const
{
  return number_of_particles_;
}

int SecondOrderModel::GetNumberOfStateVariables() const
{
//  return number_of_state_variables_;
  return number_of_reduced_state_variables_;
}

void SecondOrderModel::ApplyPeriodicBoundaryConditions()
{
  static const float phi_size = 2.0 * M_PI;
  static const float x_rsize = 1.0f / x_size_, y_rsize = 1.0f / y_size_, phi_rsize = 1.0f / phi_size;

#pragma unroll
  for (int i = 0; i < number_of_particles_; ++i)
  {
    int ii = i * number_of_state_variables_;
    system_state_[ii] -= std::floorf(system_state_[ii] * x_rsize) * x_size_;
    system_state_[ii + 1] -= std::floorf(system_state_[ii + 1] * y_rsize) * y_size_;
    system_state_[ii + 2] -= std::floor(system_state_[ii + 2] * phi_rsize) * phi_size;
  } // i

  for (int i = 0; i < number_of_particles_; ++i)
  {
    int ii = i * number_of_state_variables_;
    reduced_system_state_for_renderer_[number_of_reduced_state_variables_ * i] = system_state_[ii];
    reduced_system_state_for_renderer_[number_of_reduced_state_variables_ * i + 1] = system_state_[ii + 1];
    reduced_system_state_for_renderer_[number_of_reduced_state_variables_ * i + 2] = system_state_[ii + 2];
//    reduced_system_state_for_renderer_[number_of_reduced_state_variables_ * i + 2] = (system_state_[ii + 3]);
  } // i
}

Real SecondOrderModel::GetXSize() const
{
  return x_size_;
}

Real SecondOrderModel::GetYSize() const
{
  return y_size_;
}