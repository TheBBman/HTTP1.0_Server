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

    ###########################################################
    ##                                                       ##
    ## REPLACE CONTENT OF THIS FILE WITH YOUR PROJECT REPORT ##
    ##                                                       ##
    ###########################################################