// Ash Crypto
// Copyright (c) 2017-2020, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <string>
#include <vector>

namespace utils
{

class NotImplementedException : public std::logic_error
{

public:
    NotImplementedException(const std::string& funcname);
};

std::string getOsString();

void openBrowser(const std::string& url_str);

bool isNumeric(const std::string_view& s);
bool isBoolean(const std::string_view s);
bool convertToBool(const std::string_view s);

std::string getUserFolder();
std::string getDataFolder();
std::string getDefaultConfigFile();
std::string getDefaultDatabaseFolder();

} // namespace