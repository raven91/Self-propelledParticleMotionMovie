//
// Created by Nikita Kruk on 17.02.20.
//

#include "FirstOrderModel.hpp"
#include "SecondOrderModel.hpp"
#include "Renderer.hpp"

int main()
{
  FirstOrderModel model;
//  SecondOrderModel model;
  Renderer renderer(&model);
  renderer.Start();

  return 0;
}