#pragma once

#include "Engine.h"

#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#if defined(L_PLATFORM_WINDOWS)

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <shlobj.h>
#include <atlstr.h>
#include <commdlg.h>

#elif defined(L_PLATFORM_LINUX)

#include "tinyfiledialogs.h"

#endif

namespace Louron {

	namespace Utils 
    {
		static std::string OpenFile(const char* filter, const std::filesystem::path& initialDir = "")
		{

        #if defined(L_PLATFORM_WINDOWS)

            OPENFILENAMEA ofn;
            CHAR szFile[260] = { 0 };
            ZeroMemory(&ofn, sizeof(OPENFILENAME));
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Engine::Get().GetWindow().GetNativeWindow());
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);

            std::string initialDirStr;

            // If initialDir is not passed (empty string), use current directory
            if (initialDir.empty())
            {
                CHAR currentDir[256] = { 0 };
                if (GetCurrentDirectoryA(256, currentDir))
                    ofn.lpstrInitialDir = currentDir;
            }
            else
            {
                // If initialDir is passed, validate and use it if it's a valid directory
                if (std::filesystem::exists(initialDir.lexically_normal()) && std::filesystem::is_directory(initialDir.lexically_normal()))
                {
                    initialDirStr = initialDir.lexically_normal().string(); // Store path in a string
                    ofn.lpstrInitialDir = initialDirStr.c_str(); // Use the valid string
                }
                else
                {
                    // Fallback to current directory if the provided path is invalid
                    CHAR currentDir[256] = { 0 };
                    if (GetCurrentDirectoryA(256, currentDir))
                        ofn.lpstrInitialDir = currentDir;
                }
            }

            ofn.lpstrFilter = filter;
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

            if (GetOpenFileNameA(&ofn) == TRUE)
                return ofn.lpstrFile;

            return std::string();

        #elif defined (L_PLATFORM_LINUX)

            const char* path = tinyfd_openFileDialog("Open File", initialDir.empty() ? nullptr : initialDir.string().c_str(), 0, nullptr, nullptr, 0);
            return path ? std::string(path) : std::string();

        #else

            return "";
            
        #endif

		}

		static std::string OpenDirectory()
        {

        #if defined(L_PLATFORM_WINDOWS)

            // Initialize COM
            CoInitialize(NULL);

            // Create the IFileOpenDialog object
            IFileOpenDialog* pFileOpen = nullptr;
            HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pFileOpen));

            if (FAILED(hr))
            {
                CoUninitialize();
                return std::string(); // Return empty string on failure
            }

            // Set the dialog to pick folders
            DWORD dwFlags;
            pFileOpen->GetOptions(&dwFlags);
            pFileOpen->SetOptions(dwFlags | FOS_PICKFOLDERS); // Add the folder picking option

            // Show the dialog
            hr = pFileOpen->Show(NULL);
            if (FAILED(hr))
            {
                pFileOpen->Release();
                CoUninitialize();
                return std::string(); // Return empty string if dialog fails
            }

            // Retrieve the selected folder
            IShellItem* pItem = nullptr;
            hr = pFileOpen->GetResult(&pItem);
            if (FAILED(hr))
            {
                pFileOpen->Release();
                CoUninitialize();
                return std::string(); // Return empty string if no result
            }

            // Get the folder path
            PWSTR pszFolderPath = nullptr;
            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFolderPath);
            if (FAILED(hr))
            {
                pItem->Release();
                pFileOpen->Release();
                CoUninitialize();
                return std::string(); // Return empty string if path retrieval fails
            }

            // Convert the WCHAR path to std::string using WideCharToMultiByte
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, pszFolderPath, -1, NULL, 0, NULL, NULL);
            std::string strFolderPath(size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, pszFolderPath, -1, &strFolderPath[0], size_needed, NULL, NULL);


            // Clean up
            CoTaskMemFree(pszFolderPath);
            pItem->Release();
            pFileOpen->Release();
            CoUninitialize();

            return strFolderPath; // Return the selected folder path

        #elif defined (L_PLATFORM_LINUX)

            const char* path = tinyfd_selectFolderDialog("Select Folder", nullptr);
            return path ? std::string(path) : std::string();

        #else

            return "";

        #endif

        }

        static std::string SaveFile(const char* filter, const std::filesystem::path& initialDir = "")
        {

        #if defined(L_PLATFORM_WINDOWS)

            OPENFILENAMEA ofn;
            CHAR szFile[260] = { 0 };
            ZeroMemory(&ofn, sizeof(OPENFILENAME));
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Engine::Get().GetWindow().GetNativeWindow());
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);

            std::string initialDirStr;

            // If initialDir is not passed (empty string), use current directory
            if (initialDir.empty())
            {
                CHAR currentDir[256] = { 0 };
                if (GetCurrentDirectoryA(256, currentDir))
                    ofn.lpstrInitialDir = currentDir;
            }
            else
            {
                // If initialDir is passed, validate and use it if it's a valid directory
                if (std::filesystem::exists(initialDir.lexically_normal()) && std::filesystem::is_directory(initialDir.lexically_normal()))
                {
                    initialDirStr = initialDir.lexically_normal().string(); // Store path in a string
                    ofn.lpstrInitialDir = initialDirStr.c_str(); // Use the valid string
                }
                else
                {
                    // Fallback to current directory if the provided path is invalid
                    CHAR currentDir[256] = { 0 };
                    if (GetCurrentDirectoryA(256, currentDir))
                        ofn.lpstrInitialDir = currentDir;
                }
            }

            ofn.lpstrFilter = filter;
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

            // Sets the default extension by extracting it from the filter
            ofn.lpstrDefExt = strchr(filter, '\0') + 1;

            if (GetSaveFileNameA(&ofn) == TRUE)
                return ofn.lpstrFile;

            return std::string();

        #elif defined(L_PLATFORM_LINUX)

            const char* path = tinyfd_saveFileDialog("Save File", initialDir.empty() ? nullptr : initialDir.string().c_str(), 0, nullptr, nullptr);
            return path ? std::string(path) : std::string();

        #else

            return "";

        #endif

		}

        static bool IsPathHidden(const std::filesystem::path& p)
        {

        #if defined(L_PLATFORM_WINDOWS)

            // Use the wide-character version
            DWORD attributes = GetFileAttributesW(p.c_str());
            if (attributes != INVALID_FILE_ATTRIBUTES) {
                return (attributes & FILE_ATTRIBUTE_HIDDEN) != 0;
            }
            return false; // Invalid path or error

        #elif defined(L_PLATFORM_MAC) || defined(L_PLATFORM_LINUX)

            // Unix-like system check - file name starts with a dot
            return !p.filename().empty() && p.filename().string().front() == '.';

        #else

            return false;

        #endif

        }

	}

}