SMTP proxy over SSH connections
===============================

`ssh-smtp` creates an SMTP service that recognizes `MAIL FROM` SMTP commands
and connects to the appropriate server using SSH, and interconnects the socket
with the remote SMTP server. This way, the confidentiality, integrity and
authentication of the SMTP connection is handled by SSH, thus

 - existing SSH keys can be used,
 - there's no need for TLS configuration, SSH provides PFS by using DH, and
 - if you already have an SSH connection open, multiplexing avoids the need
   to open another TCP socket, if your SSH configuration supports it.

Compiling
---------

Execute `qmake && make` which should compile everything and produce `ssh-smtp`

Usage
-----

Linux users should modify `ssh-smtp.sample.conf` according to the comments, and
save it into `~/.config/dnet/ssh-smtp.conf`. Windows and Mac OS should work as
well, but I'll leave that as an exercise to the reader (see Registry/plist).

License
-------

The whole project is licensed under MIT license.

Runtime dependencies
--------------------

 - QT4 Core (Debian/Ubuntu package: `libqtcore4`)
 - QT4 Network (Debian/Ubuntu package: `libqt4-network`)

Build dependencies
------------------

 - QT4 development files (Debian/Ubuntu package: `libqt4-dev`)
