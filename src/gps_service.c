#include "gm_type.h"
#include "math.h"
#include "stdio.h"
#include "gm_stdlib.h"
#include "gm_gprs.h"
#include "gps_service.h"
#include "fifo.h"
#include "gps.h"
#include "protocol.h"
#include "log_service.h"
#include "gm_memory.h"
#include "utility.h"
#include "gsm.h"
#include "config_service.h"
#include "protocol_goome.h"
#include "protocol_concox.h"
#include "protocol_jt808.h"
#include "gprs.h"
#include "system_state.h"
#include "led.h"
#include "update_service.h"
#include "hard_ware.h"
#include "applied_math.h"
#include "gps_service.h"
#include "agps_service.h"
#include "update_service.h"
#include "at_command.h"
#include "wifi.h"
#include "sms.h"
#define GPS_PING_TIME 60

typedef struct
{
	bool heart_atonce;     // 立即发送心跳
    u32 heart_send_time;   // 心跳发送时间
    u32 heart_locate_send_time;  // 定位发送时间   超越平台PROTOCOL_JT808只支持定位,不支持心跳,所以用定位代替心跳
    u32 heart_receive_time;  // 心跳收到时间
	u32 receive_heart_counts;  //收到心跳计数
	u32 send_location_counts;  //发送定位数据计数
    u32 data_finish_time;  //进入到SOCKET_STATUS_DATA_FINISH状态的时间
    u32 lbs_send_time;	//移动定位服务发送时间
    u32 wifi_send_time;	//wifi发送时间
	bool wifi_send_atonce; //立即上传一条WiFi信息
    u32 saved_socket_ack;  //发消息前，ack的值
    u32 connection_time;  //连接建立时间
    u32 recorder_send_time;
    bool temperature_atonce; // 立即发送温度
    u32 temperature_send_time;   // 温度发送时间
    s8  last_temperature;  //最近一次发送的温度值
    u32 signal_send_time; //GPS.GSM信号数据上传时间
}SocketTypeExtend;



static SocketType s_gps_socket = {-1,"",SOCKET_STATUS_ERROR,};
static SocketTypeExtend s_gps_socket_extend = {0,0,0,0,0,0,0,0,0};

static GM_ERRCODE gps_service_transfer_status(u8 new_status);
static void gps_service_init_proc(void);
static void gps_service_connecting_proc(void);
static void gps_service_login_proc(void);
static void gps_service_work_proc(void);
static void gps_service_close(void);

static GM_ERRCODE protocol_pack_gps_msg(GpsDataModeEnum mode, const GPSData *gps, u8 *pdata, u16 *idx, u16 len);
static GM_ERRCODE protocol_pack_lbs_msg(u8 *pdata, u16 *idx, u16 len);
static GM_ERRCODE protocol_pack_alarm_msg(AlarmInfo *alarm, u8 *pdata, u16 *idx, u16 len);
static GM_ERRCODE protocol_pack_device_state_msg(u8 *pdata, u16 *idx, u16 len);
static GM_ERRCODE protocol_pack_position_request_msg(u8 *mobile_num, u8 num_len, u8 *command, u8 cmd_len,u8 *pdata, u16 *idx, u16 len);
static GM_ERRCODE protocol_pack_wifi_msg(u8 *pdata, u16 *idx, u16 len);


static GM_ERRCODE gps_service_transfer_status(u8 new_status)
{
    u8 old_status = (u8)s_gps_socket.status;
    GM_ERRCODE ret = GM_PARAM_ERROR;
    switch(s_gps_socket.status)
    {
        case SOCKET_STATUS_INIT:
            switch(new_status)
            {
                case SOCKET_STATUS_INIT:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_GET_HOST:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_CONNECTING:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_LOGIN:
                    break;
                case SOCKET_STATUS_WORK:
                    break;
                case SOCKET_STATUS_DATA_FINISH:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_ERROR:
                    ret = GM_SUCCESS;
                    break;
                default:
                    break;
            }
            break;
        case SOCKET_STATUS_GET_HOST:
            switch(new_status)
            {
                case SOCKET_STATUS_INIT:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_GET_HOST:
                    break;
                case SOCKET_STATUS_CONNECTING:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_LOGIN:
                    break;
                case SOCKET_STATUS_WORK:
                    break;
                case SOCKET_STATUS_DATA_FINISH:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_ERROR:
                    ret = GM_SUCCESS;
                    break;
                default:
                    break;
            }
            break;
        case SOCKET_STATUS_CONNECTING:
            switch(new_status)
            {
                case SOCKET_STATUS_INIT:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_GET_HOST:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_CONNECTING:
                    break;
                case SOCKET_STATUS_LOGIN:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_WORK:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_DATA_FINISH:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_ERROR:
                    ret = GM_SUCCESS;
                    break;
                default:
                    break;
            }
            break;
        case SOCKET_STATUS_LOGIN:
            switch(new_status)
            {
                case SOCKET_STATUS_INIT:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_GET_HOST:
                    break;
                case SOCKET_STATUS_CONNECTING:
                    break;
                case SOCKET_STATUS_LOGIN:
                    break;
                case SOCKET_STATUS_WORK:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_DATA_FINISH:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_ERROR:
                    ret = GM_SUCCESS;
                    break;
                default:
                    break;
            }
            break;
        case SOCKET_STATUS_WORK:
            switch(new_status)
            {
                case SOCKET_STATUS_INIT:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_GET_HOST:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_CONNECTING:
                    break;
                case SOCKET_STATUS_LOGIN:
                    break;
                case SOCKET_STATUS_WORK:
                    break;
                case SOCKET_STATUS_DATA_FINISH:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_ERROR:
                    ret = GM_SUCCESS;
                    break;
                default:
                    break;
            }
            break;
        case SOCKET_STATUS_DATA_FINISH:
            switch(new_status)
            {
                case SOCKET_STATUS_INIT:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_GET_HOST:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_CONNECTING:
                    break;
                case SOCKET_STATUS_LOGIN:
                    break;
                case SOCKET_STATUS_WORK:
                    break;
                case SOCKET_STATUS_DATA_FINISH:
                    break;
                case SOCKET_STATUS_ERROR:
                    ret = GM_SUCCESS;
                    break;
                default:
                    break;
            }
            break;
        case SOCKET_STATUS_ERROR:
            switch(new_status)
            {
                case SOCKET_STATUS_INIT:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_GET_HOST:
                    break;
                case SOCKET_STATUS_CONNECTING:
                    break;
                case SOCKET_STATUS_LOGIN:
                    break;
                case SOCKET_STATUS_WORK:
                    break;
                case SOCKET_STATUS_DATA_FINISH:
                    break;
                case SOCKET_STATUS_ERROR:
                    ret = GM_SUCCESS;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }


    if(GM_SUCCESS == ret)
    {
        s_gps_socket.status = new_status;
        s_gps_socket.status_fail_count = 0;
        LOG(INFO,"clock(%d) gps_service_transfer_status from %s to %s success", util_clock(), 
            gm_socket_status_string((SocketStatus)old_status), gm_socket_status_string((SocketStatus)new_status));
    }
    else
    {
        LOG(WARN,"clock(%d) gps_service_transfer_status assert(from %s to %s) failed", util_clock(), 
            gm_socket_status_string((SocketStatus)old_status), gm_socket_status_string((SocketStatus)new_status));
    }

    return ret;

}


/*
// have ip, use ip to connect
// no ip, call gm_socket_get_host_by_name 
*/
static void gps_service_init_proc(void)
{
    u8 IP[4];
    gm_socket_get_host_by_name_trigger(&s_gps_socket);
    system_state_get_ip_cache(SOCKET_INDEX_MAIN, IP);
    if(GM_SUCCESS == gm_is_valid_ip(IP))
    {
        GM_memcpy( s_gps_socket.ip , IP, sizeof(IP));
        gps_service_transfer_status(SOCKET_STATUS_CONNECTING);
        if(GM_SUCCESS == gm_socket_connect(&s_gps_socket))
        {
        }
        // else do nothing .   connecting_proc will deal.
    }
    else if((!s_gps_socket.excuted_get_host) && (GM_SUCCESS == gm_is_valid_ip(s_gps_socket.ip)))
    {
        gps_service_transfer_status(SOCKET_STATUS_CONNECTING);
        if(GM_SUCCESS == gm_socket_connect(&s_gps_socket))
        {
        }
        // else do nothing .   connecting_proc will deal.
    }
}

static void gps_service_connecting_proc(void)
{
    u32 current_time = util_clock();

    if((current_time - s_gps_socket.send_time) > CONNECT_TIME_OUT)
    {
        s_gps_socket.status_fail_count ++;
        gps_service_connection_failed();

        if(s_gps_socket.status == SOCKET_STATUS_CONNECTING && 
            s_gps_socket.status_fail_count < MAX_CONNECT_REPEAT)
        {
            if(GM_SUCCESS == gm_socket_connect(&s_gps_socket))
            {
                //do nothing. wait callback
            }
        }
        
    }
}



static void gps_service_login_proc(void)
{
    u32 current_time = util_clock();

    if((current_time - s_gps_socket.send_time) >= MESSAGE_TIME_OUT)
    {
        ++s_gps_socket.status_fail_count;
        if(s_gps_socket.status_fail_count >= GPS_LOGIN_MSG_FAIL_MAX)
        {
			JsonObject* p_json_log = json_create();
			json_add_string(p_json_log, "event", "login");
			json_add_string(p_json_log, "state", "failed");
			json_add_int(p_json_log, "fail count", s_gps_socket.status_fail_count);
			log_service_upload(WARN, p_json_log);
   
            system_state_set_gpss_reboot_reason("login failed.");
            gps_service_close();
            s_gps_socket_extend.data_finish_time = util_clock();
            gps_service_transfer_status(SOCKET_STATUS_DATA_FINISH);
        }
        else
        {
            if(GM_SUCCESS != protocol_send_login_msg(&s_gps_socket))
            {
                return;
            }
        }
    }

    protocol_msg_receive(&s_gps_socket);
}



void report_wifi_lbs(void)
{
	u8 work_mode = 0;
	config_service_get(CFG_WORKMODE, TYPE_BYTE, &work_mode, sizeof(work_mode));

	if (work_mode == 0 && gps_is_fixed())
	{
		wifi_destroy();
		return;
	}
	
	if (get_wifi_scan_ap_num() > 0)
	{
		gps_service_push_wifi();
	}
	else
	{
		gps_service_push_lbs();
	}
	wifi_destroy();
	s_gps_socket_extend.wifi_send_time = util_clock();
}



/*
    get msg from fifo
    if have no gps msg, sending heartbeat, else sending gpsmsg
*/
static void gps_service_work_proc(void)
{
    u32 current_time = util_clock();
    u16 value_u16;
    bool lbs_enable;
    u32 send_counts = 0;
    u8 work_mode = 0;
    u16 upload_time;
    u8 key_feature;
    u16 signal_interval;

	//服务模块工作状态并且不是休眠状态LED常亮
	if (GM_SYSTEM_STATE_WORK == system_state_get_work_state())
	{
		led_set_gsm_state(GM_LED_ON);
	}
    
    config_service_get(CFG_IS_LBS_ENABLE, TYPE_BOOL, &lbs_enable, sizeof(lbs_enable));
    config_service_get(CFG_WORKMODE, TYPE_BYTE, &work_mode, sizeof(work_mode));
	config_service_get(CFG_UPLOADTIME, TYPE_SHORT, &upload_time, sizeof(upload_time));
	if (upload_time > 0)
	{
	    if (hard_ware_is_device_w() && work_mode != 0)
	    {
			if ((s_gps_socket_extend.wifi_send_atonce == true) || ((current_time - s_gps_socket_extend.wifi_send_time) >= upload_time && VEHICLE_STATE_RUN == system_state_get_vehicle_state()))
	        {
	        	if (hard_ware_device_has_wifi())
	        	{
	            	wifi_create((PsFuncPtr)report_wifi_lbs);
	            }
	            else
	            {
	            	gps_service_push_lbs();
	            }
	            s_gps_socket_extend.wifi_send_atonce = false;
	            s_gps_socket_extend.wifi_send_time = current_time;
	        }
	    }
	    else if(lbs_enable && (!gps_is_fixed()))
	    {
	        u16 wait_gps_time = 0;
	        config_service_get(CFG_WAIT_GPS_TIME, TYPE_SHORT, &wait_gps_time, sizeof(wait_gps_time));
	        wait_gps_time = wait_gps_time < SECONDS_PER_MIN ? SECONDS_PER_MIN:wait_gps_time;
	        if((current_time > wait_gps_time) || (s_gps_socket_extend.lbs_send_time > 0))
	        {
	            //启动了足够久（注意一百多天归0） 或已经发过
	            config_service_get(CFG_LBS_INTERVAL, TYPE_SHORT, &value_u16, sizeof(value_u16));
	            if((current_time - s_gps_socket_extend.lbs_send_time) >= value_u16)
	            {
	                gps_service_push_lbs();
	            }
	        }
	    }
    }

    if(PROTOCOL_JT808 == config_service_get_app_protocol())
    {
        config_service_get(CFG_JT_HBINTERVAL, TYPE_SHORT, &value_u16, sizeof(value_u16));
        if(value_u16)  // != 0
        {
            if((current_time - s_gps_socket_extend.heart_locate_send_time) >= value_u16 || s_gps_socket_extend.heart_atonce)
            {
                gps_service_send_one_locate(GPS_MODE_FIX_TIME, false);
                s_gps_socket_extend.heart_locate_send_time = current_time;
				s_gps_socket_extend.heart_atonce = false;
            }
        }
    }

    config_service_get(CFG_HEART_INTERVAL, TYPE_SHORT, &value_u16, sizeof(value_u16));
    if((current_time - s_gps_socket_extend.heart_send_time) >= value_u16 || s_gps_socket_extend.heart_atonce)
    {
        protocol_send_heartbeat_msg(&s_gps_socket);
        s_gps_socket_extend.heart_send_time = current_time;
		s_gps_socket_extend.heart_atonce = false;
    }

    config_service_get(CFG_KEY_FEATURE, TYPE_BYTE, &key_feature, sizeof(u8));
	if (KEY_TEMP == key_feature)
	{
		if ((current_time - s_gps_socket_extend.temperature_send_time) >= value_u16 || s_gps_socket_extend.temperature_atonce)
	    {
	        protocol_send_temperature_msg(&s_gps_socket);
	        s_gps_socket_extend.temperature_send_time = current_time;
			s_gps_socket_extend.temperature_atonce = false;
			s_gps_socket_extend.last_temperature = hard_ware_get_temperature();
	    }
	}

	config_service_get(CFG_SIGNAL_INTERVAL, TYPE_SHORT, &signal_interval, sizeof(signal_interval));
	if (signal_interval > 0 && (current_time - s_gps_socket_extend.signal_send_time) >= signal_interval)
	{
		protocol_send_signal_msg(&s_gps_socket);
		s_gps_socket_extend.signal_send_time = current_time;
	}
	
    /*
        连接建立成功2次心跳间隔后才检测心跳有没有超时
    */
    if((current_time - s_gps_socket_extend.connection_time) > (2*value_u16)
        && (current_time - s_gps_socket_extend.heart_receive_time) >= GPS_HEART_MSG_RECEIVE_TIME_OUT)
    {
        LOG(INFO,"clock(%d) gps_service_work_proc heart break. last heart time:%d", util_clock(), s_gps_socket_extend.heart_receive_time);
        system_state_set_gpss_reboot_reason("heart broken.");
        gps_service_close();
        s_gps_socket_extend.data_finish_time = util_clock();
        gps_service_transfer_status(SOCKET_STATUS_DATA_FINISH);
        return;
    }

    send_counts = protocol_send_gps_msg(&s_gps_socket);
    s_gps_socket_extend.send_location_counts += send_counts;

    protocol_msg_receive(&s_gps_socket);
}


void gps_service_send_result(bool result)
{
	if (result)
	{
		s_gps_socket.send_time = util_clock();
	}
	else
	{
		system_state_set_gpss_reboot_reason("gm_socket_send cache_send");
        gps_service_destroy_gprs();
	}
}


void gps_service_connection_ok(void)
{
    u32 current_time = util_clock();
    s_gps_socket_extend.connection_time = current_time;
    gps_service_transfer_status(SOCKET_STATUS_LOGIN);

    protocol_send_login_msg(&s_gps_socket);
    
}

void gps_service_close_ok(void)
{
	s_gps_socket.id=-1;
    s_gps_socket_extend.saved_socket_ack = s_gps_socket.last_ack_seq = 0;
    switch(s_gps_socket.status)
    {
        case SOCKET_STATUS_INIT:
            break;
        case SOCKET_STATUS_CONNECTING:
            if(s_gps_socket.status_fail_count >= MAX_CONNECT_REPEAT)
		    {
		        // if excuted get_host reinit gprs, else get_host.
		        if(s_gps_socket.excuted_get_host || (s_gps_socket.addr[0] == 0))
		        {
		            //  reinit gprs
		            LOG(DEBUG,"clock(%d) gps_service_connection_failed excuted_get_host(%d)",util_clock(),s_gps_socket.excuted_get_host);
		            gps_service_destroy_gprs();
		        }
		        else
		        {
		            gps_service_transfer_status(SOCKET_STATUS_INIT);
		        }
		    }
            break;
        default:
			//socket断开后立即重连，不再等待60秒
            //s_gps_socket_extend.data_finish_time = util_clock();
        	gps_service_transfer_status(SOCKET_STATUS_DATA_FINISH);
            break;
    }
}


void gps_service_close_for_reconnect(void)
{
    gps_service_close();
    s_gps_socket_extend.data_finish_time = util_clock();
    gps_service_transfer_status(SOCKET_STATUS_DATA_FINISH);
}

void gps_service_connection_failed(void)
{
    system_state_set_gpss_reboot_reason("connect failed.");
    gps_service_close();
    
    if(s_gps_socket.status_fail_count >= MAX_CONNECT_REPEAT)
    {
        // if excuted get_host reinit gprs, else get_host.
        if(s_gps_socket.excuted_get_host || (s_gps_socket.addr[0] == 0))
        {
            //  reinit gprs
            LOG(DEBUG,"clock(%d) gps_service_connection_failed excuted_get_host(%d)",util_clock(),s_gps_socket.excuted_get_host);
            gps_service_destroy_gprs();
        }
        else
        {
            gps_service_transfer_status(SOCKET_STATUS_INIT);
        }
    }
    //else do nothing. wait gps_service_connecting_proc to deal
}


GM_ERRCODE gps_service_create(bool first_create)
{
    u8 addr[2*GOOME_DNS_MAX_LENTH+1] = {0};
    u8 IP[4] = {0};
    u32 port = 0;
    u8 idx = 0;
    GM_ERRCODE ret = GM_SUCCESS;
    u8 imei[GM_IMEI_LEN + 1] = {0};
	
    if(GM_SUCCESS != (ret = gsm_get_imei(imei)))
    {
        return ret;
    }

    /*because s_gps_socket.fifo.base_addr is not initialized, here depend on first_create
    to check base_addr is null or not*/
    if(!first_create)
    {
        if(s_gps_socket.fifo.base_addr)
        {
            return GM_SUCCESS;
        }
    }
	
    gm_socket_init(&s_gps_socket, SOCKET_INDEX_MAIN);

    GM_memset(addr, 0x00, sizeof(addr));

    if(config_service_is_test_mode() || config_service_is_default_imei())
    {
        idx = GM_sscanf((const char *)config_service_get_pointer(CFG_TEST_SERVERADDR), "%[^:]:%d", addr, &port);
        if (idx != 2)
        {
            LOG(WARN,"clock(%d) gps_service_create assert(idx ==2) failed.", util_clock());
            return GM_PARAM_ERROR;
        }
		LOG(INFO,"is_default_imei,use test server");
    }
    else
    {
        idx = GM_sscanf((const char *)config_service_get_pointer(CFG_SERVERADDR), "%[^:]:%d", addr, &port);
        if (idx != 2)
        {
            LOG(WARN,"clock(%d) gps_service_create assert(idx ==2) failed.", util_clock());
            return GM_PARAM_ERROR;
        }
		LOG(INFO,"is not default imei,use main server");
    }

	//是域名
    if(GM_SUCCESS != GM_ConvertIpAddr(addr, IP))
    {
        if(util_is_valid_dns(addr, GM_strlen((const char *)addr)))
        {
            gm_socket_set_addr(&s_gps_socket, addr, GM_strlen((const char *)addr), port, STREAM_TYPE_STREAM);
	        system_state_get_ip_cache(SOCKET_INDEX_MAIN, IP);
            gm_socket_set_ip_port(&s_gps_socket, IP, port, STREAM_TYPE_STREAM);
        }
        else
        {
            LOG(WARN,"clock(%d) gps_service_create assert(dns(%s)) failed.", util_clock(), addr);
            return GM_PARAM_ERROR;
        }
    }
	//是IP
    else
    {
        gm_socket_set_ip_port(&s_gps_socket, IP, port, STREAM_TYPE_STREAM);
        if (!config_service_is_test_mode())
        {
        	system_state_set_ip_cache(SOCKET_INDEX_MAIN, IP);
        }
    }


	s_gps_socket_extend.heart_atonce = false;
    s_gps_socket_extend.heart_send_time = 0;
    s_gps_socket_extend.heart_locate_send_time = 0;
	s_gps_socket_extend.receive_heart_counts = 0;
	s_gps_socket_extend.send_location_counts = 0;
    s_gps_socket_extend.data_finish_time = 0;
    s_gps_socket_extend.saved_socket_ack = 0;
    s_gps_socket_extend.lbs_send_time = 0;
    s_gps_socket_extend.wifi_send_time = 0;
    s_gps_socket_extend.wifi_send_atonce = false;

    LOG(INFO,"clock(%d) gps_service_create access_id(%d) fifo(%p).", util_clock(), s_gps_socket.access_id, &s_gps_socket.fifo);

	return GM_SUCCESS;
}



GM_ERRCODE gps_service_change_config(void)
{
    if (! gps_service_socket_is_same())
    {
        u8 value_u8 = 0;
        system_state_set_gpss_reboot_reason("change config.");
        gps_service_destroy();
        
        // clear previous connection info.
        s_gps_socket.ip[0] = s_gps_socket.ip[1] = s_gps_socket.ip[2] = s_gps_socket.ip[3] = 0;
        s_gps_socket.addr[0] = 0;
        system_state_set_ip_cache(SOCKET_INDEX_MAIN, s_gps_socket.ip);
        config_service_set(CFG_JT_ISREGISTERED, TYPE_BOOL, &value_u8, sizeof(value_u8));
        
        gps_service_create(false);

		system_state_set_reported_gps_since_modify_ip(false);
    }
    return GM_SUCCESS;
}


GM_ERRCODE gps_service_destroy(void)
{
    if(SOCKET_STATUS_ERROR == s_gps_socket.status)
    {
        return GM_SUCCESS;
    }
    
    gps_service_close();
    led_set_gsm_state(GM_LED_FLASH);

    fifo_delete(&s_gps_socket.fifo);

    s_gps_socket_extend.data_finish_time = util_clock();
    gps_service_transfer_status(SOCKET_STATUS_ERROR);
    
	return GM_SUCCESS;
}

static void gps_service_close(void)
{
    gps_service_save_to_history_file();
	if(s_gps_socket.id >=0)
	{
		if (hard_ware_is_at_command())
		{
			if (util_clock() - s_gps_socket.at_close_clock > 1)
			{
				at_command_close_connect(s_gps_socket.access_id);
				s_gps_socket.at_close_clock = util_clock();
			}
		}
		else 
		{
			GM_SocketClose(s_gps_socket.id);
			s_gps_socket.id=-1;
		}
   	 	s_gps_socket_extend.saved_socket_ack = s_gps_socket.last_ack_seq = 0;
	}
}


static void gps_service_data_finish_proc(void)
{
    u32 current_time = util_clock();

	if(gprs_check_need_reboot(s_gps_socket_extend.heart_receive_time))
	{
        //以下日志如果放开,会打印过多.(10ms一次)
        //LOG(INFO,"clock(%d) gps_service_data_finish_proc heart break. last heart time:%d", util_clock(), s_gps_socket_extend.heart_receive_time);
        return;
	}

    if(! s_gps_socket.fifo.base_addr)
    {
        return;
    }

    if((current_time - s_gps_socket_extend.data_finish_time) > GPS_PING_TIME)
    {
        LOG(DEBUG,"clock(%d) gps_service_data_finish_proc cur(%d) - fin(%d) > GPS_PING_TIME(%d).",
            util_clock(), current_time, s_gps_socket_extend.data_finish_time,GPS_PING_TIME);

        // 可以重建连接
        gps_service_transfer_status(SOCKET_STATUS_INIT);
    }
}


GM_ERRCODE gps_service_timer_proc(void)
{
    // 如果gprs成功, 但socket无法收发数据, 则heart_receive_time 不会更新
	if(gprs_check_need_reboot(s_gps_socket_extend.heart_receive_time))
	{
        // 如果未初始化, 相当于gprs_check_need_reboot(0) 即检测启动了21分钟.
        gprs_destroy();
        return GM_ERROR_STATUS;
	}

    if(!s_gps_socket.fifo.base_addr)
    {
        return GM_SUCCESS;
    }

    switch(s_gps_socket.status)
    {
        case SOCKET_STATUS_INIT:
            gps_service_init_proc();
            break;
        case SOCKET_STATUS_CONNECTING:
            gps_service_connecting_proc();
            break;
        case SOCKET_STATUS_LOGIN:
            gps_service_login_proc();
            break;
        case SOCKET_STATUS_WORK:
            gps_service_work_proc();
            break;
        case SOCKET_STATUS_DATA_FINISH:
            gps_service_data_finish_proc();
            break;
        case SOCKET_STATUS_ERROR:
            gps_service_data_finish_proc();
            break;
        default:
        {
            char reason[30];
            LOG(WARN,"clock(%d) gps_service_timer_proc assert(s_gps_socket.status(%d)) unknown.",util_clock(), s_gps_socket.status);

            //  reinit gprs
            snprintf(reason,sizeof(reason),"unknown status %d", s_gps_socket.status);
            reason[sizeof(reason) - 1] = 0;
            system_state_set_gpss_reboot_reason(reason);
            gps_service_destroy_gprs();
            return GM_ERROR_STATUS;
        }
    }

    return GM_SUCCESS;
}



u8 gps_service_socket_is_same(void)
{
    u8 addr[CONFIG_STRING_MAX_LEN+1];
    u8 IP[4];
    u32 port = 0;
    u8 idx = 0;

    GM_memset(addr, 0x00, sizeof(addr));
    idx = GM_sscanf((const char *)config_service_get_pointer(CFG_SERVERADDR), "%[^:]:%d", addr, &port);
    if (idx != 2)
    {
        LOG(WARN,"clock(%d) gps_service_socket_is_same assert(idx ==2) failed.",util_clock());
        return 1; // not change.
    }

    if(s_gps_socket.port != port)
    {
        return 0;
    }
    
    if(GM_SUCCESS == GM_ConvertIpAddr(addr, IP))
    {
        if(s_gps_socket.ip[0] == IP[0] && s_gps_socket.ip[1] == IP[1] && 
            s_gps_socket.ip[2] == IP[2] && s_gps_socket.ip[3] == IP[3])
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        if(0 == GM_strcmp(s_gps_socket.addr, (const char *)addr))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    // return 1;
}




static GM_ERRCODE protocol_pack_gps_msg(GpsDataModeEnum mode, const GPSData *gps, u8 *pdata, u16 *idx, u16 len)
{
    switch(config_service_get_app_protocol())
    {
    case PROTOCOL_GOOME:
        protocol_goome_pack_gps_msg(mode, gps, pdata, idx, len);  //17 bytes
        break;
    case PROTOCOL_CONCOX:
        protocol_concox_pack_gps_msg(mode, gps, pdata, idx, len); // 39 bytes
        break;
    case PROTOCOL_JT808:
        protocol_jt_pack_gps_msg(mode, gps, pdata, idx, len);  // 53 bytes
        break;
    default:
        LOG(WARN,"clock(%d) protocol_pack_gps_msg assert(app protocol(%d)) failed.",util_clock(), config_service_get_app_protocol());
        return GM_SYSTEM_ERROR;
    }
    return GM_SUCCESS;
}

GM_ERRCODE gps_service_cache_send(u8 *data, u8 len)
{
	GM_ERRCODE ret;
    gps_service_confirm_gps_cache(&s_gps_socket);

    if(GM_SUCCESS == gps_service_save_to_cache(data, len))
    {
        if(config_service_get_app_protocol() == PROTOCOL_JT808)
        {
            //转义后带基站的消息可能超过100字节, 所以在此处转义
            u8 buff[HIS_FILE_FRAME_SIZE*2];
            u16 buf_len = sizeof(buff);
            protocol_jt_pack_escape(data, len, buff, &buf_len);
            ret = gm_socket_send(&s_gps_socket, buff, buf_len);
		
            if(GM_SUCCESS == ret)
            {
                s_gps_socket.send_time = util_clock();
                return GM_SUCCESS;
            }
            else if (GM_MEM_NOT_ENOUGH != ret)
            {
                system_state_set_gpss_reboot_reason("gm_socket_send cache_send");
                gps_service_destroy_gprs();
                return GM_NET_ERROR;
            }
        }
        else
        {
        	ret = gm_socket_send(&s_gps_socket, data, len);
		
            if(GM_SUCCESS == ret)
            {
                s_gps_socket.send_time = util_clock();
                return GM_SUCCESS;
            }
            else if (GM_MEM_NOT_ENOUGH != ret)
            {
                system_state_set_gpss_reboot_reason("gm_socket_send cache_send");
                gps_service_destroy_gprs();
                return GM_NET_ERROR;
            }
        }
    }
    return GM_MEM_NOT_ENOUGH;
}

void gps_service_confirm_gps_cache(SocketType *socket)
{
	if (!hard_ware_is_at_command())
	{
		//AT指令不支持获取ack_seq，发送成功直接累加
		gm_socket_get_ackseq(socket, &socket->last_ack_seq);
	}
    if(socket->access_id != SOCKET_INDEX_MAIN)
    {
        return;
    }
    
    gps_service_confirm_cache(&s_gps_socket_extend.saved_socket_ack, s_gps_socket.last_ack_seq);
}


GM_ERRCODE gps_service_push_gps(GpsDataModeEnum mode, const GPSData *gps)
{
    u8 buff[HIS_FILE_FRAME_SIZE];
    u16 len = sizeof(buff);
    u16 idx = 0;
    bool gps_close = false;
   

    if(NULL == gps)
    {
        return GM_PARAM_ERROR;
    }

	LOG(INFO,"Report GPS,LAT:%f,LNG:%f,ALT:%f,SPEED:%f,COURSE:%f,SATES:%d,HDOP:%f,SIGNAL:%d",
		gps->lat,
		gps->lng,
		gps->alt,
		gps->speed,
		gps->course,
		gps->satellites_tracked,
		gps->hdop,
		gps->signal_intensity_grade);

    config_service_get(CFG_GPS_CLOSE, TYPE_BOOL, &gps_close, sizeof(gps_close));
    if(gps_close)
    {
        LOG(DEBUG,"clock(%d) gps_service_push_gps not enabled.",util_clock());
        return GM_ERROR_STATUS;
    }
    
    system_state_set_last_gps(gps);
    if(GM_SUCCESS != protocol_pack_gps_msg(mode, gps, buff, &idx, len))
    {
        return GM_SYSTEM_ERROR;
    }

    len=idx;  // idx is msg len

    if(SOCKET_STATUS_WORK == s_gps_socket.status)
    {
        LOG(DEBUG,"clock(%d) gps_service_push_gps msglen(%d)", util_clock(), idx);
        if(GM_SUCCESS == gps_service_cache_send(buff, idx))
        {	
        	s_gps_socket_extend.send_location_counts++;
            return GM_SUCCESS;
        }
    }

    return gps_service_push_to_stack(buff, idx);
}

void gps_service_heart_atonce(void)
{
	LOG(INFO,"gps_service_heart_atonce");
	s_gps_socket_extend.heart_atonce = true;
}

void gps_service_wifi_atonce(void)
{
	LOG(INFO,"gps_service_wifi_atonce");
	s_gps_socket_extend.wifi_send_atonce = true;
}

GM_ERRCODE gps_service_send_one_locate(GpsDataModeEnum mode, bool use_lbs)
{
    u8 buff[HIS_FILE_FRAME_SIZE];
    u16 len = sizeof(buff);
    u16 idx = 0;
    GPSData gps;
    GM_ERRCODE ret;

    LOG(DEBUG,"clock(%d) gps_service_send_one_locate mode(%d) use_lbs(%d).",util_clock(), mode, use_lbs);
    if(s_gps_socket.status != SOCKET_STATUS_LOGIN && s_gps_socket.status != SOCKET_STATUS_WORK)
    {
        LOG(WARN,"clock(%d) gps_service_send_one_locate socket->status(%s) error.", util_clock(), gm_socket_status_string((SocketStatus)s_gps_socket.status));
        return GM_PARAM_ERROR;
    }
    
    //超越平台 不认心跳包,没有GPS包就会显示掉线
    if(use_lbs)
    {
        if(!gps_is_fixed())
        {
            //未定位, 发LBS
            if(GM_SUCCESS != protocol_pack_lbs_msg(buff, &idx, len))
            {
                return GM_SYSTEM_ERROR;
            }
            
            len=idx;  // idx is msg len
            
            if(SOCKET_STATUS_WORK == s_gps_socket.status)
            {
                LOG(DEBUG,"clock(%d) gps_service_send_one_locate lbs msglen(%d)", util_clock(), idx);
                ret = gm_socket_send(&s_gps_socket, buff, idx);
			
                if(GM_SUCCESS == ret)
                {
                    s_gps_socket.send_time = util_clock();
                    return GM_SUCCESS;
                }
                else if (GM_MEM_NOT_ENOUGH != ret)
                {
                    system_state_set_gpss_reboot_reason("gm_socket_send one_locate");
                    gps_service_destroy_gprs();
                }
            }
            
            return GM_SUCCESS;
        }
    }

    //已定位,或不用lbs
    GM_memset(&gps,0,sizeof(gps));
    gps_get_last_data(&gps);
	gps_find_optimal_data_by_time_and_hdop(&gps);
    if(gps.gps_time == (time_t)0)
    {
        system_state_get_last_gps(&gps);
    }
    
    if(gps.gps_time == (time_t)0)
    {
        gps.lat = agps_service_get_unfix_lat();
        gps.lng = agps_service_get_unfix_lng();

        if(gps.lat < 0.00001f)
        {
            // prevent send not located position.
            return GM_SUCCESS;
        }

    }

    //use current time.
    gps.gps_time = util_get_utc_time();
	//如果静止状态，速度设置为0
	if (VEHICLE_STATE_STATIC == system_state_get_vehicle_state())
	{
		gps.speed = 0;
	}

    if(GM_SUCCESS != protocol_pack_gps_msg(mode, &gps, buff, &idx, len))
    {
        return GM_SYSTEM_ERROR;
    }

    len=idx;  // idx is msg len

    if(SOCKET_STATUS_WORK == s_gps_socket.status)
    {
        LOG(DEBUG,"clock(%d) gps_service_send_one_locate gps msglen(%d)", util_clock(), idx);
        ret = gm_socket_send(&s_gps_socket, buff, idx);
	
        if(GM_SUCCESS == ret)
        {
            s_gps_socket.send_time = util_clock();
            return GM_SUCCESS;
        }
        else if (GM_MEM_NOT_ENOUGH != ret)
        {
            system_state_set_gpss_reboot_reason("gm_socket_send one_locate");
            gps_service_destroy_gprs();
        }
    }

    return GM_SUCCESS;
}


GM_ERRCODE gps_service_temperature_atonce(void)
{
	LOG(INFO,"gps_service_temperature_atonce");
	s_gps_socket_extend.temperature_atonce = true;
	return GM_SUCCESS;
}



static GM_ERRCODE protocol_pack_lbs_msg(u8 *pdata, u16 *idx, u16 len)
{
    switch(config_service_get_app_protocol())
    {
    case PROTOCOL_GOOME:
        protocol_goome_pack_lbs_msg(pdata, idx, len);  //47 bytes
        break;
    case PROTOCOL_CONCOX:
        protocol_concox_pack_lbs_msg(pdata, idx, len); //63 bytes
        break;
    case PROTOCOL_JT808:
        protocol_jt_pack_lbs_msg(pdata, idx, len);  // max 97 bytes
        break;
    default:
        LOG(WARN,"clock(%d) protocol_pack_lbs_msg assert(app protocol(%d)) failed.",util_clock(), config_service_get_app_protocol());
        return GM_SYSTEM_ERROR;
    }
    return GM_SUCCESS;
}



GM_ERRCODE gps_service_push_lbs(void)
{
    u8 buff[HIS_FILE_FRAME_SIZE];
    u16 len = sizeof(buff);
    u16 idx = 0;

    s_gps_socket_extend.lbs_send_time = util_clock();
    
    if(GM_SUCCESS != protocol_pack_lbs_msg(buff, &idx, len))
    {
        return GM_SYSTEM_ERROR;
    }

    len=idx;  // idx is msg len

    if(SOCKET_STATUS_WORK == s_gps_socket.status)
    {
        LOG(DEBUG,"clock(%d) gps_service_push_lbs msglen(%d)", util_clock(), idx);
        if(GM_SUCCESS == gps_service_cache_send(buff, idx))
        {
            return GM_SUCCESS;
        }
    }

    return gps_service_push_to_stack(buff, idx);
}


static GM_ERRCODE protocol_pack_wifi_msg(u8 *pdata, u16 *idx, u16 len)
{
    switch(config_service_get_app_protocol())
    {
    case PROTOCOL_GOOME:
        protocol_goome_pack_wifi_msg(pdata, idx, len);  //82 bytes
        break;
    case PROTOCOL_CONCOX:
    case PROTOCOL_JT808:
    default:
        LOG(WARN,"clock(%d) protocol_pack_wifi_msg assert(app protocol(%d)) failed.",util_clock(), config_service_get_app_protocol());
        return GM_SYSTEM_ERROR;
    }
    return GM_SUCCESS;
}



GM_ERRCODE gps_service_push_wifi(void)
{
	u8 buff[HIS_FILE_FRAME_SIZE];
	u16 len = sizeof(buff);
	u16 idx = 0;

	s_gps_socket_extend.wifi_send_time = util_clock();
	
	if(GM_SUCCESS != protocol_pack_wifi_msg(buff, &idx, len))
	{
		return GM_SYSTEM_ERROR;
	}

	len=idx;  // idx is msg len

	if(SOCKET_STATUS_WORK == s_gps_socket.status)
	{
		LOG(DEBUG,"clock(%d) gps_service_push_wifi msglen(%d)", util_clock(), idx);
		if(GM_SUCCESS == gps_service_cache_send(buff, idx))
		{
			return GM_SUCCESS;
		}
	}

	return gps_service_push_to_stack(buff, idx);
}


#ifdef _SW_SUPPORT_RECORD_
static GM_ERRCODE protocol_pack_recorder_response_msg(bool start, u8 *pdata, u16 *idx, u16 len)
{
    switch(config_service_get_app_protocol())
    {
    case PROTOCOL_GOOME:
        protocol_goome_pack_recorder_response_msg(start, pdata, idx, len);  //47 bytes
        break;
    case PROTOCOL_CONCOX:
    case PROTOCOL_JT808:
    default:
        LOG(WARN,"clock(%d) protocol_pack_lbs_msg assert(app protocol(%d)) failed.",util_clock(), config_service_get_app_protocol());
        return GM_SYSTEM_ERROR;
    }
    return GM_SUCCESS;
}



GM_ERRCODE gps_service_push_recorder_response_state(bool start)
{
	u8 buff[HIS_FILE_FRAME_SIZE];
	u16 len = sizeof(buff);
	u16 idx = 0;

	s_gps_socket_extend.recorder_send_time = util_clock();
	
	if(GM_SUCCESS != protocol_pack_recorder_response_msg(start, buff, &idx, len))
	{
		return GM_SYSTEM_ERROR;
	}

	len=idx;  // idx is msg len

	if(SOCKET_STATUS_WORK == s_gps_socket.status)
	{
		LOG(DEBUG,"clock(%d) gps_service_push_recorder_response_state msglen(%d)", util_clock(), idx);
		if(GM_SUCCESS == gps_service_cache_send(buff, idx))
		{
			return GM_SUCCESS;
		}
	}

	return gps_service_push_to_stack(buff, idx);
}



static GM_ERRCODE protocol_pack_recorder_file_msg(void *arg, u8 *pdata, u16 *idx, u16 len)
{
    switch(config_service_get_app_protocol())
    {
    case PROTOCOL_GOOME:
        protocol_goome_pack_recorder_file_msg(arg, pdata, idx, len);
        break;
    case PROTOCOL_CONCOX:
    case PROTOCOL_JT808:
    default:
        LOG(WARN,"clock(%d) protocol_pack_lbs_msg assert(app protocol(%d)) failed.",util_clock(), config_service_get_app_protocol());
        return GM_SYSTEM_ERROR;
    }
    return GM_SUCCESS;
}



GM_ERRCODE gps_service_send_one_recorder_file_pack(void *arg)
{
    u8 *buff;
    //u16 len = 0;
    u16 idx = 0;

    LOG(DEBUG,"clock(%d) gps_service_send_one_recorder_file_pack.",util_clock());
    if(s_gps_socket.status != SOCKET_STATUS_LOGIN && s_gps_socket.status != SOCKET_STATUS_WORK)
    {
        LOG(WARN,"clock(%d) gps_service_send_one_recorder_file_pack socket->status(%s) error.", util_clock(), gm_socket_status_string((SocketStatus)s_gps_socket.status));
        return GM_PARAM_ERROR;
    }

    buff = GM_MemoryAlloc(RECD_DATA_MAX_LEN);
    if (!buff)
    {
        LOG(WARN,"clock(%d) gps_service_send_one_recorder_file_pack GM_MemoryAlloc fail.", util_clock());
        return GM_SYSTEM_ERROR;
    }

    GM_memset(buff, 0x00, RECD_DATA_MAX_LEN);

    if(GM_SUCCESS != protocol_pack_recorder_file_msg(arg, buff, &idx, RECD_DATA_MAX_LEN))
	{
		GM_MemoryFree(buff);
		buff = NULL;
		return GM_SYSTEM_ERROR;
	}
    
    //len=idx;  // idx is msg len

    if(SOCKET_STATUS_WORK == s_gps_socket.status)
    {
        LOG(DEBUG,"clock(%d) gps_service_send_one_recorder_file_pack msglen(%d)", util_clock(), idx);
        if(GM_SUCCESS == gm_socket_send(&s_gps_socket, buff, idx))
        {
            s_gps_socket.send_time = util_clock();
            GM_MemoryFree(buff);
			buff = NULL;
			return GM_SUCCESS;
        }
        /*
        else
        {
            system_state_set_gpss_reboot_reason("gm_socket_send one_recorder_file");
            gps_service_destroy_gprs();
        }*/
    }

    
    GM_MemoryFree(buff);
	buff = NULL;
	
    return GM_MEM_NOT_ENOUGH;
}
#endif



static GM_ERRCODE protocol_pack_alarm_msg(AlarmInfo *alarm,u8 *pdata, u16 *idx, u16 len)
{
    switch(config_service_get_app_protocol())
    {
    case PROTOCOL_GOOME:
        protocol_goome_pack_alarm_msg(alarm, pdata, idx, len);  //35 bytes
        break;
    case PROTOCOL_CONCOX:
        protocol_concox_pack_alarm_msg(alarm, pdata, idx, len);   //42 bytes
        break;
    case PROTOCOL_JT808:
        protocol_jt_pack_gps_msg2(pdata, idx, len);  // 53 bytes
        break;
    default:
        LOG(WARN,"clock(%d) protocol_pack_alarm_msg assert(app protocol(%d)) failed.", util_clock(), config_service_get_app_protocol());
        return GM_SYSTEM_ERROR;
    }
    return GM_SUCCESS;
}

/**
 * 短信报警
*/
static GM_ERRCODE gps_service_push_sms_alarm(AlarmInfo *alarm)
{
    //GM_ERRCODE content;
    u16 lencont = 0;
    u8 imei[GM_IMEI_LEN + 1] = {0};
    char *number = NULL;
    u8 number_index = 0;
    u8 buff[1024] = {0};
    u8 zone = 0;
    u8 bcd_tim[6] = {0};
    ST_Time current_time;
    GPSData gps;
    //获取imei
    gsm_get_imei(imei);
    //获取本地时间
    zone = config_service_get_zone();
    util_get_current_local_time(bcd_tim, &current_time, zone);
    //GPS
    GM_memset(&gps,0,sizeof(gps));
    gps_get_last_data(&gps);
    if(gps.gps_time == (time_t)0)
    {
        system_state_get_last_gps(&gps);
    }
    
    if(gps.gps_time == (time_t)0)
    {
        gps.lat = agps_service_get_unfix_lat();
        gps.lng = agps_service_get_unfix_lng();
		gps.alt = 0;
    }

    lencont += GM_snprintf((char*)buff, 1023,"IMEI:%s,Time:%d-%02d-%02d %02d:%02d:%02d,AlarmType:", imei,current_time.year,current_time.month,current_time.day,current_time.hour,current_time.minute,current_time.second);
    
    
    switch (alarm->type)
    {
        case ALARM_POWER_OFF:
            lencont += GM_snprintf((char*)&buff[lencont],1023-lencont, "POWER OFF");
            break;
        case ALARM_BATTERY_LOW:
            lencont += GM_snprintf((char*)&buff[lencont],1023-lencont, "BATTERY LOW");
            break;
        case ALARM_SHOCK:
            lencont += GM_snprintf((char*)&buff[lencont],1023-lencont, "SHOCK");
            break;
        case ALARM_MOVE:
            lencont += GM_snprintf((char*)&buff[lencont],1023-lencont, "MOVE");
            break;
        case ALARM_SPEED:
            lencont += GM_snprintf((char*)&buff[lencont],1023-lencont, "SPEED");
            break;
        case ALARM_FAKE_CELL:
            lencont += GM_snprintf((char*)&buff[lencont],1023-lencont, "FAKE CELL");
            break;
        case ALARM_POWER_HIGH:
            lencont += GM_snprintf((char*)&buff[lencont],1023-lencont, "POWER HIGH");
            break;
        case ALARM_COLLISION:
            lencont += GM_snprintf((char*)&buff[lencont],1023-lencont, "COLLISION");
            break;
        case ALARM_SPEED_UP:
            lencont += GM_snprintf((char*)&buff[lencont],1023-lencont, "SPEED UP");
            break;
        case ALARM_SPEED_DOWN:
            lencont += GM_snprintf((char*)&buff[lencont],1023-lencont, "SPEED DOWN");
            break;
        case ALARM_TURN_OVER:
            lencont += GM_snprintf((char*)&buff[lencont],1023-lencont, "TURN OVER");
            break;
        case ALARM_SHARP_TURN:
            lencont += GM_snprintf((char*)&buff[lencont],1023-lencont, "SHARP TURN");
            break;
        case ALARM_SOS:
            lencont += GM_snprintf((char*)&buff[lencont],1023-lencont, "SOS");
            break;
        default:
            lencont += GM_snprintf((char*)&buff[lencont],1023-lencont, "REMOVE");
            break;
    }

    
    if(fabs(gps.lat) < 0.00001f)
    {
        // prevent send not located position.
        lencont += GM_snprintf((char*)&buff[lencont],1023-lencont, ",Address:null");
    }
    else
    {
        u8 ns = 'N';
        u8 ew = 'E';
        if(gps.lat<0)
        {
            ns ='S';
        }
        if(gps.lng<0)
        {
            ew = 'W';
        }
        lencont += GM_snprintf((char*)&buff[lencont],1023-lencont, ",Address:<%02d-%02d %02d:%02d>",current_time.month,current_time.day,current_time.hour,current_time.minute);
        lencont += GM_snprintf((char*)&buff[lencont],1023-lencont, "http://maps.google.com/maps?q=%c%.6f,%c%.6f",ns,fabs(gps.lat),ew,fabs(gps.lng));
    }

	for(number_index=0; number_index<3; number_index++)
	{
		number = config_service_get_pointer((ConfigParamEnum)(CFG_USER1_NUMBER+number_index));
	    if (GM_strlen(number) >= 3)
	    {
	    	sms_send((char*)buff,lencont,number,GM_DEFAULT_DCS);
	    }
	}

    return GM_SUCCESS;
}

GM_ERRCODE gps_service_push_alarm(AlarmInfo *alarm)
{
    u8 buff[HIS_FILE_FRAME_SIZE];
    u16 len = sizeof(buff);
    u16 idx = 0;

    gps_service_push_sms_alarm(alarm);

    //不定位不报警,很可能造成断电报警丢失
	//先报上去,等GPS定位了也会上报位置的——王志华

    if(GM_SUCCESS != protocol_pack_alarm_msg(alarm, buff, &idx, len))
    {
    	LOG(ERROR,"Failed to protocol_pack_alarm_msg!");
        return GM_SYSTEM_ERROR;
    }

    len=idx;

    if(SOCKET_STATUS_WORK == s_gps_socket.status)
    {
        LOG(DEBUG,"clock(%d) gps_service_push_alarm msglen(%d) alarm(%d)", util_clock(), len,alarm->type);
        if(GM_SUCCESS == gps_service_cache_send(buff, idx))
        {
            return GM_SUCCESS;
        }
    }

    return gps_service_push_to_stack(buff, idx);
}


static GM_ERRCODE protocol_pack_device_state_msg(u8 *pdata, u16 *idx, u16 len)
{
    switch(config_service_get_app_protocol())
    {
    case PROTOCOL_GOOME:
        protocol_goome_pack_device_state_msg(pdata, idx, len);
        break;
    default:
        LOG(WARN,"clock(%d) protocol_pack_device_state_msg assert(app protocol(%d)) failed.", util_clock(), config_service_get_app_protocol());
        return GM_SYSTEM_ERROR;
    }
    return GM_SUCCESS;
}



GM_ERRCODE gps_service_report_device_state(void)
{
	u8 buff[HIS_FILE_FRAME_SIZE];
	u16 len = sizeof(buff);
	u16 idx = 0;

	if(GM_SUCCESS != protocol_pack_device_state_msg(buff, &idx, len))
	{
		LOG(ERROR,"Failed to gps_service_report_device_state!");
		return GM_SYSTEM_ERROR;
	}

	len=idx;

	if(SOCKET_STATUS_WORK == s_gps_socket.status)
	{
		LOG(DEBUG,"clock(%d) gps_service_report_device_state msglen(%d)", util_clock(), len);
		if(GM_SUCCESS == gps_service_cache_send(buff, idx))
		{
			return GM_SUCCESS;
		}
	}

	return gps_service_push_to_stack(buff, idx);
}



static GM_ERRCODE protocol_pack_position_request_msg(u8 *mobile_num, u8 num_len, u8 *command, u8 cmd_len,u8 *pdata, u16 *idx, u16 len)
{
    switch(config_service_get_app_protocol())
    {
    case PROTOCOL_GOOME:
        protocol_goome_pack_position_request_msg(mobile_num,num_len,command,cmd_len, pdata, idx, len);  //53+cmd_len bytes
        break;
    case PROTOCOL_CONCOX:
        protocol_concox_pack_position_request_msg(mobile_num,num_len,command,cmd_len, pdata, idx, len);   //45 bytes
        break;
    default:
        LOG(WARN,"clock(%d) gps_service_push_position_request assert(app protocol(%d)) failed.", util_clock(), config_service_get_app_protocol());
        return GM_SYSTEM_ERROR;
    }
    return GM_SUCCESS;
}




GM_ERRCODE gps_service_push_position_request(u8 *mobile_num, u8 num_len, u8 *command, u8 cmd_len)
{
    u8 buff[HIS_FILE_FRAME_SIZE];
    u16 len = sizeof(buff);
    u16 idx = 0;
    GM_ERRCODE ret;

    if(GM_SUCCESS != protocol_pack_position_request_msg(mobile_num,num_len,command,cmd_len, buff, &idx, len))
    {
        LOG(ERROR,"Failed to gps_service_push_position_request!");
        return GM_SYSTEM_ERROR;
    }

    len=idx;

    if(SOCKET_STATUS_WORK == s_gps_socket.status)
    {
        LOG(DEBUG,"clock(%d) gps_service_push_position_request msglen(%d) mobile_num(%s)", util_clock(), len,mobile_num);
        ret = gm_socket_send(&s_gps_socket, buff, len);
	
        if(GM_SUCCESS == ret)
        {
            s_gps_socket.send_time = util_clock();
            return GM_SUCCESS;
        }
        else if (GM_MEM_NOT_ENOUGH != ret)
        {
            LOG(WARN,"Failed to send data by socket!");
            system_state_set_gpss_reboot_reason("gm_socket_send one_locate");
            gps_service_destroy_gprs();
        }
    }

    return gps_service_push_to_stack(buff, idx);
}


void gps_service_after_register_response(void)
{
    if(SOCKET_STATUS_LOGIN != s_gps_socket.status)
    {
        LOG(WARN,"clock(%d) gps_service_after_register_response assert(socket->status(%d) == SOCKET_STATUS_LOGIN) failed.", util_clock(), s_gps_socket.status);
        return;
    }
    protocol_send_login_msg(&s_gps_socket);
}


void gps_service_after_login_response(void)
{	
	JsonObject* p_json_log = json_create();
	u8 work_mode;
	
    char ip_str[16] = {0};
	U8* ip = gps_service_get_current_ip();
	GM_snprintf(ip_str, 16, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	
    if(SOCKET_STATUS_LOGIN != s_gps_socket.status)
    {
        LOG(WARN,"clock(%d) gps_service_after_login_response assert(socket->status(%d) == SOCKET_STATUS_LOGIN) failed.", util_clock(), s_gps_socket.status);
        return;
    }
    
    if(GM_SUCCESS != protocol_send_device_msg(&s_gps_socket))
    {
        return;
    }
    
    protocol_send_heartbeat_msg(&s_gps_socket);
    s_gps_socket_extend.heart_locate_send_time = s_gps_socket_extend.heart_send_time = util_clock();
	config_service_get(CFG_WORKMODE, TYPE_BYTE, &work_mode, sizeof(work_mode));
	if (work_mode != 0 && hard_ware_is_device_w())
	{
	    s_gps_socket_extend.wifi_send_time = util_clock();
	    if (false == system_state_has_reported_lbs_since_boot() && false == system_state_has_reported_gps_since_boot())
	    {
	    	gps_service_wifi_atonce();
	    	system_state_set_has_reported_lbs_since_boot(true);
	    }
    }
    
    if(PROTOCOL_JT808 == config_service_get_app_protocol())
    {
        gps_service_send_one_locate(GPS_MODE_FIX_TIME, false);
    }
    
    gps_service_transfer_status(SOCKET_STATUS_WORK);
	
	json_add_string(p_json_log, "event", "login");
	json_add_string(p_json_log, "state", "ok");
	json_add_int(p_json_log, "protocol", config_service_get_app_protocol());
	json_add_string(p_json_log, "ip", ip_str);
	log_service_upload(INFO, p_json_log);
}

void gps_service_after_receive_heartbeat(void)
{
    if(SOCKET_STATUS_LOGIN != s_gps_socket.status && SOCKET_STATUS_WORK != s_gps_socket.status)
    {
        LOG(WARN,"clock(%d) gps_service_after_receive_heartbeat assert(socket->status(%d)) failed.", util_clock(), s_gps_socket.status);
        return;
    }
    
    LOG(INFO,"clock(%d) gps_service_after_receive_heartbeat.", util_clock());
    s_gps_socket_extend.heart_receive_time = util_clock();
	s_gps_socket_extend.receive_heart_counts++;
}

U32 gps_service_get_heart_counts(void)
{
	return s_gps_socket_extend.receive_heart_counts;
}

U32 gps_service_get_location_counts(void)
{
	return s_gps_socket_extend.send_location_counts;
}

void gps_service_after_server_req(void)
{
    if(SOCKET_STATUS_WORK != s_gps_socket.status)
    {
        LOG(WARN,"clock(%d) gps_service_after_server_req assert(socket->status(%d)) failed.", util_clock(), s_gps_socket.status);
        return;
    }
    protocol_send_general_ack(&s_gps_socket);
}


void gps_service_after_server_locate_req(void)
{
    if(SOCKET_STATUS_WORK != s_gps_socket.status)
    {
        LOG(WARN,"clock(%d) gps_service_after_server_locate_req assert(socket->status(%d)) failed.", util_clock(), s_gps_socket.status);
        return;
    }

    //发送一条gps数据
    gps_service_send_one_locate(GPS_MODE_FIX_TIME, true);
}


void gps_service_after_receive_remote_msg(u8 *pRet, u16 len)
{
    protocol_send_remote_ack(&s_gps_socket, pRet, len);
}

void gps_service_after_param_get(void)
{
    protocol_send_param_get_ack(&s_gps_socket);
}

void gps_service_destroy_gprs(void)
{
    if(SOCKET_STATUS_DATA_FINISH == update_service_get_status() || 
        SOCKET_STATUS_ERROR == update_service_get_status())
    {
        LOG(DEBUG,"clock(%d) gps_service_destroy_gprs update_service_status(%d)",util_clock(),update_service_get_status());
        gprs_destroy();
    }
    else
    {
        //wait gprs reinit
        gps_service_destroy();
    }
}

bool gps_service_is_login_ok(void)
{
	return (SOCKET_STATUS_WORK == s_gps_socket.status);
}

U8* gps_service_get_current_ip(void)
{
	return s_gps_socket.ip;
}


s8 gps_service_get_last_temperature(void)
{
	return s_gps_socket_extend.last_temperature;
}

/*
防止从心跳间隔长设置到短时，会导致长时间未收到心跳而重启
180s-->10s，在心跳发送100s后设置，会触发gprs_check_need_reboot工作而重启
在设置心跳时，将心跳时间复位
*/
GM_ERRCODE gps_service_set_heart_receive_time(u32 time)
{
	if(SOCKET_STATUS_LOGIN != s_gps_socket.status && SOCKET_STATUS_WORK != s_gps_socket.status)
    {
        return GM_ERROR_STATUS;
    }
	s_gps_socket_extend.heart_receive_time = time;
	return GM_SUCCESS;
}



