rm -f /mnt/os2data/public/pwicq/pwicq-standard-2.0.0-1.i586.tar.gz
tar --create --gzip -h --exclude=/usr/share/pwicq/sounds/* --file=/mnt/os2data/public/pwicq/pwicq-standard-2.0.0-1.i586.tar.gz /usr/share/pwicq/*

