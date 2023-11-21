#ifndef H_GC
#define H_GC

namespace PrettyEngine {
	class GCObject {
	public:
		unsigned int userCount = 0;
		bool useGC = true;
	};
}

#endif