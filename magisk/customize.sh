#!/sbin/sh
SKIPUNZIP=1

ui_print "= CuprumTurbo Scheduler Module ="
ui_print "- Installing..."

ui_print "- Extracting module files."
if [ -d "$MODPATH" ]; then
    rm -rf "$MODPATH"
fi
unzip -o "$ZIPFILE" -x "META-INF/*" -d "$MODPATH" >/dev/null 2>&1
chmod -R 0777 "$MODPATH"

ui_print "- Installation finished."
