# Distributed-File-Sync  

This project is a peer to peer file sync program. Upon running, it will create a shared folder in your Documents folder called `File Sync Shared Folder`. Anything you add, remove, or edit from this folder will add, remove, or edit it on computers of connected nodes. Multiple files can be altered at once and it can send files of any format and size. No user interaction is required other than your interactions with the file system itself.

---

## How it Works

### Node discovery

When the program is run it will automatically connect to the existing network if there is one or start the network if there isn't. Any time a node is created, it broadcasts to the entire local network, on a certain port, that it has arrived. All existing nodes, if any, will then send back a targeted acknowledgement. 

### Initial File Sync

Whenever a node first arrives, it broadcasts its arrival as well as a hash table of its files. All nodes in the network receive this and send back their hash tables as part of the acknowledgement. All nodes compare the hash table they received with their own hash table. Any discrepencies they see will then cause them request files from the foreign hash table's sender. As a design choice, we chose to handle files of the same path but different hashes by claiming the existing node's file as the correct one, so if a new node joins with a file known by others but has a different hash then they delete it and receive the one known by the network.

### Changes During Runtime

If a change is made inside the shared folder while a node is on the network then the file change data is stored and sent across the network to other nodes. They then take the necessary action to recreate that change. For example, if node A and node B exist and A edits the contents of a file, then a fileChangeData is sent to B with details of the transaction. B then uses that to adjust his local files and hashes to reflect the change.

---

## How it's Made

### Thread Overview

The program features 4 main threads. These are the main, udp discovery, udp handler, and directory watcher. The main simply creates a Node object and launches the other 3 threads then waits for them to finish. The udp discovery is for announcing one's arrival as well as collecting all udp traffic on the main udp port. The udp handler is a loop that repeatedly grabs from the udp queue one packet at a time and takes the necessary action depending on the contents of the packet. Finally, the directory watcher is for catching changes in the shared folder and packaging them for the udp handler to take care of. 

### How Messages are Sent Across the Network

This project uses the SFML multimedia library for C++ because it offers class based wrappers for the winsock networking sockets. Information is is packaged into a Packet object (provided by SFML) and then sent to receivers. The Packet class takes care of endieness issues for us.

### How Directory Changes are Caught

We used the Microsoft Windows Directory Watcher for automatically detecting when a change is made. We then made the program sift through the directory to figure exactly what kind of change occured. This is then packed into a fileChangeData object which contains the minimum necessary information for recreating the change on another computer.

### How Files are Sent

When a file is requested by one client to another, the two negotiate a TCP transfer and get a 1-to-1 server-client system set up. The server then sends TCP packets containing 1KB of the requested file's data along with a few things for book keeping. The client receives this, writes it to a file, and sends back their position in the file. The server receives this, seeks to that postision, and proceeds to send the next chunk of data. This continues until the file position hits the end.

### How Packets are Structured

All packets have a pid, or packet ID, as there first element. This identifies in what way the packet should be handled and, as such, lets the program now what more information it can extract. 

### How File Hashes are Calculated

Files are hashed into an unsigned 64 bit integer. This is done by starting the hash off as the file's size. It then iterates over every character in the file's path(relative to the shared folder) then every byte in the file's contents. Each character/byte's value is multiplied my a single prime from a predetermined list(`2,7,17,29,41,53,67,79,97`) in  ascending order, resetting back to 2 after 97.
