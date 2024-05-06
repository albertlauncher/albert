#! /bin/sh

docker build -t albert.flatpak -f flatpak.Dockerfile ..
docker run --rm -it \
  --env="DISPLAY=host.docker.internal:0" \
  -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
  --cap-add CAP_SYS_ADMIN \
  --cap-add CAP_NET_ADMIN \
  -v `pwd`/..:/src \
  albert.flatpak

  #--security-opt seccomp=unconfined --security-opt apparmor=unconfined \



