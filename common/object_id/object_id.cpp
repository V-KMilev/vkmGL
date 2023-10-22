#include "object_id.h"

#include <utility>

namespace Core {
	namespace Common {
		ObjectID::ObjectID(uint32_t id) : _mID(id) {}
		ObjectID::~ObjectID() {}

		ObjectID::ObjectID(const ObjectID& other) {
			this->_mID = other._mID;
		}

		ObjectID& ObjectID::operator = (const ObjectID& other) {
			if (this == &other) {
				return *this;
			}

			this->_mID = other._mID;
			return *this;
		}

		ObjectID::ObjectID(ObjectID&& other) {
			this->_mID = std::move(other._mID);
		}

		ObjectID& ObjectID::operator = (ObjectID&& other) {
			if (this == &other) {
				return *this;
			}

			this->_mID = std::move(other._mID);
			return *this;
		}
	};
};
