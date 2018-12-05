
#include "MyTask.h"
#include "BitMap.h"
#include "Mylogger.h"
#include "Dictionary.h"
#include "CacheManager.h"
#include "Pthread.h"

#include <json/json.h>

namespace mysp
{

MyTask::MyTask(const string & queryWord, const TcpConnectionPtr & conn)
: _queryWord(queryWord)
, _conn(conn){
}

void MyTask::excute(){

	Cache & cache = CacheManager::getCache(atoi(pthreadName));
	string result = cache.getElement(_queryWord);
	if (result.size()){
		LogInfo("find %s's query result in Cache", _queryWord);
		_conn->sendInLoop(result);
		return;
	}else {
		LogInfo("donnot find %s's query result in Cache", _queryWord);
	}

	auto dict = getDict();
	auto index = getIndexTable();

	BitMap candidata(dict.size());

	for (int i = 0; i != getLength(_queryWord); ++i){
		string s = getNthChar(_queryWord, i);
		auto container = index[s];
		for (int idx : container){
			candidata.setMap(idx);
		}
	}

	for (size_t idx = 0; idx != dict.size(); ++idx){
		if (candidata.isSet(idx)){
			int dist = distance(dict[idx].first);
			if (dist <= 3){
				_resultQue.push(MyResult(
							dict[idx].first,
							dict[idx].second,
							dist));
			}
		}
	}

	response();
}

void MyTask::response(){
	Json::Value item, arr;

	for (size_t i = 0; i != 6; ++i){
		if (_resultQue.size()){
			arr.append(_resultQue.top()._word);
			_resultQue.pop();
		}else{
			break;
		}
	}
	string sendItem = arr.toStyledString();

	if (_resultQue.size())
		CacheManager::getCache(atoi(pthreadName)).addElement(_queryWord, sendItem);
	_conn->sendInLoop(sendItem);
}

int MyTask::distance(const string & rhs){
	const int size1 = getLength(_queryWord), size2 = getLength(rhs);
	if (size1 == 0)
		return size2;
	if (size2 == 0)
		return size1;

	//ed[i][j]表示 word1[0]~word1[i-1]与word2[0]~word2[j-1]之间的最小编辑距离
	vector<vector<int>> ed(size1 + 1, vector<int>(size2 + 1, 0));

	// 初始化任意字符与空字符之间的编辑距离
	for (int i = 0; i != size1 + 1; ++i)
	{
		ed[i][0] = i;
	}
	for (int i = 0; i != size2 + 1; ++i)
	{
		ed[0][i] = i;
	}

	for (int i = 1; i != size1 + 1; ++i)
	{
		for (int j = 1; j != size2 + 1; ++j)
		{
			if (getNthChar(_queryWord, i - 1) == getNthChar(rhs, j - 1))
				ed[i][j] = ed[i - 1][j - 1];
			else
				// 将3种编辑结尾的方法都尝试，取最小的结果
				ed[i][j] = min(min(ed[i][j - 1], ed[i - 1][j]), ed[i - 1][j - 1]) + 1;
		}
	}
	return ed[size1][size2];
}

string MyTask::getNthChar(const string & word, int n){
	int pos = 0;
	while (n-- ){
		pos += getBytesCode(word[pos]);
	}
	return string(word, pos, getBytesCode(word[pos]));

}
int MyTask::getLength(const string & word){
	int cnt = 0;
	for (auto cit = word.cbegin(); cit != word.cend();){
		cit += getBytesCode(*cit);
		++cnt;
	}

	return cnt;
}

int MyTask::getBytesCode(char c){
	if (!(c & 0x80)){
		return 1;	
	}
	int cnt = 0;
	do {
		c <<= 1;
		cnt ++;
	}while (c & 0x80);
	return cnt;
}

}
