#ifndef __DATAUTILS_H__
#define __DATAUTILS_H__

#include <iterator>

namespace DataUtils {
	template<typename RandomIt,typename T>
	T* binarySearchExlusive(RandomIt first, RandomIt last, const T& value, int (*compare)(const T& a, const T& b)) {
		RandomIt from = first;
		RandomIt to = last;
		while (from != to) {
			RandomIt mid = from + (to-from) / 2;

			int cmp = compare(value, *mid);

			if (cmp == 0) {
				return mid;
			}
			else if (cmp < 0) {
				if (mid == to)
					goto fail;
				to = mid;
			}
			else {
				if (mid == from)
					goto fail;
				from = mid;
			}
		}

		if (compare(value, *from) == 0)
			return from;

	fail:
		return nullptr;
	}
}

#endif