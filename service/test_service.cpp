
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>

#include <cassert>
#include <string>
#include <leveldb/db.h>

#include <glog/logging.h>

#include "libdf/libdataflow.h"

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast/http.hpp>

#define TEST_PLUGIN_PATH "./plugins/"

//c++ - Error: Expected template-name before '<' token, you need to add namespace "namespace dataflow" before the CDataProducer
class CDemoProducer : public dataFlow::CDataProducer<int>
{
public:
	int start(int j)
	{
		/* initial list.c++ 11 function . lamda funtion capture this .*/
		m_testTrd = std::thread{[this, j]()
			{
				run(j);
			}};
	}
	int wait_stop()
	{
		m_testTrd.join();
	}
private:
	void run(int j)
	{
		while(true)
		{
			produce(j++); //	this->produce(j++);
			if( j > 16 )
			{
				break;
			}
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
private:
	std::thread m_testTrd;
};

class CDemoConsumer : public dataFlow::CDataConsumer<CDemoConsumer, int>
{
/* in c++, default member function and member variable accessed property is private. */
public:
	void consume(int i)
	{
		//std::cout<<"consume:i="<< i << std::endl;
		//LOG(ERROR)<<"consume:i="<< i;
		LOG_IF_EVERY_N(ERROR, i > 10, 3)<<"consume:i="<< i;
	}
};

int testDataFlow()
{
	//add a thread session
	const auto spDemoProducer = std::make_shared<CDemoProducer>();
	const auto spDemoConsumer = std::make_shared<CDemoConsumer>();

	dataFlow::BindOutput(spDemoProducer, spDemoConsumer);

	spDemoProducer->start(15);
	spDemoProducer->wait_stop();
}

int main(int argc, char** argv)
{
    dataFlow::Initialize(argc, argv);

    dataFlow::AddPluginDir(TEST_PLUGIN_PATH);
    dataFlow::LoadALLPlugins();


	testDataFlow();

    return 0;
}

/*to do:
 *1、+ gflags
 *2、+ glog
 *3、+ mysql levelDb mongoDb
 *4、+ kafka + redis
 *5、 + boost asio
 */