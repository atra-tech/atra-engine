
#include "common.h"

#define DECLARE_TYPEID(type) \
	extern u64 G_COMPONENT_COUNT; \
    static u64 type##_TypeID

#define REGISTER_TYPEID(type) \
	type##_TypeID = G_COMPONENT_COUNT++

#define IMPORT_TYPEID(type) \
	extern static u64 type##_TypeID

#define TYPEID(type) \
	type##_TypeID

typedef struct TestPos {
	int x, y;
} TestPos;

