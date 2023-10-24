#pragma once

namespace Core {
	namespace Common {

		class ObjectID {
			public:
				ObjectID();
				~ObjectID() = default;

				ObjectID(const ObjectID& other);
				ObjectID& operator = (const ObjectID& other);

				ObjectID(ObjectID && other);
				ObjectID& operator = (ObjectID && other);

				unsigned int getID() const;

			private:
				unsigned int _mID;

				static unsigned int _mIDCount;
		};
	};
};

