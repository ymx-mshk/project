
#include "../../include/first/Configuration.h"
#include "../../include/first/Mylogger.h"

#include <stdlib.h>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <iostream>
#include <fstream>

namespace dict
{

Configuration * Configuration::_pInstance;

Configuration::Configuration(const string & file){
	init(file);
}

Configuration * Configuration::getInstance(){
	if (!_pInstance){
		//这里把配置文件路径写死了，也可以给getInstance方法添加string参数
		//默认值为string()，然后在主函数里调用getInstance并传入配置文件路径
		_pInstance = new Configuration("/home/yuki/Cpp/project/conf/dict.conf");
		atexit(destroy);
	}
	return _pInstance;
}

void Configuration::destroy(){
	if (_pInstance)
		delete _pInstance;
}

void Configuration::init(const string & file){
	std::ifstream input(file);

	if (!input){
		LogError("Open config file \"%s\" failed", file.c_str());
		exit(EXIT_FAILURE);
	}else{
		LogInfo("Open config file \"%s\" success", file.c_str());
	}

	std::string line;
	std::vector<string> vec;
	while (getline(input, line)){
		if (!line.size())
			continue;
		//使用boost库的字符串分割函数
		boost::split(vec, line, boost::is_any_of(" \t,:"), boost::token_compress_on);
		//因为配置文件都是键值对,而且每项一行，所以vec里只有两个元素
		_conf.emplace(vec[0], vec[1]);
	}

	input.close();
}

}
