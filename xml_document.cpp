#include "xml_document.h"

#include "utility.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>

XMLNode::XMLNode(
    const std::string& name, XMLNode* parent
) : name_(name) {
    if( parent != nullptr ) {
        parent->addNode(this);
    }
}

XMLNode::XMLNode(
    const std::string& name, const std::string& value, XMLNode* parent
) : name_(name), value_(value) {
    if( parent != nullptr ) {
        parent->addNode(this);
    }
}

XMLNode::~XMLNode() {
    nodes_.erase(nodes_.begin(), nodes_.end());
    attributes_.erase(attributes_.begin(), attributes_.end());
}

void XMLNode::addNode(XMLNode* node) {
    nodes_.push_back(node);
}

int XMLNode::nodeCount() const {
    return nodes_.size();
}

const XMLNode* XMLNode::node(int index) const {
    if( index < 0 || index >= nodeCount() ) {
        return nullptr;
    }
    return nodes_.at(index);
}

XMLNode* XMLNode::node(int index) {
    if( index < 0 || index >= nodeCount() ) {
        return nullptr;
    }
    return nodes_.at(index);
}

XMLNode* XMLNode::addValuedNode(const std::string& name, const std::string& value) {
    return new XMLNode(name, value, this);
}

void XMLNode::addAttr(XMLAttr* attr) {
    attributes_.push_back(attr);
}

XMLAttr* XMLNode::addValuedAttr(const std::string& label, const std::string& value) {
    return new XMLAttr(label, value, this);
}

int XMLNode::attrCount() const {
    return attributes_.size();
}

const XMLAttr* XMLNode::attr(int index) const {
    if( index < 0 || index >= attrCount() ) {
        return nullptr;
    }
    return attributes_.at(index);
}

XMLAttr* XMLNode::attr(int index) {
    if( index < 0 || index >= attrCount() ) {
        return nullptr;
    }
    return attributes_.at(index);
}


const std::string& XMLNode::name() const {
    return name_;
}

bool XMLNode::hasValue() const {
    return !value_.empty();
}

const std::string& XMLNode::value() const {
    return value_;
}

/******************************************************************/

XMLAttr::XMLAttr(
    const std::string& label,
    const std::string& value,
    XMLNode* parent
) : label_(label), value_(value) {
    if( parent != nullptr ) {
        parent->addAttr(this);
    }
}

XMLAttr::~XMLAttr() {
}

const std::string& XMLAttr::label() const {
    return label_;
}

const std::string& XMLAttr::value() const {
    return value_;
}

/******************************************************************/

namespace {
    /*!
     * result can be:
     * <nodename>...</nodename>
     * or
     * <nodename attr="" ...>...</nodename>
     * or
     * <nodename attr="" .../>
     */
    void save_node(std::ofstream& file, const XMLNode* node, std::string offset) {
        file << offset << "<" << node->name();
        if( node->attrCount() > 0 ) {
            for( int i = 0; i < node->attrCount(); i++ ) {
                const XMLAttr* attr = node->attr(i);
                file << " " << attr->label() << "=\"" << attr->value() << "\"";
            }
        }
        if( !node->hasValue() && !node->nodeCount() > 0 ) {
            file << "/>" << std::endl;
            return;
        }
        if( node->hasValue() ) {
            file << ">" << node->value() << "</" << node->name() << ">" << std::endl;
        } else {
            file << ">" << std::endl;
            for( int i = 0; i < node->nodeCount(); i++ ) {
                const XMLNode* child = node->node(i);
                save_node(file, child, offset.append("  "));
                offset = offset.substr(2);
            }
            file << offset << "</" << node->name() << ">" << std::endl;
        }
    }

    XMLNode* read_node(std::string& str, XMLNode* parent) {
        // starting with a node
        // 1. get name
        int endtag = str.find_first_of(" />");
        std::string name = str.substr(0, endtag);
        XMLNode* node = new XMLNode(name, parent);
        // read potential attributes
        str = str.substr(name.length(), str.length()-name.length()+1);
        str = Utility::trim(str);
        // 2. get attributes if any
        int attrtag = str.find_first_of("=");
        while( attrtag >= 0 ) {
            endtag = str.find_first_of(" />");
            std::string label = str.substr(0, attrtag);
            std::string value = str.substr(attrtag+1, endtag-attrtag-1);
            value = Utility::trim(value, '"');
            node->addValuedAttr(label, value);
            str = str.substr(endtag, str.length()-endtag+1);
            str = Utility::trim(str);
            attrtag = str.find_first_of("=");
        }
        // read potential value or nodes
        return node;
    }
}

XMLNode* XMLDocument::read_doc(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        Logger::error() << "unable to open file for load: " << filename << Logger::endl;
        return nullptr;
    }

    XMLNode* parent = nullptr;
    XMLNode* cur_node = nullptr;
    std::string str;
    while (std::getline(file, str)) {
        if( Utility::startsWith(str, "</") ) {
            // end tag
        } else {
            int begintag = str.find_first_of("<");
            if( begintag >= 0 ) {
                str = str.substr(begintag+1,str.length() - begintag + 1);
                bool is_complete = (str.find_first_of("/") != std::string::npos);
                XMLNode* node = read_node(str, cur_node);
                if( parent == nullptr ) {
                    parent = node;
                    cur_node = parent;
                }
                if( !is_complete ) {
                    cur_node = node;
                }
            }
        }
    }
    return parent;
}

bool XMLDocument::write_doc(const XMLNode* node, const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        Logger::error() << "unable to open file for save: " << filename << Logger::endl;
        return false;
    }

    save_node(file, node, "");
    return true;
}
