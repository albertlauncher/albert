choose:
    @just --command-color blue --highlight --choose

build_dir := "build/just"

configure:
    cmake -S . -B {{build_dir}} -DBUILD_TESTS=ON

build build_type='Debug': configure
    cmake --build {{build_dir}} --config {{build_type}} -j$(nproc)

test: build
    ctest --test-dir {{build_dir}} --output-on-failure

install prefix='/usr': build
    cmake --install {{build_dir}} --prefix {{prefix}}

build-dev-image:
    docker build --file .docker/ubuntu.Dockerfile --target dev -t albert:dev .

docker-test-arch:
    docker build \
      --progress plain \
      --file .docker/arch.Dockerfile \
      --tag albert:test-arch \
      .

docker-test-arch-arm:
    docker build \
      --progress plain \
      --file .docker/arch.Dockerfile \
      --platform linux/arm64 \
      --build-arg BASE_IMAGE=agners/archlinuxarm \
      --target build \
      --tag albert:test-arch-arm \
      .

docker-test-ubuntu base-image='ubuntu:24.04':
    docker build \
      --progress plain \
      --file .docker/ubuntu.Dockerfile \
      --build-arg BASE_IMAGE={{base-image}} \
      --target build \
      --tag albert:test-ubuntu \
      .

docker-test-fedora base-image="fedora:latest":
    docker build \
      --progress plain \
      --file .docker/fedora.Dockerfile \
      --build-arg BASE_IMAGE={{base-image}} \
      --target build \
      --tag albert:test-fedora \
      .
