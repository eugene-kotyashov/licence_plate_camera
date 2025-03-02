@echo off
setlocal EnableDelayedExpansion
set url=http://192.168.0.64//ISAPI/System/IO/outputs/1/trigger
set data="<IOPortData xmlns=\"http://www.hikvision.com/ver10/XMLSchema\" version=\"1.0\"><outputState>low</outputState></IOPortData>"
curl -X PUT -u admin:Neolink79 -H "Content-Type: application/xml" -d !data! !url!