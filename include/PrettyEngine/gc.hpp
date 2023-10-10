#pragma once

namespace PrettyEngine {
	class GCObject {
	public:
		unsigned int userCount = 0;
		bool useGC = true;
	};
}
