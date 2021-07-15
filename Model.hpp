//
// Created by Nikita Kruk on 18.02.20.
//

#ifndef SPPKURAMOTOWITHINERTIAMOVIE_MODEL_HPP
#define SPPKURAMOTOWITHINERTIAMOVIE_MODEL_HPP

#include "Definitions.hpp"

#include <vector>

class Model
{
 public:

  Model();
  virtual ~Model();

  virtual void ReadNewState(Real &t) = 0;
  virtual void SkipTimeUnits(int t, Real delta_t) = 0;
  virtual void ReopenFile() = 0;

  virtual const std::vector<Real> &GetCurrentState() = 0;
  virtual int GetNumberOfParticles() const = 0;
  virtual int GetNumberOfStateVariables() const = 0;
  virtual void ApplyPeriodicBoundaryConditions() = 0;
  virtual Real GetXSize() const = 0;
  virtual Real GetYSize() const = 0;

};

#endif //SPPKURAMOTOWITHINERTIAMOVIE_MODEL_HPP
