/**
 * @file    Logging module
 * @brief   initialize_log() 함수를 명시적으로 호출하면, log level, log target 
 *			(file, debugger, console, etc) 지정/변경 가능
 *
 *			log format 지정/변경 가능
 *
 *			multi thread 환경에서 serialization 이 됨
 *
 *			log_err, con_err 같은 매크로만 사용하면 debugger, console 로 메세지 출력 가능
 * @ref     
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2015/01/12 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/

#ifndef _log_h_
#define _log_h_

#pragma warning(disable:4005)
/*
1>C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\include\stdint.h(78): warning C4005: 'INT32_MAX' : macro redefinition
1>          C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\include\intsafe.h(176) : see previous definition of 'INT32_MAX'
*/
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#pragma warning(default:4005)
#include <boost/format.hpp>

#include "Win32Utils.h"
#include "Queue.h"

/// @brief log level
#define log_level_debug         0
#define log_level_info          1
#define log_level_warn          2
#define log_level_critical      3
#define log_level_error         log_level_critical

/// @brief log to 
#define log_to_none		0
#define log_to_file		1
#define log_to_ods		2
#define log_to_con		4

/// @brief  log_mask
#define log_mask_all    0xffffffff
#define log_mask_sys    0x00000001      // for log_info, log_err, xxx

//
// C like APIs
//

bool 
initialize_log(
	_In_ uint32_t log_level,
	_In_opt_z_ const wchar_t* log_file_path
	);

void 
finalize_log(
	);

void
set_log_format(
	_In_ bool show_process_name, 
	_In_ bool show_pid_tid,
	_In_ bool show_function_name
	);

void 
set_log_mask(
    _In_ uint32_t mask
    );        

void
log_write_fmt(
    _In_ uint32_t log_mask,
    _In_ uint32_t log_level,
	_In_ uint32_t log_to,
	_In_z_ const char* function,
    _In_z_ const char* fmt, 
    _In_ ...
    );

void
log_write_fmt_without_deco(
    _In_ uint32_t log_mask, 
    _In_ uint32_t log_level,
    _In_ uint32_t log_to,
    _In_z_ const char* fmt,
    _In_ ...
    );



//
// define macro for convenience
//
#define log_err		log_write_fmt( log_mask_sys, log_level_error, log_to_con | log_to_ods | log_to_file, __FUNCTION__, 
#define log_warn	log_write_fmt( log_mask_sys, log_level_warn, log_to_con | log_to_ods | log_to_file, __FUNCTION__,  
#define log_info	log_write_fmt( log_mask_sys, log_level_info, log_to_con | log_to_ods | log_to_file, __FUNCTION__, 
#define log_dbg		log_write_fmt( log_mask_sys, log_level_debug, log_to_con | log_to_ods | log_to_file, __FUNCTION__, 
#define log_msg     log_write_fmt_without_deco( log_mask_sys, log_level_debug, log_to_ods | log_to_file, 


#define con_err		log_write_fmt( log_mask_sys, log_level_error, log_to_con, __FUNCTION__, 
#define con_warn	log_write_fmt( log_mask_sys, log_level_warn, log_to_con, __FUNCTION__,  
#define con_info	log_write_fmt( log_mask_sys, log_level_info, log_to_con, __FUNCTION__, 
#define con_dbg		log_write_fmt( log_mask_sys, log_level_debug, log_to_con, __FUNCTION__, 
#define con_msg     log_write_fmt_without_deco( log_mask_sys, log_level_debug, log_to_con, 


#define log_end		);
#define con_end		);



/// @brief	logger_impl class
typedef class slogger: private boost::noncopyable
{
public:
    explicit slogger();
    ~slogger();

    bool slog_start(_In_ uint32_t base_log_level, _In_opt_z_ const wchar_t *log_file_path);
	void slog_stop();

	uint32_t slog_get_base_log_level() { return _base_log_level; }
    void slog_set_base_log_level(_In_ uint32_t base_log_level){ if (_base_log_level != base_log_level) _base_log_level = base_log_level; }	
	
	void slog_write(_In_ uint32_t level, _In_ uint32_t log_to, _In_z_ const char* log_message);
private:
    bool volatile		_stop_logger;
    uint32_t			_base_log_level;
	boost::mutex		_lock;


	typedef class log_entry
	{
	public:
		log_entry(_In_ uint32_t log_level, _In_ uint32_t log_to, const char* log_message)
            : _log_level(log_level), _log_to(log_to), _msg(log_message)
        {
        }

        uint32_t    _log_level;
		uint32_t	_log_to;
        std::string _msg;
	} *plog_entry;

    Queue<plog_entry>	_log_queue;
    boost::thread*		_logger_thread;
    HANDLE				_log_file_handle;

    void slog_thread();
} *pslogger;


//
// Logger helper class
//
typedef class Logger
{
public:
	Logger(
		_In_ bool show_process_name = true,
		_In_ bool show_pid_tid = true,
		_In_ bool show_function_name = true
		)
	{
		::set_log_format(show_process_name,
						 show_pid_tid,
						 show_function_name);

	}
	
	~Logger()
	{
		finalize_log(); 
	}

	bool 
	initialize_log(
		_In_ uint32_t log_level,
		_In_opt_z_ const wchar_t* log_file_path
		)
	{
		return ::initialize_log(log_level, log_file_path);
	}

	void
	set_log_format(
		_In_ bool show_process_name, 
		_In_ bool show_pid_tid,
		_In_ bool show_function_name
		)
	{
		::set_log_format(show_process_name, 
						 show_pid_tid, 
						 show_function_name);
	}

	void 
	set_log_mask(
		_In_ uint32_t mask
		)
	{
		::set_log_mask(mask);
	}
} *PLogger;


#endif//_log_h_