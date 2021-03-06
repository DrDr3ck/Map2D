#ifndef xml_document_h
#define xml_document_h

#include <string>
#include <vector>

/********************************************************************/

class XMLNode;

class XMLAttr {
public:
    XMLAttr(const std::string& label, const std::string& value, XMLNode* parent=nullptr);
    ~XMLAttr();

    const std::string& label() const;
    const std::string& value() const;

private:
    std::string label_;
    std::string value_;
};

class XMLNode {
public:
    XMLNode(const std::string& name, XMLNode* parent = nullptr);
    XMLNode(const std::string& name, const std::string& value, XMLNode* parent);
    ~XMLNode();

    void addNode(XMLNode* node);
    int nodeCount() const;
    const XMLNode* node(int index) const;
    XMLNode* node(int index);
    const std::vector<XMLNode*>& nodes() const { return nodes_; }

    XMLNode* getNodeFromTag(const std::string& tag_name);
    XMLNode* getNodeFromName(const std::string& node_name);

    XMLNode* addValuedNode(const std::string& name, const std::string& value);

    void addAttr(XMLAttr* attr);
    XMLAttr* addValuedAttr(const std::string& label, const std::string& value);
    int attrCount() const;
    const XMLAttr* attr(int index) const;
    XMLAttr* attr(int index);
    const std::vector<XMLAttr*>& attributes() const { return attributes_; }

    XMLAttr* getAttrFromName(const std::string& attr_name);

    const std::string& name() const;
    bool hasValue() const;
    const std::string& value() const;
    XMLNode* parent() const { return parent_; }
private:
    std::string name_;
    std::vector<XMLNode*> nodes_;
    std::vector<XMLAttr*> attributes_;
    std::string value_;
    XMLNode* parent_;
};

class XMLDocument {
public:
    static XMLNode* read_doc(const std::string& filename);
    static bool write_doc(const XMLNode* node, const std::string& filename);
};

/********************************************************************/

#endif // xml_document_h
