//
// Created by Nikita Kruk on 17.02.20.
//

#include <cmath>

void Jet2Rgb(float j, float &r, float &g, float &b)
{
  float eps = 0.2f;
  j = (1.0f - 2.0 * eps) * j + eps;
//  j = (1.0f - eps) * j + eps;

  if (j <= 1.0f / 8.0f)
  {
    r = 0.0f;
    g = 0.0f;
    b = (1.0f - 9.0f / 16.0f) / (1.0f / 8.0f - 0.0f) * (j - 0.0f) + 9.0f / 16.0f;
  } else if (j <= 3.0f / 8.0f)
  {
    r = 0.0f;
    g = (1.0f - 0.0f) / (3.0f / 8.0f - 1.0f / 8.0f) * (j - 1.0f / 8.0f) + 0.0f;
    b = 1.0f;
  } else if (j <= 5.0f / 8.0f)
  {
    r = (1.0f - 0.0f) / (5.0f / 8.0f - 3.0f / 8.0f) * (j - 3.0f / 8.0f) + 0.0f;
    g = 1.0f;
    b = (0.0f - 1.0f) / (5.0f / 8.0f - 3.0f / 8.0f) * (j - 3.0f / 8.0f) + 1.0f;
  } else if (j <= 7.0f / 8.0f)
  {
    r = 1.0f;
    g = (0.0f - 1.0f) / (7.0f / 8.0f - 5.0f / 8.0f) * (j - 5.0f / 8.0f) + 1.0f;
    b = 0.0f;
  } else
  {
    r = (9.0f / 16.0f - 1.0f) / (1.0f - 7.0f / 8.0f) * (j - 7.0f / 8.0f) + 1.0f;
    g = 0.0f;
    b = 0.0f;
  }
}

void Hsv2Rgb(float h, float s, float v, float &r, float &g, float &b)
{
//	h = (tanhf(100.0f * (h - red_blue_boundary) + 2.0f)) / 4.0f;

//	h = 2.0f / 3.0f * (1.0f - h);// 240.0 / 360.0 * (1 - h);

  if (0 == std::floorf(s))
  {
    r = v;
    g = v;
    b = v;
  } else
  {
    float var_h = h * 6.0f;

    if (6 == std::floorf(var_h))
    {
      var_h = 0.0f;
    }
    float var_i = std::floorf(var_h);
    float var_1 = v * (1.0f - s);
    float var_2 = v * (1.0f - s * (var_h - var_i));
    float var_3 = v * (1.0f - s * (1.0f - (var_h - var_i)));
    float var_r = 0.0f;
    float var_g = 0.0f;
    float var_b = 0.0f;

    if (0 == std::floorf(var_i))
    {
      var_r = v;
      var_g = var_3;
      var_b = var_1;
    } else if (1 == std::floorf(var_i))
    {
      var_r = var_2;
      var_g = v;
      var_b = var_1;
    } else if (2 == std::floorf(var_i))
    {
      var_r = var_1;
      var_g = v;
      var_b = var_3;
    } else if (3 == std::floorf(var_i))
    {
      var_r = var_1;
      var_g = var_2;
      var_b = v;
    } else if (4 == std::floorf(var_i))
    {
      var_r = var_3;
      var_g = var_1;
      var_b = v;
    } else
    {
      var_r = v;
      var_g = var_1;
      var_b = var_2;
    }

    r = var_r;
    g = var_g;
    b = var_b;
  }

//	//black values for the test
//	r = 0.0;
//	g = 0.0;
//	b = 0.0;
}

void DoubleColoring(float c, float &r, float &g, float &b)
{
  if (c >= 0.4f)
  {
//    r = 0.99f; g = 0.13f; b = 0.13f;
//    r = 0.75f; g = 0.13f; b = 0.13f;
    r = 0.75f; g = 0.0f; b = 0.0f;
  }
  else
  {
//    r = 0.0f; g = 0.5f; b = 0.99f;
//    r = 0.13f; g = 0.13f; b = 0.75f;
    r = 0.0f; g = 1.0f; b = 1.0f;
  }
}