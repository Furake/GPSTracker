/**
 * Copyright @ 深圳市谷米万物科技有限公司. 2009-2019. All rights reserved.
 * File name:        hard_ware.h
 * Author:           王志华       
 * Version:          1.0
 * Date:             2019-03-01
 * Description:      简单硬件（开关）读写操作封装、电源管理、电池充电管理
 * Others:           需要永久存储的状态,放到system_state模块
 * Function List:    
    1. 创建hard_ware模块
    2. 销毁hard_ware模块
    3. 定时处理入口
    4. 获取ACC输入状态
    5. 设置GPS LED状态
    6. 设置GSM LED状态
    7. 设置断油电IO状态
    8. 设置看门狗电平
    9. 获取供电电源电压值
    10.获取外部电池电量百分比
    11.重启系统
    12.休眠
    13.唤醒
    
 * History: 
    1. Date:         2019-03-01
       Author:       王志华
       Modification: 创建初始版本
    2. Date:          
       Author:         
       Modification: 

 */

#ifndef __HARD_WARE_H__
#define __HARD_WARE_H__

#include "gm_type.h"
#include "error_code.h"
#include "system_state.h"
#include "config_service.h"

typedef enum
{
    VOLT_NONE = 0,
    VOLT_12V = 1,
    VOLT_24V = 2,
    VOLT_36V = 3,
    VOLT_48V = 4,
    VOLT_60V = 6,
    VOLT_72V = 7,
    VOLT_84V = 8,
    VOLT_96V = 9,

    VOLT_MAX
}GM_POWER_VOLTAGE_GRADE;


/**
 * Function:   1.创建hard_ware模块
 * Description:创建hard_ware模块
 * Input:      无
 * Output:     无
 * Return:     GM_SUCCESS——成功；其它错误码——失败
 * Others:     使用前必须调用,否则调用其它接口返回失败错误码
 */
GM_ERRCODE hard_ware_create(void);

/**
 * Function:   2.销毁hard_ware模块
 * Description:销毁hard_ware模块
 * Input:      无
 * Output:     无
 * Return:     GM_SUCCESS——成功；其它错误码——失败
 * Others:       
 */
GM_ERRCODE hard_ware_destroy(void);

/**
 * Function:   3.hard_ware模块定时处理入口
 * Description:hard_ware模块定时处理入口
 * Input:       无
 * Output:      无
 * Return:      GM_SUCCESS——成功；其它错误码——失败
 * Others:      1秒调用1次
 */
GM_ERRCODE hard_ware_timer_proc(void);



/**
 * Function:   判断设备型号是否能检测到vcdt电压
 * Description:
 * Input:      设备型号ID
 * Output:     无
 * Return:     true:可检测 false:不可检测
 * Others:       
 */
bool hard_ware_has_vcdt_voltage(void);


/**
 * Function:   4.1 获取ACC输入状态
 * Description:
 * Input:      无
 * Output:     无
 * Return:     GM_SUCCESS——成功；其它错误码——失败
 * Others:     
 */
GM_ERRCODE hard_ware_get_acc_level(bool* p_state);

/**
 * Function:   4.2 获取ACC线状态
 * Description:震动报警时使用
 * Input:      无
 * Output:     无
 * Return:     GM_SUCCESS——成功；其它错误码——失败
 * Others:     
 */
GM_ERRCODE hard_ware_get_acc_line_level(bool* p_state);


/**
 * Function:   获取温度传感器温度值
 * Description:
 * Input:      无
 * Output:     无
 * Return:     当前温度值
 * Others:     
 */
s8 hard_ware_get_temperature(void);


/**
 * Function:   5. 设置GPS LED状态
 * Description:只提供给led模块使用,其它模块不要直接使用这个接口
 * Input:      is_on:true——亮灯；false——灭灯
 * Output:     无
 * Return:     GM_SUCCESS——成功；其它错误码——失败
 * Others:       
 */
GM_ERRCODE hard_ware_set_gps_led(bool is_on);

/**
 * Function:   6. 设置GSM LED状态
 * Description:只提供给led模块使用,其它模块不要直接使用这个接口
 * Input:      is_on:true——亮灯；false——灭灯
 * Output:     无
 * Return:     GM_SUCCESS——成功；其它错误码——失败
 * Others:       
 */
GM_ERRCODE hard_ware_set_gsm_led(bool is_on);


/**
 * Function:   设置POWER LED状态
 * Description:只提供给led模块使用,其它模块不要直接使用这个接口
 * Input:	   state:true——亮灯；false——灭灯
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE hard_ware_set_power_led(bool is_on);


/**
 * Function:   7. 设置断油电IO状态
 * Description:
 * Input:      state:true——断油电；false——恢复油电
 * Output:     无
 * Return:     GM_SUCCESS——成功；其它错误码——失败
 * Others:       
 */
GM_ERRCODE hard_ware_set_relay(bool state);

/**
 * Function:   8. 设置看门狗电平
 * Description:
 * Input:      state:true——高电平；false——低电平
 * Output:     无
 * Return:     GM_SUCCESS——成功；其它错误码——失败
 * Others:       
 */
GM_ERRCODE hard_ware_set_watchdog(bool state);


/**
 * Function:   9. 获取供电电源电压值
 * Description:单位伏特（v）
 * Input:      无
 * Output:     p_voltage:电压值指针
 * Return:     GM_SUCCESS——成功；其它错误码——失败
 * Others:       
 */
GM_ERRCODE hard_ware_get_power_voltage(float* p_voltage);

/**
 * Function:    获取内部电池电量级别
 * Description: 0:无电（关机）  <=3.45V
                1:电量极低（不足以打电话发短信等） 3.45V --- 3.5V
                2:点亮很低（低电报警） <=3.6V
                3:电量低（可正常使用） <=3.8V
                4:电量中  <=3.95V
                5:电量高  <=4.1V
                6:电量极高  >4.1V
 * Input:       无
 * Output:      p_level:电量级别指针
 * Return:      GM_SUCCESS——成功；其它错误码——失败
 * Others:       
 */
GM_ERRCODE hard_ware_get_internal_battery_voltage(float* p_voltage);

/**
 * Function:   是否在充电
 * Description: 
 * Input:      无
 * Output:     无
 * Return:     true——在充电；false——不在充电
 * Others:       
 */
bool hard_ware_battery_is_charging(void);


/**
 * Function:   是否已充满
 * Description: 
 * Input:      无
 * Output:     无
 * Return:     true——充满；false——未充满
 * Others:       
 */
bool hard_ware_battery_is_full(void);

/**
 * Function:    10.获取外部电池电量百分比
 * Description:单位伏特（v）
 * Input:      无
 * Output:     p_percent:百分比指针
 * Return:     GM_SUCCESS——成功；其它错误码——失败
 * Others:       
 */
GM_ERRCODE hard_ware_get_extern_battery_percent(U8* p_percent);

/**
 * Function:    10.获取内部电池电量百分比
 * Description:单位伏特（v）
 * Input:      无
 * Output:     无
 * Return:     百分比值
 * Others:       
 */
u8 hard_ware_get_internal_battery_percent(void);


/**
 * Function:    获取内部电池电量级别
 * Description: 0:无电（关机）  <=3.45V
                1:电量极低（不足以打电话发短信等） 3.45V --- 3.5V
                2:点亮很低（低电报警） <=3.6V
                3:电量低（可正常使用） <=3.8V
                4:电量中  <=3.95V
                5:电量高  <=4.1V
                6:电量极高  >4.1V
 * Input:       无
 * Output:      p_level:电量级别指针
 * Return:      GM_SUCCESS——成功；其它错误码——失败
 * Others:       
 */
GM_ERRCODE hard_ware_get_internal_battery_level(U8* p_level);


/**
 * Function:   11.重启
 * Description:重启整个系统软件
 * Input:      reason:重启原因;delay_seconds:延迟秒数
 * Output:     无
 * Return:     GM_SUCCESS——成功；其它错误码——失败
 * Others:       
 */
GM_ERRCODE hard_ware_reboot(const BootReason reason,U16 delay_seconds);

/**
 * Function:   12.休眠
 * Description:使系统休眠
 * Input:      无
 * Output:     无
 * Return:     GM_SUCCESS——成功；其它错误码——失败
 * Others:       
 */
GM_ERRCODE hard_ware_sleep(void);

/**
 * Function:   13.关闭GPS
 * Description:使系统休眠
 * Input:      无
 * Output:     无
 * Return:     GM_SUCCESS——成功；其它错误码——失败
 * Others:       
 */
GM_ERRCODE hard_ware_close_gps(void);


/**
 * Function:   14.唤醒
 * Description:使系统唤醒
 * Input:      无
 * Output:     无
 * Return:     GM_SUCCESS——成功；其它错误码——失败
 * Others:       
 */
GM_ERRCODE hard_ware_awake(void);

/**
 * Function:   15.打开GPS
 * Description:使系统休眠
 * Input:      无
 * Output:     无
 * Return:     GM_SUCCESS——成功；其它错误码——失败
 * Others:       
 */
GM_ERRCODE hard_ware_open_gps(void);


GM_ERRCODE reinit_relay_gpio(void);
/**
 * Function:   判断设备型号是否为obd系列
 * Description:
 * Input:      设备型号ID
 * Output:     无
 * Return:     true:为obd系列设备 false:非obd系列设备
 * Others:       
 */
bool hard_ware_is_device_obd(void);


/**
 * Function:   判断设备型号是否为W系列
 * Description:
 * Input:      设备型号ID
 * Output:     无
 * Return:     true:为W系列设备 false:非W系列设备
 * Others:       
 */
bool hard_ware_is_device_w(void);

/**
 * Function:   判断设备型号是否为05系列
 * Description:
 * Input:      设备型号ID
 * Output:     无
 * Return:     true:为05系列设备 false:非05系列设备
 * Others:       
 */
bool hard_ware_is_device_05(void);



/**
 * Function:   判断设备型号是否有电池
 * Description:
 * Input:      设备型号ID
 * Output:     无
 * Return:     true:有电池设备 false:无电池设备
 * Others:       
 */

bool hard_ware_has_battery(void);


/**
 * Function:   判断设备型号是否有WIFI
 * Description: 判断设备型号是否有WIFI
 * Input:      设备型号ID
 * Output:     无
 * Return:     true:有WIFI false:无WIFI
 * Others:       
 */
bool hard_ware_device_has_wifi(void);

/**
 * Function:   判断设备型号是否有录音功能
 * Description: 判断设备型号是否有录音功能
 * Input:      设备型号ID
 * Output:     无
 * Return:     true:有录音功能 false:无录音功能
 * Others:       
 */
bool hard_ware_device_has_recorder(void);


/**
 * Function:   判断设备型号是否有断油电功能
 * Description: 判断设备型号是否有断油电功能
 * Input:      设备型号ID
 * Output:     无
 * Return:     true:有断油电功能 false:无断油电功能
 * Others:       
 */
bool hard_ware_device_has_relay(void);

/**
 * Function:   判断设备型号sensor是否与GPS天线同一面
 * Description: 判断设备型号sensor是否与GPS天线同一面
 * Input:      设备型号ID
 * Output:     无
 * Return:     true:与GPS天线同一面 false:与GPS天线不同一面
 * Others:       
 */
bool hard_ware_sensor_same_side_as_gps_antenna(void);


/**
 * Function:   判断设备型号是否不锁IP
 * Description: 判断设备型号是否不锁IP
 * Input:      设备型号ID
 * Output:     无
 * Return:     true:不锁IP false:锁IP
 * Others:       
 */
bool hard_ware_device_unlock_ip(void);


/**
 * Function:   设备是否有sos报警功能
 * Description:
 * Input:      无
 * Output:     无
 * Return:     true 有，false 无
 * Others:       
 */
bool hard_ware_has_sos(void);


/**
 * Function:   设置型号是否执行AT指令
 * Description:是否执行AT指令
 * Input:      无
 * Output:     无
 * Return:     true AT指令  false 非AT指令
 * Others:       
 */
bool hard_ware_is_at_command(void);


GM_ERRCODE hard_ware_ec20_power_key(bool is_on);

GM_ERRCODE hard_ware_ec20_reset(bool is_on);





#endif



