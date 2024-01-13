#ifndef H_GC
#define H_GC

#include <vector>

namespace PrettyEngine {
	/// Object that can be used by a GrabageCollector.
	class GCObject {
	public:
		unsigned int userCount = 0;
		bool useGC = true;
	};
}

#endif