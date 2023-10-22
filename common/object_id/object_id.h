#pragma once

#include <cstdint>

namespace Core {
	namespace Common {

		class ObjectID {
			public:
				ObjectID(uint32_t id);
				~ObjectID();

				ObjectID(const ObjectID& other);
				ObjectID& operator = (const ObjectID& other);

				ObjectID(ObjectID && other);
				ObjectID& operator = (ObjectID && other);

				bool init();
				bool deinit();

			private:
				uint32_t _mID;
		};
	};
};
