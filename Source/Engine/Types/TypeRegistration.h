#pragma once

template <typename T>
struct TypeRegistrationHelper
{
public:
    TypeRegistrationHelper(const char* typePath, const char* typeName);

    ~TypeRegistrationHelper();

    TypeMetaData* m_Data;
    const char* m_Path;
};

#include "TypeRegistration.inl"
