
Davici Utilities for Strongswan
===============================


Copyright
=========

Davici Utilities for Strongswan
Copyright (C) 2026 David M. Syzdek <david@syzdek.net>.  
All rights reserved.  

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   * Neither the name of David M. Syzdek nor the
     names of its contributors may be used to endorse or promote products
     derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVID M. SYZDEK BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.


Overview
==========

This project provides a shell interface to the strongSwan vici plugin. The
utility can format responses from the vici socket in either JSON, XML, YAML,
or native vici output.  The utility attempts to implement the commands and
registered events documented in the [Versatile IKE Control Interface (VICI) protocol](https://github.com/strongswan/strongswan/blob/master/src/libcharon/plugins/vici/README.md).
Currently the following widgets are supported:

   *  alert             - displays alert events
   *  child-updown      - displays child-updown events
   *  child-rekey       - displays child-rekey events
   *  clear-creds       - clears loaded certs, private keys and shared keys
   *  diagnostics       - retreieves various diagnostics information
   *  get-algorithms    - lists loaded algorithms and their implementation
   *  get-authorities   - lists loaded CA names
   *  get-conns         - lists connections loaded over vici
   *  get-counters      - lists global or connection-specific counters
   *  get-keys          - lists private keys identifiers loaded over vici
   *  get-pools         - lists loaded pools.
   *  get-shared        - lists unique shared key identifiers loaded over vici
   *  ike-rekey         - displays ike-rekey events
   *  ike-update        - displays ike-update events
   *  ike-updown        - displays ike-updown events
   *  initiate          - initiates an SA
   *  install           - installs a CHILD_SA's 'trap, drop or bypass policy
   *  list-authorities  - Lists loaded certification authorities
   *  list-conns        - lists all loaded connections
   *  list-policies     - lists installed trap, drop and bypass policies
   *  list-sas          - lists active IKE_SAs and associated CHILD_SAs
   *  log               - displays debug log messages
   *  raw               - queues a command or event to the vici control socket
   *  rekey             - initiates rekeying of an SA
   *  reload-settings   - reloads strongswan.conf settings and plugins
   *  reset-counters    - resets global or connection-specific counters
   *  stats             - returns IKE daemon statistics
   *  terminate         - terminates an SA
   *  uninstall         - uninstalls a CHILD_SA's 'trap, drop or bypass policy
   *  unload-authority  - unloads a certification authority into the daemon
   *  unload-conn       - unloads a connection definition from the daemon
   *  unload-pool       - unloads a virtual IP and attribute pool.
   *  version           - returns daemon and system versions

Examples
========

The following example queues the "version" command and displays the response
using JSON:

    $ davicictl version -O json
    {"version-reply": {"daemon": "charon", "version": "5.9.14", "sysname": "Linux", "release": "6.12.19-akcom-1_acs", "machine": "x86_64"}}

The following example queues the "version" command and displays the response
using beautified JSON:

    davicictl version -O json --pretty
    {
       "version-reply": {
          "daemon": "charon",
          "version": "5.9.14",
          "sysname": "Linux",
          "release": "6.12.19-akcom-1_acs",
          "machine": "x86_64"
       }
    }

The following example queues the "version" command and displays the response
using XML:

    $ davicictl version -O xml
    <?xml version="1.0" encoding="UTF-8"?>
    <vici><version-reply><daemon>charon</daemon><version>5.9.14</version><sysname>Linux</sysname><release>6.12.19-akcom-1_acs</release><machine>x86_64</machine></version-reply></vici>

The following example queues the "version" command and displays the response
using beautified XML:

    davicictl version -O xml --pretty
    <?xml version="1.0" encoding="UTF-8"?>
    <vici>
       <version-reply>
          <daemon>charon</daemon>
          <version>5.9.14</version>
          <sysname>Linux</sysname>
          <release>6.12.19-akcom-1_acs</release>
          <machine>x86_64</machine>
       </version-reply>
    </vici>

The following example queues the "version" command and displays the response
using YAML:

    $ davicictl version -O xml
    ---
    version-reply:
       daemon: charon
       version: 5.9.14
       sysname: Linux
       release: 6.12.19-akcom-1_acs
       machine: x86_64


Maintainers
===========

* David M. Syzdek <david@syzdek.net>


Source Code
===========

The source code for this project is maintained using git (http://git-scm.com).
The following contains information to checkout the source code from the git
repository.

Browse Source:

   * https://github.com/syzdek/davici-utils.git

Git URLs:

   * https://github.com/syzdek/davici-utils.git

Preparing Source:

    $ git clone https://github.com/syzdek/davici-utils.git
    $ cd davici-utils
    $ ./autogen.sh


Package Maintenance Notes
=========================

This is a collection of notes for developers to use when maintaining this
package.

New Release Checklist:

   * Switch to 'master' branch in Git repository.
   * Update version in configure.ac.
   * Update date and version in ChangeLog.
   * Commit configure.ac and ChangeLog changes to repository.
   * Create tag in git repository:
   
         $ git tag -s v${MAJOR}.${MINOR}
       
   * Push repository to publishing server:
   
         $ git push --tags origin master:master next:next pu:pu

 Creating Source Distribution Archives:

    $ ./configure
    $ make update
    $ make distcheck
    $ make dist-bzip2
    $ make dist-xz

