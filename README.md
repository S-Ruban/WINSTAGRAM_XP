# `WINSTAGRAM_XP`

This is a stand-alone Instagram front-end application for Windows XP.

## Setup Process

First, you'll need a Windows XP environment. If you have an old laptop/desktop lying around, good for you. Else you can install a VM using either [Oracle Virtual Box](https://www.virtualbox.org/) or [VMWare](https://www.vmware.com/products/desktop-hypervisor/workstation-and-fusion). I chose VMWare for this project since I'm having some issues with Virtual Box.

## Some prerequisites for the Windows XP environment

- 512 MB RAM
- 5 GB Storage
- An active internet connection

My VM has 1 GB RAM and 5 GB storage.

Setting up a Windows XP VM is pretty straightforward and quick (since you're running it on a modern CPU), but just incase you want some resources:

- https://www.wikihow.com/Install-Windows-XP
- https://www.wikihow.com/Install-Windows-XP-Professional-on-a-Vmware-Workstation

You can find a Windows XP ISO and product key from the internet.

</b>

Since this project is written in C++, the Windows XP environment should have Microsoft Visual C++ 6.0 installed. You can find it [here](https://winworldpc.com/product/visual-c/6x).

## Source Code Management

Windows XP is ancient. As a result, `git` isn't supported and does not work.

While you could consider using a shared folder between host and VM, errors may arise due to the difference in OS and file storage format (XP uses FAT32 and later Windows versions use NTFS). Setting up an FTP server on my host was a hassle so I ditched that option as well.

As a workaround, I decided to use the good ol-fashioned HTTP server to pull/push code.

`curl` and `wget` aren't available in Windows XP. Luckily, curl.se (an archived page rather) has an older `curl` (version 7.8) that is still compatible with Windows XP. You can download it from [here](https://web.archive.org/web/20211208160135/https://curl.se/windows/).

## Server

The [development server](./server/server.py) is written in Python using Flask. Running the code is enough to start the server. The server has the following endpoints:

- `/upload` - receives a `POST` request to upload a certain file. Files uploaded are stored in [client](./client/).

- `/delete_all` - receives a `DELETE` request that deletes all the files stored in [client](./client/).

- `/list_files` - receives a `GET` request and returns a JSON output of all the available files stored in [client](./client/).

- `/download/<path:filename>` - receives a `GET` request and returns the file `filepath`.

The idea is that whenever the client (Windows XP) needs to update the server with updated code, it sends a `DELETE` request to `/delete_all` to remove all the source code before sending a `POST` request to `/upload` to upload all the source code again.

When the client needs to fetch the updated data, it sends a `GET` request to `/list_files` to list all the available files on the server, and then for every file, it sends a `GET` request to `/download/<path:filename>`.

It's not the most efficient SCM, but you know...`¯\_(ツ)_/¯`

PS: If you have a better flow, please feel free to create a PR.

## Client

The [client](./client/) stores the actual source code of the project. This source code is part of the MSVC++ 6.0 project inside Windows XP.

[download_all_files.bat](./client/download_all_files.bat) downloads the latest code from host to VM as explained above.

[upload_all_files.bat](./client/upload_all_files.bat) uploads the latest code from VM to host as explained above.
