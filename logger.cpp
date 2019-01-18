#include "logger.h"

#include "utility.h"

/********************************************************************/

LoggerString::LoggerString(const std::string& type, const std::string& string) : type_(type), string_(string) {
    time_left_ = LoggerMgr::timeLeft();
}

LoggerString::LoggerString(const LoggerString& logger_string) {
    if( &logger_string != this ) {
        type_ = logger_string.type_;
        string_ = logger_string.string_;
        time_left_ = logger_string.time_left_;
    }
}

void LoggerString::animate(float time_passed) {
    if( time_left_ > 0.f ) {
        time_left_ -= time_passed;
    }
}

std::string LoggerString::full_string() const {
    std::string full;
    full.append(type_);
    full.append(": ");
    full.append(string_);
    return full;
}

/********************************************************************/

Logger::Logger(LoggerType type) : type_(type) {
}

Logger::Logger(const Logger& logger) {
    if( &logger != this ) {
        type_ = logger.type_;
        string_ = logger.string_;
    }
}

Logger::~Logger() {
}

std::string Logger::endl("endl");

Logger& Logger::operator<<(const std::string& str) {
    if( str != Logger::endl ) {
        this->appendString(str);
    } else {
        // display string in terminal
        std::cout << getType() << ": " << string_ << std::endl;
        if( type_ != LoggerType::DEBUG ) {
            LoggerMgr::instance()->addString( LoggerString(getType(), string_) );
        }
    }
    return *this;
}

Logger& Logger::operator<<(const int& value) {
    this->appendString(Utility::itos(value));
    return *this;
}

void Logger::appendString(const std::string& str) {
    string_.append(str);
}

Logger Logger::info() {
    return Logger(LoggerType::INFO);
}

Logger Logger::warning() {
    return Logger(LoggerType::WARNING);
}

Logger Logger::error() {
    return Logger(LoggerType::ERROR);
}

Logger Logger::debug() {
    return Logger(LoggerType::DEBUG);
}

const std::string& Logger::getType() const {
    if( type_ == LoggerType::INFO ) {
        static std::string info_str("Info");
        return info_str;
    }
    if( type_ == LoggerType::WARNING ) {
        static std::string warning_str("Warning");
        return warning_str;
    }
    if( type_ == LoggerType::ERROR ) {
        static std::string error_str("Error");
        return error_str;
    }
    static std::string debug_str("Debug");
    return debug_str;
}

/********************************************************************/

// Initialize singleton_ to nullptr
LoggerMgr* LoggerMgr::singleton_ = nullptr;

LoggerMgr* LoggerMgr::instance() {
    if( singleton_ == nullptr ) {
        singleton_ = new LoggerMgr();
        Logger::debug() << "creating LoggerMgr singleton" << Logger::endl;
    }
    return singleton_;
}

void LoggerMgr::kill() {
    if( singleton_ != nullptr ) {
        Logger::debug() << "destroying LoggerMgr singleton" << Logger::endl;
        delete singleton_;
        singleton_ = nullptr;
    }
}

void LoggerMgr::addString(const LoggerString& str) {
    terminal_.push_back( str );
}

void LoggerMgr::animate(float time_passed) {
    for( auto& log_str : terminal_ ) {
        log_str.animate(time_passed);
    }
}

void LoggerMgr::moveInJournal(int index) {
    LoggerString str = terminal_[index];
    journal_.push_back(str);
    terminal_.erase( terminal_.begin() + index);
}

/********************************************************************/
