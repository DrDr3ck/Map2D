#include "translator.h"
#include "logger.h"

#include <iostream>
#include <fstream>

// Initialize singleton_ to nullptr
Translator* Translator::singleton_ = nullptr;

Translator::Translator() {
    clear();
}

Translator* Translator::instance() {
    if( singleton_ == nullptr ) {
        singleton_ = new Translator();
        Logger::debug() << "creating Translator singleton" << Logger::endl;
    }
    return singleton_;
}

void Translator::kill() {
    if( singleton_ != nullptr ) {
        Logger::debug() << "destroying Translator singleton" << Logger::endl;
        delete singleton_;
        singleton_ = nullptr;
    }
}

void Translator::clear() {
    empty_ = true;
    dictionary_.clear();
}

void Translator::readDictionary(const std::string& filename) {
    // clear map
    clear();
    // open file
    std::ifstream file(filename);
    if (!file) {
        Logger::error() << tr("unable to read translator file: ") << filename << Logger::endl;
        return;
    }
    // fill map with entries coming from file
    std::string str;
    while (std::getline(file, str)) {
        str = str.substr(2);
        int index = str.find_first_of("','");
        if( index > 0 ) {
            std::string left = str.substr(0,index);
            std::string right = str.substr(index+3);
            right = right.substr(0, right.length()-2);
            dictionary_.insert(std::pair<std::string, std::string>(left,right) );
        }
    }
    empty_ = (dictionary_.size() == 0);
}

const std::string& tr(const std::string& str) {
    return Translator::instance()->translate(str);
}

const std::string& Translator::translate(const std::string& str) {
    if( empty_ ) return str;
    std::map<std::string, std::string>::iterator it = dictionary_.find(str);
    if( it == dictionary_.end() ) {
        return str;
    }
    return it->second;
}
