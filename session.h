#ifndef session_h
#define session_h

#include <string>
#include <map>

/********************************************************************/

#define ValueDesc std::pair<std::string,std::string>

class Session {
private:
  Session() {}
  ~Session() {}

public:
    static Session* instance();
    static void kill();

    void loadSession();
    void saveSession();

    bool getBoolean(const std::string& label);
    int getInteger(const std::string& label);
    float getFloat(const std::string& label);
    std::string getString(const std::string& label);

private:
    static Session* singleton_;
    std::map< std::string, ValueDesc > dictionary_;
};

/********************************************************************/

#endif // session_h
