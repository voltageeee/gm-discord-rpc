# gm-discord-rpc
a discord rpc garry's mod module that can be used by anyone without the need of installing this on the server

this is made possible with gm_rocx by Earu: https://github.com/Earu/gm_rocx, gmod-gameevent by RaphaelIT7: https://github.com/RaphaelIT7/gmod-gameevent and garrysmod_common by danielga: https://github.com/danielga/garrysmod_common

# usage
```
download the latest version from the "releases" tab
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
