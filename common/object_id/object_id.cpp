#include "object_id.h"

#include <utility>

namespace Core {
	namespace Common {
		unsigned int ObjectID::_mIDCount = 0;

		ObjectID::ObjectID() : _mID(_mIDCount) {
			_mIDCount++;
		}

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

		unsigned int ObjectID::getID() const {
			return _mID;
		}
	};
};
