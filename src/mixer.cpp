#include "mixer.hpp"

using namespace std;

// Setup the dht
Mixer::Mixer()
{
    int i, rc, fd;
    int s = -1, s6 = -1, port;
    bool have_id = false;
    time_t tosleep = 0;
    char *id_file = "dht.id";
    int opt;
    bool ipv4 = true;
    struct sockaddr_in sin;
    struct sockaddr_storage from;
    socklen_t fromlen;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;


    /* Ids need to be distributed evenly, so you cannot just use your
       bittorrent id.  Either generate it randomly, or take the SHA-1 of
       something. */

    // TODO: Make sure to link agaisnt libssl and libcrypto

    unsigned char ibuf[] = "friedrich";
    unsigned char obuf[20];

    SHA1(ibuf, 9, obuf);
    memcpy(this->myid, obuf, sizeof(obuf));
    have_id = true;

    ofstream out;
    out.open(id_file);
    if(out.fail()){
        cerr << "Failed to open idfile" << endl;
        exit(1);
    }
    out << this->myid;
    out.close();

    port = 1234;

    /* If you set dht_debug to a stream, every action taken by the DHT will
       be logged. */
    if(!this->quiet)
        dht_debug = stdout;

    /* We need an IPv4 socket, bound to a stable port.  Rumour
       has it that uTorrent likes you better when it is the same as your
       Bittorrent port. */
    if(ipv4) {
        s = socket(PF_INET, SOCK_DGRAM, 0);
        if(s < 0) {
            perror("socket(IPv4)");
        }
        rc = set_nonblocking(s, 1);
        if(rc < 0) {
            perror("set_nonblocking(IPv4)");
            exit(1);
        }
    }

    if(s < 0) {
        fprintf(stderr, "That's just not right!");
        exit(1);
    }

    if(s >= 0) {
        sin.sin_port = htons(port);
        rc = bind(s, (struct sockaddr*)&sin, sizeof(sin));
        if(rc < 0) {
            perror("bind(IPv4)");
            exit(1);
        }
    }


    /* Init the dht. */
    rc = dht_init(s, s6, myid, (unsigned char*)"JC\0\0");
    if(rc < 0) {
        perror("dht_init");
        exit(1);
    }

    init_signals();

    {
        struct sockaddr_in sin[500];
        struct sockaddr_in6 sin6[500];
        int num = 500, num6 = 500;
        int i;
        i = dht_get_nodes(sin, &num, sin6, &num6);
        printf("Found %d (%d + %d) good nodes.\n", i, num, num6);
    }

}

Mixer::~Mixer()
{
    dht_uninit();
}

static int
set_nonblocking(int fd, int nonblocking)
{
    int rc;
    rc = fcntl(fd, F_GETFL, 0);
    if(rc < 0)
        return -1;

    rc = fcntl(fd, F_SETFL,
               nonblocking ? (rc | O_NONBLOCK) : (rc & ~O_NONBLOCK));
    if(rc < 0)
        return -1;

    return 0;
}
static void
init_signals(void)
{
    struct sigaction sa;
    sigset_t ss;

    sigemptyset(&ss);
    sa.sa_handler = sigdump;
    sa.sa_mask = ss;
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    sigemptyset(&ss);
    sa.sa_handler = sigtest;
    sa.sa_mask = ss;
    sa.sa_flags = 0;
    sigaction(SIGUSR2, &sa, NULL);

    sigemptyset(&ss);
    sa.sa_handler = sigexit;
    sa.sa_mask = ss;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
}