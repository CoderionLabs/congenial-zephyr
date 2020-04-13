# Congenial Zephyr

Zephyr is an anonymous messaging system that protects the privacy of message contents and message metadata. Users that use Zephyr do not reveal who they are talking to or the contents of their messages. The goal of Zephyr is to decrease the amount of information being sent by the user and hide as much metadata as possible. [https://arxiv.org/pdf/1910.13337.pdf](https://arxiv.org/pdf/1910.13337.pdf)

### TODO

* Implement peer to peer mix nodes.
* Create easy to use UI for clients.
* Setup mailbox database to avoid storing all message in memory.
* Write documentation.

## Installing

It is best to not install Zephyr on your host operating system. To install Zephyr please use docker.

```bash
git clone https://github.com/MutexUnlocked/congenial-zephyr
cd congenial-zephyr/src/Network/Server/docker
docker build -t dems .
```

The image will take 20-30 minutes to build. Once it is done building you will need to create a docker network that supports ipv6. 

```bash
docker network create --driver bridge --ipv6 --subnet 2a02:6b8:b010:9020:1::/80 ipv6_dems
```

Next, you will need to create docker containers. Use this line to create a container.

```bash
docker create --name dems0 --network ipv6_dems --ip ipv4address  -t -i dems
```

Finally, start each container and run the required mixer, mailbox,  pkg, and info nodes on each container. 

The info and mixer nodes will require a list of ip addresses of the other nodes. Here is an example file.

```text
MIXERS
172.18.0.3
172.18.0.4
MAILBOXES
172.18.0.7
PKGS
172.18.0.2
INFO
172.18.0.5
172.18.0.6
```



