# gm-discord-rpc
a discord rpc garry's mod module that can be used by anyone without the need of installing this on the server

this is made possible by utilizing source engine's event system (initially, i didn't think about it). thanks to the guys from Garry's Mod discord:
thanks to wholecream
![image](https://github.com/user-attachments/assets/c1b9ea54-d8be-4da0-a2bc-3e9054891573)

honorable mention:
![image](https://github.com/user-attachments/assets/85d090a7-25e1-442d-98e8-9b7a48bf45ae)


# usage
```
download the latest version from the "releases" tab
download discord-rpc.dll from here: https://github.com/discord/discord-rpc/releases/tag/v3.4.0
place discord-rpc.dll in your GarrysMod folder, near gmod.exe
place the lua folder in your garrysmod root directory
launch discord & garry's mod
connect to the server
enjoy!
```

# compiling
```
download the latest premake5 and add it to your PATH
git clone --recurse submodules garrysmod_common by danielga: https://github.com/danielga/garrysmod_common
run premake5 --gmcommon=path/to/garrysmod_common vs2019 in the project directory
open projects/windows/vs2019/vltg_ds.sln
compile with vs2019 toolset
```

playing on a server without any related gmsv modules installed:
![image](https://github.com/user-attachments/assets/df5d4216-8ca9-4820-83a0-d46fb6e0aeeb)

