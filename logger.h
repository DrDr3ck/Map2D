#ifndef logger_h
#define logger_h

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>

/********************************************************************/

class LoggerString {
public:
    LoggerString(const std::string& type, const std::string& string);
    LoggerString(const LoggerString& logger_string);
    ~LoggerString() {}

    const std::string& type() const { return type_; }
    const std::string& string() const { return string_; }
    std::string full_string() const;
    float timeLeft() const { return time_left_; }
    void animate(float time_passed);

    bool expired() const { return time_left_ <= 0.f; }
private:
    std::string type_;
    std::string string_;
    float time_left_; // en milliseconds
};

/********************************************************************/

class LoggerMgr {
private:
  LoggerMgr() {}
  ~LoggerMgr() {}

public:
    static LoggerMgr* instance();
    static void kill();

    static float timeLeft() {
        return 5000.; // 5000 milliseconds
    }

    void addString(const LoggerString& string);

    const std::vector<LoggerString>& terminal() const { return terminal_; }
    const std::vector<LoggerString>& journal() const { return journal_; }

    void moveInJournal(int index);

    void animate(float time_passed);

    void clear() { terminal_.clear(); journal_.clear(); }

private:
    static LoggerMgr* singleton_;
    std::vector<LoggerString> terminal_;
    std::vector<LoggerString> journal_;
};

class Logger {
public:
    enum class LoggerType { INFO, WARNING, ERROR, DEBUG };
public:
    Logger(LoggerType type);
    Logger(const Logger& logger);
    ~Logger();

    static std::string endl;

    Logger& operator<<(const std::string& str);
    Logger& operator<<(const int& value);

    const std::string& getString() const { return string_; }
    const LoggerType& type() const { return type_; }
    const std::string& getType() const;

    static Logger info();
    static Logger warning();
    static Logger error();
    static Logger debug();

protected:
    void appendString(const std::string& str);

private:
    LoggerType type_;
    std::string string_;
};

/********************************************************************/

#endif // logger_h
