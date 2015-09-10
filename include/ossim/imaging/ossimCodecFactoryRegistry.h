#ifndef ossimCodecFactoryRegistry_HEADER
#define ossimCodecFactoryRegistry_HEADER
#include <ossim/base/ossimFactoryListInterface.h>
#include <ossim/imaging/ossimCodecBase.h>
#include <ossim/imaging/ossimCodecFactoryInterface.h>

class OSSIM_DLL ossimCodecFactoryRegistry : public ossimFactoryListInterface<ossimCodecFactoryInterface, 
																	ossimCodecBase>
{
public:
	static ossimCodecFactoryRegistry* instance();
	virtual ~ossimCodecFactoryRegistry();

   /**
   * Will loop through all registered factories trying to allocate a codec for the passed
   * in type.
   *
   *  @param   in type.  Type of Codec to allocate.
   *  @return ossimCodecBase. Newly allocated Codec  
   */
   virtual ossimCodecBase* createCodec(const ossimString& type)const;

   /**
   * Will loop through all registered factories trying to allocate a codec for the passed
   * in type in the type keyword of the keywordlist.
   *
   * @param in kwl.  State infromation to load when allocating the codec
   * @param in prefix. prefix value for all keys
   *
   *  @return ossimCodecBase. Newly allocated codec
   */
   virtual ossimCodecBase* createCodec(const ossimKeywordlist& kwl, const char* prefix=0)const;


   /**
   * Loop through all factories and get a list of supported types.
   *
   * @param out typeNames. The resulting typename list.  Values will be appended.
   */
   virtual void getTypeNameList(std::vector<ossimString>& typeNames)const;


private:
   /** hidden from use default constructor */
   ossimCodecFactoryRegistry();

   /** hidden from use copy constructor */
   ossimCodecFactoryRegistry(const ossimCodecFactoryRegistry& obj);

   /** hidden from use operator = */
   const ossimCodecFactoryRegistry& operator=(const ossimCodecFactoryRegistry& rhs);

	static ossimCodecFactoryRegistry* m_instance;
};

#endif
