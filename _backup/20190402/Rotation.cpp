#include "Output.h"
#include "Common.h"

/**************************回転(角度で適当にやってるやつ)**********************************/
void Rotation_Deg(double x0, double y0, double all_height,
                  double rotation_angle, int rotation_times) //rotation_angleは回転角度[deg]
{
  Output_Angle(rf.s0.angle_old - 0.5 * rotation_angle, rf.s1.angle_old, rf.s2.angle_old,
               rb.s0.angle_old - 0.5 * rotation_angle, rb.s1.angle_old, rb.s2.angle_old,
               lb.s0.angle_old - 0.5 * rotation_angle, lb.s1.angle_old, lb.s2.angle_old,
               lf.s0.angle_old - 0.5 * rotation_angle, lf.s1.angle_old, lf.s2.angle_old,
               10, 0.02);

  for (int i(0); i < rotation_times; i++)
  {
    double height_angle(45);

    Output_Angle(rf.s0.angle_old + 1 * rotation_angle, rf.s1.angle_old, rf.s2.angle_old,
                 rb.s0.angle_old + 1 * rotation_angle, rb.s1.angle_old - height_angle, rb.s2.angle_old,
                 lb.s0.angle_old + 1 * rotation_angle, lb.s1.angle_old, lb.s2.angle_old,
                 lf.s0.angle_old + 1 * rotation_angle, lf.s1.angle_old - height_angle, lf.s2.angle_old,
                 10, 0.02);
    wait(0.5);

    Output_Angle(rf.s0.angle_old, rf.s1.angle_old, rf.s2.angle_old,
                 rb.s0.angle_old, rb.s1.angle_old + height_angle, rb.s2.angle_old,
                 lb.s0.angle_old, lb.s1.angle_old, lb.s2.angle_old,
                 lf.s0.angle_old, lf.s1.angle_old + height_angle, lf.s2.angle_old,
                 10, 0.02);
    wait(0.5);

    Output_Angle(rf.s0.angle_old - 1 * rotation_angle, rf.s1.angle_old - height_angle, rf.s2.angle_old,
                 rb.s0.angle_old - 1 * rotation_angle, rb.s1.angle_old, rb.s2.angle_old,
                 lb.s0.angle_old - 1 * rotation_angle, lb.s1.angle_old - height_angle, lb.s2.angle_old,
                 lf.s0.angle_old - 1 * rotation_angle, lf.s1.angle_old, lf.s2.angle_old,
                 10, 0.02);
    wait(0.5);

    Output_Angle(rf.s0.angle_old, rf.s1.angle_old + height_angle, rf.s2.angle_old,
                 rb.s0.angle_old, rb.s1.angle_old, rb.s2.angle_old,
                 lb.s0.angle_old, lb.s1.angle_old + height_angle, lb.s2.angle_old,
                 lf.s0.angle_old, lf.s1.angle_old, lf.s2.angle_old,
                 10, 0.02);
    wait(0.5);
  }
  Output_Angle(rf.s0.angle_old + 0.5 * rotation_angle, rf.s1.angle_old, rf.s2.angle_old,
               rb.s0.angle_old + 0.5 * rotation_angle, rb.s1.angle_old, rb.s2.angle_old,
               lb.s0.angle_old + 0.5 * rotation_angle, lb.s1.angle_old, lb.s2.angle_old,
               lf.s0.angle_old + 0.5 * rotation_angle, lf.s1.angle_old, lf.s2.angle_old,
               10, 0.02);
}

/*********************************座標指定で回転**********************************/
static double rotpos[2] = {0, 0};
void Rotate_Coordinate(double x, double y, double z0, double z1, double z2, double z3,
                       double rotangle, double all_height, double step_times, double wait_time)
{
  double temp = 0.1901 / 2; // 胴体の幅の半分[m]
  double firstangle = atan(y / x);
  if (firstangle < 0)
  {
    if (x < 0)
      firstangle += PI;
  }
  double length = sqrt((x + temp) * (x + temp) + (y + temp) * (y + temp));
  for (int i(1); i <= step_times + 1; i++)
  {
    double deltaangle = rotangle * PI / 180 * i / (step_times + 1);
    x = length * cos(firstangle + deltaangle) - temp;
    y = length * sin(firstangle + deltaangle) - temp;
    Output_Coordinate(x, y, z0, x, y, z1, x, y, z2, x, y, z3, all_height, 0, wait_time);
  }
  rotpos[0] = x, rotpos[1] = y;
}

void Rotation(double x0, double y0, double all_height, double leg_height,
              double rotation_angle, int rotation_times, double wait_time) //rotation_angleは回転角度[deg]
{
  double delta_angle = rotation_angle / rotation_times;
  int steptimes = 2;
  Output_Coordinate(x0, y0, leg_height,
                    x0, y0, 0,
                    x0, y0, leg_height,
                    x0, y0, 0,
                    all_height, steptimes, wait_time); // 右前左後ろup
  Rotate_Coordinate(x0, y0, leg_height, 0, leg_height, 0,
                    delta_angle / 2, all_height, rotation_times, wait_time);
  Output_Coordinate(rotpos[0], rotpos[1], 0,
                    rotpos[0], rotpos[1], 0,
                    rotpos[0], rotpos[1], 0,
                    rotpos[0], rotpos[1], 0,
                    all_height, steptimes, wait_time);
  Output_Coordinate(rotpos[0], rotpos[1], 0,
                    rotpos[0], rotpos[1], leg_height,
                    rotpos[0], rotpos[1], 0,
                    rotpos[0], rotpos[1], leg_height,
                    all_height, steptimes, wait_time); // 左前右後ろup
  for (int i = 0; i < rotation_times - 1; i++)
  {
    Rotate_Coordinate(rotpos[0], rotpos[1], 0, leg_height, 0, leg_height,
                      -delta_angle, all_height, rotation_times, wait_time);
    Output_Coordinate(rotpos[0], rotpos[1], 0,
                      rotpos[0], rotpos[1], 0,
                      rotpos[0], rotpos[1], 0,
                      rotpos[0], rotpos[1], 0,
                      all_height, steptimes, wait_time);
    Output_Coordinate(rotpos[0], rotpos[1], leg_height,
                      rotpos[0], rotpos[1], 0,
                      rotpos[0], rotpos[1], leg_height,
                      rotpos[0], rotpos[1], 0,
                      all_height, steptimes, wait_time); // 右前左後ろup
    Rotate_Coordinate(rotpos[0], rotpos[1], leg_height, 0, leg_height, 0,
                      delta_angle, all_height, rotation_times, wait_time);
    Output_Coordinate(rotpos[0], rotpos[1], 0,
                      rotpos[0], rotpos[1], 0,
                      rotpos[0], rotpos[1], 0,
                      rotpos[0], rotpos[1], 0,
                      all_height, steptimes, wait_time);
    Output_Coordinate(rotpos[0], rotpos[1], 0,
                      rotpos[0], rotpos[1], leg_height,
                      rotpos[0], rotpos[1], 0,
                      rotpos[0], rotpos[1], leg_height,
                      all_height, steptimes, wait_time); // 左前右後ろup
  }
  Rotate_Coordinate(rotpos[0], rotpos[1], 0, leg_height, 0, leg_height,
                    -delta_angle / 2, all_height, rotation_times, wait_time);
  Output_Coordinate(rotpos[0], rotpos[1], 0,
                    rotpos[0], rotpos[1], 0,
                    rotpos[0], rotpos[1], 0,
                    rotpos[0], rotpos[1], 0,
                    all_height, steptimes, wait_time);
}