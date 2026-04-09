

#ifndef KIWIX_XMLRPC_H_
#define KIWIX_XMLRPC_H_

#include <stdexcept>
#include <tools/otherTools.h>
#include <pugixml.hpp>

namespace kiwix {

class InvalidRPCNode : public std::runtime_error {
  public:
    InvalidRPCNode(const std::string& msg) : std::runtime_error(msg) {};
};

class Struct;
class Array;

class Value {
    pugi::xml_node m_value;

  public:
    Value(pugi::xml_node value) : m_value(value) { }

    void set(int value) {
      if (!m_value.child("int"))
        m_value.append_child("int");
      m_value.child("int").text().set(value);
    };

    int getAsI() const {
      if (!m_value.child("int"))
        throw InvalidRPCNode("Type Error");
      return m_value.child("int").text().as_int();
    }

    void set(bool value) {
      if (!m_value.child("boolean"))
        m_value.append_child("boolean");
      m_value.child("boolean").text().set(value);
    };

    int getAsB() const {
      if (!m_value.child("boolean"))
        throw InvalidRPCNode("Type Error");
      return m_value.child("boolean").text().as_bool();
    }

    void set(const std::string& value) {
      if (!m_value.child("string"))
        m_value.append_child("string");
      m_value.child("string").text().set(value.c_str());
    };

    std::string getAsS() const {
      if (!m_value.child("string"))
        throw InvalidRPCNode("Type Error");
      return m_value.child("string").text().as_string();
    }

    void set(double value) {
      if (!m_value.child("double"))
        m_value.append_child("double");
      m_value.child("double").text().set(value);
    };

    double getAsD() const {
      if (!m_value.child("double"))
        throw InvalidRPCNode("Type Error");
      return m_value.child("double").text().as_double();
    }

    inline Struct getStruct();
    inline Array getArray();
};

class Array {
    pugi::xml_node m_array;
  public:
    Array(pugi::xml_node array) : m_array(array) {
      if (!m_array.child("data"))
        m_array.append_child("data");
    }

    Value addValue() {
      auto value = m_array.child("data").append_child("value");
      return Value(value);
    }

    Value getValue(int index) const {
      auto value = m_array.child("data").child("value");
      while(index && value) {
        value = value.next_sibling();
        index--;
      }
      if (0==index) {
        return Value(value);
      } else {
        throw InvalidRPCNode("Index error");
      }
    }

};

class Member {
    pugi::xml_node m_member;

  public:
    Member(pugi::xml_node member) : m_member(member) { }

    Value getValue() const {
      return Value(m_member.child("value"));
    };
};

class Struct {
    pugi::xml_node m_struct;

  public:
    Struct(pugi::xml_node _struct) : m_struct(_struct) { }

    Member getMember(const std::string& name) const {
      for(auto member=m_struct.first_child(); member; member=member.next_sibling()) {
        std::string member_name = member.child("name").text().get();
        if (member_name == name) {
          return Member(member);
        }
      }
      throw InvalidRPCNode("Key Error");
   }

   Member addMember(const std::string& name) {
     auto member = m_struct.append_child("member");
     member.append_child("name").text().set(name.c_str());
     member.append_child("value");
     return Member(member);
   }
};

class Fault : public Struct {
  public:
    Fault(pugi::xml_node fault) : Struct(fault) {};

    int getFaultCode() const {
      return getMember("faultCode").getValue().getAsI();
    }

    std::string getFaultString() const {
      return getMember("faultString").getValue().getAsS();
    }
};

Struct Value::getStruct() {
  if (!m_value.child("struct"))
    m_value.append_child("struct");
  return Struct(m_value.child("struct"));
}

Array Value::getArray() {
  if (!m_value.child("array"))
    m_value.append_child("array");
  return Array(m_value.child("array"));
}

class Param {
    pugi::xml_node m_param;
  public:
    Param(pugi::xml_node param) : m_param(param) {
      if (!m_param.child("value"))
        m_param.append_child("value");
    };

    Value getValue() const {
      return Value(m_param.child("value"));
    };
};

class Params {
    pugi::xml_node m_params;

  public:
    Params(pugi::xml_node params) : m_params(params) {};

    Param addParam() {
      auto param = m_params.append_child("param");
      return Param(param);
    }
    Param getParam(int index) const {
      auto param = m_params.child("param");
      while(index && param) {
        param = param.next_sibling();
        index--;
      }
      if (0==index) {
        return Param(param);
      } else {
        throw InvalidRPCNode("Index Error");
      }
    }
};

class MethodCall {
    pugi::xml_document m_doc;

  public:
    MethodCall(const std::string& methodName, const std::string& secret) {
      auto mCall = m_doc.append_child("methodCall");
      mCall.append_child("methodName").text().set(methodName.c_str());
      mCall.append_child("params");
      if (!secret.empty()) {
        getParams().addParam().getValue().set(secret);
      }
    }

    Params getParams() const {
      return Params(m_doc.child("methodCall").child("params"));
    }

    Value newParamValue() {
      return getParams().addParam().getValue();
    }

    std::string toString() const {
      return nodeToString(m_doc);
    }
};


class MethodResponse {
    pugi::xml_document m_doc;

  public:
    MethodResponse(const std::string& content) {
      m_doc.load_buffer(content.c_str(), content.size());
    }

    Params getParams() const {
      auto params = m_doc.child("methodResponse").child("params");
      if (!params)
        throw InvalidRPCNode("No params");
      return Params(params);
    }

    Value getParamValue(int index) const {
      return getParams().getParam(index).getValue();
    }

    bool isFault() const {
      return (!!m_doc.child("methodResponse").child("fault"));
    }

    Fault getFault() const {
      auto fault = m_doc.child("methodResponse").child("fault");
      if (!fault)
        throw InvalidRPCNode("No fault");
      return Fault(fault.child("value").child("struct"));
    }
};



};


#endif // KIWIX_XMLRPC_H_
