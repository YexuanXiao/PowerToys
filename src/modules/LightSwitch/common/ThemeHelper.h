#pragma once
#include <array>
#include <string>
void SetSystemTheme(bool dark);
void SetAppsTheme(bool dark);
bool GetCurrentSystemTheme();
bool GetCurrentAppsTheme();
bool SetWallpaperViaRegistry(std::wstring const& wallpaperPath, int style) noexcept;