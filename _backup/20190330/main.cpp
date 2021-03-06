#include "Common.h"
#include "Creeping.h"
#include "Output.h"
#include "Rotation.h"
#include "Trot.h"
#include "Function.h"
#include "SO1602A.h"
#include "PCA9685.h"
#include "VL53L0X.h"
#include "Mode_select.h"
#include "Distance_sensor.h"
#include "LidarLitev2.h"

/************************各種ピン設定***********************/
Serial pc(USBTX, USBRX, 115200); //PCとのシリアル通信
DigitalIn UB(USER_BUTTON);       //マイコン上のボタン
I2C i2c_1(PB_7, PB_8);
BNO055 bno(PC_12, PB_10);       //BNO055
AnalogIn distance_sensor(PC_1); //距離センサ
SO1602A oled(i2c_1);            //OLEDディスプレイ
LidarLitev2 lidar(PB_7, PB_8);

Timer dt;

#ifndef ODE
PCA9685 pwm(PB_7, PB_8); //サーボドライバ
#else
PCA9685 pwm;
#endif

Timer timer_vl[2]; //VL53L0x
VL53L0X vl[2] = {VL53L0X(&i2c_1, &timer_vl[0]), VL53L0X(&i2c_1, &timer_vl[1])};
DigitalInOut Xshut[2] = {DigitalInOut(PC_8), DigitalInOut(PC_6)};

DigitalIn button_white(PC_7);      //白ボタン
DigitalIn button_red(PA_6);        //赤ボタン
DigitalIn button_blue_left(PA_7);  //青_1ボタン(左下)
DigitalIn button_blue_right(PB_6); //青_2ボタン(右下)

PwmOut speaker(PC_7); //スピーカー

/*****************グローバル変数****************/
double roll, pitch, yaw;
double roll_offset, pitch_offset, yaw_offset;
double all_height_old;
int mode(0);            //0なら普通の歩行、1なら全体の流れ、2ならSlope
bool yaw_mode(1);       //0ならyaw角補正なし、1ならyaw角補正あり
bool lidarlite_mode(1); //lidarliteで測距する場合は1
int pc_debug(1);        //0ならprintfなし、1ならprintfあり、3ならグラフ化用の出力
int oled_debug(1);
int vl_output[2];      //[mm]
double vl_distance[2]; //[m]
double yaw_ref(0.0);   //Yaw角の目標値(機体の向き)
double delta_yaw(0.0);
double x_now(0.0); //機体のx座標[m]
double y_now(0.0); //機体のy座標[m]

/****************インスタンス化*********************/
Servo rf_servo[3], rb_servo[3], lb_servo[3], lf_servo[3];

Leg rf(rf_servo[0], rf_servo[1], rf_servo[2]);
Leg rb(rb_servo[0], rb_servo[1], rb_servo[2]);
Leg lb(lb_servo[0], lb_servo[1], lb_servo[2]);
Leg lf(lf_servo[0], lf_servo[1], lf_servo[2]);

/****************タイマー割り込み*****************/
//Ticker timer_oled;
//Ticker timer_gyro;

/*************************************メイン関数***************************************/
#ifdef ODE
void main2()
#else
int main()
#endif
{
  /***********初期化**************/
  pc.baud(9600);                    //シリアル通信初期設定
  initServoDriver();                //サーボドライバ初期化
  bno.setmode(OPERATION_MODE_NDOF); //BNO055初期化
  lidar.configure();                //Lidarlite初期化
  while (1)
  {
    //oled.clear();
    pc.printf("Lidar: %d cm\n", lidar.distance());
    wait(0.1);
  }
  //VL53L0X_init(); //測距センサ初期化

  oled.init(); //OLEDディスプレイ初期化
  //i2c_1.frequency(300000); //I2C 200kHz
  wait(0.1);

  /***************起動**************/
  oled.printf("    Hello!! \n   KRA Ilias");
  pc.printf("Initialized\n");
  wait(1.0);
  //timer_oled.attach(&OLED_display, 0.5);
  //timer_gyro.attach(&Get_gyro, 0.01);

  /************モード選択*************/
#ifndef ODE
  Mode_select();
#endif

  /******************各種設定*********************/
  oled.clear();
  oled.printf("Program Start!");

  Set_Angle_Offset(82, 117, 108,
                   133, 127, 102,
                   61, 102, 105,
                   153, 135, 125);

  Set_Angle_Min(-100, -120, -120,
                -100, -120, -120,
                -100, -120, -120,
                -100, -120, -120); //最小角度設定

  Set_Angle_Max(100, 120, 120,
                100, 120, 120,
                100, 120, 120,
                100, 120, 120); //最大角度設定

  Set_Gyro_Offset(); //ジャイロのオフセット設定

  /***********************************************************/

  //Offset_Position();

  //Wait_UB();

  if (mode == 0 || mode == 1 || mode == 3) //普通の歩行またはslope
  {
    pc.printf("mode=1\n");
#ifndef ODE
    /*while (Distance() > 20 || Distance() < 10)
    {
      if (pc_debug == 4)
      {
        pc.printf("Please hold out your hand on the sensor.\n");
      }
      wait(0.01);
    }*/
#endif
    if (pc_debug == 1)
    {
      pc.printf("Start Walking\n");
    }
    Body_Up(0.17, 0.15, 0.040, 0.28);
    wait(1);

    oled.clear();
    oled.printf("Ready!");

    Creeping(0.17, 0.15, 0.030, 0.28,
             0.40, 0.20,
             5, 0.01,
             15);
  }

  if (mode == 2) //坂登り
  {
#ifndef ODE
    while (Distance() > 20 || Distance() < 10)
    {
      if (pc_debug == 3)
      {
        pc.printf("Please hold out your hand on the sensor.\n");
      }
      wait(0.01);
    }
#endif
    pc.printf("Start Climbing\n");

    Body_Up(0.15, 0.25, 0.020, 0.30);
    //Body_Up_Dainozi(0.15, 0.15, 0.02, 0.30);

    Creeping(0.15, 0.25, 0.020, 0.30,
             0.40, 0.20,
             7, 0.01,
             2); //坂登りのダメダメパラメータ(坂を登り終えたらbreakする)

    pc.printf("Finish climbing\n");

    /*坂登り終了*/

    /*yaw角から回転角を正確に求める場合*/
    /*bno.get_angles();
      yaw = bno.euler.yaw - yaw_offset;
      double rotate_angle = yaw - initial_yaw;*/
    /*
    Output_Coordinate(0.15, 0.15, 0,
                      0.15, 0.15, 0,
                      0.15, 0.15, 0,
                      0.15, 0.15, 0,
                      0.30,
                      20, 0.05);

    */

    Rotation_Deg(0.15, 0.15, 0.30,
                 45, 4); //90度回転
    wait(1);
  }

  if (mode == 4)
  {
    Wait_UB();

    Body_Up(0.15, 0.25, 0.020, 0.30);

    Output_Coordinate(0.15, 0.15, 0,
                      0.15, 0.15, 0,
                      0.15, -0.02, 0,
                      0.15, -0.02, 0,
                      0.29,
                      10, 0.01);

    Wait_UB();

    Output_Coordinate(0.15, 0.15, 0,
                      0.15, 0.15, 0,
                      0.15, -0.02, 0.29,
                      0.15, -0.02, 0,
                      0.30,
                      10, 0.1);

    Output_Coordinate(0.15, 0.15, 0,
                      0.15, 0.15, 0,
                      0.15, 0.15, 0.3,
                      0.15, -0.02, 0,
                      0.30,
                      10, 0.1);

    Output_Coordinate(0.15, 0.15, 0,
                      0.15, 0.15, 0,
                      0.15, 0.15, 0.3,
                      0.15, -0.02, 0,
                      0.30,
                      10, 0.1);
  }

  if (mode == 6)
  { // ずっと立っているだけのモード
    Output_Coordinate(0.15, 0.15, 0.3,
                      0.15, 0.15, 0.3,
                      0.15, 0.15, 0.3,
                      0.15, 0.15, 0.3,
                      0.00,
                      0, 0.05);
    Output_Coordinate(0.20, 0.20, 0,
                      0.15, 0.15, 0,
                      0.15, 0.15, 0,
                      0.15, 0.15, 0,
                      0.30,
                      20, 0.05);
    wait(1);
    //bno.setmode(OPERATION_MODE_NDOF);
    Set_Gyro_Offset(); //ジャイロのオフセット設定
    while (1)
    {
      Output_Coordinate(0.20, 0.20, 0,
                        0.15, 0.15, 0,
                        0.15, 0.15, 0,
                        0.15, 0.15, 0,
                        0.30,
                        20, 0.05);
      //bno.get_angles();
      //oled.init();
      //oled.printf("%2.1f %2.1f %2.1f", bno.euler.roll, bno.euler.pitch, bno.euler.yaw);
    }
  }

  //Body_Up_Dainozi(0.15, 0.25, 0.020, 0.25);
  /*Trot(0.15, 0.25, 0.2,
       0.2, 0.15,
       1.0, 1.0,
       5);*/

  /*Output_Coordinate( 0.25, 0.25, 0,
                    0.25, 0.25, 0,
                    0.25, 0.25, 0,
                    0.25, 0.25, 0,
                    0.25,
                    20, 0.05);*/

  Wait_UB();

  Body_Down(0.15, 0.15);

  //Store_Position();

#ifndef ODE
  return 0;
#endif
}