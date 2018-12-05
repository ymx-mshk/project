
#include "../../include/offline/DictProducer.h"
#include "../../include/offline/Mylogger.h"
#include "../../include/offline/Configuration.h"

#include <fstream>
#include <sstream>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

namespace dict
{

DictProducer::DictProducer(SplitTool * splitTool)
: _splitTool(splitTool){
	_dir = getConfig("dictionaryDir");
	if (_dir.size() == 0){
		LogError("No dictionary directory");
		exit(EXIT_FAILURE);
	}
}


void DictProducer::buildDict(){

	//先创建停用词词典
	getStopWordsDict(getConfig("stopWordsDict"));
	cout << _stopWordsDict.size() << "\n\n" << endl;

	getFiles();
	std::ifstream input;
	
	std::string line, word;
	vector<string> words;
	//处理英文文件
	for (const auto & file : _enFiles){
		input.open(file);
		if (input){
			LogInfo("Open dictionary file \"%s\" success", file.c_str());
			while (getline(input, line)){
				if (!line.size())
					continue;
					strProcess(line);
					istringstream ss(line);
					while (ss >> word){
						if (_stopWordsDict.find(word) == _stopWordsDict.end())
							_dict[word]++;
					}
				}
		}else {
			LogError("Open dictionary file \"%s\"failed", file.c_str());
		}
		input.close();
	}
	//处理中文文件
	for (const auto & file : _cnFiles){
		input.open(file);
		if (input){
			LogInfo("Open dictionary file \"%s\" success", file.c_str());
			while (getline(input, line)){
				if (!line.size())
					continue;
					_splitTool->cut(words, line);
					for (const auto & e : words){
						if (_stopWordsDict.find(e) == _stopWordsDict.end())
							_dict[e]++;
					}
			}
		}else {
			LogError("Open dictionary file \"%s\"failed", file.c_str());
		}
		input.close();
	}
	LogInfo("Build dict.dat success");
}

void DictProducer::getFiles(){
	getAllFiles(_enFiles, _dir + "/en");
	getAllFiles(_cnFiles, _dir + "/cn");
}

void DictProducer::getAllFiles(vector<string> & files, const string & path){
	DIR * dir = opendir(path.c_str());
	if (!dir){
		LogError("No such dirctory");
	}
	struct dirent * fileInfo;
	string file;
	
	while ((fileInfo = readdir(dir)) != NULL){
		//如果有子目录并且不是.和..，则进入该目录继续查找
		if (fileInfo->d_type == DT_DIR 
				&& strcmp(fileInfo->d_name, ".")
				&& strcmp(fileInfo->d_name, "..")){
			
			getAllFiles(files, path + "/" + fileInfo->d_name);
		}
		if (fileInfo->d_type == DT_REG){
			file = path + "/" + fileInfo->d_name;
			files.push_back(file);
			LogDebug("add \"%s\" to dictionary files", file.c_str());
		}
	}
	
}

void DictProducer::getStopWordsDict(const string & path){

	vector<string> files;
	getAllFiles(files, path);

	ifstream input;
	for (const string & file : files){
		input.open(file);
		if (input){
			LogInfo("Open stopwords dictionary file \"%s\" success", file.c_str());
		}else{
			LogWarn("Open stopwords dictionary file \"%s\" falied", file.c_str());
			input.close();
			return;
		}

		string word;
		while(input >> word){
	
			_stopWordsDict.insert(std::move(word));
		}
		input.close();
	}
}

void DictProducer::buildIndex(){
	size_t idx = 0;
	for (const auto & record : _dict){
		for (auto cit = record.first.cbegin();
				cit != record.first.cend();){

			string s;
			int len = getBytesCode(*cit);
			//s是一个utf-8字符，每个字符当做索引
			s = string(cit, cit + len);
			cit = cit + len;

			auto it = _index.find(s);
			if (it == _index.end()){
				_index.insert(std::make_pair(std::move(s), set<size_t>{idx}));
			}else {
				it->second.insert(idx);
			}
		}
		++idx;
	}
	LogInfo("Create index.dat success");
}

void DictProducer::store() const{
	std::string file = getConfig("dictionaryStoreFile");
	if (file.size() == 0){
		LogError("No dictionary store file");
		exit(EXIT_FAILURE);
	}

	ofstream output(file);
	for (const auto & record : _dict){
		output << record.first << "  "
			   << record.second << '\n';
	}
	output.close();

	file =  getConfig("indexStoreFile");
	if (file.size() == 0){
		LogError("No index store file");
		exit(EXIT_FAILURE);
	}

	output.open(file);
	for (const auto & record : _index){
		output << record.first << ':';
		for (const auto & i : record.second){
			output << ' ' << i;
		}
		output << '\n';
	}
	output.close();
}

void DictProducer::strProcess(string & str){
	for (auto & c : str){
		if (!isalpha(c)){
			c = ' ';
		}else{
			c = tolower(c);
		}
	}
}

//返回一个utf-8编码字符的字节数
int DictProducer::getBytesCode(char c){
	if (!(c & 0x80)){
		return 1;	
	}
	int cnt = 0;
	while (c & 0x80){
		c <<= 1;
		cnt ++;
	}
	return cnt;
}

}
