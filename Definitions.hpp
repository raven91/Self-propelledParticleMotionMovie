//
// Created by Nikita Kruk on 17.02.20.
//

#ifndef SPPKURAMOTOWITHINERTIAMOVIE_DEFINITIONS_HPP
#define SPPKURAMOTOWITHINERTIAMOVIE_DEFINITIONS_HPP

#include <cmath>

typedef float Real; // type of the simulation data

enum class ParticleRepresentationType
{
  kPointMass, kStripe
};

inline double ModWrap0Denom(double numerator, double denominator)
{
  return numerator - denominator * std::floor(numerator / denominator);
}

inline double ClampWithinRange(double x, double x_min, double x_max)
{
  if (x < x_min)
  {
    return x_min;
  } else if (x > x_max)
  {
    return x_max;
  } else
  {
    return x;
  }
}

#endif //SPPKURAMOTOWITHINERTIAMOVIE_DEFINITIONS_HPP
