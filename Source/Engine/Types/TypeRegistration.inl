
template <typename T>
TypeRegistrationHelper<T>::TypeRegistrationHelper(const char* typePath, const char* typeName)
  : m_Path(typePath)
{
    m_Data = TypeManager::instance()->AddType(m_Path);

    m_Data->m_Name = typeName;
    m_Data->m_Path = typePath;
    m_Data->m_ConstructFn = [](void* dest) -> void*
    {
        if (dest)
        {
            return new (dest) T();
        }
        else
        {
            return new T();
        }
    };

    m_Data->m_SerializeFn = [](IFileStream* iniStream, void* data)
    { T::Serialize(iniStream, (T*)data); };
}

template <typename T>
TypeRegistrationHelper<T>::~TypeRegistrationHelper()
{
    TypeManager::instance()->RemoveType(m_Path);
}
