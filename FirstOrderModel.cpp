//
// Created by Nikita Kruk on 17.02.20.
//

#include "FirstOrderModel.hpp"

#include <string>
#include <sstream>
#include <cassert>
#include <iostream>

FirstOrderModel::FirstOrderModel() :
    Model(),
    number_of_particles_(10000),
    number_of_state_variables_(3),
    system_state_(number_of_particles_ * number_of_state_variables_),
    x_size_(100),
    y_size_(100)
{
  folder_ = "/Users/nikita/Documents/Projects/spc2/spc2OdeIntegration/";
//  folder_ = "/Volumes/Kruk/spc2/spc2OdeIntegration/continued/";
  file_name_ = folder_ + "v0_1_sigma_1_rho_0.01_alpha_0_Dphi_0.01_N_" + std::to_string(number_of_particles_) + "_0_0.bin";
  file_name_ = folder_
      + "v0_1_sigma_1_rho_1_V0_0_Dphi_0.5_N_" + std::to_string(number_of_particles_) + "_rho0_0.5_0.bin";
  data_file_.open(file_name_, std::ios::binary | std::ios::in);
  assert(data_file_.is_open());
}

FirstOrderModel::~FirstOrderModel()
{
  system_state_.clear();
  if (data_file_.is_open())
  {
    data_file_.close();
  }
}

void FirstOrderModel::ReadNewState(Real &t)
{
  std::vector<Real> vec(number_of_particles_ * number_of_state_variables_, 0.0f);
  data_file_.read((char *) &t, sizeof(Real));
  data_file_.read((char *) &vec[0], number_of_particles_ * number_of_state_variables_ * sizeof(Real));
  std::copy(vec.begin(), vec.end(), system_state_.begin());
  std::cout << "t:" << t << std::endl;
}

void FirstOrderModel::SkipTimeUnits(int t, Real delta_t)
{
  data_file_.seekg(int(t / delta_t) * (1l + number_of_particles_ * number_of_state_variables_) * sizeof(Real),
                   std::ios::cur);
}

void FirstOrderModel::ReopenFile()
{
  data_file_.close();
  data_file_.clear();
  data_file_.open(file_name_, std::ios::binary | std::ios::in);
  assert(data_file_.is_open());
}

const std::vector<Real> &FirstOrderModel::GetCurrentState()
{
  return system_state_;
}

int FirstOrderModel::GetNumberOfParticles() const
{
  return number_of_particles_;
}

int FirstOrderModel::GetNumberOfStateVariables() const
{
  return number_of_state_variables_;
}

void FirstOrderModel::ApplyPeriodicBoundaryConditions()
{
  static const float x_rsize = 1.0f / x_size_, y_rsize = 1.0f / y_size_;

#pragma unroll
  for (int i = 0; i < number_of_particles_; ++i)
  {
    int ii = i * number_of_state_variables_;
    system_state_[ii] -= std::floorf(system_state_[ii] * x_rsize) * x_size_;
    system_state_[ii + 1] -= std::floorf(system_state_[ii + 1] * y_rsize) * y_size_;
  } // i
}

Real FirstOrderModel::GetXSize() const
{
  return x_size_;
}

Real FirstOrderModel::GetYSize() const
{
  return y_size_;
}