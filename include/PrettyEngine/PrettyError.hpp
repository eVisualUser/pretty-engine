#ifndef HPP_PRETTY_ERROR
#define HPP_PRETTY_ERROR

#include <PrettyEngine/debug/debug.hpp>

#include <cstdlib>
#include <functional>

namespace PrettyEngine {
	template <typename T>
	class Error {
	public:
		Error(std::string errorMessage, bool test, T newValue) {
			this->message = errorMessage;
			this->state = test;
			this->_value = newValue;
		}

		T CrashError() {
			if (this->state) {
				DebugLog(LOG_ERROR, this->message, true);
				throw this->message;
				exit(-1);
			}

			return this->value;
		}

		T ShowError() {
			if (this->state) {
				DebugLog(LOG_ERROR, this->message, true);
			}
			return this->_value;
		}

		Error* Resolve(std::function<bool(T* value)> exceptFunction) {
			if (this->state) {
				this->state = (exceptFunction)(&this->_value);
			}

			return this;
		}

		T* GetValue() {
			return &this->_value;
		}

		bool state;
		std::string message;

	private:
		T _value;
	};

	template <typename T>
	class Option {
	public:
		Option(T newValue) {
			this->value = newValue;
		}

		T* GetValue() {
			return this->value;
		}

		bool HaveValue() {
			return this->value != nullptr;
		}

		void HaveValue(std::function<void(T)> update) {
			if (this->HaveValue()) {
				(update)(this->value);
			}
		}

	private:
		T value;
	};
}

#endif