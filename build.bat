@echo off
mkdir export\vpk
copy dir\englishclient_frontend.bsp.pak000_dir.vpk export\vpk
pushd export\vpk
RSPNVPK englishclient_frontend.bsp.pak000_dir.vpk -s -d ..\..\src
popd