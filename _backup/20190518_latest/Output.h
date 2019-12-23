#ifndef _OUTPUT_H_
#define _OUTPUT_H_

#include "Common.h"

/*****************************ステップ出力関数(角度指定)[いきなり目標値]***************************/

void Output_Angle(double, double, double,
                  double, double, double,
                  double, double, double,
                  double, double, double,
                  int, double);

/*****************************ステップ出力関数(角度指定[徐々に目標値]***************************/

void Output_Step_Angle(double, double, double,
                  double, double, double,
                  double, double, double,
                  double, double, double,
                  int, double);

/**************************ステップ出力関数(足先座標指定)***********************/
void Output_Coordinate(double, double, double,
                       double, double, double,
                       double, double, double,
                       double, double, double,
                       double,
                       int, double);
#endif