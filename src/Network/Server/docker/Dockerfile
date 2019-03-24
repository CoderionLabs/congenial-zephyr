# To build use:
# docker build -t orian/cppenv:v1 .
FROM ubuntu:16.04
ENV OS=ubuntu1604

LABEL description="A linux C++ build environment."

RUN apt-get update && apt-get install -y \
  gcc \
  clang \
  cmake \
  libgtest-dev \
  libgoogle-glog-dev \
  libboost-all-dev \
  g++ \
  automake \
  autoconf \
  autoconf-archive \
  libtool \
  libboost-all-dev \
  libevent-dev \
  libdouble-conversion-dev \
  libsodium-dev \
  libgoogle-glog-dev \
  libjsonrpccpp-dev \
  libmicrohttpd-dev \
  libcurl4-gnutls-dev \
  libjsonrpccpp-tools \
  libgflags-dev \
  liblz4-dev \
  liblzma-dev \
  libsnappy-dev \
  make \
  wget \
  git \
  zlib1g-dev \
  binutils-dev \
  libjemalloc-dev \
  libssl-dev \
  libiberty-dev

RUN mkdir /app
RUN git clone https://github.com/DokuEnterprise/congenial-zephyr /app/congenial-zephyr
RUN cd /app/congenial-zephyr/src
RUN cp /app/congenial-zephyr/src/Network/Server/docker/build_test_install.sh /app
RUN chmod a+x /app/build_test_install.sh
RUN cd /app && ./build_test_install.sh