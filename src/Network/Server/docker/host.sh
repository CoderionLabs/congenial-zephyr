
# To build use:
# docker build -t dems .

# docker network create --driver bridge --ipv6 --subnet 2a02:6b8:b010:9020:1::/80 ipv6_dems
# docker create --name dems0 --network ipv6_dems --ip ipv4address  -t -i dems
# and create other containers

#$ docker start -a -i 6d8af538ec5

# docker inspect <container name or container id>



apt-get update && apt-get install -y \
  gcc \
  clang \
  cmake \
  libgtest-dev \
  libgoogle-glog-dev \
  libboost-all-dev \
  g++ \
  vim \
  automake \
  autoconf \
  autoconf-archive \
  libtool \
  libboost-all-dev \
  libevent-dev \
  libdouble-conversion-dev \
  libgoogle-glog-dev \
  libjsonrpccpp-dev \
  iputils-ping \
  net-tools \
  netcat \
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

mkdir /app
git clone https://github.com/DokuEnterprise/congenial-zephyr /app/congenial-zephyr
cd /app/congenial-zephyr/src
cp /app/congenial-zephyr/src/Network/Server/docker/deps.sh /app
chmod a+x /app/deps.sh
cd /app && ./deps.sh
cp /app/congenial-zephyr/src/Network/Server/docker/zephyr.sh /app
chmod a+x /app/zephyr.sh
cd /app && ./zephyr.sh
export LD_LIBRARY_PATH=/usr/local/lib
