//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#pragma once

#include <ostream>
#include <ossim/plugin/ossimPluginConstants.h>
#include <ossim/base/JsonInterface.h>
#include <ossim/base/ossimFilename.h>
#include <vector>
#include <map>

namespace ossim
{
// Forward decl defined after JsonParam
class JsonParam;

/**
 * Base class for maintaining parameters affecting the runtime configuration of OSSIM executables.
 * The state is imported and exported via JSON. There are default configuration files that should
 * be part of the install, that are accessed by this class. Custom settings can also be loaded.
 *
 * There are two functionally equivalent forms for specifying parameters: long and short.
 * Parameters are initially declared via the long form with descriptions and default values. These
 * values must be supplied in default JSON files as part of the OSSIM install.
 *
 * Once the parameters are declared via the long form, the short form can be used to supply runtime
 * overrides.
 *
 * The long form format is
 *
 *    {  "parameters": [
 *          {
 *             "name": "<param_name>",
 *             "descr": "<param description",
 *             "type": "string"|"float"|"uint"|"int"|"bool",
 *             "value": <value>
 *          }, ...
 *       ]
 *    }
 *
 * The short form is:
 *
 *    {  "parameters": [
 *          "<param_name>": <value>, ...
 *       ]
 *    }
 *
 * The short form parameter is only accepted if it has previously been loaded via the long form so
 * that the data type is known.
 *
 * Parameters are usually accessed knowing the data type ahead of time. For example, a string
 * parameter is accessed as:
 *
 *    string paramVal = jsonConfig.getParameter("param_name").asSTring();
 *
 * If the parameter is not found, the special null-parameter is returned from getParameter(), and
 * casting to a tye will return 0, false, or empty string.
 */
class OSSIM_DLL JsonConfig : public ossim::JsonInterface
{
public:
   JsonConfig();

   /** Default Ctor loads all default .json files in the share/ossim system dir */
   JsonConfig(const ossimFilename& configFile);

   //! Destructor
   virtual ~JsonConfig();

   //! Opens and parses JSON file. The "parameters" keyword is expected in the root node
   bool open(const ossimFilename& configFile);

   //! Reads the params controlling the process from the JSON node named "parameters".
   virtual void loadJSON(const Json::Value& params_json_node);

   //! Reads the params controlling the process from the JSON node named "parameters".
   virtual void saveJSON(Json::Value& params_json_node) const;

   //! Returns a parameter (might be a null parameter if paramName not found in the configuration.
   JsonParam& getParameter(const char* paramName);

   /** Adds parameter to the configuration. Any previous parameter of the same name is replaced. */
   void setParameter(const JsonParam& p);

   //! Convenience method returns TRUE if the currently set diagnostic level is <= level
   bool diagnosticLevel(unsigned int level) const;

   //! Outputs JSON to output stream provided.
   friend std::ostream& operator<<(std::ostream& out, const JsonConfig& obj);

   bool paramExists(const char* paramName) const;

protected:
   JsonConfig(const JsonConfig& /*hide_this*/) {}

   bool getBoolValue(bool& rtn_val, const std::string& json_value) const;

   std::map<std::string, JsonParam> m_paramsMap;
   static JsonParam s_nullParam;
};


/**
 * Represents a single configuration parameter. This class provides for packing and unpacking the
 * parameter from JSON payload, and provides for handling all datatypes of parameters.
 */
class JsonParam
{
public:
   enum ParamType {
      UNASSIGNED=0,
      BOOL=1,
      INT=2,
      UINT=3,
      FLOAT=4,
      STRING=5,
      VECTOR=6
   };

   JsonParam() : _type(UNASSIGNED), _value(0) {}

   JsonParam(const ossimString& argname,
            const ossimString& arglabel,
            const ossimString& argdescr,
            ParamType   argparamType,
            void* value);

   JsonParam(const JsonParam& copy);

   ~JsonParam() { resetValue(); }

   /** Initializes from a JSON node. Return true if successful */
   bool loadJSON(const Json::Value& json_node);

   void saveJSON(Json::Value& json_node) const;

   const ossimString& name() const { return _name; }
   const ossimString& label() const { return _label; }
   const ossimString& descr() const { return _descr; }
   ParamType    type() const { return _type; }

   bool isBool() const  {return (_type == BOOL); }
   bool asBool() const;

   bool isUint() const  {return (_type == UINT); }
   unsigned int asUint() const;

   bool isInt() const  {return (_type == INT);}
   int  asInt() const;

   bool isFloat() const {return (_type == FLOAT);}
   double asFloat() const;

   bool isString() const {return (_type == STRING);}
   std::string asString() const;

   bool isVector() const {return (_type == VECTOR);}
   void asVector(std::vector<double>& v) const;

   bool operator==(const JsonParam& p) const { return (p._name == _name); }

   /** Outputs JSON to output stream provided */
   friend std::ostream& operator<<(std::ostream& out, const JsonParam& obj);

private:
   void setValue(void* value);
   void resetValue();

   ossimString _name;
   ossimString _label;
   ossimString _descr;
   ParamType   _type;
   void*       _value;
   std::vector<ossimString> _allowedValues; // only used for multiple-choice string parameters
};


}
