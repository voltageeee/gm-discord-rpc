# Garry's Mod Discord RPC
A Garry's Mod binary module that introduces advanced discord's RPC support on any server for any client.

Playing on a random DarkRP server:

![image](https://github.com/user-attachments/assets/d23b567c-25e7-4f54-96d7-d4035622fa04)

This module is made with danielga's garrysmod_common:

https://github.com/danielga/garrysmod_common

# Usage
```
Download the latest version from the "Releases" tab, according to your game bitness (x32 or x64)
Download discord-rpc for windows from here: https://github.com/discord/discord-rpc/releases/tag/v3.4.0
Place discord-rpc.dll for your game bitness in your GarrysMod folder, near gmod.exe
Place the lua folder in your garrysmod root directory
Launch discord & garry's mod
Connect to the server
Enjoy!
```

# Compiling
```
Download the latest premake5 and add it to your PATH
Clone danielga's garrysmod_common: https://github.com/danielga/garrysmod_common (don't forget --recurse-submodules option!)
Run "premake5.exe --gmcommon=./path/to/garrysmod_common vs2022" in your project directory
Open the generated project in Visual Studio
Compile
```

