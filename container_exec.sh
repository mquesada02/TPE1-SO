#!/bin/bash
# docker run -v "${PWD}:/root" --privileged -ti --name SO agodio/itba-so:2.0

sudo docker start SO
sudo docker exec -ti SO make clean -C/root/
sudo docker exec -ti SO make -C/root/
sudo docker stop SO