# CS118 Project 0

This is the repo for spring23 cs118 project 0.

## Academic Integrity Note

You are encouraged to host your code in private repositories on [GitHub](https://github.com/), [GitLab](https://gitlab.com), or other places.  At the same time, you are PROHIBITED to make your code for the class project public during the class or any time after the class.  If you do so, you will be violating academic honestly policy that you have signed, as well as the student code of conduct and be subject to serious sanctions.

## Provided Files

- `project` is folder to develop codes for future projects.
- `docker-compose.yaml` and `Dockerfile` are files configuring the containers.

## Bash commands

```bash
# Setup the container(s) (make setup)
docker compose up -d

# Bash into the container (make shell)
docker compose exec node1 bash

# Remove container(s) and the Docker image (make clean)
docker compose down -v --rmi all --remove-orphans
```

## Environment

- OS: ubuntu 22.04
- IP: 192.168.10.225. NOT accessible from the host machine.
- Port forwarding: container 8080 <-> host 8080
  - Use http://localhost:8080 to access the HTTP server in the container.
- Files in this repo are in the `/project` folder. That means, `server.c` is `/project/project/server.c` in the container.

## TODO

Name: Justin Hu
UID: 205-514-102

High level Design:

The main function of my server has 2 simple parts, setup and loop. In the setup area, I create and bind
the socket to be used (port 8080), create a list of all available files under current directory, and 
setup some premade strings to be used in generating HTTP responses. In the loop area, I accept a single
new connection, extract desired filename, search directory, and construct my response. 

Major Problems:

Everything up to receiving HTTP request was pretty straightforward, and using scandir to enumerate files
wasn't as painful as I thought it would be. Case insensitivity wasn't very difficult either. At some point
I was getting segfaults, so I used valgrind to clean up my code which took about an hour. 

First major issue I faced was constructing the HTTP response. Somehow I got the idea that the entire HTTP
response had to be within one buffer, which was a nightmare to allocate correctly and resulted in a lot of
content length incorrect errors. I also didn't read that I needed an extra \r\n at the end of the HTTP header,
which made my image files not work. Basically, I didn't understand HTTP as well as I should have.

After that, there was this weird issue of larger image file transmission somehow being cut short. I realized
that removing close(socket) at the end of the loop completely solves the issue, and to now I still have
no idea why that is the case. I tried removing the REUSEPORT option but that didn't change anything.

My last obstacle was dealing with escape and space characters in filename. This took me 
much more effort than I had initially imagined because I suppose I am not a proficient C programmer
and dealing with string pointers was a nightmare. Thankfully I found a great reference on stack overflow
that really helped me out, in particular the idea of using multiple string pointers just to help 
keep track of where I am in the original and new strings.

I never had to debug anything for case insensitivity, filetype, or large binary data. Everything
just worked first try, which I suppose was nice. I only ever tested on html, txt, and jpg.

Code References:

https://beej.us/guide/bgnet/ 
Beej's guide got me started, learning all about the socket API and how to do the project basically.

https://stackoverflow.com/questions/779875/what-function-is-to-replace-a-substring-from-a-string-in-c
Forum post about substring search and replacement

Also looked up quite a few linux man pages.