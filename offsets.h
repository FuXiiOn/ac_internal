#pragma once
#include <iostream>
#include <windows.h>
#include "geom.h"
#define STR_MERGE_IMPL(a, b) a##b
#define STR_MERGE(a, b) STR_MERGE_IMPL(a, b)
#define MAKE_PAD(size) STR_MERGE(_pad, __COUNTER__)[size]
#define DEFINE_MEMBER_N(type, name, offset) struct {unsigned char MAKE_PAD(offset); type name;}

class ent
{
public:
	char pad_0000[4]; //0x0000
	Vector3 headpos; //0x0004
	char pad_0010[36]; //0x0010
	Vector3 bodypos; //0x0034
	float yaw; //0x0040
	float pitch; //0x0044
	char pad_0048[36]; //0x0048
	bool isShifting; //0x006C
	char pad_006D[139]; //0x006D
	int32_t health; //0x00F8
	char pad_00FC[296]; //0x00FC
	bool bAttack; //0x0224
	char name[16]; //0x0225
	int8_t N000001C5; //0x0235
	char pad_0236[246]; //0x0236
	int32_t team; //0x032C
	char pad_0330[68]; //0x0330
	class weaponClass* currWeapon; //0x0374
	char pad_0378[296]; //0x0378
}; //Size: 0x04A0
static_assert(sizeof(ent) == 0x4A0);

class weaponClass
{
public:
	char pad_0000[4]; //0x0000
	int32_t weaponId; //0x0004
	class ent* weaponOwner; //0x0008
	char pad_000C[4]; //0x000C
	class ammoPtr* ammoReserve; //0x0010
	class N000003B0* ammoPtr; //0x0014
	char pad_0018[44]; //0x0018
}; //Size: 0x0044
static_assert(sizeof(weaponClass) == 0x44);

class ammoInfo
{
public:
	int32_t ammoClip; //0x0000
	char pad_0004[64]; //0x0004
}; //Size: 0x0044
static_assert(sizeof(ammoInfo) == 0x44);

class N000003B0
{
public:
	int32_t ammoClip; //0x0000
	char pad_0004[64]; //0x0004
}; //Size: 0x0044
static_assert(sizeof(N000003B0) == 0x44);

class entList
{
public:
	union {
		DEFINE_MEMBER_N(ent*, ent1, 0x0004);
		DEFINE_MEMBER_N(ent*, ent2, 0x0008);
		DEFINE_MEMBER_N(ent*, ent3, 0x000C);
		DEFINE_MEMBER_N(ent*, ent4, 0x0010);
		DEFINE_MEMBER_N(ent*, ent5, 0x0014);
		DEFINE_MEMBER_N(ent*, ent6, 0x0018);
		DEFINE_MEMBER_N(ent*, ent7, 0x001C);
		DEFINE_MEMBER_N(ent*, ent8, 0x0020);
		DEFINE_MEMBER_N(ent*, ent9, 0x0024);
		DEFINE_MEMBER_N(ent*, ent10, 0x0028);
		DEFINE_MEMBER_N(ent*, ent11, 0x002C);
		DEFINE_MEMBER_N(ent*, ent12, 0x0030);
		DEFINE_MEMBER_N(ent*, ent13, 0x0034);
		DEFINE_MEMBER_N(ent*, ent14, 0x0038);
		DEFINE_MEMBER_N(ent*, ent15, 0x003C);
		DEFINE_MEMBER_N(ent*, ent16, 0x0040);
		DEFINE_MEMBER_N(ent*, ent17, 0x0044);
		DEFINE_MEMBER_N(ent*, ent18, 0x0048);
		DEFINE_MEMBER_N(ent*, ent19, 0x004C);
		DEFINE_MEMBER_N(ent*, ent20, 0x0050);
		DEFINE_MEMBER_N(ent*, ent21, 0x0054);
		DEFINE_MEMBER_N(ent*, ent22, 0x0058);
		DEFINE_MEMBER_N(ent*, ent23, 0x005C);
		DEFINE_MEMBER_N(ent*, ent24, 0x0060);
		DEFINE_MEMBER_N(ent*, ent25, 0x0064);
		DEFINE_MEMBER_N(ent*, ent26, 0x0068);
		DEFINE_MEMBER_N(ent*, ent27, 0x006C);
		DEFINE_MEMBER_N(ent*, ent28, 0x0070);
		DEFINE_MEMBER_N(ent*, ent29, 0x0074);
		DEFINE_MEMBER_N(ent*, ent30, 0x0078);
		DEFINE_MEMBER_N(ent*, ent31, 0x007C);
	};
};