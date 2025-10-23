#pragma once
#include <array>
#include <string>
void SetSystemTheme(bool dark);
void SetAppsTheme(bool dark);
bool GetCurrentSystemTheme();
bool GetCurrentAppsTheme();
// Returned 0 indicates success; otherwise, the reason is returned, see definatiion
int SetWallpaperViaRegistry(std::wstring const& wallpaperPath, int style) noexcept;