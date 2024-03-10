#ifndef HPP_SIMPLEX
#define HPP_SIMPLEX

#include <glm/glm.hpp>
#include <array>

namespace PrettyEngine {
	class Simplex {
	public:
		Simplex(int initSize = 4) {
			for (int i = 0; i < 4; i++) {
				this->_points[i] = glm::vec3(0.0f, 0.0f, 0.0f);
			}
		}

		Simplex& operator=(std::initializer_list<glm::vec3> list) { 
			for (auto &point: list) {
				this->_points[this->_size] = point;
				this->_size++;
			}
			this->_size = std::min(this->_size + 1, 4);
			
			return *this;
		}

		void PushFront(glm::vec3 point) { 
			this->_size++;
			this->_size = std::min(this->_size + 1, 4);

			for (char i = 0; i < 4; i++) {
				glm::vec3 local = this->_points[i];
				this->_points[i] = point;
				point = local;
			}
		}

		glm::vec3 &operator[](int i) { return this->_points[i]; }
		size_t size() const { return this->_size; }

		auto begin() const { return this->_points.begin(); }
		auto end() const { return this->_points.end() - (4 - this->_size); }

	private:
		std::array<glm::vec3, 4> _points;
		int _size = 0;
	};
}

#endif