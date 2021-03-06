# OSM Scout Server

OSM Scout Server can be used as a drop-in replacement for online map
services providing map tiles, search, and routing. As a result, an
offline operation is possible if the device has a server and map
client programs installed and running. OSM Scout Server is mainly
developed for Sailfish OS, but could be used on a regular Linux
system.

Here, the user guide is provided with the description of setting up
the server and popular clients for offline operation on Sailfish OS
device. For developer's info, see
[GitHub page](https://github.com/rinigus/osmscout-server) of the
project and it's
[README](https://github.com/rinigus/osmscout-server/blob/master/README.md).


## Mode of operation

In contrast to offline navigation solutions provided by other
applications, the server is one of the two parts that are required by
users. Users would need to have the server and a client accessing the
server running _simultaneously_ and communicating with each
other. 

After initial setup, users would mainly have the server running in the
background while accessing maps and getting navigation instructions
through client. The server's GUI is only needed for managing maps on
device. On Sailfish OS, the normal mode of operation would require
server running as one of the applications showing a cover and client
opened as needed.


## Setting up the server

There are several steps required to setup the server. The following
guide is aimed at Sailfish OS users.

### Modules

The server uses modular approach with two separate modules required
for its operation. Please proceed to Jolla Store or OpenRepos and
install _OSM Scout Server Module: Route_ and _OSM Scout Server Module:
Fonts_. After installation of the modules, please restart the server,
if instructed by the server. The modules are used automatically and
don't have to be started by users.

### Storage

OSM Scout Server needs to store maps. The storage requirements could
be significant. To store maps and manage them, a separate folder is
required. Please note that, as a part of the managing, OSM Scout
Server can delete, on your command, files from that folder. Thus, its
important to allocate such folder and assign it to OSM Scout
Manager. See [Storage allocation tutorial](storage.html) for example
on how to do it with the help of
[FileCase](https://openrepos.net/content/cepiperez/filecase-0).

### Map Manager

To download, update, and remove maps, use Map Manager. The initial
subscription of the maps and their download is described in
[Map Manager Download tutorial](manager.html). 

After the maps are downloaded, you are ready to proceed with the
configuration of your map access client. Select the corresponding
section below to see how to configure it.

### Language selection

The server uses natural language processing (NLP) library that covers
processing of addresses in large number of languages -
[libpostal](https://github.com/openvenues/libpostal). To limit usage
of resources, please specify languages as shown in
[Language selection tutorial](languages.html).

### Profile

To simplify configuration, OSM Scout Server uses profiles. You are
asked to select profile on the first start. Later, you can select
profile as shown in the [Profile selection tutorial](profiles.html).

### Settings

There are multiple settings that can be useful to tune the operation
of the server. See some examples in
[Settings examples](settings_misc.html).


## Setting up the client

After the server has been setup and the maps downloaded, the access to
the server has to be configured in the client(s).

For [Poor Maps](https://openrepos.net/content/otsaloma/poor-maps),
instructions are [here](poor_maps.html).

For [modRana](https://openrepos.net/content/martink/modrana-0),
instructions are [here](modrana.html).

After the client is setup, you can use them together with OSM Scout
Server for offline maps access.
