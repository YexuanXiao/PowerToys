#pragma once
#include <string>
void SetSystemTheme(bool dark);
void SetAppsTheme(bool dark);
bool GetCurrentSystemTheme();
bool GetCurrentAppsTheme();
// Returned 0 indicates success; otherwise, the reason is returned, see definition
int SetWallpaper(std::wstring const& wallpaperPath, int style) noexcept;