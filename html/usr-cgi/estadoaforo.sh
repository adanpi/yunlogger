#!/bin/sh
echo Content-Type: text/html
echo Expires: Tue, 17 Jun 1979 02:20:00
echo Expires: 0
echo Pragma: no-cache
echo
echo '<!DOCTYPE HTML>'
echo '<html>'

echo '<head>'
ip=`ifconfig  | grep 'inet addr:'| grep -v '127.0.0.1' | cut -d: -f2 | a
echo '<meta http-equiv="refresh" content="0; url=http://'
echo $ip
echo ':8082/aforo.xhtml" />'
echo '</head>'
echo '</html>'
