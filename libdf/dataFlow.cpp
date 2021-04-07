
#include <memory>
#include <map>
#include <string>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#include <boost/dll/import.hpp>
#include <boost/function.hpp>
#include "gflags/gflags.h"
#pragma GCC diagnostic pop

#include "comm.h"
#include "libdataflow.h"

/* 开发插件 */
DEFINE_string(confPath, "test.conf", "program configure file.");
DEFINE_int32(port, 1111, "program listen port");
DEFINE_bool(debug, true, "run debug mode");

namespace dataFlow {

#define PLUGIN_EXTENSION ".so"

	class Plugin {
		using LoadSignature    = boost::function<int()>;
		using UnLoadSignature  = boost::function<void ()>;
		using NameSignature    = boost::function<std::string& ()>;
		using VersionSignature = boost::function<std::string& ()>;
	public:
		int open(boost::filesystem::path path) {
			boost::dll::load_mode::type LoadMode = boost::dll::load_mode::rtld_local;

			m_fnLoad = boost::dll::import<LoadSignature>(path, "initialize", LoadMode);
		};
		~Plugin() {
		};

	private:
		std::string m_productName;
	
	public:
		//boost::function<int (void)> m_fnLoad;
		LoadSignature m_fnLoad;
		UnLoadSignature m_fnUnLoad;
		NameSignature m_fnPluginName;
		VersionSignature m_fnPluginVersion;
	};

	static std::unique_ptr<CDataFlow> g_pstCore;

	class CDataFlow
	{
		//class iterator;

	public:
		CDataFlow() = default;
		//~CDataFlow() = delete;

		int IsPluginExists(std::string plugin_name) {
			return boost::filesystem::exists(plugin_name);
		}

		int AddPluginDir(std::string& PluginDir) {
			m_pluginDirPath.push_back(PluginDir);
		}

		int LoadAllPlugins() {
			int  iPluginCount = 0;
			for (const auto& dir : m_pluginDirPath) 
			{
				using namespace boost::filesystem;
				//for ( boost::filesystem::directory_entry const& entry : boost::filesystem::recursive_directory_iterator(dir, 
				//														boost::filesystem::symlink_option::none))
				for (directory_entry const& entry : recursive_directory_iterator(dir, symlink_option::none))
				{
					auto ext = entry.path().extension().string();
					if (ext != PLUGIN_EXTENSION)
					{
						continue;
					}
					auto plugin = std::make_shared<Plugin>(entry.path());
					if (!plugin->open(ext)) {
						continue;
					}

					if (IsPluginExists(plugin->m_fnPluginName())) {
						continue;
					}

					iPluginCount++;
					m_plugins[plugin->m_fnPluginName()] = plugin;
				}
			}
			return iPluginCount;
		}

		template<typename T>
		bool registerFactory(std::map<std::string, std::shared_ptr<T>> &map, std::shared_ptr<T> spFactory) {
            auto it = map.find(spFactory->getFactoryName());
            if( it !=  factoryMap.end())
			{
                //std::cout<<"Factory name conflicted:"<<spFactory->getFactoryName();
				return false;
			}
			return map.emplace(spFactory);
		}
#if 1	
		template<typename TCFactoryMapType>
		typename TCFactoryMapType::map_type getFactory(const TCFactoryMapType& factoryMap, const std::string strProductName) {
			
			//遍历各个map，返回对应的factory
			auto it = factoryMap.find(strProductName);
			if( it ==  factoryMap.end())
			{
				return nullptr;
			}

			return std::shared_ptr<typename TCFactoryMapType::map_type>(it->second, [typename TCFactoryMapType::map_type::elements*]()
			{
			});
		}

		std::shared_ptr<CProducerFactory> findProducerFactoryByName(const std::string strProduceName)
		{
			return getFactory(m_ProducerMap, strProduceName);
		}

		std::shared_ptr<CConsumerFactory> findConsumerFactoryByName(const std::string strConsumerName)
		{
			return getFactory(m_ConsumerMap, strConsumerName);
		}

		std::shared_ptr<CTransformerFactory> findTransformerFactoryByName(const std::string strTransformerName)
		{
			return getFactory(m_TransformerMap, strTransformerName);
		}
#endif
		bool registerProducerFactory(const std::shared_ptr<CProducerFactory> spFactory) {
			registerFactory(m_ProducerMap, spFactory);//自动推导，这里调用模板函数，不需要再写模板类型
		}

		bool registerConsumerFactory(const std::shared_ptr<CConsumerFactory> spFactory) {
			registerFactory(m_ConsumerMap, spFactory);
		}

		bool registerTransformerFactory(const std::shared_ptr<CTransformerFactory> spFactory) {
			return registerFactory(m_TransformerMap, spFactory);
		}
#if 0
		iterator begin() {
			new iterator(this);
		}
#endif
	private:
		std::map<std::string, std::shared_ptr<CProducerFactory>> m_ProducerMap;
		std::map<std::string, std::shared_ptr<CConsumerFactory>> m_ConsumerMap;
		std::map<std::string, std::shared_ptr<CTransformerFactory>> m_TransformerMap;
		std::map<std::string, std::shared_ptr<Plugin>> m_plugins;

		std::vector<std::string> m_pluginDirPath;
	};

	int UCore_Init() {
		g_pstCore->reset(std::unique_ptr<CDataFlow>(new CDataFlow));
	}

	DF_API LONG Initialize(int argc, char** argv)
	{
		//parse the command line arguments through the google flags
		gflags::ParseCommandLineFlags(&argc, &argv, true); 

		if (FLAGS_debug) {
   			std::cout << "this is a debug mode..." << std::endl;
  		}

		//log configuration , about path.

		//
		g_pstCore = std::make_unique<CDataFlow>();
	}
	DF_API LONG Uninitialize(int argc, char** argv);
	
	//for plugin side call.
	DF_API VOID RegisterProducerFactory(const std::shared_ptr<CProducerFactory> spProducer)
	{
		g_pstCore->registerProducerFactory(spProducer);
	}
	DF_API VOID RegisterConsumerFactory(const std::shared_ptr<CConsumerFactory> spConsumer)
	{
		g_pstCore->registerConsumerFactory(spConsumer);
	}
	DF_API VOID RegisterTransformerFactory(const std::shared_ptr<CTransformerFactory> spTransformer)
	{
		g_pstCore->registerTransformerFactory(spTransformer);
	}
	
	//for process side call.
	DF_API std::shared_ptr<CProducerFactory> FindProducerFactoryByName(boost::string_view FactoryName)
	{
		//g_pstCore->
	}
	DF_API std::shared_ptr<CConsumerFactory> FindConsumerFactoryByName(boost::string_view FactoryName);
	DF_API std::shared_ptr<CTransformerFactory> FindTransformerFactoryByName(boost::string_view FactoryName);
	
	//for process side call.
	DF_API bool AddPluginDir(boost::string_view pluginDir);	
	DF_API DFErrorCode LoadPlugin(boost::string_view pluginPath); //when loading plugin, you need to register the factory
	DF_API DFErrorCode RemovePlugin(boost::string_view pluginPath);
	
	DF_API LONG LoadALLPlugins();
	DF_API VOID RemoveAllPlugins();
	
	DF_API AddExitFun();
}
