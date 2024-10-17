# Dockerfiles


## Build

```sh
docker build --progress=plain -f "${path}/arch.Dockerfile" -t albert:arch --platform linux/amd64 .
docker build --progress=plain -f "${path}/fedora.Dockerfile" -t albert:fedora .
docker build --progress=plain -f "${path}/ubuntu.Dockerfile" -t albert:ubuntu .
```

- `--platform linux/amd64`. Arch has no ARM image. Needed on docker for mac to emulate.
- `--progress=plain` Disables the buildkit output folding


## Run using X

Don't forget to install and run [XQuartz](https://www.xquartz.org/) on macOS.

```sh
docker run --rm -it \
  -e QT_LOGGING_RULES="albert*=true" \
  -e DISPLAY="host.docker.internal:0" \
  albert:ubuntu -c "xterm & albert"
```
