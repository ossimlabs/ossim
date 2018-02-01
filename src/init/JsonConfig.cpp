//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/init/JsonConfig.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimPreferences.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimDirectory.h>
#include <opencv2/features2d/features2d.hpp>
#include <memory>

using namespace std;

namespace ossim
{
JsonParam JsonConfig::s_nullParam;

JsonParam::JsonParam(const ossimString& argname,
                   const ossimString& arglabel,
                   const ossimString& argdescr,
                   ParamType   argparamType,
                   void* value)
:  _name (argname),
   _label (arglabel),
   _descr (argdescr),
   _type (argparamType),
   _value(0)
{
   setValue(value);
}

JsonParam::JsonParam(const JsonParam& copy)
:  _label (copy._label),
   _name (copy._name),
   _descr (copy._descr),
   _type (copy._type),
   _value (0)
{
   setValue(copy._value);
}

void JsonParam::setValue(void* value)
{
   if (!value)
      return;

   switch (_type)
   {
   case JsonParam::BOOL:
      _value = new bool;
      memcpy(_value, value, sizeof(bool));
      break;
   case JsonParam::INT:
      _value = new int;
      memcpy(_value, value, sizeof(int));
      break;
   case JsonParam::UINT:
      _value = new unsigned int;
      memcpy(_value, value, sizeof(unsigned int));
      break;
   case JsonParam::FLOAT:
      _value = new double;
      memcpy(_value, value, sizeof(double));
      break;
   case JsonParam::STRING:
      _value = new string(*(string*)(value));
      break;
   case JsonParam::VECTOR:
      _value = new vector<double>(*(vector<double>*)(value));
      break;
   default:
      _value = 0;
   }
}

void JsonParam::resetValue()
{
   if (!_value)
      return;

   switch (_type)
   {
   case JsonParam::BOOL:
      delete (bool*)_value;
      break;
   case JsonParam::INT:
   case JsonParam::UINT:
      delete (int*)_value;
      break;
   case JsonParam::FLOAT:
      delete (double*)_value;
      break;
   case JsonParam::STRING:
      delete (string*)_value;
      break;
   case JsonParam::VECTOR:
      ((vector<double>*)_value)->clear();
      delete (vector<double>*)_value;
      break;
   default:
      break;
   }
   _value = 0;
}

bool JsonParam::loadJSON(const Json::Value& paramNode)
{
   try
   {
      _name = paramNode["name"].asString();
      _label = paramNode["label"].asString();
      _descr = paramNode["descr"].asString();
      Json::Value value = paramNode["value"];

      ossimString ptype = paramNode["type"].asString();
      if (ptype.empty() || _name.empty())
         return false;

      ptype.upcase();

      if (ptype == "VECTOR")
      {
         _type = JsonParam::VECTOR;
         vector<double> v;
         if (value.isArray())
         {
            int n = value.size();
            for (unsigned int j=0; j<n; ++j)
               v.push_back(value[j].asDouble());
         }
         setValue(&v);
      }
      else
      {
         // Screen for param value list as found in the default config JSONs. Pick the first element
         // as the default:
         if (value.isArray())
            value = value[0];

         if (ptype == "BOOL")
         {
            _type = JsonParam::BOOL;
            bool v = value.asBool();
            setValue(&v);
         }
         else if (ptype == "UINT")
         {
            _type = JsonParam::UINT;
            unsigned int v = value.asUInt();
            setValue(&v);
         }
         else if (ptype == "INT")
         {
            _type = JsonParam::INT;
            int v = value.asInt();
            setValue(&v);
         }
         else if (ptype == "FLOAT")
         {
            _type = JsonParam::FLOAT;
            double v = value.asDouble();
            setValue(&v);
         }
         else if (ptype == "STRING")
         {
            _type = JsonParam::STRING;
            string v = value.asString();
            setValue(&v);
         }
      }
   }
   catch (exception& e)
   {
      ossimNotify(ossimNotifyLevel_WARN)<<"JsonParam::loadJSON() parse error encountered. Ignoring "
            "but should check the JsonConfig JSON for parameter <"<<_name<<">."<<endl;
      return false;
   }
   return true;
}

void JsonParam::saveJSON(Json::Value& paramNode) const
{
   vector<double>& v = *(vector<double>*)_value; // maybe not used since maybe not valid cast
   unsigned int n = 0;
   int i;
   double f;
   string s;
   bool b;

   paramNode["name"] = _name.string();
   paramNode["label"] = _label.string();
   paramNode["descr"] = _descr.string();

   switch (_type)
   {
   case JsonParam::BOOL:
      paramNode["type"] = "bool";
      b = *(bool*)_value;
      paramNode["value"] = b;
      break;

   case JsonParam::INT:
      paramNode["type"] = "int";
      i = *(int*)_value;
      paramNode["value"] = i;
      break;

   case JsonParam::UINT:
      paramNode["type"] = "uint";
      n = *(unsigned int*)_value;
      paramNode["value"] = n;
      break;

   case JsonParam::FLOAT:
      paramNode["type"] = "float";
      f = *(double*)_value;
      paramNode["value"] = f;
      break;

   case JsonParam::STRING:
      paramNode["type"] = "string";
      s = *(string*)_value;
      paramNode["value"] = s;
      break;

   case JsonParam::VECTOR:
      paramNode["type"] = "vector";
      n = v.size();
      for (unsigned int j=0; j<n; ++j)
         paramNode["value"][j] = v[j];
      break;

   default:
      break;
   }
}

bool JsonParam::asBool() const
{
   if (_type == BOOL)
      return *(bool*)_value;
   return false;
}

unsigned int JsonParam::asUint() const
{
   if (_type == UINT)
      return *(unsigned int*)_value;
   return 0;
}

int  JsonParam::asInt() const
{
   if ((_type == INT) || (_type == UINT))
      return *(int*)_value;
   return 0;
}

double JsonParam::asFloat() const
{
   if (_type == FLOAT)
      return *(double*)_value;
   return ossim::nan();
}

std::string JsonParam::asString() const
{
   if (_type == STRING)
      return *(string*)_value;
   return "";
}

void JsonParam::asVector(std::vector<double>& v) const
{
   v.clear();
   if (_type == VECTOR)
      v = *(vector<double>*)_value;
}

ostream& operator<<(std::ostream& out, const JsonParam& obj)
{
   Json::Value jsonNode;
   obj.saveJSON(jsonNode);
   out << jsonNode << endl;
   return out;
}

//-------------------------------------------------------------

JsonConfig::JsonConfig()
{
   // This ctor could eventually curl a spring config server for the param JSON. For now it
   // is reading the installed share/ossim system directory for config JSON files.

   // The previous parameters list is cleared first for a fresh start:
   m_paramsMap.clear();
   ossimFilename configFilename;
   try
   {
      // First establish the directory location of the default config files:
      ossimFilename shareName (ossimPreferences::instance()->
         preferencesKWL().findKey( std::string( "ossim_share_directory" )));
      if (!shareName.isDir())
         throw ossimException("Nonexistent share drive provided for config files.");

      // Fetch all JSON files:
      ossimDirectory shareDir;
      if (!shareDir.open(shareName))
         throw ossimException("Share drive provided for config files is not readable.");
      std::vector<ossimFilename> jsonFiles;
      shareDir.findAllFilesThatMatch(jsonFiles, ".*\\.json");

      // Process those that contain the "parameters" JSON node:
      for (unsigned int i=0; i<jsonFiles.size(); i++)
      {
         configFilename = jsonFiles[i];
         if (!open(configFilename))
            throw ossimException("Bad file open or parse.");
      }

   }
   catch (ossimException& e)
   {
      ossimNotify(ossimNotifyLevel_WARN)<<"JsonConfig::readConfig():  Could not open/parse "
            "config file at <"<< configFilename << ">. Error: "<<e.what()<<endl;
   }
}

JsonConfig::JsonConfig(const ossimFilename& configFile)
{
   if (!open(configFile))
      throw ossimException("Bad file open or parse.");
}

JsonConfig::~JsonConfig()
{
   m_paramsMap.clear();
}

bool JsonConfig::open(const ossimFilename& configFileName)
{
   ifstream configFile (configFileName.string());
   if (configFile.fail())
      return false;
   Json::Value jsonRoot;
   configFile >> jsonRoot;
   if (jsonRoot.empty())
      return false;
   if (jsonRoot.isMember("parameters"))
   {
      Json::Value& paramsNode = jsonRoot["parameters"];
      loadJSON(paramsNode);
   }
   configFile.close();
   return true;
}

JsonParam& JsonConfig::getParameter(const char* paramName)
{
   map<string, JsonParam>::iterator i = m_paramsMap.find(string(paramName));
   if (i != m_paramsMap.end())
      return i->second;
   return s_nullParam;
}

void JsonConfig::setParameter(const JsonParam& p)
{
   // Tricky stuff to make sure it is a deep copy of the parameter:
   string key = p.name().string();
   std::map<std::string, JsonParam>::iterator iter = m_paramsMap.find(key);
   if (iter != m_paramsMap.end())
      m_paramsMap.erase(key);
   m_paramsMap.emplace(key, p);
}

bool JsonConfig::paramExists(const char* paramName) const
{
   map<string, JsonParam>::const_iterator i = m_paramsMap.find(string(paramName));
   if (i != m_paramsMap.end())
      return true;
   return false;
}

void JsonConfig::loadJSON(const Json::Value& json_node)
{
   Json::Value paramNode;

   // Support two forms: long (with full param descriptions and types), or short (just name: value)
   if (json_node.isArray())
   {
      // Long form:
      for (unsigned int i=0; i<json_node.size(); ++i)
      {
         paramNode = json_node[i];
         JsonParam p;
         if (p.loadJSON(paramNode))
            setParameter(p);
      }
   }
   else
   {
      // Short form expects a prior entry in the params map whose value will be overriden here:
      Json::Value::Members members = json_node.getMemberNames();
      for (size_t i=0; i<members.size(); ++i)
      {
         JsonParam& p = getParameter(members[i].c_str());
         if (p.name().empty())
         {
            ossimNotify(ossimNotifyLevel_WARN)<<"JsonConfig::loadJSON():  Attempted to override "
                  "nonexistent parameter <"<< members[i] << ">. Ignoring request."<<endl;
            continue;
         }
         if (p.descr().contains("DEPRECATED"))
         {
            ossimNotify(ossimNotifyLevel_WARN)<<"JsonConfig::loadJSON()  Parameter "<<p.name()
                  <<" "<<p.descr()<<endl;
            continue;
         }

         // Create a full JSON representation of the named parameter from the default list, replace
         // its value, and recreate the parameter from the updated full JSON:
         p.saveJSON(paramNode);
         paramNode["value"] = json_node[p.name().string()];
         p.loadJSON(paramNode);
     }
   }
}


void JsonConfig::saveJSON(Json::Value& json_node) const
{
   Json::Value paramNode;

   map<string, JsonParam>::const_iterator param = m_paramsMap.begin();
   int entry = 0;
   while (param != m_paramsMap.end())
   {
      param->second.saveJSON(paramNode);
      json_node[entry++] = paramNode;
      ++param;
   }
}

bool JsonConfig::diagnosticLevel(unsigned int level) const
{
   map<string, JsonParam>::const_iterator i = m_paramsMap.find(string("diagnosticLevel"));
   if (i != m_paramsMap.end())
   {
      unsigned int levelSetting = i->second.asUint();
      return (level <= levelSetting);
   }
   return false;
}

std::ostream& operator<<(std::ostream& out, const JsonConfig& obj)
{
   Json::Value configJsonNode;
   obj.saveJSON(configJsonNode);
   out<<configJsonNode<<endl;
   return out;
}

}
