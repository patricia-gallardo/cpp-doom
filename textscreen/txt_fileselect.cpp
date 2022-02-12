//
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
// Routines for selecting files.
//

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "doomkeys.hpp"

#include "txt_fileselect.hpp"
#include "txt_inputbox.hpp"
#include "txt_gui.hpp"
#include "txt_io.hpp"
#include "txt_main.hpp"
#include "txt_widget.hpp"
#include "memory.hpp"

struct txt_fileselect_s {
    txt_widget_t widget;
    txt_inputbox_t *inputbox;
    int size;
    const char *prompt;
    const char **extensions;
};

// Dummy value to select a directory.

const char *TXT_DIRECTORY[] = { "__directory__", nullptr };

#ifndef _WIN32

#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <sys/wait.h>

static char *ExecReadOutput(char **argv)
{
    int status;
    int pipefd[2];

    if (pipe(pipefd) != 0)
    {
        return nullptr;
    }

    int pid = fork();

    if (pid == 0)
    {
        dup2(pipefd[1], fileno(stdout));
        execv(argv[0], argv);
        exit(-1);
    }

    fcntl(pipefd[0], F_SETFL, O_NONBLOCK);

    // Read program output into 'result' string.
    // Wait until the program has completed and (if it was successful)
    // a full line has been read.

    char *result = nullptr;
    int result_len = 0;
    int completed = 0;

    while (!completed
        || (status == 0 && (result == nullptr || strchr(result, '\n') == nullptr)))
    {
        if (!completed && waitpid(pid, &status, WNOHANG) != 0)
        {
            completed = 1;
        }

        char buf[64];
        ssize_t bytes = read(pipefd[0], buf, sizeof(buf));

        if (bytes < 0)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                status = -1;
                break;
            }
        }
        else
        {
            char *new_result = static_cast<char *>(realloc(result, (static_cast<size_t>(result_len + bytes + 1))));
            if (new_result == nullptr)
            {
                break;
            }
            result = new_result;
            memcpy(result + result_len, buf, static_cast<size_t>(bytes));
            result_len += static_cast<int>(bytes);
            result[result_len] = '\0';
        }

        usleep(100 * 1000);
        TXT_Sleep(1);
        TXT_UpdateScreen();
    }

    close(pipefd[0]);
    close(pipefd[1]);

    // Must have a success exit code.

    if (WEXITSTATUS(status) != 0)
    {
        free(result);
        result = nullptr;
    }

    // Strip off newline from the end.

    if (result != nullptr && result[result_len - 1] == '\n')
    {
        result[result_len - 1] = '\0';
    }

    return result;
}

#endif

// This is currently disabled on Windows because it doesn't work.
// Current issues:
//   * On Windows Vista+ the mouse cursor freezes when the dialog is
//     opened. This is probably some conflict with SDL (might be
//     resolved by opening the dialog in a separate thread so that
//     TXT_UpdateScreen can be run in the background).
//   * On Windows XP the program exits/crashes when the dialog is
//     closed.
#if defined(_WIN32)

int TXT_CanSelectFiles()
{
    return 0;
}

char *TXT_SelectFile(const char *, const char **)
{
    return nullptr;
}

#elif defined(xxxdisabled_WIN32)

// Windows code. Use comdlg32 to pop up a dialog box.

#include <windows.h>
#include <shlobj.h>

static BOOL (*MyGetOpenFileName)(LPOPENFILENAME) = nullptr;
static LPITEMIDLIST (*MySHBrowseForFolder)(LPBROWSEINFO) = nullptr;
static BOOL (*MySHGetPathFromIDList)(LPITEMIDLIST, LPTSTR) = nullptr;

// Load library functions from DLL files.

static int LoadDLLs()
{
    HMODULE comdlg32, shell32

    comdlg32 = LoadLibraryW(L"comdlg32.dll");
    if (comdlg32 == nullptr)
    {
        return 0;
    }

    shell32 = LoadLibraryW(L"shell32.dll");
    if (shell32 == nullptr)
    {
        FreeLibrary(comdlg32);
        return 0;
    }

    MyGetOpenFileName =
        (void *) GetProcAddress(comdlg32, "GetOpenFileNameA");
    MySHBrowseForFolder =
        (void *) GetProcAddress(shell32, "SHBrowseForFolder");
    MySHGetPathFromIDList =
        (void *) GetProcAddress(shell32, "SHGetPathFromIDList");

    return MyGetOpenFileName != nullptr
        && MySHBrowseForFolder != nullptr
        && MySHGetPathFromIDList != nullptr;
}

static int InitLibraries()
{
    static int initted = 0, success = 0;

    if (!initted)
    {
        success = LoadDLLs();
        initted = 1;
    }

    return success;
}

// Generate the "filter" string from the list of extensions.

static char *GenerateFilterString(const char **extensions)
{
    unsigned int result_len = 1;
    unsigned int i;
    char *result, *out;
    size_t out_len, offset;

    if (extensions == nullptr)
    {
        return nullptr;
    }

    for (i = 0; extensions[i] != nullptr; ++i)
    {
        result_len += 16 + strlen(extensions[i]) * 3;
    }

    result = malloc(result_len);
    out = result; out_len = result_len;

    for (i = 0; extensions[i] != nullptr; ++i)
    {
        // .wad files (*.wad)\0
        offset = TXT_snprintf(out, out_len, "%s files (*.%s)",
                              extensions[i], extensions[i]);
        out += offset + 1; out_len -= offset + 1;

        // *.wad\0
        offset = TXT_snprintf(out, out_len, "*.%s", extensions[i]);
        out_len += offset + 1; out_len -= offset + 1;
    }

    *out = '\0';

    return result;
}

int TXT_CanSelectFiles()
{
    return InitLibraries();
}

static char *SelectDirectory(char *window_title)
{
    LPITEMIDLIST pidl;
    BROWSEINFO bi;
    char selected[MAX_PATH] = "";
    char *result;

    ZeroMemory(&bi, sizeof(bi));
    bi.hwndOwner = nullptr;
    bi.lpszTitle = window_title;
    bi.pszDisplayName = selected;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    pidl = MySHBrowseForFolder(&bi);

    result = nullptr;

    if (pidl != nullptr)
    {
        if (MySHGetPathFromIDList(pidl, selected))
        {
            result = strdup(selected);
        }

        // TODO: Free pidl
    }

    return result;
}

char *TXT_SelectFile(const char *window_title, const char **extensions)
{
    OPENFILENAME fm;
    char selected[MAX_PATH] = "";
    char *filter_string, *result;

    if (!InitLibraries())
    {
        return nullptr;
    }

    if (extensions == TXT_DIRECTORY)
    {
        return SelectDirectory(window_title);
    }

    filter_string = GenerateFilterString(extensions);

    ZeroMemory(&fm, sizeof(fm));
    fm.lStructSize = sizeof(OPENFILENAME);
    fm.hwndOwner = nullptr;
    fm.lpstrTitle = window_title;
    fm.lpstrFilter = filter_string;
    fm.lpstrFile = selected;
    fm.nMaxFile = MAX_PATH;
    fm.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    fm.lpstrDefExt = "";

    if (!MyGetOpenFileName(&fm))
    {
        result = nullptr;
    }
    else
    {
        result = strdup(selected);
    }

    free(filter_string);

    return result;
}

#elif defined(__MACOSX__)

// Mac OS X code. Popping up a dialog requires Objective C/Cocoa
// but we can get away with using AppleScript which avoids adding
// an Objective C dependency. This is rather silly.

// Printf format string for the "wrapper" portion of the AppleScript:
#define APPLESCRIPT_WRAPPER "copy POSIX path of (%s) to stdout"

static char *CreateEscapedString(const char *original)
{
    char *result;
    const char *in;
    char *out;

    // We need to take care not to overflow the buffer, so count exactly.
#define ESCAPED_CHARS "\"\\"
    size_t count_extras = 2;    // start counting the two quotes
    for (in = original; *in; ++in)
    {
        if (strchr(ESCAPED_CHARS, *in))
        {
            ++count_extras;
        }
    }

    result = static_cast<char *>(malloc(strlen(original) + count_extras + 1));
    if (!result)
    {
        return nullptr;
    }
    out = result;
    *out++ = '"';
    for (in = original; *in; ++in)
    {
        if (strchr(ESCAPED_CHARS, *in))
        {
            *out++ = '\\';
        }
        *out++ = *in;
    }
    *out++ = '"';
    *out = 0;

    return result;
#undef ESCAPED_CHARS
}

// Build list of extensions, like: {"wad","lmp","txt"}

static char *CreateExtensionsList(const char **extensions)
{
    char *result, *escaped;
    unsigned int result_len;
    unsigned int i;

    if (extensions == nullptr)
    {
        return nullptr;
    }

    result_len = 3;
    for (i = 0; extensions[i] != nullptr; ++i)
    {
        result_len += 5 + strlen(extensions[i]) * 2;
    }

    result = static_cast<char *>(malloc(result_len));
    if (!result)
    {
        return nullptr;
    }
    TXT_StringCopy(result, "{", result_len);

    for (i = 0; extensions[i] != nullptr; ++i)
    {
        escaped = CreateEscapedString(extensions[i]);
        if (!escaped)
        {
            free(result);
            return nullptr;
        }
        TXT_StringConcat(result, escaped, result_len);
        free(escaped);

        if (extensions[i + 1] != nullptr)
        {
            TXT_StringConcat(result, ",", result_len);
        }
    }

    TXT_StringConcat(result, "}", result_len);

    return result;
}

static char *GenerateSelector(const char *const window_title, const char **extensions)
{
    const char *chooser;
    char *ext_list = nullptr;
    char *window_title_escaped = nullptr;
    char *result = nullptr;
    unsigned int result_len = 64;

    if (extensions == TXT_DIRECTORY)
    {
        chooser = "choose folder";
    }
    else
    {
        chooser = "choose file";
        ext_list = CreateExtensionsList(extensions);
        if (!ext_list)
        {
            return nullptr;
        }
    }

    // Calculate size.

    if (window_title != nullptr)
    {
        window_title_escaped = CreateEscapedString(window_title);
        if (!window_title_escaped)
        {
            free(ext_list);
            return nullptr;
        }
        result_len += strlen(window_title_escaped);
    }
    if (ext_list != nullptr)
    {
        result_len += strlen(ext_list);
    }

    result = static_cast<char *>(malloc(result_len));
    if (!result)
    {
        free(window_title_escaped);
        free(ext_list);
        return nullptr;
    }

    TXT_StringCopy(result, chooser, result_len);

    if (window_title_escaped != nullptr)
    {
        TXT_StringConcat(result, " with prompt ", result_len);
        TXT_StringConcat(result, window_title_escaped, result_len);
    }

    if (ext_list != nullptr)
    {
        TXT_StringConcat(result, " of type ", result_len);
        TXT_StringConcat(result, ext_list, result_len);
    }

    free(window_title_escaped);
    free(ext_list);
    return result;
}

static char *GenerateAppleScript(const char *window_title, const char **extensions)
{
    char *selector, *result;
    size_t result_len;

    selector = GenerateSelector(window_title, extensions);
    if (!selector)
    {
        return nullptr;
    }

    result_len = strlen(APPLESCRIPT_WRAPPER) + strlen(selector);
    result = static_cast<char *>(malloc(result_len));
    if (!result)
    {
        free(selector);
        return nullptr;
    }

    TXT_snprintf(result, result_len, APPLESCRIPT_WRAPPER, selector);
    free(selector);

    return result;
}

int TXT_CanSelectFiles()
{
    return 1;
}

char *TXT_SelectFile(const char *window_title, const char **extensions)
{
    char *argv[4];
    char *result, *applescript;

    applescript = GenerateAppleScript(window_title, extensions);
    if (!applescript)
    {
        return nullptr;
    }

    argv[0] = const_cast<char *>("/usr/bin/osascript");
    argv[1] = const_cast<char *>("-e");
    argv[2] = applescript;
    argv[3] = nullptr;

    result = ExecReadOutput(argv);

    free(applescript);

    return result;
}

#else

// Linux version: invoke the Zenity command line program to pop up a
// dialog box. This avoids adding Gtk+ as a compile dependency.

#define ZENITY_BINARY "/usr/bin/zenity"

static unsigned int NumExtensions(const char **extensions)
{
    unsigned int result = 0;

    if (extensions != nullptr)
    {
        for (result = 0; extensions[result] != nullptr; ++result);
    }

    return result;
}

static int ZenityAvailable()
{
    return system(ZENITY_BINARY " --help >/dev/null 2>&1") == 0;
}

int TXT_CanSelectFiles()
{
    return ZenityAvailable();
}

//
// ExpandExtension
// given an extension (like wad)
// return a pointer to a string that is a case-insensitive
// pattern representation (like [Ww][Aa][Dd])
//
static char *ExpandExtension(const char *orig)
{
    size_t oldlen = strlen(orig);
    size_t newlen = oldlen * 4; // pathological case: 'w' => '[Ww]'
    char *newext = static_cast<char *>(malloc(static_cast<size_t>(newlen + 1)));

    if (newext == nullptr)
    {
        return nullptr;
    }

    char *c = newext;
    for (size_t i = 0; i < oldlen; ++i)
    {
        if (isalpha(orig[i]))
        {
            *c++ = '[';
            *c++ = static_cast<char>(tolower(orig[i]));
            *c++ = static_cast<char>(toupper(orig[i]));
            *c++ = ']';
        }
        else
        {
            *c++ = orig[i];
        }
    }
    *c = '\0';
    return newext;
}

char *TXT_SelectFile(const char *window_title, const char **extensions)
{
    if (!ZenityAvailable())
    {
        return nullptr;
    }

    char **argv = static_cast<char **>(std::calloc(5 + NumExtensions(extensions), sizeof(char *)));
    argv[0] = strdup(ZENITY_BINARY);
    argv[1] = strdup("--file-selection");
    int argc = 2;

    if (window_title != nullptr)
    {
        size_t len = 10 + strlen(window_title);
        argv[argc] = static_cast<char *>(malloc(len));
        TXT_snprintf(argv[argc], len, "--title=%s", window_title);
        ++argc;
    }

    if (extensions == TXT_DIRECTORY)
    {
        argv[argc] = strdup("--directory");
        ++argc;
    }
    else if (extensions != nullptr)
    {
        for (int i = 0; extensions[i] != nullptr; ++i)
        {
            char * newext = ExpandExtension(extensions[i]);
            if (newext)
            {
                size_t len = 30 + strlen(extensions[i]) + strlen(newext);
                argv[argc] = static_cast<char *>(malloc(len));
                TXT_snprintf(argv[argc], len, "--file-filter=.%s | *.%s",
                             extensions[i], newext);
                ++argc;
                free(newext);
            }
        }

        argv[argc] = strdup("--file-filter=*.* | *.*");
        ++argc;
    }

    argv[argc] = nullptr;

    char *result = ExecReadOutput(argv);

    for (int i = 0; i < argc; ++i)
    {
        free(argv[i]);
    }

    free(argv);

    return result;
}

#endif

static void TXT_FileSelectSizeCalc(void *uncast_fileselect)
{
    auto *fileselect = reinterpret_cast<txt_fileselect_t *>(uncast_fileselect);

    // Calculate widget size, but override the width to always
    // be the configured size.

    TXT_CalcWidgetSize(fileselect->inputbox);
    fileselect->widget.w = static_cast<unsigned int>(fileselect->size);
    fileselect->widget.h = fileselect->inputbox->widget.h;
}

static void TXT_FileSelectDrawer(void *uncast_fileselect)
{
    auto *fileselect = reinterpret_cast<txt_fileselect_t *>(uncast_fileselect);

    // Input box widget inherits all the properties of the
    // file selector.

    fileselect->inputbox->widget.x = fileselect->widget.x + 2;
    fileselect->inputbox->widget.y = fileselect->widget.y;
    fileselect->inputbox->widget.w = fileselect->widget.w - 2;
    fileselect->inputbox->widget.h = fileselect->widget.h;

    // Triple bar symbol gives a distinguishing look to the file selector.
    TXT_DrawCodePageString("\xf0 ");
    TXT_BGColor(TXT_COLOR_BLACK, 0);
    TXT_DrawWidget(fileselect->inputbox);
}

static void TXT_FileSelectDestructor(void *uncast_fileselect)
{
    auto *fileselect = reinterpret_cast<txt_fileselect_t *>(uncast_fileselect);

    TXT_DestroyWidget(fileselect->inputbox);
}

static int DoSelectFile(txt_fileselect_t *fileselect)
{
    char *path;

    if (TXT_CanSelectFiles())
    {
        path = TXT_SelectFile(fileselect->prompt,
                              fileselect->extensions);

        // Update inputbox variable.
        // If cancel was pressed (ie. nullptr was returned by TXT_SelectFile)
        // then reset to empty string, not nullptr).

        if (path == nullptr)
        {
            path = strdup("");
        }

        char **var = static_cast<char **>(fileselect->inputbox->value);
        free(*var);
        *var = path;
        return 1;
    }

    return 0;
}

static int TXT_FileSelectKeyPress(void *uncast_fileselect, int key)
{
    auto *fileselect = reinterpret_cast<txt_fileselect_t *>(uncast_fileselect);

    // When the enter key is pressed, pop up a file selection dialog,
    // if file selectors work. Allow holding down 'alt' to override
    // use of the native file selector, so the user can just type a path.

    if (!fileselect->inputbox->editing
     && !TXT_GetModifierState(TXT_MOD_ALT)
     && key == KEY_ENTER)
    {
        if (DoSelectFile(fileselect))
        {
            return 1;
        }
    }

    return TXT_WidgetKeyPress(fileselect->inputbox, key);
}

static void TXT_FileSelectMousePress(void *uncast_fileselect,
                                     int x, int y, int b)
{
    auto *fileselect = reinterpret_cast<txt_fileselect_t *>(uncast_fileselect);

    if (!fileselect->inputbox->editing
     && !TXT_GetModifierState(TXT_MOD_ALT)
     && b == TXT_MOUSE_LEFT)
    {
        if (DoSelectFile(fileselect))
        {
            return;
        }
    }

    TXT_WidgetMousePress(fileselect->inputbox, x, y, b);
}

static void TXT_FileSelectFocused(void *uncast_fileselect, int focused)
{
    auto *fileselect = reinterpret_cast<txt_fileselect_t *>(uncast_fileselect);

    TXT_SetWidgetFocus(fileselect->inputbox, focused);
}

txt_widget_class_t txt_fileselect_class =
{
    TXT_AlwaysSelectable,
    TXT_FileSelectSizeCalc,
    TXT_FileSelectDrawer,
    TXT_FileSelectKeyPress,
    TXT_FileSelectDestructor,
    TXT_FileSelectMousePress,
    nullptr,
    TXT_FileSelectFocused,
};

// If the (inner) inputbox widget is changed, emit a change to the
// outer (fileselect) widget.

static void InputBoxChanged(void *, void *uncast_fileselect)
{
    auto *fileselect = reinterpret_cast<txt_fileselect_t *>(uncast_fileselect);

    TXT_EmitSignal(&fileselect->widget, "changed");
}

txt_fileselect_t *TXT_NewFileSelector(char **variable, int size,
                                      const char *prompt, const char **extensions)
{
    auto *fileselect = create_struct<txt_fileselect_t>();

    TXT_InitWidget(fileselect, &txt_fileselect_class);
    fileselect->inputbox = TXT_NewInputBox(variable, 1024);
    fileselect->inputbox->widget.parent = &fileselect->widget;
    fileselect->size = size;
    fileselect->prompt = prompt;
    fileselect->extensions = extensions;

    TXT_SignalConnect(fileselect->inputbox, "changed",
                      InputBoxChanged, fileselect);

    return fileselect;
}

