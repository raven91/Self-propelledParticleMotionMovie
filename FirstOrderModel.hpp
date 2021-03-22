//
// Created by Nikita Kruk on 17.02.20.
//

#ifndef SPPKURAMOTOWITHINERTIAMOVIE_FIRSTORDERMODEL_HPP
#define SPPKURAMOTOWITHINERTIAMOVIE_FIRSTORDERMODEL_HPP

#include "Definitions.hpp"
#include "Model.hpp"

#include <vector>
#include <fstream>
#include <string>

class FirstOrderModel : public Model
{
 public:

  FirstOrderModel();
  ~FirstOrderModel();

  virtual void ReadNewState(Real &t);
  virtual void SkipTimeUnits(int t, Real delta_t);
  virtual void ReopenFile();

  virtual const std::vector<Real> &GetCurrentState();
  virtual int GetNumberOfParticles() const;
  virtual int GetNumberOfStateVariables() const;
  virtual void ApplyPeriodicBoundaryConditions(Real box_size);

 private:

  std::ifstream data_file_;
  int number_of_particles_;
  int number_of_state_variables_;
  std::vector<Real> system_state_;
  std::string folder_;
  std::string file_name_;

};

#endif //SPPKURAMOTOWITHINERTIAMOVIE_FIRSTORDERMODEL_HPP
