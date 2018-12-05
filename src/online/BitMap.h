
#ifndef __MYSP_BITMAP_H__
#define __MYSP_BITMAP_H__

#include <vector>
using namespace std;

namespace mysp
{

class BitMap{
	public:
		explicit BitMap(const size_t);

		bool isSet(const size_t) const;
		void setMap(const size_t);
	private:
		vector<char> _bitmap;
};

}

#endif
