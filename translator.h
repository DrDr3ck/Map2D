#ifndef translator_h
#define translator_h

#include <string>
#include <map>

/********************************************************************/

std::string tr(const std::string& str);

class Translator {
private:
  Translator();
  ~Translator() {}

public:
    static Translator* instance();
    static void kill();

    void readDictionary(const std::string& filename = "none");
    const std::string& translate(const std::string& str);

protected:
    void clear();

private:
    bool empty_ = true;
    std::map<std::string, std::string> dictionary_;
    static Translator* singleton_;
};

/********************************************************************/

#endif // translator_h
