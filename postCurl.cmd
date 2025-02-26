@echo off
setlocal EnableDelayedExpansion
set url=http://192.168.0.64/ISAPI/Traffic/channels/1/searchLPListAudit
set data="<LPListAuditSearchDescription version=\"2.0\" xmlns=\"http://www.isapi.org/ver20/XMLSchema\"><searchID>1</searchID><searchResultPosition>0</searchResultPosition><maxResults>100</maxResults><type>blackList</type><LicensePlate></LicensePlate><cardNo></cardNo><cardID></cardID></LPListAuditSearchDescription>"
curl -X POST -u admin:Neolink79 -H "Content-Type: application/xml" -d !data! !url!