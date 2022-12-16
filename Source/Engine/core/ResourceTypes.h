#pragma once

struct FromFileResourceParameters
{
    std::string path;

    std::string const& to_string() const { return path; }
};

namespace std
{

template <>
struct hash<FromFileResourceParameters>
{
    std::size_t operator()(FromFileResourceParameters const& obj)
    {
        return std::hash<std::string>{}(obj.path);
    }
};

} // namespace std

enum class ResourceStatus
{
    Error,
    Loading,
    Loaded
};

struct NoInit
{
};
