#ifndef __DISTSYS_LOG_
#define __DISTSYS_LOG_
#include <string>
#include <stdint.h>
#include "singleton.h"
#include <vector>
#include <memory>
#include <map>
#include <list>
#include <sstream>
#include <fstream>
#include <stdarg.h>
#define SYLAR_LOG_LEVEL(logger, level) \
	if(logger->getLevel() <= level) \
	      sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger, level, __FILE__,  __LINE__, 0, sylar::GetThreadId(),\
			    sylar::GetFiberId(), time(0)))).getSS()

#define SYLAR_LOG_DEBUG(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::DEBUG)
#define SYLAR_LOG_INFO(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::INFO)
#define SYLAR_LOG_WARN(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::WARN)
#define SYLAR_LOG_ERROR(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::ERROR)
#define SYLAR_LOG_FATAL(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::FATAL)


#define SYLAR_LOG_FMT_LEVEL(logger, level, fmt, ...) \
    if(logger->getLevel() <= level) \
        sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger, level,__FILE__, __LINE__, 0, sylar::GetThreadId(),\
                sylar::GetFiberId(), time(0)))).getEvent()->format(fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别debug的日志写入到logger
 */
#define SYLAR_LOG_FMT_DEBUG(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::DEBUG, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别info的日志写入到logger
 */
#define SYLAR_LOG_FMT_INFO(logger, fmt, ...)  SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::INFO, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别warn的日志写入到logger
 */
#define SYLAR_LOG_FMT_WARN(logger, fmt, ...)  SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::WARN, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别error的日志写入到logger
 */
#define SYLAR_LOG_FMT_ERROR(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::ERROR, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别fatal的日志写入到logger
 */
#define SYLAR_LOG_FMT_FATAL(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::FATAL, fmt, __VA_ARGS__)
namespace sylar{
class Logger;
    //日志级别
class LogLevel{
    public:
    enum Level{
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL =5
    };
    static const char* ToString(LogLevel::Level level);
};
//日志事件
class LogEvent{
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file, int32_t m_line, uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time);
        const char* getFile() const {return m_file;}
        int32_t getLine() const {return m_line;}
        uint32_t getElapse() const {return m_elapse;}
        uint32_t getThreadId()const {return m_threadId;}
        uint32_t getFiberId()const {return m_fiberId;}
        uint64_t getTime() const {return m_time;}
        std::string getContent()const {return m_ss.str();}
        std::stringstream& getSS(){return m_ss;}
	std::shared_ptr<Logger> getLogger() const {return m_logger;}
	LogLevel::Level getLevel() const{ return m_level;}
	void format(const char* fmt, ...);
	void format(const char* fmt, va_list al);
    private:
        const char* m_file = nullptr;//文件名
        int32_t m_line = 0;//行号
        uint32_t m_elapse=0;//程序启动总时间
        int32_t m_threadId = 0;//线程号
        uint32_t m_fiberId = 0;//携程id
        uint64_t m_time=0;//时间辍
        std::stringstream m_ss;
	std::shared_ptr<Logger> m_logger;
	LogLevel::Level m_level;
};

class LogEventWrap{
public:
	LogEventWrap(LogEvent::ptr e);
	~LogEventWrap();
	std::stringstream& getSS();
	LogEvent::ptr getEvent() const { return m_event;}
private:
	LogEvent::ptr m_event;	
};
//日志格式器
class LogFormatter{
    public:
        typedef std::shared_ptr<LogFormatter> ptr;
        LogFormatter(const std::string& pattern);
        //%t   %thread_id %m%n
        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
    public:
        class FormatItem {
            public:
                typedef std::shared_ptr<FormatItem> ptr;
                //FormatItem(const std::string& fmt = "");
                virtual ~FormatItem(){}
                virtual void format(std::ostream& os,std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        };
        void init();
    private:
        std::string m_pattern;
        std::vector<FormatItem::ptr> m_items;
};
    //日志输出器
class LogAppender{
    public:
        typedef std::shared_ptr<LogAppender>ptr;
        virtual ~LogAppender(){};
       virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
       void setFormatter(LogFormatter::ptr val){ m_formatter = val;}
       LogFormatter::ptr getFormatter()const { return m_formatter;}
       void setLevel(LogLevel::Level val) { m_level = val;}
       LogLevel::Level getLevel() const {return m_level;}
    protected:
        LogLevel::Level m_level = LogLevel::DEBUG;
        LogFormatter::ptr m_formatter;
};
//日志器
class Logger : public std::enable_shared_from_this<Logger>{
    public:
        typedef std::shared_ptr<Logger> ptr;
        Logger(const std::string& name = "root");
        void log(LogLevel::Level level,LogEvent::ptr event);
        void debug(LogEvent::ptr event);
        void info(LogEvent::ptr event);
        void warn(LogEvent::ptr event);
        void error(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);
        void addAppender(LogAppender::ptr appender);
        void delAppender(LogAppender::ptr appender);
        LogLevel::Level getLevel() const {return m_level;}
        void setLevel(LogLevel::Level val){m_level = val;}
        const std::string& getName() const{return m_name;}
    private:
        std::string m_name;//日志名称
        LogLevel::Level m_level;//日志级别
        std::list<LogAppender::ptr> m_appenders;//Appender集合
	LogFormatter::ptr m_formatter;
};
//输出到控制台
class StdoutLogAppender : public LogAppender{
    public:
	StdoutLogAppender(){}
        StdoutLogAppender(const std::string& filename);
        typedef std::shared_ptr<StdoutLogAppender> ptr;
        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event)override;
    private:
	std::string m_filename;
};
//输入到文件
class FileLogAppender: public LogAppender{
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
	FileLogAppender(){};
        FileLogAppender(const std::string& filename);
        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event)override;

        bool reopen();
    private:
        std::string m_filename;
        std::ofstream m_filestream;
};

class LoggerManager {
public:
	LoggerManager();
	Logger::ptr getLogger(const std::string& name);

	void init();
private:
	std::map<std::string, Logger::ptr> m_loggers;
	Logger::ptr m_root;

};

typedef sylar::singleton<LoggerManager> LoggerMgr;
}
#endif









