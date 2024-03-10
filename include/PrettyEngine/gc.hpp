#ifndef H_GC
#define H_GC

namespace PrettyEngine {
	/// Object that can be used by a GarbageCollector.
	/// Collected externally because of needs link to the use context.
	class GCObject {
	public:
		unsigned int userCount = 0;
		bool useGC = true;
	};

	/// Wrapper for a GCObject to protect from a garbage collection at the wrong moment.
	template<typename T>
	class GCObjectPtr {
	public:
		GCObjectPtr(GCObject* newPtr) {
			this->_ptr = newPtr;
			newPtr->userCount++;
		}

		~GCObjectPtr() {
			this->_ptr->userCount -= 1;
		}

		T* Get() {
			return this->_ptr;
		}
 
 	private:
		T* _ptr;
	};
}

#endif