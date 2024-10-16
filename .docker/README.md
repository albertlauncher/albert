# Dockerfiles 

## Build

```sh
docker build --platform linux/amd64 --progress=plain -f arch.Dockerfile          -t albert:arch ..
docker build --platform linux/amd64 --progress=plain -f fedora.latest.Dockerfile -t albert:fedora-latest ..
docker build --platform linux/amd64 --progress=plain -f ubuntu.2404.Dockerfile   -t albert:ubuntu-2404 ..
```

- `--platform linux/amd64` Makes the dockerfiles work on other platforms (Apple Silicon, Pi, …)
- `--progress=plain` Disables the buildkit output folding

## Run

```sh
docker run --rm -it --platform linux/amd64 -e QT_LOGGING_RULES="albert*=true" -e DISPLAY="host.docker.internal:0" --name albert_arch          albert:arch          -c "xterm & albert"
docker run --rm -it --platform linux/amd64 -e QT_LOGGING_RULES="albert*=true" -e DISPLAY="host.docker.internal:0" --name albert_fedora-latest albert:fedora-latest -c "xterm & albert"
docker run --rm -it --platform linux/amd64 -e QT_LOGGING_RULES="albert*=true" -e DISPLAY="host.docker.internal:0" --name albert_ubuntu-2404   albert:ubuntu-2404   -c "xterm & albert"
```

Don't forget to install and run [XQuartz](https://www.xquartz.org/) on macOS
