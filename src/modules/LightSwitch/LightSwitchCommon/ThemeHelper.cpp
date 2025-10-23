#include <windows.h>
#include <logger/logger_settings.h>
#include <logger/logger.h>
#include <utils/logger_helper.h>
#include "ThemeHelper.h"
#include <array>
#include <string>
#include <wil/resource.h>

// Controls changing the themes.

static void ResetColorPrevalence()
{
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER,
                     L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                     0,
                     KEY_SET_VALUE,
                     &hKey) == ERROR_SUCCESS)
    {
        DWORD value = 0; // back to default value
        RegSetValueEx(hKey, L"ColorPrevalence", 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(value));
        RegCloseKey(hKey);

        SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, reinterpret_cast<LPARAM>(L"ImmersiveColorSet"), SMTO_ABORTIFHUNG, 5000, nullptr);

        SendMessageTimeout(HWND_BROADCAST, WM_THEMECHANGED, 0, 0, SMTO_ABORTIFHUNG, 5000, nullptr);

        SendMessageTimeout(HWND_BROADCAST, WM_DWMCOLORIZATIONCOLORCHANGED, 0, 0, SMTO_ABORTIFHUNG, 5000, nullptr);
    }
}

void SetAppsTheme(bool mode)
{
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER,
                     L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                     0,
                     KEY_SET_VALUE,
                     &hKey) == ERROR_SUCCESS)
    {
        DWORD value = mode;
        RegSetValueEx(hKey, L"AppsUseLightTheme", 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(value));
        RegCloseKey(hKey);

        SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, reinterpret_cast<LPARAM>(L"ImmersiveColorSet"), SMTO_ABORTIFHUNG, 5000, nullptr);

        SendMessageTimeout(HWND_BROADCAST, WM_THEMECHANGED, 0, 0, SMTO_ABORTIFHUNG, 5000, nullptr);
    }
}

void SetSystemTheme(bool mode)
{
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER,
                     L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                     0,
                     KEY_SET_VALUE,
                     &hKey) == ERROR_SUCCESS)
    {
        DWORD value = mode;
        RegSetValueEx(hKey, L"SystemUsesLightTheme", 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(value));
        RegCloseKey(hKey);

        if (mode) // if are changing to light mode
        {
            ResetColorPrevalence();
            Logger::info(L"[LightSwitchService] Reset ColorPrevalence to default when switching to light mode.");
        }

        SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, reinterpret_cast<LPARAM>(L"ImmersiveColorSet"), SMTO_ABORTIFHUNG, 5000, nullptr);

        SendMessageTimeout(HWND_BROADCAST, WM_THEMECHANGED, 0, 0, SMTO_ABORTIFHUNG, 5000, nullptr);
    }
}

// Can think of this as "is the current theme light?"
bool GetCurrentSystemTheme()
{
    HKEY hKey;
    DWORD value = 1; // default = light
    DWORD size = sizeof(value);

    if (RegOpenKeyEx(HKEY_CURRENT_USER,
                     L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                     0,
                     KEY_READ,
                     &hKey) == ERROR_SUCCESS)
    {
        RegQueryValueEx(hKey, L"SystemUsesLightTheme", nullptr, nullptr, reinterpret_cast<LPBYTE>(&value), &size);
        RegCloseKey(hKey);
    }

    return value == 1; // true = light, false = dark
}

bool GetCurrentAppsTheme()
{
    HKEY hKey;
    DWORD value = 1;
    DWORD size = sizeof(value);

    if (RegOpenKeyEx(HKEY_CURRENT_USER,
                     L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                     0,
                     KEY_READ,
                     &hKey) == ERROR_SUCCESS)
    {
        RegQueryValueEx(hKey, L"AppsUseLightTheme", nullptr, nullptr, reinterpret_cast<LPBYTE>(&value), &size);
        RegCloseKey(hKey);
    }

    return value == 1; // true = light, false = dark
}

std::array<std::wstring, 2u> getStyleValue(int style) noexcept
{
    switch (style)
    {
    case 0: // Fill
        return { L"10", L"0" };
    case 1: // Fit
        return { L"6", L"0" };
    case 2: // Stretch
        return { L"2", L"0" };
    case 3: // Tile
        return { L"0", L"1" };
    case 4: // Center
        return { L"0", L"0" };
    case 5: // Span
        return { L"22", L"0" };
    default:
        std::terminate();
    }
}

int SetWallpaperViaRegistry(std::wstring const& wallpaperPath, int style) noexcept
{
    HKEY hKeyDesktop{};
    HKEY hKeyWallpapers{};

    auto closeKey = wil::scope_exit([&hKeyDesktop, &hKeyWallpapers]() {
        if (RegCloseKey(hKeyDesktop) != ERROR_SUCCESS)
        {
            std::terminate();
        }
        if (RegCloseKey(hKeyWallpapers) != ERROR_SUCCESS)
        {
            std::terminate();
        }
    });

    auto [styleValue, tileValue] = getStyleValue(style);

    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Control Panel\\Desktop", 0, KEY_WRITE, &hKeyDesktop) != ERROR_SUCCESS)
    {
        return 1;
    }

    if (RegSetValueExW(hKeyDesktop, L"Wallpaper", 0, REG_SZ, reinterpret_cast<const BYTE*>(wallpaperPath.data()), static_cast<DWORD>((wallpaperPath.size() + 1u) * sizeof(wchar_t))) != ERROR_SUCCESS)
    {
        return 2;
    }

    if (RegSetValueExW(hKeyDesktop, L"WallpaperStyle", 0, REG_SZ, reinterpret_cast<const BYTE*>(styleValue.data()), static_cast<DWORD>(styleValue.size() + 1u)) != ERROR_SUCCESS)
    {
        return 3;
    }

    if (RegSetValueExW(hKeyDesktop, L"TileWallpaper", 0, REG_SZ, reinterpret_cast<const BYTE*>(tileValue.data()), static_cast<DWORD>(tileValue.size() + 1u)) != ERROR_SUCCESS)
    {
        return 4;
    }

    // Another wallpaper path cache
    // If it is not executed, the wallpaper will revert after switching to another virtual desktop
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Wallpapers", 0, KEY_WRITE, &hKeyWallpapers) != ERROR_SUCCESS)
    {
        return 5;
    }

    if (RegSetValueExW(hKeyWallpapers, L"CurrentWallpaperPath", 0, REG_SZ, reinterpret_cast<const BYTE*>(wallpaperPath.data()), static_cast<DWORD>((wallpaperPath.size() + 1u) * sizeof(wchar_t))) != ERROR_SUCCESS)
    {
        return 6;
    }

    DWORD backgroundType = 0; // 0 = picture, 1 = solid color, 2 = slideshow
    if (RegSetValueExW(hKeyWallpapers, L"BackgroundType", 0, REG_DWORD, reinterpret_cast<const BYTE*>(&backgroundType), static_cast<DWORD>(sizeof(DWORD))) != ERROR_SUCCESS)
    {
        return 7;
    }

    // notify the system about the change
    if (SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, nullptr, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE) == 0)
    {
        return 8;
    }

    return 0;
}
