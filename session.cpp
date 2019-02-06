#include "session.h"
#include "logger.h"
#include "translator.h"
#include "utility.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>

using namespace std;

// Initialize singleton_ to nullptr
Session* Session::singleton_ = nullptr;

Session* Session::instance() {
    if( singleton_ == nullptr ) {
        singleton_ = new Session();
        Logger::debug() << "creating Session singleton" << Logger::endl;
        singleton_->loadSession();
    }
    return singleton_;
}

void Session::kill() {
    if( singleton_ != nullptr ) {
        Logger::debug() << "destroying Session singleton" << Logger::endl;
        singleton_->saveSession();
        delete singleton_;
        singleton_ = nullptr;
    }
}

static string option_filename = "bakhar.opt";

void Session::loadSession() {
    ifstream file(option_filename);
    if (!file) {
        Logger::error() << tr("Unable to read resource file: ") << option_filename << Logger::endl;
        return;
    }

    std::string str;
    while (std::getline(file, str)) {
        if( !Utility::startsWith(str, "*") ) {
            continue;
        }
        int index = str.find(':');
        string label = str.substr(0, index);
        string value = str.substr(index+1);
        index = value.find('#');
        string desc = "# no description";
        if( index >= 0 ) {
            desc = value.substr(index);
            value = value.substr(0,index);
        }
        value = Utility::trim(value);
        ValueDesc vd = std::pair<string, string>(value, desc);
        dictionary_.insert(std::pair<string,ValueDesc>(label, vd));
    }
}

void Session::saveSession() {
    std::ofstream file(option_filename);
    if (!file) {
        Logger::error() << tr("unable to open file for save: ") << option_filename << Logger::endl;
        return;
    }

    Logger::info() << tr("Saving ") << option_filename << "..." << Logger::endl;

    time_t now = time(0);
    char* date = ctime(&now);

    file << "# edit at your own risk !!" << std::endl;
    file << "# date " << date << std::endl;

    for( auto attr : dictionary_ ) {
        file << attr.first << ": " << attr.second.first << " " << attr.second.second << std::endl;
    }

    file.close();
}

bool Session::getBoolean(const string& label, bool default_value) {
    map<string, ValueDesc>::iterator it = dictionary_.find(label);
    if( it != dictionary_.end() ) {
        const string& value = it->second.first;
        if( value == "true" ) {
            return true;
        }
    }
    ValueDesc vd = std::pair<string, string>(default_value?"true":"false", "");
    dictionary_.insert(std::pair<string,ValueDesc>(label, vd));
    return default_value;
}

void Session::setBoolean(const string& label, bool value) {
    map<string, ValueDesc>::iterator it = dictionary_.find(label);
    if( it != dictionary_.end() ) {
        dictionary_.erase(it);
    }
    ValueDesc vd = std::pair<string, string>(value?"true":"false", "");
    dictionary_.insert(std::pair<string,ValueDesc>(label, vd));
}

int Session::getInteger(const string& label, int default_value) {
    map<string, ValueDesc>::iterator it = dictionary_.find(label);
    int value = default_value;
    if( it != dictionary_.end() ) {
        const string& str_value = it->second.first;
        value = ::atoi(str_value.c_str());
    }
    ValueDesc vd = std::pair<string, string>(Utility::itos(default_value), "");
    dictionary_.insert(std::pair<string,ValueDesc>(label, vd));
    return value;
}

void Session::setInteger(const string& label, int value) {
    map<string, ValueDesc>::iterator it = dictionary_.find(label);
    if( it != dictionary_.end() ) {
        dictionary_.erase(it);
    }
    ValueDesc vd = std::pair<string, string>(Utility::itos(value), "");
    dictionary_.insert(std::pair<string,ValueDesc>(label, vd));
}

float Session::getFloat(const string& label, float default_value) {
    map<string, ValueDesc>::iterator it = dictionary_.find(label);
    float value = default_value;
    if( it != dictionary_.end() ) {
        const string& str_value = it->second.first;
        value = ::atof(str_value.c_str());
    }
    ValueDesc vd = std::pair<string, string>(Utility::ftos(default_value), "");
    dictionary_.insert(std::pair<string,ValueDesc>(label, vd));
    return value;
}

string Session::getString(const string& label, string default_value) {
    map<string, ValueDesc>::iterator it = dictionary_.find(label);
    string value = default_value;
    if( it != dictionary_.end() ) {
        value = it->second.first;
    }
    ValueDesc vd = std::pair<string, string>(default_value, "");
    dictionary_.insert(std::pair<string,ValueDesc>(label, vd));
    return value;
}


