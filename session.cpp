#include "session.h"
#include "logger.h"
#include "translator.h"
#include "utility.h"
#include <iostream>
#include <sstream>
#include <fstream>

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
    // TODO save session
}

bool Session::getBoolean(const string& label) {
    map<string, ValueDesc>::iterator it = dictionary_.find(label);
    if( it != dictionary_.end() ) {
        const string& value = it->second.first;
        if( value == "true" ) {
            return true;
        }
    }
    return false;
}

int Session::getInteger(const string& label) {
    map<string, ValueDesc>::iterator it = dictionary_.find(label);
    int value = 0;
    if( it != dictionary_.end() ) {
        const string& str_value = it->second.first;
        value = ::atoi(str_value.c_str());
    }
    return value;
}

float Session::getFloat(const string& label) {
    map<string, ValueDesc>::iterator it = dictionary_.find(label);
    float value = 0.f;
    if( it != dictionary_.end() ) {
        const string& str_value = it->second.first;
        value = ::atof(str_value.c_str());
    }
    return value;
}

string Session::getString(const string& label) {
    map<string, ValueDesc>::iterator it = dictionary_.find(label);
    string value = "";
    if( it != dictionary_.end() ) {
        value = it->second.first;
    }
    return value;
}


