#pragma once

#include "gl_basic_object.h"

namespace Core{
	BasicObject::BasicObject() :
		_mTexture(nullptr),
		_mVA(nullptr),
		_mVB(nullptr),
		_mIB(nullptr),
		_mVBL(nullptr) {}

	BasicObject::BasicObject(const BasicObject& other) {
		_mID        = other._mID;
		_mWorldData = other._mWorldData;

		_mTexture = other._mTexture;
		_mVA      = other._mVA;
		_mVB      = other._mVB;
		_mIB      = other._mIB;
		_mVBL     = other._mVBL;
	}

	BasicObject& BasicObject::operator=(const BasicObject& other) {
		if (this == &other) {
			return *this; // Handle self-assignment
		}
		
		_mID        = other._mID;
		_mWorldData = other._mWorldData;

		_mTexture = other._mTexture;
		_mVA      = other._mVA;
		_mVB      = other._mVB;
		_mIB      = other._mIB;
		_mVBL     = other._mVBL;
		return *this;
	}
};