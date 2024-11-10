# vtm

It is a text-based application where the entire user interface is represented by a mosaic of text cells forming a TUI matrix. The resulting TUI matrix is ​​just rendered either into its own GUI window or into a compatible text console.

```mermaid
graph TB
  subgraph GUI[Native GUI Window]
    subgraph TUI[TUI Matrix]
      subgraph DESK[Desktop]
        direction LR
        subgraph APP1[Application]
          direction LR
          App1[Application UI]
        end
        subgraph APP2[Application]
          direction LR
          App2[Application UI]
        end
      end
    end
  end
  subgraph GUI2[Generic Text Console]
    subgraph TUI2[TUI Matrix]
      subgraph DESK2[Desktop]
        direction LR
        subgraph APP21[Application]
          direction LR
          App21[Application UI]
        end
        subgraph APP22[Application]
          direction LR
          App22[Application UI]
        end
      end
    end
  end
  subgraph GUI3[Native GUI Window]
    subgraph TUI3[TUI Matrix]
      subgraph APP33[Application]
        direction LR
        App33[Application UI]
      end
    end
  end
```

It can run indefinitely nested, forming a text-based desktop environment.

<a href="https://www.youtube.com/watch?v=kofkoxGjFWQ">
  <img width="400" alt="Demo on YouTube" src="https://user-images.githubusercontent.com/11535558/146906370-c9705579-1bbb-4e9e-8977-47312f551cc8.gif">
</a>

# Supported platforms

- Windows
  - Windows 8.1 and later
- Unix
  - Linux
  - macOS
  - FreeBSD
  - NetBSD
  - OpenBSD
  - [`...`](https://en.wikipedia.org/wiki/POSIX#POSIX-oriented_operating_systems)

[Tested Terminals](https://github.com/directvt/vtm/discussions/72)

Currently, rendering into a native GUI window is only available on the Windows platform; on Unix platforms, a terminal emulator is required.

# Binary downloads

![Linux](.resources/status/linux.svg)     [![Intel 64-bit](.resources/status/arch_x86_64.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_linux_x86_64.zip) [![Intel 32-bit](.resources/status/arch_x86.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_linux_x86.zip) [![ARM 64-bit](.resources/status/arch_arm64.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_linux_arm64.zip) [![ARM 32-bit](.resources/status/arch_arm32.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_linux_arm32.zip)  
![Windows](.resources/status/windows.svg) [![Intel 64-bit](.resources/status/arch_x86_64.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_windows_x86_64.zip)  [![Intel 32-bit](.resources/status/arch_x86.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_windows_x86.zip)  [![ARM 64-bit](.resources/status/arch_arm64.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_windows_arm64.zip)  [![ARM 32-bit](.resources/status/arch_arm32.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_windows_arm32.zip)  
![macOS](.resources/status/macos.svg)     [![Universal](.resources/status/arch_any.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_macos_any.zip)  

# Documentation

- [Architecture](doc/architecture.md)
- [Building from source](doc/build.md)
- [Command-line options](doc/command-line-options.md)
- [User interface](doc/user-interface.md)
- [Settings](doc/settings.md)
- [Unicode Character Geometry Modifiers](doc/character_geometry.md)
