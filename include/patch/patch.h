#ifndef PATCH_H
#define PATCH_H

#include <cstdint>
#include <string>

bool hook(const std::string &source, void *to);
bool unhook(const std::string &source);
#endif