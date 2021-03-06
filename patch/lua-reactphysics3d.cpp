﻿#define LUA_LIB

#ifdef __cplusplus
extern "C" {
#endif
#include <lauxlib.h>
#include <lua.h>

LUAMOD_API int luaopen_reactphysics3d(lua_State* L);

#ifdef __cplusplus
}
#endif

#include "b3r32.h"
#include <LuaBridge/LuaBridge.h>
#include <fix16.h>
#include <reactphysics3d/reactphysics3d.h>

static reactphysics3d::PhysicsCommon physicsCommon;

class lCommon {
public:
    static reactphysics3d::PhysicsWorld* createPhysicsWorld()
    {

        // Create a physics world
        reactphysics3d::PhysicsWorld* world = physicsCommon.createPhysicsWorld();
        return world;
    }
    static void destroyPhysicsWorld(reactphysics3d::PhysicsWorld* world)
    {
        physicsCommon.destroyPhysicsWorld(world);
    }

    /// Create and return a sphere collision shape
    static reactphysics3d::SphereShape* createSphereShape(const reactphysics3d::decimal radius)
    {
        return physicsCommon.createSphereShape(radius);
    }

    /// Destroy a sphere collision shape
    static void destroySphereShape(reactphysics3d::SphereShape* sphereShape)
    {
        return physicsCommon.destroySphereShape(sphereShape);
    }

    /// Create and return a box collision shape
    static reactphysics3d::BoxShape* createBoxShape(const reactphysics3d::Vector3& extent)
    {
        return physicsCommon.createBoxShape(extent);
    }

    /// Destroy a box collision shape
    static void destroyBoxShape(reactphysics3d::BoxShape* boxShape)
    {
        physicsCommon.destroyBoxShape(boxShape);
    }

    /// Create and return a capsule shape
    static reactphysics3d::CapsuleShape* createCapsuleShape(reactphysics3d::decimal radius, reactphysics3d::decimal height)
    {
        return physicsCommon.createCapsuleShape(radius, height);
    }

    /// Destroy a capsule collision shape
    static void destroyCapsuleShape(reactphysics3d::CapsuleShape* capsuleShape)
    {
        physicsCommon.destroyCapsuleShape(capsuleShape);
    }

    /// Create and return a convex mesh shape
    static reactphysics3d::ConvexMeshShape* createConvexMeshShape(reactphysics3d::PolyhedronMesh* polyhedronMesh, const reactphysics3d::Vector3& scaling = reactphysics3d::Vector3(1, 1, 1))
    {
        return physicsCommon.createConvexMeshShape(polyhedronMesh, scaling);
    }

    /// Destroy a convex mesh shape
    static void destroyConvexMeshShape(reactphysics3d::ConvexMeshShape* convexMeshShape)
    {
        physicsCommon.destroyConvexMeshShape(convexMeshShape);
    }

    /// Create and return a height-field shape
    static reactphysics3d::HeightFieldShape* createHeightFieldShape(int nbGridColumns, int nbGridRows, reactphysics3d::decimal minHeight, reactphysics3d::decimal maxHeight,
        const void* heightFieldData, reactphysics3d::HeightFieldShape::HeightDataType dataType,
        int upAxis = 1, reactphysics3d::decimal integerHeightScale = 1.0f,
        const reactphysics3d::Vector3& scaling = reactphysics3d::Vector3(1, 1, 1))
    {
        return physicsCommon.createHeightFieldShape(nbGridColumns, nbGridRows, minHeight, maxHeight, heightFieldData, dataType, upAxis, integerHeightScale, scaling);
    }

    /// Destroy a height-field shape
    static void destroyHeightFieldShape(reactphysics3d::HeightFieldShape* heightFieldShape)
    {
        physicsCommon.destroyHeightFieldShape(heightFieldShape);
    }

    /// Create and return a concave mesh shape
    static reactphysics3d::ConcaveMeshShape* createConcaveMeshShape(reactphysics3d::TriangleMesh* triangleMesh, const reactphysics3d::Vector3& scaling = reactphysics3d::Vector3(1, 1, 1))
    {
        return physicsCommon.createConcaveMeshShape(triangleMesh, scaling);
    }

    /// Destroy a concave mesh shape
    void destroyConcaveMeshShape(reactphysics3d::ConcaveMeshShape* concaveMeshShape)
    {
        physicsCommon.destroyConcaveMeshShape(concaveMeshShape);
    }
};

class lEventListener : public reactphysics3d::EventListener {
public:
    lEventListener()
        : EventListener()
    {
        L = NULL;
    }
    virtual ~lEventListener() {}
    int lregister(lua_State* L)
    {
        this->L = L;
        auto l = luabridge::Stack<lEventListener*>::get(L, 1);
        lua_settop(L, 2);
        if (lua_isfunction(L, 2)) {
            lua_getglobal(L, "rp3d");
            if (lua_isnil(L, -1)) {
                lua_createtable(L, 0, 2);
                lua_setglobal(L, "rp3d");
                lua_getglobal(L, "rp3d");
            }
            lua_pushvalue(L, 2);
            lua_rawsetp(L, -2, l);
        } else {
            luaL_error(L, "2th arg must be function.");
        }
        return 0;
    }
    void onContact(const CollisionCallback::CallbackData& callbackData) override
    {
        assert(L != NULL);
        lua_getglobal(L, "rp3d");
        lua_rawgetp(L, -1, this);
        reactphysics3d::uint nb = callbackData.getNbContactPairs();
        lua_createtable(L, nb, 0);
        for (size_t i = 0; i < nb; i++) {
            reactphysics3d::CollisionCallback::ContactPair pair = callbackData.getContactPair(0);
            lua_createtable(L, 0, 0);
            luabridge::Stack<reactphysics3d::CollisionBody*>::push(L, pair.getBody1());
            lua_setfield(L, -2, "body1");
            luabridge::Stack<reactphysics3d::CollisionBody*>::push(L, pair.getBody2());
            lua_setfield(L, -2, "body2");
            lua_pushvalue(L, -2);
        }

        lua_setfield(L, -2, "proxyShape2");
        lua_pcall(L, 1, 0, -2);
    }

    void onTrigger(const reactphysics3d::OverlapCallback::CallbackData& callbackData) override
    {
    }

private:
    lua_State* L;
};

static int
lr32new(lua_State* L, int32_t const& i);

static int
lr32add(lua_State* L)
{
    lua_settop(L, 2);
    lua_getfield(L, 1, "__i__");
    lua_Integer a = luaL_checkinteger(L, -1);
    lua_getfield(L, 2, "__i__");
    lua_Integer b = luaL_checkinteger(L, -1);
    fix16_t v = fix16_sadd((fix16_t)a, (fix16_t)b);
    lr32new(L, v);
    return 1;
}

static int
lr32sub(lua_State* L)
{
    lua_settop(L, 2);
    lua_getfield(L, 1, "__i__");
    lua_Integer a = luaL_checkinteger(L, -1);
    lua_getfield(L, 2, "__i__");
    lua_Integer b = luaL_checkinteger(L, -1);
    fix16_t v = fix16_ssub((fix16_t)a, (fix16_t)b);
    lr32new(L, v);
    return 1;
}

static int
lr32mul(lua_State* L)
{
    lua_settop(L, 2);
    lua_getfield(L, 1, "__i__");
    lua_Integer a = luaL_checkinteger(L, -1);
    lua_getfield(L, 2, "__i__");
    lua_Integer b = luaL_checkinteger(L, -1);
    fix16_t v = fix16_smul((fix16_t)a, (fix16_t)b);
    lr32new(L, v);
    return 1;
}

static int
lr32div(lua_State* L)
{
    lua_settop(L, 2);
    lua_getfield(L, 1, "__i__");
    lua_Integer a = luaL_checkinteger(L, -1);
    lua_getfield(L, 2, "__i__");
    lua_Integer b = luaL_checkinteger(L, -1);
    fix16_t v = fix16_sdiv((fix16_t)a, (fix16_t)b);
    lr32new(L, v);
    return 1;
}

static int
lr32tostring(lua_State* L)
{
    lua_settop(L, 1);
    lua_getfield(L, 1, "__i__");
    lua_Integer a = luaL_checkinteger(L, -1);
    float f = fix16_to_float(a);
    char buffer[128] = {0};
    snprintf(buffer, 128, "fp: %f", f);
    lua_pushstring(L, buffer);
    return 1;
}

static int
printTable(lua_State* L)
{
    lua_pushnil(L); /* first key */
    while (lua_next(L, -2) != 0) {
        /* uses 'key' (at index -2) and 'value' (at index -1) */
        printf("%s - %s\n",
            lua_typename(L, lua_type(L, -2)),
            lua_typename(L, lua_type(L, -1)));
        /* removes 'value'; keeps 'key' for next iteration */
        printf("%s - %s\n",
            lua_tostring(L, -2),
            "h");
        lua_pop(L, 1);
    }
}

static int
lr32new(lua_State* L, int32_t const& i)
{
    // 跟多是查看数据
    lua_createtable(L, 0, 1);
    lua_pushinteger(L, i);
    lua_setfield(L, -2, "__i__");
    // meta
    luaL_Reg l[] = {
        {"__add", lr32add},
        {"__sub", lr32sub},
        {"__mul", lr32mul},
        {"__div", lr32div},
        {"__tostring", lr32tostring},
        {NULL, NULL},
    };
    luaL_newlib(L, l);
    lua_setmetatable(L, -2);
    return 0;
}

namespace luabridge {

// math
template <>
struct Stack<b3R32> {
    static void push(lua_State* L, b3R32 const& r)
    {
        lr32new(L, r._i);
        lua_getfield(L, -1, "__i__");
        assert(luaL_checkinteger(L, -1) == r._i);
        lua_pop(L, 1);
    }

    static b3R32 get(lua_State* L, int index)
    {
        if (lua_isnumber(L, index)) {
            lua_Number i = lua_tonumber(L, index);
            return b3R32(i);
        } else if (lua_isinteger(L, index)) {
            lua_Integer i = lua_tointeger(L, index);
            return b3R32(i);
        } else if (lua_istable(L, index)) {
            lua_pushvalue(L, index);
            lua_getfield(L, -1, "__i__");
            lua_Integer i = luaL_checkinteger(L, -1);
            lua_pop(L, 2);
            b3R32 r;
            r._i = i;
            return r;
        } else {
            luaL_error(L, "#%d argments must be table, type is %s", index, lua_typename(L, index));
        }
        return b3R32::zero();
    }
};

template <>
struct Stack<b3R32 const&> : Stack<b3R32> {
};

template <>
struct Stack<reactphysics3d::Vector2> {
    static void push(lua_State* L, reactphysics3d::Vector2 const& vec2)
    {
        lua_createtable(L, 0, 2);
        Stack<reactphysics3d::decimal>::push(L, vec2.x);
        lua_setfield(L, -2, "x");
        Stack<reactphysics3d::decimal>::push(L, vec2.y);
        lua_setfield(L, -2, "y");

        // create meta bable
    }

    static reactphysics3d::Vector2 get(lua_State* L, int index)
    {
        if (!lua_istable(L, index)) {
            luaL_error(L, "#%d argments must be table", index);
        }
        reactphysics3d::Vector2 vec2;
        lua_pushvalue(L, index);
        lua_getfield(L, -1, "x");
        vec2.x = Stack<reactphysics3d::decimal>::get(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "y");
        vec2.y = Stack<reactphysics3d::decimal>::get(L, -1);
        lua_pop(L, 2);
        return vec2;
    }
};

template <>
struct Stack<reactphysics3d::Vector2 const&> : Stack<reactphysics3d::Vector2> {
};

template <>
struct Stack<reactphysics3d::Vector3> {
    static void push(lua_State* L, reactphysics3d::Vector3 const& vec3)
    {
        lua_createtable(L, 0, 3);
        Stack<reactphysics3d::decimal>::push(L, vec3.x);
        lua_setfield(L, -2, "x");
        Stack<reactphysics3d::decimal>::push(L, vec3.y);
        lua_setfield(L, -2, "y");
        Stack<reactphysics3d::decimal>::push(L, vec3.z);
        lua_setfield(L, -2, "z");

        // create meta bable
    }

    static reactphysics3d::Vector3 get(lua_State* L, int index)
    {
        if (!lua_istable(L, index)) {
            luaL_error(L, "#%d argments must be table", index);
        }
        reactphysics3d::Vector3 vec3;
        lua_pushvalue(L, index);
        lua_getfield(L, -1, "x");
        vec3.x = Stack<reactphysics3d::decimal>::get(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "y");
        vec3.y = Stack<reactphysics3d::decimal>::get(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "z");
        vec3.z = Stack<reactphysics3d::decimal>::get(L, -1);
        lua_pop(L, 2);
        return vec3;
    }
};

template <>
struct Stack<reactphysics3d::Vector3 const&> : Stack<reactphysics3d::Vector3> {
};

template <>
struct Stack<reactphysics3d::Quaternion> {
    static void push(lua_State* L, reactphysics3d::Quaternion const& quat)
    {
        lua_createtable(L, 0, 4);
        Stack<reactphysics3d::decimal>::push(L, quat.x);
        lua_setfield(L, -2, "x");
        Stack<reactphysics3d::decimal>::push(L, quat.y);
        lua_setfield(L, -2, "y");
        Stack<reactphysics3d::decimal>::push(L, quat.z);
        lua_setfield(L, -2, "z");
        Stack<reactphysics3d::decimal>::push(L, quat.w);
        lua_setfield(L, -2, "w");

        // create meta bable
    }

    static reactphysics3d::Quaternion get(lua_State* L, int index)
    {
        if (!lua_istable(L, index)) {
            luaL_error(L, "#%d argments must be table", index);
        }
        reactphysics3d::Quaternion quat;
        lua_pushvalue(L, index);
        lua_getfield(L, -1, "x");
        quat.x = Stack<reactphysics3d::decimal>::get(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "y");
        quat.y = Stack<reactphysics3d::decimal>::get(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "z");
        quat.z = Stack<reactphysics3d::decimal>::get(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "w");
        quat.w = Stack<reactphysics3d::decimal>::get(L, -1);
        lua_pop(L, 2);
        return quat;
    }
};

template <>
struct Stack<reactphysics3d::Quaternion const&> : Stack<reactphysics3d::Quaternion> {
};

template <>
struct Stack<reactphysics3d::Matrix3x3> {
    static void push(lua_State* L, reactphysics3d::Matrix3x3 const& mat)
    {
        lua_createtable(L, 3, 0);
        Stack<reactphysics3d::Vector3>::push(L, mat[0]);
        lua_rawseti(L, -2, 1);
        Stack<reactphysics3d::Vector3>::push(L, mat[1]);
        lua_rawseti(L, -2, 2);
        Stack<reactphysics3d::Vector3>::push(L, mat[2]);
        lua_rawseti(L, -2, 3);

        // create meta bable
    }

    static reactphysics3d::Matrix3x3 get(lua_State* L, int index)
    {
        if (!lua_istable(L, index)) {
            luaL_error(L, "#%d argments must be table", index);
        }
        reactphysics3d::Matrix3x3 mat;
        lua_pushvalue(L, index);
        lua_rawgeti(L, -1, 1);
        mat[0] = Stack<reactphysics3d::Vector3>::get(L, -1);
        lua_pop(L, 1);
        lua_rawgeti(L, -1, 2);
        mat[1] = Stack<reactphysics3d::Vector3>::get(L, -1);
        lua_pop(L, 1);
        lua_rawgeti(L, -1, 3);
        mat[2] = Stack<reactphysics3d::Vector3>::get(L, -1);
        lua_pop(L, 2);
        return mat;
    }
};

template <>
struct Stack<reactphysics3d::Matrix3x3 const&> : Stack<reactphysics3d::Matrix3x3> {
};

template <>
struct Stack<reactphysics3d::Transform> {
    static void push(lua_State* L, reactphysics3d::Transform const& trans)
    {
        lua_createtable(L, 0, 2);
        Stack<reactphysics3d::Vector3>::push(L, trans.getPosition());
        lua_setfield(L, -2, "position");
        Stack<reactphysics3d::Quaternion>::push(L, trans.getOrientation());
        lua_setfield(L, -2, "rotation");

        // create meta bable
    }

    static reactphysics3d::Transform get(lua_State* L, int index)
    {
        if (!lua_istable(L, index)) {
            luaL_error(L, "#%d argments must be table", index);
        }
        reactphysics3d::Transform trans;
        lua_pushvalue(L, index);
        lua_getfield(L, -1, "position");
        reactphysics3d::Vector3 position = Stack<reactphysics3d::Vector3>::get(L, -1);
        trans.setPosition(position);
        lua_pop(L, 1);
        lua_getfield(L, -1, "rotation");
        reactphysics3d::Quaternion quat = Stack<reactphysics3d::Quaternion>::get(L, -1);
        trans.setOrientation(quat);
        lua_pop(L, 2);
        return trans;
    }
};

template <>
struct Stack<reactphysics3d::Transform const&> : Stack<reactphysics3d::Transform> {
};

// body
template <>
struct Stack<reactphysics3d::BodyType> {
    static void push(lua_State* L, reactphysics3d::BodyType const& r)
    {
        lua_pushinteger(L, (lua_Integer)r);
    }

    static reactphysics3d::BodyType get(lua_State* L, int index)
    {
        lua_Integer i = luaL_checkinteger(L, index);
        return (reactphysics3d::BodyType)i;
    }
};

template <>
struct Stack<reactphysics3d::BodyType const&> : Stack<reactphysics3d::BodyType> {
};

// shape
template <>
struct Stack<reactphysics3d::AABB> {
    static void push(lua_State* L, reactphysics3d::AABB const& aabb)
    {
        lua_createtable(L, 0, 3);
        luaL_error(L, "aabb not imp");
        /*Stack<b3R32>::push(L, step.dt);
			lua_setfield(L, -2, "dt");
			lua_pushinteger(L, step.velocityIterations);
			lua_setfield(L, -2, "velocityIterations");
			lua_pushboolean(L, step.sleeping);
			lua_setfield(L, -2, "velocityIterations");*/
    }

    static reactphysics3d::AABB get(lua_State* L, int index)
    {
        if (!lua_istable(L, index)) {
            luaL_error(L, "#%d argments must be table", index);
        }

        reactphysics3d::AABB aabb;
        /*lua_pushvalue(L, index);
			lua_getfield(L, -1, "dt");
			step.dt = Stack<b3R32>::get(L, -1);
			lua_pop(L, 1);
			lua_getfield(L, -1, "velocityIterations");
			step.velocityIterations = static_cast<u32>(lua_tointeger(L, -1));
			lua_pop(L, 1);
			lua_getfield(L, -1, "sleeping");
			step.sleeping = lua_toboolean(L, -1);
			lua_pop(L, 2);*/
        return aabb;
    }
};

template <>
struct Stack<reactphysics3d::AABB const&> : Stack<reactphysics3d::AABB> {
};

// shape
//template <>
//struct Stack<reactphysics3d::AABB> {
//    static void push(lua_State* L, reactphysics3d::AABB const& aabb)
//    {
//        lua_createtable(L, 0, 3);
//        luaL_error(L, "aabb not imp");
//        /*Stack<b3R32>::push(L, step.dt);
//			lua_setfield(L, -2, "dt");
//			lua_pushinteger(L, step.velocityIterations);
//			lua_setfield(L, -2, "velocityIterations");
//			lua_pushboolean(L, step.sleeping);
//			lua_setfield(L, -2, "velocityIterations");*/
//    }
//
//    static reactphysics3d::AABB get(lua_State* L, int index)
//    {
//        if (!lua_istable(L, index)) {
//            luaL_error(L, "#%d argments must be table", index);
//        }
//
//        reactphysics3d::AABB aabb;
//        /*lua_pushvalue(L, index);
//			lua_getfield(L, -1, "dt");
//			step.dt = Stack<b3R32>::get(L, -1);
//			lua_pop(L, 1);
//			lua_getfield(L, -1, "velocityIterations");
//			step.velocityIterations = static_cast<u32>(lua_tointeger(L, -1));
//			lua_pop(L, 1);
//			lua_getfield(L, -1, "sleeping");
//			step.sleeping = lua_toboolean(L, -1);
//			lua_pop(L, 2);*/
//        return aabb;
//    }
//};
//
//template <>
//struct Stack<reactphysics3d::AABB const&> : Stack<reactphysics3d::AABB> {
//};

//template <>
//struct Stack<reactphysics3d::WorldSettings> {
//    static void push(lua_State* L, reactphysics3d::WorldSettings const& settings)
//    {
//        lua_createtable(L, 0, 8);
//        Stack<b3R32>::push(L, settings.persistentContactDistanceThreshold);
//        lua_setfield(L, -2, "persistentContactDistanceThreshold");
//        /*lua_pushlightuserdata(L, def.userData);
//			lua_setfield(L, -2, "userData");
//			lua_pushboolean(L, def.sensor);
//			lua_setfield(L, -2, "sensor");
//			Stack<b3R32>::push(L, def.density);
//			lua_setfield(L, -2, "density");
//			Stack<b3R32>::push(L, def.friction);
//			lua_setfield(L, -2, "friction");
//			Stack<b3R32>::push(L, def.restitution);
//			lua_setfield(L, -2, "restitution");
//			Stack<b3Transform>::push(L, def.local);
//			lua_setfield(L, -2, "local");*/
//    }
//
//    static reactphysics3d::WorldSettings get(lua_State* L, int index)
//    {
//        if (!lua_istable(L, index)) {
//            luaL_error(L, "#%d argments must be table", index);
//        }
//
//        reactphysics3d::WorldSettings settings;
//        lua_pushvalue(L, index);
//        /*lua_getfield(L, -1, "shape");
//			int idx = lua_absindex(L, -1);
//			const b3Shape *shape = Stack<b3Polyhedron *>::get(L, idx);
//			if (shape == NULL) {
//				luaL_error(L, "shape must be");
//			}
//			def.shape = shape;
//			lua_pop(L, 1);
//			lua_getfield(L, -1, "sensor");
//			if (!lua_isnil(L, -1)) {
//				def.sensor = lua_toboolean(L, -1);
//			}
//			lua_pop(L, 1);
//			lua_getfield(L, -1, "density");
//			if (!lua_isnil(L, -1)) {
//				def.density = Stack<r32>::get(L, -1);
//			}
//			lua_pop(L, 1);
//			lua_getfield(L, -1, "friction");
//			if (!lua_isnil(L, -1)) {
//				def.friction = Stack<r32>::get(L, -1);
//			}
//			lua_pop(L, 1);
//			lua_getfield(L, -1, "restitution");
//			if (!lua_isnil(L, -1)) {
//				def.restitution = Stack<r32>::get(L, -1);
//			}
//			lua_pop(L, 1);
//			lua_getfield(L, -1, "local");
//			if (!lua_isnil(L, -1)) {
//				def.local = Stack<b3Transform>::get(L, -1);
//			}
//			lua_pop(L, 2);*/
//        return settings;
//    }
//};
//
//template <>
//struct Stack<reactphysics3d::WorldSettings const&> : Stack<reactphysics3d::WorldSettings> {
//};

} // namespace luabridge

LUAMOD_API
int luaopen_reactphysics3d(lua_State* L)
{
    luaL_checkversion(L);

    luabridge::getGlobalNamespace(L)
        .beginNamespace("rp3d")
        // body
        .beginClass<reactphysics3d::CollisionBody>("CollisionBody")
        .addFunction("isActive", &reactphysics3d::CollisionBody::isActive)
        .addFunction("getTransform", &reactphysics3d::CollisionBody::getTransform)
        .addFunction("setTransform", &reactphysics3d::CollisionBody::setTransform)
        .addFunction("addCollider", &reactphysics3d::CollisionBody::addCollider)
        .addFunction("removeCollider", &reactphysics3d::CollisionBody::removeCollider)
        //.addFunction("getCollider", &reactphysics3d::CollisionBody::getCollider)
        .endClass()
        .deriveClass<reactphysics3d::RigidBody, reactphysics3d::CollisionBody>("RigidBody")
        .addFunction("getTransform", &reactphysics3d::RigidBody::getTransform)
        .addFunction("setTransform", &reactphysics3d::RigidBody::setTransform)
        .addFunction("getMass", &reactphysics3d::RigidBody::getMass)
        .addFunction("setMass", &reactphysics3d::RigidBody::setMass)
        .addFunction("getLinearVelocity", &reactphysics3d::RigidBody::getLinearVelocity)
        .addFunction("setLinearVelocity", &reactphysics3d::RigidBody::setLinearVelocity)
        .addFunction("getAngularVelocity", &reactphysics3d::RigidBody::getAngularVelocity)
        .addFunction("setAngularVelocity", &reactphysics3d::RigidBody::setAngularVelocity)
        .addFunction("getLocalInertiaTensor", &reactphysics3d::RigidBody::getLocalInertiaTensor)
        .addFunction("setLocalInertiaTensor", &reactphysics3d::RigidBody::setLocalInertiaTensor)
        .addFunction("getType", &reactphysics3d::RigidBody::getType)
        .addFunction("setType", &reactphysics3d::RigidBody::setType)
        .addFunction("isGravityEnabled", &reactphysics3d::RigidBody::isGravityEnabled)
        .addFunction("enableGravity", &reactphysics3d::RigidBody::enableGravity)
        .addFunction("getLinearDamping", &reactphysics3d::RigidBody::getLinearDamping)
        .addFunction("setLinearDamping", &reactphysics3d::RigidBody::setLinearDamping)
        .addFunction("getAngularDamping", &reactphysics3d::RigidBody::getAngularDamping)
        .addFunction("setAngularDamping", &reactphysics3d::RigidBody::setAngularDamping)
        .addFunction("addCollider", &reactphysics3d::RigidBody::addCollider)
        .addFunction("removeCollider", &reactphysics3d::RigidBody::removeCollider)
        .endClass()
        .beginClass<reactphysics3d::Collider>("Collider")
        /*.addFunction("getLocalToBodyTransform", &reactphysics3d::Collider::getLocalToBodyTransform)
        .addFunction("setLocalToBodyTransform", &reactphysics3d::Collider::setLocalToBodyTransform)
        .addFunction("getLocalToWorldTransform", &reactphysics3d::Collider::getLocalToWorldTransform)*/
        .endClass()
        // shape
        .beginClass<reactphysics3d::CollisionShape>("CollisionShape")
        .endClass()
        .deriveClass<reactphysics3d::ConvexShape, reactphysics3d::CollisionShape>("ConvexShape")
        .endClass()
        .deriveClass<reactphysics3d::CapsuleShape, reactphysics3d::ConvexShape>("CapsuleShape")
        .endClass()
        .deriveClass<reactphysics3d::SphereShape, reactphysics3d::ConvexShape>("SphereShape")
        .endClass()
        .deriveClass<reactphysics3d::ConvexPolyhedronShape, reactphysics3d::ConvexShape>("ConvexPolyhedronShape")
        .endClass()
        .deriveClass<reactphysics3d::BoxShape, reactphysics3d::ConvexPolyhedronShape>("BoxShape")
        .endClass()
        .deriveClass<reactphysics3d::TriangleShape, reactphysics3d::ConvexPolyhedronShape>("TriangleShape")
        .endClass()
        .deriveClass<reactphysics3d::ConvexMeshShape, reactphysics3d::ConvexPolyhedronShape>("ConvexMeshShape")
        .endClass()
        .deriveClass<reactphysics3d::ConcaveShape, reactphysics3d::CollisionShape>("ConcaveShape")
        .endClass()
        /*.deriveClass<reactphysics3d::ConcaveMeshShape, reactphysics3d::ConcaveShape>("ConcaveMeshShape")
		.endClass()*/
        /*.deriveClass<reactphysics3d::HeightFieldShape, reactphysics3d::ConcaveShape>("HeightFieldShape")
		.endClass()*/

        // engine
        .beginClass<reactphysics3d::PhysicsWorld>("PhysicsWorld")
        .addFunction("createCollisionBody", &reactphysics3d::PhysicsWorld::createCollisionBody)
        .addFunction("destroyCollisionBody", &reactphysics3d::PhysicsWorld::destroyCollisionBody)
        .addFunction("getCollisionDispatch", &reactphysics3d::PhysicsWorld::getCollisionDispatch)
        .addFunction("raycast", &reactphysics3d::PhysicsWorld::raycast)
        .addFunction("getName", &reactphysics3d::PhysicsWorld::getName)
        .addFunction("update", &reactphysics3d::PhysicsWorld::update)
        .addFunction("getNbIterationsVelocitySolver", &reactphysics3d::PhysicsWorld::getNbIterationsVelocitySolver)
        .addFunction("setNbIterationsVelocitySolver", &reactphysics3d::PhysicsWorld::setNbIterationsVelocitySolver)
        .addFunction("getNbIterationsPositionSolver", &reactphysics3d::PhysicsWorld::getNbIterationsPositionSolver)
        .addFunction("setNbIterationsPositionSolver", &reactphysics3d::PhysicsWorld::setNbIterationsPositionSolver)
        .addFunction("setContactsPositionCorrectionTechnique", &reactphysics3d::PhysicsWorld::setContactsPositionCorrectionTechnique)
        .addFunction("setJointsPositionCorrectionTechnique", &reactphysics3d::PhysicsWorld::setJointsPositionCorrectionTechnique)
        .addFunction("createRigidBody", &reactphysics3d::PhysicsWorld::createRigidBody)
        .addFunction("destroyRigidBody", &reactphysics3d::PhysicsWorld::destroyRigidBody)
        .addFunction("createJoint", &reactphysics3d::PhysicsWorld::createJoint)
        .addFunction("destroyJoint", &reactphysics3d::PhysicsWorld::destroyJoint)
        .addFunction("getGravity", &reactphysics3d::PhysicsWorld::getGravity)
        .addFunction("setGravity", &reactphysics3d::PhysicsWorld::setGravity)
        .addFunction("isGravityEnabled", &reactphysics3d::PhysicsWorld::isGravityEnabled)
        .addFunction("isSleepingEnabled", &reactphysics3d::PhysicsWorld::isSleepingEnabled)
        .addFunction("enableSleeping", &reactphysics3d::PhysicsWorld::enableSleeping)
        .addFunction("setEventListener", &reactphysics3d::PhysicsWorld::setEventListener)
        .endClass()
        // math
        .beginClass<reactphysics3d::Quaternion>("Quaternion")
        .addStaticFunction("identity", &reactphysics3d::Quaternion::identity)
        .endClass()
        .beginClass<reactphysics3d::Matrix3x3>("Matrix3x3")
        .addStaticFunction("identity", &reactphysics3d::Matrix3x3::identity)
        .addStaticFunction("zero", &reactphysics3d::Matrix3x3::zero)
        .endClass()
        .beginClass<reactphysics3d::Transform>("Transform")
        .addStaticFunction("identity", &reactphysics3d::Transform::identity)
        .endClass()
        // r32
        .beginClass<b3R32>("b3R32")
        .addStaticFunction("fromInt", &b3R32::fromInt)
        .addStaticFunction("fromFlt32", &b3R32::fromFlt32)
        .addStaticFunction("fromFlt64", &b3R32::fromFlt64)
        .addStaticFunction("MAXIMUM", &b3R32::maximum)
        .addStaticFunction("MINIMUM", &b3R32::minimum)
        .addStaticFunction("PI", &b3R32::pi)
        .addStaticFunction("E", &b3R32::e)
        .addStaticFunction("ONE", &b3R32::one)
        .endClass()
        // l
        /*.beginClass<reactphysics3d::CollisionCallback>("CollisionCallback")
        .endClass()*/
        .beginClass<reactphysics3d::EventListener>("EventListener")
        .endClass()
        .deriveClass<lEventListener, reactphysics3d::EventListener>("lEventListener")
        .addConstructor<void (*)()>()
        .addCFunction("lregister", &lEventListener::lregister)
        .endClass()
        .beginClass<lCommon>("lCommon")
        .addStaticFunction("createPhysicsWorld", &lCommon::createPhysicsWorld)
        .addStaticFunction("destroyPhysicsWorld", &lCommon::destroyPhysicsWorld)
        .addStaticFunction("createSphereShape", &lCommon::createSphereShape)
        .addStaticFunction("destroySphereShape", &lCommon::destroySphereShape)
        .addStaticFunction("createBoxShape", &lCommon::createBoxShape)
        .addStaticFunction("destroyBoxShape", &lCommon::destroyBoxShape)
        .addStaticFunction("createCapsuleShape", &lCommon::createCapsuleShape)
        .addStaticFunction("destroyCapsuleShape", &lCommon::destroyCapsuleShape)
        /*.addStaticFunction("createConvexMeshShape", &lCommon::createConvexMeshShape)
        .addStaticFunction("destroyConvexMeshShape", &lCommon::destroyConvexMeshShape)*/
        /*.addStaticFunction("createHeightFieldShape", &lCommon::createHeightFieldShape)
        .addStaticFunction("destroyHeightFieldShape", &lCommon::destroyHeightFieldShape)*/
        /*.addStaticFunction("createConcaveMeshShape", &lCommon::createConcaveMeshShape)
        .addStaticFunction("destroyConcaveMeshShape", &lCommon::destroyConcaveMeshShape)*/
        .endClass()
        .endNamespace();
    return 0;
}
