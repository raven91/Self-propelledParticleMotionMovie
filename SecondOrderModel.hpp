//
// Created by Nikita Kruk on 17.02.20.
//

#ifndef SPPKURAMOTOWITHINERTIAMOVIE_SECONDORDERMODEL_HPP
#define SPPKURAMOTOWITHINERTIAMOVIE_SECONDORDERMODEL_HPP

#include "Definitions.hpp"
#include "Model.hpp"

#include <vector>
#include <fstream>

class SecondOrderModel : public Model
{
 public:

  SecondOrderModel();
  ~SecondOrderModel();

  virtual void ReadNewState(Real &t);
  virtual void SkipTimeUnits(int t, Real delta_t);
  virtual void ReopenFile();

  virtual const std::vector<Real> &GetCurrentState();
  virtual int GetNumberOfParticles() const;
  virtual int GetNumberOfStateVariables() const;
  virtual void ApplyPeriodicBoundaryConditions();
  virtual Real GetXSize() const;
  virtual Real GetYSize() const;

 private:

  std::ifstream data_file_;
  int number_of_particles_;
  int number_of_state_variables_;
  std::vector<Real> system_state_;
  int number_of_reduced_state_variables_;
  std::vector<Real> reduced_system_state_for_renderer_;
  std::string folder_;
  std::string file_name_;
  Real x_size_;
  Real y_size_;

};

#endif //SPPKURAMOTOWITHINERTIAMOVIE_SECONDORDERMODEL_HPP
