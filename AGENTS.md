# AGENTS.md

This file provides guidance to agents when working with code in this repository.

## Project Overview

A Windows C++ DLL library (`zen_misc_tool`) providing string encoding conversion utilities (UTF-8, local codepage, UTF-16). Built with Visual Studio 2022 (v143 toolset), targets x64 Windows.

## Build

- **IDE**: Visual Studio 2022, solution file: `zen_misc_tool.sln`
- **x64 Debug** → `bin/x64/Debug/zen_misc_toold.dll` (note the typo: `toold`)
- **x64 Release** → `bin/x64/Release/zen_misc_tool.dll`
- Win32 configurations produce `.exe` (Application type), x64 produces `.dll` (DynamicLibrary) — this asymmetry is intentional in the `.vcxproj`
- Pre-build event (`copy.bat`) runs automatically for **both** x64 Debug and x64 Release, copying `zen_misc_tool/zen_misc_tool.h` → `include/zen_misc/`; it does NOT run for Win32

## External Dependencies

Dependencies are resolved via shared `.props` files (NOT in this repo):
- `../../../VisualStudioProp/Normal.props`
- `../../../3rd/x64-windows/share/3rdLibDir.props`
- `../../../Libraries/x64-windows/share/LibrariesDir.props`

Key third-party libraries:
- **`boost::locale`** — all string encoding conversions (`boost/locale.hpp`); `std::locale` is constructed once in `Impl()` and cached as `systemLocale_` — do not re-introduce per-call `generator` construction
- **`zen_rttr`** — custom RTTR (runtime type reflection) for DLL export; linked via `zen_rttr_core_096d.lib` (Debug) / `zen_rttr_core_096.lib` (Release); `ZEN_RTTR_DLL` must be defined before **any** `zen_rttr/` include

## Architecture

- **Public API header**: `include/zen_misc/zen_misc_tool.h` — consumers include only this; `zen_misc_tool/zen_misc_tool.h` is the source copy kept in sync by `copy.bat`
- **No traditional DLL exports** — all access goes through `zen_rttr` global method lookup; never add `__declspec(dllexport)`
- **Factory loading**: `getMiscToolFactory(lib_dir)` (inline in public header) uses a `static std::atomic<bool> init` guard — safe for post-init reads; DLL load itself is still not protected by a mutex for concurrent first calls
- **Factory registration**: `zen_misc_tool_factory.cpp` registers via `RTTR_REGISTRATION`; the method name `"zen::misc::getMiscToolFactory"` must match exactly or the factory is silently unfindable at runtime
- **`ZEN_RTTR_DLL` in factory `.cpp`**: must be defined again explicitly before `#include <zen_rttr/registration>` in `zen_misc_tool_factory.cpp` — the public header's definition does not carry over
- **Pimpl pattern**: `StringTool` (outer class) is a thin delegator only; all `boost::locale` calls live in `StringTool::Impl` inside `zen_misc_tool_impl.cpp`
- **Composite conversions**: `unicode16ToLocal` and `localToUnicode16` are two-step operations bridged through UTF-8 (`UTF-16 ↔ UTF-8 ↔ Local`); they return empty string/wstring if any intermediate step fails

## Adding New Conversion Methods

Requires changes in exactly 4 places:
1. `IStringTool` — add pure virtual declaration (public header)
2. `StringTool` — add override declaration (`zen_misc_tool_impl.h`)
3. `StringTool::Impl` — add implementation (`zen_misc_tool_impl.cpp`)
4. `StringTool` delegator — add forwarding method (`zen_misc_tool_impl.cpp`)

## Key Conventions

- Namespace macros: `ZEN_MISC_TOOL_NAMESPACE_BEGIN` / `ZEN_MISC_TOOL_NAMESPACE_END` (expand to `namespace zen { namespace misc {` / `}}`)
- Header guards use double-underscore pattern: `#ifndef __zen_misc_tool_h__`
- Source files are UTF-8 with BOM
- Error handling: all conversions catch all exceptions and return empty `std::string` / `std::wstring` — never propagate exceptions to caller
- Factory header is intentionally misspelled: `zen_msic_tool_factory.h` (`msic` not `misc`) — referenced by name in `.vcxproj`; do not rename
- `IStringTool` copy constructor and assignment operator are explicitly deleted

## No Test Framework

No automated tests exist. Validation is manual: build x64 Debug and exercise via `getMiscToolFactory()` in a separate test executable.
