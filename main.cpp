#include "mbed.h"
#include "MCP23017.h"
#include "WattBob_TextLCD.h"
#include "TCS3472_I2C.h"
#include "stdint.h"
#include "VL6180.h"
#include "TCS3200.h"
#define IDENTIFICATIONMODEL_ID 0x0000
#define     BACK_LIGHT_ON(INTERFACE)    INTERFACE->write_bit(1,BL_BIT)
#define     BACK_LIGHT_OFF(INTERFACE)   INTERFACE->write_bit(0,BL_BIT)
MCP23017            *par_port;
WattBob_TextLCD     *lcd;
//------------------------------端口定义-----------------------------------------
TCS3472_I2C rgb_sensor(p9, p10);//颜色传感器// 10-yellow-clock  9-orange-date
Serial pc(USBTX, USBRX);  // 9600 baud
DigitalOut FPGA_1(p14);//
DigitalOut FPGA_2(p15);//
DigitalOut FPGA_3(p16);//
DigitalOut FPGA_4(p17);//
DigitalOut FPGA_5(p18);
DigitalOut FPGA_6(p19);
DigitalOut FPGA_7(p20);
DigitalIn sensor1(p21);//端口为小写p
DigitalIn sensor2(p22);//端口为小写p
DigitalIn sensor3(p23);//端口为小写p
DigitalIn sensor4(p24);//端口为小写p
VL6180  TOF_sensor(p28, p27); // 距离传感器 27-yellow-clock  28-orange-data
DigitalOut myled1(LED1);//LED需要大写
DigitalOut myled2(LED2);//LED需要大写
DigitalOut myled3(LED3);//LED需要大写
DigitalOut myled4(LED4);//LED需要大写
//函数声明-----------------------------------------------------------------------
int readcard(int sensor1,int sensor2,int sensor3,int sensor4);
void lcd_printf_number(int a,int b,int c,int d);//lcd输出数字
void sys_init();//系统初始化
void fpga_transmit_machine(int i);// 0小盘转 1大盘转 2小盘推 3大盘推 4开盖
void fpga_transmit_state(int i); //小盘
void fpga_send_machine(int a,int b,int c);
void fpga_send_state(int a,int b,int c,int d);
//char colour_1(uint8_t a, uint8_t b, uint8_t c);
char colour_1();
//主函数开始----------------------------------------------------------------------
int main()
{
//变量定义-----------------------------------------------------------------------
   // uint8_t red_1,blue_1,green_1;//I2C接口
    uint8_t dist;   //unsigned int 8位
    int key;
    char command='0';      
    int s=1;//state;
    int m=0;//
    int flag_red=1;
    int flag_green=1;
    int flag_blue=1;
    int flag_yellow=1;
    int k=0;
//游戏开始-----------------------------------------------------------------------
    while(1){
            sys_init();
            do {
                command=pc.getc(); 
                lcd->cls(); 
                lcd->locate(0,0);
                lcd->printf("%c",command);
                wait(2);
            } while(command!='s');
        //    pc.printf("i got command=%c\n\r",command);//距离
            wait(4);
            key = readcard(sensor1,sensor2,sensor3,sensor4);//读卡判断
            pc.printf("%06d",key);
            if(key!=1111) {
            wait(1);
            fpga_transmit_machine(5);//选择升盖电机
            wait(2);
            fpga_transmit_state(2);//升   
            dist =    TOF_sensor.getDistance();   
            
            while(1){
                dist = TOF_sensor.getDistance();  
               // pc.printf("%d",dist);
                if(dist<100)
                {
                    pc.printf("3");
                    k=0;
                    break;
                }
                else if(dist>=100)
                {   
                    if(k<=5)
                    {
                    pc.printf("1");
                    k=k+1;
                    wait(2);
                    }else{
                    pc.printf("2");
                    k=0;
                    break;
                    }
                }
                }
            do{    
                    command=pc.getc();
                    lcd->cls(); 
                    lcd->locate(0,0);
                    lcd->printf("%c",command);
                    wait(2);
               //   pc.printf("i got command=%c\n\r",command);//调试用程序
                    if(command=='1'){
                        s=1;
                        fpga_transmit_machine(1);//选定大盘
                        //pc.printf("machine 1 is specified\n\r");//调试用程序
                        fpga_transmit_state(s);//1位置
                        //pc.printf("machine 1 is in %d state\n\r",s);//调试用程序
                        fpga_transmit_machine(2);//选定小盘
                        //pc.printf("machine 2 is specified\n\r");//调试用程序
                        fpga_transmit_state(s);//1位置
                        //pc.printf("machine 2 is in %d state\n\r",s);//调试用程序
                        wait(2);
                        if(flag_red==1)
                        {
                            fpga_transmit_machine(1);//选定小盘电机
                          // pc.printf("machine 1 is specified\n\r");//调试用程序
                            fpga_transmit_state(3);//小盘电机180的位置
                           //pc.printf("machine 1 is in 3 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(3);//选定小盘推的电机
                          // pc.printf("machine 3 is specified\n\r");//调试用程序
                            fpga_transmit_state(2);//小盘推
                         //  pc.printf("machine 3 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_state(1);//小盘收回
                          //pc.printf("machine 3 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(5);//选择升盖电机
                          //pc.printf("5 machine is specified\n\r");//调试用程序
                            fpga_transmit_state(1); //降下
                         // pc.printf("5 machine is down    n\r");//调试用程序
                             wait(1);
                            pc.printf("e");//调试用程序
                           // wait(10);
                            flag_red=0;
                        }
                        else
                        {
                    
                            while(colour_1()!='r')//为真循环
                            {   
                                if(s<9)
                                {   
                                    s++;
                                }
                                else if(s==9)
                                {
                                    s=0;
                                }
                                fpga_transmit_machine(2);//选定大盘
                                fpga_transmit_state(s);   
                                //pc.printf("machine 2 is in %d state\n\r",s);//调试用程序
                                wait(3);
                            }
                            wait(2);
                            if(s<2)
                            {
                                s=s+8;   
                                //pc.printf("machine 2 is going to the %d state\n\r",s);//调试用程
                            }
                            else{
                                s=s-2; 
                                    //pc.printf("machine 2 is going to the %d state\n\r",s);//调试用程序
                            }
                            wait(2);
                            fpga_transmit_machine(2);
                            fpga_transmit_state(s);
                            wait(2);
                            fpga_transmit_machine(1);//选定小盘电机
                           //pc.printf("machine 1 is specified\n\r");//调试用程序
                            fpga_transmit_state(1);//小盘电机1的位置(0度)
                           //pc.printf("machine 1 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(4);//选定大盘推的电机
                         // pc.printf("machine 4 is specified\n\r");//调试用程序
                            fpga_transmit_state(2);//推
                         //pc.printf("machine 4 is in 2 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_state(1);//回来
                           //pc.printf("machine 4 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(1);//选定小盘电机
                          // pc.printf("machine 1 is specified\n\r");//调试用程序
                            fpga_transmit_state(3);//小盘电机180的位置
                           //pc.printf("machine 1 is in 3 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(3);//选定小盘推的电机
                          // pc.printf("machine 3 is specified\n\r");//调试用程序
                            fpga_transmit_state(2);//小盘推
                         //  pc.printf("machine 3 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_state(1);//小盘收回
                          //pc.printf("machine 3 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(5);//选择升盖电机
                          //pc.printf("5 machine is specified\n\r");//调试用程序
                            fpga_transmit_state(1); //降下
                         // pc.printf("5 machine is down    n\r");//调试用程序
                            //wait(1);
                            pc.printf("e");//调试用程序
                         //   wait(10);
                        }
                    }else if(command=='2'){     //2为绿 
                        s=1;
                        fpga_transmit_machine(1);//选定大盘
                        //pc.printf("machine 1 is specified\n\r");//调试用程序
                        fpga_transmit_state(s);//1位置
                        //pc.printf("machine 1 is in %d state\n\r",s);//调试用程序
                        fpga_transmit_machine(2);//选定大盘
                        //pc.printf("machine 2 is specified\n\r");//调试用程序
                        fpga_transmit_state(s);//1位置
                        //pc.printf("machine 2 is in %d state\n\r",s);//调试用程序
                        wait(2);
                        if(flag_green==1)
                        {
                            fpga_transmit_machine(1);//选定小盘电机
                          // pc.printf("machine 1 is specified\n\r");//调试用程序
                            fpga_transmit_state(4);//小盘电机270的位置
                           //pc.printf("machine 1 is in 3 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(3);//选定小盘推的电机
                          // pc.printf("machine 3 is specified\n\r");//调试用程序
                            fpga_transmit_state(2);//小盘推
                         //  pc.printf("machine 3 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_state(1);//小盘收回
                          //pc.printf("machine 3 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(5);//选择升盖电机
                          //pc.printf("5 machine is specified\n\r");//调试用程序
                            fpga_transmit_state(1); //降下
                         // pc.printf("5 machine is down    n\r");//调试用程序
                             wait(1);
                            pc.printf("e");//调试用程序
                        //    wait(10);
                            flag_green=0;
                        }
                        else
                        {
                            while(colour_1()!='g')//为真循环
                            {   
                                if(s<9)
                                {   
                                    s++;
                                }
                                else if(s==9)
                                {
                                    s=0;
                                }
                                fpga_transmit_machine(2);//选定大盘
                                fpga_transmit_state(s);   
                                //pc.printf("machine 2 is in %d state\n\r",s);//调试用程序
                                wait(3);
                            }
                                wait(2);
                                 if(s<2)
                                 {
                                    s=s+8;   
                                    //pc.printf("machine 2 is going to the %d state\n\r",s);//调试用程
                                 }
                                 else{
                                    s=s-2; 
                                    //pc.printf("machine 2 is going to the %d state\n\r",s);//调试用程序
                                }
                                wait(2);
                            fpga_transmit_machine(2);
                            fpga_transmit_state(s);
                            wait(2);
                            fpga_transmit_machine(1);//选定小盘电机
                           //pc.printf("machine 1 is specified\n\r");//调试用程序
                            fpga_transmit_state(2);//小盘电机1的位置(90度)
                           //pc.printf("machine 1 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(4);//选定大盘推的电机
                         // pc.printf("machine 4 is specified\n\r");//调试用程序
                            fpga_transmit_state(2);//推
                         //pc.printf("machine 4 is in 2 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_state(1);//回来
                           //pc.printf("machine 4 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(1);//选定小盘电机
                          // pc.printf("machine 1 is specified\n\r");//调试用程序
                            fpga_transmit_state(4);//小盘电机270的位置
                           //pc.printf("machine 1 is in 3 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(3);//选定小盘推的电机
                          // pc.printf("machine 3 is specified\n\r");//调试用程序
                            fpga_transmit_state(2);//小盘推
                         //  pc.printf("machine 3 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_state(1);//小盘收回
                          //pc.printf("machine 3 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(5);//选择升盖电机
                          //pc.printf("5 machine is specified\n\r");//调试用程序
                            fpga_transmit_state(1); //降下
                         // pc.printf("5 machine is down    n\r");//调试用程序
                            wait(1);
                            pc.printf("e");//调试用程序
                           // wait(10);                     //pc.printf("5 machine is down    n\r");//调试用程序
                        }
                    }else if(command=='3'){     //3为蓝色
                        s=1;
                        fpga_transmit_machine(1);//选定大盘
                        //pc.printf("machine 1 is specified\n\r");//调试用程序
                        fpga_transmit_state(s);//1位置
                        //pc.printf("machine 1 is in %d state\n\r",s);//调试用程序
                        fpga_transmit_machine(2);//选定大盘
                        //pc.printf("machine 2 is specified\n\r");//调试用程序
                        fpga_transmit_state(s);//1位置
                        //pc.printf("machine 2 is in %d state\n\r",s);//调试用程序
                        wait(2);
                        if(flag_blue==1)
                        {
                            fpga_transmit_machine(1);//选定小盘电机
                          // pc.printf("machine 1 is specified\n\r");//调试用程序
                            fpga_transmit_state(1);//小盘电机0的位置
                           //pc.printf("machine 1 is in 3 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(3);//选定小盘推的电机
                          // pc.printf("machine 3 is specified\n\r");//调试用程序
                            fpga_transmit_state(2);//小盘推
                         //  pc.printf("machine 3 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_state(1);//小盘收回
                          //pc.printf("machine 3 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(5);//选择升盖电机
                          //pc.printf("5 machine is specified\n\r");//调试用程序
                            fpga_transmit_state(1); //降下
                         // pc.printf("5 machine is down    n\r");//调试用程序
                             wait(1);
                            pc.printf("e");//调试用程序
                           // wait(10);
                            flag_blue=0;
                        }
                        else
                        {
                            while(colour_1()!='b')//为真循环
                            {   
                                if(s<9)
                                {   
                                    s++;
                                }
                                else if(s==9)
                                {
                                    s=0;
                                }
                                fpga_transmit_machine(2);//选定大盘
                                fpga_transmit_state(s);   
                                //pc.printf("machine 2 is in %d state\n\r",s);//调试用程序
                                wait(3);
                            }
                                wait(2);
                                 if(s<2)
                                 {
                                    s=s+8;   
                                    //pc.printf("machine 2 is going to the %d state\n\r",s);//调试用程
                                 }
                                 else{
                                    s=s-2; 
                                    //pc.printf("machine 2 is going to the %d state\n\r",s);//调试用程序
                                }
                                wait(2);
                            fpga_transmit_machine(2);
                            fpga_transmit_state(s);
                            wait(2);
                            fpga_transmit_machine(1);//选定小盘电机
                           //pc.printf("machine 1 is specified\n\r");//调试用程序
                            fpga_transmit_state(3);//小盘电机1的位置(180度)
                           //pc.printf("machine 1 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(4);//选定大盘推的电机
                         // pc.printf("machine 4 is specified\n\r");//调试用程序
                            fpga_transmit_state(2);//推
                         //pc.printf("machine 4 is in 2 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_state(1);//回来
                           //pc.printf("machine 4 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(1);//选定小盘电机
                          // pc.printf("machine 1 is specified\n\r");//调试用程序
                            fpga_transmit_state(1);//小盘电机0的位置
                           //pc.printf("machine 1 is in 3 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(3);//选定小盘推的电机
                          // pc.printf("machine 3 is specified\n\r");//调试用程序
                            fpga_transmit_state(2);//小盘推
                         //  pc.printf("machine 3 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_state(1);//小盘收回
                          //pc.printf("machine 3 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(5);//选择升盖电机
                          //pc.printf("5 machine is specified\n\r");//调试用程序
                            fpga_transmit_state(1); //降下
                         // pc.printf("5 machine is down    n\r");//调试用程序
                             wait(1);
                            pc.printf("e");//调试用程序
                           // wait(10);
                        }
                    }else if(command=='4'){      //4为黄色呃
                        s=1;
                        fpga_transmit_machine(1);//选定大盘
                        //pc.printf("machine 1 is specified\n\r");//调试用程序
                        fpga_transmit_state(s);//1位置
                        //pc.printf("machine 1 is in %d state\n\r",s);//调试用程序
                        fpga_transmit_machine(2);//选定大盘
                        //pc.printf("machine 2 is specified\n\r");//调试用程序
                        fpga_transmit_state(s);//1位置
                        //pc.printf("machine 2 is in %d state\n\r",s);//调试用程序
                        wait(2);
                        if(flag_yellow==1)
                        {
                            fpga_transmit_machine(1);//选定小盘电机
                          // pc.printf("machine 1 is specified\n\r");//调试用程序
                            fpga_transmit_state(2);//小盘电机0的位置
                           //pc.printf("machine 1 is in 3 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(3);//选定小盘推的电机
                          // pc.printf("machine 3 is specified\n\r");//调试用程序
                            fpga_transmit_state(2);//小盘推
                         //  pc.printf("machine 3 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_state(1);//小盘收回
                          //pc.printf("machine 3 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(5);//选择升盖电机
                          //pc.printf("5 machine is specified\n\r");//调试用程序
                            fpga_transmit_state(1); //降下
                         // pc.printf("5 machine is down    n\r");//调试用程序
                             wait(1);
                            pc.printf("e");//调试用程序
                        //    wait(10);
                            flag_yellow=0;
                        }
                        else
                        {
                            while(colour_1()!='y')//为真循环
                            {   
                                if(s<9)
                                {   
                                    s++;
                                }
                                else if(s==9)
                                {
                                    s=0;
                                }
                                fpga_transmit_machine(2);//选定大盘
                                fpga_transmit_state(s);   
                                //pc.printf("machine 2 is in %d state\n\r",s);//调试用程序
                                wait(3);
                            }
                                wait(2);
                                 if(s<2)
                                 {
                                    s=s+8;   
                                    //pc.printf("machine 2 is going to the %d state\n\r",s);//调试用程
                                 }
                                 else{
                                    s=s-2; 
                                    //pc.printf("machine 2 is going to the %d state\n\r",s);//调试用程序
                                }
                                wait(2);
                            fpga_transmit_machine(2);
                            fpga_transmit_state(s);
                            wait(2);
                            fpga_transmit_machine(1);//选定小盘电机
                           //pc.printf("machine 1 is specified\n\r");//调试用程序
                            fpga_transmit_state(4);//小盘电机1的位置(270度)
                           //pc.printf("machine 1 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(4);//选定大盘推的电机
                         // pc.printf("machine 4 is specified\n\r");//调试用程序
                            fpga_transmit_state(2);//推
                         //pc.printf("machine 4 is in 2 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_state(1);//回来
                           //pc.printf("machine 4 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(1);//选定小盘电机
                          // pc.printf("machine 1 is specified\n\r");//调试用程序
                            fpga_transmit_state(2);//小盘电机90的位置
                           //pc.printf("machine 1 is in 3 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(3);//选定小盘推的电机
                          // pc.printf("machine 3 is specified\n\r");//调试用程序
                            fpga_transmit_state(2);//小盘推
                         //  pc.printf("machine 3 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_state(1);//小盘收回
                          //pc.printf("machine 3 is in 1 state\n\r");//调试用程序
                            wait(3);
                            fpga_transmit_machine(5);//选择升盖电机
                          //pc.printf("5 machine is specified\n\r");//调试用程序
                            fpga_transmit_state(1); //降下
                         // pc.printf("5 machine is down    n\r");//调试用程序
                             wait(1);
                            pc.printf("e");//调试用程序
                           // wait(10);
                        }
                    }

                }while(command!='r');
            }   else 
                {
                //pc.printf("maintanance mode ");
                while(1){
                    do {
                        command=pc.getc();  
                         lcd->cls(); 
                        lcd->locate(0,0);
                        lcd->printf("%c",command);
                        wait(2);
                    } while(command!='m');//maintaince mode 为真循环
                    dist = TOF_sensor.getDistance();
                    wait(1);
                    dist = TOF_sensor.getDistance();
                    pc.printf("%d",dist);//距离
                    wait(1);
                   // pc.printf("red=%d,green=%d,blue=%d",rgb_sensor.getRedData(),rgb_sensor.getGreenData(), rgb_sensor.getBlueData());//距离
                    wait(1);
                    pc.printf("%c",colour_1());
                    wait(1);
                    fpga_transmit_machine(1);//检测小盘
                    for(s=1;s<=4;s++)
                    {
                        fpga_transmit_state(s);
                        wait(4);
                    }
                    fpga_transmit_machine(2);//检测大盘
                    for(s=1;s<=9;s++)
                    {
                        fpga_transmit_state(s);
                        wait(4);
                    }
                    for(m=3;m<=5;m++){//检测剩下所有
                        fpga_transmit_machine(m);//
                        wait(1);
                        for(s=2;s>=1;s--)
                        {
                            fpga_transmit_state(s);
                            wait(4);
                        }
                    }  
                }//else里的while(1)函数
            }//else 函数
    }//初始的while(1)函数
}//main 函数

//-----------------------------函数定义----------------------------------
int readcard(int sensor1,int sensor2,int sensor3,int sensor4){
        int key;
        int a=sensor1;
        int b=sensor2;
        int c=sensor3;
        int d=sensor4;
        a=a*1000;
        b=b*100;
        c=c*10;
        key=a+b+c+d;
          wait(1);
     //   pc.printf("%06d",key);//001110 00开头的为读卡信息
        return key;
}
void lcd_printf_number(int a,int b,int c,int d){
            lcd->cls();
            lcd->locate(c,d);
            switch(b){
            case 1:lcd->printf("red:%d",a);break;
            case 2:lcd->printf("green:%d",a);break;
            case 3:lcd->printf("blue:%d",a);break;
            case 4:lcd->printf("distant:%d",a);break;
            case 5:lcd->printf("key:%d",a);break;
            default:lcd->printf("error");
            }
            wait(1);
    }
void sys_init(){
    par_port = new MCP23017(p9, p10, 0x40);             //IO口扩展用不需要改
    par_port->config(0x0F00, 0x0F00, 0x0F00);           // 将MCP23017 chip 连接到LCD显示屏上
    lcd = new WattBob_TextLCD(par_port);
    BACK_LIGHT_ON(par_port);
    TOF_sensor.VL6180_Init();   //initialize the distance sensor
    rgb_sensor.enablePowerAndRGBC();    // initialize the color sensor
    rgb_sensor.setIntegrationTime(10);
    fpga_send_state(0,0,0,0);
    fpga_send_machine(0,0,0);
    }


char colour_1(){
uint8_t a= rgb_sensor.getRedData();
uint8_t b= rgb_sensor.getGreenData();
uint8_t c= rgb_sensor.getBlueData(); 
/*long w;   //距离黄色的距离 unit8 0~255
long x;   //距离红色的位置
long y;   //g
long z;   //b 
w=((255-a)*(255-a)+(255-b)*(255-b)+(0-c)*(0-c)); //c语言没有乘方符号^   !!!!!
x=((255-a)*(255-a)+(0-b)*(0-b)+(0-c)*(0-c));
y=((0-a)*(0-a)+(255-b)*(255-b)+(0-c)*(0-c));
z=((0-a)*(0-a)+(0-b)*(0-b)+(255-c)*(255-c));
// pc.printf("w=%d,x=%d,y=%d,z=%d\n\r",w,x,y,z);
    wait(1);*/
    if((a>=15)||(b>=15)||(c>=15))
    {
        if((a>b)&&(a>=c))
        {   
            return 'r';   
        }
        else if((b>=a)&&(b>=c)){
            if(b-a<15)
            return 'y';
            else
            return 'g';
            }
        else if((c>=a)&&(c>=b))
            return 'b';
        else 
            return 'n';
    }
    else 
        return 'n';
          
}


void fpga_transmit_state(int i){     //abc共8位传输码
  switch(i)
        {                         
        case 1: fpga_send_state(0,0,0,1); break;//
        case 2: fpga_send_state(0,0,1,0);  break;//
        case 3: fpga_send_state(0,0,1,1); break;//
        case 4: fpga_send_state(0,1,0,0); break;//
        case 5: fpga_send_state(0,1,0,1);  break;//
        case 6: fpga_send_state(0,1,1,0); break;//
        case 7: fpga_send_state(0,1,1,1); break;//
        case 8: fpga_send_state(1,0,0,0); break;//
        case 9: fpga_send_state(1,0,0,1);  break;//
        case 0: fpga_send_state(1,0,1,0); break;//
        }
    }
void fpga_transmit_machine(int i){     //abc共8位传输码
  switch(i)
        {                         
      
        case 1: fpga_send_machine(0,0,1); break;//
        case 2: fpga_send_machine(0,1,0); break;//
        case 3: fpga_send_machine(0,1,1); break;//
        case 4: fpga_send_machine(1,0,0); break;// 
        case 5: fpga_send_machine(1,0,1); break;//   
        }
}

void fpga_send_state(int a,int b,int c,int d){
    FPGA_4=a;
    FPGA_5=b;
    FPGA_6=c;
    FPGA_7=d;
    }

void fpga_send_machine(int a,int b,int c){
    FPGA_1=a;
    FPGA_2=b;
    FPGA_3=c;
    }