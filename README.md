# TermSrv Patcher

TermSrv Patcher is a Windows utility that backs up, patches, and restores `termsrv.dll` to enable multiple concurrent RDP sessions. It searches for known byte signatures baked into the tool and applies the matching replacement.

## Getting Started

These steps help you build and run the tool locally. Running the patch requires Administrator rights and should be done with care.

### Prerequisites

- Windows with an administrator account  
- Visual Studio (Desktop development with C++) or MinGW-w64  
- CMake (optional, if you prefer generating project files)  
- A backup of `C:\Windows\System32\termsrv.dll` (the tool creates its own backup)

### Installing

Visual Studio:
1. Open the solution or add `termsrvpatcher.cpp` to a new C++ Console project.
2. Select Release x64.
3. Build to produce `termsrvpatcher.exe`.

MinGW alternative:

```powershell
g++ -std=c++17 -O2 termsrvpatcher.cpp -luser32 -lshell32 -o termsrvpatcher.exe
```

### Usage

1. Run `termsrvpatcher.exe` as Administrator.  
2. The tool:
   - creates `%TEMP%\patcher`,  
   - copies and backs up `termsrv.dll` to `termsrv-bak.dll`,  
   - scans for the first matching signature and applies the replacement bytes,  
   - stops `termservice` and `umrdpservice`, replaces the DLL, then restarts the services.
3. On success it prints `successfully patched!!`. If no signature matches, it prints `no patch needed!!` and leaves the system untouched.

### Running the tests

There are no automated tests. Recommended manual checks:

```powershell
.\termsrvpatcher.exe   # run as Administrator
qwinsta                # inspect active sessions
```

After patching, attempt parallel RDP connections from another machine to confirm behavior.

## Deployment

Use only on systems where you have admin rights. Create a system restore point or manual backup before patching. Windows updates may overwrite `termsrv.dll`; rerun the patcher after updates if needed.

## Built With

- C++17 and WinAPI  
- Visual Studio / MinGW-w64

## Contributing

Pull requests are welcome. Keep the signature (Find/Replace) format consistent and add new patterns when Microsoft ships new `termsrv.dll` builds.

## Versioning

Follow [SemVer](http://semver.org/). Keep tags and releases in the GitHub repository.

## Authors

- Original author – TermSrv Patcher project

## License

This project is licensed under the MIT License – see `LICENSE.md` for details.

## Acknowledgments

- Signature data for Windows 10/11 builds  
- Thanks to the RDP patching community for inspiration
